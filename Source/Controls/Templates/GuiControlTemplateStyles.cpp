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
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, BackgroundTemplate);
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
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, BackgroundTemplate);
			}

			controls::GuiSelectableButton::IStyleController* GuiTreeViewTemplate_StyleProvider::CreateItemExpandingDecorator()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, ExpandingDecoratorTemplate);
			}

			Color GuiTreeViewTemplate_StyleProvider::GetTextColor()
			{
				return controlTemplate->GetTextColor();
			}

/***********************************************************************
GuiTabTemplate_StyleProvider
***********************************************************************/

			controls::GuiSelectableButton::IStyleController* GuiTabTemplate_StyleProvider::CreateHeaderTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiSelectableButtonTemplate, HeaderTemplate);
			}

			controls::GuiButton::IStyleController* GuiTabTemplate_StyleProvider::CreateDropdownTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiButtonTemplate, DropdownTemplate);
			}

			controls::GuiMenu::IStyleController* GuiTabTemplate_StyleProvider::CreateMenuTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiMenuTemplate, MenuTemplate);
			}

			controls::GuiToolstripButton::IStyleController* GuiTabTemplate_StyleProvider::CreateMenuItemTemplate()
			{
				GET_FACTORY_FROM_TEMPLATE(GuiToolstripButtonTemplate, MenuItemTemplate);
			}

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

			GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider(TemplateProperty<GuiTabTemplate> factory)
				:GuiControlTemplate_StyleProvider(factory)
			{
				if (!(controlTemplate = dynamic_cast<GuiTabTemplate*>(GetBoundsComposition())))
				{
					CHECK_FAIL(L"GuiTabTemplate_StyleProvider::GuiTabTemplate_StyleProvider()#An instance of GuiTabTemplate is expected.");
				}
				Initialize();
			}

			GuiTabTemplate_StyleProvider::~GuiTabTemplate_StyleProvider()
			{
				delete headerOverflowMenu;
			}

			void GuiTabTemplate_StyleProvider::SetCommandExecutor(controls::ITabCommandExecutor* value)
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
GuiBindableDataVisualizer::Factory
***********************************************************************/

			GuiBindableDataVisualizer::Factory::Factory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory)
				:templateFactory(_templateFactory)
			{
			}

			GuiBindableDataVisualizer::Factory::~Factory()
			{
			}

			Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::Factory::CreateVisualizer(controls::list::IDataGridContext* dataGridContext)
			{
				auto visualizer = DataVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(dataGridContext).Cast<GuiBindableDataVisualizer>();
				if (visualizer)
				{
					visualizer->templateFactory = templateFactory;
				}
				return visualizer;
			}

/***********************************************************************
GuiBindableDataVisualizer::DecoratedFactory
***********************************************************************/

			GuiBindableDataVisualizer::DecoratedFactory::DecoratedFactory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory, Ptr<controls::list::IDataVisualizerFactory> _decoratedFactory)
				:DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>(_decoratedFactory)
				, templateFactory(_templateFactory)
			{
			}

			GuiBindableDataVisualizer::DecoratedFactory::~DecoratedFactory()
			{
			}

			Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::DecoratedFactory::CreateVisualizer(controls::list::IDataGridContext* dataGridContext)
			{
				auto visualizer = DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(dataGridContext).Cast<GuiBindableDataVisualizer>();
				if (visualizer)
				{
					visualizer->templateFactory = templateFactory;
				}
				return visualizer;
			}

/***********************************************************************
GuiBindableDataVisualizer
***********************************************************************/

			GuiTemplate* GuiBindableDataVisualizer::CreateTemplateInternal(GuiTemplate* childTemplate)
			{
				visualizerTemplate = templateFactory(dataGridContext->GetViewModelContext());
				CHECK_ERROR(visualizerTemplate, L"GuiBindableDataVisualizer::CreateTemplateInternal(GuiTemplate*)#An instance of GuiGridVisualizerTemplate is expected.");

				if (childTemplate)
				{
					childTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
					visualizerTemplate->GetContainerComposition()->AddChild(childTemplate);
				}
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

			void GuiBindableDataVisualizer::BeforeVisualizeCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)
			{
				DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);
				auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier));
				if (listViewItemView)
				{
					visualizerTemplate->SetText(column == 0 ? listViewItemView->GetText(row) : listViewItemView->GetSubItem(row, column - 1));
				}
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

			GuiBindableDataEditor::Factory::Factory(TemplateProperty<GuiGridEditorTemplate> _templateFactory)
				:templateFactory(_templateFactory)
			{
			}

			GuiBindableDataEditor::Factory::~Factory()
			{
			}

			Ptr<controls::list::IDataEditor> GuiBindableDataEditor::Factory::CreateEditor(controls::list::IDataGridContext* dataGridContext)
			{
				auto editor = DataEditorFactory<GuiBindableDataEditor>::CreateEditor(dataGridContext).Cast<GuiBindableDataEditor>();
				if (editor)
				{
					editor->templateFactory = templateFactory;

					// Invoke GuiBindableDataEditor::CreateTemplateInternal
					// so that GuiBindableDataEditor::BeforeEditCell is able to set RowValue and CellValue to the editor
					editor->GetTemplate();
				}
				return editor;
			}

/***********************************************************************
GuiBindableDataEditor
***********************************************************************/

			GuiTemplate* GuiBindableDataEditor::CreateTemplateInternal()
			{
				editorTemplate = templateFactory(dataGridContext->GetViewModelContext());
				CHECK_ERROR(editorTemplate, L"GuiBindableDataEditor::CreateTemplateInternal()#An instance of GuiGridEditorTemplate is expected.");

				editorTemplate->CellValueChanged.AttachMethod(this, &GuiBindableDataEditor::editorTemplate_CellValueChanged);
				return editorTemplate;
			}

			void GuiBindableDataEditor::editorTemplate_CellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				dataGridContext->RequestSaveData();
			}

			GuiBindableDataEditor::GuiBindableDataEditor()
			{
			}

			GuiBindableDataEditor::~GuiBindableDataEditor()
			{
			}

			void GuiBindableDataEditor::BeforeEditCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)
			{
				DataEditorBase::BeforeEditCell(itemProvider, row, column);
				auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier));
				if (listViewItemView)
				{
					editorTemplate->SetText(column == 0 ? listViewItemView->GetText(row) : listViewItemView->GetSubItem(row, column - 1));
				}
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
		}
	}
}

#undef GET_FACTORY_FROM_TEMPLATE
#undef GET_FACTORY_FROM_TEMPLATE_OPT