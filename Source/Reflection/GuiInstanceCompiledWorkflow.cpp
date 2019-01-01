#include "GuiInstanceCompiledWorkflow.h"
#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace stream;
		using namespace workflow::runtime;
		using namespace controls;

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		bool GuiInstanceCompiledWorkflow::Initialize(bool initializeContext, workflow::runtime::WfAssemblyLoadErrors& loadErrors)
		{
			if (binaryToLoad)
			{
				assembly = WfAssembly::Deserialize(*binaryToLoad.Obj(), loadErrors);
				if (!assembly)
				{
					return false;
				}
				context = nullptr;
				binaryToLoad = nullptr;
			}

			if (initializeContext && !context)
			{
				context = new WfRuntimeGlobalContext(assembly);
				LoadFunction<void()>(context, L"<initialize>")();
			}
			return true;
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

			vint GetMaxPassIndex()override
			{
				return 1;
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
									FOREACH(WString, loadError, loadErrors.duplicatedTypes)
									{
										errors.Add({ {resource},L"Failed to add an existing type: " + loadError });
									}
									FOREACH(WString, loadError, loadErrors.unresolvedTypes)
									{
										errors.Add({ {resource},L"Unable to resolve type: " + loadError });
									}
									FOREACH(WString, loadError, loadErrors.unresolvedMembers)
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

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& stream)override
			{
				if (auto obj = content.Cast<GuiInstanceCompiledWorkflow>())
				{
					internal::ContextFreeWriter writer(stream);

					vint type = (vint)obj->type;
					writer << type;

					if (obj->type == GuiInstanceCompiledWorkflow::InstanceClass)
					{
						MemoryStream memoryStream;
						obj->assembly->Serialize(memoryStream);
						writer << (IStream&)memoryStream;
					}
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& stream, GuiResourceError::List& errors)override
			{
				internal::ContextFreeReader reader(stream);

				vint type;
				reader << type;
				
				auto obj = MakePtr<GuiInstanceCompiledWorkflow>();
				obj->type = (GuiInstanceCompiledWorkflow::AssemblyType)type;
				if (obj->type == GuiInstanceCompiledWorkflow::InstanceClass)
				{
					auto memoryStream = MakePtr<MemoryStream>();
					reader << (IStream&)*memoryStream.Obj();
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

			void Load()override
			{
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceCompiledWorkflowTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiRuntimeTypeResolversPlugin)
	}
}