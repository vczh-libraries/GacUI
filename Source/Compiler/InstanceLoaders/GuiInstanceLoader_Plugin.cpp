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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiControl>::content.typeName);
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
						auto info = GuiInstancePropertyInfo::Collection(nullptr);
						info->acceptableTypes.Add(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
						info->acceptableTypes.Add(TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo());
						if (propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiInstanceRootObject>()))
						{
							info->acceptableTypes.Add(TypeInfoRetriver<GuiComponent*>::CreateTypeInfo());
						}
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
							auto value = values[0].expression;
							auto td = values[0].typeInfo->GetTypeDescriptor();

							Ptr<WfExpression> expr;
							if (td->CanConvertTo(description::GetTypeDescriptor<GuiComponent>()))
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
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiControlHost>()))
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
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
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
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
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
					return nullptr;
				}
			};

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
				auto createControl = MakePtr<WfNewClassExpression>();
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
			extern void LoadTemplates(IGuiInstanceLoaderManager* manager);

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
			GlobalStringKey::Get(description::TypeInfo<TYPENAME>::content.typeName),\
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
		manager->CreateVirtualType(GlobalStringKey::Get(description::TypeInfo<TYPENAME>::content.typeName),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD\
				)\
			)

	#define ADD_VIRTUAL_CONTROL_2(VIRTUALTYPENAME, TYPENAME, STYLE_METHOD, ARGUMENT_METHOD, TEMPLATE)\
		manager->CreateVirtualType(GlobalStringKey::Get(description::TypeInfo<TYPENAME>::content.typeName),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD,\
				L ## #ARGUMENT_METHOD\
				)\
			)

	#define ADD_VIRTUAL_CONTROL_F(VIRTUALTYPENAME, TYPENAME, STYLE_METHOD, TEMPLATE, INIT_FUNCTION)\
		manager->CreateVirtualType(GlobalStringKey::Get(description::TypeInfo<TYPENAME>::content.typeName),\
		new GuiTemplateControlInstanceLoader<TYPENAME, TEMPLATE##_StyleProvider, TEMPLATE>(\
				L"presentation::controls::Gui" L ## #VIRTUALTYPENAME,\
				L ## #STYLE_METHOD,\
				INIT_FUNCTION\
				)\
			)

					manager->SetLoader(new GuiControlInstanceLoader);

					ADD_TEMPLATE_CONTROL	(							GuiCustomControl,		CreateCustomControlStyle,											GuiControlTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiLabel,				CreateLabelStyle,													GuiLabelTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiButton,				CreateButtonStyle,													GuiButtonTemplate											);
					ADD_TEMPLATE_CONTROL	(							GuiScrollContainer,		CreateScrollContainerStyle,											GuiScrollViewTemplate										);
					ADD_TEMPLATE_CONTROL	(							GuiWindow,				CreateWindowStyle,													GuiWindowTemplate											);
					ADD_TEMPLATE_CONTROL_2	(							GuiTextList,			CreateTextListStyle,				CreateTextListItemStyle,		GuiTextListTemplate											);
					ADD_TEMPLATE_CONTROL_2	(							GuiBindableTextList,	CreateTextListStyle,				CreateTextListItemStyle,		GuiTextListTemplate											);
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
					ADD_VIRTUAL_CONTROL		(DocumentTextBox,			GuiDocumentLabel,		CreateDocumentTextBoxStyle,											GuiDocumentLabelTemplate									);
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
					LoadTemplates(manager);

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