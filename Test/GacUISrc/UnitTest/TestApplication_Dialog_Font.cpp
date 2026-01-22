#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceOpenAndClose = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog Font Test" ClientSize="x:480 y:320">
        <FontDialog ref.Name="fontDialog" ShowEffect="true"/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0" PreferredMinSize="y:20">
            <Label ref.Name="labelFont" Text="Sample Text"/>
          </Cell>

          <Cell Site="row:1 column:0">
            <Button Text="Change Font Simple">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                fontDialog.ShowEffect = false;
                fontDialog.SelectedFont = labelFont.DisplayFont;
                if (fontDialog.ShowDialog())
                {
                  labelFont.Font = fontDialog.SelectedFont;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:2 column:0">
            <Button Text="Change Font">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                fontDialog.ShowEffect = true;
                fontDialog.SelectedFont = labelFont.DisplayFont;
                if (fontDialog.ShowDialog())
                {
                  labelFont.Font = fontDialog.SelectedFont;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Dialog Font")
	{
		TEST_CASE(L"Open and Close (Simple)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Font/OpenAndClose_Simple"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceOpenAndClose
			);
		});

		TEST_CASE(L"Open and Close")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Font/OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceOpenAndClose
			);
		});
	});
}