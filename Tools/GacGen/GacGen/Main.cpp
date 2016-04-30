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

typedef Dictionary<WString, Ptr<GuiInstanceCompiledWorkflow>>	CompiledScriptMap;

void CollectWorkflowScriptsInFolder(Ptr<GuiResourceFolder> folder, CompiledScriptMap& scripts)
{
	FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
	{
		auto compiled = item->GetContent().Cast<GuiInstanceCompiledWorkflow>();
		if (compiled)
		{
			scripts.Add(item->GetResourcePath(), compiled);
		}
	}
	FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
	{
		CollectWorkflowScriptsInFolder(subFolder, scripts);
	}
}

void CollectWorkflowScripts(Ptr<GuiResource> resource, CompiledScriptMap& scripts)
{
	auto precompiledFolder = resource->GetFolder(L"Precompiled");
	CollectWorkflowScriptsInFolder(precompiledFolder, scripts);
}

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

	Dictionary<WString, Ptr<WfClassDeclaration>> typeSchemas;
	List<WString> typeSchemaOrder;
	Dictionary<WString, Ptr<Instance>> instances;
	Ptr<WfLexicalScopeManager> schemaManager;
	{
		Regex regexClassName(L"((<namespace>[^:]+)::)*(<type>[^:]+)");
		Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
		SearchAllInstances(regexClassName, resolver, resource, instances);

		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			SearchAllEventHandlers(config, instances, instance, instance->eventHandlers);
		}

		List<WString> schemaPaths;
		List<Ptr<GuiInstanceSharedScript>> schemas;
		Dictionary<vint, WString> schemaPathMap;
		SearchAllSchemas(regexClassName, resource, schemaPaths, schemas);

		if (schemas.Count() > 0)
		{
			if (!config->workflowTable)
			{
				config->workflowTable = WfLoadTable();
			}
			schemaManager = new WfLexicalScopeManager(config->workflowTable);

			FOREACH_INDEXER(Ptr<GuiInstanceSharedScript>, schema, schemaIndex, schemas)
			{
				vint codeIndex = schemaManager->AddModule(schema->code);
				schemaPathMap.Add(codeIndex, schemaPaths[schemaIndex]);
			}

			if (schemaManager->errors.Count() == 0)
			{
				schemaManager->Rebuild(false);
				if (schemaManager->errors.Count() == 0)
				{
					List<Ptr<WfDeclaration>> decls;
					CopyFrom(
						decls,
						From(schemaManager->GetModules())
							.SelectMany([](Ptr<WfModule> module)
							{
								return LazyList<Ptr<WfDeclaration>>(module->declarations);
							})
						);
					for (vint i = 0; i < decls.Count(); i++)
					{
						auto decl = decls[i];
						if (auto nsDecl = decl.Cast<WfNamespaceDeclaration>())
						{
							CopyFrom(decls, nsDecl->declarations, true);
						}
						else if (auto classDecl = decl.Cast<WfClassDeclaration>())
						{
							if (classDecl->kind == WfClassKind::Interface)
							{
								auto td = schemaManager->declarationTypes[classDecl.Obj()];
								auto typeName = td->GetTypeName();
								typeSchemas.Add(typeName, classDecl);
								typeSchemaOrder.Add(typeName);
							}
						}
					}
				}
			}
		}
	}

	{
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
						PrintInformationMessage(L"Pass: (1/11) Collect workflow scripts");
						break;
					case IGuiResourceTypeResolver_Precompile::Workflow_CompileViewModel:
						PrintInformationMessage(L"Pass: (2/11) Compile view model scripts");
						break;
					case IGuiResourceTypeResolver_Precompile::Workflow_CompileShared:
						PrintInformationMessage(L"Pass: (3/11) Compile shared scripts");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_CollectInstanceTypes:
						PrintInformationMessage(L"Pass: (4/11) Collect instances");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_ValidateDependency:
						PrintInformationMessage(L"Pass: (5/11) Validate instances");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_GenerateTemporaryClass:
						PrintInformationMessage(L"Pass: (6/11) Generate instance type stubs");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_CompileTemporaryClass:
						PrintInformationMessage(L"Pass: (7/11) Compile instance type stubs");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_GenerateInstanceCtor:
						PrintInformationMessage(L"Pass: (8/11) Generate instance constructor types");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_CompileInstanceCtor:
						PrintInformationMessage(L"Pass: (9/11) Compile instance constructor types");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_GenerateInstanceClass:
						PrintInformationMessage(L"Pass: (10/11) Generate instance types");
						break;
					case IGuiResourceTypeResolver_Precompile::Instance_CompileInstanceClass:
						PrintInformationMessage(L"Pass: (11/11) Compile instance types");
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

		PrintSuccessMessage(L"gacgen> Compiling...");
		List<WString> errors;
		Callback callback;
		resource->Precompile(&callback, errors);
		if (errors.Count() > 0)
		{
			SaveErrors(errorFilePath, errors);
			return;
		}
	}

	GetResourceManager()->SetResource(L"GACGEN", resource, GuiResourceUsage::DevelopmentTool);

	if (config->cppOutput)
	{
		filesystem::Folder(resource->GetWorkingDirectory() + config->cppOutput->output).Create(true);
		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			if (instance->context->codeBehind)
			{
				WriteControlClassHeaderFile(config, instance);
				WriteControlClassCppFile(config, instance);
			}
		}
		WritePartialClassHeaderFile(config, schemaManager, typeSchemas, typeSchemaOrder, instances);
		WritePartialClassCppFile(config, schemaManager, typeSchemas, typeSchemaOrder, instances);
		WriteGlobalHeaderFile(config, instances);
	}

	if (config->resOutput)
	{
		filesystem::Folder(resource->GetWorkingDirectory() + config->resOutput->output).Create(true);
		if (config->resOutput->precompiledBinary != L"")
		{
			WString fileName = config->resOutput->output + config->resOutput->precompiledBinary;
			OPEN_BINARY_FILE(L"Precompiled Binary Resource");
			resource->SavePrecompiledBinary(fileStream);
		}
		if (config->resOutput->precompiledCompressed != L"")
		{
			WString fileName = config->resOutput->output + config->resOutput->precompiledCompressed;
			OPEN_BINARY_FILE(L"Precompiled Compressed Binary Resource");
			LzwEncoder encoder;
			EncoderStream encoderStream(fileStream, encoder);
			resource->SavePrecompiledBinary(encoderStream);
		}
	}

	{
		PrintSuccessMessage(L"gacgen> Dumping workflow scripts ... : " + inputPath);
		CompiledScriptMap scripts;
		CollectWorkflowScripts(resource, scripts);

		Group<WString, WString> paths;
		FOREACH(WString, path, scripts.Keys())
		{
			auto pair = INVLOC.FindLast(path, L"/", Locale::None);
			auto key = path.Right(path.Length() - pair.key - pair.value);
			paths.Add(key, path);
		}

		FOREACH_INDEXER(WString, key, keyIndex, paths.Keys())
		{
			auto& pathList = paths.GetByIndex(keyIndex);
			FOREACH_INDEXER(WString, path, pathIndex, pathList)
			{
				auto compiled = scripts[path];
				auto& codes = compiled->assembly->insAfterCodegen->moduleCodes;

				WString text = path + L"\r\n";
				FOREACH_INDEXER(WString, code, codeIndex, codes)
				{
					text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
					text += code + L"\r\n";
				}

				auto fileName = logFolderPath / (pathList.Count() > 1 ? key + L"[" + itow(pathIndex) + L"].txt" : key + L".txt");
				File(fileName).WriteAllText(text);
			}
		}
	}
}