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
				return 2;
			}

			void Initialize(Ptr<GuiResourceItem> resource, GuiResourceInitializeContext& context)override
			{
				if (auto compiled = resource->GetContent().Cast<GuiInstanceCompiledWorkflow>())
				{
					switch (context.passIndex)
					{
					case 0:
						if (compiled->type == GuiInstanceCompiledWorkflow::ViewModel)
						{
							if (context.usage == GuiResourceUsage::DevelopmentTool)
							{
								compiled->Initialize(false);
							}
						}
						break;
					case 1:
						if (compiled->type == GuiInstanceCompiledWorkflow::Shared)
						{
							compiled->Initialize(true);
						}
						break;
					case 2:
						if (compiled->type == GuiInstanceCompiledWorkflow::InstanceCtor && compiled->type != GuiInstanceCompiledWorkflow::InstanceClass)
						{
							compiled->Initialize(false);
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

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				if (auto obj = resource.Cast<GuiInstanceCompiledWorkflow>())
				{
					internal::ContextFreeWriter writer(stream);

					vint type = (vint)obj->type;
					writer << type << obj->classFullName;

					MemoryStream memoryStream;
					obj->assembly->Serialize(memoryStream);
					writer << (IStream&)memoryStream;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)override
			{
				internal::ContextFreeReader reader(stream);
				auto obj = MakePtr<GuiInstanceCompiledWorkflow>();

				vint type;
				auto memoryStream = MakePtr<MemoryStream>();;
				reader << type << obj->classFullName << (IStream&)*memoryStream.Obj();

				obj->type = (GuiInstanceCompiledWorkflow::AssemblyType)type;
				obj->binaryToLoad = memoryStream;
				return obj;
			}
		};

/***********************************************************************
Plugin
***********************************************************************/

		class GuiRuntimeTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiRuntimeTypeResolversPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
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