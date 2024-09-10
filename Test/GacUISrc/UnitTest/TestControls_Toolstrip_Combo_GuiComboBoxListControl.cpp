#include "TestControls.h"

TEST_FILE
{
	const auto resourceComboBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          list.Items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiComboBox" ClientSize="x:320 y:240">
        <ComboBox ref.Name="combo">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:-1 bottom:-1" PreferredMinSize="x:160 y:30"/>
          <att.ListControl>
            <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false"/>
          </att.ListControl>
        </ComboBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiComboBoxListControl")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					TEST_ASSERT(comboBox->GetSelectedIndex() == -1);
					TEST_ASSERT(comboBox->GetSelectedItem().IsNull());
					comboBox->SetSelectedIndex(0);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 0);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 1");
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/GuiComboBoxListControl/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceComboBox
				);
		});
	});
}