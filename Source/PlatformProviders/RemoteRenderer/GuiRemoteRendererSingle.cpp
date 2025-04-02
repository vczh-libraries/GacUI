#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace elements;
	using namespace remoteprotocol;

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
		if (screen)
		{
			auto currentScreen = GetCurrentController()->ScreenService()->GetScreen(window);
			if (screen != currentScreen)
			{
				screen = currentScreen;
				events->OnControllerScreenUpdated(GetScreenConfig(screen));
			}

			auto newWindowSizingConfig = GetWindowSizingConfig();
			if (newWindowSizingConfig.bounds != windowSizingConfig.bounds)
			{
				windowSizingConfig = newWindowSizingConfig;
				if (!updatingBounds)
				{
					events->OnWindowBoundsUpdated(windowSizingConfig);
				}
			}
			else if (
				newWindowSizingConfig.clientBounds != windowSizingConfig.clientBounds ||
				newWindowSizingConfig.sizeState != windowSizingConfig.sizeState ||
				newWindowSizingConfig.customFramePadding != windowSizingConfig.customFramePadding)
			{
				windowSizingConfig = newWindowSizingConfig;
				events->OnWindowBoundsUpdated(windowSizingConfig);
			}
		}
	}

	void GuiRemoteRendererSingle::Opened()
	{
		events->OnControllerConnect();
	}

	void GuiRemoteRendererSingle::Moved()
	{
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
		}
	}

	void GuiRemoteRendererSingle::RenderingAsActivated()
	{
		events->OnWindowActivatedUpdated(true);
	}

	void GuiRemoteRendererSingle::RenderingAsDeactivated()
	{
		events->OnWindowActivatedUpdated(false);
	}

	GuiRemoteRendererSingle::GuiRemoteRendererSingle()
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
		GetCurrentController()->CallbackService()->UninstallListener(this);
		window->UninstallListener(this);
		window = nullptr;
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

	void GuiRemoteRendererSingle::ProcessRemoteEvents()
	{
		CHECK_FAIL(L"This function should not be called!");
	}
}