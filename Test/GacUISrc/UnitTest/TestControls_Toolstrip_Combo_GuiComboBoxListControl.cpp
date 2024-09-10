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

	const auto resourceComboBoxWithItemTemplate = LR"GacUISrc(
<Resource>
  <Instance name="MyListItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::MyListItemTemplate">
      <TextListItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
        <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
          <SolidBorder>
            <att.Color-bind><![CDATA[
              cast Color (
                not self.VisuallyEnabled ? "#00000000" :
                not self.Selected ? "#000088" :
                "#88FF88"
              )
            ]]></att.Color-bind>
          </SolidBorder>
          <Bounds AlignmentToParent="left:5 top:1 right:5 bottom:1" MinSizeLimitation="LimitToElement">
            <SolidLabel Font-bind="self.Font" Ellipse="false">
              <att.Text-format><![CDATA[[$(self.Index)] = $(self.Text)$(' ' & cast string (self.Context) ?? '')]]></att.Text-format>
              <att.Color-bind><![CDATA[
                cast Color (
                  not self.VisuallyEnabled ? "#888888" :
                  not self.Selected ? self.TextColor :
                  "#88FF88"
                )
              ]]></att.Color-bind>
            </SolidLabel>
          </Bounds>
        </Bounds>
      </TextListItemTemplate>
    </Instance>
  </Instance>
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
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ListControl>
            <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
              <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
            </TextList>
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
			resourceComboBoxWithItemTemplate,
			WString::Unmanaged(L"GuiComboBoxListControl/ItemTemplate"));
	});
}