#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "DarkSkin.h"
#include "DemoReflection.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include "resource.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::elements;
using namespace vl::presentation::templates;
using namespace demo;

void GuiMain()
{
	LoadDemoTypes();
	windows::SetWindowDefaultIcon(MAINICON);
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{
		demo::MainWindow window;
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		GetApplication()->Run(&window);
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}