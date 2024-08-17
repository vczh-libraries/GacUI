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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            list.Items.Add(new TextItem^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiTextList")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiTextList"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <BindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                <att.ItemSource-eval>self.items</ItemSource-eval>
                <att.TextProperty>Text</att.TextProperty>
              </BindableTextList>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTextList")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiBindableTextList"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            var treeViewItem = new TreeViewItem^(null, $"Item $(start + item)");
            var treeNode = new MemoryNodeProvider^(treeViewItem);
            list.Nodes.Children.Add(treeNode);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TreeView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TreeView>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiTreeView")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiTreeView"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            items.Children.Add(new TreeViewData^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <BindableTreeView ref.Name="list" env.ItemType="TreeViewData^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                <att.ItemSource-eval>self.items</ItemSource-eval>
                <att.TextProperty>Text</att.TextProperty>
                <att.ChildrenProperty>Children</att.ChildrenProperty>
              </BindableTreeView>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableTreeView")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiBindableTreeView"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            var listViewItem = new ListViewItem^();
            listViewItem.Text = $"Item $(start + item)";
            list.Items.Add(listViewItem);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ListView ref.Name="list" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </ListView>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiListView")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiListView"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <BindableListView ref.Name="list" env.ItemType="TextItem^" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                <att.ItemSource-eval>self.items</ItemSource-eval>
                <att.Columns>
                  <_ TextProperty="Text"/>
                </att.Columns>
              </BindableListView>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableListView")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiBindableListView"));
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
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <BindableDataGrid ref.Name="list" env.ItemType="TextItem^" View="SmallIcon" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                <att.ItemSource-eval>self.items</ItemSource-eval>
                <att.Columns>
                  <_ TextProperty="Text"/>
                </att.Columns>
              </BindableDataGrid>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"GuiBindableDataGrid")
		{
			GuiListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiListControl/GuiBindableDataGrid"));
		});
	}
}