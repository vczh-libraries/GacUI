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

			ResourceMap								resources;
			ResourceMap								instanceResources;

			void GetClassesInResource(Ptr<GuiResource> resource, Ptr<GuiResourceFolder> folder)
			{
				FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
				{
					if (auto compiled = item->GetContent().Cast<GuiInstanceCompiledWorkflow>())
					{
						if (compiled->type == GuiInstanceCompiledWorkflow::InstanceCtor)
						{
							FOREACH(WString, name, compiled->containedClassNames)
							{
								instanceResources.Add(name, resource);
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

			Ptr<GuiInstanceConstructorResult> RunInstanceConstructor(const WString& classFullName, description::Value rootInstance)override
			{
				vint index = instanceResources.Keys().IndexOf(classFullName);
				if (index == -1) return nullptr;

				auto resource = instanceResources.Values()[index];
				auto ctorFullName = classFullName + L"<Ctor>";
				auto td = description::GetTypeDescriptor(ctorFullName);
				if (!td) return nullptr;

				auto ctor = td->GetConstructorGroup()->GetMethod(0);
				Array<Value> arguments;
				auto ctorInstance = ctor->Invoke(Value(), arguments);
					
				auto initialize = td->GetMethodGroupByName(L"<initialize-instance>", false)->GetMethod(0);
				{
					arguments.Resize(2);
					auto resolver = MakePtr<GuiResourcePathResolver>(resource, resource->GetWorkingDirectory());
					arguments[0] = rootInstance;
					arguments[1] = Value::From(resolver.Obj());
					initialize->Invoke(ctorInstance, arguments);
				}

				auto result = MakePtr<GuiInstanceConstructorResult>();
				result->ctorInstance = ctorInstance;
				return result;
			}
		};
		GUI_REGISTER_PLUGIN(GuiInstanceResourceManager)
	}
}
