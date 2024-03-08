#include "GuiInstanceCompiledWorkflow.h"
#include "../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace stream;
		using namespace workflow::runtime;
		using namespace controls;
		using namespace reflection;
		using namespace reflection::description;

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		GuiInstanceCompiledWorkflow::GuiInstanceCompiledWorkflow()
		{
		}

		GuiInstanceCompiledWorkflow::~GuiInstanceCompiledWorkflow()
		{
			UnloadAssembly();
		}

		bool GuiInstanceCompiledWorkflow::Initialize(bool initializeContext, workflow::runtime::WfAssemblyLoadErrors& loadErrors)
		{
			if (binaryToLoad)
			{
				assembly = WfAssembly::Deserialize(*binaryToLoad.Obj(), loadErrors);
				if (!assembly)
				{
					return false;
				}
				binaryToLoad = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				context = nullptr;
#endif
			}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			if (initializeContext && !context)
			{
				context = Ptr(new WfRuntimeGlobalContext(assembly));
				LoadFunction<void()>(context, L"<initialize>")();
			}
#endif
			if (initializeContext)
			{
				if (assembly->typeImpl)
				{
					if (auto tm = GetGlobalTypeManager())
					{
						tm->AddTypeLoader(assembly->typeImpl);
					}
				}
			}
			return true;
		}

		void GuiInstanceCompiledWorkflow::UnloadAssembly()
		{
			UnloadTypes();
			assembly = nullptr;
		}

		void GuiInstanceCompiledWorkflow::UnloadTypes()
		{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			context = nullptr;
#endif
			if (assembly && assembly->typeImpl)
			{
				if (auto tm = GetGlobalTypeManager())
				{
					tm->RemoveTypeLoader(assembly->typeImpl);
				}
			}
		}

/***********************************************************************
Compiled Workflow Type Resolver (Workflow)
***********************************************************************/

		class GuiResourceCompiledWorkflowTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Initialize
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

			bool GetInitializePassSupport(vint passIndex)override
			{
				switch (passIndex)
				{
				case Workflow_Initialize:
					return true;
				default:
					return false;
				}
			}

			void Initialize(Ptr<GuiResourceItem> resource, GuiResourceInitializeContext& context, GuiResourceError::List& errors)override
			{
				if (auto compiled = resource->GetContent().Cast<GuiInstanceCompiledWorkflow>())
				{
					switch (context.passIndex)
					{
					case 0:
						if (compiled->type == GuiInstanceCompiledWorkflow::InstanceClass)
						{
							if (context.usage == GuiResourceUsage::InstanceClass)
							{
								WfAssemblyLoadErrors loadErrors;
								if (!compiled->Initialize(true, loadErrors))
								{
									for (auto loadError : loadErrors.duplicatedTypes)
									{
										errors.Add({ {resource},L"Failed to add an existing type: " + loadError });
									}
									for (auto loadError : loadErrors.unresolvedTypes)
									{
										errors.Add({ {resource},L"Unable to resolve type: " + loadError });
									}
									for (auto loadError : loadErrors.unresolvedMembers)
									{
										errors.Add({ {resource},L"Unable to resolve member: " + loadError });
									}
								}
							}
						}
						break;
					}
				}
			}

			IGuiResourceTypeResolver_Initialize* Initialize()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& resourceStream)override
			{
				if (auto obj = content.Cast<GuiInstanceCompiledWorkflow>())
				{
					internal::ContextFreeWriter writer(resourceStream);

					vint type = (vint)obj->type;
					writer << type;

					if (obj->type == GuiInstanceCompiledWorkflow::InstanceClass)
					{
						stream::MemoryStream memoryStream;
						obj->assembly->Serialize(memoryStream);
						writer << (stream::IStream&)memoryStream;
					}
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& resourceStream, GuiResourceError::List& errors)override
			{
				internal::ContextFreeReader reader(resourceStream);

				vint type;
				reader << type;

				auto obj = Ptr(new GuiInstanceCompiledWorkflow);
				obj->type = (GuiInstanceCompiledWorkflow::AssemblyType)type;
				if (obj->type == GuiInstanceCompiledWorkflow::InstanceClass)
				{
					auto memoryStream = Ptr(new stream::MemoryStream);
					reader << (stream::IStream&)*memoryStream.Obj();
					obj->binaryToLoad = memoryStream;
				}
				return obj;
			}
		};

/***********************************************************************
Plugin
***********************************************************************/

		class GuiRuntimeTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:

			GUI_PLUGIN_NAME(GacUI_Compiler_WorkflowTypeResolvers)
			{
				GUI_PLUGIN_DEPEND(GacUI_Res_ResourceResolver);
			}

			void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
				if (controllerUnrelatedPlugins)
				{
					IGuiResourceResolverManager* manager = GetResourceResolverManager();
					manager->SetTypeResolver(Ptr(new GuiResourceCompiledWorkflowTypeResolver));
				}
			}

			void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiRuntimeTypeResolversPlugin)
	}
}