/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEPARTIALCLASS
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEPARTIALCLASS

#include "../Controls/GuiApplication.h"
#include "../Resources/GuiResourceManager.h"
#include "GuiInstanceCompiledWorkflow.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection;

/***********************************************************************
PartialClass
***********************************************************************/

		template<typename T>
		class GuiInstancePartialClass
		{
		private:
			WString											className;
			reflection::description::Value					ctorInstance;

		protected:
			bool InitializeFromResource()
			{
				if (ctorInstance.IsNull())
				{
					auto rootInstance = description::Value::From(dynamic_cast<T*>(this));
					auto resource = GetResourceManager()->GetResourceFromClassName(className);
					auto ctorFullName = className + L"<Ctor>";
					auto td = description::GetTypeDescriptor(ctorFullName);
					if (!td) return nullptr;

					auto ctor = td->GetConstructorGroup()->GetMethod(0);
					Array<Value> arguments;
					ctorInstance = ctor->Invoke(Value(), arguments);
					
					auto initialize = td->GetMethodGroupByName(L"<initialize-instance>", false)->GetMethod(0);
					{
						arguments.Resize(2);
						auto resolver = MakePtr<GuiResourcePathResolver>(resource, resource->GetWorkingDirectory());
						arguments[0] = rootInstance;
						arguments[1] = Value::From(resolver.Obj());
						initialize->Invoke(ctorInstance, arguments);
					}
				}
				return false;
			}

			template<typename TControl>
			void LoadInstanceReference(const WString& name, TControl*& reference)
			{
				reference = ctorInstance.GetProperty(name).GetRawPtr()->SafeAggregationCast<TControl>();
			}
		public:
			GuiInstancePartialClass(const WString& _className)
				:className(_className)
			{
			}

			virtual ~GuiInstancePartialClass()
			{
			}
		};

#define GUI_INSTANCE_REFERENCE(NAME) LoadInstanceReference(L ## #NAME, this->NAME)
	}
}

#endif