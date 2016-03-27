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
			Ptr<workflow::runtime::WfRuntimeGlobalContext>	context;
		};

		class IGuiInstanceResourceManager : public IDescriptable, public Description<IGuiInstanceResourceManager>
		{
		public:
			virtual bool								SetResource(const WString& name, Ptr<GuiResource> resource, GuiResourceUsage usage = GuiResourceUsage::Application) = 0;
			virtual Ptr<GuiResource>					GetResource(const WString& name) = 0;
			virtual Ptr<GuiInstanceConstructorResult>	RunInstanceConstructor(const WString& classFullName, description::Value instance) = 0;
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
			Ptr<workflow::runtime::WfRuntimeGlobalContext>	context;

		protected:
			bool InitializeFromResource()
			{
				if (!context)
				{
					auto value = description::Value::From(dynamic_cast<T*>(this));
					if (auto result = GetInstanceResourceManager()->RunInstanceConstructor(className, value))
					{
						context = result->context;
						return true;
					}
				}
				return false;
			}

			template<typename TControl>
			void LoadInstanceReference(const WString& name, TControl*& reference)
			{
				reference = 0;
				vint index = context->assembly->variableNames.IndexOf(name);
				CHECK_ERROR(index != -1, L"GuiInstancePartialClass<T>::LoadInstanceReference<TControl>(const WString&, TControl*&)#Failed to find instance reference.");

				auto value = context->globalVariables->variables[index];
				auto td = description::GetTypeDescriptor<TControl>();
				if (!value.GetTypeDescriptor() || !value.GetTypeDescriptor()->CanConvertTo(td))
				{
					CHECK_ERROR(index != -1, L"GuiInstancePartialClass<T>::LoadInstanceReference<TControl>(const WString&, TControl*&)#Wrong instance reference type.");
					return;
				}

				reference = description::UnboxValue<TControl*>(value);
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