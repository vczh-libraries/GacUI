#include "GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{
			ITheme* currentTheme=0;

			ITheme* GetCurrentTheme()
			{
				return currentTheme;
			}

			void SetCurrentTheme(ITheme* theme)
			{
				currentTheme=theme;
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