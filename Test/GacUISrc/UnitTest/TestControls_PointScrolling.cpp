#include "TestControls.h"
#include "TestPointInput.h"

using namespace gacui_unittest_template;

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

TEST_FILE
{
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
            <MultilineTextBox ref.Name="editor">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </MultilineTextBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	for (bool editor : { false,true })
	TEST_CASE(editor ? L"Multiline text point scrolling" : L"List point scrolling")
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
