#include "TestRemote_GraphicsHost_Shared.h"

namespace remote_graphics_host_tests
{
	class BoxedString : public Object
	{
	public:
		WString value;

		BoxedString(WString&& _value)
			: value(std::move(_value))
		{
		}
	};

	WString GetRaiserName(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
		WString& senderName = sender->GetInternalProperty(WString::Unmanaged(L"Name")).Cast<BoxedString>()->value;
		WString& sourceName = arguments.eventSource->GetInternalProperty(WString::Unmanaged(L"Name")).Cast<BoxedString>()->value;
		return sender == arguments.eventSource ? senderName : sourceName + L"->" + senderName;
	}

/***********************************************************************
AttachAndLogEvents
***********************************************************************/

	void AttachNotifyEvent(GuiGraphicsComposition* eventOwner, GuiNotifyEvent& event, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
		{
			TEST_ASSERT(eventOwner == sender);
			TEST_ASSERT(arguments.compositionSource == arguments.eventSource);
			eventLogs.Add(
				GetRaiserName(sender, arguments)
				+ WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"()"));
		});
	}

	void AttachMouseEvent(GuiGraphicsComposition* eventOwner, GuiMouseEvent& event, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
		{
			TEST_ASSERT(eventOwner == sender);
			TEST_ASSERT(
				arguments.compositionSource == arguments.eventSource ||
				dynamic_cast<GuiWindow*>(arguments.eventSource->GetAssociatedControl())
				);
			eventLogs.Add(
				GetRaiserName(sender, arguments)
				+ WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"(")
				+ (arguments.ctrl ? WString::Unmanaged(L"C") : WString::Empty)
				+ (arguments.shift ? WString::Unmanaged(L"S") : WString::Empty)
				+ (arguments.left ? WString::Unmanaged(L"L") : WString::Empty)
				+ (arguments.middle ? WString::Unmanaged(L"M") : WString::Empty)
				+ (arguments.right ? WString::Unmanaged(L"R") : WString::Empty)
				+ WString::Unmanaged(L":") + itow(arguments.x)
				+ WString::Unmanaged(L",") + itow(arguments.y)
				+ WString::Unmanaged(L",") + itow(arguments.wheel)
				+ WString::Unmanaged(L")"));
		});
	}

	void AttachKeyEvent(GuiGraphicsComposition* eventOwner, GuiKeyEvent& event, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiKeyEventArgs& arguments)
		{
			TEST_ASSERT(eventOwner == sender);
			TEST_ASSERT(arguments.compositionSource == arguments.eventSource);
			eventLogs.Add(
				GetRaiserName(sender, arguments)
				+ WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"(")
				+ (arguments.ctrl ? WString::Unmanaged(L"C") : WString::Empty)
				+ (arguments.shift ? WString::Unmanaged(L"S") : WString::Empty)
				+ (arguments.alt ? WString::Unmanaged(L"A") : WString::Empty)
				+ (arguments.capslock ? WString::Unmanaged(L"^") : WString::Empty)
				+ (arguments.autoRepeatKeyDown ? WString::Unmanaged(L"~") : WString::Empty)
				+ WString::Unmanaged(L":") + GetCurrentController()->InputService()->GetKeyName(arguments.code)
				+ WString::Unmanaged(L")"));
		});
	}

	void AttachCharEvent(GuiGraphicsComposition* eventOwner, GuiCharEvent& event, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiCharEventArgs& arguments)
		{
			TEST_ASSERT(eventOwner == sender);
			TEST_ASSERT(arguments.compositionSource == arguments.eventSource);
			eventLogs.Add(
				GetRaiserName(sender, arguments)
				+ WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"(")
				+ (arguments.ctrl ? WString::Unmanaged(L"C") : WString::Empty)
				+ (arguments.shift ? WString::Unmanaged(L"S") : WString::Empty)
				+ (arguments.alt ? WString::Unmanaged(L"A") : WString::Empty)
				+ (arguments.capslock ? WString::Unmanaged(L"^") : WString::Empty)
				+ WString::Unmanaged(L":") + WString::FromChar(arguments.code)
				+ WString::Unmanaged(L")"));
		});
	}

	void AttachAndLogEvents(GuiGraphicsComposition* sender, const wchar_t* name, List<WString>& eventLogs)
	{
		sender->SetInternalProperty(WString::Unmanaged(L"Name"), Ptr(new BoxedString(WString::Unmanaged(name))));
		auto e = sender->GetEventReceiver();
		AttachNotifyEvent(sender, e->mouseEnter, L"Enter", eventLogs);
		AttachNotifyEvent(sender, e->mouseLeave, L"Leave", eventLogs);
		AttachMouseEvent(sender, e->mouseMove, L"Move", eventLogs);
		AttachMouseEvent(sender, e->horizontalWheel, L"HWheel", eventLogs);
		AttachMouseEvent(sender, e->verticalWheel, L"VWheel", eventLogs);
		AttachMouseEvent(sender, e->leftButtonDown, L"LDown", eventLogs);
		AttachMouseEvent(sender, e->leftButtonUp, L"LUp", eventLogs);
		AttachMouseEvent(sender, e->leftButtonDoubleClick, L"LDbClick", eventLogs);
		AttachMouseEvent(sender, e->middleButtonDown, L"MDown", eventLogs);
		AttachMouseEvent(sender, e->middleButtonUp, L"MUp", eventLogs);
		AttachMouseEvent(sender, e->middleButtonDoubleClick, L"MDbClick", eventLogs);
		AttachMouseEvent(sender, e->rightButtonDown, L"RDown", eventLogs);
		AttachMouseEvent(sender, e->rightButtonUp, L"RUp", eventLogs);
		AttachMouseEvent(sender, e->rightButtonDoubleClick, L"RDbClick", eventLogs);
		AttachKeyEvent(sender, e->previewKey, L"KeyPreview", eventLogs);
		AttachKeyEvent(sender, e->keyDown, L"KeyDown", eventLogs);
		AttachKeyEvent(sender, e->keyUp, L"KeyUp", eventLogs);
		AttachCharEvent(sender, e->previewCharInput, L"CharPreview", eventLogs);
		AttachCharEvent(sender, e->charInput, L"Char", eventLogs);
		AttachNotifyEvent(sender, e->gotFocus, L"GotFocus", eventLogs);
		AttachNotifyEvent(sender, e->lostFocus, L"LostFocus", eventLogs);
	}

/***********************************************************************
Make*Info
***********************************************************************/

	NativeWindowKeyInfo MakeKeyInfo(bool ctrl, bool shift, bool alt, VKEY code)
	{
		NativeWindowKeyInfo info;
		info.ctrl = ctrl;
		info.shift = shift;
		info.alt = alt;
		info.capslock = false;
		info.autoRepeatKeyDown = false;
		info.code = code;
		return info;
	}

	NativeWindowCharInfo MakeCharInfo(bool ctrl, bool shift, bool alt, wchar_t code)
	{
		NativeWindowCharInfo info;
		info.ctrl = ctrl;
		info.shift = shift;
		info.alt = alt;
		info.capslock = false;
		info.code = code;
		return info;
	}

	NativeWindowMouseInfo MakeMouseInfo(bool left, bool middle, bool right, vint x, vint y, vint wheel)
	{
		NativeWindowMouseInfo info;
		info.ctrl = false;
		info.shift = false;
		info.left = left;
		info.middle = middle;
		info.right = right;
		info.nonClient = false;
		info.x = x;
		info.y = y;
		info.wheel = wheel;
		return info;
	}

	remoteprotocol::IOMouseInfoWithButton MakeMouseInfoWithButton(remoteprotocol::IOMouseButton button, bool left, bool middle, bool right, vint x, vint y, vint wheel)
	{
		IOMouseInfoWithButton info;
		info.button = button;
		info.info = MakeMouseInfo(left, middle, right, x, y, wheel);
		return info;
	}
}