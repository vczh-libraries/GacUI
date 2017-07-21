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
using namespace darkskin;

class DarkSkin : public Object, public virtual theme::ITheme
{
public:
	GuiWindow::IStyleController* CreateWindowStyle()override
	{
	}

	GuiCustomControl::IStyleController* CreateCustomControlStyle()override
	{
	}

	GuiTooltip::IStyleController* CreateTooltipStyle()override
	{
	}

	GuiLabel::IStyleController* CreateLabelStyle()override
	{
	}

	GuiLabel::IStyleController* CreateShortcutKeyStyle()override
	{
	}

	GuiScrollContainer::IStyleProvider* CreateScrollContainerStyle()override
	{
	}

	GuiControl::IStyleController* CreateGroupBoxStyle()override
	{
	}

	GuiTab::IStyleController* CreateTabStyle()override
	{
	}

	GuiComboBoxListControl::IStyleController* CreateComboBoxStyle()override
	{
	}

	GuiScrollView::IStyleProvider* CreateMultilineTextBoxStyle()override
	{
	}

	GuiSinglelineTextBox::IStyleProvider* CreateTextBoxStyle()override
	{
	}

	text::ColorEntry GetDefaultTextBoxColorEntry()override
	{
	}

	GuiDocumentViewer::IStyleProvider* CreateDocumentViewerStyle()override
	{
	}

	GuiDocumentLabel::IStyleController* CreateDocumentLabelStyle()override
	{
	}

	GuiDocumentLabel::IStyleController* CreateDocumentTextBoxStyle()override
	{
	}

	GuiListView::IStyleProvider* CreateListViewStyle()override
	{
	}

	GuiTreeView::IStyleProvider* CreateTreeViewStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateListItemBackgroundStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateTreeItemExpanderStyle()override
	{
	}

	GuiToolstripMenu::IStyleController* CreateMenuStyle()override
	{
	}

	GuiToolstripMenuBar::IStyleController* CreateMenuBarStyle()override
	{
	}

	GuiControl::IStyleController* CreateMenuSplitterStyle()override
	{
	}

	GuiToolstripButton::IStyleController* CreateMenuBarButtonStyle()override
	{
	}

	GuiToolstripButton::IStyleController* CreateMenuItemButtonStyle()override
	{
	}

	GuiToolstripToolBar::IStyleController* CreateToolBarStyle()override
	{
	}

	GuiToolstripButton::IStyleController* CreateToolBarButtonStyle()override
	{
	}

	GuiToolstripButton::IStyleController* CreateToolBarDropdownButtonStyle()override
	{
	}

	GuiToolstripButton::IStyleController* CreateToolBarSplitButtonStyle()override
	{
	}

	GuiControl::IStyleController* CreateToolBarSplitterStyle()override
	{
	}

	GuiButton::IStyleController* CreateButtonStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateCheckBoxStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateRadioButtonStyle()override
	{
	}

	GuiDatePicker::IStyleProvider* CreateDatePickerStyle()override
	{
	}

	GuiScroll::IStyleController* CreateHScrollStyle()override
	{
	}

	GuiScroll::IStyleController* CreateVScrollStyle()override
	{
	}

	GuiScroll::IStyleController* CreateHTrackerStyle()override
	{
	}

	GuiScroll::IStyleController* CreateVTrackerStyle()override
	{
	}

	GuiScroll::IStyleController* CreateProgressBarStyle()override
	{
	}

	vint GetScrollDefaultSize()override
	{
	}

	vint GetTrackerDefaultSize()override
	{
	}

	GuiVirtualTextList::IStyleProvider* CreateTextListStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateCheckTextListItemStyle()override
	{
	}

	GuiSelectableButton::IStyleController* CreateRadioTextListItemStyle()override
	{
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