#include "TuiWindowsController.Windows.h"

#ifdef VCZH_MSVC
#include "../WinNativeWindow.h"

namespace vl::presentation::windows
{
	TuiWindowsController::TuiWindowsController(INativeController* services)
		: TuiControllerBase(services)
	{
	}

	void TuiWindowsController::PumpPlatformEvents()
	{
		MSG message;
		while (!console::TUI::IsStopRequested() && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	void TuiWindowsController::ApplyTitle(const WString& title)
	{
		SetConsoleTitleW(title.Buffer());
	}
}

int SetupTuiWindowsRenderer()
{
	using namespace vl;
	using namespace vl::presentation;
	using namespace vl::presentation::windows;
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	EnableCrossKernelCrashing();
	StartWindowsNativeController(GetModuleHandleW(nullptr));
	{
		TuiWindowsController controller(GetWindowsNativeController());
		console::TUI::InstallListener(&controller);
		console::TUI::Start({});
		console::TUI::UninstallListener(&controller);
	}
	SetNativeController(nullptr);
	StopWindowsNativeController();
	CoUninitialize();
	return 0;
}
#endif
