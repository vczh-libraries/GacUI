#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"MouseEntered and MouseLeaved on ControlHost")
	{
		GraphicsHostProtocol protocol;
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

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(c, L"host.container", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOMouseEntered();
			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(false, false, false, 320, 240, 0));
			protocol.GetEvents()->OnIOMouseLeaved();

			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"host.container.Enter()",
				L"host.container.Move(:320,240,0)",
				L"host.container->host.bounds.Move(:320,240,0)",
				L"host.container.Leave()",
				L"host.bounds.Leave()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Mouse events on compositions")
	{
		GraphicsHostProtocol protocol;
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

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(x, L"x", eventLogs);
			AttachAndLogEvents(y, L"y", eventLogs);
			AttachAndLogEvents(z, L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.MouseMove(NativePoint{ 30,30 });
			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"y->x.Move(:20,20,0)",
				L"y->host.bounds.Move(:30,30,0)"
				);

			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"y.LDown(L:10,10,0)",
				L"y->x.LDown(L:20,20,0)",
				L"y->host.bounds.LDown(L:30,30,0)",

				L"y.LUp(:10,10,0)",
				L"y->x.LUp(:20,20,0)",
				L"y->host.bounds.LUp(:30,30,0)",

				L"y.LDown(L:10,10,0)",
				L"y->x.LDown(L:20,20,0)",
				L"y->host.bounds.LDown(L:30,30,0)",
				L"y.LDbClick(L:10,10,0)",
				L"y->x.LDbClick(L:20,20,0)",
				L"y->host.bounds.LDbClick(L:30,30,0)",

				L"y.LUp(:10,10,0)",
				L"y->x.LUp(:20,20,0)",
				L"y->host.bounds.LUp(:30,30,0)"
				);

			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"y.Leave()",
				L"x.Leave()",
				L"z.Enter()",
				L"z.Move(:10,10,0)",
				L"z->host.bounds.Move(:70,70,0)"
				);

			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, true, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, false, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, true, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, true, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"z.MDown(M:10,10,0)",
				L"z->host.bounds.MDown(M:70,70,0)",

				L"z.MUp(:10,10,0)",
				L"z->host.bounds.MUp(:70,70,0)",

				L"z.MDown(M:10,10,0)",
				L"z->host.bounds.MDown(M:70,70,0)",
				L"z.MDbClick(M:10,10,0)",
				L"z->host.bounds.MDbClick(M:70,70,0)",

				L"z.MUp(:10,10,0)",
				L"z->host.bounds.MUp(:70,70,0)"
				);

			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"z.Leave()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"y->x.Move(:20,20,0)",
				L"y->host.bounds.Move(:30,30,0)"
				);

			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, true, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, false, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, true, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, true, 30, 30, 0));
			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"y.RDown(R:10,10,0)",
				L"y->x.RDown(R:20,20,0)",
				L"y->host.bounds.RDown(R:30,30,0)",

				L"y.RUp(:10,10,0)",
				L"y->x.RUp(:20,20,0)",
				L"y->host.bounds.RUp(:30,30,0)",

				L"y.RDown(R:10,10,0)",
				L"y->x.RDown(R:20,20,0)",
				L"y->host.bounds.RDown(R:30,30,0)",
				L"y.RDbClick(R:10,10,0)",
				L"y->x.RDbClick(R:20,20,0)",
				L"y->host.bounds.RDbClick(R:30,30,0)",

				L"y.RUp(:10,10,0)",
				L"y->x.RUp(:20,20,0)",
				L"y->host.bounds.RUp(:30,30,0)"
				);

			protocol.Mouse4Click(NativePoint{ 30,30 }, false, false, false, true);
			protocol.Mouse4DBClick(NativePoint{ 30,30 }, false, false, false, true);
			protocol.Mouse5Click(NativePoint{ 30,30 }, false, false, false, true);
			protocol.Mouse5DBClick(NativePoint{ 30,30 }, false, false, false, true);
			AssertEventLogs(
				eventLogs,
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"y.4Down(W:10,10,0)",
				L"y->x.4Down(W:20,20,0)",
				L"y->host.bounds.4Down(W:30,30,0)",
				L"y.4Up(W:10,10,0)",
				L"y->x.4Up(W:20,20,0)",
				L"y->host.bounds.4Up(W:30,30,0)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)",
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"y.4Down(W:10,10,0)",
				L"y->x.4Down(W:20,20,0)",
				L"y->host.bounds.4Down(W:30,30,0)",
				L"y.4Up(W:10,10,0)",
				L"y->x.4Up(W:20,20,0)",
				L"y->host.bounds.4Up(W:30,30,0)",
				L"y.4Down(W:10,10,0)",
				L"y->x.4Down(W:20,20,0)",
				L"y->host.bounds.4Down(W:30,30,0)",
				L"y.4DbClick(W:10,10,0)",
				L"y->x.4DbClick(W:20,20,0)",
				L"y->host.bounds.4DbClick(W:30,30,0)",
				L"y.4Up(W:10,10,0)",
				L"y->x.4Up(W:20,20,0)",
				L"y->host.bounds.4Up(W:30,30,0)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)",
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"y.5Down(W:10,10,0)",
				L"y->x.5Down(W:20,20,0)",
				L"y->host.bounds.5Down(W:30,30,0)",
				L"y.5Up(W:10,10,0)",
				L"y->x.5Up(W:20,20,0)",
				L"y->host.bounds.5Up(W:30,30,0)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)",
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"y.5Down(W:10,10,0)",
				L"y->x.5Down(W:20,20,0)",
				L"y->host.bounds.5Down(W:30,30,0)",
				L"y.5Up(W:10,10,0)",
				L"y->x.5Up(W:20,20,0)",
				L"y->host.bounds.5Up(W:30,30,0)",
				L"y.5Down(W:10,10,0)",
				L"y->x.5Down(W:20,20,0)",
				L"y->host.bounds.5Down(W:30,30,0)",
				L"y.5DbClick(W:10,10,0)",
				L"y->x.5DbClick(W:20,20,0)",
				L"y->host.bounds.5DbClick(W:30,30,0)",
				L"y.5Up(W:10,10,0)",
				L"y->x.5Up(W:20,20,0)",
				L"y->host.bounds.5Up(W:30,30,0)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)"
				);

			protocol.MouseMove(NativePoint{ 70,70 }, false, false, false, true);
			AssertEventLogs(
				eventLogs,
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"y.Leave()",
				L"x.Leave()",
				L"z.Enter()",
				L"z.Move(W:10,10,0)",
				L"z->host.bounds.Move(W:70,70,0)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)"
				);

			protocol.HWheelRight(1, NativePoint{ 70,70 }, false, false, false, true);
			protocol.WheelDown(1, NativePoint{ 70,70 }, false, false, false, true);
			AssertEventLogs(
				eventLogs,
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"z.HWheel(W:10,10,120)",
				L"z->host.bounds.HWheel(W:70,70,120)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)",
				L"host.bounds.KeyPreview(W:?)",
				L"host.bounds.KeyDown(W:?)",
				L"z.VWheel(W:10,10,-120)",
				L"z->host.bounds.VWheel(W:70,70,-120)",
				L"host.bounds.KeyPreview(:?)",
				L"host.bounds.KeyUp(:?)"
				);

			protocol.GetEvents()->OnIOMouseLeaved();
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
		StartRemoteControllerTest(protocol);
	});

	auto mouseEventFilteringTest = [&]<typename TCallback>(TCallback&& callback)
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiBoundsComposition* x = nullptr, * y = nullptr, * z = nullptr;
		bool enteringZ = false;

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();

			x = new GuiBoundsComposition();
			x->SetExpectedBounds(Rect({ 10,10 }, { 100,100 }));

			y = new GuiBoundsComposition();
			y->SetExpectedBounds(Rect({ 10,10 }, { 80,80}));

			z = new GuiBoundsComposition();
			z->SetExpectedBounds(Rect({ 60,60 }, { 100,100 }));
			enteringZ = callback(z);

			x->AddChild(y);
			controlHost->GetContainerComposition()->AddChild(x);
			controlHost->GetContainerComposition()->AddChild(z);

			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			TEST_ASSERT(x->GetCachedBounds() == Rect({ 10,10 }, { 100,100 }));
			TEST_ASSERT(y->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			TEST_ASSERT(z->GetCachedBounds() == Rect({ 60,60 }, { 100,100 }));

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(x, L"x", eventLogs);
			AttachAndLogEvents(y, L"y", eventLogs);
			AttachAndLogEvents(z, L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOMouseEntered();
			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"y->x.Move(:20,20,0)",
				L"y->host.bounds.Move(:30,30,0)"
				);

			if (enteringZ)
			{
				TEST_ASSERT(protocol.capturing == false);
				protocol.GetEvents()->OnControllerDisconnect();
				protocol.capturing = true;
				protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
				TEST_ASSERT(protocol.capturing == false);
			}

			if (enteringZ)
			{
				auto nonClientMouse5Down = MakeMouseInfoWithButton(NativeMouseButton::Mouse5, false, false, false, 30, 30, 0);
				nonClientMouse5Down.info.nonClient = true;
				protocol.GetEvents()->OnIOButtonDown(nonClientMouse5Down);
				TEST_ASSERT(protocol.capturing == false);
				auto nonClientMouse5Up = MakeMouseInfoWithButton(NativeMouseButton::Mouse5, false, false, false, 30, 30, 0);
				nonClientMouse5Up.info.nonClient = true;
				protocol.GetEvents()->OnIOButtonUp(nonClientMouse5Up);
				TEST_ASSERT(protocol.capturing == false);

				protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Mouse5, false, false, false, 30, 30, 0));
				TEST_ASSERT(protocol.capturing == true);
				protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Mouse5, false, false, false, 30, 30, 0));
				TEST_ASSERT(protocol.capturing == false);
				AssertEventLogs(
					eventLogs,
					L"y.5Down(:10,10,0)",
					L"y->x.5Down(:20,20,0)",
					L"y->host.bounds.5Down(:30,30,0)",
					L"y.5Up(:10,10,0)",
					L"y->x.5Up(:20,20,0)",
					L"y->host.bounds.5Up(:30,30,0)",
					L"y.5Down(:10,10,0)",
					L"y->x.5Down(:20,20,0)",
					L"y->host.bounds.5Down(:30,30,0)",
					L"y.5Up(:10,10,0)",
					L"y->x.5Up(:20,20,0)",
					L"y->host.bounds.5Up(:30,30,0)"
					);
			}

			TEST_ASSERT(protocol.capturing == false);
			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Left, enteringZ, false, false, 30, 30, 0));
			if (enteringZ)
			{
				AssertEventLogs(
					eventLogs,
					L"y.LDown(L:10,10,0)",
					L"y->x.LDown(L:20,20,0)",
					L"y->host.bounds.LDown(L:30,30,0)"
					);
			}
			else
			{
				AssertEventLogs(
					eventLogs,
					L"y.LDown(:10,10,0)",
					L"y->x.LDown(:20,20,0)",
					L"y->host.bounds.LDown(:30,30,0)"
					);
			}

			if (enteringZ)
			{
				protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Mouse4, true, false, false, 30, 30, 0));
				TEST_ASSERT(protocol.capturing == true);
				protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 30, 30, 0));
				TEST_ASSERT(protocol.capturing == true);
				protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 30, 30, 0));
				protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Mouse4, true, false, false, 30, 30, 0));
				TEST_ASSERT(protocol.capturing == true);
				AssertEventLogs(
					eventLogs,
					L"y.4Down(L:10,10,0)",
					L"y->x.4Down(L:20,20,0)",
					L"y->host.bounds.4Down(L:30,30,0)",
					L"y.LUp(:10,10,0)",
					L"y->x.LUp(:20,20,0)",
					L"y->host.bounds.LUp(:30,30,0)",
					L"y.LDown(L:10,10,0)",
					L"y->x.LDown(L:20,20,0)",
					L"y->host.bounds.LDown(L:30,30,0)",
					L"y.4Up(L:10,10,0)",
					L"y->x.4Up(L:20,20,0)",
					L"y->host.bounds.4Up(L:30,30,0)"
					);
			}

			if (enteringZ)
			{
				TEST_ASSERT(protocol.capturing == true);
				protocol.GetEvents()->OnControllerDisconnect();
				protocol.capturing = false;
				protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
				TEST_ASSERT(protocol.capturing == true);
			}

			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(enteringZ, false, false, 70, 70, 0));
			if (enteringZ)
			{
				AssertEventLogs(
					eventLogs,
					L"y.Leave()",
					L"x.Leave()",
					L"z.Enter()",
					L"y.Move(L:50,50,0)",
					L"y->x.Move(L:60,60,0)",
					L"y->host.bounds.Move(L:70,70,0)"
					);
			}
			else
			{
				AssertEventLogs(
					eventLogs,
					L"y.Move(:50,50,0)",
					L"y->x.Move(:60,60,0)",
					L"y->host.bounds.Move(:70,70,0)"
					);
			}

			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Middle, false, false, false, 70, 70, 0));
			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Right, false, false, false, 70, 70, 0));
			protocol.GetEvents()->OnIOHWheel(MakeMouseInfo(false, false, false, 70, 70, 100));
			protocol.GetEvents()->OnIOVWheel(MakeMouseInfo(false, false, false, 70, 70, -100));
			AssertEventLogs(
				eventLogs,
				L"y.LDbClick(:50,50,0)",
				L"y->x.LDbClick(:60,60,0)",
				L"y->host.bounds.LDbClick(:70,70,0)",

				L"y.MDbClick(:50,50,0)",
				L"y->x.MDbClick(:60,60,0)",
				L"y->host.bounds.MDbClick(:70,70,0)",

				L"y.RDbClick(:50,50,0)",
				L"y->x.RDbClick(:60,60,0)",
				L"y->host.bounds.RDbClick(:70,70,0)",

				L"y.HWheel(:50,50,100)",
				L"y->x.HWheel(:60,60,100)",
				L"y->host.bounds.HWheel(:70,70,100)",

				L"y.VWheel(:50,50,-100)",
				L"y->x.VWheel(:60,60,-100)",
				L"y->host.bounds.VWheel(:70,70,-100)"
				);

			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"y.LUp(:50,50,0)",
				L"y->x.LUp(:60,60,0)",
				L"y->host.bounds.LUp(:70,70,0)"
				);
			TEST_ASSERT(protocol.capturing == false);

			protocol.GetEvents()->OnIOMouseLeaved();
			if (enteringZ)
			{
				AssertEventLogs(
					eventLogs,
					L"z.Leave()",
					L"host.bounds.Leave()"
					);
			}
			else
			{
				AssertEventLogs(
					eventLogs,
					L"y.Leave()",
					L"x.Leave()",
					L"host.bounds.Leave()"
					);
			}
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	};

	TEST_CATEGORY(L"Mouse capturing on compositions")
	{
		mouseEventFilteringTest([](GuiBoundsComposition* z)
		{
			return true;
		});
	});

	TEST_CATEGORY(L"Mouse over invisible compositions")
	{
		mouseEventFilteringTest([](GuiBoundsComposition* z)
		{
			z->SetVisible(false);
			return false;
		});
	});

	TEST_CATEGORY(L"Mouse over transparent compositions")
	{
		mouseEventFilteringTest([](GuiBoundsComposition* z)
		{
			z->SetTransparentToMouse(true);
			return false;
		});
	});

	TEST_CATEGORY(L"Mouse capturing on compositions but being deleted")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiBoundsComposition* x = nullptr, * y = nullptr, * z = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();

			x = new GuiBoundsComposition();
			x->SetExpectedBounds(Rect({ 10,10 }, { 100,100 }));

			y = new GuiBoundsComposition();
			y->SetExpectedBounds(Rect({ 10,10 }, { 80,80}));

			z = new GuiBoundsComposition();
			z->SetExpectedBounds(Rect({ 60,60 }, { 100,100 }));

			x->AddChild(y);
			controlHost->GetContainerComposition()->AddChild(x);
			controlHost->GetContainerComposition()->AddChild(z);

			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			TEST_ASSERT(x->GetCachedBounds() == Rect({ 10,10 }, { 100,100 }));
			TEST_ASSERT(y->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			TEST_ASSERT(z->GetCachedBounds() == Rect({ 60,60 }, { 100,100 }));

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(x, L"x", eventLogs);
			AttachAndLogEvents(y, L"y", eventLogs);
			AttachAndLogEvents(z, L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOMouseEntered();
			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(false, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"host.bounds.Enter()",
				L"x.Enter()",
				L"y.Enter()",
				L"y.Move(:10,10,0)",
				L"y->x.Move(:20,20,0)",
				L"y->host.bounds.Move(:30,30,0)"
				);

			protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 30, 30, 0));
			AssertEventLogs(
				eventLogs,
				L"y.LDown(L:10,10,0)",
				L"y->x.LDown(L:20,20,0)",
				L"y->host.bounds.LDown(L:30,30,0)"
				);

			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(true, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"y.Leave()",
				L"x.Leave()",
				L"z.Enter()",
				L"y.Move(L:50,50,0)",
				L"y->x.Move(L:60,60,0)",
				L"y->host.bounds.Move(L:70,70,0)"
				);

			TEST_ASSERT(GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing() == true);
			SafeDeleteComposition(x);
			TEST_ASSERT(GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing() == false);
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 70, 70, 0));
			AssertEventLogs(
				eventLogs,
				L"z.LDbClick(L:10,10,0)",
				L"z->host.bounds.LDbClick(L:70,70,0)"
				);

			protocol.GetEvents()->OnIOMouseMoving(MakeMouseInfo(true, false, false, 10, 10, 0));
			AssertEventLogs(
				eventLogs,
				L"z.Leave()",
				L"host.bounds.Move(L:10,10,0)"
				);

			protocol.GetEvents()->OnIOButtonDoubleClick(MakeMouseInfoWithButton(NativeMouseButton::Left, true, false, false, 10, 10, 0));
			AssertEventLogs(
				eventLogs,
				L"host.bounds.LDbClick(L:10,10,0)"
				);

			protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(NativeMouseButton::Left, false, false, false, 10, 10, 0));
			AssertEventLogs(
				eventLogs,
				L"host.bounds.LUp(:10,10,0)"
				);

			protocol.GetEvents()->OnIOMouseLeaved();
			AssertEventLogs(
				eventLogs,
				L"host.bounds.Leave()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}
