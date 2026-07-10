#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Commands)
***********************************************************************/

	void GuiRemoteRendererSingle::UnregisterGlobalShortcutKeys()
	{
		auto inputService = GetCurrentController()->InputService();
		for (vint id : globalShortcuts.Keys())
		{
			inputService->UnregisterGlobalShortcutKey(id);
		}
		globalShortcuts.Clear();

	}

	void GuiRemoteRendererSingle::GlobalShortcutKeyActivated(vint id)
	{
		if (!CanSendEvents()) return;
		vint index = globalShortcuts.Keys().IndexOf(id);
		if (index != -1)
		{
			events->OnIOGlobalShortcutKey(globalShortcuts.Values()[index].id);
		}
	}

	void GuiRemoteRendererSingle::RequestIOUpdateGlobalShortcutKey(const Ptr<collections::List<remoteprotocol::GlobalShortcutKey>>& arguments)
	{
		if (disconnectingFromCore) return;
		UnregisterGlobalShortcutKeys();
		if (arguments)
		{
			auto inputService = GetCurrentController()->InputService();
			for (auto&& shortcut : *arguments.Obj())
			{
				vint id = inputService->RegisterGlobalShortcutKey(shortcut.ctrl, shortcut.shift, shortcut.alt, shortcut.code);
				if (id != -1)
				{
					globalShortcuts.Add(id, shortcut);
				}
			}
		}
	}

	void GuiRemoteRendererSingle::RequestIORequireCapture()
	{
		if (disconnectingFromCore) return;
		window->RequireCapture();
	}

	void GuiRemoteRendererSingle::RequestIOReleaseCapture()
	{
		if (disconnectingFromCore) return;
		window->ReleaseCapture();
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyPressing(vint id, const VKEY& arguments)
	{
		if (!CanSendEvents()) return;
		bool result = GetCurrentController()->InputService()->IsKeyPressing(arguments);
		events->RespondIOIsKeyPressing(id, result);
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyToggled(vint id, const VKEY& arguments)
	{
		if (!CanSendEvents()) return;
		bool result = GetCurrentController()->InputService()->IsKeyToggled(arguments);
		events->RespondIOIsKeyToggled(id, result);
	}

/***********************************************************************
* Rendering (INativeWindow)
***********************************************************************/

	void GuiRemoteRendererSingle::LeftButtonDown(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonUp(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDown(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonUp(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDown(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonUp(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::HorizontalWheel(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		auto copy = info;
		if (pendingHWheel) copy.wheel += pendingHWheel.Value().wheel;
		pendingHWheel = copy;
	}

	void GuiRemoteRendererSingle::VerticalWheel(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		auto copy = info;
		if (pendingVWheel) copy.wheel += pendingVWheel.Value().wheel;
		pendingVWheel = copy;
	}

	void GuiRemoteRendererSingle::MouseMoving(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingMouseMove = info;
		if (renderingDom)
		{
			INativeWindowListener::HitTestResult hitTestResult = INativeWindowListener::NoDecision;
			INativeCursor* cursor = nullptr;
			HitTest(renderingDom, window->Convert(NativePoint{ info.x,info.y }), hitTestResult, cursor);
			window->SetWindowCursor(cursor);
		}
	}

	void GuiRemoteRendererSingle::MouseEntered()
	{
		if (!CanSendEvents()) return;
		events->OnIOMouseEntered();
	}

	void GuiRemoteRendererSingle::MouseLeaved()
	{
		if (!CanSendEvents()) return;
		events->OnIOMouseLeaved();
	}

	void GuiRemoteRendererSingle::KeyDown(const NativeWindowKeyInfo& info)
	{
		if (!CanSendEvents()) return;
		if (info.autoRepeatKeyDown)
		{
			pendingKeyAutoDown = info;
		}
		else
		{
			events->OnIOKeyDown(info);
		}
	}

	void GuiRemoteRendererSingle::KeyUp(const NativeWindowKeyInfo& info)
	{
		if (!CanSendEvents()) return;
		pendingKeyAutoDown.Reset();
		if (!info.ctrl && !info.shift && info.code == VKEY::KEY_MENU)
		{
			window->SupressAlt();
		}
		events->OnIOKeyUp(info);
	}

	void GuiRemoteRendererSingle::Char(const NativeWindowCharInfo& info)
	{
		if (!CanSendEvents()) return;
		events->OnIOChar(info);
	}

	void GuiRemoteRendererSingle::SendAccumulatedMessages()
	{
		if (!CanSendEvents())
		{
			ClearPendingCoreEvents();
			return;
		}
		if (pendingMouseMove)
		{
			events->OnIOMouseMoving(pendingMouseMove.Value());
			pendingMouseMove.Reset();
		}
		if (pendingHWheel)
		{
			events->OnIOHWheel(pendingHWheel.Value());
			pendingHWheel.Reset();
		}
		if (pendingVWheel)
		{
			events->OnIOVWheel(pendingVWheel.Value());
			pendingVWheel.Reset();
		}
		if (pendingKeyAutoDown)
		{
			events->OnIOKeyDown(pendingKeyAutoDown.Value());
			pendingKeyAutoDown.Reset();
		}
		if (pendingWindowBoundsUpdate)
		{
			events->OnWindowBoundsUpdated(pendingWindowBoundsUpdate.Value());
			pendingWindowBoundsUpdate.Reset();
		}
	}
}
