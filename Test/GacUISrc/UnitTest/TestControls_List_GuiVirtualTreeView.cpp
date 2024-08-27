#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/

	const WString fragmentTreeListItemTemplate = LR"GacUISrc(
  <Instance name="ItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::ItemTemplate">
      <TreeListItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
      </TreeListItemTemplate>
    </Instance>
  </Instance>
)GacUISrc";

	/***********************************************************************
	GuiTreeView
	***********************************************************************/
	{
		const WString fragmentTreeViewFirst = LR"GacUISrc(
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
      <Window ref.Name="self" Text-format="GuiTreeView [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItem.text ?? '')]" ClientSize="x:320 y:240">
        <TreeView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
)GacUISrc";

		const WString fragmentTreeViewSecond = LR"GacUISrc(
        </TreeView>
      </Window>
    </Instance>
  </Instance>
)GacUISrc";

		const WString resourceTreeView = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeListItemTemplate + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
</Resource>
)GacUISrc";

		auto getRootItems = [](GuiWindow* window)
		{
			auto listControl = FindObjectByName<GuiTreeView>(window, L"list");
			return UnboxValue<Ptr<IValueList>>(BoxParameter(listControl->Nodes()->Children()));
		};

		auto getChildItems = [](Value node)
		{
			auto treeItem = UnboxValue<Ptr<MemoryNodeProvider>>(node);
			return UnboxValue<Ptr<IValueList>>(BoxParameter(treeItem->Children()));
		};

		auto updateText = [](Value node, WString text)
		{
			auto treeItem = UnboxValue<Ptr<MemoryNodeProvider>>(node);
			treeItem->GetData().Cast<TreeViewItem>()->text = text;
		};

		auto notifyNodeDataModified = [](GuiWindow* window, Value node)
		{
			auto listControl = FindObjectByName<GuiTreeView>(window, L"list");
			auto treeItem = UnboxValue<Ptr<MemoryNodeProvider>>(node);
			listControl->Nodes()->UpdateTreeViewData(treeItem.Obj());
		};

		TEST_CATEGORY(L"GuiTreeView")
		{
			GuiVirtualTreeView_TestCases(
				resourceTreeView,
				WString::Unmanaged(L"GuiTreeView"),
				getRootItems,
				getChildItems,
				updateText,
				notifyNodeDataModified);
		});

		TEST_CATEGORY(L"GuiTreeView/GuiTreeItemTemplate")
		{
			//GuiTreeItemTemplate_TestCases(
			//	resourceTreeListItemTemplate,
			//	WString::Unmanaged(L"GuiTreeView/GuiTreeItemTemplate"),
			//	getRootItems,
			//	getChildItems,
			//	updateText,
			//	notifyNodeDataModified);
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
    using presentation::*;

    class TreeViewData
    {
      prop Text:string = "" {not observe}
      prop Image:GuiImageData^ = null {not observe}
      prop Children:observe TreeViewData^[] = {} {const, not observe}
      prop DoNotUse : object = null {not observe}

      new(){}
      new(text:string){Text=text;}
    }
  ]]></Workflow></Script>
)GacUISrc";

		const WString fragmentTreeViewFirst = LR"GacUISrc(
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
      <Window ref.Name="self" Text-format="GuiBindableTreeView [$(list.SelectedItemIndex)] -&gt; [$((cast (TreeViewData^) list.SelectedItem).Text ?? '')]" ClientSize="x:320 y:240">
        <BindableTreeView ref.Name="list" env.ItemType="TreeViewData^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.ItemSource-eval>self.items</ItemSource-eval>
          <att.ReverseMappingProperty>DoNotUse</att.ReverseMappingProperty>
          <att.TextProperty>Text</att.TextProperty>
          <att.ImageProperty>Image</att.ImageProperty>
          <att.ChildrenProperty>Children</att.ChildrenProperty>
)GacUISrc";

		const WString fragmentTreeViewSecond = LR"GacUISrc(
        </BindableTreeView>
      </Window>
    </Instance>
  </Instance>
)GacUISrc";

		const WString resourceTreeView = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeListItemTemplate + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
</Resource>
)GacUISrc";

		auto getRootItems = [](GuiWindow* window)
		{
			auto listControl = FindObjectByName<GuiBindableTreeView>(window, L"list");
			return UnboxValue<Ptr<IValueList>>(listControl->GetItemSource().GetProperty(L"Children"));
		};

		auto getChildItems = [](Value node)
		{
			return UnboxValue<Ptr<IValueList>>(node.GetProperty(L"Children"));
		};

		auto updateText = [](Value node, WString text)
		{
			node.SetProperty(L"Text", BoxValue(text));
		};

		auto notifyNodeDataModified = [](GuiWindow* window, Value node)
		{
			auto listControl = FindObjectByName<GuiBindableTreeView>(window, L"list");
			listControl->NotifyNodeDataModified(node);
		};

		TEST_CATEGORY(L"GuiBindableTreeView")
		{
			GuiVirtualTreeView_TestCases(
				resourceTreeView,
				WString::Unmanaged(L"GuiBindableTreeView"),
				getRootItems,
				getChildItems,
				updateText,
				notifyNodeDataModified);
		});

		TEST_CATEGORY(L"GuiBindableTreeView/GuiTreeItemTemplate")
		{
			//GuiTreeItemTemplate_TestCases(
			//	resourceTreeListItemTemplate,
			//	WString::Unmanaged(L"GuiBindableTreeView/GuiTreeItemTemplate"),
			//	getRootItems,
			//	getChildItems,
			//	updateText,
			//	notifyNodeDataModified);
		});
	}
}