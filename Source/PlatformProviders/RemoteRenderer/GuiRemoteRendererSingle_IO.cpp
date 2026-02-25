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
		vint index = globalShortcuts.Keys().IndexOf(id);
		if (index != -1)
		{
			events->OnIOGlobalShortcutKey(globalShortcuts.Values()[index].id);
		}
	}

	void GuiRemoteRendererSingle::RequestIOUpdateGlobalShortcutKey(const Ptr<collections::List<remoteprotocol::GlobalShortcutKey>>& arguments)
	{
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
		window->RequireCapture();
	}

	void GuiRemoteRendererSingle::RequestIOReleaseCapture()
	{
		window->ReleaseCapture();
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyPressing(vint id, const VKEY& arguments)
	{
		bool result = GetCurrentController()->InputService()->IsKeyPressing(arguments);
		events->RespondIOIsKeyPressing(id, result);
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyToggled(vint id, const VKEY& arguments)
	{
		bool result = GetCurrentController()->InputService()->IsKeyToggled(arguments);
		events->RespondIOIsKeyToggled(id, result);
	}

/***********************************************************************
* Rendering (INativeWindow)
***********************************************************************/

	void GuiRemoteRendererSingle::LeftButtonDown(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonUp(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDown(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonUp(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDown(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonUp(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		pendingMouseMove.Reset();
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::HorizontalWheel(const NativeWindowMouseInfo& info)
	{
		auto copy = info;
		if (pendingHWheel) copy.wheel += pendingHWheel.Value().wheel;
		pendingHWheel = copy;
	}

	void GuiRemoteRendererSingle::VerticalWheel(const NativeWindowMouseInfo& info)
	{
		auto copy = info;
		if (pendingVWheel) copy.wheel += pendingVWheel.Value().wheel;
		pendingVWheel = copy;
	}

	void GuiRemoteRendererSingle::MouseMoving(const NativeWindowMouseInfo& info)
	{
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
		events->OnIOMouseEntered();
	}

	void GuiRemoteRendererSingle::MouseLeaved()
	{
		events->OnIOMouseLeaved();
	}

	void GuiRemoteRendererSingle::KeyDown(const NativeWindowKeyInfo& info)
	{
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
		pendingKeyAutoDown.Reset();
		if (!info.ctrl && !info.shift && info.code == VKEY::KEY_MENU)
		{
			window->SupressAlt();
		}
		events->OnIOKeyUp(info);
	}

	void GuiRemoteRendererSingle::Char(const NativeWindowCharInfo& info)
	{
		events->OnIOChar(info);
	}

	void GuiRemoteRendererSingle::SendAccumulatedMessages()
	{
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