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
	darkSkin->NAME = [](const Value&)\
	{\
		return new darkskin::CLASS;\
	};\

Ptr<theme::ThemeTemplates> CreateDarkSkin()
{
	auto darkSkin = MakePtr<theme::ThemeTemplates>();

	SP(window, WindowTemplate);
	SP(customControl, CustomControlTemplate);
	SP(tooltip, TooltipTemplate);
	SP(label, LabelTemplate);
	SP(shortcutKey, ShortcutKeyTemplate);
	SP(scrollView, ScrollViewTemplate);
	SP(groupBox, GroupBoxTemplate);
	SP(tab, TabTemplate);
	SP(comboBox, ComboBoxTemplate);
	SP(multilineTextBox, MultilineTextBoxTemplate);
	SP(singlelineTextBox, SinglelineTextBoxTemplate);
	SP(documentViewer, DocumentViewerTemplate);
	SP(documentLabel, DocumentLabelTemplate);
	SP(documentTextBox, DocumentTextBoxTemplate);
	SP(listView, ListViewTemplate);
	SP(treeView, TreeViewTemplate);
	SP(textList, TextListTemplate);
	SP(menu, ToolstripMenuTemplate);
	SP(menuBar, ToolstripTemplate);
	SP(menuSplitter, MenuSplitterTemplate);
	SP(menuBarButton, MenuBarButtonTemplate);
	SP(menuItemButton, MenuItemButtonTemplate);
	SP(toolBar, ToolstripTemplate);
	SP(toolBarButton, ToolstripButtonTemplate);
	SP(toolBarDropdownButton, ToolstripDropdownButtonTemplate);
	SP(toolBarSplitButton, ToolstripSplitButtonTemplate);
	SP(toolBarSplitter, ToolstripSplitterTemplate);
	SP(button, ButtonTemplate);
	SP(checkBox, CheckBoxTemplate);
	SP(radioButton, RadioButtonTemplate);
	SP(datePicker, DatePickerTemplate);
	SP(hScroll, HScrollTemplate);
	SP(vScroll, VScrollTemplate);
	SP(hTracker, HTrackerTemplate);
	SP(vTracker, VTrackerTemplate);
	SP(progressBar, ProgressBarTemplate);

	return darkSkin;
}

void GuiMain()
{
	{
		FileStream fileStream(L"UI.bin", FileStream::ReadOnly);
		auto resource = GuiResource::LoadPrecompiledBinary(fileStream);
		GetResourceManager()->SetResource(L"Resource", resource, GuiResourceUsage::DataOnly);
	}

	LoadDemoTypes();

	theme::RegisterTheme(L"DarkSkin", CreateDarkSkin());
	{
		demo::MainWindow window;
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		GetApplication()->Run(&window);
	}
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