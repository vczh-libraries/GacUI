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
	FileStream fileStream(errorFilePath.GetFullPath(), FileStream::WriteOnly);
	if (fileStream.IsAvailable())
	{
		BomEncoder encoder(BomEncoder::Utf16);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);

		List<WString> output;
		GuiResourceError::SortAndLog(errors, output);
		FOREACH(WString, line, output)
		{
			PrintErrorMessage(line);
			writer.WriteLine(line);
		}

		PrintInformationMessage(L"gacgen> Error information is saved at : " + errorFilePath.GetFullPath());
	}
	else
	{
		PrintErrorMessage(L"gacgen> Unable to write : " + errorFilePath.GetFullPath());
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

void CompileResource(bool partialMode, FilePath inputPath)
{
	PrintSuccessMessage(L"gacgen> Clearning logs ... : " + inputPath.GetFullPath());
#if defined VCZH_64
	FilePath logFolderPath = inputPath.GetFullPath() + L".log/x64";
#else
	FilePath logFolderPath = inputPath.GetFullPath() + L".log/x32";
#endif
	if (partialMode)
	{
		PrintInformationMessage(L"gacgen> Partial mode activated, all output files will be put under " + logFolderPath.GetFullPath());
	}
	FilePath scriptFilePath = logFolderPath / L"Workflow.txt";
	FilePath errorFilePath = logFolderPath / L"Errors.txt";
	FilePath workingDir = inputPath.GetFolder();

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
	if (auto precompiledFolder = PrecompileAndWriteErrors(resource, &callback, errors, errorFilePath))
	{
		if (errors.Count() == 0)
		{
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

					auto output = WriteCppCodesToFile(compiled, input, cppFolder);
					if (cppOutput->cppResource != L"")
					{
						WriteEmbeddedResource(resource, input, output, false, cppFolder / cppOutput->cppResource);
					}
					if (cppOutput->cppCompressed != L"")
					{
						WriteEmbeddedResource(resource, input, output, true, cppFolder / cppOutput->cppCompressed);
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
				}

				if (resOutput)
				{
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
						PrintSuccessMessage(L"Generating compressed resource files : " + filePath.GetFullPath());
						WriteBinaryResource(resource, false, false, {}, filePath);
					}
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
	}
}

void DumpResource(FilePath inputPath, FilePath outputPath)
{
	PrintSuccessMessage(L"gacgen> Dumping : " + inputPath.GetFullPath());
	List<GuiResourceError> errors;
	auto resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"error> Failed to load resource.");
		PrintErrors(errors);
		return;
	}

	auto config = CodegenConfig::LoadConfig(resource, errors);
	if (!config)
	{
		PrintErrorMessage(L"error> Failed to load config.");
		return;
	}
	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"error> Failed to load resource metadata.");
		PrintErrors(errors);
		return;
	}

	auto doc = MakePtr<XmlDocument>();
	auto xmlRoot = MakePtr<XmlElement>();
	xmlRoot->name.value = L"ResourceMetadata";
	doc->rootElement = xmlRoot;
	
	xmlRoot->subNodes.Add(resource->GetMetadata()->SaveToXml());

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

	switch (arguments->Count())
	{
	case 1:
		CompileResource(false, arguments->Get(0));
		break;
	case 2:
		if (arguments->Get(0) == L"/P")
		{
			CompileResource(true, arguments->Get(1));
			break;
		}
	case 3:
		if (arguments->Get(0) == L"/D")
		{
			DumpResource(arguments->Get(1), arguments->Get(2));
			break;
		}
	default:
		PrintErrorMessage(L"Usage: GacGen.exe <input-resource-xml-file>");
		PrintErrorMessage(L"Usage: GacGen.exe /P <input-resource-xml-file>");
		PrintErrorMessage(L"Usage: GacGen.exe /D <input-resource-xml-file> <output-xml>");
		return;
	}
}
