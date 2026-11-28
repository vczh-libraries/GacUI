#include "../Remote/GuiRemoteController.h"
#include "ServicesImpl/WindowsClipboardService.h"
#include "ServicesImpl/WindowsImageService.h"

namespace vl::presentation::windows
{
	// The Core runs a protocol loop and has no ordinary Windows controller.
	class WindowsRemoteClipboardService : public WindowsClipboardService, public IGuiRemoteClipboardEventSource
	{
	public:
		WindowsRemoteClipboardService(WindowsImageService* images)
			: WindowsClipboardService(images)
		{
			auto owner = CreateWindowExW(0, L"STATIC", L"GacUI Core Clipboard", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
			CHECK_ERROR(owner, L"WindowsRemoteClipboardService()#Failed to create the clipboard owner window.");
			SetOwnerHandle(owner);
		}

		~WindowsRemoteClipboardService()
		{
			auto owner = ownerHandle;
			SetOwnerHandle(nullptr);
			DestroyWindow(owner);
		}

		bool PumpClipboardEvents() override
		{
			bool changed = readPending;
			readPending = false;
			MSG message;
			while (PeekMessageW(&message, ownerHandle, 0, 0, PM_REMOVE))
			{
				if (message.message == WM_CLIPBOARDUPDATE) changed = true;
				DispatchMessageW(&message);
			}
			return changed;
		}
	};
}

int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol, const vl::presentation::GuiRemoteControllerConfig& config)
{
	using namespace vl;
	using namespace vl::presentation::windows;
	auto initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	CHECK_ERROR(SUCCEEDED(initialized) || initialized == RPC_E_CHANGED_MODE, L"SetupRemoteNativeController()#COM initialization failed.");
	int result;
	{
		WindowsImageService images;
		WindowsRemoteClipboardService clipboard(&images);
		result = SetupRemoteNativeControllerInternal(protocol, config, &clipboard);
	}
	if (SUCCEEDED(initialized)) CoUninitialize();
	return result;
}
