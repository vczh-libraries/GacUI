#include "TestRemote_GraphicsHost_Shared.h"
#include "TestPointInput.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"

class MouseInputRenderer : public remote_renderer::GuiRemoteRendererSingle
{
public:
	MouseInputRenderer(IGuiRemoteProtocolEvents* _events)
		: GuiRemoteRendererSingle(false)
	{
		events = _events;
	}

	using GuiRemoteRendererSingle::PointSequenceStarted;
	using GuiRemoteRendererSingle::PointDown;
	using GuiRemoteRendererSingle::PointUpdate;
	using GuiRemoteRendererSingle::PointUp;
	using GuiRemoteRendererSingle::PointEntered;
	using GuiRemoteRendererSingle::PointLeaved;
	using GuiRemoteRendererSingle::PointSequenceEnded;
	using GuiRemoteRendererSingle::MouseDown;
	using GuiRemoteRendererSingle::MouseUp;
	using GuiRemoteRendererSingle::MouseDoubleClick;
	using GuiRemoteRendererSingle::MouseMoving;
	using GuiRemoteRendererSingle::HorizontalWheel;
	using GuiRemoteRendererSingle::VerticalWheel;
	using GuiRemoteRendererSingle::SendAccumulatedMessages;
	using GuiRemoteRendererSingle::DisconnectFromCore;
};

class PointScrollView : public GuiScrollView
{
protected:
	Size QueryFullSize()override { return Size(1000, 1000); }
	void UpdateView(Rect)override {}
	void OnPointAction(GuiGraphicsComposition* sender, GuiPointEventArgs& arguments)override
	{
		arguments.handled = consume;
		GuiScrollView::OnPointAction(sender, arguments);
	}
	void OnPointSequenceEnded(GuiGraphicsComposition* sender, GuiPointSequenceEndedEventArgs& arguments)override
	{
		arguments.handled = consume;
		arguments.fallbackClick = explicitFallback;
		GuiScrollView::OnPointSequenceEnded(sender, arguments);
	}
public:
	bool consume = false;
	bool explicitFallback = false;
	PointScrollView(bool hasScrolls = true) : GuiScrollView(theme::ThemeName::ScrollView)
	{
		SetControlTemplate([=](const description::Value&)
		{
			auto ct = new templates::GuiScrollViewTemplate;
			ct->SetContainerComposition(ct);
			if (hasScrolls)
			{
				auto h = new GuiScroll(theme::ThemeName::HScroll);
				auto v = new GuiScroll(theme::ThemeName::VScroll);
				for (auto scroll : { h,v })
				{
					scroll->SetControlTemplate([](const description::Value&) { return new templates::GuiScrollTemplate; });
					ct->AddChild(scroll->GetBoundsComposition());
				}
				h->GetBoundsComposition()->SetExpectedBounds(Rect(0, 190, 190, 200));
				v->GetBoundsComposition()->SetExpectedBounds(Rect(190, 0, 200, 190));
				ct->SetHorizontalScroll(h);
				ct->SetVerticalScroll(v);
			}
			return ct;
		});
	}
};

TEST_FILE
{
	TEST_CATEGORY(L"Point samples use current fractional DPI and hosted window offsets")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiControl* surface = nullptr;
		Point expectedFirst, expectedSecond;
		Rect expectedContact;
		bool checkSamples = false;
		bool liftedFirst = false;
		vint clicks = 0;
		protocol.OnNextFrame([&]()
		{
			surface = new GuiControl(theme::ThemeName::CustomControl);
			surface->SetAcceptPointEvents(true);
			surface->GetBoundsComposition()->SetExpectedBounds(Rect(0, 0, 400, 400));
			controlHost->GetContainerComposition()->AddChild(surface->GetBoundsComposition());
			surface->GetBoundsComposition()->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs& a)
			{
				if (!checkSamples) return;
				TEST_ASSERT(a.GetPoints().Count() == 2);
				TEST_ASSERT((bool)a.GetPoints()[0] == !liftedFirst);
				if (!liftedFirst) TEST_ASSERT(a.GetPoints()[0].Value() == expectedFirst);
				TEST_ASSERT(a.GetPoints()[1].Value() == expectedSecond);
				TEST_ASSERT(a.position == (a.pointId == 0 ? expectedFirst : expectedSecond));
				if (a.pointId == 0) TEST_ASSERT(a.contactBounds.Value() == expectedContact);
			});
			surface->GetBoundsComposition()->GetEventReceiver()->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				TEST_ASSERT(!surface->GetPointEventComposition());
				TEST_ASSERT(Point(a.x, a.y) == expectedFirst);
				clicks++;
			});
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			PointProtocolInput input(protocol.GetEvents());
			auto host = surface->GetBoundsComposition()->GetRelatedGraphicsHost();
			auto window = host->GetNativeWindow();
			auto manager = host->GetPointActionManager();
			auto local = [&](NativePoint screen)
			{
				auto origin = window->GetClientBoundsInScreen().LeftTop();
				auto result = window->Convert(NativePoint(screen.x - origin.x, screen.y - origin.y));
				auto bounds = surface->GetBoundsComposition()->GetGlobalBounds();
				return Point(result.x - bounds.x1, result.y - bounds.y1);
			};
			input.Start(NativePoint(101, 103));
			input.Down(1, NativePoint(203, 157));
			checkSamples = true;
			for (auto scale : { 1.25,1.5,1.75,1.0 })
			{
				auto screen = GraphicsHostProtocol::MakeUnitTestScreenConfig().screenConfig;
				screen.scalingX = scale;
				screen.scalingY = scale + 0.25;
				protocol.GetEvents()->OnControllerScreenUpdated(screen);
				controlHost->ForceCalculateSizeImmediately();
				expectedFirst = local(NativePoint(101, 103));
				expectedSecond = local(NativePoint(203, 157));
				{ auto lt = local(NativePoint(98, 99)); auto rb = local(NativePoint(104, 107)); expectedContact = Rect(lt.x, lt.y, rb.x, rb.y); }
				input.action.info.contactBounds = NativeRect(98, 99, 104, 107);
				input.Move(NativePoint(101, 103));
				TEST_ASSERT(manager->GetAxis() == 0 && manager->GetDisplacement() == Point());
			}
			auto bounds = window->GetBounds();
			window->SetBounds(NativeRect(bounds.x1 + 20, bounds.y1 + 30, bounds.x2 + 20, bounds.y2 + 30));
			expectedFirst = local(NativePoint(101, 103));
			expectedSecond = local(NativePoint(203, 157));
			{ auto lt = local(NativePoint(98, 99)); auto rb = local(NativePoint(104, 107)); expectedContact = Rect(lt.x, lt.y, rb.x, rb.y); }
			input.Move(NativePoint(101, 103));
			TEST_ASSERT(manager->GetDisplacement() == Point());
			liftedFirst = true;
			input.Up(0, NativePoint(101, 103));
			input.Move(NativePoint(203, 157), 1);
			checkSamples = false;
			input.Up(1, NativePoint(203, 157)); input.End();
			TEST_ASSERT(clicks == 0);
			input.Start(NativePoint(101, 103));
			auto screen = GraphicsHostProtocol::MakeUnitTestScreenConfig().screenConfig;
			screen.scalingX = 1.5; screen.scalingY = 1.25;
			protocol.GetEvents()->OnControllerScreenUpdated(screen);
			controlHost->ForceCalculateSizeImmediately();
			expectedFirst = local(NativePoint(101, 103));
			input.Up(0, NativePoint(101, 103)); input.End();
			TEST_ASSERT(clicks == 1);
			input.Start(NativePoint(101, 103));
			input.Move(NativePoint(113, 103));
			TEST_ASSERT(manager->GetDisplacement() == Point(8, 0) && manager->GetAxis() == 0);
			input.Move(NativePoint(116, 103));
			TEST_ASSERT(manager->GetDisplacement() == Point(10, 0) && manager->GetAxis() == 1);
			input.Up(0, NativePoint(116, 103)); input.End();
			TEST_ASSERT(clicks == 1);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Point selection belongs to one control and clears on lifecycle changes")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		PointScrollView* outer = nullptr;
		PointScrollView* inner = nullptr;
		GuiButton* child = nullptr;
		GuiControl* unrelated = nullptr;
		vint childActions = 0, innerActions = 0, outerActions = 0;
		vint childEnds = 0, innerEnds = 0, outerEnds = 0, outerQueries = 0;
		bool handleQuery = false, acceptHandledQuery = false;
		protocol.OnNextFrame([&]()
		{
			outer = new PointScrollView(false);
			inner = new PointScrollView(false);
			child = new GuiButton(theme::ThemeName::Button);
			unrelated = new GuiControl(theme::ThemeName::CustomControl);
			outer->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 400, 400));
			inner->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 210, 210));
			child->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 100, 100));
			unrelated->GetBoundsComposition()->SetExpectedBounds(Rect(450, 10, 550, 110));
			controlHost->GetContainerComposition()->AddChild(outer->GetBoundsComposition());
			controlHost->GetContainerComposition()->AddChild(unrelated->GetBoundsComposition());
			outer->GetContainerComposition()->AddChild(inner->GetBoundsComposition());
			inner->GetContainerComposition()->AddChild(child->GetBoundsComposition());
			child->GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				TEST_ASSERT(!a.handled);
				a.handled = handleQuery;
			});
			child->GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				TEST_ASSERT(a.handled == handleQuery);
				// A later handler on the same receiver cannot erase the base adapter's selection.
				a.acceptedComposition = nullptr;
			});
			outer->GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				TEST_ASSERT(a.handled == handleQuery);
				outerQueries++;
				if (handleQuery && !a.acceptedComposition)
				{
					if (acceptHandledQuery) a.acceptedComposition = outer->GetBoundsComposition();
				}
				else if (outer->GetAcceptPointEvents())
				{
					TEST_ASSERT(a.acceptedComposition == (child->GetAcceptPointEvents() ? child->GetBoundsComposition() : inner->GetBoundsComposition()));
					a.acceptedComposition = outer->GetBoundsComposition();
				}
			});
			child->GetBoundsComposition()->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs&) { childActions++; });
			inner->GetBoundsComposition()->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs&) { innerActions++; });
			outer->GetBoundsComposition()->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs&) { outerActions++; });
			child->GetBoundsComposition()->GetEventReceiver()->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs&) { childEnds++; });
			inner->GetBoundsComposition()->GetEventReceiver()->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs&) { innerEnds++; });
			outer->GetBoundsComposition()->GetEventReceiver()->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs&) { outerEnds++; });
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			PointProtocolInput input(protocol.GetEvents());
			input.Start(NativePoint(50, 50));
			TEST_ASSERT(inner->GetPointEventComposition() == inner->GetBoundsComposition());
			TEST_ASSERT(outer->GetPointEventComposition() == inner->GetBoundsComposition());
			TEST_ASSERT(!child->GetPointEventComposition() && !unrelated->GetPointEventComposition());
			input.Move(NativePoint(50, 10));
			input.Up(0, NativePoint(50, 10)); input.End();
			TEST_ASSERT(childActions == 0 && innerActions == 3 && outerActions == 0 && innerEnds == 1 && childEnds == 0 && outerEnds == 0);
			handleQuery = true;
			child->SetAcceptPointEvents(true);
			input.Start(NativePoint(50, 50));
			TEST_ASSERT(child->GetPointEventComposition() == child->GetBoundsComposition());
			TEST_ASSERT(inner->GetPointEventComposition() == child->GetBoundsComposition() && outer->GetPointEventComposition() == child->GetBoundsComposition());
			child->SetAcceptPointEvents(false);
			input.Move(NativePoint(500, 450));
			input.Up(0, NativePoint(500, 450)); input.End();
			TEST_ASSERT(childActions == 3 && childEnds == 1 && innerActions == 3 && innerEnds == 1 && outerEnds == 0 && outerQueries == 2);
			inner->SetAcceptPointEvents(false); outer->SetAcceptPointEvents(false);
			input.Start(NativePoint(50, 50));
			TEST_ASSERT(!controlHost->GetPointEventComposition());
			input.Up(0, NativePoint(50, 50)); input.End();
			TEST_ASSERT(childEnds == 1 && innerEnds == 1 && outerEnds == 0 && outerQueries == 3);
			child->SetAcceptPointEvents(true);
			input.Start(NativePoint(50, 50));
			inner->GetContainerComposition()->RemoveChild(child->GetBoundsComposition());
			TEST_ASSERT(!controlHost->GetPointEventComposition() && !child->GetPointEventComposition());
			input.Up(0, NativePoint(50, 50)); input.End();
			TEST_ASSERT(childEnds == 1);
			inner->GetContainerComposition()->AddChild(child->GetBoundsComposition());
			controlHost->ForceCalculateSizeImmediately();
			input.Start(NativePoint(50, 50));
			auto host = child->GetBoundsComposition()->GetRelatedGraphicsHost();
			auto window = host->GetNativeWindow();
			host->SetNativeWindow(nullptr);
			TEST_ASSERT(!child->GetPointEventComposition());
			host->SetNativeWindow(window);
			input.Up(0, NativePoint(50, 50)); input.End();
			TEST_ASSERT(childEnds == 1);
			input.Start(NativePoint(50, 50)); input.Up(0, NativePoint(50, 50)); input.End();
			TEST_ASSERT(childEnds == 2 && !controlHost->GetPointEventComposition());
			child->SetAcceptPointEvents(false);
			inner->SetAcceptPointEvents(true); outer->SetAcceptPointEvents(true);
			auto queries = outerQueries;
			input.Start(NativePoint(50, 50));
			TEST_ASSERT(!controlHost->GetPointEventComposition() && outerQueries == queries + 1);
			input.Up(0, NativePoint(50, 50)); input.End();
			TEST_ASSERT(childEnds == 2 && innerEnds == 1 && outerEnds == 0);
			acceptHandledQuery = true;
			input.Start(NativePoint(50, 50));
			TEST_ASSERT(outer->GetPointEventComposition() == outer->GetBoundsComposition() && outerQueries == queries + 2);
			input.Move(NativePoint(50, 10));
			input.Up(0, NativePoint(50, 10)); input.End();
			TEST_ASSERT(outerActions == 3 && outerEnds == 1 && !controlHost->GetPointEventComposition());
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Point scrolling recognition and deferred clicks")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		PointScrollView* scroll = nullptr;
		GuiButton* button = nullptr;
		vint clicks = 0;
		List<NativeMouseButton> downs, ups, physicalUps;
		protocol.OnNextFrame([&]()
		{
			scroll = new PointScrollView;
			scroll->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 210, 210));
			controlHost->GetContainerComposition()->AddChild(scroll->GetBoundsComposition());
			button = new GuiButton(theme::ThemeName::Button);
			button->GetBoundsComposition()->SetExpectedBounds(Rect(20, 20, 100, 100));
			scroll->GetContainerComposition()->AddChild(button->GetBoundsComposition());
			auto physical = new GuiBoundsComposition;
			physical->SetExpectedBounds(Rect(450, 10, 550, 110));
			controlHost->GetContainerComposition()->AddChild(physical);
			physical->GetEventReceiver()->mouseUp.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a) { physicalUps.Add(a.button); });
			button->Clicked.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { clicks++; });
			button->GetBoundsComposition()->GetEventReceiver()->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				TEST_ASSERT(!scroll->GetPointEventComposition());
				TEST_ASSERT(a.ctrl && a.shift && a.alt && a.osSuper);
				TEST_ASSERT(a.x == 20 && a.y == 20);
				TEST_ASSERT(button->TypedControlTemplateObject(true)->GetState() == (a.button == NativeMouseButton::Left ? ButtonState::Pressed : ButtonState::Active));
				downs.Add(a.button);
			});
			button->GetBoundsComposition()->GetEventReceiver()->mouseUp.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				TEST_ASSERT(downs.Count() == ups.Count() + 1);
				TEST_ASSERT(button->TypedControlTemplateObject(true)->GetState() == ButtonState::Active);
				ups.Add(a.button);
			});
			scroll->GetBoundsComposition()->GetEventReceiver()->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& a)
			{
				TEST_ASSERT(scroll->GetPointEventComposition() == scroll->GetBoundsComposition());
				TEST_ASSERT(controlHost->GetPointEventComposition() == scroll->GetBoundsComposition());
				TEST_ASSERT(!button->GetPointEventComposition());
				TEST_ASSERT(a.compositionSource == scroll->GetBoundsComposition() && a.eventSource == a.compositionSource);
			});
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(scroll->GetAcceptPointEvents() && !button->GetAcceptPointEvents());
			PointProtocolInput input(protocol.GetEvents());
			auto manager = scroll->GetBoundsComposition()->GetRelatedGraphicsHost()->GetPointActionManager();
			auto resetView = [&]() { scroll->SetViewPosition(Point(300, 300)); };
			auto finish = [&](NativePoint p)
			{
				input.Up(0, p);
				input.End();
				TEST_ASSERT(!scroll->GetPointEventComposition() && !controlHost->GetPointEventComposition());
				TEST_ASSERT(!manager->GetFallbackClick() && manager->GetAxis() == 0);
			};
			protocol.MouseMove(NativePoint(500, 400));
			input.Start(NativePoint(50, 50));
			input.Up(0, NativePoint(50, 50));
			input.End(499999);
			TEST_ASSERT(clicks == 1 && downs.Count() == 1 && downs[0] == NativeMouseButton::Left && ups[0] == downs[0]);
			input.Start(NativePoint(50, 50));
			input.Up(0, NativePoint(50, 50));
			input.End(500000);
			TEST_ASSERT(clicks == 1 && downs.Count() == 2 && downs[1] == NativeMouseButton::Right && ups[1] == downs[1]);
			for (auto delta : { NativePoint(7,0), NativePoint(8,0), NativePoint(9,0), NativePoint(0,9), NativePoint(6,6) })
			{
				resetView();
				input.Start(NativePoint(120, 120));
				input.Move(NativePoint(delta.x + 120, delta.y + 120));
				auto expectedAxis = delta.x.value == 9 || delta.x.value == 6 ? 1 : delta.y.value == 9 ? 2 : 0;
				TEST_ASSERT(manager->GetAxis() == expectedAxis);
				TEST_ASSERT(scroll->GetViewPosition() == Point(300 - (expectedAxis == 1 ? delta.x.value : 0), 300 - (expectedAxis == 2 ? delta.y.value : 0)));
				finish(NativePoint(delta.x + 120, delta.y + 120));
			}
			resetView();
			input.Start(NativePoint(120, 120));
			input.Move(NativePoint(90, 119));
			TEST_ASSERT(scroll->GetViewPosition() == Point(330, 300));
			input.Move(NativePoint(140, 30));
			TEST_ASSERT(scroll->GetViewPosition() == Point(280, 300));
			input.Move(NativePoint(120, 120));
			TEST_ASSERT(scroll->GetViewPosition() == Point(300, 300) && manager->GetAxis() == 1);
			finish(NativePoint(120, 120));
			resetView();
			input.Start(NativePoint(120, 120));
			finish(NativePoint(120, 90));
			TEST_ASSERT(scroll->GetViewPosition() == Point(300, 330));
			for (vint unavailable = 0; unavailable < 3; unavailable++)
			{
				resetView();
				auto bar = scroll->GetHorizontalScroll();
				if (unavailable == 0) bar->SetVisible(false);
				if (unavailable == 1) bar->SetEnabled(false);
				if (unavailable == 2) scroll->SetEnabled(false);
				auto before = bar->GetPosition();
				input.Start(NativePoint(120, 120));
				input.Move(NativePoint(70, 80));
				TEST_ASSERT(bar->GetPosition() == before && scroll->GetVerticalScroll()->GetPosition() == 300);
				finish(NativePoint(70, 80));
				bar->SetVisible(true); bar->SetEnabled(true); scroll->SetEnabled(true);
			}
			resetView();
			input.Start(NativePoint(120, 120));
			input.Move(NativePoint(2000, 120));
			TEST_ASSERT(scroll->GetViewPosition().x == 0);
			input.Move(NativePoint(-2000, 120));
			TEST_ASSERT(scroll->GetViewPosition().x == scroll->GetHorizontalScroll()->GetTotalSize() - scroll->GetHorizontalScroll()->GetPageSize());
			finish(NativePoint(-2000, 120));
			for (bool firstUp : { false,true })
			{
				resetView();
				input.Start(NativePoint(120, 120));
				input.Down(1, NativePoint(150, 150));
				if (firstUp)
				{
					input.Up(0, NativePoint(120, 120));
					input.Move(NativePoint(150, 40), 1);
					input.Up(1, NativePoint(150, 40));
				}
				else
				{
					input.Up(1, NativePoint(150, 150));
					input.Up(0, NativePoint(120, 120));
				}
				input.End();
				TEST_ASSERT(scroll->GetViewPosition() == Point(300, 300));
			}
			scroll->consume = true;
			resetView();
			input.Start(NativePoint(120, 120));
			input.Move(NativePoint(120, 70));
			TEST_ASSERT(scroll->GetViewPosition() == Point(300, 300) && manager->GetAxis() == 2);
			finish(NativePoint(120, 120));
			input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			TEST_ASSERT(clicks == 1 && downs.Count() == 2);
			scroll->explicitFallback = true;
			input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			TEST_ASSERT(clicks == 2 && downs.Count() == 3);
			scroll->consume = scroll->explicitFallback = false;
			for (auto reason : { PointSequenceEndReason::Canceled_Unknown, PointSequenceEndReason::Canceled_PlatformCanceled, PointSequenceEndReason::Canceled_CaptureLost, PointSequenceEndReason::Canceled_WindowDestroyed })
			{
				input.Start(NativePoint(50, 50));
				input.End(1000000, reason);
				TEST_ASSERT(!scroll->GetPointEventComposition() && !manager->GetFallbackClick());
				input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			}
			TEST_ASSERT(clicks == 6 && downs.Count() == 7 && ups.Count() == downs.Count());
			TEST_ASSERT(!GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing());
			for (auto physicalButton : { NativeMouseButton::Left,NativeMouseButton::Mouse5 })
			{
				protocol.MouseMove(NativePoint(500, 50));
				protocol.GetEvents()->OnIOButtonDown(MakeMouseInfoWithButton(physicalButton, physicalButton == NativeMouseButton::Left, false, false, 500, 50, 0));
				TEST_ASSERT(protocol.capturing);
				input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
				TEST_ASSERT(protocol.capturing);
				protocol.GetEvents()->OnIOButtonUp(MakeMouseInfoWithButton(physicalButton, false, false, false, 600, 400, 0));
				TEST_ASSERT(!protocol.capturing && physicalUps[physicalUps.Count() - 1] == physicalButton);
			}
			TEST_ASSERT(clicks == 8 && downs.Count() == 9 && ups.Count() == downs.Count() && physicalUps.Count() == 2);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Scroll views accept point input through the protocol")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		vint actions = 0;
		protocol.OnNextFrame([&]()
		{
			auto scroll = new GuiScrollContainer(theme::ThemeName::ScrollView);
			scroll->SetControlTemplate([](const description::Value&)
			{
				auto ct = new templates::GuiScrollViewTemplate;
				ct->SetContainerComposition(ct);
				return ct;
			});
			scroll->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 210, 210));
			controlHost->GetContainerComposition()->AddChild(scroll->GetBoundsComposition());
			controlHost->ForceCalculateSizeImmediately();
			scroll->GetBoundsComposition()->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs&) { actions++; });
			auto events = protocol.GetEvents();
			NativeWindowPointInfo sample;
			sample.sequenceId = 1;
			sample.activePointCount = 1;
			sample.position = NativePoint(30, 30);
			IOPointAction a;
			a.info = EncodePointInfo(sample);
			a.eventIndex = 0;
			events->OnIOPointStarted(a);
			a.eventIndex = 1;
			events->OnIOPointDown(a);
			TEST_ASSERT(actions == 1);
			a.eventIndex = 2;
			a.info.activePointCount = 0;
			events->OnIOPointUp(a);
			IOPointEnd end;
			end.sequenceId = L"1";
			end.eventIndex = 3;
			end.timestamp = L"0";
			end.reason = PointSequenceEndReason::Succeeded;
			events->OnIOPointEnded(end);
			TEST_ASSERT(actions == 2);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CASE(L"Point unsigned decimal decoding is exact and canonical")
	{
		for (auto text : { L"0", L"4294967295", L"9007199254740993", L"18446744073709551615" })
		{
			TEST_ASSERT(u64tow(DecodePointUnsigned(text)) == text);
		}
		for (auto text : { L"", L"00", L"01", L"-1", L"+1", L" 1", L"1 ", L"1.0", L"1e3", L"18446744073709551616" })
		{
			TEST_ERROR(DecodePointUnsigned(text));
		}
		NativeWindowPointInfo info;
		info.sequenceId = (vuint64_t)-1;
		info.timestamp = 9007199254740993ULL;
		TEST_ASSERT(DecodePointInfo(EncodePointInfo(info)) == info);
		info.platformFrameId = 0;
		TEST_ASSERT(DecodePointInfo(EncodePointInfo(info)) == info);
		info.platformFrameId = (vuint64_t)-1;
		TEST_ASSERT(DecodePointInfo(EncodePointInfo(info)) == info);
	});

	for (vint dpi : { 1,2 })
	for (vint mode = 0; mode < 4; mode++)
	TEST_CATEGORY(L"Remote point capture, sparse contacts and fallback: " + itow(mode) + L" at DPI " + itow(dpi))
	{
		auto config = GraphicsHostProtocol::MakeUnitTestScreenConfig();
		config.screenConfig.scalingX = config.screenConfig.scalingY = (double)dpi;
		GraphicsHostProtocol protocol(config);
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiBoundsComposition* parent = nullptr;
		GuiBoundsComposition* child = nullptr;
		vint queries = 0, parentQueries = 0, actions = 0, ends = 0, mouseDowns = 0, mouseUps = 0;
		List<WString> pointLogs;
		vuint64_t localSequence = 0;
		bool accepted = mode < 2;
		const List<Nullable<Point>>* borrowed = nullptr;
		protocol.OnNextFrame([&]()
		{
			parent = new GuiBoundsComposition;
			parent->SetExpectedBounds(Rect(10, 10, 210, 210));
			child = new GuiBoundsComposition;
			child->SetExpectedBounds(Rect(10, 10, 110, 110));
			parent->AddChild(child);
			controlHost->GetContainerComposition()->AddChild(parent);
			controlHost->ForceCalculateSizeImmediately();
			child->GetEventReceiver()->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				queries++;
				TEST_ASSERT(a.sequenceId > localSequence);
				localSequence = a.sequenceId;
				TEST_ASSERT(a.eventType == GuiPointEventsType::Started);
				TEST_ASSERT(a.position == Point(10, 10));
				TEST_ASSERT(a.GetPoints().Count() == 1 && a.GetPoints()[0].Value() == a.position);
				a.acceptedComposition = accepted && mode != 1 ? child : nullptr;
			});
			parent->GetEventReceiver()->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				parentQueries++;
				TEST_ASSERT(a.position == Point(20, 20));
				a.acceptedComposition = accepted && mode == 1 ? parent : nullptr;
			});
			(mode == 1 ? parent : child)->GetEventReceiver()->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs& a)
			{
				actions++;
				pointLogs.Add(itow((vint)a.eventType) + L":" + itow(a.pointId) + L":" + itow(a.activePointCount));
				TEST_ASSERT(accepted && a.sequenceId == localSequence);
				TEST_ASSERT(a.timestamp == 9007199254740993ULL);
				TEST_ASSERT(a.ctrl && a.shift && a.alt && a.osSuper);
				TEST_ASSERT(a.contactBounds.Value() == (mode == 1 ? Rect(18, 17, 22, 23) : Rect(8, 7, 12, 13)));
				TEST_ASSERT(a.pressure.Value() == 0.75 && a.orientation.Value() == 1.25);
				TEST_ASSERT(a.platformFrameId.Value() == (vuint64_t)-1);
				TEST_ASSERT(a.GetPoints().Count() > a.pointId);
				TEST_ASSERT((bool)a.GetPoints()[a.pointId] == (a.eventType != GuiPointEventsType::Up));
				if (a.pointId == 2) TEST_ASSERT(!a.GetPoints()[0]);
			});
			(mode == 1 ? parent : child)->GetEventReceiver()->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& a)
			{
				ends++;
				TEST_ASSERT(a.accepted == accepted && a.sequenceId == localSequence);
				borrowed = &a.GetPoints();
				if (!accepted) TEST_ASSERT(a.GetPoints().Count() == 0);
				else if (a.reason != PointSequenceEndReason::Succeeded)
				{
					if (a.GetPoints().Count() == 1) TEST_ASSERT(a.GetPoints()[0]);
					else TEST_ASSERT(!a.GetPoints()[0] && a.GetPoints()[1]);
				}
			});
			child->GetEventReceiver()->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				mouseDowns++;
				TEST_ASSERT(!accepted && a.ctrl && a.shift && a.alt && a.osSuper);
			});
			child->GetEventReceiver()->mouseUp.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs&) { mouseUps++; });
		});
		protocol.OnNextFrame([&]()
		{
			auto events = protocol.GetEvents();
			NativeWindowPointInfo sample;
			sample.sequenceId = (vuint64_t)-1;
			sample.timestamp = 9007199254740993ULL;
			sample.activePointCount = 1;
			sample.position = NativePoint(30 * dpi, 30 * dpi);
			sample.primary = true;
			sample.ctrl = sample.shift = sample.alt = sample.osSuper = true;
			sample.contactBounds = NativeRect(28 * dpi, 27 * dpi, 32 * dpi, 33 * dpi);
			sample.pressure = 0.75;
			sample.orientation = 1.25;
			sample.platformFrameId = (vuint64_t)-1;
			IOPointAction a;
			a.eventIndex = 0;
			a.info = EncodePointInfo(sample);
			events->OnIOPointDown(a); // Outside a sequence.
			events->OnIOPointStarted(a);
			TEST_ASSERT(queries == 1 && parentQueries == 1);
			events->OnIOPointStarted(a); // Nested Started cannot replace capture.
			a.eventIndex = 1;
			events->OnIOPointDown(a);
			events->OnIOPointDown(a); // Duplicate is ignored.
			a.eventIndex = 3;
			TEST_ERROR(events->OnIOPointUpdate(a)); // Missing index is a protocol failure.
			a.eventIndex = 2;
			a.info.sequenceId = L"123";
			events->OnIOPointUpdate(a); // Foreign sequence does not advance the index.
			a.info.sequenceId = u64tow(sample.sequenceId);
			events->OnIOPointUpdate(a);
			a.eventIndex++;
			a.info.pointId = 1; a.info.activePointCount = 2; a.info.primary = false;
			events->OnIOPointDown(a);
			a.eventIndex++;
			a.info.pointId = 0; a.info.activePointCount = 1;
			events->OnIOPointUp(a);
			a.eventIndex++;
			a.info.pointId = 1;
			events->OnIOPointLeave(a);
			a.eventIndex++;
			events->OnIOPointEnter(a);
			a.eventIndex++;
			a.info.pointId = 2; a.info.activePointCount = 2;
			events->OnIOPointDown(a);
			a.eventIndex++;
			a.info.pointId = 1; a.info.activePointCount = 1;
			events->OnIOPointUp(a);
			a.eventIndex++;
			a.info.pointId = 2; a.info.activePointCount = 0;
			events->OnIOPointUp(a);
			TEST_ASSERT(ends == 0); // Final Up does not synthesize End.
			IOPointEnd end;
			end.sequenceId = a.info.sequenceId; end.eventIndex = ++a.eventIndex;
			end.timestamp = L"18446744073709551615"; end.accepted = !accepted;
			end.reason = PointSequenceEndReason::Succeeded;
			events->OnIOPointEnded(end);
			TEST_ASSERT(ends == (accepted ? 1 : 0) && (!borrowed || borrowed->Count() == 0));
			TEST_ASSERT(actions == (accepted ? 9 : 0));
			if (accepted) AssertEventLogs(pointLogs, L"1:0:1", L"2:0:1", L"1:1:2", L"3:0:1", L"5:1:1", L"4:1:1", L"1:2:2", L"3:1:1", L"3:2:0");
			else AssertEventLogs(pointLogs);
			TEST_ASSERT(mouseDowns == (accepted ? 0 : 1) && mouseUps == mouseDowns);
			a.info = EncodePointInfo(sample); a.eventIndex = 0;
			events->OnIOPointStarted(a); // Completed wire identity stays retired.
			events->OnIOPointEnded(end);
			TEST_ASSERT(queries == 1 && ends == (accepted ? 1 : 0));
			for (auto reason : { PointSequenceEndReason::Canceled_PlatformCanceled, PointSequenceEndReason::Canceled_CaptureLost, PointSequenceEndReason::Canceled_WindowDestroyed, PointSequenceEndReason::Canceled_Unknown })
			{
				sample.sequenceId--;
				a.info = EncodePointInfo(sample); a.eventIndex = 0;
				events->OnIOPointStarted(a);
				a.eventIndex = 1; events->OnIOPointDown(a);
				if (reason == PointSequenceEndReason::Canceled_CaptureLost)
				{
					a.eventIndex = 2; a.info.pointId = 1; a.info.activePointCount = 2; events->OnIOPointDown(a);
					a.eventIndex = 3; a.info.pointId = 0; a.info.activePointCount = 1; events->OnIOPointUp(a);
				}
				end.sequenceId = a.info.sequenceId; end.eventIndex = a.eventIndex + 1; end.reason = reason;
				events->OnIOPointEnded(end);
				TEST_ASSERT(!borrowed || borrowed->Count() == 0);
				a.eventIndex = 2; events->OnIOPointUpdate(a);
			}
			TEST_ASSERT(ends == (accepted ? 5 : 0) && queries == 5);
			pointLogs.Clear();
			// Native renderer always owns acquisition; Core asks again each sequence.
			accepted = !accepted;
			MouseInputRenderer renderer(events);
			sample.sequenceId = 123;
			bool rendererAccept = false;
			renderer.PointSequenceStarted(sample, rendererAccept);
			TEST_ASSERT(rendererAccept);
			renderer.PointDown(sample);
			renderer.PointUpdate(sample);
			renderer.PointUpdate(sample); // Stationary samples cannot be coalesced.
			sample.activePointCount = 0;
			renderer.PointUp(sample);
			PointSequenceEndInfo boundary;
			boundary.sequenceId = sample.sequenceId; boundary.timestamp = sample.timestamp;
			boundary.accepted = true; boundary.reason = PointSequenceEndReason::Succeeded;
			renderer.PointSequenceEnded(boundary);
			TEST_ASSERT(ends == (accepted ? 1 : 5) && queries == 6);
			if (accepted) AssertEventLogs(pointLogs, L"1:0:1", L"2:0:1", L"2:0:1", L"3:0:0");
			else AssertEventLogs(pointLogs);
			renderer.DisconnectFromCore();
			sample.sequenceId++;
			renderer.PointSequenceStarted(sample, rendererAccept);
			renderer.PointDown(sample);
			TEST_ASSERT(ends == (accepted ? 1 : 5) && queries == 6);

			// A replacement connection retires active capture and may restart its wire IDs.
			sample.activePointCount = 1;
			a.info = EncodePointInfo(sample); a.eventIndex = 0;
			events->OnIOPointStarted(a);
			a.eventIndex = 1; events->OnIOPointDown(a);
			events->OnControllerDisconnect();
			TEST_ASSERT(queries == 7 && ends == (accepted ? 2 : 5) && borrowed->Count() == 0);
			TEST_ASSERT(!GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing());
			a.eventIndex = 2; events->OnIOPointUpdate(a);
			events->OnIOPointEnded(end);
			TEST_ASSERT(queries == 7 && ends == (accepted ? 2 : 5));
			events->OnControllerConnect(MakeGlobalConfig());
			a.eventIndex = 0; events->OnIOPointStarted(a);
			a.eventIndex = 1; events->OnIOPointDown(a);
			a.eventIndex = 2; a.info.activePointCount = 0; events->OnIOPointUp(a);
			end.sequenceId = a.info.sequenceId; end.eventIndex = 3; end.reason = PointSequenceEndReason::Succeeded;
			events->OnIOPointEnded(end);
			TEST_ASSERT(queries == 8 && ends == (accepted ? 3 : 5) && borrowed->Count() == 0);
		});
		protocol.OnNextFrame([&]() { controlHost->Hide(); });
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});


	TEST_CATEGORY(L"Core automation and renderer wire identities share the window allocator")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		RemoteProtocolAutomationService automation;
		bool accepted = true;
		vuint64_t sequence = 0;
		vint queries = 0, ends = 0, clicks = 0;
		bool validSequences = true, validEnds = true;
		auto run = [&](const wchar_t* command)
		{
			TEST_ASSERT(automation.RunIOCommand({}, WString::Unmanaged(command)) == L"Queued");
		};
		protocol.OnNextFrame([&]()
		{
			auto button = new GuiButton(theme::ThemeName::Button);
			button->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 110, 110));
			controlHost->GetContainerComposition()->AddChild(button->GetBoundsComposition());
			button->Clicked.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { clicks++; });
			auto receiver = button->GetBoundsComposition()->GetEventReceiver();
			receiver->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition* sender, GuiPointSequenceStartedEventArgs& a)
			{
				validSequences &= a.sequenceId > sequence;
				sequence = a.sequenceId;
				queries++;
				a.acceptedComposition = accepted ? sender : nullptr;
			});
			receiver->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& a)
			{
				validEnds &= a.sequenceId == sequence && a.accepted == accepted;
				ends++;
			});
			controlHost->ForceCalculateSizeImmediately();
			run(L"!PointDown:7,30,30"); run(L"!PointUp:7,30,30");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 1 && ends == (accepted ? 1 : 0) && clicks == 0);
			auto events = protocol.GetEvents();
			NativeWindowPointInfo sample;
			sample.sequenceId = 1; sample.activePointCount = 1; sample.position = NativePoint(30, 30);
			IOPointAction a; a.eventIndex = 0; a.info = EncodePointInfo(sample);
			events->OnIOPointStarted(a);
			a.eventIndex = 1; events->OnIOPointDown(a);
			a.eventIndex = 2; a.info.activePointCount = 0; events->OnIOPointUp(a);
			IOPointEnd end; end.sequenceId = L"1"; end.eventIndex = 3; end.timestamp = L"0";
			end.reason = PointSequenceEndReason::Succeeded;
			events->OnIOPointEnded(end);
			TEST_ASSERT(queries == 2 && ends == 2 && clicks == 0);
			run(L"!PointDown:7,30,30"); run(L"!PointUp:7,30,30");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 3 && ends == 3 && clicks == 0);
			accepted = false;
			run(L"!PointDown:7,30,30"); run(L"!PointDown:8,40,40");
			run(L"!PointUp:7,30,30"); run(L"!PointUp:8,40,40");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 4 && ends == 3 && clicks == 1);
			run(L"!PointDown:7,30,30"); run(L"!PointCancel:7,CaptureLost");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 5 && ends == 3 && clicks == 1);
			TEST_ASSERT(!GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing());
			run(L"!PointDown:7,30,30"); run(L"!PointUp:7,30,30");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 6 && ends == 3 && clicks == 2 && validSequences && validEnds);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CASE(L"Remote renderer owns native point sequences")
	{
		MouseInputRenderer renderer(nullptr);
		NativeWindowPointInfo info;
		info.sequenceId = 1;
		info.activePointCount = 1;
		bool accept = false;
		renderer.PointSequenceStarted(info, accept);
		TEST_ASSERT(accept);
	});
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
	TEST_CATEGORY(L"Alt and Super survive mouse serialization, hosted routing and renderer batching")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		NativeWindowMouseInfo expected;
		NativeMouseButton expectedButton = NativeMouseButton::Left;
		vint buttons = 0;
		List<WindowMouseInfo> wheels;

		protocol.OnNextFrame([&]()
		{
			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			auto receiver = controlHost->GetBoundsComposition()->GetEventReceiver();
			auto checkButton = [&](GuiGraphicsComposition*, GuiMouseEventArgs& args)
			{
				TEST_ASSERT(args.alt == expected.alt && args.osSuper == expected.osSuper);
				TEST_ASSERT(args.ctrl == expected.ctrl && args.shift == expected.shift);
				TEST_ASSERT(args.button == expectedButton);
				TEST_ASSERT(args.left == expected.left && args.middle == expected.middle && args.right == expected.right);
				buttons++;
			};
			receiver->mouseDown.AttachLambda(checkButton);
			receiver->mouseUp.AttachLambda(checkButton);
			receiver->mouseDoubleClick.AttachLambda(checkButton);
			auto checkWheel = [&](GuiGraphicsComposition*, GuiMouseEventArgs& args)
			{
				wheels.Add(args);
			};
			receiver->horizontalWheel.AttachLambda(checkWheel);
			receiver->verticalWheel.AttachLambda(checkWheel);
		});
		protocol.OnNextFrame([&]()
		{
			MouseInputRenderer renderer(protocol.GetEvents());
			expected.x = 20;
			expected.y = 30;
			renderer.MouseMoving(expected);
			renderer.SendAccumulatedMessages();
			for (vint mask = 0; mask < 16; mask++)
			{
				expected.ctrl = (mask & 1) != 0;
				expected.shift = (mask & 2) != 0;
				expected.alt = (mask & 4) != 0;
				expected.osSuper = (mask & 8) != 0;
				NativeWindowMouseInfo restored;
				ConvertJsonToCustomType(ConvertCustomTypeToJson(expected), restored);
				TEST_ASSERT(restored.ctrl == expected.ctrl && restored.shift == expected.shift);
				TEST_ASSERT(restored.alt == expected.alt && restored.osSuper == expected.osSuper);
				for (auto button : { NativeMouseButton::Left, NativeMouseButton::Middle, NativeMouseButton::Right, NativeMouseButton::Mouse4, NativeMouseButton::Mouse5 })
				{
					expectedButton = button;
					expected.left = button == NativeMouseButton::Left;
					expected.middle = button == NativeMouseButton::Middle;
					expected.right = button == NativeMouseButton::Right;
					renderer.MouseDown(button, expected);
					expected.left = expected.middle = expected.right = false;
					renderer.MouseUp(button, expected);
					expected.left = button == NativeMouseButton::Left;
					expected.middle = button == NativeMouseButton::Middle;
					expected.right = button == NativeMouseButton::Right;
					renderer.MouseDoubleClick(button, expected);
					expected.left = expected.middle = expected.right = false;
					renderer.MouseUp(button, expected);
				}
			}
			TEST_ASSERT(buttons == 16 * 5 * 4);
			for (auto horizontal : { false, true })
			{
				wheels.Clear();
				expected = {};
				expected.x = 20;
				expected.y = 30;
				auto wheel = [&](const NativeWindowMouseInfo& info)
				{
					if (horizontal) renderer.HorizontalWheel(info);
					else renderer.VerticalWheel(info);
				};
				expected.wheel = 60;
				wheel(expected);
				wheel(expected);
				expected.alt = true;
				expected.wheel = -120;
				wheel(expected);
				TEST_ASSERT(wheels.Count() == 1 && !wheels[0].alt && !wheels[0].osSuper && wheels[0].wheel == 120);
				expected.osSuper = true;
				expected.wheel = 240;
				wheel(expected);
				TEST_ASSERT(wheels.Count() == 2 && wheels[1].alt && !wheels[1].osSuper && wheels[1].wheel == -120);
				renderer.SendAccumulatedMessages();
				TEST_ASSERT(wheels.Count() == 3 && wheels[2].alt && wheels[2].osSuper && wheels[2].wheel == 240);
			}
			expected.wheel = 120;
			renderer.VerticalWheel(expected);
			renderer.HorizontalWheel(expected);
			renderer.MouseMoving(expected);
			auto before = wheels.Count();
			renderer.DisconnectFromCore();
			renderer.SendAccumulatedMessages();
			renderer.VerticalWheel(expected);
			renderer.SendAccumulatedMessages();
			TEST_ASSERT(wheels.Count() == before);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

}
