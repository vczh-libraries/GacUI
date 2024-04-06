#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "DarkSkin.h"
#include "../../../Source/UnitTestUtilities/SnapshotViewer/Source/GuiUnitTestSnapshotViewer.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace gaclib_controls;

class ViewModelImpl : public Object, public virtual IUnitTestSnapshotViewerViewModel
{
};

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{
		UnitTestSnapshotViewerWindow window(Ptr(new ViewModelImpl));
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		GetApplication()->Run(&window);
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = SetupHostedWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}