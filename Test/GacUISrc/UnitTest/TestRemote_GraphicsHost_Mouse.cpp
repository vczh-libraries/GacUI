#include "TestRemote_GraphicsHost_Shared.h"
#include "TestPointInput.h"
#include "TestControls.h"
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include <cmath>

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
	void OnPointGesture(GuiGraphicsComposition* sender, GuiPointGestureEventArgs& arguments)override
	{
		arguments.handled = consume;
		GuiScrollView::OnPointGesture(sender, arguments);
	}
	void OnPointSequenceEnded(GuiGraphicsComposition* sender, GuiPointSequenceEndedEventArgs& arguments)override
	{
		arguments.handled = consume;
		GuiScrollView::OnPointSequenceEnded(sender, arguments);
	}
public:
	bool consume = false;
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
	TEST_CATEGORY(L"Borrowed context menus consume only eligible right-button releases")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiControl* parent = nullptr;
		GuiControl* child = nullptr;
		GuiBoundsComposition* descendant = nullptr;
		GuiPopup* parentMenu = nullptr;
		GuiPopup* childMenu = nullptr;
		Ptr<GuiDisposedFlag> firstDisposed, secondDisposed;
		vint parentOpened = 0, childOpened = 0;
		bool suppress = false;
		protocol.OnNextFrame([&]()
		{
			using namespace vl::reflection::description;
			auto controlType = GetTypeDescriptor<GuiControl>();
			auto control = new GuiControl(theme::ThemeName::CustomControl);
			auto first = new GuiPopup(theme::ThemeName::Window);
			auto second = new GuiPopup(theme::ThemeName::Window);
			firstDisposed = first->GetDisposedFlag();
			secondDisposed = second->GetDisposedFlag();
			TEST_ASSERT(control->GetContextMenu() == nullptr);
			TEST_ASSERT(control->GetDisplayContextMenuOnRightClick());
			control->SetContextMenu(first);
			control->SetDisplayContextMenuOnRightClick(false);
			TEST_ASSERT(control->GetContextMenu() == first);
			TEST_ASSERT(!control->GetDisplayContextMenuOnRightClick());
			auto menuProperty = controlType->GetPropertyByName(L"ContextMenu", true);
			auto displayProperty = controlType->GetPropertyByName(L"DisplayContextMenuOnRightClick", true);
			TEST_ASSERT(UnboxValue<GuiPopup*>(menuProperty->GetValue(Value::From(control))) == first);
			auto controlValue = Value::From(control);
			menuProperty->SetValue(controlValue, Value::From(second));
			displayProperty->SetValue(controlValue, BoxValue(true));
			TEST_ASSERT(control->GetContextMenu() == second);
			TEST_ASSERT(UnboxValue<bool>(displayProperty->GetValue(Value::From(control))));
			TEST_ASSERT(!firstDisposed->IsDisposed());
			control->SetContextMenu(nullptr);
			TEST_ASSERT(control->GetContextMenu() == nullptr && !secondDisposed->IsDisposed());
			control->SetContextMenu(second);
			SafeDeleteControl(control);
			TEST_ASSERT(!firstDisposed->IsDisposed() && !secondDisposed->IsDisposed());
			SafeDeleteControl(first);
			SafeDeleteControl(second);

			parent = new GuiControl(theme::ThemeName::CustomControl);
			child = new GuiControl(theme::ThemeName::CustomControl);
			parent->GetBoundsComposition()->SetExpectedBounds(Rect(20, 30, 400, 350));
			child->GetBoundsComposition()->SetExpectedBounds(Rect(40, 50, 160, 140));
			parent->AddChild(child);
			controlHost->AddChild(parent);
			descendant = new GuiBoundsComposition;
			descendant->SetExpectedBounds(Rect(10, 11, 50, 40));
			child->GetContainerComposition()->AddChild(descendant);
			descendant->GetEventReceiver()->mouseUp.AttachLambda([&](auto*, GuiMouseEventArgs& arguments)
			{
				arguments.handled = suppress;
			});
			parentMenu = new GuiPopup(theme::ThemeName::Window);
			childMenu = new GuiPopup(theme::ThemeName::Window);
			parentMenu->SetClientSize(Size(80, 60));
			childMenu->SetClientSize(Size(80, 60));
			parentMenu->WindowOpened.AttachLambda([&](auto*, GuiEventArgs&) { parentOpened++; });
			childMenu->WindowOpened.AttachLambda([&](auto*, GuiEventArgs&) { childOpened++; });
			parent->SetContextMenu(parentMenu);
			child->SetContextMenu(childMenu);
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			auto receiver = child->GetBoundsComposition()->GetEventReceiver();
			auto dispatch = [&](NativeMouseButton button, bool handled)
			{
				GuiMouseEventArgs arguments(child->GetBoundsComposition());
				arguments.button = button;
				arguments.handled = handled;
				arguments.x = 15;
				arguments.y = 16;
				receiver->mouseUp.Execute(arguments);
				return arguments.handled;
			};
			for (auto button : { NativeMouseButton::Left,NativeMouseButton::Middle,NativeMouseButton::Mouse4,NativeMouseButton::Mouse5 })
			{
				TEST_ASSERT(!dispatch(button, false));
				TEST_ASSERT(!childMenu->GetOpening());
			}
			TEST_ASSERT(dispatch(NativeMouseButton::Right, true));
			TEST_ASSERT(!childMenu->GetOpening());
			child->SetContextMenu(nullptr);
			TEST_ASSERT(!dispatch(NativeMouseButton::Right, false));
			child->SetContextMenu(childMenu);
			child->SetDisplayContextMenuOnRightClick(false);
			TEST_ASSERT(!dispatch(NativeMouseButton::Right, false));
			child->SetDisplayContextMenuOnRightClick(true);
			protocol.MouseMove(NativePoint(75, 96));
			protocol._RDown();
			TEST_ASSERT(childOpened == 0 && parentOpened == 0);
			protocol._RUp();
			TEST_ASSERT(childOpened == 1 && parentOpened == 0 && childMenu->GetOpening());
			TEST_ASSERT(childMenu->GetLocation() == NativePoint(75, 96));
			childMenu->Hide();
			child->SetContextMenu(nullptr);
			protocol.RClick(NativePoint(75, 96));
			TEST_ASSERT(childOpened == 1 && parentOpened == 1 && parentMenu->GetOpening());
			TEST_ASSERT(parentMenu->GetLocation() == NativePoint(75, 96));
			parentMenu->Hide();
			suppress = true;
			protocol.RClick(NativePoint(75, 96));
			TEST_ASSERT(childOpened == 1 && parentOpened == 1);
			suppress = false;
			child->SetContextMenu(childMenu);
			auto bounds = child->GetBoundsComposition();
			child->SetDisplayContextMenuOnRightClick(false);
			child->RefreshThemes();
			TEST_ASSERT(child->GetBoundsComposition() == bounds && child->GetContextMenu() == childMenu);
			TEST_ASSERT(!child->GetDisplayContextMenuOnRightClick());
			child->SetControlTemplate([](const description::Value&)
			{
				auto result = new templates::GuiControlTemplate;
				result->SetContainerComposition(result);
				return result;
			});
			child->SetDisplayContextMenuOnRightClick(true);
			GetApplication()->RefreshThemes();
			TEST_ASSERT(child->GetBoundsComposition() == bounds && child->GetContextMenu() == childMenu);
			TEST_ASSERT(child->GetDisplayContextMenuOnRightClick());
			parent->GetBoundsComposition()->SetExpectedBounds(Rect(100, 80, 480, 400));
			controlHost->ForceCalculateSizeImmediately();
			protocol.RClick(NativePoint(155, 146));
			TEST_ASSERT(childOpened == 2 && parentOpened == 1);
			TEST_ASSERT(childMenu->GetLocation() == NativePoint(155, 146));
			childMenu->Hide();
			parent->GetBoundsComposition()->SetExpectedBounds(Rect(480, 360, 640, 480));
			controlHost->ForceCalculateSizeImmediately();
			protocol.RClick(NativePoint(620, 460));
			TEST_ASSERT(childOpened == 3 && parentOpened == 1);
			auto screen = controlHost->GetRelatedScreen()->GetClientBounds();
			auto popup = childMenu->GetNativeWindow()->GetBounds();
			TEST_ASSERT(popup.x2 == screen.x2 && popup.y2 == screen.y2);
			TEST_ASSERT(popup.x1 >= screen.x1 && popup.y1 >= screen.y1);
			childMenu->Hide();
			child->SetContextMenu(nullptr);
			parent->SetContextMenu(nullptr);
			SafeDeleteControl(childMenu);
			SafeDeleteControl(parentMenu);
		});
		protocol.OnNextFrame([&]()
		{
			// SafeDeleteControl defers control-host destruction until queued events finish.
			TEST_ASSERT(firstDisposed->IsDisposed() && secondDisposed->IsDisposed());
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	for (bool accepted : { false,true })
	TEST_CATEGORY(L"Touch opens a modal window with reentrant input, accepted=" + itow(accepted))
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiWindow* dialog = nullptr;
		GuiButton* opener = nullptr;
		GuiButton* closer = nullptr;
		PointProtocolInput input(nullptr, false);
		vint clicks = 0;
		bool nestedInput = true;
		NativePoint dialogPoint;
		protocol.OnNextFrame([&]()
		{
			input.events = protocol.GetEvents();
			opener = new GuiButton(theme::ThemeName::Button);
			opener->SetAcceptPointEvents(accepted);
			opener->GetBoundsComposition()->SetExpectedBounds(Rect(20, 20, 120, 80));
			controlHost->AddChild(opener);
			dialog = new GuiWindow(theme::ThemeName::Window);
			dialog->SetClientSize({ 200,200 });
			closer = new GuiButton(theme::ThemeName::Button);
			closer->SetAcceptPointEvents(true);
			closer->GetBoundsComposition()->SetExpectedBounds(Rect(20, 20, 120, 80));
			dialog->AddChild(closer);
			closer->Clicked.AttachLambda([&](auto*, auto&) { clicks++; dialog->Hide(); });
			opener->Clicked.AttachLambda([&](auto*, auto&)
			{
				clicks++;
				dialog->ShowModal(controlHost, []() {});
				dialog->GetNativeWindow()->SetBounds(NativeRect(250, 100, 450, 300));
				dialog->ForceCalculateSizeImmediately();
				auto origin = dialog->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				auto mainOrigin = controlHost->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				dialogPoint = NativePoint(origin.x - mainOrigin.x + 50, origin.y - mainOrigin.y + 50);
				// A synchronous dialog pumps the remaining End and a new Down before this click returns.
				if (nestedInput)
				{
					if (!accepted) input.End();
					input.Start(dialogPoint);
					TEST_ASSERT(closer->GetPointEventComposition() == closer->GetBoundsComposition());
				}
			});
			controlHost->ForceCalculateSizeImmediately();
		});
		for (vint i = 0; i < 2; i++)
		protocol.OnNextFrame([&, i]()
		{
			input.Start(NativePoint(50, 50));
			input.Up(0, NativePoint(50, 50));
			if (accepted) input.End();
			TEST_ASSERT(clicks == i * 2 + 1);
			TEST_ASSERT(closer->GetPointEventComposition() == closer->GetBoundsComposition());
			input.Up(0, dialogPoint);
			input.End();
			TEST_ASSERT(clicks == i * 2 + 2 && !dialog->GetOpening());
			TEST_ASSERT(!closer->GetPointEventComposition());
		});
		protocol.OnNextFrame([&]()
		{
			nestedInput = false;
			protocol.LClick(NativePoint(50, 50));
			protocol.LClick(dialogPoint);
			TEST_ASSERT(clicks == 6 && !dialog->GetOpening());
		});
		protocol.OnNextFrame([&]()
		{
			SafeDeleteControl(dialog);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Generated clicks hit children and bubble independently of point ownership")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiControl* parent = nullptr;
		GuiButton* child = nullptr;
		GuiBoundsComposition* hit = nullptr;
		bool handleChild = false;
		PointProtocolInput input(nullptr, false);
		vint clicks = 0;
		vint phases[2][3] = {};
		protocol.OnNextFrame([&]()
		{
			parent = new GuiControl(theme::ThemeName::CustomControl);
			parent->SetAcceptPointEvents(true);
			parent->GetBoundsComposition()->SetExpectedBounds(Rect(20, 30, 320, 230));
			controlHost->AddChild(parent);
			child = new GuiButton(theme::ThemeName::Button);
			child->GetBoundsComposition()->SetExpectedBounds(Rect(30, 40, 130, 90));
			parent->AddChild(child);
			hit = new GuiBoundsComposition;
			hit->SetAlignmentToParent(Margin(0, 0, 0, 0));
			child->GetContainerComposition()->AddChild(hit);
			child->Clicked.AttachLambda([&](auto*, auto&) { clicks++; });
			for (vint i = 0; i < 2; i++)
			{
				auto receiver = (i ? child->GetBoundsComposition() : parent->GetBoundsComposition())->GetEventReceiver();
				auto record = [&, i](GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments, vint phase)
				{
					TEST_ASSERT(arguments.eventSource == child->GetBoundsComposition() && arguments.compositionSource == hit);
					TEST_ASSERT(sender == (i ? child->GetBoundsComposition() : parent->GetBoundsComposition()));
					TEST_ASSERT(Point(arguments.x, arguments.y) == (i ? Point(20, 20) : Point(50, 60)));
					phases[i][phase]++;
					if (i && handleChild) arguments.handled = true;
				};
				receiver->mouseDown.AttachLambda([=](auto* sender, auto& args) { record(sender, args, 0); });
				receiver->mouseUp.AttachLambda([=](auto* sender, auto& args) { record(sender, args, 1); });
				receiver->mouseDoubleClick.AttachLambda([=](auto* sender, auto& args) { record(sender, args, 2); });
			}
			controlHost->ForceCalculateSizeImmediately();
			input.events = protocol.GetEvents();
		});
		protocol.OnNextFrame([&]()
		{
			auto clock = Ptr(new gacui_unittest_template::TooltipTimer);
			for (bool childAccepts : { false,true })
			{
				child->SetAcceptPointEvents(childAccepts);
				input.Start(NativePoint(70, 90));
				TEST_ASSERT(parent->GetPointEventComposition() == (childAccepts ? child->GetBoundsComposition() : parent->GetBoundsComposition()));
				input.Up(0, NativePoint(70, 90)); input.End(100000);
			}
			TEST_ASSERT(clicks == 2);
			TEST_ASSERT(phases[0][0] == 2 && phases[0][1] == 2 && phases[0][2] == 1);
			TEST_ASSERT(phases[1][0] == 2 && phases[1][1] == 2 && phases[1][2] == 1);
			protocol.LClick(NativePoint(70, 90));
			TEST_ASSERT(clicks == 3);
			TEST_ASSERT(phases[0][0] == 3 && phases[0][1] == 3 && phases[0][2] == 1);
			TEST_ASSERT(phases[1][0] == 3 && phases[1][1] == 3 && phases[1][2] == 1);
			handleChild = true;
			clock->Advance(500);
			input.Start(NativePoint(70, 90));
			input.Up(0, NativePoint(70, 90)); input.End(100000);
			TEST_ASSERT(clicks == 4);
			TEST_ASSERT(phases[0][0] == 3 && phases[0][1] == 3 && phases[0][2] == 1);
			TEST_ASSERT(phases[1][0] == 4 && phases[1][1] == 4 && phases[1][2] == 1);
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Deferred left clicks pair by UTC generation time")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiControl* surface = nullptr;
		List<WString> mouseEvents;
		Point expectedPosition(80, 70);
		Func<void()> nestedClick;
		Func<void()> onEnd;
		protocol.OnNextFrame([&]()
		{
			surface = new GuiControl(theme::ThemeName::CustomControl);
			surface->SetAcceptPointEvents(true);
			surface->GetBoundsComposition()->SetExpectedBounds(Rect(20, 30, 220, 230));
			controlHost->AddChild(surface);
			auto hit = new GuiBoundsComposition;
			hit->SetExpectedBounds(Rect(10, 10, 180, 180));
			surface->GetContainerComposition()->AddChild(hit);
			hit->GetEventReceiver();
			auto receiver = surface->GetBoundsComposition()->GetEventReceiver();
			receiver->pointSequenceEnded.AttachLambda([&](auto*, GuiPointSequenceEndedEventArgs&) { if (onEnd) onEnd(); });
			auto record = [&, hit](const wchar_t* name, GuiMouseEventArgs& a, bool pressed)
			{
				TEST_ASSERT(!surface->GetPointEventComposition());
				TEST_ASSERT(a.eventSource == hit);
				TEST_ASSERT(a.compositionSource == hit);
				TEST_ASSERT(a.button == NativeMouseButton::Left);
				TEST_ASSERT(a.left == pressed && !a.right && !a.middle);
				TEST_ASSERT(a.ctrl && a.shift && a.alt && a.osSuper);
				TEST_ASSERT(Point(a.x, a.y) == expectedPosition);
				mouseEvents.Add(name);
			};
			receiver->mouseMove.AttachLambda([record](auto*, GuiMouseEventArgs& a) { record(L"Move", a, false); });
			receiver->mouseDown.AttachLambda([&, record](auto*, GuiMouseEventArgs& a)
			{
				record(L"Down", a, true);
				if (nestedClick)
				{
					auto click = nestedClick;
					nestedClick = {};
					click();
				}
			});
			receiver->mouseDoubleClick.AttachLambda([record](auto*, GuiMouseEventArgs& a) { record(L"Double", a, true); });
			receiver->mouseUp.AttachLambda([record](auto*, GuiMouseEventArgs& a) { record(L"Up", a, false); });
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			auto clock = Ptr(new gacui_unittest_template::TooltipTimer);
			PointProtocolInput input(protocol.GetEvents());
			auto tap = [&]()
			{
				input.Start(NativePoint(100, 100));
				input.Up(0, NativePoint(103, 104));
				input.End(100000);
			};
			for (auto interval : { 0,499,500,501 })
			{
				clock->Advance(500);
				tap();
				AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
				clock->Advance(interval);
				tap();
				if (interval < 500) AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
				else AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			}
			clock->Advance(500);
			for (vint i = 0; i < 4; i++)
			{
				tap();
				if (i % 2 == 0) AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
				else AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
			}
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			clock->Advance(501);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			clock->Advance(499);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");

			// Commit the first click before its Down callback pumps another complete sequence.
			nestedClick = tap;
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Move", L"Down", L"Double", L"Up", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");

			auto host = surface->GetBoundsComposition()->GetRelatedGraphicsHost();
			auto window = host->GetNativeWindow();
			host->SetNativeWindow(window);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			host->SetNativeWindow(nullptr);
			host->SetNativeWindow(window);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			GuiWindow replacement(theme::ThemeName::Window);
			host->SetNativeWindow(replacement.GetNativeWindow());
			host->SetNativeWindow(window);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			onEnd = [&]() { clock->Advance(500); };
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			onEnd = {};
			clock->Advance(499);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			clock = nullptr;
			clock = Ptr(new gacui_unittest_template::TooltipTimer);
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Up");
			tap();
			AssertEventLogs(mouseEvents, L"Move", L"Down", L"Double", L"Up");
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Only generated left clicks update host-local pairing")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiWindow* otherWindow = nullptr;
		GuiControl* surfaces[3] = {};
		vint doubles[3] = {}, downs[3] = {}, ups[3] = {};
		vint flags = 3, selectGesture = 0;
		bool accepted = true;
		protocol.OnNextFrame([&]()
		{
			otherWindow = new GuiWindow(theme::ThemeName::Window);
			otherWindow->SetClientSize(Size(240, 230));
			for (vint i = 0; i < 3; i++)
			{
				auto surface = surfaces[i] = new GuiControl(theme::ThemeName::CustomControl);
				surface->GetBoundsComposition()->SetExpectedBounds(i == 1 ? Rect(240, 30, 440, 230) : Rect(20, 30, 220, 230));
				(i == 2 ? otherWindow : controlHost)->AddChild(surface);
				auto receiver = surface->GetBoundsComposition()->GetEventReceiver();
				receiver->pointSequenceStarted.AttachLambda([&, surface](auto*, GuiPointSequenceStartedEventArgs& a)
				{
					if (accepted) a.acceptedComposition = surface->GetBoundsComposition();
					a.generateLeftButtonClick = (flags & 1) != 0;
					a.generateRightButtonClick = (flags & 2) != 0;
				});
				receiver->pointGesture.AttachLambda([&](auto*, GuiPointGestureEventArgs& a)
				{
					a.selected = selectGesture == 1 && (a.gesture.TryGet<GuiPointGesture_ShortPressing>() || a.gesture.TryGet<GuiPointGesture_LongPressing>())
						|| selectGesture == 2 && a.gesture.TryGet<GuiPointGesture_Scrolling>()
						|| selectGesture == 3 && a.gesture.TryGet<GuiPointGesture_DraggingAndRotation>();
				});
				receiver->mouseDown.AttachLambda([&, i](auto*, GuiMouseEventArgs&) { downs[i]++; });
				receiver->mouseUp.AttachLambda([&, i](auto*, GuiMouseEventArgs&) { ups[i]++; });
				receiver->mouseDoubleClick.AttachLambda([&, i](auto*, GuiMouseEventArgs& a)
				{
					TEST_ASSERT(a.button == NativeMouseButton::Left && a.left);
					TEST_ASSERT(!surfaces[i]->GetPointEventComposition());
					doubles[i]++;
				});
			}
			otherWindow->ShowWithOwner(controlHost);
			otherWindow->GetNativeWindow()->SetBounds(NativeRect(400, 250, 640, 480));
			otherWindow->ForceCalculateSizeImmediately();
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			gacui_unittest_template::TooltipTimer clock;
			PointProtocolInput input(protocol.GetEvents(), false);
			auto tap = [&](NativePoint position, vuint64_t duration = 100000)
			{
				input.Start(position);
				input.At(duration);
				input.Up(0, position);
				input.End(duration);
			};
			tap(NativePoint(100, 100));
			tap(NativePoint(500, 350));
			TEST_ASSERT(doubles[0] == 0 && doubles[2] == 0);
			tap(NativePoint(300, 100));
			tap(NativePoint(500, 350));
			TEST_ASSERT(doubles[0] == 0 && doubles[1] == 1 && doubles[2] == 1);

			// Neither physical mouse events nor rejected point compatibility can prime or consume a pair.
			for (bool prime : { false,true })
			for (vint scenario = 0; scenario < 11; scenario++)
			{
				clock.Advance(500);
				auto before = doubles[0];
				if (prime) tap(NativePoint(100, 100));
				flags = scenario == 0 ? 0 : scenario == 1 ? 2 : 3;
				selectGesture = scenario == 2 ? 1 : scenario == 6 ? 2 : scenario == 7 ? 3 : 0;
				accepted = scenario != 9;
				if (scenario == 10)
				{
					protocol.LClick(NativePoint(100, 100));
				}
				else
				{
					input.Start(NativePoint(100, 100));
					if (scenario == 4) { input.Down(1, NativePoint(100, 100)); input.Up(1, NativePoint(100, 100)); }
					if (scenario == 5 || scenario == 6 || scenario == 7) { input.Move(NativePoint(110, 100)); input.Move(NativePoint(100, 100)); }
					if (scenario != 3) input.Up(0, NativePoint(100, 100));
					input.End(scenario == 8 ? 500000 : 100000, scenario == 3 ? PointSequenceEndReason::Canceled_PlatformCanceled : PointSequenceEndReason::Succeeded);
				}
				TEST_ASSERT(doubles[0] == before);
				flags = 3; selectGesture = 0; accepted = true;
				tap(NativePoint(100, 100));
				TEST_ASSERT(doubles[0] == before + (prime ? 1 : 0));
			}
			clock.Advance(500);
			flags = 1;
			auto before = doubles[0];
			tap(NativePoint(100, 100), 500000);
			TEST_ASSERT(doubles[0] == before);
			tap(NativePoint(100, 100), 500000);
			TEST_ASSERT(doubles[0] == before + 1);
			flags = 3;

			// A synthetic pair targets its own start position while physical capture stays on another control.
			for (auto button : { NativeMouseButton::Left,NativeMouseButton::Right })
			{
				clock.Advance(500);
				auto physical = MakeMouseInfoWithButton(button, button == NativeMouseButton::Left, false, button == NativeMouseButton::Right, 300, 100, 0);
				protocol.GetEvents()->OnIOButtonDown(physical);
				auto window = GetCurrentController()->WindowService()->GetMainWindow();
				TEST_ASSERT(window->IsCapturing());
				auto physicalDowns = downs[1], physicalUps = ups[1];
				tap(NativePoint(100, 100));
				tap(NativePoint(100, 100));
				TEST_ASSERT(window->IsCapturing());
				TEST_ASSERT(downs[1] == physicalDowns && ups[1] == physicalUps);
				physical.info.left = physical.info.right = false;
				physical.info.x = physical.info.y = 100;
				protocol.GetEvents()->OnIOButtonUp(physical);
				TEST_ASSERT(ups[1] == physicalUps + 1 && !window->IsCapturing());
			}
			for (vint i = 0; i < 3; i++) TEST_ASSERT(downs[i] == ups[i]);
			delete otherWindow;
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Press offers are independent of deferred click flags")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiControl* surface = nullptr;
		vint flags = 0, response = 0, offers = 0, ends = 0;
		vint selectMoving = 0;
		bool expectLong = false;
		Point expectedPosition(80, 70);
		bool ending = false;
		List<WString> order;
		List<NativeMouseButton> clicks;
		protocol.OnNextFrame([&]()
		{
			surface = new GuiControl(theme::ThemeName::CustomControl);
			surface->SetAcceptPointEvents(true);
			surface->GetBoundsComposition()->SetExpectedBounds(Rect(20, 30, 220, 230));
			controlHost->GetContainerComposition()->AddChild(surface->GetBoundsComposition());
			auto receiver = surface->GetBoundsComposition()->GetEventReceiver();
			receiver->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& a)
			{
				a.generateLeftButtonClick = (flags & 1) != 0;
				a.generateRightButtonClick = (flags & 2) != 0;
			});
			receiver->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs& a)
			{
				if (a.eventType == GuiPointEventsType::Up) order.Add(L"Up");
			});
			receiver->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				auto shortPress = a.gesture.TryGet<GuiPointGesture_ShortPressing>();
				auto longPress = a.gesture.TryGet<GuiPointGesture_LongPressing>();
				if (!shortPress && !longPress)
				{
					a.selected = (selectMoving == 1 && a.gesture.TryGet<GuiPointGesture_Scrolling>()) || (selectMoving == 2 && a.gesture.TryGet<GuiPointGesture_DraggingAndRotation>());
					return;
				}
				TEST_ASSERT(ending);
				TEST_ASSERT((longPress != nullptr) == expectLong);
				TEST_ASSERT((shortPress ? shortPress->position : longPress->position) == expectedPosition);
				offers++;
				order.Add(L"Press");
				TEST_ASSERT(a.compositionSource == surface->GetBoundsComposition() && a.eventSource == a.compositionSource);
				TEST_ASSERT(surface->GetPointEventComposition() == a.compositionSource);
				a.selected = response == 1;
				a.handled = response != 0;
			});
			receiver->pointGesture.AttachLambda([](GuiGraphicsComposition*, GuiPointGestureEventArgs& a) { a.selected = false; });
			receiver->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				if (a.gesture.TryGet<GuiPointGesture_ShortPressing>() || a.gesture.TryGet<GuiPointGesture_LongPressing>()) TEST_ASSERT(a.selected == (response == 1));
			});
			receiver->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& a)
			{
				ends++;
				order.Add(L"End");
				TEST_ASSERT(surface->GetPointEventComposition() == a.compositionSource);
				if (a.reason == PointSequenceEndReason::Succeeded)
				{
					for (auto point : a.GetPoints()) TEST_ASSERT(!point);
				}
				a.handled = true;
			});
			receiver->mouseMove.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs&)
			{
				TEST_ASSERT(!surface->GetPointEventComposition());
				order.Add(L"MouseMove");
			});
			receiver->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				TEST_ASSERT(!surface->GetPointEventComposition());
				clicks.Add(a.button);
				order.Add(L"MouseDown");
			});
			receiver->mouseUp.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs&)
			{
				TEST_ASSERT(!surface->GetPointEventComposition());
				order.Add(L"MouseUp");
			});
			controlHost->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			PointProtocolInput input(protocol.GetEvents(), false);
			for (flags = 0; flags < 4; flags++)
			for (response = 0; response < 3; response++)
			for (auto duration : { 499999,500000 })
			{
				expectLong = duration == 500000;
				order.Clear(); clicks.Clear(); offers = ends = 0; ending = false;
				input.Start(NativePoint(100, 100));
				input.At(duration);
				input.Up(0, NativePoint(100, 100));
				TEST_ASSERT(offers == 0 && ends == 0 && clicks.Count() == 0);
				ending = true;
				input.End(duration);
				TEST_ASSERT(offers == 1 && ends == 1 && !surface->GetPointEventComposition());
				auto right = duration == 500000 && (flags & 2) != 0;
				auto click = response != 1 && (right || (flags & 1) != 0);
				TEST_ASSERT(clicks.Count() == (click ? 1 : 0));
				if (click) TEST_ASSERT(clicks[0] == (right ? NativeMouseButton::Right : NativeMouseButton::Left));
				TEST_ASSERT(order.Count() == (click ? 6 : 3));
				TEST_ASSERT(order[0] == L"Up" && order[1] == L"Press" && order[2] == L"End");
				if (click) TEST_ASSERT(order[3] == L"MouseMove" && order[4] == L"MouseDown" && order[5] == L"MouseUp");
			}
			flags = 3; response = 0; expectLong = false;
			for (vint scenario = 0; scenario < 10; scenario++)
			{
				offers = ends = 0; clicks.Clear(); ending = false;
				selectMoving = scenario == 8 ? 1 : scenario == 9 ? 2 : 0;
				input.Start(NativePoint(100, 100));
				if (scenario == 4) { input.Move(NativePoint(109, 100)); input.Move(NativePoint(100, 100)); }
				if (scenario == 5) { input.Down(1, NativePoint(100, 100)); input.Up(1, NativePoint(100, 100)); }
				if (scenario == 8) input.Move(NativePoint(100, 120));
				if (scenario != 6)
				{
					auto position = scenario == 0 ? NativePoint(108, 100) : scenario == 1 ? NativePoint(100, 108) : scenario == 2 ? NativePoint(106, 106) : scenario == 3 ? NativePoint(109, 100) : NativePoint(100, 100);
					input.Up(0, position);
				}
				TEST_ASSERT(offers == 0);
				ending = true;
				input.End(499999, scenario == 6 ? PointSequenceEndReason::Canceled_PlatformCanceled : PointSequenceEndReason::Succeeded);
				auto eligible = scenario < 2 || scenario == 7;
				TEST_ASSERT(offers == (eligible ? 1 : 0) && ends == 1 && clicks.Count() == (eligible ? 1 : 0));
				TEST_ASSERT(!surface->GetPointEventComposition());
			}
			flags = 0; response = 1; selectMoving = 0;
			for (auto duration : { 499999,500000 })
			{
				auto window = surface->GetBoundsComposition()->GetRelatedGraphicsHost()->GetNativeWindow();
				auto bounds = window->GetBounds();
				auto origin = window->GetClientBoundsInScreen().LeftTop();
				auto firstScreen = NativePoint(origin.x + 100, origin.y + 100);
				offers = ends = 0; ending = false; expectLong = duration == 500000;
				input.Start(NativePoint(100, 100));
				input.Up(0, NativePoint(100, 100));
				auto screen = GraphicsHostProtocol::MakeUnitTestScreenConfig().screenConfig;
				screen.scalingX = 2.0; screen.scalingY = 1.25;
				protocol.GetEvents()->OnControllerScreenUpdated(screen);
				window->SetBounds(NativeRect(bounds.x1 + 20, bounds.y1 + 30, bounds.x2 + 20, bounds.y2 + 30));
				controlHost->ForceCalculateSizeImmediately();
				origin = window->GetClientBoundsInScreen().LeftTop();
				auto logical = window->Convert(NativePoint(firstScreen.x - origin.x, firstScreen.y - origin.y));
				auto target = surface->GetBoundsComposition()->GetGlobalBounds().LeftTop();
				expectedPosition = Point(logical.x - target.x, logical.y - target.y);
				ending = true;
				input.End(duration);
				TEST_ASSERT(offers == 1 && ends == 1 && !surface->GetPointEventComposition());
				window->SetBounds(bounds);
				screen.scalingX = screen.scalingY = 1.0;
				protocol.GetEvents()->OnControllerScreenUpdated(screen);
				controlHost->ForceCalculateSizeImmediately();
			}
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Gesture selection, incremental manipulation and raw lifecycle")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiWindow* pointWindow = nullptr;
		GuiControl* surface = nullptr;
		bool manipulate = true, chooseScroll = false;
		vint actions = 0, ends = 0, clicks = 0, gestures = 0, initialOffers = 0;
		double x = 150, y = 150;
		List<GuiPointGesture_Scrolling> scrolling;
		protocol.OnNextFrame([&]()
		{
			pointWindow = new GuiWindow(theme::ThemeName::CustomFrameWindow);
			pointWindow->SetClientSize(Size(400, 400));
			surface = new GuiControl(theme::ThemeName::CustomControl);
			surface->SetAcceptPointEvents(true);
			surface->GetBoundsComposition()->SetExpectedBounds(Rect(0, 0, 400, 400));
			pointWindow->GetContainerComposition()->AddChild(surface->GetBoundsComposition());
			auto receiver = surface->GetBoundsComposition()->GetEventReceiver();
			receiver->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs& a) { actions++; a.handled = true; });
			receiver->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				TEST_ASSERT(a.compositionSource == surface->GetBoundsComposition() && a.eventSource == a.compositionSource);
				TEST_ASSERT(surface->GetPointEventComposition() == a.compositionSource);
				gestures++;
				if (auto transform = a.gesture.TryGet<GuiPointGesture_DraggingAndRotation>())
				{
					if (!a.selected)
					{
						initialOffers++;
						TEST_ASSERT(transform->rotation == 0 && transform->scale == 1 && transform->distance == Point());
					}
					if (!manipulate) return;
					a.selected = true;
					auto real = transform->scale * std::cos(transform->rotation);
					auto imaginary = transform->scale * std::sin(transform->rotation);
					auto nx = real * x - imaginary * y + transform->distance.x;
					y = imaginary * x + real * y + transform->distance.y;
					x = nx;
				}
				else if (auto scroll = a.gesture.TryGet<GuiPointGesture_Scrolling>())
				{
					scrolling.Add(*scroll);
					a.selected = chooseScroll;
				}
				a.handled = true;
			});
			receiver->pointGesture.AttachLambda([](GuiGraphicsComposition*, GuiPointGestureEventArgs& a) { a.selected = false; });
			receiver->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				TEST_ASSERT(a.selected == (a.gesture.TryGet<GuiPointGesture_Scrolling>() ? chooseScroll : a.gesture.TryGet<GuiPointGesture_DraggingAndRotation>() ? manipulate : false));
			});
			receiver->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& a)
			{
				ends++;
				TEST_ASSERT(surface->GetPointEventComposition() == a.compositionSource);
				a.handled = true;
			});
			receiver->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs&) { clicks++; });
			pointWindow->ShowWithOwner(controlHost);
			pointWindow->GetNativeWindow()->SetBounds(NativeRect(0, 0, 400, 400));
			pointWindow->ForceCalculateSizeImmediately();
		});
		protocol.OnNextFrame([&]()
		{
			PointProtocolInput input(protocol.GetEvents(), false);
			auto position = [&](double ex, double ey)
			{
				TEST_ASSERT(std::abs(x - ex) < 0.001 && std::abs(y - ey) < 0.001);
			};
			input.Start(NativePoint(100, 100));
			TEST_ASSERT(initialOffers == 1 && actions == 1);
			position(150, 150);
			input.Move(NativePoint(110, 120)); position(160, 170);
			auto screen = GraphicsHostProtocol::MakeUnitTestScreenConfig().screenConfig;
			screen.scalingX = 1.5; screen.scalingY = 1.25;
			protocol.GetEvents()->OnControllerScreenUpdated(screen);
			input.Move(NativePoint(110, 120)); position(160, 170);
			screen.scalingX = screen.scalingY = 1.0;
			protocol.GetEvents()->OnControllerScreenUpdated(screen);
			auto window = surface->GetBoundsComposition()->GetRelatedGraphicsHost()->GetNativeWindow();
			auto bounds = window->GetBounds();
			window->SetBounds(NativeRect(bounds.x1 + 20, bounds.y1 + 30, bounds.x2 + 20, bounds.y2 + 30));
			TEST_ASSERT(window->GetClientBoundsInScreen().LeftTop() == NativePoint(bounds.x1 + 20, bounds.y1 + 30));
			input.Move(NativePoint(110, 120)); position(160, 170);
			window->SetBounds(bounds);
			input.Down(1, NativePoint(210, 120)); position(160, 170);
			screen.scalingX = 1.5; screen.scalingY = 1.25;
			protocol.GetEvents()->OnControllerScreenUpdated(screen);
			input.Move(NativePoint(210, 120), 1); position(160, 170);
			window->SetBounds(NativeRect(bounds.x1 + 20, bounds.y1 + 30, bounds.x2 + 20, bounds.y2 + 30));
			TEST_ASSERT(window->GetClientBoundsInScreen().LeftTop() == NativePoint(bounds.x1 + 20, bounds.y1 + 30));
			input.Move(NativePoint(110, 120)); position(160, 170);
			window->SetBounds(bounds);
			screen.scalingX = screen.scalingY = 1.0;
			protocol.GetEvents()->OnControllerScreenUpdated(screen);
			input.Move(NativePoint(210, 120), 1); position(160, 170);
			input.Move(NativePoint(110, 220), 1); position(60, 170);
			input.Move(NativePoint(110, 320), 1); position(10, 220);
			input.Move(NativePoint(210, 120), 1); position(160, 170);
			input.Down(2, NativePoint(250, 250));
			input.Move(NativePoint(260, 260), 2); position(160, 170);
			input.Up(2, NativePoint(260, 260)); position(160, 170);
			input.Move(NativePoint(210, 120), 1); position(160, 170);
			input.Up(1, NativePoint(210, 120));
			input.Down(3, NativePoint(110, 120)); position(160, 170);
			input.Move(NativePoint(210, 120), 3); position(160, 170);
			input.Move(NativePoint(210, 120), 3); position(160, 170);
			for (vint i = 0; i < 50; i++)
			{
				input.Move(NativePoint(209, 133), 3);
				auto stationaryX = x, stationaryY = y;
				input.Move(NativePoint(209, 133), 3);
				position(stationaryX, stationaryY);
				input.Move(NativePoint(210, 120), 3);
			}
			TEST_ASSERT(std::abs(x - 160) <= 0.51 && std::abs(y - 170) <= 0.51);
			input.Up(3, NativePoint(210, 120));
			auto beforeX = x, beforeY = y;
			input.Up(0, NativePoint(113, 124)); input.End();
			position(beforeX + 3, beforeY + 4);
			TEST_ASSERT(ends == 1 && !surface->GetPointEventComposition() && clicks == 0 && scrolling.Count() == 0);

			manipulate = false;
			input.Start(NativePoint(100, 100));
			input.Move(NativePoint(106, 106));
			TEST_ASSERT(scrolling.Count() == 1 && !scrolling[0].isVertical && scrolling[0].distance == 6);
			chooseScroll = true;
			input.Move(NativePoint(110, 150));
			TEST_ASSERT(scrolling.Count() == 2 && scrolling[1].distance == 4 && !scrolling[1].isVertical);
			input.Down(1, NativePoint(200, 200));
			auto count = gestures;
			input.Up(1, NativePoint(200, 200));
			input.Move(NativePoint(80, 80));
			input.Up(0, NativePoint(70, 70)); input.End();
			TEST_ASSERT(gestures == count && ends == 2 && clicks == 0 && initialOffers == 2);

			chooseScroll = false;
			input.Start(NativePoint(100, 100));
			input.Up(0, NativePoint(100, 100)); input.End();
			TEST_ASSERT(ends == 3 && clicks == 1 && initialOffers == 3);
			manipulate = true;
			input.Start(NativePoint(100, 100)); input.End(0, PointSequenceEndReason::Canceled_PlatformCanceled);
			TEST_ASSERT(ends == 4 && clicks == 1 && initialOffers == 4 && !surface->GetPointEventComposition());
			delete pointWindow;
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

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
			List<GuiPointGesture_Scrolling> scrolling;
			surface->GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				if (auto payload = a.gesture.TryGet<GuiPointGesture_Scrolling>()) scrolling.Add(*payload);
			});
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
				TEST_ASSERT(scrolling.Count() == 0);
			}
			auto bounds = window->GetBounds();
			window->SetBounds(NativeRect(bounds.x1 + 20, bounds.y1 + 30, bounds.x2 + 20, bounds.y2 + 30));
			expectedFirst = local(NativePoint(101, 103));
			expectedSecond = local(NativePoint(203, 157));
			{ auto lt = local(NativePoint(98, 99)); auto rb = local(NativePoint(104, 107)); expectedContact = Rect(lt.x, lt.y, rb.x, rb.y); }
			input.Move(NativePoint(101, 103));
			TEST_ASSERT(scrolling.Count() == 0);
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
			TEST_ASSERT(scrolling.Count() == 0);
			input.Move(NativePoint(116, 103));
			TEST_ASSERT(scrolling.Count() == 1 && !scrolling[0].isVertical && scrolling[0].distance == 10);
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
			auto resetView = [&]() { scroll->SetViewPosition(Point(300, 300)); };
			auto finish = [&](NativePoint p)
			{
				input.Up(0, p);
				input.End();
				TEST_ASSERT(!scroll->GetPointEventComposition() && !controlHost->GetPointEventComposition());
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
			TEST_ASSERT(scroll->GetViewPosition() == Point(300, 300));
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
			TEST_ASSERT(scroll->GetViewPosition() == Point(300, 300));
			finish(NativePoint(120, 120));
			input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			TEST_ASSERT(clicks == 2 && downs.Count() == 3);
			// A handled End does not veto an automatic click.
			input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			TEST_ASSERT(clicks == 3 && downs.Count() == 4);
			scroll->consume = false;
			for (auto reason : { PointSequenceEndReason::Canceled_Unknown, PointSequenceEndReason::Canceled_PlatformCanceled, PointSequenceEndReason::Canceled_CaptureLost, PointSequenceEndReason::Canceled_WindowDestroyed })
			{
				input.Start(NativePoint(50, 50));
				input.End(1000000, reason);
				TEST_ASSERT(!scroll->GetPointEventComposition());
				input.Start(NativePoint(50, 50)); finish(NativePoint(50, 50));
			}
			TEST_ASSERT(clicks == 7 && downs.Count() == 8 && ups.Count() == downs.Count());
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
			TEST_ASSERT(clicks == 9 && downs.Count() == 10 && ups.Count() == downs.Count() && physicalUps.Count() == 2);
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
			IOPointEnd end = {};
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
				TEST_ASSERT(a.ctrl && a.shift && a.alt && a.osSuper);
				if (accepted) TEST_ASSERT(!controlHost->GetPointEventComposition());
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
			TEST_ASSERT(mouseDowns == (accepted ? 6 : 1));
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
			TEST_ASSERT(queries == 1 && ends == (accepted ? 1 : 0) && clicks == 1);
			auto events = protocol.GetEvents();
			NativeWindowPointInfo sample;
			sample.sequenceId = 1; sample.activePointCount = 1; sample.position = NativePoint(30, 30);
			IOPointAction a; a.eventIndex = 0; a.info = EncodePointInfo(sample);
			events->OnIOPointStarted(a);
			a.eventIndex = 1; events->OnIOPointDown(a);
			a.eventIndex = 2; a.info.activePointCount = 0; events->OnIOPointUp(a);
			IOPointEnd end = {}; end.sequenceId = L"1"; end.eventIndex = 3; end.timestamp = L"0";
			end.reason = PointSequenceEndReason::Succeeded;
			events->OnIOPointEnded(end);
			TEST_ASSERT(queries == 2 && ends == 2 && clicks == 2);
			run(L"!PointDown:7,30,30"); run(L"!PointUp:7,30,30");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 3 && ends == 3 && clicks == 3);
			accepted = false;
			run(L"!PointDown:7,30,30"); run(L"!PointDown:8,40,40");
			run(L"!PointUp:7,30,30"); run(L"!PointUp:8,40,40");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 4 && ends == 3 && clicks == 4);
			run(L"!PointDown:7,30,30"); run(L"!PointCancel:7,CaptureLost");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 5 && ends == 3 && clicks == 4);
			TEST_ASSERT(!GetCurrentController()->WindowService()->GetMainWindow()->IsCapturing());
			run(L"!PointDown:7,30,30"); run(L"!PointUp:7,30,30");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(queries == 6 && ends == 3 && clicks == 5 && validSequences && validEnds);
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
