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
		for (auto closeOwner : { false,true })
		TEST_CASE(closeOwner ? L"CloseTooltipWindowOwner" : L"HideTooltipWindowOwner")
		{
			GuiWindow* ownerWindow = nullptr;
			GuiTooltip* tooltipWindow = nullptr;
			vint closed = 0;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&]()
				{
					ownerWindow = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::MainWindow"));
					ownerWindow->SetText(L"Tooltip owner");
					ownerWindow->SetClientSize({ {240},{160} });
					ownerWindow->SetLocation({ {100},{80} });
					ownerWindow->Show();
				});
				protocol->OnNextIdleFrame(L"Secondary owner visible", [&]()
				{
					auto owner = FindObjectByName<GuiButton>(ownerWindow, L"buttonTooltipA");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					tooltipWindow = dynamic_cast<GuiTooltip*>(owner->GetTooltipControl()->GetRelatedControlHost());
					TEST_ASSERT(tooltipWindow);
					tooltipWindow->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(closed++ == 0);
					});
					ownerWindow->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(closed == 1);
						TEST_ASSERT(!tooltipWindow->GetOpening());
						TEST_ASSERT(!tooltipWindow->GetTemporaryContentControl());
						TEST_ASSERT(!GetApplication()->GetTooltipOwner());
						closed++;
					});
				});
				protocol->OnNextIdleFrame(L"Tooltip visible over secondary owner", [&, protocol]()
				{
					protocol->MouseMove(protocol->LocationOf(tooltipWindow->GetTemporaryContentControl()));
					if (closeOwner) ownerWindow->Close();
					else ownerWindow->Hide();
				});
				protocol->OnNextIdleFrame(L"Main window survives owner and tooltip", [&]()
				{
					TEST_ASSERT(closed == 2);
					SafeDeleteControl(ownerWindow);
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				closeOwner ? L"Application/Tooltip/CloseWindowOwner" : L"Application/Tooltip/HideWindowOwner",
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});

		TEST_CASE(L"DestroyVisibleTooltipOwner")
		{
			GuiTooltip* tooltipWindow = nullptr;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					tooltipWindow = dynamic_cast<GuiTooltip*>(owner->GetTooltipControl()->GetRelatedControlHost());
					TEST_ASSERT(tooltipWindow);
				});
				protocol->OnNextIdleFrame(L"Tooltip before owner deletion", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					protocol->MouseMove(protocol->LocationOf(owner->GetTooltipControl()));
					SafeDeleteControl(owner);
					TEST_ASSERT(tooltipWindow->GetTemporaryContentControl() == nullptr);
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == nullptr);
					window->SetText(L"Tooltip owner removed");
					GetApplication()->RefreshThemes();
				});
				protocol->OnNextIdleFrame(L"Tooltip closed after owner deletion", [&, protocol]()
				{
					TEST_ASSERT(!tooltipWindow->GetOpening());
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipB");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == owner);
				});
				protocol->OnNextIdleFrame(L"Surviving owner reuses tooltip", [&, protocol]()
				{
					TEST_ASSERT(tooltipWindow->GetOpening());
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/DestroyVisibleTooltipOwner"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});

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
