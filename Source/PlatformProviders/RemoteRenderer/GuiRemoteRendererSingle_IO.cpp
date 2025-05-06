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
		CHECK_FAIL(L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyToggled(vint id, const VKEY& arguments)
	{
		CHECK_FAIL(L"Not Implemented");
	}

/***********************************************************************
* Rendering (INativeWindow)
***********************************************************************/

	void GuiRemoteRendererSingle::LeftButtonDown(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonUp(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Left;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDown(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonUp(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Right;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDown(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDown(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonUp(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonUp(arguments);
	}

	void GuiRemoteRendererSingle::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
	{
		IOMouseInfoWithButton arguments;
		arguments.button = IOMouseButton::Middle;
		arguments.info = info;
		events->OnIOButtonDoubleClick(arguments);
	}

	void GuiRemoteRendererSingle::HorizontalWheel(const NativeWindowMouseInfo& info)
	{
		events->OnIOHWheel(info);
	}

	void GuiRemoteRendererSingle::VerticalWheel(const NativeWindowMouseInfo& info)
	{
		events->OnIOVWheel(info);
	}

	void GuiRemoteRendererSingle::MouseMoving(const NativeWindowMouseInfo& info)
	{
		if (renderingDom)
		{
			INativeWindowListener::HitTestResult hitTestResult = INativeWindowListener::NoDecision;
			INativeCursor* cursor = nullptr;
			HitTest(renderingDom, window->Convert(NativePoint{ info.x,info.y }), hitTestResult, cursor);
			window->SetWindowCursor(cursor);
		}
		events->OnIOMouseMoving(info);
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
		events->OnIOKeyDown(info);
	}

	void GuiRemoteRendererSingle::KeyUp(const NativeWindowKeyInfo& info)
	{
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
}