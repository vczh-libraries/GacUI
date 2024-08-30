#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	GuiTreeView
	***********************************************************************/
	{
		const WString resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func MakeItem(name:string, item:int) : MemoryNodeProvider^
        {
          var treeViewItem = new TreeViewItem^(null, $"$(name) $(item)");
          var treeNode = new MemoryNodeProvider^(treeViewItem);
          return treeNode;
        }
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            var mainItem = MakeItem("Item", item);
            mainItem.Children.Add(MakeItem("Child 1st", item));
            mainItem.Children.Add(MakeItem("Child 2nd", item));
            mainItem.Children.Add(MakeItem("Child 3rd", item));
            mainItem.Children[1].Children.Add(MakeItem("Leaf 2/1st", item));
            mainItem.Children[1].Children.Add(MakeItem("Leaf 2/2nd", item));
            mainItem.Children[2].Children.Add(MakeItem("Leaf 3/1st", item));
            mainItem.Children[2].Children.Add(MakeItem("Leaf 3/2nd", item));
            list.Nodes.Children.Add(mainItem);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiTreeView" ClientSize="x:480 y:320">
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
			GuiVirtualTreeListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiVirtualTreeListControl/GuiTreeView"));
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
        func MakeItem(name:string, item:int) : TreeViewData^
        {
          return new TreeViewData^($"$(name) $(item)");
        }
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            var mainItem = MakeItem("Item", item);
            mainItem.Children.Add(MakeItem("Child 1st", item));
            mainItem.Children.Add(MakeItem("Child 2nd", item));
            mainItem.Children.Add(MakeItem("Child 3rd", item));
            mainItem.Children[1].Children.Add(MakeItem("Leaf 2/1st", item));
            mainItem.Children[1].Children.Add(MakeItem("Leaf 2/2nd", item));
            mainItem.Children[2].Children.Add(MakeItem("Leaf 3/1st", item));
            mainItem.Children[2].Children.Add(MakeItem("Leaf 3/2nd", item));
            items.Children.Add(mainItem);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableTreeView" ClientSize="x:480 y:320">
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
			GuiVirtualTreeListControl_TestCases(
				resourceListControl,
				WString::Unmanaged(L"GuiVirtualTreeListControl/GuiBindableTreeView"));
		});
	}
}