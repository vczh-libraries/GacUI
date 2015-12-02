#include "GuiInstanceLoader_TemplateControl.h"

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

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

#endif
			
/***********************************************************************
GuiPredefinedInstanceLoadersPlugin
***********************************************************************/

			Ptr<WfExpression> CreateStandardDataPicker()
			{
				using TControl = GuiDatePicker;
				using TControlStyle = GuiDateComboBoxTemplate_StyleProvider;
				using TTemplate = GuiDatePickerTemplate;

				auto controlType = TypeInfoRetriver<TControl*>::CreateTypeInfo();
				auto createControl = MakePtr<WfNewTypeExpression>();
				createControl->type = GetTypeFromTypeInfo(controlType.Obj());
				createControl->arguments.Add(GuiTemplateControlInstanceLoader<TControl, TControlStyle, TTemplate>::CreateIThemeCall(L"CreateDatePickerStyle"));

				return createControl;
			}

			void InitializeTrackerProgressBar(const WString& variableName, Ptr<WfBlockStatement> block)
			{
				auto refVariable = MakePtr<WfReferenceExpression>();
				refVariable->name.value = variableName;

				auto refSetPageSize = MakePtr<WfMemberExpression>();
				refSetPageSize->parent = refVariable;
				refSetPageSize->name.value = L"SetPageSize";

				auto refZero = MakePtr<WfIntegerExpression>();
				refZero->value.value = L"0";

				auto call = MakePtr<WfCallExpression>();
				call->function = refSetPageSize;
				call->arguments.Add(refZero);

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = call;
				block->statements.Add(stat);
			}

			extern void LoadTabControls(IGuiInstanceLoaderManager* manager);
			extern void LoadToolstripControls(IGuiInstanceLoaderManager* manager);
			extern void LoadListControls(IGuiInstanceLoaderManager* manager);
			extern void LoadDocumentControls(IGuiInstanceLoaderManager* manager);
			extern void LoadCompositions(IGuiInstanceLoaderManager* manager);

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

	#define ADD_TEMPLATE_CONTROL(TYPENAME, STYLE_METHOD, TEMPLATE)\
		manager->SetLoader(\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::" L ## #TYPENAME,\
				L ## #STYLE_METHOD\
				)\
			)

	#define ADD_TEMPLATE_CONTROL_2(TYPENAME, STYLE_METHOD, ARGUMENT_METHOD, TEMPLATE)\
		manager->SetLoader(\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::" L ## #TYPENAME,\
				L ## #STYLE_METHOD,\
				L ## #ARGUMENT_METHOD\
				)\
			)

	#define ADD_TEMPLATE_CONTROL_3(TYPENAME, STYLE_METHOD, ARGUMENT_FUNCTION, TEMPLATE)\
		manager->SetLoader(\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::" L ## #TYPENAME,\
				L ## #STYLE_METHOD,\
				ARGUMENT_FUNCTION\
				)\
			)

	#define ADD_VIRTUAL_CONTROL(VIRTUALTYPENAME, TYPENAME, STYLE_METHOD, TEMPLATE)\
		manager->CreateVirtualType(GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD\
				)\
			)

	#define ADD_VIRTUAL_CONTROL_2(VIRTUALTYPENAME, TYPENAME, STYLE_METHOD, ARGUMENT_METHOD, TEMPLATE)\
		manager->CreateVirtualType(GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD,\
				L ## #ARGUMENT_METHOD\
				)\
			)

	#define ADD_VIRTUAL_CONTROL_F(VIRTUALTYPENAME, TYPENAME, STYLE_METHOD, TEMPLATE, INIT_FUNCTION)\
		manager->CreateVirtualType(GlobalStringKey::Get(description::GetTypeDescriptor<TYPENAME>()->GetTypeName()),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD,\
				INIT_FUNCTION\
				)\
			)

					manager->SetLoader(new GuiControlInstanceLoader);
					ADD_VIRTUAL_TYPE_LOADER(GuiComboBoxListControl,						GuiComboBoxInstanceLoader);

					ADD_TEMPLATE_CONTROL	(							GuiCustomControl,		CreateCustomControlStyle,											GuiControlTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiLabel,				CreateLabelStyle,													GuiLabelTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiButton,				CreateButtonStyle,													GuiButtonTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiScrollContainer,		CreateScrollContainerStyle,											GuiScrollViewTemplate										);
					ADD_TEMPLATE_CONTROL	(							GuiWindow,				CreateWindowStyle,													GuiWindowTemplate											);
					ADD_TEMPLATE_CONTROL_2	(							GuiTextList,			CreateTextListStyle,				CreateTextListItemStyle,		GuiTextListTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiMultilineTextBox,	CreateMultilineTextBoxStyle,										GuiMultilineTextBoxTemplate									);
					ADD_TEMPLATE_CONTROL	(							GuiSinglelineTextBox,	CreateTextBoxStyle,													GuiSinglelineTextBoxTemplate								);
					ADD_TEMPLATE_CONTROL	(							GuiDatePicker,			CreateDatePickerStyle,												GuiDatePickerTemplate										);
					ADD_TEMPLATE_CONTROL_3	(							GuiDateComboBox,		CreateComboBoxStyle,				CreateStandardDataPicker,		GuiDateComboBoxTemplate										);
					ADD_TEMPLATE_CONTROL	(							GuiStringGrid,			CreateListViewStyle,												GuiListViewTemplate											);

					ADD_VIRTUAL_CONTROL		(GroupBox,					GuiControl,				CreateGroupBoxStyle,												GuiControlTemplate											);
					ADD_VIRTUAL_CONTROL		(MenuSplitter,				GuiControl,				CreateMenuSplitterStyle,											GuiControlTemplate											);
					ADD_VIRTUAL_CONTROL		(MenuBarButton,				GuiToolstripButton,		CreateMenuBarButtonStyle,											GuiToolstripButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(MenuItemButton,			GuiToolstripButton,		CreateMenuItemButtonStyle,											GuiToolstripButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(ToolstripDropdownButton,	GuiToolstripButton,		CreateToolBarDropdownButtonStyle,									GuiToolstripButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(ToolstripSplitButton,		GuiToolstripButton,		CreateToolBarSplitButtonStyle,										GuiToolstripButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(ToolstripSplitter,			GuiControl,				CreateToolBarSplitterStyle,											GuiControlTemplate											);
					ADD_VIRTUAL_CONTROL		(CheckBox,					GuiSelectableButton,	CreateCheckBoxStyle,												GuiSelectableButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(RadioButton,				GuiSelectableButton,	CreateRadioButtonStyle,												GuiSelectableButtonTemplate									);
					ADD_VIRTUAL_CONTROL		(HScroll,					GuiScroll,				CreateHScrollStyle,													GuiScrollTemplate											);
					ADD_VIRTUAL_CONTROL		(VScroll,					GuiScroll,				CreateVScrollStyle,													GuiScrollTemplate											);
					ADD_VIRTUAL_CONTROL_F	(HTracker,					GuiScroll,				CreateHTrackerStyle,												GuiScrollTemplate,				InitializeTrackerProgressBar);
					ADD_VIRTUAL_CONTROL_F	(VTracker,					GuiScroll,				CreateVTrackerStyle,												GuiScrollTemplate,				InitializeTrackerProgressBar);
					ADD_VIRTUAL_CONTROL_F	(ProgressBar,				GuiScroll,				CreateProgressBarStyle,												GuiScrollTemplate,				InitializeTrackerProgressBar);
					ADD_VIRTUAL_CONTROL_2	(CheckTextList,				GuiTextList,			CreateTextListStyle,				CreateCheckTextListItemStyle,	GuiTextListTemplate											);
					ADD_VIRTUAL_CONTROL_2	(RadioTextList,				GuiTextList,			CreateTextListStyle,				CreateRadioTextListItemStyle,	GuiTextListTemplate											);

					LoadTabControls(manager);
					LoadToolstripControls(manager);
					LoadListControls(manager);
					LoadDocumentControls(manager);
					LoadCompositions(manager);

	#undef ADD_VIRTUAL_TYPE_LOADER
	#undef ADD_TEMPLATE_CONTROL
	#undef ADD_TEMPLATE_CONTROL_2
	#undef ADD_TEMPLATE_CONTROL_3
	#undef ADD_VIRTUAL_CONTROL
	#undef ADD_VIRTUAL_CONTROL_2
	#undef ADD_VIRTUAL_CONTROL_F
	#endif
				}

				void Unload()override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiPredefinedInstanceLoadersPlugin)
		}
	}
}