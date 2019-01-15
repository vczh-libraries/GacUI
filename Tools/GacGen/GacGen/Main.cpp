#include "GacGen.h"

using namespace vl::filesystem;

Array<WString>* arguments = 0;

int wmain(int argc, wchar_t* argv[])
{
	Array<WString> _arguments(argc - 1);
	for (vint i = 1; i < argc; i++)
	{
		_arguments[i - 1] = argv[i];
	}
	arguments = &_arguments;
	SetupWindowsDirect2DRenderer();
}

void PrintErrors(List<GuiResourceError>& errors)
{
	List<WString> output;
	GuiResourceError::SortAndLog(errors, output);
	FOREACH(WString, line, output)
	{
		PrintErrorMessage(line);
	}
}

void SaveErrors(FilePath errorFilePath, List<GuiResourceError>& errors)
{
	PrintErrors(errors);
	if (WriteErrors(errors, errorFilePath))
	{
		PrintErrorMessage(L"error> Error information is saved at : " + errorFilePath.GetFullPath());
	}
	else
	{
		PrintErrorMessage(L"error> Unable to write : " + errorFilePath.GetFullPath());
	}
}

class Callback : public Object, public IGuiResourcePrecompileCallback, public IWfCompilerCallback
{
public:
	vint lastPass = -1;

	void OnLoadEnvironment()override
	{
		PrintInformationMessage(L"    Workflow: Loading metadata from registered types ...");
	}

	void OnInitialize(analyzer::WfLexicalScopeManager* manager)override
	{
		PrintInformationMessage(L"    Workflow: Creating metadata from declarations ...");
	}

	void OnValidateModule(Ptr<WfModule> module)override
	{
		PrintInformationMessage(L"    Workflow: Validating module " + module->name.value + L" ...");
	}

	void OnGenerateMetadata()override
	{
		PrintInformationMessage(L"    Workflow: Generating metadata ...");
	}

	void OnGenerateCode(Ptr<WfModule> module)override
	{
		PrintInformationMessage(L"    Workflow: Generating code for module " + module->name.value + L" ...");
	}

	void OnGenerateDebugInfo()override
	{
		PrintInformationMessage(L"    Workflow: Generating debug information ...");
	}

	IWfCompilerCallback* GetCompilerCallback()override
	{
		return this;
	}

	void PrintPass(vint passIndex)
	{
		if (lastPass != passIndex)
		{
			lastPass = passIndex;
			switch (passIndex)
			{
			case IGuiResourceTypeResolver_Precompile::Workflow_Collect:
				PrintInformationMessage(L"Pass: (1/8) Collect workflow scripts");
				break;
			case IGuiResourceTypeResolver_Precompile::Workflow_Compile:
				PrintInformationMessage(L"Pass: (2/8) Compile view model scripts");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CollectInstanceTypes:
				PrintInformationMessage(L"Pass: (3/8) Collect instances");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CompileInstanceTypes:
				PrintInformationMessage(L"Pass: (4/8) Validate instances");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CollectEventHandlers:
				PrintInformationMessage(L"Pass: (5/8) Generate instance type stubs");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CompileEventHandlers:
				PrintInformationMessage(L"Pass: (6/8) Compile instance type stubs");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_GenerateInstanceClass:
				PrintInformationMessage(L"Pass: (7/8) Generate instance types");
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CompileInstanceClass:
				PrintInformationMessage(L"Pass: (8/8) Compile instance types");
				break;
			}
		}
	}

	void OnPerPass(vint passIndex)override
	{
		PrintPass(passIndex);
	}

	void OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource)override
	{
		PrintPass(passIndex);
		PrintInformationMessage(L"    " + resource->GetResourcePath());
	}
};

struct LoadConfigResult
{
	Ptr<GuiResource>			resource;
	Ptr<CodegenConfig>			config;
};

LoadConfigResult LoadConfig(FilePath inputPath)
{
	List<GuiResourceError> errors;
	auto resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);

	for (vint i = errors.Count() - 1; i >= 0; i--)
	{
		auto error = errors[i];
		if (INVLOC.FindFirst(error.message, L"Path of imported folder does not exist:", Locale::None).key == 0)
		{
			errors.RemoveAt(i);
		}
	}

	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"error> Failed to load resource.");
		PrintErrors(errors);
		return {};
	}

	auto config = CodegenConfig::LoadConfig(resource, errors);
	if (!config)
	{
		PrintErrorMessage(L"error> Failed to load config.");
		return {};
	}
	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"error> Failed to load resource metadata.");
		PrintErrors(errors);
		return {};
	}

	LoadConfigResult result;
	result.resource = resource;
	result.config = config;
	return result;
}

bool LoadDependencies(Ptr<CodegenConfig> config, Dictionary<WString, FilePath>& resourceMappings, const WString& logFolderPostfix, FilePath errorFilePath)
{
	if (config->metadata->version == L"")
	{
		config->metadata->version = GuiResource::CurrentVersionString;
	}

	if (config->metadata->version != GuiResource::CurrentVersionString)
	{
		PrintErrorMessage(L"error> Version of the resource should be \"" + WString(GuiResource::CurrentVersionString) + L"\"");
		return false;
	}

	if (config->metadata->dependencies.Count() > 0 && config->metadata->name == L"")
	{
		PrintErrorMessage(L"error> Name of the resource should not be empty if it has dependencies.");
		return false;
	}

	SortedList<WString> requireds, loaded;
	CopyFrom(requireds, config->metadata->dependencies);

	while (requireds.Count() > 0)
	{
		auto required = requireds[requireds.Count() - 1];
		requireds.RemoveAt(requireds.Count() - 1);
		if (loaded.Contains(required)) continue;

		vint index = resourceMappings.Keys().IndexOf(required);
		if (index == -1)
		{
			PrintErrorMessage(L"error> Cannot find the resource file of name: " + required);
			return false;
		}

		FilePath resourceBinary = FilePath(resourceMappings.Values()[index].GetFullPath() + logFolderPostfix) / L"ScriptedResource.bin";
		MemoryStream resourceStream;
		{
			FileStream fileStream(resourceBinary.GetFullPath(), FileStream::ReadOnly);
			if (!fileStream.IsAvailable())
			{
				PrintErrorMessage(L"error> Cannot find binary resource file, please make sure the required resource \"" + required + L"\" has been properly compiled: " + required);
				return false;
			}
			CopyStream(fileStream, resourceStream);
		}

		loaded.Add(required);
		{
			resourceStream.SeekFromBegin(0);
			stream::internal::ContextFreeReader reader(resourceStream);

			WString metadataText;
			reader << metadataText;

			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
			auto xmlMetadata = parser->Parse({}, metadataText, errors);
			if (!xmlMetadata || errors.Count() > 0)
			{
				PrintErrorMessage(L"error> The binary resource file has an invalid metadata, please make sure the required resource \"" + required + L"\" has been properly compiled: " + required);
				return false;
			}

			auto metadata = MakePtr<GuiResourceMetadata>();
			metadata->LoadFromXml(xmlMetadata, {}, errors);
			if (errors.Count() > 0)
			{
				PrintErrorMessage(L"error> The binary resource file has an invalid metadata, please make sure the required resource \"" + required + L"\" has been properly compiled: " + required);
				return false;
			}

			CopyFrom(requireds, metadata->dependencies, true);
		}

		resourceStream.SeekFromBegin(0);
		GuiResourceError::List errors;
		GetResourceManager()->LoadResourceOrPending(resourceStream, errors, GuiResourceUsage::InstanceClass);
		if (errors.Count() > 0)
		{
			SaveErrors(errorFilePath, errors);
			return false;
		}
	}

	List<WString> pendings;
	GetResourceManager()->GetPendingResourceNames(pendings);
	if (pendings.Count() > 0)
	{
		PrintErrorMessage(L"error> Unable to find dependencies: "
			+ From(pendings)
				.Aggregate(WString::Empty, [](const WString& a, const WString& b) {return a == L"" ? b : a + L", " + b; })
			);
		return false;
	}
	return true;
}

void CompileResource(bool partialMode, FilePath inputPath, Nullable<FilePath> mappingPath)
{
	PrintSuccessMessage(L"gacgen> Clearning logs ... : " + inputPath.GetFullPath());
#if defined VCZH_64
	WString logFolderPostfix = L".log/x64";
#else
	WString logFolderPostfix = L".log/x32";
#endif

	FilePath logFolderPath = inputPath.GetFullPath() + logFolderPostfix;
	FilePath scriptFilePath = logFolderPath / L"Workflow.txt";
	FilePath errorFilePath = logFolderPath / L"Errors.txt";
	FilePath workingDir = inputPath.GetFolder();

	{
		auto loadConfigResult = LoadConfig(inputPath);
		if (!loadConfigResult.resource) return;

		Dictionary<WString, FilePath> resourceMappings;
		if (mappingPath)
		{
			FileStream fileStream(mappingPath.Value().GetFullPath(), FileStream::ReadOnly);
			if (!fileStream.IsAvailable())
			{
				PrintErrorMessage(L"error> Failed to load mapping file: " + mappingPath.Value().GetFullPath());
				return;
			}
			BomDecoder decoder;
			DecoderStream decoderStream(fileStream, decoder);
			StreamReader reader(decoderStream);
			while (!reader.IsEnd())
			{
				auto line = reader.ReadLine();
				if (line != L"")
				{
					auto arrow = INVLOC.FindFirst(line, L"=>", Locale::None);
					if (arrow.key == -1)
					{
						PrintErrorMessage(L"warning> Unable to parse mapping information: " + line);
						return;
					}
					auto name = line.Left(arrow.key);
					auto value = line.Right(line.Length() - arrow.key - arrow.value);
					if (resourceMappings.Keys().Contains(name))
					{
						PrintErrorMessage(L"warning> Find duplicate mapping information: " + line);
						return;
					}
					resourceMappings.Add(name, value);
				}
			}
		}

		if (!LoadDependencies(loadConfigResult.config, resourceMappings, logFolderPostfix, errorFilePath))
		{
			return;
		}
	}

	if (partialMode)
	{
		PrintInformationMessage(L"gacgen> Partial mode activated, all output files will be put under " + logFolderPath.GetFullPath());
	}

	{
		Folder logFolder(logFolderPath);
		if (logFolder.Exists())
		{
			if (!logFolder.Delete(true))
			{
				PrintErrorMessage(L"gacgen> Unable to delete file in the log folder : " + logFolderPath.GetFullPath());
				return;
			}
		}
		if (!logFolder.Create(true))
		{
			Thread::Sleep(500);
			if (!logFolder.Create(true))
			{
				PrintErrorMessage(L"gacgen> Unable to create log folder : " + logFolderPath.GetFullPath());
				return;
			}
		}
	}

	Ptr<GuiResource> resource;
	{
		PrintSuccessMessage(L"gacgen> Making : " + inputPath.GetFullPath());
		List<GuiResourceError> errors;
		resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
		if (errors.Count() > 0)
		{
			PrintErrorMessage(L"error> Failed to load resource.");
			SaveErrors(errorFilePath, errors);
			return;
		}
	}

	List<FilePath> cppResourcePaths;
	List<FilePath> cppCompressedPaths;
	List<FilePath> resResourcePaths;
	List<FilePath> resCompressedPaths;
	List<FilePath> resAssemblyPaths;
	Ptr<CodegenConfig::CppOutput> cppOutput;
	Ptr<CodegenConfig::ResOutput> resOutput;
	{
		Ptr<CodegenConfig> config;
		{
			List<GuiResourceError> errors;
			config = CodegenConfig::LoadConfig(resource, errors);
			if (!config)
			{
				PrintErrorMessage(L"error> Failed to load config.");
				return;
			}
			if (errors.Count() > 0)
			{
				PrintErrorMessage(L"error> Failed to load resource metadata.");
				SaveErrors(errorFilePath, errors);
				return;
			}
		}
		{
			cppResourcePaths.Add(logFolderPath / L"Resource.bin");
			cppCompressedPaths.Add(logFolderPath / L"Compressed.bin");
			resResourcePaths.Add(logFolderPath / L"ScriptedResource.bin");
			resCompressedPaths.Add(logFolderPath / L"ScriptedCompressed.bin");
			resAssemblyPaths.Add(logFolderPath / L"Assembly.bin");
		}

		cppOutput = config->cppOutput;
#ifdef VCZH_64
		resOutput = config->resOutputx64;
#else
		resOutput = config->resOutputx32;
#endif
		if (cppOutput)
		{
			if (config->cppOutput->resource != L"") cppResourcePaths.Add(workingDir / config->cppOutput->resource);
			if (config->cppOutput->compressed != L"") resAssemblyPaths.Add(workingDir / config->cppOutput->compressed);
		}
#ifdef VCZH_64
		if (resOutput)
#else
		if (resOutput)
#endif
		{
			if (resOutput->resource != L"") resResourcePaths.Add(workingDir / resOutput->resource);
			if (resOutput->compressed != L"") resCompressedPaths.Add(workingDir / resOutput->compressed);
			if (resOutput->assembly != L"") resAssemblyPaths.Add(workingDir / resOutput->assembly);
		}
	}

	PrintSuccessMessage(L"gacgen> Compiling...");
	List<GuiResourceError> errors;
	Callback callback;

	auto precompiledFolder = PrecompileResource(resource, &callback, errors);
	if (errors.Count() > 0)
	{
		SaveErrors(errorFilePath, errors);
		return;
	}

	if (auto compiled = WriteWorkflowScript(precompiledFolder, L"Workflow/InstanceClass", scriptFilePath))
	{
		if (cppOutput)
		{
			PrintSuccessMessage(L"gacgen> Generating C++ source code ...");
			auto input = MakePtr<WfCppInput>(cppOutput->name);
			input->multiFile = WfCppFileSwitch::Enabled;
			input->reflection = WfCppFileSwitch::Enabled;
			input->comment = L"GacGen.exe " + FilePath(inputPath).GetName();
			input->defaultFileName = cppOutput->name + L"PartialClasses";
			input->includeFileName = cppOutput->name;
			CopyFrom(input->normalIncludes, cppOutput->normalIncludes);
			CopyFrom(input->reflectionIncludes, cppOutput->reflectionIncludes);

			FilePath cppFolder = workingDir / cppOutput->sourceFolder;
			if (partialMode)
			{
				File(logFolderPath / L"CppOutput.txt").WriteAllText(cppFolder.GetFullPath(), false, BomEncoder::Mbcs);

				cppFolder = logFolderPath / L"Source";
				if (!Folder(cppFolder).Create(true))
				{
					PrintSuccessMessage(L"gacgen> Unable to create source folder : " + cppFolder.GetFullPath());
				}
			}

			auto output = WriteCppCodesToFile(resource, compiled, input, cppFolder, errors);
			if (errors.Count() > 0)
			{
				SaveErrors(errorFilePath, errors);
				return;
			}

			if (cppOutput->cppResource != L"")
			{
				WriteEmbeddedResource(resource, input, output, false, cppFolder / cppOutput->cppResource);
			}
			if (cppOutput->cppCompressed != L"")
			{
				WriteEmbeddedResource(resource, input, output, true, cppFolder / cppOutput->cppCompressed);
			}
		}

		FOREACH(FilePath, filePath, cppResourcePaths)
		{
			PrintSuccessMessage(L"Generating binary resource file (no script): " + filePath.GetFullPath());
			WriteBinaryResource(resource, false, false, filePath, {});
		}
		FOREACH(FilePath, filePath, cppCompressedPaths)
		{
			PrintSuccessMessage(L"Generating compressed resource file (no script): " + filePath.GetFullPath());
			WriteBinaryResource(resource, true, false, filePath, {});
		}
		FOREACH(FilePath, filePath, resResourcePaths)
		{
			PrintSuccessMessage(L"Generating binary resource files : " + filePath.GetFullPath());
			WriteBinaryResource(resource, false, true, filePath, {});
		}
		FOREACH(FilePath, filePath, resCompressedPaths)
		{
			PrintSuccessMessage(L"Generating compressed resource files : " + filePath.GetFullPath());
			WriteBinaryResource(resource, true, true, filePath, {});
		}
		FOREACH(FilePath, filePath, resAssemblyPaths)
		{
			PrintSuccessMessage(L"Generating assembly files : " + filePath.GetFullPath());
			WriteBinaryResource(resource, false, false, {}, filePath);
		}

		if (partialMode)
		{
			List<WString> lines;
			List<FilePath>* outputPaths[] =
			{
				&cppResourcePaths,
				&cppCompressedPaths,
				&resResourcePaths,
				&resCompressedPaths,
				&resAssemblyPaths,
			};

			for (vint i = 0; i < sizeof(outputPaths) / sizeof(*outputPaths); i++)
			{
				auto& paths = *outputPaths[i];
				if (paths.Count() == 2)
				{
					lines.Add(L"copy \"" + paths[0].GetFullPath() + L"\" \"" + paths[1].GetFullPath() + L"\"");
				}
			}

			if (lines.Count() > 0)
			{
				File(logFolderPath / L"Deploy.bat").WriteAllLines(lines, false, BomEncoder::Mbcs);
			}
		}
	}
}

void DumpResource(FilePath inputPath, FilePath outputPath)
{
	PrintSuccessMessage(L"gacgen> Dumping : " + inputPath.GetFullPath());
	auto loadConfigResult = LoadConfig(inputPath);
	if (!loadConfigResult.resource) return;

	auto doc = MakePtr<XmlDocument>();
	auto xmlRoot = MakePtr<XmlElement>();
	xmlRoot->name.value = L"ResourceMetadata";
	doc->rootElement = xmlRoot;
	
	xmlRoot->subNodes.Add(loadConfigResult.resource->GetMetadata()->SaveToXml());
	{
		SortedList<WString> paths;
		List<Ptr<GuiResourceFolder>> folders;
		folders.Add(loadConfigResult.resource);
		for (vint i = 0; i < folders.Count(); i++)
		{
			auto currentFolder = folders[i];
			if (currentFolder->GetFileAbsolutePath() != L"" && !paths.Contains(currentFolder->GetFileAbsolutePath()))
			{
				paths.Add(currentFolder->GetFileAbsolutePath());
			}

			FOREACH(Ptr<GuiResourceItem>, item, currentFolder->GetItems())
			{
				if (item->GetFileAbsolutePath() != L"" && !paths.Contains(item->GetFileAbsolutePath()))
				{
					paths.Add(item->GetFileAbsolutePath());
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, currentFolder->GetFolders())
			{
				folders.Add(folder);
			}
		}

		auto xmlInputs = MakePtr<XmlElement>();
		xmlInputs->name.value = L"Inputs";
		xmlRoot->subNodes.Add(xmlInputs);

		FOREACH(WString, path, paths)
		{
			auto xmlInput = MakePtr<XmlElement>();
			xmlInput->name.value = L"Input";
			xmlInputs->subNodes.Add(xmlInput);
			{
				auto attr = MakePtr<XmlAttribute>();
				attr->name.value = L"Path";
				attr->value.value = path;
				xmlInput->attributes.Add(attr);
			}
		}
	}
	{
		SortedList<WString> paths;
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x32" / L"Resource.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x32" / L"Compressed.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x32" / L"ScriptedResource.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x32" / L"ScriptedCompressed.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x32" / L"Assembly.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x64" / L"Resource.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x64" / L"Compressed.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x64" / L"ScriptedResource.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x64" / L"ScriptedCompressed.bin").GetFullPath());
		paths.Add((FilePath(inputPath.GetFullPath() + L".log") / L"x64" / L"Assembly.bin").GetFullPath());

		auto xmlOutputs = MakePtr<XmlElement>();
		xmlOutputs->name.value = L"Outputs";
		xmlRoot->subNodes.Add(xmlOutputs);

		FOREACH(WString, path, paths)
		{
			auto xmlOutput = MakePtr<XmlElement>();
			xmlOutput->name.value = L"Output";
			xmlOutputs->subNodes.Add(xmlOutput);
			{
				auto attr = MakePtr<XmlAttribute>();
				attr->name.value = L"Path";
				attr->value.value = path;
				xmlOutput->attributes.Add(attr);
			}
		}
	}

	{
		FileStream fileStream(outputPath.GetFullPath(), FileStream::WriteOnly);
		if (!fileStream.IsAvailable())
		{
			PrintErrorMessage(L"error> Failed to write file: " + outputPath.GetFullPath());
			return;
		}

		BomEncoder encoder(BomEncoder::Utf8);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		XmlPrint(doc, writer);
	}
}

void GuiMain()
{
	Console::SetTitle(L"Vczh GacUI Resource Code Generator for C++");

	if (arguments->Count() > 0)
	{
		if (arguments->Get(0) == L"/P")
		{
			switch (arguments->Count())
			{
			case 2:
				CompileResource(true, arguments->Get(1), {});
				return;
			case 3:
				CompileResource(true, arguments->Get(1), { arguments->Get(2) });
				return;
			}
		}
		else if (arguments->Get(0) == L"/D")
		{
			switch (arguments->Count())
			{
			case 3:
				DumpResource(arguments->Get(1), arguments->Get(2));
				return;
			}
		}
		else
		{
			switch (arguments->Count())
			{
			case 1:
				CompileResource(false, arguments->Get(0), {});
				return;
			}
		}
	}

	PrintErrorMessage(L"Usage");
	PrintErrorMessage(L"    GacGen.exe <input-xml>");
	PrintErrorMessage(L"    GacGen.exe /P <input-xml> [<mapping-file>]");
	PrintErrorMessage(L"    GacGen.exe /D <input-xml> <output-xml>");
}
