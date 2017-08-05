#include "GuiThemeStyleFactory.h"
#include "../Templates/GuiControlTemplateStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{
			using namespace collections;
			using namespace controls;
			using namespace templates;

			class Theme : public Object, public virtual theme::ITheme
			{
			public:
				Dictionary<WString, Ptr<ThemeTemplates>>	templates;
				ThemeTemplates*								first = nullptr;
				ThemeTemplates*								last = nullptr;

				bool RegisterTheme(const WString& name, Ptr<ThemeTemplates> theme)
				{
					CHECK_ERROR(theme->previous == nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme object has been registered");
					CHECK_ERROR(theme->next == nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme object has been registered");

					if (templates.Keys().Contains(name))
					{
						return false;
					}
					templates.Add(name, theme);

					if (last)
					{
						last->next = theme.Obj();
					}
					theme->previous = last;
					last = theme.Obj();

					return true;
				}

				Ptr<ThemeTemplates> UnregisterTheme(const WString& name)
				{
					vint index = templates.Keys().IndexOf(name);
					if (index == -1)
					{
						return nullptr;
					}

					auto themeTemplates = templates.Values().Get(index);

					if (themeTemplates->previous)
					{
						themeTemplates->previous->next = themeTemplates->next;
					}
					else
					{
						first = themeTemplates->next;
					}

					if (themeTemplates->next)
					{
						themeTemplates->next->previous = themeTemplates->previous;

					}
					else
					{
						last = themeTemplates->previous;
					}

					templates.Remove(name);
					return themeTemplates;
				}

#define SP(TYPE, NAME)\
	auto current = last;\
	while (current) \
	{\
		if (current->NAME)\
		{\
			return new TYPE##_StyleProvider([=](auto _)\
			{\
				return current->NAME(description::Value());\
			});\
		}\
		current = current->previous;\
	}\
	throw Exception(L"Control template for \"" L ## #NAME L"\" is not defined.");\

				GuiWindow::IStyleController* CreateWindowStyle()override
				{
					SP(GuiWindowTemplate, window);
				}

				GuiCustomControl::IStyleController* CreateCustomControlStyle()override
				{
					SP(GuiControlTemplate, customControl);
				}

				GuiTooltip::IStyleController* CreateTooltipStyle()override
				{
					SP(GuiWindowTemplate, tooltip);
				}

				GuiLabel::IStyleController* CreateLabelStyle()override
				{
					SP(GuiLabelTemplate, label);
				}

				GuiLabel::IStyleController* CreateShortcutKeyStyle()override
				{
					SP(GuiLabelTemplate, shortcutKey);
				}

				GuiScrollContainer::IStyleProvider* CreateScrollContainerStyle()override
				{
					SP(GuiScrollViewTemplate, scrollView);
				}

				GuiControl::IStyleController* CreateGroupBoxStyle()override
				{
					SP(GuiControlTemplate, groupBox);
				}

				GuiTab::IStyleController* CreateTabStyle()override
				{
					SP(GuiTabTemplate, tab);
				}

				GuiComboBoxListControl::IStyleController* CreateComboBoxStyle()override
				{
					SP(GuiComboBoxTemplate, comboBox);
				}

				GuiScrollView::IStyleProvider* CreateMultilineTextBoxStyle()override
				{
					SP(GuiMultilineTextBoxTemplate, multilineTextBox);
				}

				GuiSinglelineTextBox::IStyleProvider* CreateTextBoxStyle()override
				{
					SP(GuiSinglelineTextBoxTemplate, singlelineTextBox);
				}

				GuiDocumentViewer::IStyleProvider* CreateDocumentViewerStyle()override
				{
					SP(GuiDocumentViewerTemplate, documentViewer);
				}

				GuiDocumentLabel::IStyleController* CreateDocumentLabelStyle()override
				{
					SP(GuiDocumentLabelTemplate, documentLabel);
				}

				GuiDocumentLabel::IStyleController* CreateDocumentTextBoxStyle()override
				{
					SP(GuiDocumentLabelTemplate, documentTextBox);
				}

				GuiListView::IStyleProvider* CreateListViewStyle()override
				{
					SP(GuiListViewTemplate, listView);
				}

				GuiTreeView::IStyleProvider* CreateTreeViewStyle()override
				{
					SP(GuiTreeViewTemplate, treeView);
				}

				GuiSelectableButton::IStyleController* CreateListItemBackgroundStyle()override
				{
					SP(GuiSelectableButtonTemplate, listItemBackground);
				}

				GuiSelectableButton::IStyleController* CreateTreeItemExpanderStyle()override
				{
					SP(GuiSelectableButtonTemplate, treeItemExpander);
				}

				GuiToolstripMenu::IStyleController* CreateMenuStyle()override
				{
					SP(GuiMenuTemplate, menu);
				}

				GuiToolstripMenuBar::IStyleController* CreateMenuBarStyle()override
				{
					SP(GuiControlTemplate, menuBar);
				}

				GuiControl::IStyleController* CreateMenuSplitterStyle()override
				{
					SP(GuiControlTemplate, menuSplitter);
				}

				GuiToolstripButton::IStyleController* CreateMenuBarButtonStyle()override
				{
					SP(GuiToolstripButtonTemplate, menuBarButton);
				}

				GuiToolstripButton::IStyleController* CreateMenuItemButtonStyle()override
				{
					SP(GuiToolstripButtonTemplate, menuItemButton);
				}

				GuiToolstripToolBar::IStyleController* CreateToolBarStyle()override
				{
					SP(GuiControlTemplate, toolBar);
				}

				GuiToolstripButton::IStyleController* CreateToolBarButtonStyle()override
				{
					SP(GuiToolstripButtonTemplate, toolBarButton);
				}

				GuiToolstripButton::IStyleController* CreateToolBarDropdownButtonStyle()override
				{
					SP(GuiToolstripButtonTemplate, toolBarDropdownButton);
				}

				GuiToolstripButton::IStyleController* CreateToolBarSplitButtonStyle()override
				{
					SP(GuiToolstripButtonTemplate, toolBarSplitButton);
				}

				GuiControl::IStyleController* CreateToolBarSplitterStyle()override
				{
					SP(GuiControlTemplate, toolBarSplitter);
				}

				GuiButton::IStyleController* CreateButtonStyle()override
				{
					SP(GuiButtonTemplate, button);
				}

				GuiSelectableButton::IStyleController* CreateCheckBoxStyle()override
				{
					SP(GuiSelectableButtonTemplate, checkBox);
				}

				GuiSelectableButton::IStyleController* CreateRadioButtonStyle()override
				{
					SP(GuiSelectableButtonTemplate, radioButton);
				}

				GuiDatePicker::IStyleProvider* CreateDatePickerStyle()override
				{
					SP(GuiDatePickerTemplate, datePicker);
				}

				GuiScroll::IStyleController* CreateHScrollStyle()override
				{
					SP(GuiScrollTemplate, hScroll);
				}

				GuiScroll::IStyleController* CreateVScrollStyle()override
				{
					SP(GuiScrollTemplate, vScroll);
				}

				GuiScroll::IStyleController* CreateHTrackerStyle()override
				{
					SP(GuiScrollTemplate, hTracker);
				}

				GuiScroll::IStyleController* CreateVTrackerStyle()override
				{
					SP(GuiScrollTemplate, vTracker);
				}

				GuiScroll::IStyleController* CreateProgressBarStyle()override
				{
					SP(GuiScrollTemplate, progressBar);
				}

				GuiVirtualTextList::IStyleProvider* CreateTextListStyle()override
				{
					SP(GuiTextListTemplate, textList);
				}

				GuiSelectableButton::IStyleController* CreateCheckTextListItemStyle()override
				{
					SP(GuiSelectableButtonTemplate, listItemBackground);
				}

				GuiSelectableButton::IStyleController* CreateRadioTextListItemStyle()override
				{
					SP(GuiSelectableButtonTemplate, treeItemExpander);
				}

#undef SP
			};

			Theme* currentTheme = nullptr;

			ITheme* GetCurrentTheme()
			{
				return currentTheme;
			}

			void InitializeTheme()
			{
				CHECK_ERROR(currentTheme == nullptr, L"vl::presentation::theme::InitializeTheme()#Theme has already been initialized");
				currentTheme = new Theme;
			}

			void FinalizeTheme()
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::FinalizeTheme()#Theme has not been initialized");
				delete currentTheme;
				currentTheme = nullptr;
			}

			bool RegisterTheme(const WString& name, Ptr<ThemeTemplates> theme)
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme has already been initialized");
				return currentTheme->RegisterTheme(name, theme);
			}

			Ptr<ThemeTemplates> UnregisterTheme(const WString& name)
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::UnregisterTheme(const WString&)#Theme has already been initialized");
				return currentTheme->UnregisterTheme(name);
			}

			namespace g
			{
				controls::GuiWindow* NewWindow()
				{
					return new controls::GuiWindow(GetCurrentTheme()->CreateWindowStyle());
				}

				controls::GuiCustomControl* NewCustomControl()
				{
					return new controls::GuiCustomControl(GetCurrentTheme()->CreateCustomControlStyle());
				}

				controls::GuiLabel* NewLabel()
				{
					return new controls::GuiLabel(GetCurrentTheme()->CreateLabelStyle());
				}

				controls::GuiScrollContainer* NewScrollContainer()
				{
					return new controls::GuiScrollContainer(GetCurrentTheme()->CreateScrollContainerStyle());
				}

				controls::GuiControl* NewGroupBox()
				{
					return new controls::GuiControl(GetCurrentTheme()->CreateGroupBoxStyle());
				}

				controls::GuiTab* NewTab()
				{
					return new controls::GuiTab(GetCurrentTheme()->CreateTabStyle());
				}

				controls::GuiTabPage* NewTabPage()
				{
					return new controls::GuiTabPage(GetCurrentTheme()->CreateCustomControlStyle());
				}

				controls::GuiComboBoxListControl* NewComboBox(controls::GuiSelectableListControl* containedListControl)
				{
					return new controls::GuiComboBoxListControl(GetCurrentTheme()->CreateComboBoxStyle(), containedListControl);
				}

				controls::GuiMultilineTextBox* NewMultilineTextBox()
				{
					return new controls::GuiMultilineTextBox(GetCurrentTheme()->CreateMultilineTextBoxStyle());
				}

				controls::GuiSinglelineTextBox* NewTextBox()
				{
					return new controls::GuiSinglelineTextBox(GetCurrentTheme()->CreateTextBoxStyle());
				}

				controls::GuiDocumentViewer* NewDocumentViewer()
				{
					return new controls::GuiDocumentViewer(GetCurrentTheme()->CreateDocumentViewerStyle());
				}

				controls::GuiDocumentLabel* NewDocumentLabel()
				{
					return new controls::GuiDocumentLabel(GetCurrentTheme()->CreateDocumentLabelStyle());
				}

				controls::GuiDocumentLabel* NewDocumentTextBox()
				{
					return new controls::GuiDocumentLabel(GetCurrentTheme()->CreateDocumentTextBoxStyle());
				}

				controls::GuiListView* NewListView()
				{
					controls::GuiListView* listview=new controls::GuiListView(GetCurrentTheme()->CreateListViewStyle());
					return listview;
				}

				controls::GuiTreeView* NewTreeView()
				{
					return new controls::GuiTreeView(GetCurrentTheme()->CreateTreeViewStyle());
				}

				controls::GuiToolstripMenu* NewMenu(controls::GuiControl* owner)
				{
					return new controls::GuiToolstripMenu(GetCurrentTheme()->CreateMenuStyle(), owner);
				}

				controls::GuiToolstripMenuBar* NewMenuBar()
				{
					return new controls::GuiToolstripMenuBar(GetCurrentTheme()->CreateMenuBarStyle());
				}

				controls::GuiControl* NewMenuSplitter()
				{
					return new controls::GuiControl(GetCurrentTheme()->CreateMenuSplitterStyle());
				}

				controls::GuiToolstripButton* NewMenuBarButton()
				{
					return new controls::GuiToolstripButton(GetCurrentTheme()->CreateMenuBarButtonStyle());
				}

				controls::GuiToolstripButton* NewMenuItemButton()
				{
					return new controls::GuiToolstripButton(GetCurrentTheme()->CreateMenuItemButtonStyle());
				}

				controls::GuiToolstripToolBar* NewToolBar()
				{
					return new controls::GuiToolstripToolBar(GetCurrentTheme()->CreateToolBarStyle());
				}

				controls::GuiToolstripButton* NewToolBarButton()
				{
					return new controls::GuiToolstripButton(GetCurrentTheme()->CreateToolBarButtonStyle());
				}

				controls::GuiToolstripButton* NewToolBarDropdownButton()
				{
					return new controls::GuiToolstripButton(GetCurrentTheme()->CreateToolBarDropdownButtonStyle());
				}

				controls::GuiToolstripButton* NewToolBarSplitButton()
				{
					return new controls::GuiToolstripButton(GetCurrentTheme()->CreateToolBarSplitButtonStyle());
				}

				controls::GuiControl* NewToolBarSplitter()
				{
					return new controls::GuiControl(GetCurrentTheme()->CreateToolBarSplitterStyle());
				}

				controls::GuiButton* NewButton()
				{
					return new controls::GuiButton(GetCurrentTheme()->CreateButtonStyle());
				}

				controls::GuiSelectableButton* NewCheckBox()
				{
					return new controls::GuiSelectableButton(GetCurrentTheme()->CreateCheckBoxStyle());
				}

				controls::GuiSelectableButton* NewRadioButton()
				{
					return new controls::GuiSelectableButton(GetCurrentTheme()->CreateRadioButtonStyle());
				}

				controls::GuiDatePicker* NewDatePicker()
				{
					return new controls::GuiDatePicker(GetCurrentTheme()->CreateDatePickerStyle());
				}

				controls::GuiDateComboBox* NewDateComboBox()
				{
					return new controls::GuiDateComboBox(GetCurrentTheme()->CreateComboBoxStyle(), NewDatePicker());
				}

				controls::GuiScroll* NewHScroll()
				{
					return new controls::GuiScroll(GetCurrentTheme()->CreateHScrollStyle());
				}

				controls::GuiScroll* NewVScroll()
				{
					return new controls::GuiScroll(GetCurrentTheme()->CreateVScrollStyle());
				}

				controls::GuiScroll* NewHTracker()
				{
					controls::GuiScroll* scroll=new controls::GuiScroll(GetCurrentTheme()->CreateHTrackerStyle());
					scroll->SetPageSize(0);
					return scroll;
				}

				controls::GuiScroll* NewVTracker()
				{
					controls::GuiScroll* scroll=new controls::GuiScroll(GetCurrentTheme()->CreateVTrackerStyle());
					scroll->SetPageSize(0);
					return scroll;
				}
				
				controls::GuiScroll* NewProgressBar()
				{
					controls::GuiScroll* scroll=new controls::GuiScroll(GetCurrentTheme()->CreateProgressBarStyle());
					scroll->SetPageSize(0);
					return scroll;
				}

				controls::GuiTextList* NewTextList()
				{
					return new controls::GuiTextList(GetCurrentTheme()->CreateTextListStyle());
				}
			}
		}
	}
}