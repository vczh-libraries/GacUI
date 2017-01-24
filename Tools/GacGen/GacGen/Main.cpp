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

void SaveErrors(FilePath errorFilePath, List<WString>& errors)
{
	FileStream fileStream(errorFilePath.GetFullPath(), FileStream::WriteOnly);
	if (fileStream.IsAvailable())
	{
		BomEncoder encoder(BomEncoder::Utf16);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);

		FOREACH(WString, error, errors)
		{
			PrintErrorMessage(error);
			writer.WriteLine(error);
		}

		PrintInformationMessage(L"gacgen> Error information is saved at : " + errorFilePath.GetFullPath());
	}
	else
	{
		PrintErrorMessage(L"gacgen> Unable to write : " + errorFilePath.GetFullPath());
	}
}
class Callback : public Object, public IGuiResourcePrecompileCallback
{
public:
	vint lastPass = -1;

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
	Console::WriteLine(L"Vczh GacUI Resource Code Generator for C++");

	if (arguments->Count() != 1)
	{
		PrintErrorMessage(L"GacGen.exe only accept 1 input file.");
		return;
	}
	WString inputPath = arguments->Get(0);
	
	PrintSuccessMessage(L"gacgen> Clearning logs ... : " + inputPath);
	FilePath logFolderPath = inputPath + L".log";
	FilePath scriptFilePath = logFolderPath / L"Workflow.txt";
	FilePath errorFilePath = logFolderPath / L"Errors.txt";
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
		PrintSuccessMessage(L"gacgen> Making : " + inputPath);
		List<WString> errors;
		resource = GuiResource::LoadFromXml(arguments->Get(0), errors);
		if (!resource)
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
	List<WString> errors;
	Callback callback;
	resource->Precompile(&callback, errors);
	if (errors.Count() > 0)
	{
		SaveErrors(errorFilePath, errors);
		return;
	}

	if(config->cppOutput)
	{
		auto item = resource->GetValueByPath(L"Precompiled/Workflow/InstanceClass");
		auto compiled = item.Cast<GuiInstanceCompiledWorkflow>();
		{
			WString text;
			auto& codes = compiled->assembly->insAfterCodegen->moduleCodes;
			FOREACH_INDEXER(WString, code, codeIndex, compiled->assembly->insAfterCodegen->moduleCodes)
			{
				text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
				text += code + L"\r\n";
			}
			File(scriptFilePath).WriteAllText(text);
		}
		{
			auto input = MakePtr<WfCppInput>(config->cppOutput->name);
			input->comment = L"Source: Host.sln";
			input->defaultFileName = config->cppOutput->name + L"PartialClasses";
			input->includeFileName = config->cppOutput->name;
			if (config->cppOutput->normalInclude != L"")
			{
				input->normalIncludes.Add(config->cppOutput->normalInclude);
			}
			if (config->cppOutput->reflectionInclude != L"")
			{
				input->reflectionIncludes.Add(config->cppOutput->reflectionInclude);
			}

			FilePath sourceFolder = resource->GetWorkingDirectory() + config->cppOutput->sourceFolder;
			Folder(sourceFolder).Create(true);
			auto output = GenerateCppFiles(input, compiled->metadata.Obj());
			FOREACH_INDEXER(WString, fileName, index, output->cppFiles.Keys())
			{
				WString code = output->cppFiles.Values()[index];
				File file(sourceFolder / fileName);

				if (file.Exists())
				{
					code = MergeCppFileContent(file.ReadAllText(), code);
				}
				file.WriteAllText(code, false, BomEncoder::Utf8);
			}
		}
	}

#define OPEN_BINARY_FILE(NAME, FILENAME) \
			WString fileName = config->resource->GetWorkingDirectory() + FILENAME; \
			Folder(FilePath(fileName).GetFolder()).Create(true); \
			FileStream fileStream(fileName, FileStream::WriteOnly); \
			if (!fileStream.IsAvailable()) \
			{ \
				PrintErrorMessage(L"error> Failed to generate " + fileName); \
				return; \
			} \
			PrintSuccessMessage(L"gacgen> Generating " + fileName);

	if (config->resOutput)
	{
		if (config->resOutput->resource != L"")
		{
			OPEN_BINARY_FILE(L"Precompiled Binary Resource", config->resOutput->resource);
			resource->SavePrecompiledBinary(fileStream);
		}
		if (config->resOutput->compressed != L"")
		{
			OPEN_BINARY_FILE(L"Precompiled Compressed Binary Resource", config->resOutput->compressed);
			LzwEncoder encoder;
			EncoderStream encoderStream(fileStream, encoder);
			resource->SavePrecompiledBinary(encoderStream);
		}
	}

	if (config->cppOutput)
	{
		resource->RemoveFolder(L"Precompiled");
		if (config->resOutput->resource != L"")
		{
			OPEN_BINARY_FILE(L"Precompiled Binary Resource", config->resOutput->resource);
			resource->SavePrecompiledBinary(fileStream);
		}
		if (config->resOutput->compressed != L"")
		{
			OPEN_BINARY_FILE(L"Precompiled Compressed Binary Resource", config->resOutput->compressed);
			LzwEncoder encoder;
			EncoderStream encoderStream(fileStream, encoder);
			resource->SavePrecompiledBinary(encoderStream);
		}
	}
}