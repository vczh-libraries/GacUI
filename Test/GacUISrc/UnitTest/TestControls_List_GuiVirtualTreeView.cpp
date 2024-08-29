#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared (Image)
	***********************************************************************/

	const WString fragmentImageResource = LR"GacUISrc(
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>
)GacUISrc";

	const WString fragmentMemberWithoutImage = LR"GacUISrc(
        func GetImage(index:int) : GuiImageData^ { return null; }
)GacUISrc";

	const WString fragmentMemberWithImage = LR"GacUISrc(
        var smallImages : GuiImageData^[] = null;
        func EnsureImages() : void
        {
          if (smallImages is not null)
          {
            return;
          }
          smallImages = {
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Cert", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Data", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Link", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Folder", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Light", true));
          };
        }
        func GetImage(index:int) : GuiImageData^
        {
          EnsureImages();
          return smallImages[(index - 1) % 5];
        }
)GacUISrc";

	/***********************************************************************
	Shared (TreeListItemTemplate)
	***********************************************************************/

	const WString fragmentTreeListItemTemplate1 = LR"GacUISrc(
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
      <TreeItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
        <SolidBorder>
          <att.Color-bind><![CDATA[
            cast Color (
              not self.VisuallyEnabled ? "#00000000" :
              not self.Selected ? "#000088" :
              "#88FF88"
            )
          ]]></att.Color-bind>
        </SolidBorder>
        <Table BorderVisible="true" CellPadding="1" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:1 column:0" PreferredMinSize-bind="{x:self.Level * 12 y:0}"/>
          <Cell Site="row:1 column:1">
            <CheckBox ref.Name="bullet" Selected-bind="self.Expanding" Visible-bind="self.Expandable" AutoFocus="false" AutoSelection="false">
              <att.ControlTemplate>gacuisrc_unittest::CheckedBulletTemplate</att.ControlTemplate>
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var nodeItemView = self.AssociatedListControl.ItemProvider.RequestView(INodeItemView::GetIdentifier()) as INodeItemView*;
                var node = nodeItemView.RequestNode(self.Index);
                node.Expanding = not node.Expanding;
              }]]></ev.Clicked-eval>
            </CheckBox>
          </Cell>
          <Cell Site="row:1 column:2" PreferredMinSize="x:16 y:16">
            <ImageFrame Image-bind="self.Image.Image ?? null" FrameIndex-bind="self.Image.FrameIndex ?? 0" Enabled-bind="self.VisuallyEnabled" Stretch="true"/>
          </Cell>
          <Cell Site="row:1 column:3">
            <SolidLabel Font-bind="self.Font" Ellipse="true" VerticalAlignment="Center" HorizontalAlignment="Left">
              <att.Text-format><![CDATA[$(self.Text)$(cast string (self.Context) ?? '')]]></att.Text-format>
              <att.Color-bind><![CDATA[
                cast Color (
                  not self.VisuallyEnabled ? "#888888" :
                  not self.Selected ? self.TextColor :
                  "#88FF88"
                )
              ]]></att.Color-bind>
            </SolidLabel>
          </Cell>
        </Table>
      </TreeItemTemplate>
    </Instance>
  </Instance>
)GacUISrc";

	const WString fragmentTreeListItemTemplate2 = LR"GacUISrc(
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
      <TreeItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
        <Table BorderVisible="true" CellPadding="1" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:1 column:0" PreferredMinSize-bind="{x:self.Level * 12 y:0}"/>
          <Cell Site="row:1 column:1">
            <CheckBox ref.Name="bullet" Selected-bind="self.Expanding" Visible-bind="self.Expandable" AutoFocus="false" AutoSelection="false">
              <att.ControlTemplate>gacuisrc_unittest::CheckedBulletTemplate</att.ControlTemplate>
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var nodeItemView = self.AssociatedListControl.ItemProvider.RequestView(INodeItemView::GetIdentifier()) as INodeItemView*;
                var node = nodeItemView.RequestNode(self.Index);
                node.Expanding = not node.Expanding;
              }]]></ev.Clicked-eval>
            </CheckBox>
          </Cell>
          <Cell Site="row:1 column:2" PreferredMinSize="x:16 y:16">
            <ImageFrame Image-bind="self.Image.Image ?? null" FrameIndex-bind="self.Image.FrameIndex ?? 0" Enabled-bind="self.VisuallyEnabled" Stretch="true"/>
          </Cell>
          <Cell Site="row:1 column:3">
            <SolidLabel Font-bind="self.Font" Color-bind="self.TextColor" Text-bind="self.Text"  Ellipse="true" VerticalAlignment="Center" HorizontalAlignment="Left"/>
          </Cell>
        </Table>
      </TreeItemTemplate>
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
)GacUISrc";

		const WString fragmentTreeViewSecond = LR"GacUISrc(
        func MakeItem(name:string, item:int) : MemoryNodeProvider^
        {
          var treeViewItem = new TreeViewItem^(GetImage(item), $"$(name) $(item)");
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

		const WString fragmentTreeViewThird = LR"GacUISrc(
        </TreeView>
      </Window>
    </Instance>
  </Instance>
)GacUISrc";

		const WString resourceTreeView = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate1 = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeListItemTemplate1 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate2 = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeListItemTemplate2 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeViewWithImage = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentImageResource + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate2WithImage = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentImageResource + LR"GacUISrc(
)GacUISrc" + fragmentTreeListItemTemplate2 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
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

			GuiVirtualTreeView_Images_TestCases(
				resourceTreeViewWithImage,
				WString::Unmanaged(L"GuiTreeView"));
		});

		TEST_CATEGORY(L"GuiTreeItemTemplate/GuiTreeView (1)")
		{
			GuiTreeItemTemplate1_TestCases(
				resourceTreeListItemTemplate1,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiTreeView"));
		});

		TEST_CATEGORY(L"GuiTreeItemTemplate/GuiTreeView (2)")
		{
			GuiTreeItemTemplate2_TestCases(
				resourceTreeListItemTemplate2,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiTreeView"),
				getRootItems,
				getChildItems,
				updateText,
				notifyNodeDataModified);

			GuiVirtualTreeView_Images_TestCases(
				resourceTreeListItemTemplate2WithImage,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiTreeView"));
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
      new(image:GuiImageData^, text:string){ Image=image; Text=text; }
    }
  ]]></Workflow></Script>
)GacUISrc";

		const WString fragmentTreeViewFirst = LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
)GacUISrc";

		const WString fragmentTreeViewSecond = LR"GacUISrc(
        var items:TreeViewData^ = new TreeViewData^();
        func MakeItem(name:string, item:int) : TreeViewData^
        {
          return new TreeViewData^(GetImage(item), $"$(name) $(item)");
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

		const WString fragmentTreeViewThird = LR"GacUISrc(
        </BindableTreeView>
      </Window>
    </Instance>
  </Instance>
)GacUISrc";

		const WString resourceTreeView = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate1 = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeListItemTemplate1 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate2 = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeListItemTemplate2 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithoutImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeViewWithImage = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentImageResource + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
</Resource>
)GacUISrc";

		const WString resourceTreeListItemTemplate2WithImage = LR"GacUISrc(
<Resource>
)GacUISrc" + fragmentImageResource + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewData + LR"GacUISrc(
)GacUISrc" + fragmentTreeListItemTemplate2 + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewFirst + LR"GacUISrc(
)GacUISrc" + fragmentMemberWithImage + LR"GacUISrc(
)GacUISrc" + fragmentTreeViewSecond + LR"GacUISrc(
          <att.ItemTemplate>gacuisrc_unittest::ItemTemplate</att.ItemTemplate>
)GacUISrc" + fragmentTreeViewThird + LR"GacUISrc(
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

			GuiVirtualTreeView_Images_TestCases(
				resourceTreeViewWithImage,
				WString::Unmanaged(L"GuiBindableTreeView"));
		});

		TEST_CATEGORY(L"GuiTreeItemTemplate/GuiBindableTreeView (1)")
		{
			GuiTreeItemTemplate1_TestCases(
				resourceTreeListItemTemplate1,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiBindableTreeView"));
		});

		TEST_CATEGORY(L"GuiTreeItemTemplate/GuiBindableTreeView (2)")
		{
			GuiTreeItemTemplate2_TestCases(
				resourceTreeListItemTemplate2,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiBindableTreeView"),
				getRootItems,
				getChildItems,
				updateText,
				notifyNodeDataModified);

			GuiVirtualTreeView_Images_TestCases(
				resourceTreeListItemTemplate2WithImage,
				WString::Unmanaged(L"GuiTreeItemTemplate/GuiBindableTreeView"));
		});
	}
}