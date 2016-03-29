/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEPARTIALCLASS
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEPARTIALCLASS

#include "../Controls/GuiApplication.h"
#include "GuiInstanceCompiledWorkflow.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection;

/***********************************************************************
Resource
***********************************************************************/

		class GuiInstanceConstructorResult : public Object
		{
		public:
			reflection::description::Value				ctorInstance;
		};

		class IGuiInstanceResourceManager : public IDescriptable, public Description<IGuiInstanceResourceManager>
		{
		public:
			virtual bool								SetResource(const WString& name, Ptr<GuiResource> resource, GuiResourceUsage usage = GuiResourceUsage::Application) = 0;
			virtual Ptr<GuiResource>					GetResource(const WString& name) = 0;
			virtual Ptr<GuiInstanceConstructorResult>	RunInstanceConstructor(const WString& classFullName, description::Value rootInstance) = 0;
		};

		extern IGuiInstanceResourceManager*			GetInstanceResourceManager();

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
					auto value = description::Value::From(dynamic_cast<T*>(this));
					if (auto result = GetInstanceResourceManager()->RunInstanceConstructor(className, value))
					{
						ctorInstance = result->ctorInstance;
						return true;
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