#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resourceListItemTemplate= LR"GacUISrc(
<Resource>
  <Instance name="MyListItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::MyListItemTemplate">
      <TextListItemTemplate ref.Name="self" AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
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
              <att.Text-format><![CDATA[
                $(self.Text)$(' ' & cast string (self.Context) ?? '')
              ]]></att.Text-format>
              <att.Color-bind><![CDATA[
                cast Color (
                  not self.VisuallyEnabled ? "#888888" :
                  not self.Selected ? "#FFFFFF" :
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

	TEST_CATEGORY(L"GuiListItemTemplate")
	{
		TEST_CASE(L"MouseVisualEffect")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/MouseVisualEffect"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListItemTemplate
				);
		});
	});
}