#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "DarkSkin.h"
#include "../../../Source/UnitTestUtilities/SnapshotViewer/Application/GuiUnitTestSnapshotViewerApp.h"
#include <Windows.h>

using namespace vl;
using namespace vl::filesystem;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;
using namespace gaclib_controls;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{

#if defined VCZH_MSVC
#ifdef _WIN64
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"..\\..\\..\\..\\..\\Resources\\UnitTestSnapshots";
#else
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"..\\..\\..\\..\\Resources\\UnitTestSnapshots";
#endif
#elif defined VCZH_GCC
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"../../Resources/UnitTestSnapshots";
#endif
		UnitTestSnapshotViewerAppWindow window(Ptr(new UnitTestSnapshotViewerViewModel(snapshotFolderPath)));
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		window.WindowOpened.AttachLambda([&](auto&&...)
		{
			window.ShowMaximized();
		});
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