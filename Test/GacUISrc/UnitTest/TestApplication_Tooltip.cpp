#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTooltipWindows = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Tooltip" ClientSize="x:480 y:320">
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Tooltip")
	{
		TEST_CASE(L"ShowTooltipAndWaitForClosing")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
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
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
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
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
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
