#include "GuiInstanceLoader.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "GuiInstanceLoader_WorkflowCompiler.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;
		using namespace stream;

		using namespace controls;

#define ERROR_CODE_PREFIX L"================================================================"

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
				return 3;
			}

			void Precompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, collections::List<WString>& errors)override
			{
				if (context.passIndex == 3)
				{
					if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
					{
						obj->ApplyStyles(context.resolver, errors);
						if (auto assembly = Workflow_PrecompileInstanceContext(obj, errors))
						{
							auto compiled = MakePtr<GuiInstanceCompiledWorkflow>();
							compiled->type = GuiInstanceCompiledWorkflow::InstanceCtor;
							compiled->classFullName = obj->className;
							compiled->assembly = assembly;
							context.targetFolder->CreateValueByPath(L"Workflow/InstanceCtor/" + resource->GetResourcePath(), L"Workflow", compiled);
						}
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
				return 1;
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
					auto table = GetParserManager()->GetParsingTable(L"WORKFLOW");
					List<Ptr<ParsingError>> scriptErrors;
					compiled->assembly = Compile(table, compiled->codes, scriptErrors);
					compiled->codes.Clear();

					if (scriptErrors.Count() > 0)
					{
						errors.Add(ERROR_CODE_PREFIX L"Failed to parse the shared workflow script");
						FOREACH(Ptr<ParsingError>, error, scriptErrors)
						{
							errors.Add(
								L"Row: " + itow(error->codeRange.start.row + 1) +
								L", Column: " + itow(error->codeRange.start.column + 1) +
								L", Message: " + error->errorMessage);
						}
					}
					else
					{
						compiled->Initialize();
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
Compiled Workflow Type Resolver (Script)
***********************************************************************/

		class GuiResourceCompiledWorkflowTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_DirectLoadStream
		{
		public:
			WString GetType()override
			{
				return L"Workflow";
			}

			bool XmlSerializable()override
			{
				return false;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				if (auto obj = resource.Cast<GuiInstanceCompiledWorkflow>())
				{
					internal::Writer writer(stream);

					vint type = (vint)obj->type;
					writer << type << obj->classFullName;

					MemoryStream memoryStream;
					obj->assembly->Serialize(memoryStream);
					writer << (IStream&)memoryStream;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)override
			{
				internal::Reader reader(stream);
				auto obj = MakePtr<GuiInstanceCompiledWorkflow>();

				vint type;
				MemoryStream memoryStream;
				reader << type << obj->classFullName << (IStream&)memoryStream;

				obj->type = (GuiInstanceCompiledWorkflow::AssemblyType)type;
				obj->assembly = new WfAssembly(memoryStream);
				return obj;
			}
		};

/***********************************************************************
Shared Script Type Resolver
***********************************************************************/

		class GuiPredefinedTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiPredefinedTypeResolversPlugin()
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
				manager->SetTypeResolver(new GuiResourceCompiledWorkflowTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedTypeResolversPlugin)
	}
}