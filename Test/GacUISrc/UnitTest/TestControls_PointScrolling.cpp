#include "TestControls.h"
#include "TestPointInput.h"

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

TEST_FILE
{
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
          <StackItem><CheckBox ref.Name="target" Text="Tap activation"/></StackItem>
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
				protocol->OnNextIdleFrame(L"Ready for text selection", [=]()
				{
					auto view = findView();
					state->origin = protocol->LocationOf(view, 0.3, 0.3);
					state->input->Start(state->origin);
					TEST_ASSERT(!view->GetPointEventComposition());
					TEST_ASSERT(!FindObjectByName<GuiScrollContainer>(GetApplication()->GetMainWindow(), L"outer")->GetPointEventComposition());
					state->input->Move(NativePoint(state->origin.x + 60, state->origin.y + 30));
				});
				protocol->OnNextIdleFrame(L"Touch drag selects text", [=]()
				{
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					TEST_ASSERT(textBox->GetCaretBegin() != textBox->GetCaretEnd());
					TEST_ASSERT(textBox->GetViewPosition() == Point());
					state->input->Up(0, NativePoint(state->origin.x + 60, state->origin.y + 30)); state->input->End();
					textBox->SetControlTemplate([](const description::Value&) { return new darkskin::DocumentViewerTemplate; });
				});
				protocol->OnNextIdleFrame(L"Replaced template keeps text input", [=]()
				{
					state->origin = protocol->LocationOf(findView(), 0.4, 0.4);
					state->input->Start(state->origin);
					TEST_ASSERT(!findView()->GetPointEventComposition());
					state->input->Move(NativePoint(state->origin.x + 50, state->origin.y));
				});
				protocol->OnNextIdleFrame(L"Selection after template replacement", [=]()
				{
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					TEST_ASSERT(textBox->GetCaretBegin() != textBox->GetCaretEnd());
					state->input->Up(0, NativePoint(state->origin.x + 50, state->origin.y)); state->input->End();
					textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);
					state->input->Start(state->origin);
					TEST_ASSERT(findView()->GetPointEventComposition() == findView()->GetBoundsComposition());
					state->input->Move(NativePoint(state->origin.x, state->origin.y - 40));
				});
				protocol->OnNextIdleFrame(L"View-only text still scrolls", [=]()
				{
					TEST_ASSERT(findView()->GetViewPosition().y == 40);
					state->input->Up(0, NativePoint(state->origin.x, state->origin.y - 40)); state->input->End();
					auto textBox = FindObjectByName<GuiMultilineTextBox>(GetApplication()->GetMainWindow(), L"editor");
					textBox->SetEditMode(GuiDocumentEditMode::Editable);
					auto begin = textBox->GetCaretBegin(), end = textBox->GetCaretEnd();
					auto receiver = textBox->GetBoundsComposition()->GetEventReceiver();
					auto handler = receiver->pointSequenceStarted.AttachLambda([](GuiGraphicsComposition* sender, GuiPointSequenceStartedEventArgs& a) { a.acceptedComposition = sender; });
					state->input->Start(state->origin);
					TEST_ASSERT(textBox->GetPointEventComposition() == textBox->GetBoundsComposition());
					state->input->Move(NativePoint(state->origin.x, state->origin.y - 20));
					state->input->Up(0, NativePoint(state->origin.x, state->origin.y - 20)); state->input->End();
					TEST_ASSERT(textBox->GetCaretBegin() == begin && textBox->GetCaretEnd() == end);
					receiver->pointSequenceStarted.Detach(handler);
					state->origin = protocol->LocationOf(textBox->GetVerticalScroll(), 0.5, 0.5);
					state->input->Start(state->origin);
					TEST_ASSERT(textBox->GetPointEventComposition() == textBox->GetBoundsComposition());
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
