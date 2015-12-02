#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiTabInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiTab, GuiTabTemplate_StyleProvider, GuiTabTemplate>
			class GuiTabInstanceLoader : public BASE_TYPE
			{
			public:
				GuiTabInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiTab>()->GetTypeName(), L"CreateTabStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiTabPage>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiTab*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto tabPage = dynamic_cast<GuiTabPage*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->CreatePage(tabPage);
								return true;
							}
						}
					}
					return false;
				}
		};
#undef BASE_TYPE

/***********************************************************************
GuiTabPageInstanceLoader
***********************************************************************/

			class GuiTabPageInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiTabPageInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTabPage>()->GetTypeName());
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						auto info = GuiInstancePropertyInfo::Collection();
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiControl>());
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiGraphicsComposition>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiTabPage*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetContainerComposition()->AddChild(control->GetBoundsComposition());
								return true;
							}
							else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetContainerComposition()->AddChild(composition);
								return true;
							}
						}
					}
					return false;
				}
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadTabControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiTabInstanceLoader);
				manager->SetLoader(new GuiTabPageInstanceLoader);
			}
		}
	}
}

#endif