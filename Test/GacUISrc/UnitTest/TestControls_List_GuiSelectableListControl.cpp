#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	GuiTextList
	***********************************************************************/
	{
		const WString resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var counter : int = 0;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          list.Items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <ev.SelectionChanged-eval><![CDATA[{
            self.counter = self.counter + 1;
            var title = $"[$(self.counter)]";
            for (item in list.SelectedItems)
            {
              title = title & $" $(item)";
            }
            self.Text = title & $" [$(list.SelectedItemIndex) -> $(list.SelectedItemText)]";
          }]]></ev.SelectionChanged-eval>
        </TextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiSelectableListControl (SingleSelect)")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTextList"));
		});

		TEST_CATEGORY(L"GuiSelectableListControl (MultiSelect)")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTextList"));
		});
	}
}