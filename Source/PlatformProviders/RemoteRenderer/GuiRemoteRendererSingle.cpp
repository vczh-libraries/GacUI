#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace elements;
	using namespace remoteprotocol;

	bool GuiRemoteRendererSingle::CanSendEvents()
	{
		return events && !disconnectingFromCore;
	}

	void GuiRemoteRendererSingle::ClearPendingCoreEvents()
	{
		pendingMouseMove.Reset();
		pendingHWheel.Reset();
		pendingVWheel.Reset();
		pendingKeyAutoDown.Reset();
		pendingWindowBoundsUpdate.Reset();
	}

	void GuiRemoteRendererSingle::DisconnectFromCore()
	{
		disconnectingFromCore = true;
		if (window)
		{
			window->ReleaseCapture();
		}
		UnregisterGlobalShortcutKeys();
		ClearPendingCoreEvents();
	}

	void GuiRemoteRendererSingle::RequestCoreForceExitByFatalError()
	{
		if (CanSendEvents())
		{
			events->OnControllerForceExit();
		}
	}

	remoteprotocol::ScreenConfig GuiRemoteRendererSingle::GetScreenConfig(INativeScreen* screen)
	{
		ScreenConfig response;
		response.bounds = screen->GetBounds();
		response.clientBounds = screen->GetClientBounds();
		response.scalingX = screen->GetScalingX();
		response.scalingY = screen->GetScalingY();
		return response;
	}

	remoteprotocol::WindowSizingConfig GuiRemoteRendererSingle::GetWindowSizingConfig()
	{
		WindowSizingConfig response;
		response.bounds = window->GetBounds();
		response.clientBounds = window->GetClientBoundsInScreen();
		response.sizeState = window->GetSizeState();
		response.customFramePadding = window->GetCustomFramePadding();
		return response;
	}

	void GuiRemoteRendererSingle::UpdateConfigsIfNecessary()
	{
		if (!CanSendEvents()) return;
		if (screen)
		{
			auto currentScreen = GetCurrentController()->ScreenService()->GetScreen(window);
			if (screen != currentScreen)
			{
				screen = currentScreen;
				events->OnControllerScreenUpdated(GetScreenConfig(screen));
			}

			auto newWindowSizingConfig = GetWindowSizingConfig();
			if (
				newWindowSizingConfig.bounds != windowSizingConfig.bounds ||
				newWindowSizingConfig.clientBounds != windowSizingConfig.clientBounds)
			{
				windowSizingConfig = newWindowSizingConfig;
				if (!updatingBounds)
				{
					events->OnWindowBoundsUpdated(windowSizingConfig);
				}
			}
			else if (
				newWindowSizingConfig.sizeState != windowSizingConfig.sizeState ||
				newWindowSizingConfig.customFramePadding != windowSizingConfig.customFramePadding)
			{
				windowSizingConfig = newWindowSizingConfig;
				events->OnWindowBoundsUpdated(windowSizingConfig);
			}
		}
	}

	void GuiRemoteRendererSingle::NativeWindowDestroying(INativeWindow* _window)
	{
		if (window == _window)
		{
			window->UninstallListener(this);
			window = nullptr;
		}
	}

	void GuiRemoteRendererSingle::Opened()
	{
		if (!CanSendEvents()) return;
		vl::presentation::remoteprotocol::ControllerGlobalConfig globalConfig;
#if defined VCZH_WCHAR_UTF16
		globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF16;
#elif defined VCZH_WCHAR_UTF32
		globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF32;
#endif
		events->OnControllerConnect(globalConfig);
	}

	void GuiRemoteRendererSingle::BeforeClosing(bool& cancel)
	{
		if (!disconnectingFromCore)
		{
			cancel = true;
			events->OnControllerRequestExit();
		}
	}

	void GuiRemoteRendererSingle::AfterClosing()
	{
		fatalMaskElement = nullptr;
		fatalTextElement = nullptr;
		renderingDom = nullptr;
		focusedParagraphElements.Clear();
		availableElements.Clear();
		availableImages.Clear();
	}

	void GuiRemoteRendererSingle::Closed()
	{
	}

	void GuiRemoteRendererSingle::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
	{
		NativeWindowListener_Moving(window, suggestedMinSize, bounds, fixSizeOnly, draggingBorder);
		if (disconnectingFromCore) return;
		if (draggingBorder)
		{
			auto config = GetWindowSizingConfig();
			auto dx1 = config.clientBounds.x1 - config.bounds.x1;
			auto dy1 = config.clientBounds.y1 - config.bounds.y1;
			auto dx2 = config.clientBounds.x2 - config.bounds.x2;
			auto dy2 = config.clientBounds.y2 - config.bounds.y2;

			config.bounds = bounds;
			config.clientBounds.x1 = config.bounds.x1 + dx1;
			config.clientBounds.y1 = config.bounds.y1 + dy1;
			config.clientBounds.x2 = config.bounds.x2 + dx2;
			config.clientBounds.y2 = config.bounds.y2 + dy2;

			pendingWindowBoundsUpdate = config;
		}
	}

	void GuiRemoteRendererSingle::Moved()
	{
		pendingWindowBoundsUpdate.Reset();
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::DpiChanged(bool preparing)
	{
		if (preparing)
		{
			UpdateRenderTarget(nullptr);
		}
		else
		{
			GetGuiGraphicsResourceManager()->RecreateRenderTarget(window);
			UpdateRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window));
			UpdateConfigsIfNecessary();
			if (stoppedByFatalError)
			{
				needRefresh = true;
				ForceRender();
			}
		}
	}

	void GuiRemoteRendererSingle::RenderingAsActivated()
	{
		if (disconnectingFromCore) return;
		events->OnWindowActivatedUpdated(true);
	}

	void GuiRemoteRendererSingle::RenderingAsDeactivated()
	{
		if (disconnectingFromCore) return;
		events->OnWindowActivatedUpdated(false);
	}

	GuiRemoteRendererSingle::GuiRemoteRendererSingle(bool _enabledAutomation)
		: enabledAutomation(_enabledAutomation)
	{
	}

	GuiRemoteRendererSingle::~GuiRemoteRendererSingle()
	{
	}

	void GuiRemoteRendererSingle::RegisterMainWindow(INativeWindow* _window)
	{
		window = _window;
		window->InstallListener(this);
		GetCurrentController()->CallbackService()->InstallListener(this);
	}

	void GuiRemoteRendererSingle::UnregisterMainWindow()
	{
		UnregisterGlobalShortcutKeys();
		GetCurrentController()->CallbackService()->UninstallListener(this);
	}

	bool GuiRemoteRendererSingle::IsDisconnectedFromCore()
	{
		return disconnectingFromCore;
	}

	void GuiRemoteRendererSingle::ForceExitByFatelError()
	{
		if (window)
		{
			DisconnectFromCore();
			window->Hide(true);
		}
	}

	void GuiRemoteRendererSingle::RetainByFatalError(const WString& errorMessage)
	{
		if (stoppedByFatalError) return;

		DisconnectFromCore();
		stoppedByFatalError = true;
		fatalError = errorMessage;
		supressRefresh = false;

		if (window)
		{
			titleBeforeFatalError = window->GetTitle();
			window->SetTitle(WString::Unmanaged(L"[STOPPED] ") + titleBeforeFatalError);
			needRefresh = true;
			ForceRender();
		}
	}

	WString GuiRemoteRendererSingle::GetExecutablePath()
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void GuiRemoteRendererSingle::Initialize(IGuiRemoteProtocolEvents* _events)
	{
		events = _events;
	}

	void GuiRemoteRendererSingle::Submit(bool& disconnected)
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	IGuiRemoteEventProcessor* GuiRemoteRendererSingle::GetRemoteEventProcessor()
	{
		return nullptr;
	}
}
