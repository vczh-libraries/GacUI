#include "TestRemote_ControlHost_Shared.h"

namespace remote_control_host_tests
{
/***********************************************************************
MakeGuiMain
***********************************************************************/

	Func<void()> MakeGuiMain(ControlHostProtocol& protocol, List<WString>& eventLogs, GuiWindow*& controlHost)
	{
		return [&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and destroy a control host")
			{
				auto theme = Ptr(new EmptyControlTheme);
				theme::RegisterTheme(theme);

				GuiWindow window(theme::ThemeName::Window);
				window.SetClientSize({ 640,480 });
				controlHost = &window;
				GetApplication()->Run(&window);
				controlHost = nullptr;

				theme::UnregisterTheme(theme->Name);
				AssertEventLogs(eventLogs);
			});
		};
	}

/***********************************************************************
AttachMouseEvents
***********************************************************************/

	void AttachMouseEvent(GuiNotifyEvent& event, const wchar_t* senderName, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
		{
			eventLogs.Add(WString::Unmanaged(senderName) + WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"()"));
		});
	}

	void AttachMouseEvent(GuiMouseEvent& event, const wchar_t* senderName, const wchar_t* eventName, List<WString>& eventLogs)
	{
		event.AttachLambda([=, &event, &eventLogs](GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
		{
			eventLogs.Add(WString::Unmanaged(senderName) + WString::Unmanaged(L".") + WString::Unmanaged(eventName) + WString::Unmanaged(L"(")
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

	void AttachMouseEvents(GuiGraphicsComposition* sender, const wchar_t* name, List<WString>& eventLogs)
	{
		auto e = sender->GetEventReceiver();
		AttachMouseEvent(e->mouseEnter, name, L"Enter", eventLogs);
		AttachMouseEvent(e->mouseLeave, name, L"Leave", eventLogs);
		AttachMouseEvent(e->mouseMove, name, L"Move", eventLogs);
		AttachMouseEvent(e->horizontalWheel, name, L"HWheel", eventLogs);
		AttachMouseEvent(e->verticalWheel, name, L"VWheel", eventLogs);
		AttachMouseEvent(e->leftButtonDown, name, L"LDown", eventLogs);
		AttachMouseEvent(e->leftButtonUp, name, L"LUp", eventLogs);
		AttachMouseEvent(e->leftButtonDoubleClick, name, L"LDbClick", eventLogs);
		AttachMouseEvent(e->middleButtonDown, name, L"MDown", eventLogs);
		AttachMouseEvent(e->middleButtonUp, name, L"MUp", eventLogs);
		AttachMouseEvent(e->middleButtonDoubleClick, name, L"MDbClick", eventLogs);
		AttachMouseEvent(e->rightButtonDown, name, L"RDown", eventLogs);
		AttachMouseEvent(e->rightButtonUp, name, L"RUp", eventLogs);
		AttachMouseEvent(e->rightButtonDoubleClick, name, L"RDbClick", eventLogs);
	}

/***********************************************************************
MakeMouseInfo
***********************************************************************/

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
using namespace remote_control_host_tests;