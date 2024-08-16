#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTextList = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiBindableTextList [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:320 y:240">
        <GuiBindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <att.CheckedProperty>Checked</att.CheckedProperty>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceTextListItemTemplate = LR"GacUISrc(
<Resource>
  <Instance name="CheckedBulletTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::CheckedBulletTemplate">
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
      <TextListItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
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
            <CheckBox ref.Name="bullet" Selected-bind="self.Checked" AutoFocus="false" AutoSelection="false">
              <att.ControlTemplate>gacuisrc_unittest::CheckedBulletTemplate</att.ControlTemplate>
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var textItemView = self.AssociatedListControl.ItemProvider.RequestView(ITextItemView::GetIdentifier()) as ITextItemView*;
                textItemView.SetChecked(self.Index, not bullet.Selected);
              }]]></ev.Clicked-eval>
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
        var items:observe TextItem^[] = {};
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiBindableTextList [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:320 y:240">
        <GuiBindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <att.CheckedProperty>Checked</att.CheckedProperty>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableTextList")
	{
		GuiTextList_TestCases(
			resourceTextList,
			WString::Unmanaged(L"GuiBindableTextList"),
			true,
			[](GuiWindow* window)
			{
				return UnboxValue<Ptr<IValueList>>(Value::From(window).GetProperty(L"items"));
			},
			[](GuiWindow* window, vint start, vint count)
			{
				auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
				listControl->NotifyItemDataModified(start, count);
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
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));

					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(2))->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(2))->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));

					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(0))->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(0))->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/ClickVisibleItems"),
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
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));
					UnboxValue<Ptr<TextItem>>(items->Get(1))->SetText(L"Updated Text");
					listControl->NotifyItemDataModified(1, 1);
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));
					UnboxValue<Ptr<TextItem>>(items->Get(2))->SetChecked(true);
					listControl->NotifyItemDataModified(2, 1);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));
					UnboxValue<Ptr<TextItem>>(items->Get(0))->SetText(L"New Text");
					UnboxValue<Ptr<TextItem>>(items->Get(0))->SetChecked(true);
					listControl->NotifyItemDataModified(0, 1);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text and Check", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/UpdateVisibleItems"),
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
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));
					UnboxValue<Ptr<TextItem>>(items->Get(0))->SetText(L"New Text");
					UnboxValue<Ptr<TextItem>>(items->Get(0))->SetChecked(true);
					UnboxValue<Ptr<TextItem>>(items->Get(1))->SetText(L"Updated Text");
					UnboxValue<Ptr<TextItem>>(items->Get(2))->SetChecked(true);
					listControl->NotifyItemDataModified(0, 3);
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/UpdateInvisibleItems"),
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
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));

					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(2))->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(2))->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					auto items = UnboxValue<Ptr<IValueObservableList>>(Value::From(window).GetProperty(L"items"));

					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(0))->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(UnboxValue<Ptr<TextItem>>(items->Get(0))->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/ClickVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextListItemTemplate
				);
		});
	});
}