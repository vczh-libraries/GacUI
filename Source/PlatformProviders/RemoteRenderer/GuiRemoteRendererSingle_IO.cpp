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
				vint id = inputService->RegisterGlobalShortcutKey(shortcut.ctrl, shortcut.shift, shortcut.alt, shortcut.osSuper, shortcut.code);
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


	void GuiRemoteRendererSingle::PointSequenceStarted(const NativeWindowPointInfo& firstPoint, bool& accept)
	{
		accept = true;
		if (!CanSendEvents()) return;
		SendAccumulatedMessages();
		pointSequenceId = firstPoint.sequenceId;
		pointEventIndex = 0;
		IOPointAction arguments;
		arguments.eventIndex = pointEventIndex;
		arguments.info = EncodePointInfo(firstPoint);
		events->OnIOPointStarted(arguments);
	}

	void GuiRemoteRendererSingle::SendPointAction(NativePointEvent event, const NativeWindowPointInfo& info)
	{
		if (!CanSendEvents() || pointSequenceId == 0 || info.sequenceId != pointSequenceId) return;
		CHECK_ERROR(pointEventIndex < 0x7FFFFFFF, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::SendPointAction#Point event indices exhausted.");
		SendAccumulatedMessages();
		IOPointAction arguments;
		arguments.eventIndex = ++pointEventIndex;
		arguments.info = EncodePointInfo(info);
		switch (event)
		{
		case NativePointEvent::Down: events->OnIOPointDown(arguments); break;
		case NativePointEvent::Update: events->OnIOPointUpdate(arguments); break;
		case NativePointEvent::Up: events->OnIOPointUp(arguments); break;
		case NativePointEvent::Entered: events->OnIOPointEnter(arguments); break;
		case NativePointEvent::Leaved: events->OnIOPointLeave(arguments); break;
		}
	}

	void GuiRemoteRendererSingle::PointSequenceEnded(const PointSequenceEndInfo& info)
	{
		if (!CanSendEvents() || pointSequenceId == 0 || info.sequenceId != pointSequenceId) return;
		CHECK_ERROR(pointEventIndex < 0x7FFFFFFF, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::PointSequenceEnded#Point event indices exhausted.");
		SendAccumulatedMessages();
		IOPointEnd arguments;
		arguments.sequenceId = u64tow(info.sequenceId);
		arguments.eventIndex = ++pointEventIndex;
		arguments.timestamp = u64tow(info.timestamp);
		arguments.accepted = info.accepted;
		arguments.reason = info.reason;
		events->OnIOPointEnded(arguments);
		pointSequenceId = 0;
	}

	void GuiRemoteRendererSingle::PointDown(const NativeWindowPointInfo& info)
	{
		SendPointAction(NativePointEvent::Down, info);
	}

	void GuiRemoteRendererSingle::PointUpdate(const NativeWindowPointInfo& info)
	{
		SendPointAction(NativePointEvent::Update, info);
	}

	void GuiRemoteRendererSingle::PointUp(const NativeWindowPointInfo& info)
	{
		SendPointAction(NativePointEvent::Up, info);
	}

	void GuiRemoteRendererSingle::PointEntered(const NativeWindowPointInfo& info)
	{
		SendPointAction(NativePointEvent::Entered, info);
	}

	void GuiRemoteRendererSingle::PointLeaved(const NativeWindowPointInfo& info)
	{
		SendPointAction(NativePointEvent::Leaved, info);
	}

	void GuiRemoteRendererSingle::MouseDown(NativeMouseButton button, const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		SendAccumulatedMessages();
		IOMouseInfoWithButton arguments;
		arguments.button = button;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::MouseUp(NativeMouseButton button, const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		SendAccumulatedMessages();
		IOMouseInfoWithButton arguments;
		arguments.button = button;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::MouseDoubleClick(NativeMouseButton button, const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		SendAccumulatedMessages();
		IOMouseInfoWithButton arguments;
		arguments.button = button;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::HorizontalWheel(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		if (pendingHWheel && (pendingHWheel.Value().alt != info.alt || pendingHWheel.Value().osSuper != info.osSuper)) SendAccumulatedMessages();
		auto copy = info;
		if (pendingHWheel) copy.wheel += pendingHWheel.Value().wheel;
		pendingHWheel = copy;
	}

	void GuiRemoteRendererSingle::VerticalWheel(const NativeWindowMouseInfo& info)
	{
		if (!CanSendEvents()) return;
		if (pendingVWheel && (pendingVWheel.Value().alt != info.alt || pendingVWheel.Value().osSuper != info.osSuper)) SendAccumulatedMessages();
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
		if (!info.ctrl && !info.shift && !info.osSuper && info.code == VKEY::KEY_MENU)
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
