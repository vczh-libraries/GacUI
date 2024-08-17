#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/
	
	const WString fragmentListItemTemplate = LR"GacUISrc(
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
)GacUISrc";

	/***********************************************************************
	GuiTextList
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
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
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
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
				WString::Unmanaged(L"GuiListItemTemplate/GuiTextList"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiTextList"));
		});
	}

	/***********************************************************************
	GuiBindableTextList
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
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
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTextList")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableTextList"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableTextList"));
		});
	}

	/***********************************************************************
	GuiTreeView
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          var treeViewItem = new TreeViewItem^(null, $"Item $(item)");
          var treeNode = new MemoryNodeProvider^(treeViewItem);
          list.Nodes.Children.Add(treeNode);
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <TreeView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </TreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            var treeViewItem = new TreeViewItem^(null, $"Item $(item)");
            var treeNode = new MemoryNodeProvider^(treeViewItem);
            list.Nodes.Children.Add(treeNode);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <TreeView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </TreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiTreeView")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiTreeView"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiTreeView"));
		});
	}

	/***********************************************************************
	GuiBindableTreeView
	***********************************************************************/
	{
		const WString fragmentTreeViewData = LR"GacUISrc(
  <Script name="TreeViewDataResource"><Workflow><![CDATA[
    module treeviewdata;
    using system::*;

    class TreeViewData
    {
      prop Text:string = "" {not observe}
      prop Children:observe TreeViewData^[] = {} {const, not observe}

      new(){}
      new(text:string){Text=text;}
    }
  ]]></Workflow></Script>
)GacUISrc";

		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentTreeViewData + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:TreeViewData^ = new TreeViewData^();
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Children.Add(new TreeViewData^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <BindableTreeView ref.Name="list" env.ItemType="TreeViewData^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <att.ChildrenProperty>Children</att.ChildrenProperty>
        </BindableTreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentTreeViewData + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:TreeViewData^ = new TreeViewData^();
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            items.Children.Add(new TreeViewData^($"Item $(item)"));  
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <BindableTreeView ref.Name="list" env.ItemType="TreeViewData^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <att.ChildrenProperty>Children</att.ChildrenProperty>
        </BindableTreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTreeView")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableTreeView"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableTreeView"));
		});
	}

	/***********************************************************************
	GuiListView
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
        for (item in range[1, 20])
        {
          var listViewItem = new ListViewItem^();
          listViewItem.Text = $"Item $(item)";
          list.Items.Add(listViewItem);
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <ListView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </ListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            var listViewItem = new ListViewItem^();
            listViewItem.Text = $"Item $(item)";
            list.Items.Add(listViewItem);
          }
        }
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
      }]]></ref.Ctor>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <ListView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
        </ListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiListView")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiListView"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiListView"));
		});
	}

	/***********************************************************************
	GuiBindableListView
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <BindableListView ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ TextProperty="Text"/>
          </att.Columns>
        </BindableListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
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
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
      }]]></ref.Ctor>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <BindableListView ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ TextProperty="Text"/>
          </att.Columns>
        </BindableListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableListView")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableListView"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableListView"));
		});
	}

	/***********************************************************************
	GuiBindableDataGrid
	***********************************************************************/
	{
		const WString resourceListItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiListItemTemplate" ClientSize="x:320 y:240">
        <BindableDataGrid ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ TextProperty="Text"/>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceGridItemTemplate = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentListItemTemplate + LR"GacUISrc(
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
      <ref.Ctor><![CDATA[{
        list.Arranger = new FixedHeightItemArranger^();
      }]]></ref.Ctor>
      <Window ref.Name="self" Text-format="GuiListItemTemplate $(list.SelectedItemIndex)" ClientSize="x:320 y:240">
        <BindableDataGrid ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemTemplate>gacuisrc_unittest::MyListItemTemplate</att.ItemTemplate>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ TextProperty="Text"/>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableDataGrid")
		{
			GuiListItemTemplate_TestCases(
				resourceListItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableDataGrid"));

			GuiListItemTemplate_WithAxis_TestCases(
				resourceGridItemTemplate,
				WString::Unmanaged(L"GuiListItemTemplate/GuiBindableDataGrid"));
		});
	}
}