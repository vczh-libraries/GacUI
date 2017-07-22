#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Source/DemoIncludes.h"
#include <Windows.h>

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::elements;
using namespace vl::presentation::templates;
using namespace demo;

#define SP(NAME, CLASS)\
	new NAME##_StyleProvider([](auto _)\
	{\
		return new darkskin::CLASS;\
	})\

#define NOTIMPL throw "NOT-USED-IN-THIS-TUTORIAL"

class DarkSkin : public Object, public virtual theme::ITheme
{
public:
	GuiWindow::IStyleController* CreateWindowStyle()override
	{
		return SP(GuiWindowTemplate, WindowTemplate);
	}

	GuiCustomControl::IStyleController* CreateCustomControlStyle()override
	{
		return new GuiControl::EmptyStyleController;
	}

	GuiTooltip::IStyleController* CreateTooltipStyle()override
	{
		return SP(GuiWindowTemplate, TooltipTemplate);
	}

	GuiLabel::IStyleController* CreateLabelStyle()override
	{
		return SP(GuiLabelTemplate, LabelTemplate);
	}

	GuiLabel::IStyleController* CreateShortcutKeyStyle()override
	{
		return SP(GuiLabelTemplate, ShortcutKeyTemplate);
	}

	GuiScrollContainer::IStyleProvider* CreateScrollContainerStyle()override
	{
		return SP(GuiScrollViewTemplate, ScrollViewTemplate);
	}

	GuiControl::IStyleController* CreateGroupBoxStyle()override
	{
		return SP(GuiControlTemplate, GroupBoxTemplate);
	}

	GuiTab::IStyleController* CreateTabStyle()override
	{
		return SP(GuiTabTemplate, TabTemplate);
	}

	GuiComboBoxListControl::IStyleController* CreateComboBoxStyle()override
	{
		return SP(GuiComboBoxTemplate, ComboBoxTemplate);
	}

	GuiScrollView::IStyleProvider* CreateMultilineTextBoxStyle()override
	{
		return SP(GuiMultilineTextBoxTemplate, MultilineTextBoxTemplate);
	}

	GuiSinglelineTextBox::IStyleProvider* CreateTextBoxStyle()override
	{
		return SP(GuiSinglelineTextBoxTemplate, SinglelineTextBoxTemplate);
	}

	text::ColorEntry GetDefaultTextBoxColorEntry()override
	{
		NOTIMPL;
	}

	GuiDocumentViewer::IStyleProvider* CreateDocumentViewerStyle()override
	{
		return SP(GuiDocumentViewerTemplate, DocumentViewerTemplate);
	}

	GuiDocumentLabel::IStyleController* CreateDocumentLabelStyle()override
	{
		return SP(GuiDocumentLabelTemplate, DocumentLabelTemplate);
	}

	GuiDocumentLabel::IStyleController* CreateDocumentTextBoxStyle()override
	{
		return SP(GuiDocumentLabelTemplate, DocumentTextBoxTemplate);
	}

	GuiListView::IStyleProvider* CreateListViewStyle()override
	{
		return SP(GuiListViewTemplate, ListViewTemplate);
	}

	GuiTreeView::IStyleProvider* CreateTreeViewStyle()override
	{
		return SP(GuiTreeViewTemplate, TreeViewTemplate);
	}

	GuiSelectableButton::IStyleController* CreateListItemBackgroundStyle()override
	{
		NOTIMPL;
	}

	GuiSelectableButton::IStyleController* CreateTreeItemExpanderStyle()override
	{
		NOTIMPL;
	}

	GuiToolstripMenu::IStyleController* CreateMenuStyle()override
	{
		return SP(GuiMenuTemplate, ToolstripMenuTemplate);
	}

	GuiToolstripMenuBar::IStyleController* CreateMenuBarStyle()override
	{
		return SP(GuiControlTemplate, ToolstripTemplate);
	}

	GuiControl::IStyleController* CreateMenuSplitterStyle()override
	{
		return SP(GuiControlTemplate, MenuSplitterTemplate);
	}

	GuiToolstripButton::IStyleController* CreateMenuBarButtonStyle()override
	{
		return SP(GuiToolstripButtonTemplate, MenuBarButtonTemplate);
	}

	GuiToolstripButton::IStyleController* CreateMenuItemButtonStyle()override
	{
		return SP(GuiToolstripButtonTemplate, MenuItemButtonTemplate);
	}

	GuiToolstripToolBar::IStyleController* CreateToolBarStyle()override
	{
		return SP(GuiControlTemplate, ToolstripTemplate);
	}

	GuiToolstripButton::IStyleController* CreateToolBarButtonStyle()override
	{
		return SP(GuiToolstripButtonTemplate, ToolstripButtonTemplate);
	}

	GuiToolstripButton::IStyleController* CreateToolBarDropdownButtonStyle()override
	{
		return SP(GuiToolstripButtonTemplate, ToolstripDropdownButtonTemplate);
	}

	GuiToolstripButton::IStyleController* CreateToolBarSplitButtonStyle()override
	{
		return SP(GuiToolstripButtonTemplate, ToolstripSplitButtonTemplate);
	}

	GuiControl::IStyleController* CreateToolBarSplitterStyle()override
	{
		return SP(GuiControlTemplate, ToolstripSplitterTemplate);
	}

	GuiButton::IStyleController* CreateButtonStyle()override
	{
		return SP(GuiButtonTemplate, ButtonTemplate);
	}

	GuiSelectableButton::IStyleController* CreateCheckBoxStyle()override
	{
		return SP(GuiSelectableButtonTemplate, CheckBoxTemplate);
	}

	GuiSelectableButton::IStyleController* CreateRadioButtonStyle()override
	{
		return SP(GuiSelectableButtonTemplate, RadioButtonTemplate);
	}

	GuiDatePicker::IStyleProvider* CreateDatePickerStyle()override
	{
		return SP(GuiDatePickerTemplate, DatePickerTemplate);
	}

	GuiScroll::IStyleController* CreateHScrollStyle()override
	{
		return SP(GuiScrollTemplate, HScrollTemplate);
	}

	GuiScroll::IStyleController* CreateVScrollStyle()override
	{
		return SP(GuiScrollTemplate, VScrollTemplate);
	}

	GuiScroll::IStyleController* CreateHTrackerStyle()override
	{
		return SP(GuiScrollTemplate, HTrackerTemplate);
	}

	GuiScroll::IStyleController* CreateVTrackerStyle()override
	{
		return SP(GuiScrollTemplate, VTrackerTemplate);
	}

	GuiScroll::IStyleController* CreateProgressBarStyle()override
	{
		return SP(GuiScrollTemplate, ProgressBarTemplate);
	}

	vint GetScrollDefaultSize()override
	{
		NOTIMPL;
	}

	vint GetTrackerDefaultSize()override
	{
		NOTIMPL;
	}

	GuiVirtualTextList::IStyleProvider* CreateTextListStyle()override
	{
		return SP(GuiTextListTemplate, TextListTemplate);
	}

	GuiSelectableButton::IStyleController* CreateCheckTextListItemStyle()override
	{
		NOTIMPL;
	}

	GuiSelectableButton::IStyleController* CreateRadioTextListItemStyle()override
	{
		NOTIMPL;
	}

};

void GuiMain()
{
	{
		FileStream fileStream(L"UI.bin", FileStream::ReadOnly);
		auto resource = GuiResource::LoadPrecompiledBinary(fileStream);
		GetResourceManager()->SetResource(L"Resource", resource, GuiResourceUsage::DataOnly);
	}

	LoadDemoTypes();

	DarkSkin darkSkin;
	auto oldTheme = theme::GetCurrentTheme();
	theme::SetCurrentTheme(&darkSkin);
	{
		demo::MainWindow window;
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		GetApplication()->Run(&window);
	}
	theme::SetCurrentTheme(oldTheme);
}

#define GUI_GRAPHICS_RENDERER_GDI
//#define GUI_GRAPHICS_RENDERER_DIRECT2D

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
#ifdef GUI_GRAPHICS_RENDERER_GDI
	int result = SetupWindowsGDIRenderer();
#endif
#ifdef GUI_GRAPHICS_RENDERER_DIRECT2D
	int result = SetupWindowsDirect2DRenderer();
#endif

#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}