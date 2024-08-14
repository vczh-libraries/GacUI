#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
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
	const auto resourceListItemTemplate = LR"GacUISrc(
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

	const auto resourceGridItemTemplate = LR"GacUISrc(
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
        func AddItems(count:int) : void
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

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetEnabled(false);
				});
				protocol->OnNextIdleFrame(L"Disabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetEnabled(true);
				});
				protocol->OnNextIdleFrame(L"Enabled", [=]()
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

		TEST_CASE(L"Context")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"This")));
				});
				protocol->OnNextIdleFrame(L"This", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"That")));
				});
				protocol->OnNextIdleFrame(L"That", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetContext({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/Context"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListItemTemplate
				);
		});

		TEST_CASE(L"Font")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					FontProperties font;
					font.fontFamily = L"This Font";
					font.size = 18;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"18", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					FontProperties font;
					font.fontFamily = L"That Font";
					font.size = 24;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"24", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetFont({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(19);
					listControl->SetSelected(19, true);
				});
				protocol->OnNextIdleFrame(L"Scroll to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/Font"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListItemTemplate
				);
		});

		TEST_CASE(L"DisplayItemBackground")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 1st and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetDisplayItemBackground(false);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = false", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 3rd and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetDisplayItemBackground(true);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = true", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove({ location.x + 1,location.y });
					}
				});
				protocol->OnNextIdleFrame(L"Move", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/DisplayItemBackground"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListItemTemplate
				);
		});

		TEST_CASE(L"ArrangerAndAxis")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"AddItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetArranger(Ptr(new FixedSizeMultiColumnItemArranger));
				});
				protocol->OnNextIdleFrame(L"Grid", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				});
				protocol->OnNextIdleFrame(L"Rotate (DownLeft)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				});
				protocol->OnNextIdleFrame(L"Rotate (LeftUp)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				});
				protocol->OnNextIdleFrame(L"Rotate (UpRight)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				});
				protocol->OnNextIdleFrame(L"Rotate (RightDown)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetArranger(Ptr(new FixedHeightItemArranger));
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/ArrangerAndAxis"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceGridItemTemplate
				);
		});

		TEST_CASE(L"ArrangerAndAxisWithScrolls")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"AddItems", (Value_xs(), BoxValue<vint>(100)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetArranger(Ptr(new FixedSizeMultiColumnItemArranger));
				});
				protocol->OnNextIdleFrame(L"Grid", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				});
				protocol->OnNextIdleFrame(L"Rotate (DownLeft)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				});
				protocol->OnNextIdleFrame(L"Rotate (LeftUp)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				});
				protocol->OnNextIdleFrame(L"Rotate (UpRight)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				});
				protocol->OnNextIdleFrame(L"Rotate (RightDown)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetArranger(Ptr(new FixedHeightItemArranger));
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListItemTemplate/ArrangerAndAxisWithScrolls"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceGridItemTemplate
				);
		});
	});
}