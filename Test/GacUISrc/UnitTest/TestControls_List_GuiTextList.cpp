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
	const auto resourceTextList = LR"GacUISrc(
<Resource>
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
      <Window ref.Name="self" Text-format="GuiTextList [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:320 y:240">
        <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
        </TextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceTextListItemTemplate = LR"GacUISrc(
<Resource>
  <Instance name="CheckedIconTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::CheckedIconTemplate">
      <SelectableButtonTemplate ref.Name="self" PreferredMinSize="x:32 y:16" MinSizeLimitation="LimitToElementAndChildren">
        <SolidBorder Color="#00FF00"/>
        <Bounds Visible-bind="self.Selected" PreferredMinSize="x:12 y:12" AlignmentToParent="left:2 top:2 right:-1 bottom:2">
          <SolidBackground Color="#00FF00"/>
        </Bounds>
        <Bounds Visible-bind="not self.Selected" PreferredMinSize="x:12 y:12" AlignmentToParent="left:-1 top:2 right:2 bottom:2">
          <SolidBackground Color="#888888"/>
        </Bounds>
      </SelectableButtonTemplate>
    </Instance>
  </Instance>

  <Instance name="ItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::ItemTemplate">
      <TextListItemTemplate ref.Name="self" Checked-bind="icon.Selected" MinSizeLimitation="LimitToElementAndChildren">
        <Table BorderVisible="true" CellPadding="2" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:1 column:0">
            <CheckBox ref.Name="icon" Selected-bind="self.Checked" AutoFocus="false" AutoSelection="true">
              <att.ControlTemplate>gacuisrc_unittest::CheckedIconTemplate</att.ControlTemplate>
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </CheckBox>
          </Cell>
          <Cell Site="row:1 column:1">
            <SolidLabel Font-bind="self.Font" Color-bind="self.TextColor" Text-bind="self.Text" Ellipse="true" VerticalAlignment="Center" HorizontalAlignment="Left"/>
          </Cell>
        </Table>
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
      <Window ref.Name="self" Text-format="GuiTextList [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:320 y:240">
        <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
        </TextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiTextList")
	{
		TEST_CASE(L"MakeVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Insert(0, Ptr(new TextItem(L"First Item")));
				});
				protocol->OnNextIdleFrame(L"Add to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Add(Ptr(new TextItem(L"Last Item")));
				});
				protocol->OnNextIdleFrame(L"Add to Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().RemoveAt(0);
					listControl->GetItems().RemoveAt(listControl->GetItems().Count() - 1);
				});
				protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Set(2, Ptr(new TextItem(L"Updated Item")));
				});
				protocol->OnNextIdleFrame(L"Update 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/MakeVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"MakeInvisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Insert(0, Ptr(new TextItem(L"First Item")));
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Add to Top and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Add(Ptr(new TextItem(L"Last Item")));
					listControl->SetSelected(21, true);
				});
				protocol->OnNextIdleFrame(L"Add to Last and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(21);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().RemoveAt(0);
					listControl->GetItems().RemoveAt(listControl->GetItems().Count() - 1);
				});
				protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems().Set(2, Ptr(new TextItem(L"Updated Item")));
				});
				protocol->OnNextIdleFrame(L"Update 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/MakeInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"UpdateVisibleItems (View = Check)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[1]->SetText(L"Updated Text");
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[2]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[0]->SetText(L"New Text");
					listControl->GetItems()[0]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text and Check", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/UpdateVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"UpdateInvisibleItems (View = Radio)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetView(TextListView::Radio);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[0]->SetText(L"New Text");
					listControl->GetItems()[0]->SetChecked(true);
					listControl->GetItems()[1]->SetText(L"Updated Text");
					listControl->GetItems()[2]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/UpdateInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
	});

	TEST_CATEGORY(L"Accessibility")
	{
		TEST_CASE(L"ClickVisibleItems (View = Check)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					TEST_ASSERT(listControl->GetItems()[2]->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(listControl->GetItems()[2]->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					TEST_ASSERT(listControl->GetItems()[0]->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(listControl->GetItems()[0]->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/ClickVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
	});

	TEST_CATEGORY(L"GuiTextListItemTemplate")
	{
		TEST_CASE(L"UpdateVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[1]->SetText(L"Updated Text");
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[2]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[0]->SetText(L"New Text");
					listControl->GetItems()[0]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text and Check", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/GuiTextListItemTemplate/UpdateVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextListItemTemplate
				);
		});

		TEST_CASE(L"UpdateInvisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetItems()[0]->SetText(L"New Text");
					listControl->GetItems()[0]->SetChecked(true);
					listControl->GetItems()[1]->SetText(L"Updated Text");
					listControl->GetItems()[2]->SetChecked(true);
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/GuiTextListItemTemplate/UpdateInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextListItemTemplate
				);
		});

		TEST_CASE(L"ClickVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					TEST_ASSERT(listControl->GetItems()[2]->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(listControl->GetItems()[2]->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					TEST_ASSERT(listControl->GetItems()[0]->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(listControl->GetItems()[0]->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiTextList/GuiTextListItemTemplate/ClickVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextListItemTemplate
				);
		});
	});
}