#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"

#ifdef VCZH_MSVC
#pragma comment(lib, "Uiautomationcore.lib")
#pragma comment(lib, "OleAut32.lib")

namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;

	constexpr UINT UiaDispatchMessage = WM_APP + 0x627;
	constexpr UINT UiaQueueMessage = WM_APP + 0x628;
	struct WindowsUIAutomationCall
	{
		const Func<HRESULT()>& action;
		HRESULT result = static_cast<HRESULT>(UIA_E_ELEMENTNOTAVAILABLE);
	};

	LRESULT CALLBACK WindowsUIAutomationDispatcher::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
		}
		auto self = (WindowsUIAutomationDispatcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (self && wParam == (WPARAM)self)
		{
			if (message == UiaDispatchMessage)
			{
				auto call = (WindowsUIAutomationCall*)lParam;
				if (self->window.load() == hwnd) call->result = call->action();
				return 0;
			}
			if (message == UiaQueueMessage)
			{
				auto action = (Func<void()>*)lParam;
				(*action)();
				delete action;
				return 0;
			}
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	WindowsUIAutomationDispatcher::WindowsUIAutomationDispatcher()
		: threadId(GetCurrentThreadId())
	{
		WNDCLASS definition = {};
		definition.lpfnWndProc = WindowProc;
		definition.hInstance = GetModuleHandle(nullptr);
		definition.lpszClassName = L"GacUI.UIAutomation.Dispatcher";
		RegisterClass(&definition);
		window = CreateWindow(definition.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, definition.hInstance, this);
		CHECK_ERROR(window.load(), L"Cannot create the UI Automation dispatcher.");
	}

	WindowsUIAutomationDispatcher::~WindowsUIAutomationDispatcher()
	{
		if (window.load()) std::terminate();
	}

	HRESULT WindowsUIAutomationDispatcher::Run(const Func<HRESULT()>& action)
	{
		auto hwnd = window.load();
		if (!hwnd) return UIA_E_ELEMENTNOTAVAILABLE;
		if (GetCurrentThreadId() == threadId) return action();
		WindowsUIAutomationCall call{ action };
		// No lock is held across SendMessage. WM_GETOBJECT and UIA callbacks can reenter.
		SendMessage(hwnd, UiaDispatchMessage, (WPARAM)this, (LPARAM)&call);
		return call.result;
	}

	void WindowsUIAutomationDispatcher::Queue(const Func<void()>& action)
	{
		CHECK_ERROR(GetCurrentThreadId() == threadId, L"Queue UI Automation work on the UI thread.");
		auto callback = new Func<void()>(action);
		if (!PostMessage(window.load(), UiaQueueMessage, (WPARAM)this, (LPARAM)callback)) delete callback;
	}

	void WindowsUIAutomationDispatcher::Stop()
	{
		CHECK_ERROR(GetCurrentThreadId() == threadId, L"Stop UI Automation on the UI thread.");
		auto hwnd = window.exchange(nullptr);
		if (!hwnd) return;
		MSG message;
		while (PeekMessage(&message, hwnd, UiaQueueMessage, UiaQueueMessage, PM_REMOVE))
		{
			auto action = (Func<void()>*)message.lParam;
			(*action)();
			delete action;
		}
		DestroyWindow(hwnd);
	}

	WindowsUIAutomationListener::WindowsUIAutomationListener(bool isHostedMode)
		: context(Ptr(new WindowsUIAutomationContext(isHostedMode)))
	{
	}

	WindowsUIAutomationListener::~WindowsUIAutomationListener()
	{
		if (!context->stopped) std::terminate();
	}

	void WindowsUIAutomationListener::Start(Ptr<WindowsUIAutomationListener> self)
	{
		messageHandler = self;
		GetWindowsNativeController()->CallbackService()->InstallListener(this);
		if (context->hosted) GetCurrentController()->CallbackService()->InstallListener(this);
		List<IWindowsForm*> forms;
		GetAllCreatedWindows(forms, false);
		for (auto form : forms) NativeWindowCreated(dynamic_cast<INativeWindow*>(form));
	}

	void WindowsUIAutomationListener::Stop()
	{
		GetWindowsNativeController()->CallbackService()->UninstallListener(this);
		if (context->hosted) GetCurrentController()->CallbackService()->UninstallListener(this);
		for (auto native : context->windows.Keys())
		{
			if (auto form = dynamic_cast<IWindowsForm*>(native)) form->UninstallMessageHandler(messageHandler);
		}
		context->Stop();
		messageHandler = nullptr;
	}

	void WindowsUIAutomationListener::NativeWindowCreated(INativeWindow* native)
	{
		if (context->windows.Keys().Contains(native)) return;
		HWND hwnd = nullptr;
		if (auto form = dynamic_cast<IWindowsForm*>(native))
		{
			hwnd = form->GetWindowHandle();
			form->InstallMessageHandler(messageHandler);
		}
		context->windows.Add(native, hwnd);
		context->dispatcher->Queue([state = context]() { if (!state->stopped) state->BindWindows(); });
	}

	void WindowsUIAutomationListener::NativeWindowDestroying(INativeWindow* native)
	{
		auto index = context->roots.Keys().IndexOf(native);
		if (index != -1)
		{
			auto root = context->roots.Values()[index];
			root->Retire();
			context->roots.Remove(native);
		}
		if (auto form = dynamic_cast<IWindowsForm*>(native)) form->UninstallMessageHandler(messageHandler);
		context->windows.Remove(native);
	}

	void WindowsUIAutomationListener::BeforeHandle(HWND, UINT, WPARAM, LPARAM, bool&)
	{
	}

	void WindowsUIAutomationListener::AfterHandle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip, LRESULT& result)
	{
		if (message != WM_GETOBJECT || static_cast<LONG>(lParam) != UiaRootObjectId || context->stopped) return;
		context->BindWindows();
		for (auto root : context->roots.Values())
		{
			if (root->IsLive() && root->IsRoot() && root->Handle() == hwnd)
			{
				result = UiaReturnRawElementProvider(hwnd, wParam, lParam, root->Provider());
				skip = true;
				return;
			}
		}
	}
}
#endif
