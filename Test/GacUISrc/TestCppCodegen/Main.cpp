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
		NOTIMPL;
	}

	GuiTooltip::IStyleController* CreateTooltipStyle()override
	{
		NOTIMPL;
	}

	GuiLabel::IStyleController* CreateLabelStyle()override
	{
		NOTIMPL;
	}

	GuiLabel::IStyleController* CreateShortcutKeyStyle()override
	{
		NOTIMPL;
	}

	GuiScrollContainer::IStyleProvider* CreateScrollContainerStyle()override
	{
		NOTIMPL;
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
		NOTIMPL;
	}

	GuiScrollView::IStyleProvider* CreateMultilineTextBoxStyle()override
	{
		NOTIMPL;
	}

	GuiSinglelineTextBox::IStyleProvider* CreateTextBoxStyle()override
	{
		NOTIMPL;
	}

	text::ColorEntry GetDefaultTextBoxColorEntry()override
	{
		NOTIMPL;
	}

	GuiDocumentViewer::IStyleProvider* CreateDocumentViewerStyle()override
	{
		NOTIMPL;
	}

	GuiDocumentLabel::IStyleController* CreateDocumentLabelStyle()override
	{
		NOTIMPL;
	}

	GuiDocumentLabel::IStyleController* CreateDocumentTextBoxStyle()override
	{
		NOTIMPL;
	}

	GuiListView::IStyleProvider* CreateListViewStyle()override
	{
		NOTIMPL;
	}

	GuiTreeView::IStyleProvider* CreateTreeViewStyle()override
	{
		NOTIMPL;
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
		return SP(GuiWindowTemplate, ToolstripMenuTemplate);
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
		NOTIMPL;
	}

	GuiToolstripButton::IStyleController* CreateMenuItemButtonStyle()override
	{
		NOTIMPL;
	}

	GuiToolstripToolBar::IStyleController* CreateToolBarStyle()override
	{
		return SP(GuiControlTemplate, ToolstripTemplate);
	}

	GuiToolstripButton::IStyleController* CreateToolBarButtonStyle()override
	{
		NOTIMPL;
	}

	GuiToolstripButton::IStyleController* CreateToolBarDropdownButtonStyle()override
	{
		NOTIMPL;
	}

	GuiToolstripButton::IStyleController* CreateToolBarSplitButtonStyle()override
	{
		NOTIMPL;
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
		NOTIMPL;
	}

	GuiScroll::IStyleController* CreateHScrollStyle()override
	{
		NOTIMPL;
	}

	GuiScroll::IStyleController* CreateVScrollStyle()override
	{
		NOTIMPL;
	}

	GuiScroll::IStyleController* CreateHTrackerStyle()override
	{
		NOTIMPL;
	}

	GuiScroll::IStyleController* CreateVTrackerStyle()override
	{
		NOTIMPL;
	}

	GuiScroll::IStyleController* CreateProgressBarStyle()override
	{
		NOTIMPL;
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
		NOTIMPL;
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