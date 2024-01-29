#include "TestRemote_ControlHost_Shared.h"

namespace remote_controlhost_mouse_tests
{
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
using namespace remote_controlhost_mouse_tests;

TEST_FILE
{
	TEST_CATEGORY(L"MouseEntered and MouseLeaved on ControlHost")
	{
		ControlHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		List<WString> eventLogs;

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();

			auto c = new GuiBoundsComposition();
			c->SetAlignmentToParent(Margin(0, 0, 0, 0));
			controlHost->GetContainerComposition()->AddChild(c);

			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			TEST_ASSERT(c->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));

			AttachMouseEvents(b, L"host.bounds", eventLogs);
			AttachMouseEvents(c, L"host.container", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOMouseEntered();
			protocol.events->OnIOMouseMoving(MakeMouseInfo(false, false, false, 320, 240, 0));
			protocol.events->OnIOMouseLeaved();

			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"host.container.Enter()",
				L"host.container.Move(:320,240,0)",
				L"host.bounds.Move(:320,240,0)",
				L"host.container.Leave()",
				L"host.bounds.Leave()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy([&]()
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
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO:
	//   Mouse event propogation and enter/leave/moving with @DropConsecutive/dbclick
	//   Mouse capturing
}