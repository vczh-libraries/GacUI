#include "TuiWindowsController.Windows.h"

#ifdef VCZH_MSVC
#include "../WinNativeWindow.h"

namespace vl::presentation::windows
{
	FontProperties TuiWindowsResourceService::GetDefaultFont()
	{
		auto font = defaultFont;
		font.fontFamily = L"TuiFont";
		font.size = 1;
		return font;
	}

	void TuiWindowsResourceService::SetDefaultFont(const FontProperties& value)
	{
		defaultFont = value;
		defaultFont.fontFamily = L"TuiFont";
		defaultFont.size = 1;
	}

	void TuiWindowsResourceService::EnumerateFonts(collections::List<WString>& fonts)
	{
		fonts.Add(L"TuiFont");
	}

	void TuiWindowsInputService::StartTimer()
	{
		isTimerEnabled = true;
		console::TUI::StartTimer(16);
	}

	void TuiWindowsInputService::StopTimer()
	{
		isTimerEnabled = false;
		if (console::TUI::IsInUse()) console::TUI::StopTimer();
	}

	TuiWindowsController::TuiWindowsController(HINSTANCE _instance)
		: instance(_instance)
	{
		WNDCLASSW windowClass = {};
		windowClass.lpfnWndProc = &ServiceWindowProc;
		windowClass.hInstance = instance;
		windowClass.lpszClassName = L"GacUI.TuiServices";
		CHECK_ERROR(RegisterClassW(&windowClass) != 0, L"TuiWindowsController#Failed to register service-window class.");
		serviceWindow = CreateWindowExW(0, windowClass.lpszClassName, L"GacUI TUI Services", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, this);
		if (!serviceWindow) UnregisterClassW(windowClass.lpszClassName, instance);
		CHECK_ERROR(serviceWindow != nullptr, L"TuiWindowsController#Failed to create service window.");
		inputService.SetOwnerHandle(serviceWindow);
		clipboardService.SetOwnerHandle(serviceWindow);
	}

	TuiWindowsController::~TuiWindowsController()
	{
		inputService.StopTimer();
		clipboardService.SetOwnerHandle(nullptr);
		inputService.SetOwnerHandle(nullptr);
		DestroyWindow(serviceWindow);
		UnregisterClassW(L"GacUI.TuiServices", instance);
	}

	LRESULT CALLBACK TuiWindowsController::ServiceWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			auto creation = reinterpret_cast<CREATESTRUCTW*>(lParam);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
		}
		auto controller = reinterpret_cast<TuiWindowsController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (controller)
		{
			switch (message)
			{
			case WM_CLIPBOARDUPDATE:
				controller->ClipboardUpdated();
				return 0;
			case WM_HOTKEY:
				controller->GlobalShortcutKeyActivated(static_cast<vint>(wParam));
				return 0;
			case WM_NCDESTROY:
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
				break;
			}
		}
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	INativeResourceService* TuiWindowsController::ResourceService()
	{
		return &resourceService;
	}

	INativeInputService* TuiWindowsController::InputService()
	{
		return &inputService;
	}

	INativeClipboardService* TuiWindowsController::ClipboardService()
	{
		return &clipboardService;
	}

	INativeImageService* TuiWindowsController::ImageService()
	{
		return &imageService;
	}

	WString TuiWindowsController::GetExecutablePath()
	{
		collections::Array<wchar_t> buffer(65536);
		GetModuleFileNameW(instance, &buffer[0], static_cast<DWORD>(buffer.Count()));
		return &buffer[0];
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
	try
	{
		TuiWindowsController controller(GetModuleHandleW(nullptr));
		console::TUI::InstallListener(&controller);
		try
		{
			console::TUI::Start({});
		}
		catch (...)
		{
			console::TUI::UninstallListener(&controller);
			throw;
		}
		console::TUI::UninstallListener(&controller);
	}
	catch (...)
	{
		CoUninitialize();
		throw;
	}
	CoUninitialize();
	return 0;
}
#endif
