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

#define GET_FACTORY_FROM_TEMPLATE(TEMPLATE, PROPERTY)\
	return new TEMPLATE##_StyleProvider(controlTemplate->Get##PROPERTY());\

#define GET_FACTORY_FROM_TEMPLATE_OPT(TEMPLATE, PROPERTY)\
	if (!controlTemplate->Get##PROPERTY())\
	{\
		return nullptr;\
	}\
	GET_FACTORY_FROM_TEMPLATE(TEMPLATE, PROPERTY)\

/***********************************************************************
GuiControlTemplate_StyleProvider
***********************************************************************/

			GuiControlTemplate_StyleProvider::GuiControlTemplate_StyleProvider(TemplateProperty<GuiControlTemplate> factory, description::Value viewModel)
				:associatedStyleController(0)
				, controlTemplate(factory(viewModel))
			{
				CHECK_ERROR(controlTemplate, L"GuiControlTemplate_StyleProvider::GuiControlTemplate_StyleProvider()#An instance of GuiControlTemplate is expected.");
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

			GuiLabelTemplate_StyleProvider::GuiLabelTemplate_StyleProvider(TemplateProperty<GuiLabelTemplate> factory)
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

			GuiSinglelineTextBoxTemplate_StyleProvider::GuiSinglelineTextBoxTemplate_StyleProvider(TemplateProperty<GuiSinglelineTextBoxTemplate> factory)
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

			GuiDocumentLabelTemplate_StyleProvider::GuiDocumentLabelTemplate_StyleProvider(TemplateProperty<GuiDocumentLabelTemplate> factory)
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
			
			Color GuiDocumentLabelTemplate_StyleProvider::GetCaretColor()
			{
				return controlTemplate->GetCaretColor();
			}

/***********************************************************************
GuiMenuTemplate_StyleProvider
***********************************************************************/

			GuiMenuTemplate_StyleProvider::GuiMenuTemplate_StyleProvider(TemplateProperty<GuiMenuTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
			}

			GuiMenuTemplate_StyleProvider::~GuiMenuTemplate_StyleProvider()
			{
			}

/***********************************************************************
GuiWindowTemplate_StyleProvider
***********************************************************************/

			GuiWindowTemplate_StyleProvider::GuiWindowTemplate_StyleProvider(TemplateProperty<GuiWindowTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
				, window(0)
			{
				if (!(controlTemplate = dynamic_cast<GuiWindowTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiWindowTemplate_StyleProvider::GuiWindowTemplate_StyleProvider()#An instance of GuiWindowTemplate is expected.");
				}
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
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiWindowTemplate, TooltipTemplate);
			}

			controls::GuiLabel::IStyleController* GuiWindowTemplate_StyleProvider::CreateShortcutKeyStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiLabelTemplate, ShortcutKeyTemplate);
			}

#undef WINDOW_TEMPLATE_GET
#undef WINDOW_TEMPLATE_SET

/***********************************************************************
GuiButtonTemplate_StyleProvider
***********************************************************************/

			GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider(TemplateProperty<GuiButtonTemplate> factory)
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

			void GuiButtonTemplate_StyleProvider::Transfer(controls::ButtonState value)
			{
				controlTemplate->SetState(value);
			}

/***********************************************************************
GuiSelectableButtonTemplate_StyleProvider
***********************************************************************/

			GuiSelectableButtonTemplate_StyleProvider::GuiSelectableButtonTemplate_StyleProvider(TemplateProperty<GuiSelectableButtonTemplate> factory)
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

			GuiToolstripButtonTemplate_StyleProvider::GuiToolstripButtonTemplate_StyleProvider(TemplateProperty<GuiToolstripButtonTemplate> factory)
				:GuiSelectableButtonTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiToolstripButtonTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiButtonTemplate_StyleProvider::GuiButtonTemplate_StyleProvider()#An instance of GuiToolstripButtonTemplate is expected.");
				}
			}

			GuiToolstripButtonTemplate_StyleProvider::~GuiToolstripButtonTemplate_StyleProvider()
			{
			}
				
			controls::GuiMenu::IStyleController* GuiToolstripButtonTemplate_StyleProvider::CreateSubMenuStyleController()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiMenuTemplate, SubMenuTemplate);
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

			GuiListViewColumnHeaderTemplate_StyleProvider::GuiListViewColumnHeaderTemplate_StyleProvider(TemplateProperty<GuiListViewColumnHeaderTemplate> factory)
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

			void GuiListViewColumnHeaderTemplate_StyleProvider::SetColumnSortingState(controls::ColumnSortingState value)
			{
				controlTemplate->SetSortingState(value);
			}

/***********************************************************************
GuiComboBoxTemplate_StyleProvider
***********************************************************************/

			GuiComboBoxTemplate_StyleProvider::GuiComboBoxTemplate_StyleProvider(TemplateProperty<GuiComboBoxTemplate> factory)
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

			void GuiComboBoxTemplate_StyleProvider::SetCommandExecutor(controls::IComboBoxCommandExecutor* value)
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
GuiScrollTemplate_StyleProvider
***********************************************************************/

			GuiScrollTemplate_StyleProvider::GuiScrollTemplate_StyleProvider(TemplateProperty<GuiScrollTemplate> factory)
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

			void GuiScrollTemplate_StyleProvider::SetCommandExecutor(controls::IScrollCommandExecutor* value)
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

			GuiScrollViewTemplate_StyleProvider::GuiScrollViewTemplate_StyleProvider(TemplateProperty<GuiScrollViewTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiScrollViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiScrollViewTemplate_StyleProvider::GuiScrollViewTemplate_StyleProvider()#An instance of GuiScrollViewTemplate is expected.");
				}
			}

			GuiScrollViewTemplate_StyleProvider::~GuiScrollViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiScroll::IStyleController* GuiScrollViewTemplate_StyleProvider::CreateHorizontalScrollStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiScrollTemplate, HScrollTemplate);
			}

			controls::GuiScroll::IStyleController* GuiScrollViewTemplate_StyleProvider::CreateVerticalScrollStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiScrollTemplate, VScrollTemplate);
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

			GuiMultilineTextBoxTemplate_StyleProvider::GuiMultilineTextBoxTemplate_StyleProvider(TemplateProperty<GuiMultilineTextBoxTemplate> factory)
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

			GuiDocumentViewerTemplate_StyleProvider::GuiDocumentViewerTemplate_StyleProvider(TemplateProperty<GuiDocumentViewerTemplate> factory)
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

			Color GuiDocumentViewerTemplate_StyleProvider::GetCaretColor()
			{
				return controlTemplate->GetCaretColor();
			}

/***********************************************************************
GuiTextListTemplate_StyleProvider
***********************************************************************/

			GuiTextListTemplate_StyleProvider::GuiTextListTemplate_StyleProvider(TemplateProperty<GuiTextListTemplate> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTextListTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTextListTemplate_StyleProvider::GuiTextListTemplate_StyleProvider()#An instance of GuiTextListTemplate is expected.");
				}
			}

			GuiTextListTemplate_StyleProvider::~GuiTextListTemplate_StyleProvider()
			{
			}

			Color GuiTextListTemplate_StyleProvider::GetTextColor()
			{
				return controlTemplate->GetTextColor();
			}
			
			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, BackgroundTemplate);
			}

			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::CreateCheckBulletStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, CheckBulletTemplate);
			}

			controls::GuiSelectableButton::IStyleController* GuiTextListTemplate_StyleProvider::CreateRadioBulletStyle()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, RadioBulletTemplate);
			}

/***********************************************************************
GuiListViewTemplate_StyleProvider
***********************************************************************/

			GuiListViewTemplate_StyleProvider::GuiListViewTemplate_StyleProvider(TemplateProperty<GuiListViewTemplate> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiListViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiListViewTemplate_StyleProvider::GuiListViewTemplate_StyleProvider()#An instance of GuiListViewTemplate is expected.");
				}
			}

			GuiListViewTemplate_StyleProvider::~GuiListViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiSelectableButton::IStyleController* GuiListViewTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, BackgroundTemplate);
			}

			controls::GuiListViewColumnHeader::IStyleController* GuiListViewTemplate_StyleProvider::CreateColumnStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiListViewColumnHeaderTemplate, ColumnHeaderTemplate);
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

			GuiTreeViewTemplate_StyleProvider::GuiTreeViewTemplate_StyleProvider(TemplateProperty<GuiTreeViewTemplate> factory)
				:GuiScrollViewTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTreeViewTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTreeViewTemplate_StyleProvider::GuiTreeViewTemplate_StyleProvider()#An instance of GuiTreeViewTemplate is expected.");
				}
			}

			GuiTreeViewTemplate_StyleProvider::~GuiTreeViewTemplate_StyleProvider()
			{
			}
				
			controls::GuiSelectableButton::IStyleController* GuiTreeViewTemplate_StyleProvider::CreateItemBackground()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, BackgroundTemplate);
			}

			controls::GuiSelectableButton::IStyleController* GuiTreeViewTemplate_StyleProvider::CreateItemExpandingDecorator()
			{
				GET_FACTORY_FROM_TEMPLATE_OPT(GuiSelectableButtonTemplate, ExpandingDecoratorTemplate);
			}

			Color GuiTreeViewTemplate_StyleProvider::GetTextColor()
			{
				return controlTemplate->GetTextColor();
			}

/***********************************************************************
GuiTabTemplate_StyleProvider
***********************************************************************/

			GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider(TemplateProperty<GuiTabTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTabTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider()#An instance of GuiTabTemplate is expected.");
				}
			}

			GuiTabTemplate_StyleProvider::~GuiTabTemplate_StyleProvider()
			{
			}

			void GuiTabTemplate_StyleProvider::SetCommandExecutor(controls::ITabCommandExecutor* value)
			{
				controlTemplate->SetCommands(value);
			}

			void GuiTabTemplate_StyleProvider::SetTabPages(Ptr<reflection::description::IValueObservableList> value)
			{
				controlTemplate->SetTabPages(value);
			}

			void GuiTabTemplate_StyleProvider::SetSelectedTabPage(controls::GuiTabPage* value)
			{
				controlTemplate->SetSelectedTabPage(value);
			}

/***********************************************************************
GuiDatePickerTemplate_StyleProvider
***********************************************************************/

			GuiDatePickerTemplate_StyleProvider::GuiDatePickerTemplate_StyleProvider(TemplateProperty<GuiDatePickerTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDatePickerTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDatePickerTemplate_StyleProvider::GuiDatePickerTemplate_StyleProvider()#An instance of GuiDatePickerTemplate is expected.");
				}
			}

			GuiDatePickerTemplate_StyleProvider::~GuiDatePickerTemplate_StyleProvider()
			{
				delete controlTemplate;
			}

			controls::GuiSelectableButton::IStyleController* GuiDatePickerTemplate_StyleProvider::CreateDateButtonStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, DateButtonTemplate);
			}

			GuiTextListTemplate_StyleProvider* GuiDatePickerTemplate_StyleProvider::CreateTextListStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiTextListTemplate, DateTextListTemplate);
			}

			controls::GuiTextList* GuiDatePickerTemplate_StyleProvider::CreateTextList()
			{
				auto style = CreateTextListStyle();
				return new GuiTextList(style);
			}

			controls::GuiComboBoxListControl::IStyleController* GuiDatePickerTemplate_StyleProvider::CreateComboBoxStyle()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiComboBoxTemplate, DateComboBoxTemplate);
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

			GuiDateComboBoxTemplate_StyleProvider::GuiDateComboBoxTemplate_StyleProvider(TemplateProperty<GuiDateComboBoxTemplate> factory)
				:GuiComboBoxTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiDateComboBoxTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiDateComboBoxTemplate_StyleProvider::GuiDateComboBoxTemplate_StyleProvider()#An instance of GuiDateComboBoxTemplate is expected.");
				}
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
				GET_FACTORY_FROM_TEMPLATE(GuiDatePickerTemplate, DatePickerTemplate);
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
		}
	}
}

#undef GET_FACTORY_FROM_TEMPLATE
#undef GET_FACTORY_FROM_TEMPLATE_OPT