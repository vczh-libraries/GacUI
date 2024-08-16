#include "TestControls.h"

TEST_FILE
{
	const auto resourceProgressBar = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiScroll" ClientSize="x:320 y:240">
        <ProgressBar ref.Name="progress" TotalSize="5">
          <att.BoundsComposition-set PreferredMinSize="y:20" AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </ProgressBar>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"ProgressBar")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto progress = FindObjectByName<GuiScroll>(window, L"progress");
				TEST_ASSERT(progress->GetTotalSize() == 5);
				TEST_ASSERT(progress->GetPageSize() == 0);
				TEST_ASSERT(progress->GetSmallMove() == 1);
				TEST_ASSERT(progress->GetBigMove() == 10);
				TEST_ASSERT(progress->GetPosition() == 0);
				TEST_ASSERT(progress->GetMinPosition() == 0);
				TEST_ASSERT(progress->GetMaxPosition() == 5);
				TEST_ASSERT(progress->GetAutoFocus() == false);
				progress->SetFocused();
				TEST_ASSERT(!progress->GetFocused());
				progress->SetPosition(1);
				TEST_ASSERT(progress->GetPosition() == 1);
			});
			protocol->OnNextIdleFrame(L"1", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto progress = FindObjectByName<GuiScroll>(window, L"progress");
				progress->SetPosition(2);
				TEST_ASSERT(progress->GetPosition() == 2);
			});
			protocol->OnNextIdleFrame(L"2", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto progress = FindObjectByName<GuiScroll>(window, L"progress");
				progress->SetPosition(3);
				TEST_ASSERT(progress->GetPosition() == 3);
			});
			protocol->OnNextIdleFrame(L"3", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto progress = FindObjectByName<GuiScroll>(window, L"progress");
				progress->SetPosition(4);
				TEST_ASSERT(progress->GetPosition() == 4);
			});
			protocol->OnNextIdleFrame(L"4", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto progress = FindObjectByName<GuiScroll>(window, L"progress");
				progress->SetPosition(5);
				TEST_ASSERT(progress->GetPosition() == 5);
			});
			protocol->OnNextIdleFrame(L"5", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Basic/GuiScroll/ProgressBar"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceProgressBar
			);
	});
}