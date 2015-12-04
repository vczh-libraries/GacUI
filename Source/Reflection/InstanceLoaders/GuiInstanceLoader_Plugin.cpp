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

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::Empty)
							{
								auto value = values[0].expression;
								auto type = values[0].type;

								Ptr<WfExpression> expr;
								if (type->CanConvertTo(description::GetTypeDescriptor<GuiComponent>()))
								{
									auto refControl = MakePtr<WfReferenceExpression>();
									refControl->name.value = variableName.ToString();

									auto refAddComponent = MakePtr<WfMemberExpression>();
									refAddComponent->parent = refControl;
									refAddComponent->name.value = L"AddComponent";

									auto call = MakePtr<WfCallExpression>();
									call->function = refAddComponent;
									call->arguments.Add(value);

									expr = call;
								}
								else if (type->CanConvertTo(description::GetTypeDescriptor<GuiControlHost>()))
								{
									auto refControl = MakePtr<WfReferenceExpression>();
									refControl->name.value = variableName.ToString();

									auto refAddControlHostComponent = MakePtr<WfMemberExpression>();
									refAddControlHostComponent->parent = refControl;
									refAddControlHostComponent->name.value = L"AddControlHostComponent";

									auto call = MakePtr<WfCallExpression>();
									call->function = refAddControlHostComponent;
									call->arguments.Add(value);

									expr = call;
								}
								else if (type->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
								{
									auto refControl = MakePtr<WfReferenceExpression>();
									refControl->name.value = variableName.ToString();

									auto refAddChild = MakePtr<WfMemberExpression>();
									refAddChild->parent = refControl;
									refAddChild->name.value = L"AddChild";

									auto call = MakePtr<WfCallExpression>();
									call->function = refAddChild;
									call->arguments.Add(value);

									expr = call;
								}
								else if (type->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
								{
									auto refControl = MakePtr<WfReferenceExpression>();
									refControl->name.value = variableName.ToString();

									auto refContainerComposition = MakePtr<WfMemberExpression>();
									refContainerComposition->parent = refControl;
									refContainerComposition->name.value = L"ContainerComposition";

									auto refAddChild = MakePtr<WfMemberExpression>();
									refAddChild->parent = refContainerComposition;
									refAddChild->name.value = L"AddChild";

									auto call = MakePtr<WfCallExpression>();
									call->function = refAddChild;
									call->arguments.Add(value);

									expr = call;
								}

								if (expr)
								{
									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = expr;
									block->statements.Add(stat);
								}
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
					}
					return nullptr;
				}
			};

/***********************************************************************
GuiComboBoxInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiComboBoxListControl, GuiComboBoxTemplate_StyleProvider, GuiComboBoxTemplate>
			class GuiComboBoxInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey						_ListControl;

				void AddAdditionalArguments(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfNewTypeExpression> createControl)override
				{
					vint indexListControl = arguments.Keys().IndexOf(_ListControl);
					if (indexListControl != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexListControl)[0].expression);
					}
				}
			public:
				GuiComboBoxInstanceLoader()
					:BASE_TYPE(L"presentation::controls::GuiComboBox", L"CreateComboBoxStyle")
				{
					_ListControl = GlobalStringKey::Get(L"ListControl");
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
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
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

#endif
			
/***********************************************************************
GuiPredefinedInstanceLoadersPlugin
***********************************************************************/

			Ptr<WfExpression> CreateStandardDataPicker(IGuiInstanceLoader::ArgumentMap&)
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