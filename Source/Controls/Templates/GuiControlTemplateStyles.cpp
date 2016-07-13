#include "GuiControlTemplateStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			using namespace compositions;
			using namespace elements;
			using namespace controls;
			using namespace controls::list;
			using namespace reflection::description;
			using namespace collections;

#define INITIALIZE_FACTORY_FROM_TEMPLATE(VARIABLE, PROPERTY)\
	controlTemplate->PROPERTY##Changed.AttachLambda([this](GuiGraphicsComposition*, GuiEventArgs&)\
	{\
		this->VARIABLE = 0;\
	});\

#define GET_FACTORY_FROM_TEMPLATE(TEMPLATE, VARIABLE, PROPERTY)\
	if (!this->VARIABLE)\
	{\
		this->VARIABLE = CreateTemplateFactory(controlTemplate->Get##PROPERTY());\
	}\
	return new TEMPLATE##_StyleProvider(this->VARIABLE);\

#define GET_FACTORY_FROM_TEMPLATE_OPT(TEMPLATE, VARIABLE, PROPERTY)\
	if (controlTemplate->Get##PROPERTY() == L"")\
	{\
		return 0;\
	}\
	GET_FACTORY_FROM_TEMPLATE(TEMPLATE, VARIABLE, PROPERTY)\

/***********************************************************************
GuiControlTemplate_StyleProvider
***********************************************************************/

			GuiControlTemplate_StyleProvider::GuiControlTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory, description::Value viewModel)
				:associatedStyleController(0)
				, controlTemplate(0)
			{
				GuiTemplate* itemTemplate = factory->CreateTemplate(viewModel);
				if (!(controlTemplate = dynamic_cast<GuiControlTemplate*>(itemTemplate)))
				{
					delete itemTemplate;
					CHECK_FAIL(L"GuiControlTemplate_StyleProvider::GuiControlTemplate_StyleProvider()#An instance of GuiTemplate is expected.");
				}
			}

			GuiControlTemplate_StyleProvider::~GuiControlTemplate_StyleProvider()
			{
			}

			compositions::GuiBoundsComposition* GuiControlTemplate_StyleProvider::GetBoundsComposition()
			{
				return controlTemplate;
			}

			compositions::GuiGraphicsComposition* GuiControlTemplate_StyleProvider::GetContainerComposition()
			{
				return controlTemplate->GetContainerComposition();
			}

			void GuiControlTemplate_StyleProvider::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
				associatedStyleController = controller;
			}

			void GuiControlTemplate_StyleProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				controlTemplate->SetFocusableComposition(value);
			}

			void GuiControlTemplate_StyleProvider::SetText(const WString& value)
			{
				controlTemplate->SetText(value);
			}

			void GuiControlTemplate_StyleProvider::SetFont(const FontProperties& value)
			{
				controlTemplate->SetFont(value);
			}

			void GuiControlTemplate_StyleProvider::SetVisuallyEnabled(bool value)
			{
				controlTemplate->SetVisuallyEnabled(value);
			}

/***********************************************************************
GuiLabelTemplate_StyleProvider
***********************************************************************/

			GuiLabelTemplate_StyleProvider::GuiLabelTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiLabelTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiLabelTemplate_StyleProvider::GuiLabelTemplate_StyleProvider()#An instance of GuiLabelTemplate is expected.");
				}
			}

			GuiLabelTemplate_StyleProvider::~GuiLabelTemplate_StyleProvider()
			{
			}

			Color GuiLabelTemplate_StyleProvider::GetDefaultTextColor()
			{
				return controlTemplate->GetDefaultTextColor();
			}

			void GuiLabelTemplate_StyleProvider::SetTextColor(Color value)
			{
				controlTemplate->SetTextColor(value);
			}

/***********************************************************************
GuiSinglelineTextBoxTemplate_StyleProvider
***********************************************************************/

			GuiSinglelineTextBoxTemplate_StyleProvider::GuiSinglelineTextBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiSinglelineTextBoxTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiSinglelineTextBoxTemplate_StyleProvider::GuiSinglelineTextBoxTemplate_StyleProvider()#An instance of GuiSinglelineTextBoxTemplate is expected.");
				}
			}

			GuiSinglelineTextBoxTemplate_StyleProvider::~GuiSinglelineTextBoxTemplate_StyleProvider()
			{
			}
			
			void GuiSinglelineTextBoxTemplate_StyleProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				GuiControlTemplate_StyleProvider::SetFocusableComposition(value);
				if (auto style = dynamic_cast<GuiSinglelineTextBox::StyleController*>(associatedStyleController))
				{
					auto element = style->GetTextElement();
					Array<text::ColorEntry> colors(1);
					colors[0] = controlTemplate->GetTextColor();
					element->SetColors(colors);
					element->SetCaretColor(controlTemplate->GetCaretColor());
				}
			}

			compositions::GuiGraphicsComposition* GuiSinglelineTextBoxTemplate_StyleProvider::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				controlTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
				boundsComposition->AddChild(controlTemplate);
				return controlTemplate->GetContainerComposition();
			}

/***********************************************************************
GuiDocumentLabelTemplate_StyleProvider
***********************************************************************/

			GuiDocumentLabelTemplate_StyleProvider::GuiDocumentLabelTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDocumentLabelTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDocumentLabelTemplate_StyleProvider::GuiDocumentLabelTemplate_StyleProvider()#An instance of GuiDocumentLabelTemplate is expected.");
				}
			}

			GuiDocumentLabelTemplate_StyleProvider::~GuiDocumentLabelTemplate_StyleProvider()
			{
			}

			Ptr<DocumentModel> GuiDocumentLabelTemplate_StyleProvider::GetBaselineDocument()
			{
				return controlTemplate->GetBaselineDocument();
			}

/***********************************************************************
GuiMenuTemplate_StyleProvider
***********************************************************************/

			GuiMenuTemplate_StyleProvider::GuiMenuTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
			}

			GuiMenuTemplate_StyleProvider::~GuiMenuTemplate_StyleProvider()
			{
			}

/***********************************************************************
GuiWindowTemplate_StyleProvider
***********************************************************************/

			GuiWindowTemplate_StyleProvider::GuiWindowTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
				, window(0)
			{
				if (!(controlTemplate = dynamic_cast<GuiWindowTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiWindowTemplate_StyleProvider::GuiWindowTemplate_StyleProvider()#An instance of GuiWindowTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(tooltipTemplateFactory, TooltipTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(shortcutKeyTemplateFactory, ShortcutKeyTemplate);
			}

			GuiWindowTemplate_StyleProvider::~GuiWindowTemplate_StyleProvider()
			{
			}

			void GuiWindowTemplate_StyleProvider::AttachWindow(GuiWindow* _window)
			{
				window = _window;
			}

			void GuiWindowTemplate_StyleProvider::InitializeNativeWindowProperties()
			{
				if (window && window->GetNativeWindow())
				{
					window->GetNativeWindow()->EnableCustomFrameMode();
					window->GetNativeWindow()->SetBorder(false);
				}
			}

			void GuiWindowTemplate_StyleProvider::SetSizeState(INativeWindow::WindowSizeState value)
			{
				controlTemplate->SetMaximized(value == INativeWindow::Maximized);
			}

#define WINDOW_TEMPLATE_GET(PROPERTY)\
				switch (controlTemplate->Get##PROPERTY##Option())\
				{\
					case BoolOption::AlwaysTrue: return true;\
					case BoolOption::AlwaysFalse: return false;\
					default: return controlTemplate->Get##PROPERTY();\
				}\

#define WINDOW_TEMPLATE_SET(PROPERTY)\
				if (controlTemplate->Get##PROPERTY##Option() == BoolOption::Customizable)\
				{\
					controlTemplate->Set##PROPERTY(visible);\
					if (!controlTemplate->GetCustomizedBorder() && window && window->GetNativeWindow())\
					{\
						window->GetNativeWindow()->Set##PROPERTY(visible);\
					}\
				}\

			bool GuiWindowTemplate_StyleProvider::GetMaximizedBox()
			{
				WINDOW_TEMPLATE_GET(MaximizedBox);
			}

			void GuiWindowTemplate_StyleProvider::SetMaximizedBox(bool visible)
			{
				WINDOW_TEMPLATE_SET(MaximizedBox);
			}

			bool GuiWindowTemplate_StyleProvider::GetMinimizedBox()
			{
				WINDOW_TEMPLATE_GET(MinimizedBox);
			}

			void GuiWindowTemplate_StyleProvider::SetMinimizedBox(bool visible)
			{
				WINDOW_TEMPLATE_SET(MinimizedBox);
			}

			bool GuiWindowTemplate_StyleProvider::GetBorder()
			{
				WINDOW_TEMPLATE_GET(Border);
			}

			void GuiWindowTemplate_StyleProvider::SetBorder(bool visible)
			{
				WINDOW_TEMPLATE_SET(Border);
			}

			bool GuiWindowTemplate_StyleProvider::GetSizeBox()
			{
				WINDOW_TEMPLATE_GET(SizeBox);
			}

			void GuiWindowTemplate_StyleProvider::SetSizeBox(bool visible)
			{
				WINDOW_TEMPLATE_SET(SizeBox);
			}

			bool GuiWindowTemplate_StyleProvider::GetIconVisible()
			{
				WINDOW_TEMPLATE_GET(IconVisible);
			}

			void GuiWindowTemplate_StyleProvider::SetIconVisible(bool visible)
			{
				WINDOW_TEMPLATE_SET(IconVisible);
			}

			bool GuiWindowTemplate_StyleProvider::GetTitleBar()
			{
				WINDOW_TEMPLATE_GET(TitleBar);
			}

			void GuiWindowTemplate_StyleProvider::SetTitleBar(bool visible)
			{
				WINDOW_TEMPLATE_SET(TitleBar);
			}

			controls::GuiWindow::IStyleController* GuiWindowTemplate_StyleProvider::CreateTooltipStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiWindowTemplate, tooltipTemplateFactory, TooltipTemplate);
			}

			controls::GuiLabel::IStyleController* GuiWindowTemplate_StyleProvider::CreateShortcutKeyStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiLabelTemplate, shortcutKeyTemplateFactory, ShortcutKeyTemplate);
			}

#undef WINDOW_TEMPLATE_GET
#undef WINDOW_TEMPLATE_SET

/***********************************************************************
GuiButtonTemplate_StyleProvider
***********************************************************************/

			GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiButtonTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider()#An instance of GuiButtonTemplate is expected.");
				}
			}

			GuiButtonTemplate_StyleProvider::~GuiButtonTemplate_StyleProvider()
			{
			}

			void GuiButtonTemplate_StyleProvider::Transfer(controls::GuiButton::ControlState value)
			{
				controlTemplate->SetState(value);
			}

/***********************************************************************
GuiSelectableButtonTemplate_StyleProvider
***********************************************************************/

			GuiSelectableButtonTemplate_StyleProvider::GuiSelectableButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiButtonTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiSelectableButtonTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider()#An instance of GuiSelectableButtonTemplate is expected.");
				}
			}

			GuiSelectableButtonTemplate_StyleProvider::~GuiSelectableButtonTemplate_StyleProvider()
			{
			}

			void GuiSelectableButtonTemplate_StyleProvider::SetSelected(bool value)
			{
				controlTemplate->SetSelected(value);
			}

/***********************************************************************
GuiToolstripButtonTemplate_StyleProvider
***********************************************************************/

			GuiToolstripButtonTemplate_StyleProvider::GuiToolstripButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiSelectableButtonTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiToolstripButtonTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider()#An instance of GuiToolstripButtonTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(subMenuTemplateFactory, SubMenuTemplate);
			}

			GuiToolstripButtonTemplate_StyleProvider::~GuiToolstripButtonTemplate_StyleProvider()
			{
			}
				
			controls::GuiMenu::IStyleController* GuiToolstripButtonTemplate_StyleProvider::CreateSubMenuStyleController()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiMenuTemplate, subMenuTemplateFactory, SubMenuTemplate);
			}

			void GuiToolstripButtonTemplate_StyleProvider::SetSubMenuExisting(bool value)
			{
				controlTemplate->SetSubMenuExisting(value);
			}

			void GuiToolstripButtonTemplate_StyleProvider::SetSubMenuOpening(bool value)
			{
				controlTemplate->SetSubMenuOpening(value);
			}

			controls::GuiButton* GuiToolstripButtonTemplate_StyleProvider::GetSubMenuHost()
			{
				return controlTemplate->GetSubMenuHost();
			}

			void GuiToolstripButtonTemplate_StyleProvider::SetImage(Ptr<GuiImageData> value)
			{
				controlTemplate->SetImage(value);
			}

			void GuiToolstripButtonTemplate_StyleProvider::SetShortcutText(const WString& value)
			{
				controlTemplate->SetShortcutText(value);
			}

/***********************************************************************
GuiListViewColumnHeaderTemplate_StyleProvider
***********************************************************************/

			GuiListViewColumnHeaderTemplate_StyleProvider::GuiListViewColumnHeaderTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiToolstripButtonTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiListViewColumnHeaderTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiListViewColumnHeaderTemplate_StyleProvider::GuiListViewColumnHeaderTemplate_StyleProvider()#An instance of GuiListViewColumnHeaderTemplate is expected.");
				}
			}

			GuiListViewColumnHeaderTemplate_StyleProvider::~GuiListViewColumnHeaderTemplate_StyleProvider()
			{
			}

			void GuiListViewColumnHeaderTemplate_StyleProvider::SetColumnSortingState(controls::GuiListViewColumnHeader::ColumnSortingState value)
			{
				controlTemplate->SetSortingState(value);
			}

/***********************************************************************
GuiComboBoxTemplate_StyleProvider
***********************************************************************/

			GuiComboBoxTemplate_StyleProvider::GuiComboBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiToolstripButtonTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiComboBoxTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiComboBoxTemplate_StyleProvider::GuiComboBoxTemplate_StyleProvider()#An instance of GuiComboBoxTemplate is expected.");
				}
			}

			GuiComboBoxTemplate_StyleProvider::~GuiComboBoxTemplate_StyleProvider()
			{
			}

			void GuiComboBoxTemplate_StyleProvider::SetCommandExecutor(controls::GuiComboBoxBase::ICommandExecutor* value)
			{
				controlTemplate->SetCommands(value);
			}

			void GuiComboBoxTemplate_StyleProvider::OnItemSelected()
			{
			}

			void GuiComboBoxTemplate_StyleProvider::SetTextVisible(bool value)
			{
				controlTemplate->SetTextVisible(value);
			}

/***********************************************************************
GuiDatePickerTemplate_StyleProvider
***********************************************************************/

			GuiDatePickerTemplate_StyleProvider::GuiDatePickerTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDatePickerTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDatePickerTemplate_StyleProvider::GuiDatePickerTemplate_StyleProvider()#An instance of GuiDatePickerTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(dateButtonTemplateFactory, DateButtonTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(dateTextListTemplateFactory, DateTextListTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(dateComboBoxTemplateFactory, DateComboBoxTemplate);
			}

			GuiDatePickerTemplate_StyleProvider::~GuiDatePickerTemplate_StyleProvider()
			{
				delete controlTemplate;
			}

			controls::GuiSelectableButton::IStyleController* GuiDatePickerTemplate_StyleProvider::CreateDateButtonStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, dateButtonTemplateFactory, DateButtonTemplate);
			}

			GuiTextListTemplate_StyleProvider* GuiDatePickerTemplate_StyleProvider::CreateTextListStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiTextListTemplate, dateTextListTemplateFactory, DateTextListTemplate);
			}

			controls::GuiTextList* GuiDatePickerTemplate_StyleProvider::CreateTextList()
			{
				auto style = CreateTextListStyle();
				return new GuiTextList(style, style->CreateArgument());
			}

			controls::GuiComboBoxListControl::IStyleController* GuiDatePickerTemplate_StyleProvider::CreateComboBoxStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiComboBoxTemplate, dateComboBoxTemplateFactory, DateComboBoxTemplate);
			}

			Color GuiDatePickerTemplate_StyleProvider::GetBackgroundColor()
			{
				return controlTemplate->GetBackgroundColor();
			}

			Color GuiDatePickerTemplate_StyleProvider::GetPrimaryTextColor()
			{
				return controlTemplate->GetPrimaryTextColor();
			}

			Color GuiDatePickerTemplate_StyleProvider::GetSecondaryTextColor()
			{
				return controlTemplate->GetSecondaryTextColor();
			}

/***********************************************************************
GuiDateComboBoxTemplate_StyleProvider
***********************************************************************/

			GuiDateComboBoxTemplate_StyleProvider::GuiDateComboBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiComboBoxTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDateComboBoxTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDateComboBoxTemplate_StyleProvider::GuiDateComboBoxTemplate_StyleProvider()#An instance of GuiDateComboBoxTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(datePickerTemplateFactory, DatePickerTemplate);
			}

			GuiDateComboBoxTemplate_StyleProvider::~GuiDateComboBoxTemplate_StyleProvider()
			{
			}

			controls::GuiDatePicker* GuiDateComboBoxTemplate_StyleProvider::CreateArgument()
			{
				return new GuiDatePicker(CreateDatePickerStyle());
			}

			controls::GuiDatePicker::IStyleProvider* GuiDateComboBoxTemplate_StyleProvider::CreateDatePickerStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiDatePickerTemplate, datePickerTemplateFactory, DatePickerTemplate);
			}

/***********************************************************************
GuiScrollTemplate_StyleProvider
***********************************************************************/

			GuiScrollTemplate_StyleProvider::GuiScrollTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiScrollTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider()#An instance of GuiScrollTemplate is expected.");
				}
			}

			GuiScrollTemplate_StyleProvider::~GuiScrollTemplate_StyleProvider()
			{
			}

			void GuiScrollTemplate_StyleProvider::SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)
			{
				controlTemplate->SetCommands(value);
			}

			void GuiScrollTemplate_StyleProvider::SetTotalSize(vint value)
			{
				controlTemplate->SetTotalSize(value);
			}

			void GuiScrollTemplate_StyleProvider::SetPageSize(vint value)
			{
				controlTemplate->SetPageSize(value);
			}

			void GuiScrollTemplate_StyleProvider::SetPosition(vint value)
			{
				controlTemplate->SetPosition(value);
			}

/***********************************************************************
GuiScrollViewTemplate_StyleProvider
***********************************************************************/

			GuiScrollViewTemplate_StyleProvider::GuiScrollViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiScrollViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiScrollViewTemplate_StyleProvider::GuiScrollViewTemplate_StyleProvider()#An instance of GuiScrollViewTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(hScrollTemplateFactory, HScrollTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(vScrollTemplateFactory, VScrollTemplate);
			}

			GuiScrollViewTemplate_StyleProvider::~GuiScrollViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiScroll::IStyleController* GuiScrollViewTemplate_StyleProvider::CreateHorizontalScrollStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiScrollTemplate, hScrollTemplateFactory, HScrollTemplate);
			}

			controls::GuiScroll::IStyleController* GuiScrollViewTemplate_StyleProvider::CreateVerticalScrollStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiScrollTemplate, vScrollTemplateFactory, VScrollTemplate);
			}

			vint GuiScrollViewTemplate_StyleProvider::GetDefaultScrollSize()
			{
				return controlTemplate->GetDefaultScrollSize();
			}

			compositions::GuiGraphicsComposition* GuiScrollViewTemplate_StyleProvider::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				controlTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
				boundsComposition->AddChild(controlTemplate);
				return controlTemplate->GetContainerComposition();
			}

/***********************************************************************
GuiSinglelineTextBoxTemplate_StyleProvider
***********************************************************************/

			GuiMultilineTextBoxTemplate_StyleProvider::GuiMultilineTextBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiMultilineTextBoxTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiMultilineTextBoxTemplate_StyleProvider::GuiMultilineTextBoxTemplate_StyleProvider()#An instance of GuiMultilineTextBoxTemplate is expected.");
				}
			}

			GuiMultilineTextBoxTemplate_StyleProvider::~GuiMultilineTextBoxTemplate_StyleProvider()
			{
			}
			
			void GuiMultilineTextBoxTemplate_StyleProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				GuiScrollViewTemplate_StyleProvider::SetFocusableComposition(value);
				if (auto style = dynamic_cast<GuiMultilineTextBox::StyleController*>(associatedStyleController))
				{
					auto element = style->GetTextElement();
					Array<text::ColorEntry> colors(1);
					colors[0] = controlTemplate->GetTextColor();
					element->SetColors(colors);
					element->SetCaretColor(controlTemplate->GetCaretColor());
				}
			}

/***********************************************************************
GuiDocumentViewerTemplate_StyleProvider
***********************************************************************/

			GuiDocumentViewerTemplate_StyleProvider::GuiDocumentViewerTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDocumentViewerTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDocumentViewerTemplate_StyleProvider::GuiDocumentViewerTemplate_StyleProvider()#An instance of GuiDocumentViewerTemplate is expected.");
				}
			}

			GuiDocumentViewerTemplate_StyleProvider::~GuiDocumentViewerTemplate_StyleProvider()
			{
			}

			Ptr<DocumentModel> GuiDocumentViewerTemplate_StyleProvider::GetBaselineDocument()
			{
				return controlTemplate->GetBaselineDocument();
			}

/***********************************************************************
GuiTextListTemplate_StyleProvider::ItemStyleProvider
***********************************************************************/

			GuiTextListTemplate_StyleProvider::ItemStyleProvider::ItemStyleProvider(GuiTextListTemplate_StyleProvider* _styleProvider)
				:styleProvider(_styleProvider)
			{
			}

			GuiTextListTemplate_StyleProvider::ItemStyleProvider::~ItemStyleProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::ItemStyleProvider::CreateBulletStyleController()
			{
				return styleProvider->CreateBulletStyle();
			}

/***********************************************************************
GuiTextListTemplate_StyleProvider
***********************************************************************/

			GuiTextListTemplate_StyleProvider::GuiTextListTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTextListTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTextListTemplate_StyleProvider::GuiTextListTemplate_StyleProvider()#An instance of GuiTextListTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(backgroundTemplateFactory, BackgroundTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(bulletTemplateFactory, BulletTemplate);
			}

			GuiTextListTemplate_StyleProvider::~GuiTextListTemplate_StyleProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, backgroundTemplateFactory, BackgroundTemplate);
			}

			Color GuiTextListTemplate_StyleProvider::GetTextColor()
			{
				return controlTemplate->GetTextColor();
			}

			controls::list::TextItemStyleProvider::IBulletFactory* GuiTextListTemplate_StyleProvider::CreateArgument()
			{
				return new ItemStyleProvider(this);
			}

			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::CreateBulletStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, bulletTemplateFactory, BulletTemplate);
			}

/***********************************************************************
GuiListViewTemplate_StyleProvider
***********************************************************************/

			GuiListViewTemplate_StyleProvider::GuiListViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiListViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiListViewTemplate_StyleProvider::GuiListViewTemplate_StyleProvider()#An instance of GuiListViewTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(backgroundTemplateFactory, BackgroundTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(columnHeaderTemplateFactory, ColumnHeaderTemplate);
			}

			GuiListViewTemplate_StyleProvider::~GuiListViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiSelectableButton::IStyleController* GuiListViewTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, backgroundTemplateFactory, BackgroundTemplate);
			}

			controls::GuiListViewColumnHeader::IStyleController* GuiListViewTemplate_StyleProvider::CreateColumnStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiListViewColumnHeaderTemplate, columnHeaderTemplateFactory, ColumnHeaderTemplate);
			}

			Color GuiListViewTemplate_StyleProvider::GetPrimaryTextColor()
			{
				return controlTemplate->GetPrimaryTextColor();
			}

			Color GuiListViewTemplate_StyleProvider::GetSecondaryTextColor()
			{
				return controlTemplate->GetSecondaryTextColor();
			}

			Color GuiListViewTemplate_StyleProvider::GetItemSeparatorColor()
			{
				return controlTemplate->GetItemSeparatorColor();
			}

/***********************************************************************
GuiTreeViewTemplate_StyleProvider
***********************************************************************/

			GuiTreeViewTemplate_StyleProvider::GuiTreeViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTreeViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTreeViewTemplate_StyleProvider::GuiTreeViewTemplate_StyleProvider()#An instance of GuiTreeViewTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(backgroundTemplateFactory, BackgroundTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(expandingDecoratorTemplateFactory, ExpandingDecoratorTemplate);
			}

			GuiTreeViewTemplate_StyleProvider::~GuiTreeViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiSelectableButton::IStyleController* GuiTreeViewTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, backgroundTemplateFactory, BackgroundTemplate);
			}

			controls::GuiSelectableButton::IStyleController* GuiTreeViewTemplate_StyleProvider::CreateItemExpandingDecorator()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, expandingDecoratorTemplateFactory, ExpandingDecoratorTemplate);
			}

			Color GuiTreeViewTemplate_StyleProvider::GetTextColor()
			{
				return controlTemplate->GetTextColor();
			}

/***********************************************************************
GuiTabTemplate_StyleProvider
***********************************************************************/

			void GuiTabTemplate_StyleProvider::OnHeaderButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(commandExecutor)
				{
					vint index=headerButtons.IndexOf(dynamic_cast<GuiSelectableButton*>(sender->GetAssociatedControl()));
					if(index!=-1)
					{
						commandExecutor->ShowTab(index);
					}
				}
			}

			void GuiTabTemplate_StyleProvider::OnTabHeaderBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				vint height=headerOverflowButton->GetBoundsComposition()->GetBounds().Height();
				headerOverflowButton->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(height, 0)));

				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::OnHeaderOverflowButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				headerOverflowMenu->SetClientSize(Size(0, 0));
				headerOverflowMenu->ShowPopup(headerOverflowButton, true);
			}

			void GuiTabTemplate_StyleProvider::OnHeaderOverflowMenuButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				vint index=headerOverflowMenu->GetToolstripItems().IndexOf(sender->GetRelatedControl());
				commandExecutor->ShowTab(index);
			}

			void GuiTabTemplate_StyleProvider::UpdateHeaderOverflowButtonVisibility()
			{
				if(tabHeaderComposition->IsStackItemClipped())
				{
					tabBoundsComposition->SetColumnOption(1, GuiCellOption::MinSizeOption());
				}
				else
				{
					tabBoundsComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				}
				tabBoundsComposition->ForceCalculateSizeImmediately();
			}

			void GuiTabTemplate_StyleProvider::UpdateHeaderZOrder()
			{
				vint itemCount=tabHeaderComposition->GetStackItems().Count();
				vint childCount=tabHeaderComposition->Children().Count();
				for(vint i=0;i<itemCount;i++)
				{
					GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(i);
					if(headerButtons[i]->GetSelected())
					{
						tabHeaderComposition->MoveChild(item, childCount-1);
						vint padding = controlTemplate->GetHeaderPadding();
						item->SetExtraMargin(Margin(padding, padding, padding, 0));
					}
					else
					{
						item->SetExtraMargin(Margin(0, 0, 0, 0));
					}
				}
				if(childCount>1)
				{
					tabHeaderComposition->MoveChild(tabContentTopLineComposition, childCount-2);
				}
			}

			void GuiTabTemplate_StyleProvider::UpdateHeaderVisibilityIndex()
			{
				vint itemCount=tabHeaderComposition->GetStackItems().Count();
				vint selectedItem=-1;
				for(vint i=0;i<itemCount;i++)
				{
					if(headerButtons[i]->GetSelected())
					{
						selectedItem=i;
					}
				}

				if(selectedItem!=-1)
				{
					tabHeaderComposition->EnsureVisible(selectedItem);
				}
			}

			void GuiTabTemplate_StyleProvider::UpdateHeaderLayout()
			{
				UpdateHeaderZOrder();
				UpdateHeaderVisibilityIndex();
				UpdateHeaderOverflowButtonVisibility();
			}

			void GuiTabTemplate_StyleProvider::Initialize()
			{
				tabBoundsComposition=new GuiTableComposition;
				tabBoundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				tabBoundsComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				tabBoundsComposition->SetRowsAndColumns(1, 2);
				tabBoundsComposition->SetRowOption(0, GuiCellOption::MinSizeOption());
				tabBoundsComposition->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				tabBoundsComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				controlTemplate->GetHeaderComposition()->AddChild(tabBoundsComposition);
				
				vint padding = controlTemplate->GetHeaderPadding();
				{
					GuiCellComposition* cell=new GuiCellComposition;
					tabBoundsComposition->AddChild(cell);
					cell->SetSite(0, 0, 1, 1);
					
					vint padding = controlTemplate->GetHeaderPadding();
					tabHeaderComposition=new GuiStackComposition;
					tabHeaderComposition->SetExtraMargin(Margin(padding, padding, padding, 0));
					tabHeaderComposition->SetAlignmentToParent(Margin(0, 0, 1, 0));
					tabHeaderComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					tabHeaderComposition->BoundsChanged.AttachMethod(this, &GuiTabTemplate_StyleProvider::OnTabHeaderBoundsChanged);
					cell->AddChild(tabHeaderComposition);
				}
				{
					GuiCellComposition* cell=new GuiCellComposition;
					tabBoundsComposition->AddChild(cell);
					cell->SetSite(0, 1, 1, 1);

					headerOverflowButton=new GuiButton(CreateDropdownTemplate());
					headerOverflowButton->GetBoundsComposition()->SetAlignmentToParent(Margin(-1, padding, 0, 0));
					headerOverflowButton->Clicked.AttachMethod(this, &GuiTabTemplate_StyleProvider::OnHeaderOverflowButtonClicked);
					cell->AddChild(headerOverflowButton->GetBoundsComposition());
				}

				headerOverflowMenu=new GuiToolstripMenu(CreateMenuTemplate(), 0);
				headerController=new GuiSelectableButton::MutexGroupController;
			}

			controls::GuiSelectableButton::IStyleController* GuiTabTemplate_StyleProvider::CreateHeaderTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, headerTemplateFactory, HeaderTemplate);
			}

			controls::GuiButton::IStyleController* GuiTabTemplate_StyleProvider::CreateDropdownTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiButtonTemplate, dropdownTemplateFactory, DropdownTemplate);
			}

			controls::GuiMenu::IStyleController* GuiTabTemplate_StyleProvider::CreateMenuTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiMenuTemplate, menuTemplateFactory, MenuTemplate);
			}

			controls::GuiToolstripButton::IStyleController* GuiTabTemplate_StyleProvider::CreateMenuItemTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiToolstripButtonTemplate, menuItemTemplateFactory, MenuItemTemplate);
			}

			GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTabTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider()#An instance of GuiTabTemplate is expected.");
				}
				INITIALIZE_FACTORY_FROM_TEMPLATE(headerTemplateFactory, HeaderTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(dropdownTemplateFactory, DropdownTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(menuTemplateFactory, MenuTemplate);
				INITIALIZE_FACTORY_FROM_TEMPLATE(menuItemTemplateFactory, MenuItemTemplate);
				Initialize();
			}

			GuiTabTemplate_StyleProvider::~GuiTabTemplate_StyleProvider()
			{
				delete headerOverflowMenu;
			}

			void GuiTabTemplate_StyleProvider::SetCommandExecutor(controls::GuiTab::ICommandExecutor* value)
			{
				commandExecutor=value;
			}

			void GuiTabTemplate_StyleProvider::InsertTab(vint index)
			{
				GuiSelectableButton* button=new GuiSelectableButton(CreateHeaderTemplate());
				button->SetAutoSelection(false);
				button->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				button->SetGroupController(headerController.Obj());
				button->Clicked.AttachMethod(this, &GuiTabTemplate_StyleProvider::OnHeaderButtonClicked);

				GuiStackItemComposition* item=new GuiStackItemComposition;
				item->AddChild(button->GetBoundsComposition());
				tabHeaderComposition->InsertStackItem(index, item);
				headerButtons.Insert(index, button);

				GuiToolstripButton* menuItem=new GuiToolstripButton(CreateMenuItemTemplate());
				menuItem->Clicked.AttachMethod(this, &GuiTabTemplate_StyleProvider::OnHeaderOverflowMenuButtonClicked);
				headerOverflowMenu->GetToolstripItems().Insert(index, menuItem);

				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::SetTabText(vint index, const WString& value)
			{
				headerButtons[index]->SetText(value);
				headerOverflowMenu->GetToolstripItems().Get(index)->SetText(value);
				
				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::RemoveTab(vint index)
			{
				GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(index);
				GuiSelectableButton* button=headerButtons[index];

				tabHeaderComposition->RemoveChild(item);
				item->RemoveChild(button->GetBoundsComposition());
				headerButtons.RemoveAt(index);

				headerOverflowMenu->GetToolstripItems().RemoveAt(index);
				delete item;
				delete button;
				
				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::MoveTab(vint oldIndex, vint newIndex)
			{
				GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(oldIndex);
				tabHeaderComposition->RemoveChild(item);
				tabHeaderComposition->InsertStackItem(newIndex, item);

				GuiSelectableButton* button=headerButtons[oldIndex];
				headerButtons.RemoveAt(oldIndex);
				headerButtons.Insert(newIndex, button);
				
				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::SetSelectedTab(vint index)
			{
				headerButtons[index]->SetSelected(true);
				
				UpdateHeaderLayout();
			}

			void GuiTabTemplate_StyleProvider::SetTabAlt(vint index, const WString& value, compositions::IGuiAltActionHost* host)
			{
				auto button = headerButtons[index];
				button->SetAlt(value);
				button->SetActivatingAltHost(host);
			}

			compositions::IGuiAltAction* GuiTabTemplate_StyleProvider::GetTabAltAction(vint index)
			{
				return headerButtons[index]->QueryTypedService<IGuiAltAction>();
			}

/***********************************************************************
GuiControlTemplate_ItemStyleProvider
***********************************************************************/

			GuiControlTemplate_ItemStyleProvider::GuiControlTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory)
				:factory(_factory)
			{
			}

			GuiControlTemplate_ItemStyleProvider::~GuiControlTemplate_ItemStyleProvider()
			{
			}

			void GuiControlTemplate_ItemStyleProvider::AttachComboBox(controls::GuiComboBoxListControl* value)
			{
			}

			void GuiControlTemplate_ItemStyleProvider::DetachComboBox()
			{
			}

			controls::GuiControl::IStyleController* GuiControlTemplate_ItemStyleProvider::CreateItemStyle(description::Value item)
			{
				return new GuiControlTemplate_StyleProvider(factory, item);
			}

/***********************************************************************
GuiTextListItemTemplate_ItemStyleProvider
***********************************************************************/

			GuiTextListItemTemplate_ItemStyleProvider::GuiTextListItemTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory)
				:factory(_factory)
				, listControl(0)
				, bindingView(0)
			{
			}

			GuiTextListItemTemplate_ItemStyleProvider::~GuiTextListItemTemplate_ItemStyleProvider()
			{
			}

			void GuiTextListItemTemplate_ItemStyleProvider::AttachListControl(controls::GuiListControl* value)
			{
				listControl = dynamic_cast<GuiVirtualTextList*>(value);
				bindingView = dynamic_cast<GuiListControl::IItemBindingView*>(listControl->GetItemProvider()->RequestView(GuiListControl::IItemBindingView::Identifier));
			}

			void GuiTextListItemTemplate_ItemStyleProvider::DetachListControl()
			{
				if (listControl && bindingView)
				{
					listControl->GetItemProvider()->ReleaseView(bindingView);
				}
				listControl = 0;
				bindingView = 0;
			}

			vint GuiTextListItemTemplate_ItemStyleProvider::GetItemStyleId(vint itemIndex)
			{
				return 0;
			}

			controls::GuiListControl::IItemStyleController* GuiTextListItemTemplate_ItemStyleProvider::CreateItemStyle(vint styleId)
			{
				return new GuiTextListItemTemplate_ItemStyleController(this);
			}

			void GuiTextListItemTemplate_ItemStyleProvider::DestroyItemStyle(controls::GuiListControl::IItemStyleController* style)
			{
				delete style;
			}

			void GuiTextListItemTemplate_ItemStyleProvider::Install(controls::GuiListControl::IItemStyleController* style, vint itemIndex)
			{
				if (auto controller = dynamic_cast<GuiTextListItemTemplate_ItemStyleController*>(style))
				{
					Value viewModel;
					if (bindingView)
					{
						viewModel = bindingView->GetBindingValue(itemIndex);
					}
					
					GuiTemplate* itemTemplate = factory->CreateTemplate(viewModel);
					if (auto listItemTemplate = dynamic_cast<GuiTextListItemTemplate*>(itemTemplate))
					{
						listItemTemplate->SetFont(listControl->GetFont());
						listItemTemplate->SetIndex(itemIndex);
						listItemTemplate->SetTextColor(listControl->GetTextListStyleProvider()->GetTextColor());
						controller->SetTemplate(listItemTemplate);
					}
					else
					{
						delete itemTemplate;
					}
				}
			}

			void GuiTextListItemTemplate_ItemStyleProvider::SetStyleIndex(controls::GuiListControl::IItemStyleController* style, vint value)
			{
				if (auto controller = dynamic_cast<GuiTextListItemTemplate_ItemStyleController*>(style))
				{
					if (auto itemTemplate = controller->GetTemplate())
					{
						itemTemplate->SetIndex(value);
					}
				}
			}

			void GuiTextListItemTemplate_ItemStyleProvider::SetStyleSelected(controls::GuiListControl::IItemStyleController* style, bool value)
			{
				if (auto controller = dynamic_cast<GuiTextListItemTemplate_ItemStyleController*>(style))
				{
					controller->backgroundButton->SetSelected(value);
					if (auto itemTemplate = controller->GetTemplate())
					{
						itemTemplate->SetSelected(value);
					}
				}
			}

/***********************************************************************
GuiTextListItemTemplate_ItemStyleController
***********************************************************************/

			GuiTextListItemTemplate_ItemStyleController::GuiTextListItemTemplate_ItemStyleController(GuiTextListItemTemplate_ItemStyleProvider* _itemStyleProvider)
				:itemStyleProvider(_itemStyleProvider)
				, itemTemplate(0)
				, installed(false)
				, backgroundButton(0)
			{
				backgroundButton = new GuiSelectableButton(itemStyleProvider->listControl->GetTextListStyleProvider()->CreateItemBackground());
				backgroundButton->SetAutoSelection(false);
			}

			GuiTextListItemTemplate_ItemStyleController::~GuiTextListItemTemplate_ItemStyleController()
			{
				SafeDeleteControl(backgroundButton);
			}

			GuiTextListItemTemplate* GuiTextListItemTemplate_ItemStyleController::GetTemplate()
			{
				return itemTemplate;
			}

			void GuiTextListItemTemplate_ItemStyleController::SetTemplate(GuiTextListItemTemplate* _itemTemplate)
			{
				SafeDeleteComposition(itemTemplate);
				itemTemplate = _itemTemplate;
				itemTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
				backgroundButton->GetContainerComposition()->AddChild(itemTemplate);
			}

			controls::GuiListControl::IItemStyleProvider* GuiTextListItemTemplate_ItemStyleController::GetStyleProvider()
			{
				return itemStyleProvider;
			}

			vint GuiTextListItemTemplate_ItemStyleController::GetItemStyleId()
			{
				return 0;
			}

			compositions::GuiBoundsComposition* GuiTextListItemTemplate_ItemStyleController::GetBoundsComposition()
			{
				return backgroundButton->GetBoundsComposition();
			}

			bool GuiTextListItemTemplate_ItemStyleController::IsCacheable()
			{
				return false;
			}

			bool GuiTextListItemTemplate_ItemStyleController::IsInstalled()
			{
				return installed;
			}

			void GuiTextListItemTemplate_ItemStyleController::OnInstalled()
			{
				installed = true;
			}

			void GuiTextListItemTemplate_ItemStyleController::OnUninstalled()
			{
				installed = false;
			}

/***********************************************************************
GuiTreeItemTemplate_ItemStyleProvider
***********************************************************************/

			void GuiTreeItemTemplate_ItemStyleProvider::UpdateExpandingButton(controls::tree::INodeProvider* node)
			{
				vint index=treeListControl->GetNodeItemView()->CalculateNodeVisibilityIndex(node);
				if(index!=-1)
				{
					if(auto style = treeListControl->GetArranger()->GetVisibleStyle(index))
					{
						if (auto controller = dynamic_cast<GuiTreeItemTemplate_ItemStyleController*>(style))
						{
							if (auto itemTemplate = dynamic_cast<GuiTreeItemTemplate*>(controller->GetTemplate()))
							{
								itemTemplate->SetExpanding(node->GetExpanding());
							}
						}
					}
				}
			}

			void GuiTreeItemTemplate_ItemStyleProvider::OnAttached(controls::tree::INodeRootProvider* provider)
			{
			}

			void GuiTreeItemTemplate_ItemStyleProvider::OnBeforeItemModified(controls::tree::INodeProvider* parentNode, vint start, vint count, vint newCount)
			{
			}

			void GuiTreeItemTemplate_ItemStyleProvider::OnAfterItemModified(controls::tree::INodeProvider* parentNode, vint start, vint count, vint newCount)
			{
				UpdateExpandingButton(parentNode);
			}

			void GuiTreeItemTemplate_ItemStyleProvider::OnItemExpanded(controls::tree::INodeProvider* node)
			{
				UpdateExpandingButton(node);
			}

			void GuiTreeItemTemplate_ItemStyleProvider::OnItemCollapsed(controls::tree::INodeProvider* node)
			{
				UpdateExpandingButton(node);
			}

			GuiTreeItemTemplate_ItemStyleProvider::GuiTreeItemTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory)
				:factory(_factory)
				, treeListControl(0)
				, bindingView(0)
				, itemStyleProvider(0)
			{

			}

			GuiTreeItemTemplate_ItemStyleProvider::~GuiTreeItemTemplate_ItemStyleProvider()
			{
			}
				
			void GuiTreeItemTemplate_ItemStyleProvider::BindItemStyleProvider(controls::GuiListControl::IItemStyleProvider* styleProvider)
			{
				itemStyleProvider = styleProvider;
			}

			controls::GuiListControl::IItemStyleProvider* GuiTreeItemTemplate_ItemStyleProvider::GetBindedItemStyleProvider()
			{
				return itemStyleProvider;
			}

			void GuiTreeItemTemplate_ItemStyleProvider::AttachListControl(GuiListControl* value)
			{
				treeListControl = dynamic_cast<GuiVirtualTreeListControl*>(value);
				if (treeListControl)
				{
					treeListControl->GetNodeRootProvider()->AttachCallback(this);
					bindingView = dynamic_cast<tree::INodeItemBindingView*>(treeListControl->GetNodeRootProvider()->RequestView(tree::INodeItemBindingView::Identifier));
				}
			}

			void GuiTreeItemTemplate_ItemStyleProvider::DetachListControl()
			{
				if (treeListControl)
				{
					treeListControl->GetNodeRootProvider()->DetachCallback(this);
					if (bindingView)
					{
						treeListControl->GetNodeRootProvider()->ReleaseView(bindingView);
					}
				}
				treeListControl = 0;
				bindingView = 0;
			}

			vint GuiTreeItemTemplate_ItemStyleProvider::GetItemStyleId(controls::tree::INodeProvider* node)
			{
				return 0;
			}

			controls::tree::INodeItemStyleController* GuiTreeItemTemplate_ItemStyleProvider::CreateItemStyle(vint styleId)
			{
				return new GuiTreeItemTemplate_ItemStyleController(this);
			}

			void GuiTreeItemTemplate_ItemStyleProvider::DestroyItemStyle(controls::tree::INodeItemStyleController* style)
			{
				delete style;
			}

			void GuiTreeItemTemplate_ItemStyleProvider::Install(controls::tree::INodeItemStyleController* style, controls::tree::INodeProvider* node, vint itemIndex)
			{
				if (auto controller = dynamic_cast<GuiTreeItemTemplate_ItemStyleController*>(style))
				{
					Value viewModel;
					if (bindingView)
					{
						viewModel = bindingView->GetBindingValue(node);
					}
					
					GuiTemplate* itemTemplate = factory->CreateTemplate(viewModel);
					if (auto treeItemTemplate = dynamic_cast<GuiTreeItemTemplate*>(itemTemplate))
					{
						treeItemTemplate->SetFont(treeListControl->GetFont());
						treeItemTemplate->SetIndex(itemIndex);
						controller->SetTemplate(treeItemTemplate);
					}
					else
					{
						delete itemTemplate;
					}
				}
			}

			void GuiTreeItemTemplate_ItemStyleProvider::SetStyleIndex(controls::tree::INodeItemStyleController* style, vint value)
			{
				if (auto controller = dynamic_cast<GuiTreeItemTemplate_ItemStyleController*>(style))
				{
					if (auto itemTemplate = controller->GetTemplate())
					{
						itemTemplate->SetIndex(value);
					}
				}
			}

			void GuiTreeItemTemplate_ItemStyleProvider::SetStyleSelected(controls::tree::INodeItemStyleController* style, bool value)
			{
				if (auto controller = dynamic_cast<GuiTreeItemTemplate_ItemStyleController*>(style))
				{
					if (auto itemTemplate = controller->GetTemplate())
					{
						itemTemplate->SetSelected(value);
					}
				}
			}

/***********************************************************************
GuiTreeItemTemplate_ItemStyleController
***********************************************************************/

			GuiTreeItemTemplate_ItemStyleController::GuiTreeItemTemplate_ItemStyleController(GuiTreeItemTemplate_ItemStyleProvider* _nodeStyleProvider)
				:nodeStyleProvider(_nodeStyleProvider)
				, itemTemplate(0)
				, installed(false)
			{
			}

			GuiTreeItemTemplate_ItemStyleController::~GuiTreeItemTemplate_ItemStyleController()
			{
			}

			GuiTreeItemTemplate* GuiTreeItemTemplate_ItemStyleController::GetTemplate()
			{
				return itemTemplate;
			}

			void GuiTreeItemTemplate_ItemStyleController::SetTemplate(GuiTreeItemTemplate* _itemTemplate)
			{
				SafeDeleteComposition(itemTemplate);
				itemTemplate = _itemTemplate;
			}

			controls::GuiListControl::IItemStyleProvider* GuiTreeItemTemplate_ItemStyleController::GetStyleProvider()
			{
				return nodeStyleProvider->GetBindedItemStyleProvider();
			}

			vint GuiTreeItemTemplate_ItemStyleController::GetItemStyleId()
			{
				return 0;
			}

			compositions::GuiBoundsComposition* GuiTreeItemTemplate_ItemStyleController::GetBoundsComposition()
			{
				return itemTemplate;
			}

			bool GuiTreeItemTemplate_ItemStyleController::IsCacheable()
			{
				return false;
			}

			bool GuiTreeItemTemplate_ItemStyleController::IsInstalled()
			{
				return installed;
			}

			void GuiTreeItemTemplate_ItemStyleController::OnInstalled()
			{
				installed = true;
			}

			void GuiTreeItemTemplate_ItemStyleController::OnUninstalled()
			{
				installed = false;
			}

			controls::tree::INodeItemStyleProvider* GuiTreeItemTemplate_ItemStyleController::GetNodeStyleProvider()
			{
				return nodeStyleProvider;
			}

/***********************************************************************
GuiBindableDataVisualizer::Factory
***********************************************************************/

			GuiBindableDataVisualizer::Factory::Factory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn)
				:templateFactory(_templateFactory)
				, ownerColumn(_ownerColumn)
			{
			}

			GuiBindableDataVisualizer::Factory::~Factory()
			{
			}

			Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::Factory::CreateVisualizer(const FontProperties& font, controls::GuiListViewBase::IStyleProvider* styleProvider)
			{
				auto visualizer = DataVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(font, styleProvider).Cast<GuiBindableDataVisualizer>();
				if (visualizer)
				{
					visualizer->templateFactory = templateFactory;
					visualizer->ownerColumn = ownerColumn;
				}
				return visualizer;
			}

/***********************************************************************
GuiBindableDataVisualizer::DecoratedFactory
***********************************************************************/

			GuiBindableDataVisualizer::DecoratedFactory::DecoratedFactory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn, Ptr<controls::list::IDataVisualizerFactory> _decoratedFactory)
				:DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>(_decoratedFactory)
				, templateFactory(_templateFactory)
				, ownerColumn(_ownerColumn)
			{
			}

			GuiBindableDataVisualizer::DecoratedFactory::~DecoratedFactory()
			{
			}

			Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::DecoratedFactory::CreateVisualizer(const FontProperties& font, controls::GuiListViewBase::IStyleProvider* styleProvider)
			{
				auto visualizer = DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(font, styleProvider).Cast<GuiBindableDataVisualizer>();
				if (visualizer)
				{
					visualizer->templateFactory = templateFactory;
					visualizer->ownerColumn = ownerColumn;
				}
				return visualizer;
			}

/***********************************************************************
GuiBindableDataVisualizer
***********************************************************************/

			compositions::GuiBoundsComposition* GuiBindableDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
			{
				GuiTemplate* itemTemplate = templateFactory->CreateTemplate(ownerColumn->GetViewModelContext());
				if (!(visualizerTemplate = dynamic_cast<GuiGridVisualizerTemplate*>(itemTemplate)))
				{
					delete itemTemplate;
					CHECK_FAIL(L"GuiBindableDataVisualizer::CreateBoundsCompositionInternal(presentation::compositions::GuiBoundsComposition*)#An instance of GuiGridVisualizerTemplate is expected.");
				}

				if (decoratedComposition)
				{
					decoratedComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					visualizerTemplate->GetContainerComposition()->AddChild(decoratedComposition);
				}
				visualizerTemplate->SetFont(font);
				return visualizerTemplate;
			}

			GuiBindableDataVisualizer::GuiBindableDataVisualizer()
			{
			}

			GuiBindableDataVisualizer::GuiBindableDataVisualizer(Ptr<controls::list::IDataVisualizer> _decoratedVisualizer)
				:DataVisualizerBase(_decoratedVisualizer)
			{
			}

			GuiBindableDataVisualizer::~GuiBindableDataVisualizer()
			{
			}

			void GuiBindableDataVisualizer::BeforeVisualizeCell(controls::list::IDataProvider* dataProvider, vint row, vint column)
			{
				DataVisualizerBase::BeforeVisualizeCell(dataProvider, row, column);
				if (!visualizerTemplate) return;
				visualizerTemplate->SetText(dataProvider->GetCellText(row, column));

				auto structuredDataProvider = dynamic_cast<list::StructuredDataProvider*>(dataProvider);
				if (!structuredDataProvider) return;

				auto bindableDataProvider = structuredDataProvider->GetStructuredDataProvider().Cast<list::BindableDataProvider>();
				if (!bindableDataProvider) return;

				auto columnProvider = bindableDataProvider->GetBindableColumn(column);
				if (!columnProvider) return;

				visualizerTemplate->SetRowValue(bindableDataProvider->GetRowValue(row));
				visualizerTemplate->SetCellValue(columnProvider->GetCellValue(row));
			}

			void GuiBindableDataVisualizer::SetSelected(bool value)
			{
				DataVisualizerBase::SetSelected(value);
				if (visualizerTemplate)
				{
					visualizerTemplate->SetSelected(value);
				}
			}

/***********************************************************************
GuiBindableDataEditor::Factory
***********************************************************************/

			GuiBindableDataEditor::Factory::Factory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn)
				:templateFactory(_templateFactory)
				, ownerColumn(_ownerColumn)
			{
			}

			GuiBindableDataEditor::Factory::~Factory()
			{
			}

			Ptr<controls::list::IDataEditor> GuiBindableDataEditor::Factory::CreateEditor(controls::list::IDataEditorCallback* callback)
			{
				auto editor = DataEditorFactory<GuiBindableDataEditor>::CreateEditor(callback).Cast<GuiBindableDataEditor>();
				if (editor)
				{
					editor->templateFactory = templateFactory;
					editor->ownerColumn = ownerColumn;

					// Invoke GuiBindableDataEditor::CreateBoundsCompositionInternal
					// so that GuiBindableDataEditor::BeforeEditCell is able to set RowValue and CellValue to the editor
					editor->GetBoundsComposition();
				}
				return editor;
			}

/***********************************************************************
GuiBindableDataEditor
***********************************************************************/

			compositions::GuiBoundsComposition* GuiBindableDataEditor::CreateBoundsCompositionInternal()
			{
				GuiTemplate* itemTemplate = templateFactory->CreateTemplate(ownerColumn->GetViewModelContext());
				if (!(editorTemplate = dynamic_cast<GuiGridEditorTemplate*>(itemTemplate)))
				{
					delete itemTemplate;
					CHECK_FAIL(L"GuiBindableDataEditor::CreateBoundsCompositionInternal()#An instance of GuiGridEditorTemplate is expected.");
				}

				editorTemplate->CellValueChanged.AttachMethod(this, &GuiBindableDataEditor::editorTemplate_CellValueChanged);
				return editorTemplate;
			}

			void GuiBindableDataEditor::editorTemplate_CellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (callback)
				{
					callback->RequestSaveData();
				}
			}

			GuiBindableDataEditor::GuiBindableDataEditor()
			{
			}

			GuiBindableDataEditor::~GuiBindableDataEditor()
			{
			}

			void GuiBindableDataEditor::BeforeEditCell(controls::list::IDataProvider* dataProvider, vint row, vint column)
			{
				DataEditorBase::BeforeEditCell(dataProvider, row, column);
				if (!editorTemplate) return;
				editorTemplate->SetText(dataProvider->GetCellText(row, column));

				auto structuredDataProvider = dynamic_cast<list::StructuredDataProvider*>(dataProvider);
				if (!structuredDataProvider) return;

				auto bindableDataProvider = structuredDataProvider->GetStructuredDataProvider().Cast<list::BindableDataProvider>();
				if (!bindableDataProvider) return;

				auto columnProvider = bindableDataProvider->GetBindableColumn(column);
				if (!columnProvider) return;

				editorTemplate->SetRowValue(bindableDataProvider->GetRowValue(row));
				editorTemplate->SetCellValue(columnProvider->GetCellValue(row));
			}

			description::Value GuiBindableDataEditor::GetEditedCellValue()
			{
				if (editorTemplate)
				{
					return editorTemplate->GetCellValue();
				}
				else
				{
					return description::Value();
				}
			}

/***********************************************************************
Helper Functions
***********************************************************************/

			void SplitBySemicolon(const WString& input, collections::List<WString>& fragments)
			{
				const wchar_t* attValue = input.Buffer();
				while(*attValue)
				{
					// split the value by ';'
					const wchar_t* attSemicolon = wcschr(attValue, L';');
					WString pattern;
					if(attSemicolon)
					{
						pattern = WString(attValue, vint(attSemicolon - attValue));
						attValue = attSemicolon + 1;
					}
					else
					{
						vint len = wcslen(attValue);
						pattern = WString(attValue, len);
						attValue += len;
					}

					fragments.Add(pattern);
				}
			}

			Ptr<GuiTemplate::IFactory> CreateTemplateFactory(const WString& typeValues)
			{
				List<ITypeDescriptor*> types;
				List<WString> typeNames;
				SplitBySemicolon(typeValues, typeNames);
				CopyFrom(
					types,
					From(typeNames)
						.Select<ITypeDescriptor*(*)(const WString&)>(&description::GetTypeDescriptor)
						.Where([](ITypeDescriptor* type){return type != 0; })
					);

				return GuiTemplate::IFactory::CreateTemplateFactory(types);
			}
		}
	}
}