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

		void GuiInstanceCompiledWorkflow::Initialize(bool initializeContext)
		{
			if (binaryToLoad)
			{
				assembly = new WfAssembly(*binaryToLoad.Obj());
				context = nullptr;
				binaryToLoad = nullptr;
			}

			if (initializeContext && !context)
			{
				context = new WfRuntimeGlobalContext(assembly);
				LoadFunction<void()>(context, L"<initialize>")();
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

			vint GetMaxPassIndex()override
			{
				return 1;
			}

			void Initialize(Ptr<GuiResourceItem> resource, GuiResourceInitializeContext& context)override
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
								compiled->Initialize(true);
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

			GUI_PLUGIN_NAME(GacUI_Compiled_RuntimeTypeResolvers)
			{
				GUI_PLUGIN_DEPEND(GacUI_ResourceResolver);
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