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

		TEST_CATEGORY(L"GuiTextList/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTextList"));
		});

		TEST_CATEGORY(L"GuiTextList/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTextList"));
		});
	}

	/***********************************************************************
	GuiBindableTextList
	***********************************************************************/
	{
		const WString resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
        var counter : int = 0;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <ev.SelectionChanged-eval><![CDATA[{
            self.counter = self.counter + 1;
            var title = $"[$(self.counter)]";
            for (item in list.SelectedItems)
            {
              title = title & $" $(item)";
            }
            self.Text = title & $" [$(list.SelectedItemIndex) -> $(list.SelectedItemText)]";
          }]]></ev.SelectionChanged-eval>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTextList/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableTextList"));
		});

		TEST_CATEGORY(L"GuiBindableTextList/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableTextList"));
		});
	}

	/***********************************************************************
	GuiTreeView
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
          var treeViewItem = new TreeViewItem^(null, $"Item $(item)");
          var treeNode = new MemoryNodeProvider^(treeViewItem);
          list.Nodes.Children.Add(treeNode);
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <TreeView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
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
        </TreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiTreeView/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTreeView"));
		});

		TEST_CATEGORY(L"GuiTreeView/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiTreeView"));
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

		const WString resourceListControl = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:TreeViewData^ = new TreeViewData^();
        var counter : int = 0;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Children.Add(new TreeViewData^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <BindableTreeView ref.Name="list" env.ItemType="TreeViewData^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.TextProperty>Text</att.TextProperty>
          <att.ChildrenProperty>Children</att.ChildrenProperty>
          <ev.SelectionChanged-eval><![CDATA[{
            self.counter = self.counter + 1;
            var title = $"[$(self.counter)]";
            for (item in list.SelectedItems)
            {
              title = title & $" $(item)";
            }
            self.Text = title & $" [$(list.SelectedItemIndex) -> $(list.SelectedItemText)]";
          }]]></ev.SelectionChanged-eval>
        </BindableTreeView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTreeView/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableTreeView"));
		});

		TEST_CATEGORY(L"GuiBindableTreeView/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableTreeView"));
		});
	}

	/***********************************************************************
	GuiListView
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
          var listViewItem = new ListViewItem^();
          listViewItem.Text = $"Item $(item)";
          list.Items.Add(listViewItem);
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <ListView ref.Name="list" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
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
        </ListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiListView/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiListView"));
		});

		TEST_CATEGORY(L"GuiListView/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiListView"));
		});
	}

	/***********************************************************************
	GuiBindableListView
	***********************************************************************/
	{
		const WString resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
        var counter : int = 0;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <BindableListView ref.Name="list" env.ItemType="TextItem^" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ Text="Text" TextProperty="Text"/>
          </att.Columns>
          <ev.SelectionChanged-eval><![CDATA[{
            self.counter = self.counter + 1;
            var title = $"[$(self.counter)]";
            for (item in list.SelectedItems)
            {
              title = title & $" $(item)";
            }
            self.Text = title & $" [$(list.SelectedItemIndex) -> $(list.SelectedItemText)]";
          }]]></ev.SelectionChanged-eval>
        </BindableListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableListView/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableListView"));
		});

		TEST_CATEGORY(L"GuiBindableListView/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableListView"));
		});
	}

	/***********************************************************************
	GuiBindableDataGrid
	***********************************************************************/
	{
		const WString resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items:observe TextItem^[] = {};
        var counter : int = 0;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        for (item in range[1, 20])
        {
          items.Add(new TextItem^($"Item $(item)"));
        }
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiSelectableListControl" ClientSize="x:320 y:240">
        <BindableDataGrid ref.Name="list" env.ItemType="TextItem^" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.Columns>
            <_ Text="Text" TextProperty="Text"/>
          </att.Columns>
          <ev.SelectionChanged-eval><![CDATA[{
            self.counter = self.counter + 1;
            var title = $"[$(self.counter)]";
            for (item in list.SelectedItems)
            {
              title = title & $" $(item)";
            }
            self.Text = title & $" [$(list.SelectedItemIndex) -> $(list.SelectedItemText)]";
          }]]></ev.SelectionChanged-eval>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableDataGrid/SingleSelect")
		{
			GuiSelectableListControl_SingleSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableDataGrid"));
		});

		TEST_CATEGORY(L"GuiBindableDataGrid/MultiSelect")
		{
			GuiSelectableListControl_MultiSelect_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiSelectableListControl/GuiBindableDataGrid"));
		});
	}
}