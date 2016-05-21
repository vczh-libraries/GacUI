#include <math.h>
#include "GuiWin8Styles.h"
#include "Win8Styles/GuiWin8StylesCommon.h"
#include "Win8Styles/GuiWin8ControlStyles.h"
#include "Win8Styles/GuiWin8ButtonStyles.h"
#include "Win8Styles/GuiWin8TabStyles.h"
#include "Win8Styles/GuiWin8MenuStyles.h"
#include "Win8Styles/GuiWin8ToolstripStyles.h"
#include "Win8Styles/GuiWin8ScrollableStyles.h"
#include "Win8Styles/GuiWin8ListStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win8Theme
***********************************************************************/

			Win8Theme::Win8Theme()
			{
			}

			Win8Theme::~Win8Theme()
			{
			}

			controls::GuiWindow::IStyleController* Win8Theme::CreateWindowStyle()
			{
				return new Win8WindowStyle;
			}

			controls::GuiCustomControl::IStyleController* Win8Theme::CreateCustomControlStyle()
			{
				return new GuiCustomControl::EmptyStyleController;
			}

			controls::GuiTooltip::IStyleController* Win8Theme::CreateTooltipStyle()
			{
				return new Win8TooltipStyle;
			}

			controls::GuiLabel::IStyleController* Win8Theme::CreateLabelStyle()
			{
				return new Win8LabelStyle(false);
			}

			controls::GuiLabel::IStyleController* Win8Theme::CreateShortcutKeyStyle()
			{
				return new Win8LabelStyle(true);
			}

			controls::GuiScrollContainer::IStyleProvider* Win8Theme::CreateScrollContainerStyle()
			{
				return new Win8ScrollViewProvider;
			}

			controls::GuiControl::IStyleController* Win8Theme::CreateGroupBoxStyle()
			{
				return new Win8GroupBoxStyle;
			}

			controls::GuiTab::IStyleController* Win8Theme::CreateTabStyle()
			{
				return new Win8TabStyle;
			}

			controls::GuiComboBoxBase::IStyleController* Win8Theme::CreateComboBoxStyle()
			{
				return new Win8DropDownComboBoxStyle;
			}

			controls::GuiScrollView::IStyleProvider* Win8Theme::CreateMultilineTextBoxStyle()
			{
				return new Win8MultilineTextBoxProvider;
			}

			controls::GuiSinglelineTextBox::IStyleProvider* Win8Theme::CreateTextBoxStyle()
			{
				return new Win8SinglelineTextBoxProvider;
			}

			elements::text::ColorEntry Win8Theme::GetDefaultTextBoxColorEntry()
			{
				return Win8GetTextBoxTextColor();
			}

			controls::GuiDocumentViewer::IStyleProvider* Win8Theme::CreateDocumentViewerStyle()
			{
				return new Win8DocumentViewerStyle;
			}

			controls::GuiDocumentLabel::IStyleController* Win8Theme::CreateDocumentLabelStyle()
			{
				return new Win8DocumentLabelStyle;
			}

			controls::GuiDocumentLabel::IStyleController* Win8Theme::CreateDocumentTextBoxStyle()
			{
				return new Win8DocumentTextBoxStyle;
			}

			controls::GuiListView::IStyleProvider* Win8Theme::CreateListViewStyle()
			{
				return new Win8ListViewProvider;
			}

			controls::GuiTreeView::IStyleProvider* Win8Theme::CreateTreeViewStyle()
			{
				return new Win8TreeViewProvider;
			}

			controls::GuiSelectableButton::IStyleController* Win8Theme::CreateListItemBackgroundStyle()
			{
				return new Win8SelectableItemStyle();
			}

			controls::GuiToolstripMenu::IStyleController* Win8Theme::CreateMenuStyle()
			{
				return new Win8MenuStyle;
			}

			controls::GuiToolstripMenuBar::IStyleController* Win8Theme::CreateMenuBarStyle()
			{
				return new Win8MenuBarStyle;
			}

			controls::GuiControl::IStyleController* Win8Theme::CreateMenuSplitterStyle()
			{
				return new Win8MenuSplitterStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win8Theme::CreateMenuBarButtonStyle()
			{
				return new Win8MenuBarButtonStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win8Theme::CreateMenuItemButtonStyle()
			{
				return new Win8MenuItemButtonStyle;
			}

			controls::GuiControl::IStyleController* Win8Theme::CreateToolBarStyle()
			{
				return new Win8ToolstripToolBarStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win8Theme::CreateToolBarButtonStyle()
			{
				return new Win8ToolstripButtonStyle(Win8ToolstripButtonStyle::CommandButton);
			}

			controls::GuiToolstripButton::IStyleController* Win8Theme::CreateToolBarDropdownButtonStyle()
			{
				return new Win8ToolstripButtonStyle(Win8ToolstripButtonStyle::DropdownButton);
			}

			controls::GuiToolstripButton::IStyleController* Win8Theme::CreateToolBarSplitButtonStyle()
			{
				return new Win8ToolstripButtonStyle(Win8ToolstripButtonStyle::SplitButton);
			}

			controls::GuiControl::IStyleController* Win8Theme::CreateToolBarSplitterStyle()
			{
				return new Win8ToolstripSplitterStyle;
			}

			controls::GuiButton::IStyleController* Win8Theme::CreateButtonStyle()
			{
				return new Win8ButtonStyle;
			}

			controls::GuiSelectableButton::IStyleController* Win8Theme::CreateCheckBoxStyle()
			{
				return new Win8CheckBoxStyle(Win8CheckBoxStyle::CheckBox);
			}

			controls::GuiSelectableButton::IStyleController* Win8Theme::CreateRadioButtonStyle()
			{
				return new Win8CheckBoxStyle(Win8CheckBoxStyle::RadioButton);
			}

			controls::GuiDatePicker::IStyleProvider* Win8Theme::CreateDatePickerStyle()
			{
				return new Win8DatePickerStyle;
			}

			controls::GuiScroll::IStyleController* Win8Theme::CreateHScrollStyle()
			{
				return new Win8ScrollStyle(common_styles::CommonScrollStyle::Horizontal);
			}

			controls::GuiScroll::IStyleController* Win8Theme::CreateVScrollStyle()
			{
				return new Win8ScrollStyle(common_styles::CommonScrollStyle::Vertical);
			}

			controls::GuiScroll::IStyleController* Win8Theme::CreateHTrackerStyle()
			{
				return new Win8TrackStyle(common_styles::CommonTrackStyle::Horizontal);
			}

			controls::GuiScroll::IStyleController* Win8Theme::CreateVTrackerStyle()
			{
				return new Win8TrackStyle(common_styles::CommonTrackStyle::Vertical);
			}

			controls::GuiScroll::IStyleController* Win8Theme::CreateProgressBarStyle()
			{
				return new Win8ProgressBarStyle;
			}

			vint Win8Theme::GetScrollDefaultSize()
			{
				return Win8ScrollStyle::DefaultSize;
			}

			vint Win8Theme::GetTrackerDefaultSize()
			{
				return Win8TrackStyle::HandleLong;
			}

			controls::GuiScrollView::IStyleProvider* Win8Theme::CreateTextListStyle()
			{
				return new Win8MultilineTextBoxProvider;
			}

			controls::list::TextItemStyleProvider::ITextItemStyleProvider* Win8Theme::CreateTextListItemStyle()
			{
				return new Win8TextListProvider;
			}

			controls::list::TextItemStyleProvider::ITextItemStyleProvider* Win8Theme::CreateCheckTextListItemStyle()
			{
				return new Win8CheckTextListProvider;
			}

			controls::list::TextItemStyleProvider::ITextItemStyleProvider* Win8Theme::CreateRadioTextListItemStyle()
			{
				return new Win8RadioTextListProvider;
			}
		}
	}
}