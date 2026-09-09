#include "TuiController.h"
#include "TuiGraphics.h"
#include "../Hosted/GuiHostedController.h"
#include "../Hosted/GuiHostedGraphics.h"

extern void GuiApplicationMain();

namespace vl::presentation
{
	using namespace console;
	using namespace elements;

	NativeWindowMouseInfo TuiConvertMouseInfo(const WindowMouseInfo& info)
	{
		NativeWindowMouseInfo result;
		result.ctrl = info.ctrl;
		result.shift = info.shift;
		result.alt = info.alt;
		result.osSuper = info.osSuper;
		result.left = info.left;
		result.middle = info.middle;
		result.right = info.right;
		result.x = info.x;
		result.y = info.y;
		result.wheel = info.wheel;
		return result;
	}

	TuiControllerBase::TuiControllerBase(const TuiConfiguration& value)
		: configuration(value)
	{
		CHECK_ERROR(configuration.tabInterval > 0, L"TuiControllerBase#Tab interval must be positive.");
		frameConfig = {
			BoolOption::AlwaysFalse, BoolOption::AlwaysFalse, BoolOption::AlwaysFalse,
			BoolOption::AlwaysFalse, BoolOption::AlwaysFalse, BoolOption::AlwaysFalse,
			BoolOption::AlwaysFalse
		};
	}

	TuiControllerBase::~TuiControllerBase()
	{
	}

	ITuiApplication* TuiControllerBase::GetTuiApplication()
	{
		return this;
	}

	void TuiControllerBase::Starting()
	{
		auto previousController = GetNativeController();
		auto previousHostedApplication = GetHostedApplication();
		auto previousTuiApplication = presentation::GetTuiApplication();
		auto previousResources = GetGuiGraphicsResourceManager();
		GuiHostedController hostedController(this);
		TuiGraphicsResourceManager resourceManager(configuration);
		GuiHostedGraphicsResourceManager hostedResources(&hostedController, &resourceManager);
		SetNativeController(&hostedController);
		SetHostedApplication(hostedController.GetHostedApplication());
		SetTuiApplication(GetTuiApplication());
		SetGuiGraphicsResourceManager(&hostedResources);
		callbackService.InstallListener(&resourceManager);
		auto restoreGlobals = [&]()
		{
			callbackService.UninstallListener(&resourceManager);
			SetGuiGraphicsResourceManager(previousResources);
			SetTuiApplication(previousTuiApplication);
			SetHostedApplication(previousHostedApplication);
			SetNativeController(previousController);
			TUI::Stop();
		};
		try
		{
			RegisterTuiRenderers();
			hostedController.Initialize();
			GuiApplicationMain();
			hostedController.Finalize();
		}
		catch (...)
		{
			restoreGlobals();
			throw;
		}
		restoreGlobals();
	}

	INativeCallbackService* TuiControllerBase::CallbackService()
	{
		return &callbackService;
	}

	INativeAsyncService* TuiControllerBase::AsyncService()
	{
		return &asyncService;
	}

	INativeDialogService* TuiControllerBase::DialogService()
	{
		return nullptr;
	}

	INativeAutomationService* TuiControllerBase::AutomationService()
	{
		return nullptr;
	}

	INativeScreenService* TuiControllerBase::ScreenService()
	{
		return this;
	}

	INativeWindowService* TuiControllerBase::WindowService()
	{
		return this;
	}

	vint TuiControllerBase::GetScreenCount()
	{
		return 1;
	}

	INativeScreen* TuiControllerBase::GetScreen(vint index)
	{
		return index == 0 ? this : nullptr;
	}

	INativeScreen* TuiControllerBase::GetScreen(INativeWindow* window)
	{
		return this;
	}

	NativeRect TuiControllerBase::GetBounds()
	{
		return {NativePoint(), NativeSize(TUI::GetBufferWidth(), TUI::GetBufferHeight())};
	}

	NativeRect TuiControllerBase::GetClientBounds()
	{
		return GetBounds();
	}

	WString TuiControllerBase::GetName()
	{
		return L"Terminal";
	}

	bool TuiControllerBase::IsPrimary()
	{
		return true;
	}

	double TuiControllerBase::GetScalingX()
	{
		return 1;
	}

	double TuiControllerBase::GetScalingY()
	{
		return 1;
	}

	const NativeWindowFrameConfig& TuiControllerBase::GetMainWindowFrameConfig()
	{
		return frameConfig;
	}

	const NativeWindowFrameConfig& TuiControllerBase::GetNonMainWindowFrameConfig()
	{
		return frameConfig;
	}

	INativeWindow* TuiControllerBase::CreateNativeWindow(INativeWindow::WindowMode windowMode)
	{
		CHECK_ERROR(!window, L"TuiControllerBase::CreateNativeWindow#Only one physical terminal window is allowed.");
		window = Ptr(new TuiWindow(this));
		callbackService.InvokeNativeWindowCreated(window.Obj());
		return window.Obj();
	}

	void TuiControllerBase::DestroyNativeWindow(INativeWindow* value)
	{
		CHECK_ERROR(value == window.Obj(), L"TuiControllerBase::DestroyNativeWindow#Unexpected native window.");
		window->Dispatch([](auto listener) { listener->Destroying(); }, true);
		callbackService.InvokeNativeWindowDestroying(value);
		window = nullptr;
	}

	INativeWindow* TuiControllerBase::GetMainWindow()
	{
		return window.Obj();
	}

	INativeWindow* TuiControllerBase::GetWindow(NativePoint location)
	{
		return GetBounds().Contains(location) ? window.Obj() : nullptr;
	}

	void TuiControllerBase::Run(INativeWindow* value)
	{
		CHECK_ERROR(value == window.Obj(), L"TuiControllerBase::Run#Unexpected native window.");
		BufferSizeChanged();
		value->Show();
		while (RunOneCycle()) {}
	}

	bool TuiControllerBase::RunOneCycle()
	{
		if (TUI::IsStopRequested()) return false;
		PumpPlatformEvents();
		return !TUI::IsStopRequested() && TUI::RunOneCycle();
	}

	void TuiControllerBase::Stop()
	{
		TUI::Stop();
	}

	void TuiControllerBase::BufferSizeChanged()
	{
		if (window)
		{
			window->SetClientSize(NativeSize(TUI::GetBufferWidth(), TUI::GetBufferHeight()));
			window->RedrawContent();
		}
	}

	void TuiControllerBase::Timer()
	{
		PumpPlatformEvents();
		if (TUI::IsStopRequested()) return;
		asyncService.ExecuteAsyncTasks();
		if (!TUI::IsStopRequested() && InputService()->IsTimerEnabled()) callbackService.InvokeGlobalTimer();
	}

	void TuiControllerBase::ClipboardUpdated()
	{
		if (!TUI::IsStopRequested()) callbackService.InvokeClipboardUpdated();
	}

	void TuiControllerBase::GlobalShortcutKeyActivated(vint id)
	{
		if (!TUI::IsStopRequested()) callbackService.InvokeGlobalShortcutKeyActivated(id);
	}

	void TuiControllerBase::KeyDown(const NativeWindowKeyInfo& info)
	{
		if (window) window->Dispatch([info](auto listener) { listener->KeyDown(info); });
	}

	void TuiControllerBase::KeyUp(const NativeWindowKeyInfo& info)
	{
		if (window) window->Dispatch([info](auto listener) { listener->KeyUp(info); });
	}

	void TuiControllerBase::Char(const NativeWindowCharInfo& info)
	{
		if (window) window->Dispatch([info](auto listener) { listener->Char(info); });
	}

	void TuiControllerBase::MouseMove(const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([nativeInfo](auto listener) { listener->MouseMoving(nativeInfo); });
	}

	void TuiControllerBase::MouseDown(NativeMouseButton button, const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([button, nativeInfo](auto listener) { listener->MouseDown(button, nativeInfo); });
	}

	void TuiControllerBase::MouseUp(NativeMouseButton button, const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([button, nativeInfo](auto listener) { listener->MouseUp(button, nativeInfo); });
	}

	void TuiControllerBase::MouseDoubleClick(NativeMouseButton button, const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([button, nativeInfo](auto listener) { listener->MouseDoubleClick(button, nativeInfo); });
	}

	void TuiControllerBase::MouseVerticalWheel(const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([nativeInfo](auto listener) { listener->VerticalWheel(nativeInfo); });
	}

	void TuiControllerBase::MouseHorizontalWheel(const WindowMouseInfo& info)
	{
		auto nativeInfo = TuiConvertMouseInfo(info);
		if (window) window->Dispatch([nativeInfo](auto listener) { listener->HorizontalWheel(nativeInfo); });
	}

}
