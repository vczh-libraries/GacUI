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

	Ptr<WindowsUIAutomationMetadata> UiaMetadata(GuiControl* control, bool create)
	{
		auto key = WString::Unmanaged(L"GacUI.Windows.UIAutomation.Metadata");
		auto metadata = control->GetInternalProperty(key).Cast<WindowsUIAutomationMetadata>();
		if (!metadata && create)
		{
			metadata = Ptr(new WindowsUIAutomationMetadata);
			control->SetInternalProperty(key, metadata);
		}
		return metadata;
	}

	void UiaMetadataChanged(GuiControl* control)
	{
		if (auto lifetime = control->GetInternalProperty(L"GacUI.Windows.UIAutomation").Cast<WindowsUIAutomationLifetime>(); lifetime && lifetime->node->IsLive())
		{
			auto context = lifetime->node->context;
			for (auto node : context->nodes)
				if (node->IsLive())
					for (auto parent = node->control; parent; parent = parent->GetParent())
						if (parent == control) { context->Notify(node); break; }
		}
	}

	void SetWindowsUIAutomationName(GuiControl* control, const WString& name)
	{
		UiaMetadata(control, true)->name = name;
		UiaMetadataChanged(control);
	}

	void SetWindowsUIAutomationLabel(GuiControl* control, GuiControl* label)
	{
		auto metadata = UiaMetadata(control, true);
		metadata->label = label;
		metadata->labelDisposed = label ? label->GetDisposedFlag() : nullptr;
		UiaMetadataChanged(control);
	}

	void SetWindowsUIAutomationId(GuiControl* control, const WString& id)
	{
		UiaMetadata(control, true)->id = id;
		UiaMetadataChanged(control);
	}

	void SetWindowsUIAutomationText(GuiControl* control, const WString& key, const WString& text)
	{
		UiaMetadata(control, true)->texts.Set(key, text);
		UiaMetadataChanged(control);
	}

	WString UiaLocalizedText(GuiControl* control, const WString& key, const WString& fallback)
	{
		for (auto current = control; current; current = current->GetParent())
		{
			if (auto metadata = UiaMetadata(current))
			{
				auto index = metadata->texts.Keys().IndexOf(key);
				if (index >= 0) return metadata->texts.Values()[index];
			}
		}
		return fallback;
	}

	constexpr UINT UiaDispatchMessage = WM_APP + 0x627;
	constexpr UINT UiaQueueMessage = WM_APP + 0x628;
	constexpr UINT UiaIdleMessage = WM_APP + 0x629;
	constexpr UINT_PTR UiaIdleTimer = 1;
	struct WindowsUIAutomationIdleRequest : Object
	{
		HANDLE completed = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		Func<HRESULT()> validate;
		HRESULT result = static_cast<HRESULT>(UIA_E_ELEMENTNOTAVAILABLE);
		~WindowsUIAutomationIdleRequest() { CloseHandle(completed); }
	};
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
		if (self && message == WM_TIMER && wParam == UiaIdleTimer)
		{
			KillTimer(hwnd, UiaIdleTimer);
			auto requests = std::move(self->idleRequests);
			for (auto request : requests)
			{
				request->result = request->validate();
				SetEvent(request->completed);
			}
			return 0;
		}
		if (self && wParam == (WPARAM)self)
		{
			if (message == UiaIdleMessage)
			{
				auto request = (Ptr<WindowsUIAutomationIdleRequest>*)lParam;
				self->idleRequests.Add(*request);
				delete request;
				CHECK_ERROR(SetTimer(hwnd, UiaIdleTimer, USER_TIMER_MINIMUM, nullptr), L"Cannot schedule UI Automation idle notification.");
				return 0;
			}
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
		// Final COM Release may arrive on any thread. Stop closes this posting
		// boundary before draining callbacks, so cleanup cannot target a reused HWND.
		auto callback = new Func<void()>(action);
		bool posted = false;
		SPIN_LOCK(lockPosting)
		{
			auto hwnd = window.load();
			if (hwnd) posted = PostMessage(hwnd, UiaQueueMessage, (WPARAM)this, (LPARAM)callback) != FALSE;
		}
		// Releasing captures can itself queue final-reference cleanup.
		if (!posted) delete callback;
	}

	HRESULT WindowsUIAutomationDispatcher::WaitForIdle(int milliseconds, BOOL* result, const Func<HRESULT()>& validate)
	{
		*result = FALSE;
		if (GetCurrentThreadId() == threadId)
		{
			auto status = validate();
			if (SUCCEEDED(status)) *result = HIWORD(GetQueueStatus(QS_ALLINPUT)) == 0;
			return status;
		}
		auto request = Ptr(new WindowsUIAutomationIdleRequest);
		request->validate = validate;
		CHECK_ERROR(request->completed, L"Cannot create UI Automation idle event.");
		SPIN_LOCK(lockPosting)
		{
			auto hwnd = window.load();
			if (!hwnd) return UIA_E_ELEMENTNOTAVAILABLE;
			auto posted = new Ptr<WindowsUIAutomationIdleRequest>(request);
			if (!PostMessage(hwnd, UiaIdleMessage, (WPARAM)this, (LPARAM)posted))
			{
				delete posted;
				return UIA_E_ELEMENTNOTAVAILABLE;
			}
		}
		if (WaitForSingleObject(request->completed, milliseconds) == WAIT_TIMEOUT) return S_OK;
		*result = SUCCEEDED(request->result);
		return request->result;
	}

	void WindowsUIAutomationDispatcher::Stop()
	{
		CHECK_ERROR(GetCurrentThreadId() == threadId, L"Stop UI Automation on the UI thread.");
		HWND hwnd = nullptr;
		SPIN_LOCK(lockPosting) { hwnd = window.exchange(nullptr); }
		if (!hwnd) return;
		MSG message;
		while (PeekMessage(&message, hwnd, UiaIdleMessage, UiaIdleMessage, PM_REMOVE))
		{
			auto request = (Ptr<WindowsUIAutomationIdleRequest>*)message.lParam;
			SetEvent((*request)->completed);
			delete request;
		}
		KillTimer(hwnd, UiaIdleTimer);
		for (auto request : idleRequests) SetEvent(request->completed);
		idleRequests.Clear();
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
			if (auto form = dynamic_cast<IWindowsForm*>(native))
			{
				form->UninstallMessageHandler(messageHandler);
				UiaReturnRawElementProvider(form->GetWindowHandle(), 0, 0, nullptr);
			}
		}
		context->Stop();
		UiaDisconnectAllProviders();
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
		if (context->Subscribed()) context->dispatcher->Queue([state = context]() { if (!state->stopped && state->Subscribed()) state->BindWindows(); });
	}

	void WindowsUIAutomationListener::NativeWindowDestroying(INativeWindow* native)
	{
		auto index = context->roots.Keys().IndexOf(native);
		if (index != -1)
		{
			auto root = context->roots.Values()[index];
			context->RetireSubtree(root->control->GetBoundsComposition());
			context->roots.Remove(native);
		}
		if (auto form = dynamic_cast<IWindowsForm*>(native))
		{
			form->UninstallMessageHandler(messageHandler);
			UiaReturnRawElementProvider(form->GetWindowHandle(), 0, 0, nullptr);
		}
		context->windows.Remove(native);
		context->Collect();
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
				result = UiaReturnRawElementProvider(hwnd, wParam, lParam, root->Provider().Obj());
				skip = true;
				return;
			}
		}
	}
}
#endif
