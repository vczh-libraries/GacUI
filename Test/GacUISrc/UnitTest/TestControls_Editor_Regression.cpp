#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resource_TwoTextBoxes = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Regression Test" ClientSize="x:480 y:320">
        <Table CellPadding="5" BorderVisible="true" AlignmentToParent="left:5 top:5 right:5 bottom:5">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <SinglelineTextBox ref.Name="textBox1" Text="First text box">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </SinglelineTextBox>
          </Cell>
          <Cell Site="row:1 column:0">
            <SinglelineTextBox ref.Name="textBox2" Text="Second text box">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </SinglelineTextBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_ScrollableTextBoxAndDocumentBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Regression Test" ClientSize="x:480 y:360">
        <Table CellPadding="5" BorderVisible="true" AlignmentToParent="left:5 top:5 right:5 bottom:5">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <MultilineTextBox ref.Name="textBox" HorizontalAlwaysVisible="true" VerticalAlwaysVisible="true" Text="The multiline text box keeps editing cursor only inside the document area.&#xD;&#xA;The scroll bars and the scroll corner should use the default cursor.&#xD;&#xA;Extra text keeps the scrollbar layout realistic.">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </MultilineTextBox>
          </Cell>
          <Cell Site="row:1 column:0">
            <DocumentViewer ref.Name="documentBox" HorizontalAlwaysVisible="true" VerticalAlwaysVisible="true" Text="The document box is a scope guard for the shared document viewer template.">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </DocumentViewer>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Regressions")
	{
		// Regression: changing font size on text box 1 should NOT affect text box 2.
		// Text box 2 should keep its original font size after typing.
		// If fonts are unexpectedly shared, typing in text box 2 will pick up text box 1's enlarged font.
		TEST_CASE(L"UnexpectedlySharedFonts")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox1 = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox1");
					// Increase font size on text box 1 only
					auto font = textBox1->GetDisplayFont();
					font.size += 5;
					textBox1->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"FontIncreased", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox2 = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox2");
					// Focus text box 2 and type something
					textBox2->SetFocused();
					protocol->TypeString(L"!");
				});
				protocol->OnNextIdleFrame(L"Typed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Regressions/UnexpectedlySharedFonts"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_TwoTextBoxes
			);
		});

		TEST_CASE(L"ScrollableTextBoxCursor")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiMultilineTextBox>(window, L"textBox");
					auto documentBox = FindObjectByName<GuiDocumentViewer>(window, L"documentBox");
					auto resourceService = GetCurrentController()->ResourceService();
					auto ibeam = resourceService->GetSystemCursor(INativeCursor::IBeam);

					auto assertCursor = [](INativeCursor* cursor, INativeCursor* expected)
					{
						TEST_ASSERT(cursor == expected);
					};
					auto assertNotCursor = [](INativeCursor* cursor, INativeCursor* unexpected)
					{
						TEST_ASSERT(cursor != unexpected);
					};
					auto getScrollCornerCursor = [](GuiControl* control, GuiScroll* hScroll, GuiScroll* vScroll)
					{
						auto bounds = control->GetBoundsComposition();
						auto hBounds = hScroll->GetBoundsComposition()->GetGlobalBounds();
						auto vBounds = vScroll->GetBoundsComposition()->GetGlobalBounds();
						auto rootBounds = bounds->GetGlobalBounds();
						auto globalPoint = Point((vBounds.x1 + vBounds.x2) / 2, (hBounds.y1 + hBounds.y2) / 2);
						auto localPoint = Point(globalPoint.x - rootBounds.x1, globalPoint.y - rootBounds.y1);
						auto hit = bounds->FindVisibleComposition(localPoint, true);
						TEST_ASSERT(hit != nullptr);
						return hit->GetRelatedCursor();
					};

					TEST_ASSERT(textBox->GetHorizontalScroll()->GetVisible());
					TEST_ASSERT(textBox->GetVerticalScroll()->GetVisible());
					TEST_ASSERT(textBox->GetBoundsComposition()->GetAssociatedCursor() == nullptr);
					assertCursor(textBox->GetContainerComposition()->GetParent()->GetRelatedCursor(), ibeam);
					assertCursor(textBox->GetHorizontalScroll()->GetBoundsComposition()->GetRelatedCursor(), nullptr);
					assertCursor(textBox->GetVerticalScroll()->GetBoundsComposition()->GetRelatedCursor(), nullptr);
					assertCursor(getScrollCornerCursor(textBox, textBox->GetHorizontalScroll(), textBox->GetVerticalScroll()), nullptr);

					TEST_ASSERT(documentBox->GetHorizontalScroll()->GetVisible());
					TEST_ASSERT(documentBox->GetVerticalScroll()->GetVisible());
					assertNotCursor(documentBox->GetContainerComposition()->GetParent()->GetRelatedCursor(), ibeam);
					assertNotCursor(documentBox->GetHorizontalScroll()->GetBoundsComposition()->GetRelatedCursor(), ibeam);
					assertNotCursor(documentBox->GetVerticalScroll()->GetBoundsComposition()->GetRelatedCursor(), ibeam);

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Regressions/ScrollableTextBoxCursor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_ScrollableTextBoxAndDocumentBox
			);
		});
	});
}
