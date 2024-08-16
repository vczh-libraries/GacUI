#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>
  <Instance name="MyListItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::MyListItemTemplate">
      <TextListItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
        <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
          <SolidBorder>
            <att.Color-bind><![CDATA[
              cast Color (
                not self.VisuallyEnabled ? "#00000000" :
                not self.Selected ? "#00000000" :
                "#88FF88"
              )
            ]]></att.Color-bind>
          </SolidBorder>
          <Bounds AlignmentToParent="left:5 top:1 right:5 bottom:1" MinSizeLimitation="LimitToElement">
            <SolidLabel Font-bind="self.Font" Ellipse="true">
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
        for (item in range[1, 20])
        {
          list.Items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </TextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
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
      <ref.Members><![CDATA[
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            list.Items.Add(new TextItem^($"Item $(item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </TextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiTextList")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate"));
		});
	}
}