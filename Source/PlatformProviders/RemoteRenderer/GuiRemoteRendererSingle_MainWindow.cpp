#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace remoteprotocol;

	void GuiRemoteRendererSingle::RequestWindowGetBounds(vint id)
	{
		if (!CanSendEvents()) return;
		events->RespondWindowGetBounds(id, GetWindowSizingConfig());
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBounds(const NativeRect& arguments)
	{
		if (disconnectingFromCore) return;
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestWindowNotifySetBounds(const NativeRect&)#"
		CHECK_ERROR(!updatingBounds, ERROR_MESSAGE_PREFIX L"This function cannot be called recursively.");

		updatingBounds = true;
		if (!screen)
		{
			auto primary = GetCurrentController()->ScreenService()->GetScreen((vint)0);
			NativeRect screenBounds = primary->GetBounds();
			auto x = (screenBounds.Width() - arguments.Width()) / 2;
			auto y = (screenBounds.Height() - arguments.Height()) / 2;
			window->SetBounds({ {x,y},arguments.GetSize() });

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
		if (disconnectingFromCore) return;
		window->SetTitle(arguments);
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetEnabled(const bool& arguments)
	{
		if (disconnectingFromCore) return;
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
		if (disconnectingFromCore) return;
		window->SetTopMost(arguments);
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetShowInTaskBar(const bool& arguments)
	{
		if (disconnectingFromCore) return;
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
		if (disconnectingFromCore) return;
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestWindowNotifySetClientSize(const NativeSize&)#"
		CHECK_ERROR(screen, ERROR_MESSAGE_PREFIX L"This function cannot be called before RequestWindowNotifySetBounds.");

		window->SetClientSize(arguments);
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetCustomFrameMode(const bool& arguments)
	{
		if (disconnectingFromCore) return;
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
		if (disconnectingFromCore) return;
		window->SetMaximizedBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetMinimizedBox(const bool& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetMinimizedBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBorder(const bool& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetBorder(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetSizeBox(const bool& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetSizeBox(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetIconVisible(const bool& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetIconVisible(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTitleBar(const bool& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetTitleBar(arguments);
		UpdateConfigsIfNecessary();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyActivate()
	{
		if (disconnectingFromCore) return;
		window->SetActivate();
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyShow(const remoteprotocol::WindowShowing& arguments)
	{
		if (disconnectingFromCore) return;
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

	void GuiRemoteRendererSingle::RequestWindowNotifyMinSize(const NativeSize& arguments)
	{
		if (disconnectingFromCore) return;
		auto clientSize = window->GetClientSize();
		auto size = window->GetBounds().GetSize();
		suggestedMinSize.x = arguments.x + size.x - clientSize.x;
		suggestedMinSize.y = arguments.y + size.y - clientSize.y;
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetCaret(const NativePoint& arguments)
	{
		if (disconnectingFromCore) return;
		window->SetCaretPoint(arguments);
	}
}
