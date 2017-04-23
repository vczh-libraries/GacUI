#include <math.h>
#include "GuiWin7Styles.h"

#include "Win7Styles/GuiWin7StylesCommon.h"
#include "Win7Styles/GuiWin7ControlStyles.h"
#include "Win7Styles/GuiWin7ButtonStyles.h"
#include "Win7Styles/GuiWin7TabStyles.h"
#include "Win7Styles/GuiWin7MenuStyles.h"
#include "Win7Styles/GuiWin7ToolstripStyles.h"
#include "Win7Styles/GuiWin7ScrollableStyles.h"
#include "Win7Styles/GuiWin7ListStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win7Theme
***********************************************************************/

			Win7Theme::Win7Theme()
			{
			}

			Win7Theme::~Win7Theme()
			{
			}

			controls::GuiWindow::IStyleController* Win7Theme::CreateWindowStyle()
			{
				return new Win7WindowStyle;
			}

			controls::GuiCustomControl::IStyleController* Win7Theme::CreateCustomControlStyle()
			{
				return new GuiCustomControl::EmptyStyleController;
			}

			controls::GuiTooltip::IStyleController* Win7Theme::CreateTooltipStyle()
			{
				return new Win7TooltipStyle;
			}

			controls::GuiLabel::IStyleController* Win7Theme::CreateLabelStyle()
			{
				return new Win7LabelStyle(false);
			}

			controls::GuiLabel::IStyleController* Win7Theme::CreateShortcutKeyStyle()
			{
				return new Win7LabelStyle(true);
			}

			controls::GuiScrollContainer::IStyleProvider* Win7Theme::CreateScrollContainerStyle()
			{
				return new Win7ScrollViewProvider;
			}

			controls::GuiControl::IStyleController* Win7Theme::CreateGroupBoxStyle()
			{
				return new Win7GroupBoxStyle;
			}

			controls::GuiTab::IStyleController* Win7Theme::CreateTabStyle()
			{
				return new Win7TabStyle;
			}

			controls::GuiComboBoxListControl::IStyleController* Win7Theme::CreateComboBoxStyle()
			{
				return new Win7DropDownComboBoxStyle;
			}

			controls::GuiScrollView::IStyleProvider* Win7Theme::CreateMultilineTextBoxStyle()
			{
				return new Win7MultilineTextBoxProvider;
			}

			controls::GuiSinglelineTextBox::IStyleProvider* Win7Theme::CreateTextBoxStyle()
			{
				return new Win7SinglelineTextBoxProvider;
			}

			elements::text::ColorEntry Win7Theme::GetDefaultTextBoxColorEntry()
			{
				return Win7GetTextBoxTextColor();
			}

			controls::GuiDocumentViewer::IStyleProvider* Win7Theme::CreateDocumentViewerStyle()
			{
				return new Win7DocumentViewerStyle;
			}

			controls::GuiDocumentLabel::IStyleController* Win7Theme::CreateDocumentLabelStyle()
			{
				return new Win7DocumentLabelStyle;
			}

			controls::GuiDocumentLabel::IStyleController* Win7Theme::CreateDocumentTextBoxStyle()
			{
				return new Win7DocumentTextBoxStyle;
			}

			controls::GuiListView::IStyleProvider* Win7Theme::CreateListViewStyle()
			{
				return new Win7ListViewProvider;
			}

			controls::GuiTreeView::IStyleProvider* Win7Theme::CreateTreeViewStyle()
			{
				return new Win7TreeViewProvider;
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateListItemBackgroundStyle()
			{
				return new Win7SelectableItemStyle();
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateListItemBackgroundStyle()
			{
				return new Win7TreeViewExpandingButtonStyle();
			}

			controls::GuiToolstripMenu::IStyleController* Win7Theme::CreateMenuStyle()
			{
				return new Win7MenuStyle;
			}

			controls::GuiToolstripMenuBar::IStyleController* Win7Theme::CreateMenuBarStyle()
			{
				return new Win7MenuBarStyle;
			}

			controls::GuiControl::IStyleController* Win7Theme::CreateMenuSplitterStyle()
			{
				return new Win7MenuSplitterStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win7Theme::CreateMenuBarButtonStyle()
			{
				return new Win7MenuBarButtonStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win7Theme::CreateMenuItemButtonStyle()
			{
				return new Win7MenuItemButtonStyle;
			}

			controls::GuiControl::IStyleController* Win7Theme::CreateToolBarStyle()
			{
				return new Win7ToolstripToolBarStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win7Theme::CreateToolBarButtonStyle()
			{
				return new Win7ToolstripButtonStyle(Win7ToolstripButtonStyle::CommandButton);
			}

			controls::GuiToolstripButton::IStyleController* Win7Theme::CreateToolBarDropdownButtonStyle()
			{
				return new Win7ToolstripButtonStyle(Win7ToolstripButtonStyle::DropdownButton);
			}

			controls::GuiToolstripButton::IStyleController* Win7Theme::CreateToolBarSplitButtonStyle()
			{
				return new Win7ToolstripButtonStyle(Win7ToolstripButtonStyle::SplitButton);
			}

			controls::GuiControl::IStyleController* Win7Theme::CreateToolBarSplitterStyle()
			{
				return new Win7ToolstripSplitterStyle;
			}

			controls::GuiButton::IStyleController* Win7Theme::CreateButtonStyle()
			{
				return new Win7ButtonStyle;
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateCheckBoxStyle()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::CheckBox);
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateRadioButtonStyle()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::RadioButton);
			}

			controls::GuiDatePicker::IStyleProvider* Win7Theme::CreateDatePickerStyle()
			{
				return new Win7DatePickerStyle;
			}

			controls::GuiScroll::IStyleController* Win7Theme::CreateHScrollStyle()
			{
				return new Win7ScrollStyle(Win7ScrollStyle::Horizontal);
			}

			controls::GuiScroll::IStyleController* Win7Theme::CreateVScrollStyle()
			{
				return new Win7ScrollStyle(Win7ScrollStyle::Vertical);
			}

			controls::GuiScroll::IStyleController* Win7Theme::CreateHTrackerStyle()
			{
				return new Win7TrackStyle(Win7TrackStyle::Horizontal);
			}

			controls::GuiScroll::IStyleController* Win7Theme::CreateVTrackerStyle()
			{
				return new Win7TrackStyle(Win7TrackStyle::Vertical);
			}

			controls::GuiScroll::IStyleController* Win7Theme::CreateProgressBarStyle()
			{
				return new Win7ProgressBarStyle;
			}

			vint Win7Theme::GetScrollDefaultSize()
			{
				return Win7ScrollStyle::DefaultSize;
			}

			vint Win7Theme::GetTrackerDefaultSize()
			{
				return Win7TrackStyle::HandleLong;
			}

			controls::GuiVirtualTextList::IStyleProvider* Win7Theme::CreateTextListStyle()
			{
				return new Win7TextListProvider;
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateCheckTextListItemStyle()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::CheckBox, false);
			}

			controls::GuiSelectableButton::IStyleController* Win7Theme::CreateRadioTextListItemStyle()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::RadioButton, false);
			}
		}
	}
}