#include "GacBuild.h"

namespace gacbuild
{
	using namespace vl::console;
	using namespace vl::glr;

	void Require(bool condition, const WString& message)
	{
		if (!condition) throw Exception(message);
	}

	WString ReadText(const FilePath& path)
	{
		WString text;
		Require(File(path).ReadAllTextByBom(text), L"Cannot read: " + path.GetFullPath());
		return text;
	}

	void EnsureFolder(const FilePath& path)
	{
		Require(path.IsFolder() || Folder(path).Create(true), L"Cannot create directory: " + path.GetFullPath());
	}

	void WriteLines(const FilePath& path, List<WString>& lines)
	{
		Require(File(path).WriteAllLines(lines, false, stream::BomEncoder::Utf8), L"Cannot write: " + path.GetFullPath());
	}

	WString Attribute(Ptr<xml::XmlElement> element, const WString& name)
	{
		auto attribute = xml::XmlGetAttribute(element, name);
		return attribute ? attribute->value.value : WString();
	}

	Ptr<xml::XmlElement> ReadXml(const FilePath& path, xml::Parser& parser)
	{
		Console::WriteLine(L"Reading: " + path.GetFullPath());
		WString text;
		stream::BomEncoder::Encoding encoding;
		bool containsBom;
		Require(File(path).ReadAllTextWithEncodingTesting(text, encoding, containsBom), L"Cannot read XML: " + path.GetFullPath());
		return xml::XmlParseDocument(text, parser)->rootElement;
	}

	struct Options
	{
		WString						mode;
		FilePath					gacgen;
		FilePath					cppmerge;
		FilePath					input;
		Nullable<FilePath>			mapping;
		bool						dump = false;
	};

	Options ParseOptions(const List<WString>& arguments)
	{
		Dictionary<WString, WString> values;
		for (vint i = 0; i < arguments.Count(); i++)
		{
			auto argument = arguments[i];
			Require(argument.Length() > 1 && argument[0] == L'-', L"Expected an option, got: " + argument);
			auto colon = argument.IndexOf(L':');
			auto key = wlower(colon == -1 ? argument.Sub(1, argument.Length() - 1) : argument.Sub(1, colon - 1));
			Require(key == L"mode" || key == L"pathgacgen" || key == L"pathcppmerge" || key == L"filename" || key == L"mappingfilename" || key == L"dump", L"Unknown option: " + argument);
			Require(!values.Keys().Contains(key), L"Duplicate option: " + key);
			WString value;
			if (key == L"dump")
			{
				Require(colon == -1, L"-Dump does not take a value.");
				value = L"true";
			}
			else
			{
				if (colon != -1) value = argument.Sub(colon + 1, argument.Length() - colon - 1);
				else { Require(i + 1 < arguments.Count(), L"Missing value for: " + key); value = arguments[++i]; }
				Require(value.Length() > 0, L"Empty value for: " + key);
			}
			values.Add(key, value);
		}
		const wchar_t* required[] = { L"mode", L"pathgacgen", L"pathcppmerge", L"filename" };
		for (auto key : required) Require(values.Keys().Contains(WString::Unmanaged(key)), L"Missing option: " + WString::Unmanaged(key));
		auto mode = values[L"mode"];
		Require(mode == L"GacBuild" || mode == L"GacGen", L"-mode must be GacBuild or GacGen.");
		ValidateExecutable(values[L"pathgacgen"]);
		ValidateExecutable(values[L"pathcppmerge"]);
		Options options;
		options.mode = mode;
		options.gacgen = values[L"pathgacgen"];
		options.cppmerge = values[L"pathcppmerge"];
		options.input = values[L"filename"];
		Require(options.input.IsFile(), L"Input does not exist: " + options.input.GetFullPath());
		options.dump = values.Keys().Contains(L"dump");
		Require(!options.dump || mode == L"GacBuild", L"-Dump is only valid in GacBuild mode.");
		if (values.Keys().Contains(L"mappingfilename"))
		{
			Require(mode == L"GacGen", L"-MappingFileName is only valid in GacGen mode.");
			options.mapping = FilePath(values[L"mappingfilename"]);
			Require(options.mapping.Value().IsFile(), L"Mapping does not exist: " + options.mapping.Value().GetFullPath());
		}
		return options;
	}

	struct CopyEntry
	{
		FilePath					source;
		FilePath					destination;
	};

	void RequireOutput(const FilePath& path)
	{
		auto info = GetFileInfo(path);
		Require(info && info.Value().size > 0, L"Missing or empty output: " + path.GetFullPath());
	}

	void RunGacGen(const Options& options, const FilePath& input, Nullable<FilePath> mapping, xml::Parser& parser)
	{
		auto log = FilePath(input.GetFullPath() + L".log");
		Require(!log.IsFolder() || Folder(log).Delete(true), L"Cannot clear resource cache: " + log.GetFullPath());
		try
		{
			const wchar_t* architectures[] = { L"x32", L"x64" };
			for (auto architecture : architectures)
			{
				List<WString> arguments;
				arguments.Add(architecture == architectures[0] ? L"/P32" : L"/P64");
				arguments.Add(input.GetFullPath());
				if (mapping) arguments.Add(mapping.Value().GetFullPath());
				RunProcess(options.gacgen, arguments);
				auto folder = log / WString::Unmanaged(architecture);
				auto errors = folder / L"Errors.txt";
				if (errors.IsFile()) throw Exception(ReadText(errors));
				const wchar_t* outputs[] = { L"Resource.bin", L"Compressed.bin", L"ScriptedResource.bin", L"ScriptedCompressed.bin", L"Assembly.bin", L"Workflow.txt", L"Deploy.xml" };
				for (auto output : outputs) RequireOutput(folder / WString::Unmanaged(output));
			}

			// The x32 recipe is last, matching the old post-/P64 neutral deployment.
			List<CopyEntry> copies;
			for (vint i = 1; i >= 0; i--)
			{
				auto folder = log / WString::Unmanaged(architectures[i]);
				auto deployment = ReadXml(folder / L"Deploy.xml", parser);
				Require(deployment->name.value == L"Deploy", L"Invalid deployment manifest.");
				auto entries = xml::XmlGetElements(deployment, L"File");
				for (auto entry : entries)
				{
					auto source = Attribute(entry, L"Source");
					Require(IsAbsolutePath(source), L"Deployment source must be absolute.");
					auto destination = Attribute(entry, L"Destination");
					Require(IsAbsolutePath(destination), L"Deployment destination must be absolute.");
					CopyEntry copy{ .source = source, .destination = destination };
					RequireOutput(copy.source);
					copies.Add(copy);
				}
			}

			auto cpp32 = log / L"x32" / L"CppOutput.txt";
			auto cpp64 = log / L"x64" / L"CppOutput.txt";
			Require(cpp32.IsFile() == cpp64.IsFile(), L"C++ configuration differs between architectures.");
			Nullable<FilePath> cppDestination;
			if (cpp32.IsFile())
			{
				auto output = ReadText(cpp32);
				Require(IsAbsolutePath(output) && output == ReadText(cpp64), L"C++ destinations must match and be absolute.");
				FilePath destination(output);
				EnsureFolder(destination);
				cppDestination = destination;
			}
			for (auto&& copy : copies)
			{
				Require(copy.destination.GetFolder().IsFolder() && !copy.destination.IsFolder(), L"Deployment destination directory is unavailable: " + copy.destination.GetFullPath());
			}
			if (cppDestination)
			{
				auto destination = cppDestination.Value();
				List<File> files32;
				List<File> files64;
				Require(Folder(log / L"x32" / L"Source").GetFiles(files32) && Folder(log / L"x64" / L"Source").GetFiles(files64), L"Cannot enumerate C++ staging directories.");
				SortedList<WString> names32;
				SortedList<WString> names64;
				for (auto&& file : files32) names32.Add(file.GetFilePath().GetName());
				for (auto&& file : files64) names64.Add(file.GetFilePath().GetName());
				Require(names32.Count() > 0 && names32.Count() == names64.Count(), L"C++ staged filename sets differ or are empty.");
				for (vint i = 0; i < names32.Count(); i++) Require(names32[i] == names64[i], L"C++ staged filename sets differ.");
				for (auto&& name : names32)
				{
					auto source32 = log / L"x32" / L"Source" / name;
					auto source64 = log / L"x64" / L"Source" / name;
					RequireOutput(source32);
					RequireOutput(source64);
				}
				for (auto&& name : names32)
				{
					auto source32 = log / L"x32" / L"Source" / name;
					auto source64 = log / L"x64" / L"Source" / name;
					auto target = destination / name;
					// CppMerge's historical CLI ignores failed writes. Verify with its owning library algorithms.
					auto expected = workflow::cppcodegen::MergeCppMultiPlatform(ReadText(source32), ReadText(source64));
					if (target.IsFile()) expected = workflow::cppcodegen::MergeCppFileContent(ReadText(target), expected);
					List<WString> arguments;
					arguments.Add(source32.GetFullPath());
					arguments.Add(source64.GetFullPath());
					arguments.Add(target.GetFullPath());
					Console::WriteLine(L"Merging: " + target.GetFullPath());
					RunProcess(options.cppmerge, arguments);
					Require(ReadText(target) == expected, L"CppMerge did not publish the expected output: " + target.GetFullPath());
				}
			}
			for (auto&& copy : copies)
			{
				Console::WriteLine(L"Copying: " + copy.source.GetFullPath() + L" => " + copy.destination.GetFullPath());
				CopyFileNative(copy.source, copy.destination);
			}
		}
		catch (...)
		{
			// Completed compiler caches must not hide failed merge/deployment work on a retry.
			File cache(log / L"x32" / L"Assembly.bin");
			Require(!cache.Exists() || cache.Delete(), L"Cannot invalidate failed resource cache: " + cache.GetFilePath().GetFullPath());
			throw;
		}
	}

	struct Resource : Object
	{
		FilePath					path;
		WString						name;
		SortedList<WString>			dependencies;
		bool						outdated = false;
	};

	void FindElements(Ptr<xml::XmlElement> root, const WString& name, List<Ptr<xml::XmlElement>>& found)
	{
		List<Ptr<xml::XmlElement>> pending;
		pending.Add(root);
		for (vint i = 0; i < pending.Count(); i++)
		{
			auto element = pending[i];
			if (element->name.value == name) found.Add(element);
			auto children = xml::XmlGetElements(element);
			for (auto child : children) pending.Add(child);
		}
	}

	void RunGacBuild(const Options& options, xml::Parser& parser)
	{
		auto driver = ReadXml(options.input, parser);
		Require(driver->name.value == L"GacUI", L"GacBuild requires a GacUI driver XML, distinct from resource XML.");
		List<WString> excludes;
		List<Ptr<xml::XmlElement>> exclusionElements;
		FindElements(driver, L"Exclude", exclusionElements);
		for (auto element : exclusionElements) excludes.Add(Attribute(element, L"Pattern"));
		auto root = options.input.GetFolder();
		auto log = FilePath(options.input.GetFullPath() + L".log");
		Require(!log.IsFolder() || Folder(log).Delete(true), L"Cannot clear driver log: " + log.GetFullPath());
		EnsureFolder(log);
		List<Folder> folders;
		folders.Add(Folder(root));
		SortedList<WString> resourcePaths;
		for (vint i = 0; i < folders.Count(); i++)
		{
			List<File> files;
			List<Folder> children;
			Require(folders[i].GetFiles(files) && folders[i].GetFolders(children), L"Cannot enumerate: " + folders[i].GetFilePath().GetFullPath());
			for (auto&& child : children) folders.Add(child);
			for (auto&& file : files)
			{
				auto path = file.GetFilePath().GetFullPath();
				if (path.Length() < 4 || wlower(path.Right(4)) != L".xml") continue;
				auto normalized = path;
				for (vint j = 0; j < normalized.Length(); j++)
				{
					if (normalized[j] == L'\\') normalized = normalized.Left(j) + L"/" + normalized.Sub(j + 1, normalized.Length() - j - 1);
				}
				bool excluded = false;
				for (auto&& pattern : excludes) if (INVLOC.FindFirst(normalized, pattern, Locale::None).key != -1) { excluded = true; break; }
				if (excluded) continue;
				auto document = ReadXml(file.GetFilePath(), parser);
				List<Ptr<xml::XmlElement>> roots;
				FindElements(document, L"Resource", roots);
				bool resource = false;
				for (auto element : roots)
				{
					auto configs = xml::XmlGetElements(element, L"Folder");
					for (auto config : configs) if (Attribute(config, L"name") == L"GacGenConfig") resource = true;
				}
				if (resource) resourcePaths.Add(path);
			}
		}
		List<WString> relativePaths;
		for (auto&& path : resourcePaths) relativePaths.Add(path.Sub(root.GetFullPath().Length(), path.Length() - root.GetFullPath().Length()));
		WriteLines(log / L"ResourceFiles.txt", relativePaths);

		List<Ptr<Resource>> resources;
		Dictionary<WString, Ptr<Resource>> named;
		SortedList<WString> dumpNames;
		for (vint i = 0; i < resourcePaths.Count(); i++)
		{
			auto relative = relativePaths[i];
			Array<wchar_t> flattened(relative.Length() + 1);
			for (vint j = 0; j < relative.Length(); j++) flattened[j] = relative[j] == L'\\' || relative[j] == L'/' ? L'_' : relative[j];
			flattened[relative.Length()] = 0;
			WString dumpName(&flattened[0]);
			Require(!dumpNames.Contains(dumpName), L"Resource paths produce the same flattened dump: " + dumpName);
			dumpNames.Add(dumpName);
			auto dumpPath = log / dumpName;
			List<WString> arguments;
			arguments.Add(L"/D32");
			arguments.Add(resourcePaths[i]);
			arguments.Add(dumpPath.GetFullPath());
			RunProcess(options.gacgen, arguments);
			auto dump = ReadXml(dumpPath, parser);
			Require(dump->name.value == L"ResourceMetadata", L"Invalid resource dump: " + dumpPath.GetFullPath());
			auto metadata = xml::XmlGetElement(dump, L"ResourceMetadata");
			auto inputs = xml::XmlGetElement(dump, L"Inputs");
			auto outputs = xml::XmlGetElement(dump, L"Outputs");
			Require(metadata && inputs && outputs, L"Incomplete resource dump: " + dumpPath.GetFullPath());
			auto resource = Ptr(new Resource);
			resource->path = resourcePaths[i];
			resource->name = Attribute(metadata, L"Name");
			if (auto dependencies = xml::XmlGetElement(metadata, L"Dependencies"))
			{
				auto dependencyElements = xml::XmlGetElements(dependencies, L"Resource");
				for (auto dependency : dependencyElements)
				{
					auto name = Attribute(dependency, L"Name");
					Require(name.Length() > 0 && resource->name.Length() > 0, L"Dependencies require named resources.");
					if (!resource->dependencies.Contains(name)) resource->dependencies.Add(name);
				}
			}
			vuint64_t latestInput = 0;
			auto inputElements = xml::XmlGetElements(inputs, L"Input");
			for (auto input : inputElements)
			{
				auto path = Attribute(input, L"Path");
				auto info = GetFileInfo(FilePath(path));
				Require(info, L"Resource input is unavailable: " + path);
				if (info.Value().modified > latestInput) latestInput = info.Value().modified;
			}
			auto outputElements = xml::XmlGetElements(outputs, L"Output");
			vint outputCount = 0;
			for (auto output : outputElements)
			{
				outputCount++;
				auto info = GetFileInfo(FilePath(Attribute(output, L"Path")));
				if (!info || info.Value().modified < latestInput) resource->outdated = true;
			}
			Require(outputCount == 10, L"Expected ten standard cache outputs.");
			resources.Add(resource);
			if (resource->name.Length() > 0)
			{
				Require(!named.Keys().Contains(resource->name), L"Duplicate resource name: " + resource->name);
				named.Add(resource->name, resource);
			}
		}
		for (auto resource : resources)
		{
			for (auto&& dependency : resource->dependencies) Require(named.Keys().Contains(dependency), L"Resource " + resource->name + L" depends on missing resource " + dependency);
		}
		List<Ptr<Resource>> ordered;
		SortedList<WString> completed;
		List<WString> anonymousPaths;
		List<WString> namedPaths;
		for (auto resource : resources) if (resource->name.Length() == 0) { ordered.Add(resource); anonymousPaths.Add(resource->path.GetFullPath()); }
		while (completed.Count() < named.Count())
		{
			auto before = completed.Count();
			for (vint i = 0; i < named.Count(); i++)
			{
				auto resource = named.Values()[i];
				if (completed.Contains(resource->name)) continue;
				bool ready = true;
				for (auto&& dependency : resource->dependencies) if (!completed.Contains(dependency)) ready = false;
				if (!ready) continue;
				for (auto&& dependency : resource->dependencies) if (named[dependency]->outdated) resource->outdated = true;
				completed.Add(resource->name);
				ordered.Add(resource);
				namedPaths.Add(resource->path.GetFullPath());
			}
			Require(completed.Count() > before, L"Resource dependency graph contains a cycle.");
		}
		List<WString> candidates;
		for (auto resource : ordered) if (resource->outdated) candidates.Add(resource->path.GetFullPath());
		List<WString> mappings;
		for (vint i = 0; i < named.Count(); i++) mappings.Add(named.Keys()[i] + L"=>" + named.Values()[i]->path.GetFullPath());
		auto mapping = log / L"ResourceNamedMapping.txt";
		WriteLines(log / L"ResourceAnonymousFiles.txt", anonymousPaths);
		WriteLines(log / L"ResourceNamedFiles.txt", namedPaths);
		WriteLines(log / L"BuildCandidates.txt", candidates);
		WriteLines(mapping, mappings);
		if (options.dump) return;
		for (auto resource : ordered)
		{
			Console::WriteLine((resource->outdated ? WString(L"[BUILD] ") : WString(L"[SKIPPED] ")) + resource->path.GetFullPath());
			if (resource->outdated) RunGacGen(options, resource->path, Nullable<FilePath>(mapping), parser);
		}
	}

	void Run(const List<WString>& arguments)
	{
		auto options = ParseOptions(arguments);
		xml::Parser parser;
		if (options.mode == L"GacGen") RunGacGen(options, options.input, options.mapping, parser);
		else RunGacBuild(options, parser);
	}
}
