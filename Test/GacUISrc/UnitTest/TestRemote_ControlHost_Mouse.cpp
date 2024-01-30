#include "TestRemote_ControlHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"MouseEntered and MouseLeaved on ControlHost")
	{
		ControlHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;

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

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Mouse events on compositions")
	{
		ControlHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();

			auto x = new GuiBoundsComposition();
			x->SetExpectedBounds(Rect({ 10,10 }, { 100,100 }));

			auto y = new GuiBoundsComposition();
			y->SetExpectedBounds(Rect({ 10,10 }, { 80,80}));

			auto z = new GuiBoundsComposition();
			z->SetExpectedBounds(Rect({ 60,60 }, { 100,100 }));

			x->AddChild(y);
			controlHost->GetContainerComposition()->AddChild(x);
			controlHost->GetContainerComposition()->AddChild(z);

			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			TEST_ASSERT(x->GetCachedBounds() == Rect({ 10,10 }, { 100,100 }));
			TEST_ASSERT(y->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			TEST_ASSERT(z->GetCachedBounds() == Rect({ 60,60 }, { 100,100 }));

			AttachMouseEvents(b, L"host.bounds", eventLogs);
			AttachMouseEvents(x, L"x", eventLogs);
			AttachMouseEvents(y, L"y", eventLogs);
			AttachMouseEvents(z, L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOMouseEntered();
			protocol.events->OnIOMouseMoving(MakeMouseInfo(false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"x.Move(:20,20,0)",
				L"host.bounds.Move(:30,30,0)"
				);

			protocol.events->OnIOButtonDown(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Left, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Left, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonDoubleClick(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Left, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Left, false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"y.LDown(:10,10,0)",
				L"x.LDown(:20,20,0)",
				L"host.bounds.LDown(:30,30,0)",

				L"y.LUp(:10,10,0)",
				L"x.LUp(:20,20,0)",
				L"host.bounds.LUp(:30,30,0)",

				L"y.LDown(:10,10,0)",
				L"x.LDown(:20,20,0)",
				L"host.bounds.LDown(:30,30,0)",

				L"y.LDbClick(:10,10,0)",
				L"x.LDbClick(:20,20,0)",
				L"host.bounds.LDbClick(:30,30,0)",

				L"y.LUp(:10,10,0)",
				L"x.LUp(:20,20,0)",
				L"host.bounds.LUp(:30,30,0)"
				);

			protocol.events->OnIOMouseMoving(MakeMouseInfo(false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"y.Leave()",
				L"x.Leave()",
				L"z.Enter()",
				L"z.Move(:10,10,0)",
				L"host.bounds.Move(:70,70,0)"
				);

			protocol.events->OnIOButtonDown(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Middle, false, false, false, 70, 70, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Middle, false, false, false, 70, 70, 0));
			protocol.events->OnIOButtonDoubleClick(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Middle, false, false, false, 70, 70, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Middle, false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"z.MDown(:10,10,0)",
				L"host.bounds.MDown(:70,70,0)",

				L"z.MUp(:10,10,0)",
				L"host.bounds.MUp(:70,70,0)",

				L"z.MDown(:10,10,0)",
				L"host.bounds.MDown(:70,70,0)",

				L"z.MDbClick(:10,10,0)",
				L"host.bounds.MDbClick(:70,70,0)",

				L"z.MUp(:10,10,0)",
				L"host.bounds.MUp(:70,70,0)"
				);

			protocol.events->OnIOMouseMoving(MakeMouseInfo(false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"z.Leave()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"x.Move(:20,20,0)",
				L"host.bounds.Move(:30,30,0)"
				);

			protocol.events->OnIOButtonDown(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Right, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Right, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonDoubleClick(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Right, false, false, false, 30, 30, 0));
			protocol.events->OnIOButtonUp(MakeMouseInfoWithButton(remoteprotocol::IOMouseButton::Right, false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"y.RDown(:10,10,0)",
				L"x.RDown(:20,20,0)",
				L"host.bounds.RDown(:30,30,0)",

				L"y.RUp(:10,10,0)",
				L"x.RUp(:20,20,0)",
				L"host.bounds.RUp(:30,30,0)",

				L"y.RDown(:10,10,0)",
				L"x.RDown(:20,20,0)",
				L"host.bounds.RDown(:30,30,0)",

				L"y.RDbClick(:10,10,0)",
				L"x.RDbClick(:20,20,0)",
				L"host.bounds.RDbClick(:30,30,0)",

				L"y.RUp(:10,10,0)",
				L"x.RUp(:20,20,0)",
				L"host.bounds.RUp(:30,30,0)"
				);

			protocol.events->OnIOMouseMoving(MakeMouseInfo(false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"y.Leave()",
				L"x.Leave()",
				L"z.Enter()",
				L"z.Move(:10,10,0)",
				L"host.bounds.Move(:70,70,0)"
				);

			protocol.events->OnIOHWheel(MakeMouseInfo(false, false, false, 70, 70, 0));
			protocol.events->OnIOVWheel(MakeMouseInfo(false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"z.HWheel(:10,10,0)",
				L"host.bounds.HWheel(:70,70,0)",
				L"z.VWheel(:10,10,0)",
				L"host.bounds.VWheel(:70,70,0)"
				);

			protocol.events->OnIOMouseLeaved();
			AssertEventLogs(
				eventLogs,
				L"z.Leave()",
				L"host.bounds.Leave()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO:
	//   Mouse event propogation and enter/leave/moving with @DropConsecutive/dbclick
	//   Mouse capturing
	//   log sender/compositionSource/eventSource
}