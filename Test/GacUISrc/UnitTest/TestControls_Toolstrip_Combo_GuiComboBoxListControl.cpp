#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceComboBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Ctor><![CDATA[{
        for (item in range[1, 5])
        {
          list.Items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiComboBox" ClientSize="x:320 y:240">
        <ComboBox ref.Name="combo" Alt="C">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:-1 bottom:-1" PreferredMinSize="x:160"/>
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
		GuiComboBoxListControl_TestCases(
			resourceComboBox,
			WString::Unmanaged(L"GuiComboBoxListControl"));
	});
}