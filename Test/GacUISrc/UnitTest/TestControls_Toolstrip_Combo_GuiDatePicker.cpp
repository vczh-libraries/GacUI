#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceDatePicker = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDatePicker" ClientSize="x:320 y:240">
        <att.ContainerComposition-set InternalMargin="left:10 top:10 right:10 bottom:10"/>
        <DatePicker ref.Name="datePicker" Alt="D" DateLocale="en-US" Date="2000-01-01 00:00:00.000"/>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiDatePicker")
	{
		TEST_CASE(L"Properties")
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
				WString::Unmanaged(L"Controls/Toolstrip/Combo/GuiDatePicker/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDatePicker
				);
		});

		TEST_CASE(L"Mouse")
		{
			// only works with DarkSkin or any template object with:
			//   GuiCommonDatePickerLook: look

		});
	});
}