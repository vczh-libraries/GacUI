#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/

	const WString fragmentTextListItemTemplate = LR"GacUISrc(
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
)GacUISrc";

	/***********************************************************************
	GuiTextList
	***********************************************************************/
	{
		const WString resourceTextList = LR"GacUISrc(
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

		const WString resourceTextListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentTextListItemTemplate + LR"GacUISrc(
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

		auto getItems = [](GuiWindow* window)
		{
			auto listControl = FindObjectByName<GuiTextList>(window, L"list");
			return UnboxValue<Ptr<IValueList>>(BoxParameter(listControl->GetItems()));
		};

		auto notifyItemDataModified = [](auto&&...) {};

		TEST_CATEGORY(L"GuiTextList")
		{
			GuiVirtualTextList_TestCases(
				resourceTextList,
				WString::Unmanaged(L"GuiTextList"),
				true,
				getItems,
				notifyItemDataModified);
		});

		TEST_CATEGORY(L"GuiTextList/GuiTextListItemTemplate")
		{
			GuiTextListItemTemplate_TestCases(
				resourceTextListItemTemplate,
				WString::Unmanaged(L"GuiTextList/GuiTextListItemTemplate"),
				false,
				getItems,
				notifyItemDataModified);
		});
	}

	/***********************************************************************
	GuiBindableTextList
	***********************************************************************/
	{
		const WString resourceTextList = LR"GacUISrc(
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
        <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
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

		const WString resourceTextListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentTextListItemTemplate + LR"GacUISrc(
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
        <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
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

		auto getItems = [](GuiWindow* window)
		{
			return UnboxValue<Ptr<IValueList>>(Value::From(window).GetProperty(L"items"));
		};

		auto notifyItemDataModified = [](GuiWindow* window, vint start, vint count)
		{
			auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
			listControl->NotifyItemDataModified(start, count);
		};

		TEST_CATEGORY(L"GuiBindableTextList")
		{
			GuiVirtualTextList_TestCases(
				resourceTextList,
				WString::Unmanaged(L"GuiBindableTextList"),
				true,
				getItems,
				notifyItemDataModified);
		});

		TEST_CATEGORY(L"GuiBindableTextList/GuiTextListItemTemplate")
		{
			GuiTextListItemTemplate_TestCases(
				resourceTextListItemTemplate,
				WString::Unmanaged(L"GuiBindableTextList/GuiTextListItemTemplate"),
				false,
				getItems,
				notifyItemDataModified);
		});
	}
}