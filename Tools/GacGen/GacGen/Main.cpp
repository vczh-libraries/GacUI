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

void GuiMain()
{
	Console::SetTitle(L"Vczh GacUI Resource Code Generator for C++");

	bool partialMode = false;
	FilePath inputPath;
	switch(arguments->Count())
	{
	case 1:
		inputPath = arguments->Get(0);
		break;
	case 2:
		if (arguments->Get(0) == L"/P")
		{
			partialMode = true;
			inputPath = arguments->Get(1);
			break;
		}
	default:
		PrintErrorMessage(L"Usage: GacGen32.exe [/P] <input-resource-xml-file>");
		PrintErrorMessage(L"Usage: GacGen64.exe [/P] <input-resource-xml-file>");
		return;
	}

	PrintSuccessMessage(L"gacgen> Clearning logs ... : " + inputPath.GetFullPath());
	FilePath logFolderPath = inputPath.GetFullPath() + L".log";
	if (partialMode)
	{
#if defined VCZH_64
		logFolderPath = logFolderPath / L"x64";
#else
		logFolderPath = logFolderPath / L"x32";
#endif
		PrintInformationMessage(L"gacgen> Partial mode activated, all output files will be put under " + logFolderPath.GetFullPath());
	}
	FilePath scriptFilePath = logFolderPath / L"Workflow.txt";
	FilePath errorFilePath = logFolderPath / L"Errors.txt";
	FilePath workingDir = inputPath.GetFolder();
	{
		Folder logFolder(logFolderPath);
		if (logFolder.Exists())
		{
			List<File> files;
			logFolder.GetFiles(files);
			FOREACH(File, file, files)
			{
				if (!file.Delete())
				{
					PrintSuccessMessage(L"gacgen> Unable to delete file in the log folder : " + file.GetFilePath().GetFullPath());
				}
			}
		}
		else if (!logFolder.Create(true))
		{
			PrintSuccessMessage(L"gacgen> Unable to create log folder : " + logFolderPath.GetFullPath());
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

	Ptr<CodegenConfig> config;
	{
		config = CodegenConfig::LoadConfig(resource);
		if (!config)
		{
			PrintErrorMessage(L"error> Failed to load config.");
			return;
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
				if (config->cppOutput)
				{
					PrintSuccessMessage(L"gacgen> Generating C++ source code ...");
					auto input = MakePtr<WfCppInput>(config->cppOutput->name);
					input->multiFile = WfCppFileSwitch::Enabled;
					input->reflection = WfCppFileSwitch::Enabled;
					input->comment = L"GacGen.exe " + FilePath(inputPath).GetName();
					input->defaultFileName = config->cppOutput->name + L"PartialClasses";
					input->includeFileName = config->cppOutput->name;
					CopyFrom(input->normalIncludes, config->cppOutput->normalIncludes);
					CopyFrom(input->reflectionIncludes, config->cppOutput->reflectionIncludes);

					FilePath cppFolder = workingDir / config->cppOutput->sourceFolder;
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
					if (config->cppOutput->cppResource != L"")
					{
						WriteEmbeddedResource(resource, input, output, false, cppFolder / config->cppOutput->cppResource);
					}
					if (config->cppOutput->cppCompressed != L"")
					{
						WriteEmbeddedResource(resource, input, output, true, cppFolder / config->cppOutput->cppCompressed);
					}

					if (config->cppOutput->resource != L"")
					{
						PrintSuccessMessage(L"Generating binary resource file (no script): " + config->cppOutput->resource);
						WriteBinaryResource(resource, false, false, (partialMode ? logFolderPath / L"Resource.bin" : workingDir / config->cppOutput->resource));
					}
					if (config->cppOutput->compressed != L"")
					{
						PrintSuccessMessage(L"Generating compressed resource file (no script): " + config->cppOutput->compressed);
						WriteBinaryResource(resource, true, false, (partialMode ? logFolderPath / L"Compressed.bin" : workingDir / config->cppOutput->compressed));
					}
				}

				if (config->resOutput)
				{
					if (config->resOutput->resource != L"")
					{
						PrintSuccessMessage(L"Generating binary resource files : " + config->resOutput->resource);
						WriteBinaryResource(resource, false, true, (partialMode ? logFolderPath / L"ScriptedResource.bin" : workingDir / config->resOutput->resource));
					}
					if (config->resOutput->compressed != L"")
					{
						PrintSuccessMessage(L"Generating compressed resource files : " + config->resOutput->compressed);
						WriteBinaryResource(resource, true, true, (partialMode ? logFolderPath / L"ScriptedCompressed.bin" : workingDir / config->resOutput->compressed));
					}
				}

				if (partialMode)
				{
					List<WString> lines;

					if (config->cppOutput)
					{
						if (config->cppOutput->resource != L"")
						{
							lines.Add(L"copy " + (logFolderPath / L"Resource.bin").GetFullPath() + L" " + (workingDir / config->cppOutput->resource).GetFullPath());
						}
						if (config->cppOutput->compressed != L"")
						{
							lines.Add(L"copy " + (logFolderPath / L"Compressed.bin").GetFullPath() + L" " + (workingDir / config->cppOutput->compressed).GetFullPath());
						}
					}

					if (config->resOutput)
					{
						if (config->resOutput->resource != L"")
						{
							lines.Add(L"copy " + (logFolderPath / L"ScriptedResource.bin").GetFullPath() + L" " + (workingDir / config->resOutput->resource).GetFullPath());
						}
						if (config->resOutput->compressed != L"")
						{
							lines.Add(L"copy " + (logFolderPath / L"ScriptedCompressed.bin").GetFullPath() + L" " + (workingDir / config->resOutput->compressed).GetFullPath());
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