#include "GuiInstanceLoader.h"
#include "../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../Resources/GuiParserManager.h"
#include "../Resources/GuiResourceManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;
		using namespace stream;

		using namespace controls;

		void Workflow_GenerateAssembly(Ptr<GuiInstanceCompiledWorkflow> compiled, const WString& path, collections::List<WString>& errors)
		{
			if (!compiled->assembly)
			{
				List<WString> codes;
				auto manager = Workflow_GetSharedManager();
				manager->Clear(false, true);

				if (compiled->modules.Count() > 0)
				{
					FOREACH_INDEXER(Ptr<WfModule>, module, index, compiled->modules)
					{
						{
							MemoryStream stream;
							{
								StreamWriter writer(stream);
								auto recorder = MakePtr<ParsingUpdateLocationRecorder>();
								ParsingWriter parsingWriter(writer, recorder, index);
								WfPrint(module, L"", parsingWriter);
							}
							stream.SeekFromBegin(0);
							{
								StreamReader reader(stream);

								vint row = 0;
								WString code;
								while (!reader.IsEnd())
								{
									auto rowHeader = itow(++row);
									while (rowHeader.Length() < 6)
									{
										rowHeader = L" " + rowHeader;
									}
									code += rowHeader + L" : " + reader.ReadLine() + L"\r\n";
								}
								codes.Add(code);
							}
						}
						manager->AddModule(module);
					}
				}
				else
				{
					FOREACH(WString, code, compiled->codes)
					{
						codes.Add(code);
						manager->AddModule(code);
					}
				}

				if (manager->errors.Count() == 0)
				{
					manager->Rebuild(true);
				}

				if (manager->errors.Count() == 0)
				{
					compiled->assembly = GenerateAssembly(manager);
					compiled->Initialize(true);
				}
				else
				{
					errors.Add(L"Failed to compile workflow scripts in: " + path);

					using ErrorGroup = Pair<int, LazyList<Ptr<ParsingError>>>;
					List<ErrorGroup> errorGroups;
					CopyFrom(
						errorGroups,
						From(manager->errors)
							.GroupBy([](Ptr<ParsingError> error){ return error->codeRange.codeIndex; })
							.OrderBy([](ErrorGroup g1, ErrorGroup g2){ return g1.key - g2.key; })
						);

					FOREACH(ErrorGroup, errorGroup, errorGroups)
					{
						if (errorGroup.key == -1)
						{
							FOREACH(Ptr<ParsingError>, error, errorGroup.value)
							{
								errors.Add(L"    " + error->errorMessage);
							}
						}
						else
						{
							FOREACH(Ptr<ParsingError>, error, errorGroup.value)
							{
								errors.Add(
									L"    "
									L"Row: " + itow(error->codeRange.start.row + 1) +
									L", Column: " + itow(error->codeRange.start.column + 1) +
									L", Message: " + error->errorMessage);
							}
							errors.Add(L"Workflow script for reference:");
							errors.Add(codes[errorGroup.key]);
						}
					}
					errors.Add(L"<END>");
				}
					
				compiled->codes.Clear();
				compiled->modules.Clear();
				manager->Clear(false, true);
			}
		}

/***********************************************************************
Instance Type Resolver (Instance)
***********************************************************************/

		class GuiResourceInstanceTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Instance";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 8;
			}

			void AddModule(GuiResourcePrecompileContext& context, const WString& path, Ptr<WfModule> module, GuiInstanceCompiledWorkflow::AssemblyType assemblyType)
			{
				auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
				if (!compiled)
				{
					compiled = new GuiInstanceCompiledWorkflow;
					compiled->type = assemblyType;
					context.targetFolder->CreateValueByPath(path, L"Workflow", compiled);
				}

				compiled->modules.Add(module);
			}

			void Precompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, collections::List<WString>& errors)override
			{
				switch (context.passIndex)
				{
				case 3:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							obj->ApplyStyles(context.resolver, errors);
							if (auto module = Workflow_GenerateInstanceClass(obj, *(types::ResolvingResult*)nullptr, errors, true))
							{
								AddModule(context, L"Workflow/TemporaryClass", module, GuiInstanceCompiledWorkflow::TemporaryClass);
							}

							auto record = context.targetFolder->GetValueByPath(L"ClassNameRecord").Cast<GuiResourceClassNameRecord>();
							if (!record)
							{
								record = MakePtr<GuiResourceClassNameRecord>();
								context.targetFolder->CreateValueByPath(L"ClassNameRecord", L"ClassNameRecord", record);
							}
							record->classNames.Add(obj->className);
						}
					}
					break;
				case 5:
					{
						if (auto compiled = context.targetFolder->GetValueByPath(L"Workflow/TemporaryClass").Cast<GuiInstanceCompiledWorkflow>())
						{
							if (!compiled->assembly)
							{
								break;
							}
						}
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							auto resolvingResult = MakePtr<types::ResolvingResult>();
							if (auto module = Workflow_PrecompileInstanceContext(obj, *resolvingResult.Obj(), errors))
							{
								context.additionalProperties.Add(obj, resolvingResult);
								AddModule(context, L"Workflow/InstanceCtor", module, GuiInstanceCompiledWorkflow::InstanceCtor);
								AddModule(context, L"Workflow/InstanceClass", module, GuiInstanceCompiledWorkflow::InstanceClass);
							}
						}
					}
					break;
				case 7:
					{
						if (auto compiled = context.targetFolder->GetValueByPath(L"Workflow/InstanceCtor").Cast<GuiInstanceCompiledWorkflow>())
						{
							if (!compiled->assembly)
							{
								break;
							}
							compiled->context = nullptr;
						}
						if (auto compiled = context.targetFolder->GetValueByPath(L"Workflow/TemporaryClass").Cast<GuiInstanceCompiledWorkflow>())
						{
							if (!compiled->assembly)
							{
								break;
							}
							compiled->context = nullptr;
						}
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							auto resolvingResult = context.additionalProperties[obj.Obj()].Cast<types::ResolvingResult>();
							if (auto module = Workflow_GenerateInstanceClass(obj, *resolvingResult.Obj(), errors, false))
							{
								AddModule(context, L"Workflow/InstanceClass", module, GuiInstanceCompiledWorkflow::InstanceClass);
							}
						}
					}
					break;
				case 4:
				case 6:
				case 8:
					{
						WString path;
						if (context.passIndex == 4)
						{
							path = L"Workflow/TemporaryClass";
						}
						else if (context.passIndex == 6)
						{
							path = L"Workflow/InstanceCtor";
						}
						else if (context.passIndex == 8)
						{
							path = L"Workflow/InstanceClass";
						}
						else
						{
							return;
						}

						if (auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>())
						{
							Workflow_GenerateAssembly(compiled, path, errors);
						}
					}
					break;
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource)override
			{
				if (auto obj = resource.Cast<GuiInstanceContext>())
				{
					return obj->SaveToXml();
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					Ptr<GuiInstanceContext> context = GuiInstanceContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Instance Style Type Resolver (InstanceStyle)
***********************************************************************/

		class GuiResourceInstanceStyleResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"InstanceStyle";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource)override
			{
				if (auto obj = resource.Cast<GuiInstanceStyleContext>())
				{
					return obj->SaveToXml();
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto context = GuiInstanceStyleContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Shared Script Type Resolver (Script)
***********************************************************************/

		class GuiResourceSharedScriptTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Script";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 2;
			}

			void Precompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, collections::List<WString>& errors)override
			{
				switch (context.passIndex)
				{
					case 0:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceSharedScript>())
						{
							WString path;
							GuiInstanceCompiledWorkflow::AssemblyType type = GuiInstanceCompiledWorkflow::Shared;
							if (obj->language == L"Workflow-ViewModel")
							{
								path = L"Workflow/ViewModel";
								type = GuiInstanceCompiledWorkflow::ViewModel;
							}
							else if (obj->language == L"Workflow")
							{
								path = L"Workflow/Shared";
							}
						
							auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
							if (!compiled)
							{
								compiled = new GuiInstanceCompiledWorkflow;
								compiled->type = type;
								context.targetFolder->CreateValueByPath(path, L"Workflow", compiled);
							}

							compiled->codes.Add(obj->code);
						}
					}
					break;
				case 1:
				case 2:
					{
						WString path;
						if (context.passIndex == 1)
						{
							path = L"Workflow/ViewModel";
						}
						else if (context.passIndex == 2)
						{
							path = L"Workflow/Shared";
						}
						else
						{
							return;
						}

						if (auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>())
						{
							Workflow_GenerateAssembly(compiled, path, errors);
						}
					}
					break;
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource)override
			{
				if (auto obj = resource.Cast<GuiInstanceSharedScript>())
				{
					return obj->SaveToXml();
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto schema = GuiInstanceSharedScript::LoadFromXml(xml, errors);
					return schema;
				}
				return 0;
			}
		};

/***********************************************************************
Plugin
***********************************************************************/

		class GuiCompilerTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiCompilerTypeResolversPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
			{
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceInstanceTypeResolver);
				manager->SetTypeResolver(new GuiResourceInstanceStyleResolver);
				manager->SetTypeResolver(new GuiResourceSharedScriptTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiCompilerTypeResolversPlugin)
	}
}