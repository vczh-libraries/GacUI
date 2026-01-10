#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTooltipWindows = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Tooltip" ClientSize="x:480 y:320">
        <Table CellPadding="5" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <Button ref.Name="buttonTooltipA" Text="TooltipA">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip A"/>
              </att.TooltipControl>
            </Button>
          </Cell>
          <Cell Site="row:1 column:0">
            <Button ref.Name="buttonTooltipB" Text="TooltipB">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip B"/>
              </att.TooltipControl>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Tooltip")
	{
		TEST_CASE(L"ShowTooltipAndWaitForClosing")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndWaitForClosing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndLeave")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					protocol->MouseMove({0, 0});
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After leaving tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndLeave"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndSwitchToAnother")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipB = FindObjectByName<GuiButton>(window, L"buttonTooltipB");
					auto location = protocol->LocationOf(buttonTooltipB);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipB", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip for A", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndSwitchToAnother"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
	});
}
