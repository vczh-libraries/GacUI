#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiToolstripMenuInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripMenu, GuiMenuTemplate_StyleProvider, GuiMenuTemplate>
			class GuiToolstripMenuInstanceLoader : public BASE_TYPE
			{
			public:
				static Ptr<WfExpression> ArgumentFunction()
				{
					auto expr = MakePtr<WfLiteralExpression>();
					expr->value = WfLiteralValue::Null;
					return expr;
				}
			public:
				GuiToolstripMenuInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiToolstripMenu>()->GetTypeName(), L"CreateMenuStyle", ArgumentFunction)
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
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiToolstripMenu*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetToolstripItems().Add(control);
								return true;
							}
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripMenuBarInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripMenuBar, GuiControlTemplate_StyleProvider, GuiControlTemplate>
			class GuiToolstripMenuBarInstanceLoader : public BASE_TYPE
			{
			public:
				GuiToolstripMenuBarInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiToolstripMenuBar>()->GetTypeName(), L"CreateMenuBarStyle")
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
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiToolstripMenuBar*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetToolstripItems().Add(control);
								return true;
							}
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripToolBarInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripToolBar, GuiControlTemplate_StyleProvider, GuiControlTemplate>
			class GuiToolstripToolBarInstanceLoader : public BASE_TYPE
			{
			public:
				GuiToolstripToolBarInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiToolstripToolBar>()->GetTypeName(), L"CreateToolBarStyle")
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
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiToolstripToolBar*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetToolstripItems().Add(control);
								return true;
							}
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripButtonInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripButton, GuiToolstripButtonTemplate_StyleProvider, GuiToolstripButtonTemplate>
			class GuiToolstripButtonInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey					_SubMenu;

			public:
				GuiToolstripButtonInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiToolstripButton>()->GetTypeName(), L"CreateToolBarButtonStyle")
				{
					_SubMenu = GlobalStringKey::Get(L"SubMenu");
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_SubMenu);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _SubMenu)
					{
						return GuiInstancePropertyInfo::Set(description::GetTypeDescriptor<GuiToolstripMenu>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool GetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiToolstripButton*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _SubMenu)
						{
							if (!container->GetToolstripSubMenu())
							{
								container->CreateToolstripSubMenu();
							}
							propertyValue.propertyValue = Value::From(container->GetToolstripSubMenu());
							return true;
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadToolstripControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiToolstripMenuInstanceLoader);
				manager->SetLoader(new GuiToolstripMenuBarInstanceLoader);
				manager->SetLoader(new GuiToolstripToolBarInstanceLoader);
				manager->SetLoader(new GuiToolstripButtonInstanceLoader);
			}
		}
	}
}

#endif