#include "GuiInstancePartialClass.h"
#include "TypeDescriptors/GuiReflectionBasic.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace workflow::runtime;
		using namespace controls;

/***********************************************************************
Workflow_RunPrecompiledScript
***********************************************************************/

		Ptr<workflow::runtime::WfRuntimeGlobalContext> Workflow_RunPrecompiledScript(Ptr<GuiResource> resource, Ptr<GuiResourceItem> resourceItem, description::Value rootInstance)
		{
			auto compiled = resourceItem->GetContent().Cast<GuiInstanceCompiledWorkflow>();
			auto globalContext = MakePtr<WfRuntimeGlobalContext>(compiled->assembly);

			LoadFunction<void()>(globalContext, L"<initialize>")();
			auto resolver = MakePtr<GuiResourcePathResolver>(resource, resource->GetWorkingDirectory());
			LoadFunction<void(Value, Ptr<GuiResourcePathResolver>)>(globalContext, L"<initialize-instance>")(rootInstance, resolver);

			return globalContext;
		}

/***********************************************************************
IGuiInstanceResourceManager
***********************************************************************/

		IGuiInstanceResourceManager* instanceResourceManager = 0;

		IGuiInstanceResourceManager* GetInstanceResourceManager()
		{
			return instanceResourceManager;
		}

		class GuiInstanceResourceManager : public Object, public IGuiInstanceResourceManager, public IGuiPlugin
		{
		protected:
			typedef Dictionary<WString, Ptr<GuiResource>>					ResourceMap;
			typedef Pair<Ptr<GuiResource>, Ptr<GuiResourceItem>>			ResourceItemPair;
			typedef Dictionary<WString, ResourceItemPair>					ResourceItemMap;

			ResourceMap								resources;
			ResourceItemMap							instanceCtors;

			void GetClassesInResource(Ptr<GuiResource> resource, Ptr<GuiResourceFolder> folder)
			{
				FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
				{
					if (auto compiled = item->GetContent().Cast<GuiInstanceCompiledWorkflow>())
					{
						if (compiled->type == GuiInstanceCompiledWorkflow::InstanceCtor)
						{
							if (!instanceCtors.Keys().Contains(compiled->classFullName))
							{
								instanceCtors.Add(compiled->classFullName, ResourceItemPair(resource, item));
							}
						}
					}
				}
				FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
				{
					GetClassesInResource(resource, subFolder);
				}
			}
		public:
			GuiInstanceResourceManager()
			{
			}

			void Load()override
			{
				instanceResourceManager = this;
			}

			void AfterLoad()override
			{
			}

			void Unload()override
			{
				instanceResourceManager = nullptr;
			}

			bool SetResource(const WString& name, Ptr<GuiResource> resource, GuiResourceUsage usage)override
			{
				vint index = resources.Keys().IndexOf(name);
				if (index != -1) return false;
				
				resource->Initialize(usage);
				resources.Add(name, resource);
				GetClassesInResource(resource, resource);
				return true;
			}

			Ptr<GuiResource> GetResource(const WString& name)override
			{
				vint index = resources.Keys().IndexOf(name);
				return index == -1 ? nullptr : resources.Values()[index];
			}

			Ptr<GuiInstanceConstructorResult> RunInstanceConstructor(const WString& classFullName, description::Value instance)override
			{
				vint index = instanceCtors.Keys().IndexOf(classFullName);
				if (index == -1) return nullptr;

				auto pair = instanceCtors.Values()[index];
				auto context = Workflow_RunPrecompiledScript(pair.key, pair.value, instance);
				auto result = MakePtr<GuiInstanceConstructorResult>();
				result->context = context;
				return result;
			}
		};
		GUI_REGISTER_PLUGIN(GuiInstanceResourceManager)
	}
}
