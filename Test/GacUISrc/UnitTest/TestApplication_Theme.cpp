#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceControlThemeName = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="ControlThemeName" ClientSize="x:480 y:320">
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren" CellPadding="5" BorderVisible="true">
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
            <CheckBox ref.Name="checkbox1" Text="Selectable Button" Selected="true">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </CheckBox>
          </Cell>
          
          <Cell Site="row:1 column:0">
            <CheckBox ref.Name="checkbox2" Text="Selectable Button" Selected="false">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </CheckBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Theme")
	{
		TEST_CASE(L"ControlThemeName")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());

					checkbox1->SetControlThemeName(theme::ThemeName::RadioButton);
					checkbox2->SetControlThemeName(theme::ThemeName::RadioButton);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::RadioButton);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::RadioButton);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Become RadioButton", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					checkbox1->SetControlThemeName(theme::ThemeName::CheckBox);
					checkbox2->SetControlThemeName(theme::ThemeName::CheckBox);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Back to CheckBox", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/ControlThemeName"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceControlThemeName
			);
		});

		TEST_CASE(L"ControlTemplate")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto checkboxTheme = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::CheckBox);
				auto radiobuttonTheme = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::RadioButton);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());

					checkbox1->SetControlTemplate(radiobuttonTheme);
					checkbox2->SetControlTemplate(radiobuttonTheme);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox1->GetControlTemplate());
					TEST_ASSERT(checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Become RadioButton", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					checkbox1->SetControlTemplate({});
					checkbox2->SetControlTemplate({});

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Back to CheckBox", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/ControlTemplate"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceControlThemeName
			);
		});
	});
}
