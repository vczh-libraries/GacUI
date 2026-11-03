#include "TestControls.h"
#include "TestPointInput.h"
#include "TestRemote.h"
#include "../../../Source/GacUI.h"
#include <limits>
#include <cmath>

extern void SetGuiMainProxy(const Func<void()>& proxy);

using namespace gacui_unittest_template;

namespace vl::reflection::description
{
	extern bool LoadDarkSkinTypes();
}

class PointScrollingItemArranger : public list::FixedHeightItemArranger
{
public:
	PointScrollingItemArranger()
	{
		GetRepeatComposition()->SetItemWidth(650);
	}
};

struct PointScrollingState : Object
{
	Ptr<PointProtocolInput> input;
	NativePoint origin;
	TextPos caretBegin, caretEnd;
	vint rightClicks = 0;
};

struct PointSwipeState : Object
{
	Ptr<PointProtocolInput> input;
	GuiBindableTextList* list = nullptr;
	Value page, target;
	Ptr<IValueList> items;
	NativePoint origin;
	vint maximum = 0;
};

void AssertTouchingListNoRed(GuiGraphicsComposition* composition)
{
	if (!composition->GetVisible()) return;
	if (auto background = composition->GetOwnedElement().Cast<GuiSolidBackgroundElement>())
	{
		TEST_ASSERT(background->GetColor() != Color(184, 32, 48));
	}
	for (auto child : composition->Children())
	{
		AssertTouchingListNoRed(child);
	}
}

class MomentumTestControl : public GuiControl
{
public:
	MomentumTestControl() : GuiControl(theme::ThemeName::CustomControl)
	{
		SetControlTemplate([](const Value&) { return new templates::GuiControlTemplate; });
	}
};

class MomentumTestRoot : public GuiInstanceRootObject
{
protected:
	GuiControlHost* GetControlHostForInstance()override { return nullptr; }
public:
	~MomentumTestRoot() { FinalizeInstance(); }
	void Tick()
	{
		StartPendingAnimations();
		collections::List<Ptr<IGuiAnimation>> snapshot;
		CopyFrom(snapshot, runningAnimations);
		for (auto animation : snapshot)
		{
			animation->Run();
			if (animation->GetStopped()) runningAnimations.Remove(animation.Obj());
		}
	}
	void Pause()
	{
		for (auto animation : runningAnimations) animation->Pause();
	}
};

class MomentumTestScrollView : public GuiScrollView
{
protected:
	Size QueryFullSize()override { return Size(1000, 1000); }
	void UpdateView(Rect)override {}
public:
	MomentumTestScrollView(bool hasScroll) : GuiScrollView(theme::ThemeName::CustomControl)
	{
		SetControlTemplate([=](const Value&)
		{
			auto ct = new templates::GuiScrollViewTemplate;
			if (hasScroll)
			{
				auto scroll = new GuiScroll(theme::ThemeName::CustomControl);
				scroll->SetControlTemplate([](const Value&) { return new templates::GuiScrollTemplate; });
				ct->AddChild(scroll->GetBoundsComposition());
				ct->SetVerticalScroll(scroll);
			}
			return ct;
		});
	}
};

class MomentumTestInput
{
public:
	GuiControl* control;
	collections::List<Nullable<Point>> points;
	vuint64_t sequence = 0;

	MomentumTestInput(GuiControl* value) : control(value) {}
	void Start(bool accepted = true)
	{
		GuiPointSequenceStartedEventArgs arguments(control->GetBoundsComposition(), points);
		arguments.sequenceId = ++sequence;
		arguments.timestamp = 1000000;
		if (accepted) arguments.acceptedComposition = control->GetBoundsComposition();
		control->GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.Execute(arguments);
	}
	void Move(vuint64_t time, vint distance, bool vertical = true, vint contact = 0)
	{
		auto bounds = control->GetBoundsComposition();
		GuiPointEventArgs action(bounds, contact == 0 ? GuiPointEventsType::Update : GuiPointEventsType::Down, points);
		action.sequenceId = sequence;
		action.timestamp = 1000000 + time;
		action.pointId = contact;
		bounds->GetEventReceiver()->pointAction.Execute(action);
		GuiPointGestureEventArgs gesture(bounds);
		gesture.gesture = GuiPointGesture_Scrolling{ vertical,distance };
		gesture.handled = true;
		bounds->GetEventReceiver()->pointGesture.Execute(gesture);
	}
	void End(vuint64_t time, PointSequenceEndReason reason = PointSequenceEndReason::Succeeded)
	{
		auto bounds = control->GetBoundsComposition();
		GuiPointSequenceEndedEventArgs arguments(bounds, points);
		arguments.sequenceId = sequence;
		arguments.timestamp = 1000000 + time;
		arguments.reason = reason;
		bounds->GetEventReceiver()->pointSequenceEnded.Execute(arguments);
	}
};

template<typename T>
class PointSelectionDocument : public T
{
public:
	PointSelectionDocument(theme::ThemeName themeName, const GuiDocumentConfig& config) : T(themeName, config) {}
	GuiBoundsComposition* DocumentComposition() { return this->documentComposition; }
};

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"MomentumScrolling is available to native and Workflow consumers")
		{
			TEST_ASSERT(GetTypeDescriptor<GuiComponent>() != nullptr);
			auto type = GetGlobalTypeManager()->GetTypeDescriptor(L"presentation::compositions::MomentumScrolling");
			TEST_ASSERT(type != nullptr);
			TEST_ASSERT(type->GetConstructorGroup()->GetMethodCount() == 1);
			TEST_ASSERT(type->GetMethodGroupByName(L"AttachControl", false) != nullptr);
			TEST_ASSERT(type->GetMethodGroupByName(L"DetachControl", false) != nullptr);
			TEST_ASSERT(type->GetPropertyByName(L"Deceleration", false) != nullptr);
		});

		TEST_CASE(L"Momentum samples the trailing interval and integrates elapsed deceleration")
		{
			for (auto multiplier : { 0.5,1.0,2.0 })
			for (auto vertical : { false,true })
			for (auto sign : { -1,1 })
			for (vint scenario = 0; scenario < 11; scenario++)
			{
				TooltipTimer clock;
				MomentumTestRoot root;
				auto momentum = new MomentumScrolling;
				root.AddComponent(momentum);
				MomentumTestControl control;
				momentum->AttachControl(&control);
				momentum->AttachControl(&control);
				TEST_ASSERT(!control.GetAcceptPointEvents());
				momentum->SetDeceleration(multiplier);
				MomentumTestInput input(&control);
				input.Start();
				double expected = 160 / multiplier;
				switch (scenario)
				{
				case 0: input.Move(500000, sign * 160, vertical); input.End(500000); break;
				case 1: input.Move(100000, sign * 32, vertical); input.End(100000); break;
				case 2: input.Move(500000, sign * 160, vertical); input.Move(750000, sign * 80, vertical); input.End(750000); break;
				case 3: input.Move(100000, sign * 10000, vertical); input.Move(600000, sign * 160, vertical); input.End(600000); break;
				case 4: input.Move(100000, sign * 160, vertical); input.End(1000000); expected = 0; break;
				case 5: input.Move(0, sign * 160, vertical); input.End(0); expected = 0; break;
				case 6: input.Move(250000, sign * 240, vertical); input.Move(500000, -sign * 80, vertical); input.End(500000); break;
				case 7: input.Move(250000, sign * 80, vertical); input.Move(250000, sign * 80, vertical); input.End(500000); break;
				case 8: input.Move(500000, sign * 1, vertical); input.End(500000); expected = 0; break;
				case 9: input.Move(250000, sign * 160, vertical); input.Move(500000, -sign * 160, vertical); input.End(500000); expected = 0; break;
				case 10: input.Move(500000, sign * 50, vertical); input.End(500000); expected = std::round(15.625 / multiplier); break;
				}
				momentum->SetDeceleration(100);
				vint total = 0;
				control.GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& args)
				{
					TEST_ASSERT(!args.handled);
					TEST_ASSERT(args.eventSource == control.GetBoundsComposition() && args.compositionSource == args.eventSource);
					auto scrolling = args.gesture.Get<GuiPointGesture_Scrolling>();
					TEST_ASSERT(scrolling.isVertical == vertical);
					TEST_ASSERT(scrolling.distance * sign > 0);
					total += scrolling.distance;
				});
				root.Tick();
				clock.Advance(99); root.Tick(); TEST_ASSERT(total == 0);
				clock.Advance(1); root.Tick();
				if (scenario < 4 || scenario == 6 || scenario == 7) TEST_ASSERT(total * sign > 0);
				clock.Advance(1900); root.Tick();
				TEST_ASSERT(total == sign * (vint)expected);
				clock.Advance(3000); root.Tick();
				TEST_ASSERT(total == sign * (vint)expected);
			}
		});

		TEST_CASE(L"Momentum default emits elapsed tenth-second integrals and stops at one second")
		{
			TooltipTimer clock;
			MomentumTestRoot root;
			auto momentum = new MomentumScrolling;
			root.AddComponent(momentum);
			MomentumTestControl control;
			momentum->AttachControl(&control);
			MomentumTestInput input(&control);
			input.Start(); input.Move(500000, 160); input.End(500000);
			vint total = 0, count = 0;
			control.GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](auto*, auto& args)
			{
				total += args.gesture.template Get<GuiPointGesture_Scrolling>().distance;
				count++;
			});
			root.Tick();
			for (vint i = 1; i <= 10; i++)
			{
				clock.Advance(100); root.Tick();
				auto time = i / 10.0;
				TEST_ASSERT(total == (vint)std::round(320 * time - 160 * time * time));
				TEST_ASSERT(count == i);
			}
			clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 160 && count == 10);
		});

		TEST_CASE(L"Momentum rounding cannot reverse on a near-boundary final interval")
		{
			for (auto sign : { -1,1 })
			{
				TooltipTimer clock;
				MomentumTestRoot root;
				auto momentum = new MomentumScrolling;
				root.AddComponent(momentum);
				MomentumTestControl control;
				momentum->AttachControl(&control);
				momentum->SetDeceleration(std::nextafter(0.3125, 0.0));
				MomentumTestInput input(&control);
				input.Start(); input.Move(500000, sign * 5); input.End(500000);
				vint total = 0, count = 0;
				control.GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](auto*, auto& args)
				{
					auto distance = args.gesture.template Get<GuiPointGesture_Scrolling>().distance;
					TEST_ASSERT(distance * sign > 0);
					total += distance;
					count++;
				});
				root.Tick(); clock.Advance(100); root.Tick();
				TEST_ASSERT(total == sign && count == 1);
				clock.Advance(1); root.Tick();
				TEST_ASSERT(total == sign && count == 1);
			}
		});

		TEST_CASE(L"Momentum control lifetime and reentrant detachment are independent")
		{
			for (vint scenario = 0; scenario < 10; scenario++)
			{
				TooltipTimer clock;
				MomentumTestRoot root;
				auto momentum = new MomentumScrolling;
				root.AddComponent(momentum);
				auto first = new MomentumTestControl;
				MomentumTestControl second;
				momentum->AttachControl(first);
				momentum->AttachControl(&second);
				MomentumTestInput a(first), b(&second);
				a.Start(); b.Start();
				a.Move(500000, 160); b.Move(500000, 160);
				if (scenario == 0) a.Move(500000, 0, true, 1);
				a.End(500000, scenario == 1 ? PointSequenceEndReason::Canceled_CaptureLost : PointSequenceEndReason::Succeeded);
				b.End(500000);
				vint totalA = 0, totalB = 0;
				first->GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](auto*, auto& args)
				{
					totalA += args.gesture.template Get<GuiPointGesture_Scrolling>().distance;
					if (scenario == 2) momentum->DetachControl(first);
					if (scenario == 3) momentum->DetachControl(&second);
				});
				second.GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](auto*, auto& args)
				{
					totalB += args.gesture.template Get<GuiPointGesture_Scrolling>().distance;
				});
				root.Tick();
				if (scenario == 4) a.Start(false);
				if (scenario == 5) { momentum->DetachControl(first); momentum->AttachControl(first); }
				if (scenario == 6) { delete first; first = nullptr; }
				if (scenario == 7) momentum->EnvironmentChanged();
				if (scenario == 8) root.Pause();
				if (scenario == 9) root.FinalizeInstance();
				clock.Advance(1000); root.Tick();
				if (scenario == 2) TEST_ASSERT(totalA == 30);
				else if (scenario == 3) TEST_ASSERT(totalA == 160);
				else TEST_ASSERT(totalA == 0);
				if (scenario < 7 && scenario != 3) TEST_ASSERT(totalB == 160);
				clock.Advance(1000); root.Tick();
				if (first) delete first;
			}
		});

		TEST_CASE(L"Momentum deceleration rejects invalid values")
		{
			MomentumScrolling momentum;
			TEST_ASSERT(momentum.GetDeceleration() == 1.0);
			for (auto value : { 0.0,-1.0,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN() })
			{
				TEST_ERROR(momentum.SetDeceleration(value));
				TEST_ASSERT(momentum.GetDeceleration() == 1.0);
			}
		});

		TEST_CASE(L"Post-End scrolling preserves target, scrollbar and boundary guards")
		{
			for (vint scenario = 0; scenario < 7; scenario++)
			{
				MomentumTestScrollView view(scenario != 0);
				auto bar = view.GetVerticalScroll();
				if (bar) { bar->SetTotalSize(1000); bar->SetPageSize(100); }
				if (scenario == 1) bar->SetEnabled(false);
				if (scenario == 2) bar->SetVisible(false);
				if (scenario == 3) view.SetEnabled(false);
				GuiPointGestureEventArgs arguments(view.GetBoundsComposition());
				arguments.gesture = GuiPointGesture_Scrolling{ scenario != 4,-10000 };
				if (scenario == 5) arguments.eventSource = nullptr;
				view.GetBoundsComposition()->GetEventReceiver()->pointGesture.Execute(arguments);
				TEST_ASSERT(arguments.handled == (scenario == 6));
				if (scenario == 6)
				{
					TEST_ASSERT(bar->GetPosition() == bar->GetTotalSize() - bar->GetPageSize());
					arguments.handled = false;
					arguments.gesture = GuiPointGesture_Scrolling{ true,10000 };
					view.GetBoundsComposition()->GetEventReceiver()->pointGesture.Execute(arguments);
				}
				TEST_ASSERT(view.GetViewPosition() == Point());
			}
		});

		TEST_CASE(L"Momentum ignores child ownership and late attachment, and retires idle disposal")
		{
			TooltipTimer clock;
			MomentumTestRoot root;
			auto momentum = new MomentumScrolling;
			root.AddComponent(momentum);
			auto disposed = new MomentumTestControl;
			momentum->AttachControl(disposed);
			delete disposed;
			MomentumTestControl parent, child;
			momentum->AttachControl(&parent);
			MomentumTestInput input(&parent);
			collections::List<Nullable<Point>> points;
			GuiPointSequenceStartedEventArgs started(parent.GetBoundsComposition(), points);
			started.sequenceId = ++input.sequence;
			started.timestamp = 1000000;
			started.acceptedComposition = child.GetBoundsComposition();
			parent.GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.Execute(started);
			input.Move(500000, 160); input.End(500000);
			vint total = 0;
			parent.GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](auto*, auto& args)
			{
				if (!args.handled) total += args.gesture.template Get<GuiPointGesture_Scrolling>().distance;
			});
			root.Tick(); clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 0);
			momentum->DetachControl(&parent);
			input.Start();
			momentum->AttachControl(&parent);
			input.Move(500000, 160); input.End(500000);
			root.Tick(); clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 0);
			input.Start(); input.Move(500000, 160); input.End(500000);
			root.Tick(); clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 160);
			momentum->SetDeceleration(std::numeric_limits<double>::max());
			input.Start(); input.Move(500000, 160); input.End(500000);
			root.Tick(); clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 160);
			momentum->SetDeceleration(1);
			auto handler = parent.GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.AttachLambda([](auto* sender, auto& args)
			{
				args.acceptedComposition = sender;
			});
			input.Start(false); input.Move(500000, 160); input.End(500000);
			root.Tick(); clock.Advance(1000); root.Tick();
			TEST_ASSERT(total == 320);
			parent.GetBoundsComposition()->GetEventReceiver()->pointSequenceStarted.Detach(handler);
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});

	for (vint family = 0; family < 3; family++)
	TEST_CASE(L"Exact document press selection, family " + itow(family))
	{
		UnitTestScreenConfig screen;
		screen.FastInitialize(800, 600);
		UnitTestRemoteProtocol protocol(L"DocumentPressAssertions", screen);
		GuiWindow* window = nullptr;
		GuiControl* control = nullptr;
		GuiDocumentCommonInterface* document = nullptr;
		GuiBoundsComposition* content = nullptr;
		GuiDocumentViewer* viewer = nullptr;
		GuiButton* other = nullptr;
		GuiButton* embeddedButton = nullptr;
		Ptr<PointProtocolInput> pointInput;
		vint shorts = 0, longs = 0, lefts = 0, rights = 0;
		vint hyperlinks = 0, childPresses = 0;
		bool selected = false, suppressClicks = false;
		auto position = [&](TextPos caret)
		{
			window->ForceCalculateSizeImmediately();
			auto bounds = document->GetCaretBounds(caret, true);
			return protocol.LocationOf(content, 0, 0, bounds.x1, (bounds.y1 + bounds.y2) / 2);
		};
		protocol.OnNextIdleFrame([&]()
		{
			pointInput = Ptr(new PointProtocolInput(protocol.GetEvents(), false));
			auto& input = *pointInput.Obj();
			auto press = [&](TextPos caret, bool longPress, bool expectSelected, bool expectClick = true)
			{
				shorts = longs = lefts = rights = 0; selected = false;
				auto origin = position(caret);
				input.Start(origin);
				TEST_ASSERT(control->GetPointEventComposition() == control->GetBoundsComposition());
				input.At(longPress ? 500000 : 499999);
				input.Up(0, origin);
				TEST_ASSERT(shorts == 0 && longs == 0 && lefts == 0 && rights == 0);
				input.End(longPress ? 500000 : 499999);
				TEST_ASSERT(shorts == (longPress ? 0 : 1) && longs == (longPress ? 1 : 0));
				TEST_ASSERT(selected == expectSelected && !control->GetPointEventComposition());
				TEST_ASSERT(lefts == (!longPress && !expectSelected && expectClick ? 1 : 0));
				TEST_ASSERT(rights == (longPress && !expectSelected && expectClick ? 1 : 0));
			};
			for (auto mode : { GuiDocumentEditMode::ViewOnly,GuiDocumentEditMode::Selectable,GuiDocumentEditMode::Editable })
			for (vint range = 0; range < 3; range++)
			for (vint column : { 0,2,4,6,8 })
			for (bool longPress : { false,true })
			{
				document->SetEditMode(mode);
				auto begin = TextPos(0, range == 0 ? 4 : range == 1 ? 2 : 6);
				auto end = TextPos(0, range == 0 ? 4 : range == 1 ? 6 : 2);
				auto hit = TextPos(0, column);
				document->SetCaret(begin, end);
				other->SetFocused();
				auto editable = mode != GuiDocumentEditMode::ViewOnly;
				auto consume = editable && (longPress ? range == 0 && hit != end : range != 0 && column >= 2 && column <= 6);
				press(hit, longPress, consume);
				if (editable && longPress && consume) end = hit;
				if (editable && !longPress && !consume && hit != end) begin = end = hit;
				TEST_ASSERT(document->GetCaretBegin() == begin && document->GetCaretEnd() == end);
				auto focused = control->GetBoundsComposition()->GetRelatedGraphicsHost()->GetFocusedComposition();
				TEST_ASSERT(focused == (editable && (!longPress || consume) ? control->GetFocusableComposition() : other->GetFocusableComposition()));
			}

			document->SetEditMode(GuiDocumentEditMode::Editable);
			document->SetCaret(TextPos(0, 4), TextPos(0, 4));
			other->SetFocused();
			control->SetEnabled(false);
			press(TextPos(0, 8), true, false);
			press(TextPos(0, 0), false, false);
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 4) && document->GetCaretEnd() == TextPos(0, 4));
			TEST_ASSERT(control->GetBoundsComposition()->GetRelatedGraphicsHost()->GetFocusedComposition() == other->GetFocusableComposition());
			control->SetEnabled(true);

			suppressClicks = true;
			press(TextPos(0, 8), true, true, false);
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 4) && document->GetCaretEnd() == TextPos(0, 8));
			press(TextPos(0, 0), false, false, false);
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 4) && document->GetCaretEnd() == TextPos(0, 8));
			suppressClicks = false;

			for (bool extraContact : { false,true })
			{
				shorts = longs = lefts = rights = 0;
				auto origin = position(TextPos(0, 0));
				input.Start(origin);
				if (extraContact)
				{
					input.Down(1, origin); input.Up(1, origin); input.Up(0, origin); input.End(500000);
				}
				else input.End(500000, PointSequenceEndReason::Canceled_PlatformCanceled);
				TEST_ASSERT(shorts == 0 && longs == 0 && lefts == 0 && rights == 0);
				TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 4) && document->GetCaretEnd() == TextPos(0, 8));
			}

			// Deferred clicks keep ordinary modifiers; keyboard selection still uses its existing path.
			document->SetCaret(TextPos(0, 2), TextPos(0, 2));
			input.modifiers = true;
			press(TextPos(0, 6), false, false);
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 2) && document->GetCaretEnd() == TextPos(0, 6));
			input.modifiers = false;
			protocol.KeyPress(VKEY::KEY_RIGHT, false, true, false);
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 2) && document->GetCaretEnd() == TextPos(0, 7));
			auto start = position(TextPos(0, 1)), finish = position(TextPos(0, 5));
			protocol.MouseMove(start); protocol._LDown(); protocol.MouseMove(finish); protocol._LUp();
			TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 1) && document->GetCaretEnd() == TextPos(0, 5));

			if (family == 2)
			{
				document->SetCaret(TextPos(0, 150), TextPos(0, 150));
				document->EnsureCaretVisible();
				window->ForceCalculateSizeImmediately();
				TEST_ASSERT(content->GetGlobalBounds().x1 < control->GetBoundsComposition()->GetGlobalBounds().x1);
				press(TextPos(0, 145), true, true);
				TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 150) && document->GetCaretEnd() == TextPos(0, 145));
			}
			else if (viewer)
			{
				document->SetCaret(TextPos(1, 4), TextPos(1, 4));
				viewer->SetViewPosition(Point(0, 20));
				press(TextPos(1, 8), true, true);
				TEST_ASSERT(document->GetCaretBegin() == TextPos(1, 4) && document->GetCaretEnd() == TextPos(1, 8));
				auto before = viewer->GetViewPosition();
				auto origin = protocol.LocationOf(viewer, 0.5, 0.8);
				input.Start(origin); input.Move(NativePoint(origin.x, origin.y - 30)); input.Up(0, NativePoint(origin.x, origin.y - 30)); input.End();
				TEST_ASSERT(viewer->GetViewPosition().y == before.y + 30);
				TEST_ASSERT(document->GetCaretBegin() == TextPos(1, 4) && document->GetCaretEnd() == TextPos(1, 8));
				document->SetCaret(TextPos(0, 4), TextPos(0, 4));
				suppressClicks = true;
				for (auto origin : { protocol.LocationOf(viewer->GetVerticalScroll()),protocol.LocationOf(control, 0, 0) })
				{
					longs = 0; selected = false;
					input.Start(origin); input.Up(0, origin); input.End(500000);
					TEST_ASSERT(longs == 1 && !selected);
					TEST_ASSERT(document->GetCaretBegin() == TextPos(0, 4) && document->GetCaretEnd() == TextPos(0, 4));
				}
				suppressClicks = false;
			}
			auto model = Ptr(new DocumentModel);
			auto paragraph = Ptr(new DocumentParagraphRun);
			auto text = Ptr(new DocumentTextRun);
			text->text = L"0123456789 ";
			paragraph->runs.Add(text);
			auto embedded = Ptr(new DocumentEmbeddedObjectRun);
			embedded->name = L"button";
			embedded->sizeOverride = Size(120, 30);
			paragraph->runs.Add(embedded);
			model->paragraphs.Add(paragraph);
			auto item = Ptr(new GuiDocumentItem(L"button"));
			embeddedButton = new GuiButton(theme::ThemeName::Button);
			embeddedButton->SetText(L"Embedded control");
			embeddedButton->GetBoundsComposition()->SetPreferredMinSize(Size(120, 30));
			item->GetContainer()->AddChild(embeddedButton->GetBoundsComposition());
			TEST_ASSERT(document->AddDocumentItem(item));
			document->SetDocument(model);
			document->EditHyperlink(0, 0, 10, L"touch-link");
			document->SetCaret(TextPos(), TextPos());
			document->EnsureCaretVisible();
			document->SetEditMode(GuiDocumentEditMode::ViewOnly);
			document->ActiveHyperlinkExecuted.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { hyperlinks++; });
			embeddedButton->GetBoundsComposition()->GetEventReceiver()->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& a)
			{
				if (a.gesture.TryGet<GuiPointGesture_LongPressing>()) childPresses++;
			});
		});
		protocol.OnNextIdleFrame([&]()
		{
			auto& input = *pointInput.Obj();
			auto origin = position(TextPos(0, 4));
			input.Start(origin); input.Up(0, origin); input.End(499999);
			TEST_ASSERT(hyperlinks == 1);
			TEST_ASSERT(document->GetCaretBegin() == TextPos() && document->GetCaretEnd() == TextPos());
			document->SetEditMode(GuiDocumentEditMode::Selectable);
			for (bool childAccepts : { false,true })
			{
				embeddedButton->SetAcceptPointEvents(childAccepts);
				other->SetFocused();
				longs = 0; selected = false;
				origin = protocol.LocationOf(embeddedButton);
				input.Start(origin);
				TEST_ASSERT(control->GetPointEventComposition() == (childAccepts ? embeddedButton->GetBoundsComposition() : control->GetBoundsComposition()));
				input.Up(0, origin); input.End(500000);
				TEST_ASSERT(longs == (childAccepts ? 0 : 1) && !selected);
				TEST_ASSERT(document->GetCaretBegin() == TextPos() && document->GetCaretEnd() == TextPos());
				TEST_ASSERT(!control->GetFocused());
			}
			TEST_ASSERT(childPresses == 1);
			if (viewer) control->SetControlTemplate([](const Value&) { return new darkskin::DocumentViewerTemplate; });
			else control->SetControlTemplate([](const Value&) { return new darkskin::DocumentLabelTemplate; });
		});
		protocol.OnNextIdleFrame([&]()
		{
			auto& input = *pointInput.Obj();
			shorts = longs = rights = 0; selected = false;
			auto origin = position(TextPos(0, 4));
			input.Start(origin); input.Up(0, origin); input.End(500000);
			TEST_ASSERT(shorts == 0 && longs == 1 && rights == 0 && selected);
			TEST_ASSERT(document->GetCaretBegin() == TextPos() && document->GetCaretEnd() == TextPos(0, 4));
			window->Hide();
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
			auto skin = Ptr(new darkskin::Theme);
			theme::RegisterTheme(skin);
			{
				GuiWindow host(theme::ThemeName::Window);
				window = &host;
				host.SetClientSize(Size(700, 500));
				GuiDocumentConfig config;
				config.wrapLine = family != 2;
				config.paragraphMode = family == 2 ? GuiDocumentParagraphMode::Singleline : GuiDocumentParagraphMode::Paragraph;
				config.autoExpand = family != 2;
				if (family == 0)
				{
					auto editor = new PointSelectionDocument<GuiDocumentViewer>(theme::ThemeName::DocumentViewer, config);
					control = viewer = editor; document = editor; content = editor->DocumentComposition();
				}
				else
				{
					auto editor = new PointSelectionDocument<GuiDocumentLabel>(theme::ThemeName::DocumentLabel, config);
					control = editor; document = editor; content = editor->DocumentComposition();
				}
				control->GetBoundsComposition()->SetExpectedBounds(Rect(20, 30, 420, family == 2 ? 65 : 330));
				host.AddChild(control);
				other = new GuiButton(theme::ThemeName::Button);
				other->SetText(L"Focus sentinel");
				other->GetBoundsComposition()->SetExpectedBounds(Rect(450, 30, 650, 65));
				host.AddChild(other);
				WString text;
				for (vint i = 0; i < (family == 2 ? 1 : 8); i++)
				{
					if (i) text += L"\r\n\r\n";
					for (vint j = 0; j < 20; j++) text += L"0123456789";
				}
				control->SetText(text);
				document->SetCaret(TextPos(), TextPos());
				auto receiver = control->GetBoundsComposition()->GetEventReceiver();
				receiver->pointSequenceStarted.AttachLambda([&](auto*, GuiPointSequenceStartedEventArgs& a)
				{
					if (suppressClicks) a.generateLeftButtonClick = a.generateRightButtonClick = false;
				});
				receiver->pointGesture.AttachLambda([&](auto*, GuiPointGestureEventArgs& a)
				{
					if (a.gesture.TryGet<GuiPointGesture_ShortPressing>()) { shorts++; selected = a.selected; }
					if (a.gesture.TryGet<GuiPointGesture_LongPressing>()) { longs++; selected = a.selected; }
				});
				receiver->mouseUp.AttachLambda([&](auto*, GuiMouseEventArgs& a)
				{
					if (a.button == NativeMouseButton::Left) lefts++;
					if (a.button == NativeMouseButton::Right) rights++;
				});
				GetApplication()->Run(&host);
			}
			theme::UnregisterTheme(skin->Name);
		});
		StartRemoteControllerTest(protocol);
	});

	for (auto suppressLeft : { true,false })
	TEST_CASE(suppressLeft ? L"Started suppresses deferred left clicks" : L"Started suppresses deferred right clicks")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::ClickWindow">
      <Window Text="Deferred point clicks" ClientSize="x:360 y:200">
        <Stack Direction="Vertical" Padding="8" AlignmentToParent="left:8 top:8 right:8 bottom:8">
          <ev.pointSequenceStarted-eval><![CDATA[{
            arguments.generateLeftButtonClick = not disableLeft.Selected;
            arguments.generateRightButtonClick = not disableRight.Selected;
          }]]></ev.pointSequenceStarted-eval>
          <StackItem><CheckBox ref.Name="target" Text="Tap activation">
            <att.BoundsComposition-set>
              <ev.pointGesture-eval><![CDATA[{
                if (var press : GuiPointGesture_ShortPressing? = arguments.gesture as GuiPointGesture_ShortPressing?)
                {
                  var changed : GuiPointGesture_ShortPressing = {position:{x:(press.position.x + 1) y:(press.position.y + 2)}};
                  arguments.gesture = changed of variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation, GuiPointGesture_ShortPressing, GuiPointGesture_LongPressing>;
                }
                if (var press : GuiPointGesture_LongPressing? = arguments.gesture as GuiPointGesture_LongPressing?)
                {
                  var changed : GuiPointGesture_LongPressing = {position:{x:(press.position.x + 1) y:(press.position.y + 2)}};
                  arguments.gesture = changed of variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation, GuiPointGesture_ShortPressing, GuiPointGesture_LongPressing>;
                }
              }]]></ev.pointGesture-eval>
            </att.BoundsComposition-set>
          </CheckBox></StackItem>
          <StackItem><CheckBox ref.Name="rightResult" Text="Right activation"/></StackItem>
          <StackItem><CheckBox ref.Name="disableLeft" Text="Suppress left click"/></StackItem>
          <StackItem><CheckBox ref.Name="disableRight" Text="Suppress right click"/></StackItem>
        </Stack>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";
		vint leftDown = 0, leftUp = 0, rightDown = 0, rightUp = 0, actions = 0, ends = 0, clicks = 0;
		Ptr<PointProtocolInput> input;
		GuiSelectableButton* target = nullptr;
		GuiSelectableButton* rightResult = nullptr;
		GuiSelectableButton* disableLeft = nullptr;
		GuiSelectableButton* disableRight = nullptr;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			input = Ptr(new PointProtocolInput(protocol->GetEvents(), false));
			auto sequence = [&, protocol](vuint64_t duration, vint left, vint right)
			{
				auto ld = leftDown, lu = leftUp, rd = rightDown, ru = rightUp, a = actions, e = ends, c = clicks;
				auto location = protocol->LocationOf(target);
				input->Start(location);
				TEST_ASSERT(target->GetPointEventComposition() == target->GetBoundsComposition());
				input->Up(0, location);
				input->End(duration);
				TEST_ASSERT(leftDown == ld + left && leftUp == lu + left);
				TEST_ASSERT(rightDown == rd + right && rightUp == ru + right);
				TEST_ASSERT(clicks == c + left && actions == a + 2 && ends == e + 1);
				TEST_ASSERT(!target->GetPointEventComposition());
			};
			protocol->OnNextIdleFrame(L"Ready", [&, sequence]()
			{
				auto window = GetApplication()->GetMainWindow();
				target = FindObjectByName<GuiSelectableButton>(window, L"target");
				rightResult = FindObjectByName<GuiSelectableButton>(window, L"rightResult");
				disableLeft = FindObjectByName<GuiSelectableButton>(window, L"disableLeft");
				disableRight = FindObjectByName<GuiSelectableButton>(window, L"disableRight");
				auto receiver = target->GetBoundsComposition()->GetEventReceiver();
				receiver->pointGesture.AttachLambda([&](GuiGraphicsComposition*, GuiPointGestureEventArgs& arguments)
				{
					auto shortPress = arguments.gesture.TryGet<GuiPointGesture_ShortPressing>();
					auto longPress = arguments.gesture.TryGet<GuiPointGesture_LongPressing>();
					if (shortPress || longPress)
					{
						auto bounds = target->GetBoundsComposition()->GetGlobalBounds();
						TEST_ASSERT((shortPress ? shortPress->position : longPress->position) == Point(bounds.Width() / 2 + 1, bounds.Height() / 2 + 2));
					}
				});
				receiver->pointSequenceStarted.AttachLambda([&](GuiGraphicsComposition* sender, GuiPointSequenceStartedEventArgs& arguments)
				{
					TEST_ASSERT(arguments.generateLeftButtonClick && arguments.generateRightButtonClick);
					arguments.acceptedComposition = sender;
					arguments.handled = true;
				});
				receiver->pointAction.AttachLambda([&](GuiGraphicsComposition*, GuiPointEventArgs& arguments)
				{
					actions++;
					arguments.handled = true;
				});
				receiver->pointSequenceEnded.AttachLambda([&](GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& arguments)
				{
					TEST_ASSERT(target->GetPointEventComposition() == target->GetBoundsComposition());
					TEST_ASSERT(arguments.points.Count() == 1 && !arguments.points[0]);
					ends++;
					arguments.handled = true;
				});
				receiver->mouseDown.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& arguments)
				{
					TEST_ASSERT(!target->GetPointEventComposition());
					if (arguments.button == NativeMouseButton::Left) leftDown++;
					if (arguments.button == NativeMouseButton::Right) rightDown++;
				});
				receiver->mouseUp.AttachLambda([&](GuiGraphicsComposition*, GuiMouseEventArgs& arguments)
				{
					if (arguments.button == NativeMouseButton::Left) leftUp++;
					if (arguments.button == NativeMouseButton::Right)
					{
						rightUp++;
						rightResult->SetSelected(!rightResult->GetSelected());
					}
				});
				target->Clicked.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { clicks++; });
				sequence(suppressLeft ? 0 : 500000, suppressLeft ? 1 : 0, suppressLeft ? 0 : 1);
			});
			protocol->OnNextIdleFrame(L"Default gesture activates", [&, sequence]()
			{
				disableLeft->SetSelected(suppressLeft);
				disableRight->SetSelected(!suppressLeft);
				if (suppressLeft)
				{
					sequence(0, 0, 0);
					TEST_ASSERT(target->GetSelected());
					sequence(500000, 0, 1);
				}
				else
				{
					sequence(500000, 1, 0);
					TEST_ASSERT(target->GetSelected());
				}
			});
			protocol->OnNextIdleFrame(L"Only enabled recognizer activates", [&, sequence]()
			{
				disableLeft->SetSelected(true);
				disableRight->SetSelected(true);
				sequence(0, 0, 0);
				sequence(500000, 0, 0);
				TEST_ASSERT(target->GetSelected() && rightResult->GetSelected());
			});
			protocol->OnNextIdleFrame(L"Both click recognizers disabled", [&, sequence]()
			{
				disableLeft->SetSelected(false);
				disableRight->SetSelected(false);
				sequence(0, 1, 0);
				sequence(500000, 0, 1);
				TEST_ASSERT(!target->GetSelected() && !rightResult->GetSelected());
			});
			protocol->OnNextIdleFrame(L"Fresh defaults activate both clicks again", []()
			{
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			suppressLeft ? L"Controls/PointScrolling/SuppressLeftClick" : L"Controls/PointScrolling/SuppressRightClick",
			WString::Unmanaged(L"gacuisrc_unittest::ClickWindow"), resource);
	});

	TEST_CASE(L"TouchingList XML uses identity-based deferred removal")
	{
		auto manager = GetGlobalTypeManager();
		collections::SortedList<WString> originalTypes;
		for (vint i = 0; i < manager->GetTypeDescriptorCount(); i++)
		{
			originalTypes.Add(manager->GetTypeDescriptor(i)->GetTypeName());
		}
		TEST_ASSERT(LoadDarkSkinTypes());
		const auto swipeResource = LR"GacUISrc(
<Resource>
  <Folder name="Sample" content="Link">../App/FullControlTest/TouchingListTabPage.xml</Folder>
  <Instance name="TouchingSample" content="File">../App/FullControlTest/TouchingTabPage.xml</Instance>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::SwipeWindow" xmlns:demo="demo::*">
      <Window ref.Name="self" Text="TouchingList swipes" ClientSize="x:560 y:400">
        <Tab>
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
          <att.Pages><demo:TouchingListTabPage ref.Name="sample"/></att.Pages>
        </Tab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";
		auto state = Ptr(new PointSwipeState);
		auto row = [=]() { return state->list->GetArranger()->GetVisibleStyle(1); };
		auto reveal = [=]() { return UnboxValue<vint>(BoxValue(row()).GetProperty(L"DragDistance")); };
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			state->input = Ptr(new PointProtocolInput(protocol->GetEvents(), false));
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				TEST_ASSERT(state->items->GetCount() == 10);
				TEST_ASSERT(!state->list->GetHorizontalScroll()->GetEnabled());
				AssertTouchingListNoRed(state->list->GetArranger()->GetVisibleStyle(1));
				state->target = state->items->Get(1);
				state->maximum = UnboxValue<vint>(BoxValue(row()).GetProperty(L"MaxDragDistance"));
				TEST_ASSERT(state->maximum > 32);
				auto text0 = UnboxValue<WString>(state->items->Get(0).GetProperty(L"Text"));
				auto text1 = UnboxValue<WString>(state->target.GetProperty(L"Text"));
				TEST_ASSERT(text0.Sub(9, text0.Length() - 9) != text1.Sub(9, text1.Length() - 9));
				auto foreground = FindObjectByName<GuiBoundsComposition>(row(), L"foreground");
				TEST_ASSERT(foreground->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor() == darkskin::Theme::GetColorPackage().ContentBackground);
				state->list->SetSelected(1, true);
			});
			protocol->OnNextIdleFrame(L"Selected row uses DarkSkin", [=]()
			{
				auto foreground = FindObjectByName<GuiBoundsComposition>(row(), L"foreground");
				TEST_ASSERT(foreground->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor() == darkskin::Theme::GetColorPackage().ItemBackgroundSelected);
				state->origin = protocol->LocationOf(state->list->GetArranger()->GetVisibleStyle(1), 0.7, 0.5);
				// A second contact clears the target; interpreted End must not read its properties.
				state->input->Start(state->origin);
				state->input->Down(1, NativePoint(state->origin.x + 20, state->origin.y));
				state->input->Up(1, NativePoint(state->origin.x + 20, state->origin.y));
				state->input->Up(0, state->origin); state->input->End();
				TEST_ASSERT(reveal() == 0 && state->items->GetCount() == 10);
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x - state->maximum / 2, state->origin.y));
			});
			protocol->OnNextIdleFrame(L"Partial red reveal", [=]()
			{
				TEST_ASSERT(reveal() == state->maximum / 2 && state->items->GetCount() == 10);
				auto background = FindObjectByName<GuiBoundsComposition>(row(), L"deleteBackground");
				TEST_ASSERT(background->GetVisible() && background->GetCachedBounds().Width() == state->maximum / 2);
				state->input->Up(0, NativePoint(state->origin.x - state->maximum / 2, state->origin.y)); state->input->End();
			});
			protocol->OnNextIdleFrame(L"Partial release reset", [=]()
			{
				TEST_ASSERT(reveal() == 0 && state->items->GetCount() == 10);
				AssertTouchingListNoRed(row());
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x - state->maximum - 100, state->origin.y));
			});
			protocol->OnNextIdleFrame(L"Full red reveal waits for End", [=]()
			{
				TEST_ASSERT(reveal() == state->maximum && state->items->GetCount() == 10);
				state->input->Up(0, NativePoint(state->origin.x - state->maximum - 100, state->origin.y)); state->input->End();
				TEST_ASSERT(state->items->GetCount() == 10);
			});
			protocol->OnNextIdleFrame(L"Deferred middle row removal", [=]()
			{
				TEST_ASSERT(state->items->GetCount() == 9 && !state->items->Contains(state->target));
				state->target = state->items->Get(1);
				state->origin = protocol->LocationOf(state->list->GetArranger()->GetVisibleStyle(1), 0.7, 0.5);
				state->input->Start(state->origin);
				state->input->Up(0, NativePoint(state->origin.x - state->maximum, state->origin.y)); state->input->End();
				// Clear and replace items before the pending removal runs; old indices cannot identify replacements.
				state->page.Invoke(L"ResetSwipe");
				state->items->Clear();
				state->page.Invoke(L"AddItems");
			});
			protocol->OnNextIdleFrame(L"Clear and Add before queued removal", [=]()
			{
				TEST_ASSERT(state->items->GetCount() == 5 && !state->items->Contains(state->target));
				AssertTouchingListNoRed(state->list->GetBoundsComposition());
				state->list->SetViewPosition(Point(0, 10000));
				AssertTouchingListNoRed(state->list->GetBoundsComposition());
			});
			protocol->OnNextIdleFrame(L"Newly realized rows at bottom stay closed", [=]()
			{
				AssertTouchingListNoRed(state->list->GetBoundsComposition());
				for (vint i = 0; i < 20; i++)
				{
					state->list->SetViewPosition(Point(0, i % 2 == 0 ? 0 : 10000));
					AssertTouchingListNoRed(state->list->GetBoundsComposition());
				}
				state->list->SetViewPosition(Point(0, 0));
			});
			protocol->OnNextIdleFrame(L"Recycled rows at top stay closed", [=]()
			{
				TEST_ASSERT(reveal() == 0);
				AssertTouchingListNoRed(state->list->GetBoundsComposition());
				state->list->SetEnabled(false);
			});
			protocol->OnNextIdleFrame(L"Disabled row uses DarkSkin", [=]()
			{
				auto foreground = FindObjectByName<GuiBoundsComposition>(row(), L"foreground");
				auto colors = darkskin::Theme::GetColorPackage();
				TEST_ASSERT(foreground->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor() == colors.ContentBackground);
				TEST_ASSERT(foreground->Children()[0]->GetOwnedElement().Cast<GuiSolidLabelElement>()->GetColor() == colors.TextDisabled);
				state->target = Value();
				state->page = Value();
				state->items = nullptr;
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		GacUIUnitTest_Installer installer;
		installer.installWindow = [=](GuiWindow* window)
		{
			auto page = FindObjectByName<GuiTabPage>(window, L"sample");
			state->page = BoxValue(page);
			state->list = FindObjectByName<GuiBindableTextList>(page, L"list");
			state->items = state->list->GetItemSource().Cast<IValueList>();
			TEST_ASSERT(state->items);
			state->page.Invoke(L"AddItems");
		};
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			L"Controls/PointScrolling/TouchingList", L"gacuisrc_unittest::SwipeWindow", swipeResource, installer);
		// Leave the core-only reflection inventory unchanged for subsequent tests.
		for (vint i = manager->GetTypeDescriptorCount() - 1; i >= 0; i--)
		{
			auto name = manager->GetTypeDescriptor(i)->GetTypeName();
			if (!originalTypes.Contains(name))
			{
				TEST_ASSERT(manager->SetTypeDescriptor(name, nullptr));
			}
		}
	});

	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="WideItemResource">
    <Instance ref.Class="gacuisrc_unittest::WideItem">
      <TextListItemTemplate ref.Name="self" PreferredMinSize="x:650 y:28" MinSizeLimitation="LimitToElementAndChildren">
        <SolidLabel Text-bind="self.Text" Color-bind="self.TextColor" Font-bind="self.Font" VerticalAlignment="Center"/>
      </TextListItemTemplate>
    </Instance>
  </Instance>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Point scrolling" ClientSize="x:600 y:280">
        <Table AlignmentToParent="left:5 top:5 right:5 bottom:5" CellPadding="5">
          <att.Rows><_>composeType:Percentage percentage:1.0</_></att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <TextList ref.Name="list">
              <att.ItemTemplate>gacuisrc_unittest::WideItem</att.ItemTemplate>
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </TextList>
          </Cell>
          <Cell Site="row:0 column:1">
            <ScrollContainer ref.Name="outer" ExtendToFullWidth="true" ExtendToFullHeight="true" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <MultilineTextBox ref.Name="editor">
                <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              </MultilineTextBox>
            </ScrollContainer>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	for (vint scenario = 0; scenario < 4; scenario++)
	TEST_CASE(L"Momentum preserves selected content after Up, mode " + itow(scenario))
	{
		bool document = scenario != 0;
		auto mode = scenario <= 1 ? GuiDocumentEditMode::ViewOnly : scenario == 2 ? GuiDocumentEditMode::Selectable : GuiDocumentEditMode::Editable;
		Ptr<TooltipTimer> clock;
		Ptr<PointProtocolInput> input;
		GuiScrollView* view = nullptr;
		MomentumScrolling* momentum = nullptr;
		NativePoint origin;
		TextPos begin, end;
		auto selected = [&]()
		{
			auto window = GetApplication()->GetMainWindow();
			TEST_ASSERT(FindObjectByName<GuiTextList>(window, L"list")->GetSelectedItemIndex() == 0);
			auto editor = FindObjectByName<GuiMultilineTextBox>(window, L"editor");
			TEST_ASSERT(editor->GetCaretBegin() == begin && editor->GetCaretEnd() == end);
		};
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			input = Ptr(new PointProtocolInput(protocol->GetEvents(), false));
			protocol->OnNextIdleFrame(L"Selected content ready", [&, protocol]()
			{
				view->SetViewPosition(Point());
				origin = protocol->LocationOf(view, 0.5, 0.9);
				input->Start(origin);
				TEST_ASSERT(view->GetPointEventComposition() == view->GetBoundsComposition());
				input->At(500000);
				input->Up(0, NativePoint(origin.x, origin.y - 160));
				input->End(500000);
				TEST_ASSERT(!view->GetPointEventComposition());
				TEST_ASSERT(view->GetViewPosition().y == 160);
			});
			protocol->OnNextIdleFrame(L"Final Up scrolled and released capture", [&]()
			{
				selected();
				clock->Advance(100);
				GetApplication()->GetMainWindow()->GetTimerManager()->Play();
				TEST_ASSERT(view->GetViewPosition().y == 190);
			});
			protocol->OnNextIdleFrame(L"Momentum scrolled with capture cleared", [&]()
			{
				selected();
				clock->Advance(900);
				GetApplication()->GetMainWindow()->GetTimerManager()->Play();
				TEST_ASSERT(view->GetViewPosition().y == 320);
			});
			protocol->OnNextIdleFrame(L"Momentum stopped at one second", [&]()
			{
				selected();
				input->Start(origin);
				input->At(500000); input->Up(0, NativePoint(origin.x, origin.y - 160)); input->End(500000);
				momentum->DetachControl(view);
				clock->Advance(1000);
				GetApplication()->GetMainWindow()->GetTimerManager()->Play();
				TEST_ASSERT(view->GetViewPosition().y == 480);
			});
			protocol->OnNextIdleFrame(L"Detachment stopped continuation", [&]()
			{
				selected();
				GetApplication()->GetMainWindow()->Hide();
				clock = nullptr;
			});
		});
		GacUIUnitTest_Installer installer;
		installer.installWindow = [&](GuiWindow* window)
		{
			clock = Ptr(new TooltipTimer);
			momentum = new MomentumScrolling;
			window->AddComponent(momentum);
			auto list = FindObjectByName<GuiTextList>(window, L"list");
			auto editor = FindObjectByName<GuiMultilineTextBox>(window, L"editor");
			list->SetArranger(Ptr(new PointScrollingItemArranger));
			WString text;
			for (vint i = 0; i < 80; i++)
			{
				auto line = L"Selected content " + itow(i) + L" remains selected while momentum scrolls the viewport.";
				list->GetItems().Add(Ptr(new TextItem(line)));
				text += line + L"\r\n";
			}
			list->SetSelected(0, true);
			editor->SetText(text);
			editor->SetCaret(TextPos(0, 0), TextPos(0, 8));
			editor->SetViewPosition(Point());
			begin = editor->GetCaretBegin(); end = editor->GetCaretEnd();
			editor->SetEditMode(mode);
			view = document ? static_cast<GuiScrollView*>(editor) : list;
			momentum->AttachControl(view);
		};
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			document ? L"Controls/PointScrolling/DocumentMomentum" + WString(scenario == 1 ? L"" : scenario == 2 ? L"Selectable" : L"Editable") : L"Controls/PointScrolling/ListMomentum",
			L"gacuisrc_unittest::MainWindow", resource, installer);
	});

	for (bool editor : { false,true })
	TEST_CASE(editor ? L"Multiline text point selection" : L"List point scrolling")
	{
		auto state = Ptr(new PointScrollingState);
		auto findView = [=]() -> GuiScrollView*
		{
			auto window = GetApplication()->GetMainWindow();
			if (editor) return FindObjectByName<GuiMultilineTextBox>(window, L"editor");
			return FindObjectByName<GuiTextList>(window, L"list");
		};
		auto assertSelection = [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			TEST_ASSERT(FindObjectByName<GuiTextList>(window, L"list")->GetSelectedItemIndex() == 0);
			auto textBox = FindObjectByName<GuiMultilineTextBox>(window, L"editor");
			TEST_ASSERT(textBox->GetCaretBegin() == state->caretBegin && textBox->GetCaretEnd() == state->caretEnd);
		};
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			state->input = Ptr(new PointProtocolInput(protocol->GetEvents(), false));
			if (editor)
			{
				protocol->OnNextIdleFrame(L"Ready for press selection", [=]()
				{
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					state->origin = protocol->LocationOf(textBox, 0.3, 0.3);
					state->input->Start(state->origin);
					TEST_ASSERT(textBox->GetPointEventComposition() == textBox->GetBoundsComposition());
					state->input->Up(0, state->origin); state->input->End(100000);
					TEST_ASSERT(textBox->GetCaretBegin() == textBox->GetCaretEnd());
				});
				protocol->OnNextIdleFrame(L"Short press moved the caret", [=]()
				{
					state->origin = protocol->LocationOf(findView(), 0.6, 0.4);
					state->input->Start(state->origin);
					state->input->At(500000); state->input->Up(0, state->origin); state->input->End(500000);
				});
				protocol->OnNextIdleFrame(L"Long press extended the selection", [=]()
				{
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					TEST_ASSERT(textBox->GetCaretBegin() != textBox->GetCaretEnd());
					state->caretBegin = textBox->GetCaretBegin(); state->caretEnd = textBox->GetCaretEnd();
					state->input->Start(state->origin); state->input->Up(0, state->origin); state->input->End(100000);
					assertSelection();
					textBox->SetControlTemplate([](const description::Value&) { return new darkskin::DocumentViewerTemplate; });
					textBox->SetEditMode(GuiDocumentEditMode::Selectable);
				});
				protocol->OnNextIdleFrame(L"Template refresh preserved press selection", [=]()
				{
					assertSelection();
					state->input->Start(state->origin);
					TEST_ASSERT(findView()->GetPointEventComposition() == findView()->GetBoundsComposition());
					state->input->At(500000); state->input->Up(0, state->origin); state->input->End(500000);
					TEST_ASSERT(state->rightClicks == 1);
					assertSelection();
					state->input->Start(state->origin);
					state->input->Move(NativePoint(state->origin.x, state->origin.y - 40));
					state->input->Up(0, NativePoint(state->origin.x, state->origin.y - 40)); state->input->End();
				});
				protocol->OnNextIdleFrame(L"Selectable content scrolled without changing the range", [=]()
				{
					TEST_ASSERT(findView()->GetViewPosition().y == 40);
					assertSelection();
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);
					state->input->Start(state->origin);
					state->input->Move(NativePoint(state->origin.x, state->origin.y - 20));
					state->input->Up(0, NativePoint(state->origin.x, state->origin.y - 20)); state->input->End();
				});
				protocol->OnNextIdleFrame(L"View-only content still scrolls", [=]()
				{
					TEST_ASSERT(findView()->GetViewPosition().y == 60);
					assertSelection();
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					state->origin = protocol->LocationOf(textBox->GetVerticalScroll(), 0.5, 0.5);
					state->input->Start(state->origin);
					state->input->End(0, PointSequenceEndReason::Canceled_PlatformCanceled);
					GetApplication()->GetMainWindow()->Hide();
				});
				return;
			}
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto view = findView();
				state->origin = protocol->LocationOf(view, 0.5, 0.5);
				TEST_ASSERT(view->GetHorizontalScroll()->GetEnabled() && view->GetVerticalScroll()->GetEnabled());
				TEST_ASSERT(view->GetViewPosition() == Point());
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x, state->origin.y - 60));
			});
			protocol->OnNextIdleFrame(L"Dragged down", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point(0, 60));
				assertSelection();
				state->input->Move(NativePoint(state->origin.x, state->origin.y - 20));
			});
			protocol->OnNextIdleFrame(L"Reversed vertical drag", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point(0, 20));
				assertSelection();
				state->input->Move(NativePoint(state->origin.x, state->origin.y - 2000));
			});
			protocol->OnNextIdleFrame(L"Clamped at bottom", [=]()
			{
				auto bar = findView()->GetVerticalScroll();
				TEST_ASSERT(bar->GetPosition() == bar->GetTotalSize() - bar->GetPageSize());
				state->input->Up(0, NativePoint(state->origin.x, state->origin.y - 2000)); state->input->End();
				assertSelection();
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x, state->origin.y + 2000));
			});
			protocol->OnNextIdleFrame(L"Clamped at top", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point());
				state->input->Up(0, NativePoint(state->origin.x, state->origin.y + 2000)); state->input->End();
				assertSelection();
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x - 60, state->origin.y));
			});
			protocol->OnNextIdleFrame(L"Dragged right", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point(60, 0));
				assertSelection();
				state->input->Move(NativePoint(state->origin.x - 20, state->origin.y));
			});
			protocol->OnNextIdleFrame(L"Reversed horizontal drag", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point(20, 0));
				state->input->Up(0, NativePoint(state->origin.x - 20, state->origin.y)); state->input->End();
				assertSelection();
				state->input->Start(state->origin);
				state->input->Move(NativePoint(state->origin.x + 2000, state->origin.y));
			});
			protocol->OnNextIdleFrame(L"Clamped at left", [=]()
			{
				TEST_ASSERT(findView()->GetViewPosition() == Point());
				state->input->Up(0, NativePoint(state->origin.x + 2000, state->origin.y)); state->input->End();
				assertSelection();
				state->input->Start(state->origin);
				state->input->Up(0, state->origin); state->input->End(100000);
			});
			protocol->OnNextIdleFrame(L"Tap selected content", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				if (editor)
				{
					auto textBox = FindObjectByName<GuiMultilineTextBox>(window, L"editor");
					TEST_ASSERT(textBox->GetCaretEnd() != state->caretEnd);
				}
				else TEST_ASSERT(FindObjectByName<GuiTextList>(window, L"list")->GetSelectedItemIndex() > 0);
				TEST_ASSERT(!findView()->GetPointEventComposition());
				state->input->Start(state->origin);
				state->input->Up(0, state->origin); state->input->End(500000);
				TEST_ASSERT(state->rightClicks == 1);
				window->Hide();
			});
		});
		GacUIUnitTest_Installer installer;
		installer.installWindow = [=](GuiWindow* window)
		{
			auto list = FindObjectByName<GuiTextList>(window, L"list");
			auto textBox = FindObjectByName<GuiMultilineTextBox>(window, L"editor");
			auto momentum = new MomentumScrolling;
			window->AddComponent(momentum);
			momentum->AttachControl(textBox);
			list->SetArranger(Ptr(new PointScrollingItemArranger));
			WString text;
			for (vint i = 0; i < 40; i++)
			{
				auto line = L"Row " + itow(i) + L": long overflowing content for point scrolling and selection preservation.";
				list->GetItems().Add(Ptr(new TextItem(line)));
				text += line + L"\r\n";
			}
			list->SetSelected(0, true);
			textBox->SetText(text);
			textBox->SetCaret(TextPos(0, 0), TextPos(0, 4));
			textBox->GetHorizontalScroll()->SetPosition(0);
			textBox->GetVerticalScroll()->SetPosition(0);
			state->caretBegin = textBox->GetCaretBegin();
			state->caretEnd = textBox->GetCaretEnd();
			GuiControl* target = editor ? static_cast<GuiControl*>(textBox) : static_cast<GuiControl*>(list);
			target->GetBoundsComposition()->GetEventReceiver()->mouseUp.AttachLambda([=](GuiGraphicsComposition*, GuiMouseEventArgs& a)
			{
				if (a.button == NativeMouseButton::Right) state->rightClicks++;
			});
		};
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			editor ? L"Controls/PointScrolling/MultilineTextBox" : L"Controls/PointScrolling/TextList",
			L"gacuisrc_unittest::MainWindow", resource, installer);
	});
}
