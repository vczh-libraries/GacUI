#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceDateComboBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDatePicker" ClientSize="x:320 y:240">
        <att.ContainerComposition-set InternalMargin="left:10 top:10 right:10 bottom:10"/>
        <DateComboBox ref.Name="dateCombo" SelectedDate="2000-01-01 00:00:00.000">
          <att.BoundsComposition-set PreferredMinSize="x:160 y:20"/>
          <att.DatePicker-set DateLocale="en-US"/>
        </DateComboBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiDateComboBox")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(2000, 1, 1));
					dateCombo->SetSelectedDate(DateTime::FromDateTime(1999, 12, 31));
					dateCombo->SetSubMenuOpening(true);
				});
				protocol->OnNextIdleFrame(L"1999-12-31", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					dateCombo->SetSubMenuOpening(false);
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(1999, 12, 31));
					dateCombo->SetSelectedDate(DateTime::FromDateTime(2000, 1, 31));
					dateCombo->SetSubMenuOpening(true);
				});
				protocol->OnNextIdleFrame(L"2000-1-31", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					dateCombo->SetSubMenuOpening(false);
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(2000, 1, 31));
					dateCombo->SetSelectedDate(DateTime::FromDateTime(2000, 2, 1));
					dateCombo->SetSubMenuOpening(true);
				});
				protocol->OnNextIdleFrame(L"2000-2-1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					dateCombo->SetSubMenuOpening(false);
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(2000, 2, 1));
					dateCombo->SetSelectedDate(DateTime::FromDateTime(2000, 1, 1));
					dateCombo->SetSubMenuOpening(true);
				});
				protocol->OnNextIdleFrame(L"2000-1-1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					dateCombo->SetSubMenuOpening(false);
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/GuiDateComboBox/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDateComboBox
				);
		});

		TEST_CASE(L"Mouse")
		{
			// only works with DarkSkin or any template object with:
			//   GuiCommonDatePickerLook: look

			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto location = protocol->LocationOf(dateCombo);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Combo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto datePicker = dateCombo->GetDatePicker();
					auto look = FindObjectByName<templates::GuiCommonDatePickerLook>(datePicker->TypedControlTemplateObject(false), L"look");
					auto location = protocol->LocationOf(look->GetYearCombo());
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Year", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto datePicker = dateCombo->GetDatePicker();
					auto look = FindObjectByName<templates::GuiCommonDatePickerLook>(datePicker->TypedControlTemplateObject(false), L"look");
					look->GetYearCombo()->SetSelectedIndex(look->GetYearCombo()->GetSelectedIndex() - 1);
					TEST_ASSERT(datePicker->GetDate() == DateTime::FromDateTime(1999, 1, 1));
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(2000, 1, 1));
				});
				protocol->OnNextIdleFrame(L"Set 1999", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto datePicker = dateCombo->GetDatePicker();
					auto look = FindObjectByName<templates::GuiCommonDatePickerLook>(datePicker->TypedControlTemplateObject(false), L"look");
					auto location = protocol->LocationOf(look->GetMonthCombo());
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Month", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto datePicker = dateCombo->GetDatePicker();
					auto look = FindObjectByName<templates::GuiCommonDatePickerLook>(datePicker->TypedControlTemplateObject(false), L"look");
					look->GetMonthCombo()->SetSelectedIndex(11);
					TEST_ASSERT(datePicker->GetDate() == DateTime::FromDateTime(1999, 12, 1));
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(2000, 1, 1));
				});
				protocol->OnNextIdleFrame(L"Set December", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dateCombo = FindObjectByName<GuiDateComboBox>(window, L"dateCombo");
					auto datePicker = dateCombo->GetDatePicker();
					auto look = FindObjectByName<templates::GuiCommonDatePickerLook>(datePicker->TypedControlTemplateObject(false), L"look");
					TEST_ASSERT(look->GetDateOfDayButton(4, 5) == DateTime::FromDateTime(1999, 12, 31));
					auto location = protocol->LocationOf(look->GetDayButton(4, 5));
					protocol->LClick(location);
					TEST_ASSERT(datePicker->GetDate() == DateTime::FromDateTime(1999, 12, 31));
					TEST_ASSERT(dateCombo->GetSelectedDate() == DateTime::FromDateTime(1999, 12, 31));
				});
				protocol->OnNextIdleFrame(L"Click 31 -> 1999-12-31", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/GuiDateComboBox/Mouse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDateComboBox
				);
		});
	});
}