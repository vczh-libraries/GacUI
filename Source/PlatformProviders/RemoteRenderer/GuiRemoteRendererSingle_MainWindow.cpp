#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace remoteprotocol;

	void GuiRemoteRendererSingle::RequestWindowGetBounds(vint id)
	{
		events->RespondWindowGetBounds(id, GetWindowSizingConfig());
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBounds(const NativeRect& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestWindowNotifySetBounds(const NativeRect&)#"
		CHECK_ERROR(!updatingBounds, ERROR_MESSAGE_PREFIX L"This function cannot be called recursively.");

		updatingBounds = true;
		if (!screen)
		{
			auto primary = GetCurrentController()->ScreenService()->GetScreen((vint)0);
			NativeRect screenBounds = primary->GetBounds();
			NativeRect windowBounds = arguments;
			windowBounds.x1 = (screenBounds.Width() - windowBounds.Width()) / 2;
			windowBounds.y1 = (screenBounds.Height() - windowBounds.Height()) / 2;
			window->SetBounds(windowBounds);

			screen = primary;
			windowSizingConfig = GetWindowSizingConfig();
		}
		else
		{
			window->SetBounds(arguments);
		}
		updatingBounds = false;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTitle(const WString& arguments)
	{
		window->SetTitle(arguments);
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetEnabled(const bool& arguments)
	{
		if (arguments)
		{
			window->Enable();
		}
		else
		{
			window->Disable();
		}
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTopMost(const bool& arguments)
	{
		window->SetTopMost(arguments);
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetShowInTaskBar(const bool& arguments)
	{
		if (arguments)
		{
			window->ShowInTaskBar();
		}
		else
		{
			window->HideInTaskBar();
		}
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetClientSize(const NativeSize& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestWindowNotifySetClientSize(const NativeSize&)#"
		CHECK_ERROR(screen, ERROR_MESSAGE_PREFIX L"This function cannot be called before RequestWindowNotifySetBounds.");

		window->SetClientSize(arguments);
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetCustomFrameMode(const bool& arguments)
	{
		if (window->IsCustomFrameModeEnabled() != arguments)
		{
			if (arguments)
			{
				window->EnableCustomFrameMode();
			}
			else
			{
				window->DisableCustomFrameMode();
			}
			UpdateConfigsIfNecessary();
		}
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetMaximizedBox(const bool& arguments)
	{
		window->SetMaximizedBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetMinimizedBox(const bool& arguments)
	{
		window->SetMinimizedBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBorder(const bool& arguments)
	{
		window->SetBorder(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetSizeBox(const bool& arguments)
	{
		window->SetSizeBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetIconVisible(const bool& arguments)
	{
		window->SetIconVisible(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTitleBar(const bool& arguments)
	{
		window->SetTitleBar(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyActivate()
	{
		window->SetActivate();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyShow(const remoteprotocol::WindowShowing& arguments)
	{
		if (arguments.sizeState != window->GetSizeState())
		{
			if (arguments.activate)
			{
				window->SetActivate();
			}
			switch (arguments.sizeState)
			{
			case INativeWindow::Minimized:
				window->ShowMinimized();
				break;
			case INativeWindow::Restored:
				window->ShowRestored();
				break;
			case INativeWindow::Maximized:
				window->ShowMaximized();
				break;
			}
		}
	}
}