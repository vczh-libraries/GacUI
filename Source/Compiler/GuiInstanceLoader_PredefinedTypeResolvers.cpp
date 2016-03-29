#include "GuiInstanceLoader.h"
#include "../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../Resources/GuiParserManager.h"
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

#define ERROR_CODE_PREFIX L"================================================================"

		void Workflow_GenerateAssembly(Ptr<GuiInstanceCompiledWorkflow> compiled, collections::List<WString>& errors)
		{
			auto manager = Workflow_GetSharedManager();
			manager->Clear(true, true);

			if (compiled->modules.Count() > 0)
			{
				FOREACH(Ptr<WfModule>, module, compiled->modules)
				{
					manager->AddModule(module);
				}
			}
			else
			{
				FOREACH(WString, code, compiled->codes)
				{
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
				errors.Add(ERROR_CODE_PREFIX L"Failed to compile workflow scripts");
				FOREACH(Ptr<ParsingError>, error, manager->errors)
				{
					if (compiled->modules.Count() > 0)
					{
						errors.Add(error->errorMessage);
					}
					else
					{
						errors.Add(
							L"Row: " + itow(error->codeRange.start.row + 1) +
							L", Column: " + itow(error->codeRange.start.column + 1) +
							L", Message: " + error->errorMessage);
					}
				}
			}
					
			compiled->codes.Clear();
			compiled->modules.Clear();
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
				return 5;
			}

			void Precompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, collections::List<WString>& errors)override
			{
				if (context.passIndex == 3)
				{
					if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
					{
						obj->ApplyStyles(context.resolver, errors);
						if (auto module = Workflow_PrecompileInstanceContext(obj, errors))
						{
							WString path = L"Workflow/InstanceCtor";
							auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
							if (!compiled)
							{
								compiled = new GuiInstanceCompiledWorkflow;
								compiled->type = GuiInstanceCompiledWorkflow::InstanceCtor;
								context.targetFolder->CreateValueByPath(path, L"Workflow", compiled);
							}

							compiled->containedClassNames.Add(obj->className);
							compiled->modules.Add(module);
						}
					}
				}
				else if (context.passIndex == 4 || context.passIndex == 5)
				{
					Ptr<GuiInstanceCompiledWorkflow> compiled;
					switch(context.passIndex)
					{
					case 4:
						{
							WString path = L"Workflow/InstanceCtor";
							compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
						}
						break;
					case 5:
						{
							WString path = L"Workflow/InstanceClass";
							compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
						}
						break;
					default:
						return;
					}

					if (compiled)
					{
						Workflow_GenerateAssembly(compiled, errors);
					}
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
				Ptr<GuiInstanceCompiledWorkflow> compiled;
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
							
							if (path != L"")
							{
								compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
								if (!compiled)
								{
									compiled = new GuiInstanceCompiledWorkflow;
									compiled->type = type;
									context.targetFolder->CreateValueByPath(path, L"Workflow", compiled);
								}
							}

							if (compiled)
							{
								compiled->codes.Add(obj->code);
							}
						}
					}
					return;
				case 1:
					{
						WString path = L"Workflow/ViewModel";
						compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
					}
					break;
				case 2:
					{
						WString path = L"Workflow/Shared";
						compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
					}
					break;
				default:
					return;
				}

				if (compiled)
				{
					Workflow_GenerateAssembly(compiled, errors);
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