#include "GuiInstanceLoader.h"
#include "GuiInstanceHelperTypes.h"
#include "TypeDescriptors/GuiReflectionControls.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace controls;
		using namespace compositions;
		using namespace theme;
		using namespace helper_types;

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
GuiVrtualTypeInstanceLoader
***********************************************************************/

		class GuiTemplateControlInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey								typeName;
			Func<Value()>								defaultConstructor;
			Func<Value(Ptr<GuiTemplate::IFactory>)>		templateConstructor;
		public:
			GuiTemplateControlInstanceLoader(const WString& _typeName, const Func<Value()>& _defaultConstructor, const Func<Value(Ptr<GuiTemplate::IFactory>)>& _templateConstructor)
				:typeName(GlobalStringKey::Get(_typeName))
				, defaultConstructor(_defaultConstructor)
				, templateConstructor(_templateConstructor)
			{
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeName == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if(typeName==typeInfo.typeName)
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return defaultConstructor();
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return templateConstructor(factory);
					}
				}
				return Value();
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return 0;
			}
		};

/***********************************************************************
GuiControlInstanceLoader
***********************************************************************/

		class GuiControlInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiControlInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiControl>()->GetTypeName());
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
					if (propertyInfo.typeInfo.typeDescriptor->CanConvertTo(description::GetTypeDescriptor<GuiInstanceRootObject>()))
					{
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiComponent>());
					}
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiInstanceRootObject*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						if (auto component = dynamic_cast<GuiComponent*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->AddComponent(component);
							return true;
						}
						else if (auto controlHost = dynamic_cast<GuiControlHost*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->AddComponent(new GuiObjectComponent<GuiControlHost>(controlHost));
							return true;
						}
					}
				}
				if (auto container = dynamic_cast<GuiControl*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->AddChild(control);
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
GuiTabInstanceLoader
***********************************************************************/

		class GuiTabInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiTabInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTab>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return GetTypeName() == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if(GetTypeName() == typeInfo.typeName)
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return Value::From(g::NewTab());
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return Value::From(new GuiTab(new GuiTabTemplate_StyleProvider(factory)));
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::Empty);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiTabPage>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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
GuiToolstripMenuInstanceLoader
***********************************************************************/

		class GuiToolstripMenuInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiToolstripMenuInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiToolstripMenu>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return GetTypeName() == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if(GetTypeName() == typeInfo.typeName)
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return Value::From(g::NewMenu(0));
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return Value::From(new GuiToolstripMenu(new GuiMenuTemplate_StyleProvider(factory), 0));
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::Empty);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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

/***********************************************************************
GuiToolstripMenuBarInstanceLoader
***********************************************************************/

		class GuiToolstripMenuBarInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiToolstripMenuBarInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiToolstripMenuBar>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return GetTypeName() == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if(GetTypeName() == typeInfo.typeName)
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return Value::From(new GuiToolstripMenuBar(GetCurrentTheme()->CreateMenuBarStyle()));
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return Value::From(new GuiToolstripMenuBar(new GuiControlTemplate_StyleProvider(factory)));
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::Empty);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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

/***********************************************************************
GuiToolstripToolBarInstanceLoader
***********************************************************************/

		class GuiToolstripToolBarInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiToolstripToolBarInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiToolstripToolBar>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return GetTypeName() == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if(GetTypeName() == typeInfo.typeName)
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return Value::From(new GuiToolstripToolBar(GetCurrentTheme()->CreateToolBarStyle()));
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return Value::From(new GuiToolstripToolBar(new GuiControlTemplate_StyleProvider(factory)));
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::Empty);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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

/***********************************************************************
GuiToolstripButtonInstanceLoader
***********************************************************************/

		class GuiToolstripButtonInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;
			GlobalStringKey					_SubMenu;

		public:
			GuiToolstripButtonInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiToolstripButton>()->GetTypeName());
				_SubMenu = GlobalStringKey::Get(L"SubMenu");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						return Value::From(g::NewToolBarButton());
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						return Value::From(new GuiToolstripButton(new GuiToolstripButtonTemplate_StyleProvider(factory)));
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_SubMenu);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ControlTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _SubMenu)
				{
					return GuiInstancePropertyInfo::Set(description::GetTypeDescriptor<GuiToolstripMenu>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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

/***********************************************************************
GuiSelectableListControlInstanceLoader
***********************************************************************/

		class GuiSelectableListControlInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiSelectableListControlInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiSelectableListControl>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ItemTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ItemTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiSelectableListControl*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::_ItemTemplate)
					{
						auto factory = CreateTemplateFactory(propertyValue.propertyValue.GetText());
						auto styleProvider = new GuiListItemTemplate_ItemStyleProvider(factory);
						container->SetStyleProvider(styleProvider);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiVirtualTreeViewInstanceLoader
***********************************************************************/

		class GuiVirtualTreeViewInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiVirtualTreeViewInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiVirtualTreeView>()->GetTypeName());
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(GlobalStringKey::_ItemTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ItemTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiVirtualTreeListControl*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::_ItemTemplate)
					{
						auto factory = CreateTemplateFactory(propertyValue.propertyValue.GetText());
						auto styleProvider = new GuiTreeItemTemplate_ItemStyleProvider(factory);
						container->SetNodeStyleProvider(styleProvider);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiListViewInstanceLoader
***********************************************************************/

		class GuiListViewInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			bool				bindable;
			GlobalStringKey		typeName;
			GlobalStringKey		_View, _IconSize, _ItemSource;

		public:
			GuiListViewInstanceLoader(bool _bindable)
				:bindable(_bindable)
			{
				if (bindable)
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableListView>()->GetTypeName());
				}
				else
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiListView>()->GetTypeName());
				}
				_View = GlobalStringKey::Get(L"View");
				_IconSize = GlobalStringKey::Get(L"IconSize");
				_ItemSource = GlobalStringKey::Get(L"ItemSource");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					Ptr<IValueEnumerable> itemSource;
					ListViewViewType viewType = ListViewViewType::Detail;
					GuiListViewBase::IStyleProvider* styleProvider = 0;
					Size iconSize;
					{
						vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
						if (indexItemSource != -1)
						{
							itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);
						}
						else if (bindable)
						{
							return Value();
						}

						vint indexView = constructorArguments.Keys().IndexOf(_View);
						if (indexView != -1)
						{
							viewType = UnboxValue<ListViewViewType>(constructorArguments.GetByIndex(indexView)[0]);
						}

						vint indexIconSize = constructorArguments.Keys().IndexOf(_IconSize);
						if (indexIconSize != -1)
						{
							iconSize = UnboxValue<Size>(constructorArguments.GetByIndex(indexIconSize)[0]);
						}

						vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
						if (indexControlTemplate == -1)
						{
							styleProvider = GetCurrentTheme()->CreateListViewStyle();
						}
						else
						{
							auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
							styleProvider = new GuiListViewTemplate_StyleProvider(factory);
						}
					}

					GuiVirtualListView* listView = 0;
					if (bindable)
					{
						listView = new GuiBindableListView(styleProvider, itemSource);
					}
					else
					{
						listView = new GuiListView(styleProvider);
					}
					switch (viewType)
					{
#define VIEW_TYPE_CASE(NAME)\
					case ListViewViewType::NAME:\
						if (iconSize == Size())\
						{\
							listView->ChangeItemStyle(new list::ListView##NAME##ContentProvider);\
						}\
						else\
						{\
							listView->ChangeItemStyle(new list::ListView##NAME##ContentProvider(iconSize, false));\
						}\
						break;\

						VIEW_TYPE_CASE(BigIcon)
						VIEW_TYPE_CASE(SmallIcon)
						VIEW_TYPE_CASE(List)
						VIEW_TYPE_CASE(Tile)
						VIEW_TYPE_CASE(Information)
						VIEW_TYPE_CASE(Detail)

#undef VIEW_TYPE_CASE
					}

					return Value::From(listView);
				}
				return Value();
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
					propertyNames.Add(_View);
					propertyNames.Add(_IconSize);
					if (bindable)
					{
						propertyNames.Add(_ItemSource);
					}
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _View)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<ListViewViewType>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _IconSize)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Size>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _ItemSource)
				{
					if (bindable)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}
		};

/***********************************************************************
GuiTreeViewInstanceLoader
***********************************************************************/

		class GuiTreeViewInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			bool				bindable;
			GlobalStringKey		typeName;
			GlobalStringKey		_IconSize, _ItemSource, _Nodes;

		public:
			GuiTreeViewInstanceLoader(bool _bindable)
				:bindable(_bindable)
			{
				if (bindable)
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableTreeView>()->GetTypeName());
				}
				else
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTreeView>()->GetTypeName());
				}
				_IconSize = GlobalStringKey::Get(L"IconSize");
				_ItemSource = GlobalStringKey::Get(L"ItemSource");
				_Nodes = GlobalStringKey::Get(L"Nodes");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
					GuiVirtualTreeView::IStyleProvider* styleProvider = 0;
					{
						vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
						if (indexControlTemplate == -1)
						{
							styleProvider = GetCurrentTheme()->CreateTreeViewStyle();
						}
						else
						{
							auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
							styleProvider = new GuiTreeViewTemplate_StyleProvider(factory);
						}
					}

					GuiVirtualTreeView* treeView = 0;
					if (bindable)
					{
						if (indexItemSource == -1)
						{
							return Value();
						}

						auto itemSource = constructorArguments.GetByIndex(indexItemSource)[0];
						treeView = new GuiBindableTreeView(styleProvider, itemSource);
					}
					else
					{
						treeView = new GuiTreeView(styleProvider);
					}

					vint indexIconSize = constructorArguments.Keys().IndexOf(_IconSize);
					if (indexIconSize != -1)
					{
						auto iconSize = UnboxValue<Size>(constructorArguments.GetByIndex(indexIconSize)[0]);
						treeView->SetNodeStyleProvider(new tree::TreeViewNodeItemStyleProvider(iconSize, false));
					}

					return Value::From(treeView);
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (!bindable)
				{
					propertyNames.Add(_Nodes);
				}
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
					propertyNames.Add(_IconSize);
					if (bindable)
					{
						propertyNames.Add(_ItemSource);
					}
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _Nodes)
				{
					if (!bindable)
					{
						return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<tree::MemoryNodeProvider>());
					}
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _ItemSource)
				{
					if (bindable)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
				}
				else if (propertyInfo.propertyName == _IconSize)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Size>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiTreeView*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _Nodes)
					{
						auto item = UnboxValue<Ptr<tree::MemoryNodeProvider>>(propertyValue.propertyValue);
						container->Nodes()->Children().Add(item);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiComboBoxInstanceLoader
***********************************************************************/

		class GuiComboBoxInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey						typeName;
			GlobalStringKey						_ListControl;

		public:
			GuiComboBoxInstanceLoader()
				:typeName(GlobalStringKey::Get(L"presentation::controls::GuiComboBox"))
			{
				_ListControl = GlobalStringKey::Get(L"ListControl");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexListControl = constructorArguments.Keys().IndexOf(_ListControl);
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexListControl != -1)
					{
						Ptr<GuiTemplate::IFactory> factory;
						if (indexControlTemplate != -1)
						{
							factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						}

						GuiComboBoxBase::IStyleController* styleController = 0;
						if (factory)
						{
							styleController = new GuiComboBoxTemplate_StyleProvider(factory);
						}
						else
						{
							styleController = GetCurrentTheme()->CreateComboBoxStyle();
						}

						auto listControl = UnboxValue<GuiSelectableListControl*>(constructorArguments.GetByIndex(indexListControl)[0]);
						auto comboBox = new GuiComboBoxListControl(styleController, listControl);
						return Value::From(comboBox);
					}
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(_ListControl);
				}
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
					propertyNames.Add(_ListControl);
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _ListControl)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<GuiSelectableListControl>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					info->required = true;
					return info;
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}
		};

/***********************************************************************
GuiBindableTextListInstanceLoader
***********************************************************************/

		class GuiBindableTextListInstanceLoader : public Object, public IGuiInstanceLoader
		{
			typedef Func<list::TextItemStyleProvider::ITextItemStyleProvider*()>		ItemStyleProviderFactory;
		protected:
			GlobalStringKey					typeName;
			ItemStyleProviderFactory		itemStyleProviderFactory;
			GlobalStringKey					_ItemSource;

		public:
			GuiBindableTextListInstanceLoader(const WString& type, const ItemStyleProviderFactory& factory)
				:typeName(GlobalStringKey::Get(L"presentation::controls::GuiBindable" + type + L"TextList"))
				, itemStyleProviderFactory(factory)
			{
				_ItemSource = GlobalStringKey::Get(L"ItemSource");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
					if (indexItemSource != -1)
					{
						GuiTextListTemplate_StyleProvider* styleProvider = 0;
						{
							vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
							if (indexControlTemplate != -1)
							{
								auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
								styleProvider = new GuiTextListTemplate_StyleProvider(factory);
							}
						}

						auto itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);
						GuiBindableTextList* control = 0;
						if (styleProvider)
						{
							control = new GuiBindableTextList(styleProvider, styleProvider->CreateArgument(), itemSource);
						}
						else
						{
							control = new GuiBindableTextList(GetCurrentTheme()->CreateTextListStyle(), itemStyleProviderFactory(), itemSource);
						}
						return Value::From(control);
					}
				}
				return Value();
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
					propertyNames.Add(_ItemSource);
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				if (propertyInfo.propertyName == _ItemSource)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					info->required = true;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}
		};

/***********************************************************************
GuiBindableDataColumnInstanceLoader
***********************************************************************/

		class GuiBindableDataColumnInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey		typeName;
			GlobalStringKey		_VisualizerTemplates;
			GlobalStringKey		_EditorTemplate;

		public:
			GuiBindableDataColumnInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<list::BindableDataColumn>()->GetTypeName());
				_VisualizerTemplates = GlobalStringKey::Get(L"VisualizerTemplates");
				_EditorTemplate = GlobalStringKey::Get(L"EditorTemplate");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_VisualizerTemplates);
				propertyNames.Add(_EditorTemplate);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _VisualizerTemplates)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
				}
				else if (propertyInfo.propertyName == _EditorTemplate)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<list::BindableDataColumn*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _VisualizerTemplates)
					{
						List<WString> types;
						SplitBySemicolon(propertyValue.propertyValue.GetText(), types);
						Ptr<list::IDataVisualizerFactory> factory;
						FOREACH(WString, type, types)
						{
							auto templateFactory = CreateTemplateFactory(type);
							if (factory)
							{
								factory = new GuiBindableDataVisualizer::DecoratedFactory(templateFactory, container, factory);
							}
							else
							{
								factory = new GuiBindableDataVisualizer::Factory(templateFactory, container);
							}
						}

						container->SetVisualizerFactory(factory);
						return true;
					}
					else if (propertyValue.propertyName == _EditorTemplate)
					{
						auto templateFactory = CreateTemplateFactory(propertyValue.propertyValue.GetText());
						auto factory = new GuiBindableDataEditor::Factory(templateFactory, container);
						container->SetEditorFactory(factory);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiBindableDataGridInstanceLoader
***********************************************************************/

		class GuiBindableDataGridInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey		typeName;
			GlobalStringKey		_ItemSource;
			GlobalStringKey		_ViewModelContext;
			GlobalStringKey		_Columns;

		public:
			GuiBindableDataGridInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableDataGrid>()->GetTypeName());
				_ItemSource = GlobalStringKey::Get(L"ItemSource");
				_ViewModelContext = GlobalStringKey::Get(L"ViewModelContext");
				_Columns = GlobalStringKey::Get(L"Columns");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);	
					if (indexItemSource == -1)
					{
						return Value();
					}

					GuiBindableDataGrid::IStyleProvider* styleProvider = 0;
					vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate == -1)
					{
						styleProvider = GetCurrentTheme()->CreateListViewStyle();
					}
					else
					{
						auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
						styleProvider = new GuiListViewTemplate_StyleProvider(factory);
					}
					
					auto itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);

					Value viewModelContext;
					vint indexViewModelContext = constructorArguments.Keys().IndexOf(_ViewModelContext);
					if (indexViewModelContext != -1)
					{
						viewModelContext = constructorArguments.GetByIndex(indexViewModelContext)[0];
					}

					auto dataGrid = new GuiBindableDataGrid(styleProvider, itemSource, viewModelContext);
					return Value::From(dataGrid);
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_Columns);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
					propertyNames.Add(_ItemSource);
					propertyNames.Add(_ViewModelContext);
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _Columns)
				{
					return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<list::BindableDataColumn>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				else if (propertyInfo.propertyName == _ItemSource)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					info->required = true;
					return info;
				}
				else if (propertyInfo.propertyName == _ViewModelContext)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiBindableDataGrid*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _Columns)
					{
						auto column = UnboxValue<Ptr<list::BindableDataColumn>>(propertyValue.propertyValue);
						container->AddBindableColumn(column);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiDocumentItemInstanceLoader
***********************************************************************/

		class GuiDocumentItemInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;
			GlobalStringKey					_Name;

		public:
			GuiDocumentItemInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiDocumentItem>()->GetTypeName());
				_Name = GlobalStringKey::Get(L"Name");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeName == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexName = constructorArguments.Keys().IndexOf(_Name);	
					if (indexName == -1)
					{
						return Value();
					}

					auto name = UnboxValue<WString>(constructorArguments.GetByIndex(indexName)[0]);
					auto item = new GuiDocumentItem(name);
					return Value::From(item);
				}
				return Value();
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(_Name);
				}
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
				else if (propertyInfo.propertyName == _Name)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiDocumentItem*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->GetContainer()->AddChild(control->GetBoundsComposition());
							return true;
						}
						else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->GetContainer()->AddChild(composition);
							return true;
						}
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiDocumentCommonInterfaceInstanceLoader
***********************************************************************/

		class GuiDocumentCommonInterfaceInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiDocumentCommonInterfaceInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiDocumentCommonInterface>()->GetTypeName());
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
					return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiDocumentItem>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiDocumentCommonInterface*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						if (auto item = propertyValue.propertyValue.GetSharedPtr().Cast<GuiDocumentItem>())
						{
							container->AddDocumentItem(item);
							return true;
						}
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiAxisInstanceLoader
***********************************************************************/

		class GuiAxisInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;
			GlobalStringKey					_AxisDirection;

		public:
			GuiAxisInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiAxis>()->GetTypeName());
				_AxisDirection = GlobalStringKey::Get(L"AxisDirection");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeName == typeInfo.typeName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					vint indexAxisDirection = constructorArguments.Keys().IndexOf(_AxisDirection);	
					if (indexAxisDirection == -1)
					{
						return Value();
					}

					auto axisDirection = UnboxValue<AxisDirection>(constructorArguments.GetByIndex(indexAxisDirection)[0]);
					auto axis = new GuiAxis(axisDirection);
					return Value::From(axis);
				}
				return Value();
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					propertyNames.Add(_AxisDirection);
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _AxisDirection)
				{
					auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<AxisDirection>());
					info->scope = GuiInstancePropertyInfo::Constructor;
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}
		};

/***********************************************************************
GuiCompositionInstanceLoader
***********************************************************************/

		class GuiCompositionInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;

		public:
			GuiCompositionInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiGraphicsComposition>()->GetTypeName());
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
					info->acceptableTypes.Add(description::GetTypeDescriptor<IGuiGraphicsElement>());
					return info;
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiGraphicsComposition*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->AddChild(control->GetBoundsComposition());
							return true;
						}
						else if(auto composition = dynamic_cast<GuiGraphicsComposition*>(propertyValue.propertyValue.GetRawPtr()))
						{
							container->AddChild(composition);
							return true;
						}
						else if (Ptr<IGuiGraphicsElement> element = propertyValue.propertyValue.GetSharedPtr().Cast<IGuiGraphicsElement>())
						{
							container->SetOwnedElement(element);
							return true;
						}
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiTableCompositionInstanceLoader
***********************************************************************/

		class GuiTableCompositionInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;
			GlobalStringKey					_Rows, _Columns;

		public:
			GuiTableCompositionInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTableComposition>()->GetTypeName());
				_Rows = GlobalStringKey::Get(L"Rows");
				_Columns = GlobalStringKey::Get(L"Columns");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_Rows);
				propertyNames.Add(_Columns);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _Rows || propertyInfo.propertyName == _Columns)
				{
					return GuiInstancePropertyInfo::Array(description::GetTypeDescriptor<GuiCellOption>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiTableComposition*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _Rows)
					{
						List<GuiCellOption> options;
						CopyFrom(options, GetLazyList<GuiCellOption>(UnboxValue<Ptr<IValueList>>(propertyValue.propertyValue)));
						container->SetRowsAndColumns(options.Count(), container->GetColumns());
						FOREACH_INDEXER(GuiCellOption, option, index, options)
						{
							container->SetRowOption(index, option);
						}
						return true;
					}
					else if (propertyValue.propertyName == _Columns)
					{
						List<GuiCellOption> options;
						CopyFrom(options, GetLazyList<GuiCellOption>(UnboxValue<Ptr<IValueList>>(propertyValue.propertyValue)));
						container->SetRowsAndColumns(container->GetRows(), options.Count());
						FOREACH_INDEXER(GuiCellOption, option, index, options)
						{
							container->SetColumnOption(index, option);
						}
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiCellCompositionInstanceLoader
***********************************************************************/

		class GuiCellCompositionInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey					typeName;
			GlobalStringKey					_Site;

		public:
			GuiCellCompositionInstanceLoader()
			{
				typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiCellComposition>()->GetTypeName());
				_Site = GlobalStringKey::Get(L"Site");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_Site);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _Site)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<SiteValue>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<GuiCellComposition*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _Site)
					{
						SiteValue site = UnboxValue<SiteValue>(propertyValue.propertyValue);
						container->SetSite(site.row, site.column, site.rowSpan, site.columnSpan);
						return true;
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiTreeNodeInstanceLoader
***********************************************************************/

		class GuiTreeNodeInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			GlobalStringKey							typeName;
			GlobalStringKey							_Text, _Image, _Tag;

		public:
			GuiTreeNodeInstanceLoader()
				:typeName(GlobalStringKey::Get(L"presentation::controls::tree::TreeNode"))
			{
				_Text = GlobalStringKey::Get(L"Text");
				_Image = GlobalStringKey::Get(L"Image");
				_Tag = GlobalStringKey::Get(L"Tag");
			}

			GlobalStringKey GetTypeName()override
			{
				return typeName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == GetTypeName();
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == GetTypeName())
				{
					Ptr<tree::TreeViewItem> item = new tree::TreeViewItem;
					Ptr<tree::MemoryNodeProvider> node = new tree::MemoryNodeProvider(item);
					return Value::From(node);
				}
				return Value();
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				propertyNames.Add(_Text);
				propertyNames.Add(_Image);
				propertyNames.Add(_Tag);
				propertyNames.Add(GlobalStringKey::Empty);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				if (propertyInfo.propertyName == _Text)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
				}
				else if (propertyInfo.propertyName == _Image)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<GuiImageData>());
				}
				else if (propertyInfo.propertyName == _Tag)
				{
					return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
				}
				else if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<tree::MemoryNodeProvider>());
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				if (auto container = dynamic_cast<tree::MemoryNodeProvider*>(propertyValue.instanceValue.GetRawPtr()))
				{
					if (propertyValue.propertyName == _Text)
					{
						if (auto item = container->GetData().Cast<tree::TreeViewItem>())
						{
							item->text = UnboxValue<WString>(propertyValue.propertyValue);
							container->NotifyDataModified();
							return true;
						}
					}
					else if (propertyValue.propertyName == _Image)
					{
						if (auto item = container->GetData().Cast<tree::TreeViewItem>())
						{
							item->image = UnboxValue<Ptr<GuiImageData>>(propertyValue.propertyValue);
							container->NotifyDataModified();
							return true;
						}
					}
					else if (propertyValue.propertyName == _Tag)
					{
						if (auto item = container->GetData().Cast<tree::TreeViewItem>())
						{
							item->tag = propertyValue.propertyValue;
							return true;
						}
					}
					else if (propertyValue.propertyName == GlobalStringKey::Empty)
					{
						auto item = UnboxValue<Ptr<tree::MemoryNodeProvider>>(propertyValue.propertyValue);
						container->Children().Add(item);
						return true;
					}
				}
				return false;
			}
		};

#endif

/***********************************************************************
GuiPredefinedInstanceLoadersPlugin
***********************************************************************/

		void InitializeTrackerProgressBar(GuiScroll* control)
		{
			control->SetPageSize(0);
		}

		class GuiPredefinedInstanceLoadersPlugin : public Object, public IGuiPlugin
		{
		public:
			void Load()override
			{
			}

			void AfterLoad()override
			{
#ifndef VCZH_DEBUG_NO_REFLECTION
				IGuiInstanceLoaderManager* manager=GetInstanceLoaderManager();

#define ADD_VIRTUAL_TYPE_LOADER(TYPENAME, LOADER)\
	manager->CreateVirtualType(\
		GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new LOADER\
		)

#define ADD_TEMPLATE_CONTROL(TYPENAME, CONSTRUCTOR, TEMPLATE)\
	manager->SetLoader(\
		new GuiTemplateControlInstanceLoader(\
			L"presentation::controls::" L ## #TYPENAME,\
			[](){return Value::From(CONSTRUCTOR());},\
			[](Ptr<GuiTemplate::IFactory> factory){return Value::From(new TYPENAME(new TEMPLATE##_StyleProvider(factory))); }\
			)\
		)

#define ADD_TEMPLATE_CONTROL_2(TYPENAME, CONSTRUCTOR, TEMPLATE)\
	manager->SetLoader(\
		new GuiTemplateControlInstanceLoader(\
			L"presentation::controls::" L ## #TYPENAME,\
			[](){return Value::From(CONSTRUCTOR());},\
			[](Ptr<GuiTemplate::IFactory> factory)\
			{\
				auto style = new TEMPLATE##_StyleProvider(factory);\
				auto argument = style->CreateArgument();\
				return Value::From(new TYPENAME(style, argument));\
			}\
			)\
		)

#define ADD_VIRTUAL_CONTROL(VIRTUALTYPENAME, TYPENAME, CONSTRUCTOR, TEMPLATE)\
	manager->CreateVirtualType(\
		GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader(\
			L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
			[](){return Value::From(CONSTRUCTOR());},\
			[](Ptr<GuiTemplate::IFactory> factory){return Value::From(new TYPENAME(new TEMPLATE##_StyleProvider(factory))); }\
			)\
		)

#define ADD_VIRTUAL_CONTROL_2(VIRTUALTYPENAME, TYPENAME, CONSTRUCTOR, TEMPLATE)\
	manager->CreateVirtualType(\
		GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader(\
			L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
			[](){return Value::From(CONSTRUCTOR());},\
			[](Ptr<GuiTemplate::IFactory> factory)\
			{\
				auto style = new TEMPLATE##_StyleProvider(factory);\
				auto argument = style->CreateArgument();\
				return Value::From(new TYPENAME(style, argument));\
			}\
			)\
		)

#define ADD_VIRTUAL_CONTROL_F(VIRTUALTYPENAME, TYPENAME, CONSTRUCTOR, TEMPLATE, FUNCTION)\
	manager->CreateVirtualType(\
		GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader(\
			L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
			[](){return Value::From(CONSTRUCTOR());},\
			[](Ptr<GuiTemplate::IFactory> factory)\
			{\
				auto control = new TYPENAME(new TEMPLATE##_StyleProvider(factory));\
				FUNCTION(control);\
				return Value::From(control);\
			}\
			)\
		)

				manager->SetLoader(new GuiControlInstanceLoader);
				manager->SetLoader(new GuiTabInstanceLoader);						// ControlTemplate
				manager->SetLoader(new GuiTabPageInstanceLoader);
				manager->SetLoader(new GuiToolstripMenuInstanceLoader);				// ControlTemplate
				manager->SetLoader(new GuiToolstripMenuBarInstanceLoader);			// ControlTemplate
				manager->SetLoader(new GuiToolstripToolBarInstanceLoader);			// ControlTemplate
				manager->SetLoader(new GuiToolstripButtonInstanceLoader);			// ControlTemplate
				manager->SetLoader(new GuiSelectableListControlInstanceLoader);		// ItemTemplate
				manager->SetLoader(new GuiVirtualTreeViewInstanceLoader);			// ItemTemplate
				manager->SetLoader(new GuiListViewInstanceLoader(false));			// ControlTemplate
				manager->SetLoader(new GuiTreeViewInstanceLoader(false));			// ControlTemplate
				manager->SetLoader(new GuiBindableTextListInstanceLoader(L"", [](){return GetCurrentTheme()->CreateTextListItemStyle(); }));	// ControlTemplate, ItemSource
				manager->SetLoader(new GuiListViewInstanceLoader(true));			// ControlTemplate, ItemSource
				manager->SetLoader(new GuiTreeViewInstanceLoader(true));			// ControlTemplate, ItemSource
				manager->SetLoader(new GuiBindableDataColumnInstanceLoader);		// VisualizerTemplates, EditorTemplate
				manager->SetLoader(new GuiBindableDataGridInstanceLoader);			// ControlTemplate, ItemSource

				manager->SetLoader(new GuiDocumentItemInstanceLoader);
				manager->SetLoader(new GuiDocumentCommonInterfaceInstanceLoader);

				manager->SetLoader(new GuiAxisInstanceLoader);
				manager->SetLoader(new GuiCompositionInstanceLoader);
				manager->SetLoader(new GuiTableCompositionInstanceLoader);
				manager->SetLoader(new GuiCellCompositionInstanceLoader);
				
				ADD_VIRTUAL_TYPE_LOADER(GuiComboBoxListControl,						GuiComboBoxInstanceLoader);				// ControlTemplate
				ADD_VIRTUAL_TYPE_LOADER(tree::MemoryNodeProvider,					GuiTreeNodeInstanceLoader);

				ADD_TEMPLATE_CONTROL	(							GuiCustomControl,		g::NewCustomControl,			GuiControlTemplate);			// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiLabel,				g::NewLabel,					GuiLabelTemplate);				// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiButton,				g::NewButton,					GuiButtonTemplate);				// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiScrollContainer,		g::NewScrollContainer,			GuiScrollViewTemplate);			// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiWindow,				g::NewWindow,					GuiWindowTemplate);				// ControlTemplate
				ADD_TEMPLATE_CONTROL_2	(							GuiTextList,			g::NewTextList,					GuiTextListTemplate);			// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiDocumentViewer,		g::NewDocumentViewer,			GuiDocumentViewerTemplate);		// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiDocumentLabel,		g::NewDocumentLabel,			GuiDocumentLabelTemplate);		// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiMultilineTextBox,	g::NewMultilineTextBox,			GuiMultilineTextBoxTemplate);	// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiSinglelineTextBox,	g::NewTextBox,					GuiSinglelineTextBoxTemplate);	// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiDatePicker,			g::NewDatePicker,				GuiDatePickerTemplate);			// ControlTemplate
				ADD_TEMPLATE_CONTROL_2	(							GuiDateComboBox,		g::NewDateComboBox,				GuiDateComboBoxTemplate);		// ControlTemplate
				ADD_TEMPLATE_CONTROL	(							GuiStringGrid,			g::NewStringGrid,				GuiListViewTemplate);			// ControlTemplate
																																							// ControlTemplate
				ADD_VIRTUAL_CONTROL		(GroupBox,					GuiControl,				g::NewGroupBox,					GuiControlTemplate);			// ControlTemplate
				ADD_VIRTUAL_CONTROL		(MenuSplitter,				GuiControl,				g::NewMenuSplitter,				GuiControlTemplate);			// ControlTemplate
				ADD_VIRTUAL_CONTROL		(MenuBarButton,				GuiToolstripButton,		g::NewMenuBarButton,			GuiToolstripButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(MenuItemButton,			GuiToolstripButton,		g::NewMenuItemButton,			GuiToolstripButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(ToolstripDropdownButton,	GuiToolstripButton,		g::NewToolBarDropdownButton,	GuiToolstripButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(ToolstripSplitButton,		GuiToolstripButton,		g::NewToolBarSplitButton,		GuiToolstripButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(ToolstripSplitter,			GuiControl,				g::NewToolBarSplitter,			GuiControlTemplate);			// ControlTemplate
				ADD_VIRTUAL_CONTROL		(CheckBox,					GuiSelectableButton,	g::NewCheckBox,					GuiSelectableButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(RadioButton,				GuiSelectableButton,	g::NewRadioButton,				GuiSelectableButtonTemplate);	// ControlTemplate
				ADD_VIRTUAL_CONTROL		(HScroll,					GuiScroll,				g::NewHScroll,					GuiScrollTemplate);				// ControlTemplate
				ADD_VIRTUAL_CONTROL		(VScroll,					GuiScroll,				g::NewVScroll,					GuiScrollTemplate);				// ControlTemplate
				ADD_VIRTUAL_CONTROL_F	(HTracker,					GuiScroll,				g::NewHTracker,					GuiScrollTemplate,				InitializeTrackerProgressBar);	// ControlTemplate
				ADD_VIRTUAL_CONTROL_F	(VTracker,					GuiScroll,				g::NewVTracker,					GuiScrollTemplate,				InitializeTrackerProgressBar);	// ControlTemplate
				ADD_VIRTUAL_CONTROL_F	(ProgressBar,				GuiScroll,				g::NewProgressBar,				GuiScrollTemplate,				InitializeTrackerProgressBar);	// ControlTemplate
				ADD_VIRTUAL_CONTROL_2	(CheckTextList,				GuiTextList,			g::NewCheckTextList,			GuiTextListTemplate);			// ControlTemplate
				ADD_VIRTUAL_CONTROL_2	(RadioTextList,				GuiTextList,			g::NewRadioTextList,			GuiTextListTemplate);			// ControlTemplate

				auto bindableTextListName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableTextList>()->GetTypeName());						// ControlTemplate, ItemSource
				manager->CreateVirtualType(bindableTextListName, new GuiBindableTextListInstanceLoader(L"Check", [](){return GetCurrentTheme()->CreateCheckTextListItemStyle(); }));
				manager->CreateVirtualType(bindableTextListName, new GuiBindableTextListInstanceLoader(L"Radio", [](){return GetCurrentTheme()->CreateRadioTextListItemStyle(); }));

#undef ADD_VIRTUAL_TYPE
#undef ADD_VIRTUAL_TYPE_LOADER
#endif
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceLoadersPlugin)
	}
}