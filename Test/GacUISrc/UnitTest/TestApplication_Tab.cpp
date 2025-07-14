#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTabNavigate = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="TabNavigate" ClientSize="x:480 y:320">
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Application")
	{
		TEST_CASE(L"TabNavigate")
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
				WString::Unmanaged(L"Application/TabNavigate"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigate
			);
		});
	});
}