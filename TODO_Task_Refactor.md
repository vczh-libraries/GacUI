Two work to improve `<ez:EasyLayout/>` and UIA supports.

## EazyLayout

Here are code snippets from `Release/Tutorial/GacUI_Controls/AddressBook/UI/Resource.xml`. I don't mean here is the only defect, you should take that as an example for better understanding my request:

```XML
<ez:Layout Padding="5" Border="false">
  <ez:Top>
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <ToolstripToolBar>
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
        <ToolstripButton Command-ref="commandNewFolder"/>
        <ToolstripButton Command-ref="commandDeleteFolder"/>
      </ToolstripToolBar>
    </Bounds>
  </ez:Top>
  <ez:Fill Percentage="1.0" Direction="Vertical">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <BindableTreeView ref.Name="treeViewFolders" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false" env.ItemType="demo::ICategory^">
        <att.ItemSource-eval>ViewModel.GetRootCategory()</att.ItemSource-eval>
        <att.TextProperty>Name</att.TextProperty>
        <att.ImageProperty>Image</att.ImageProperty>
        <att.ChildrenProperty>Folders</att.ChildrenProperty>
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
      </BindableTreeView>
    </Bounds>
  </ez:Fill>
</ez:Layout>
```

```XML
<ez:Row CellOption="composeType:Percentage percentage:1.0">
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:Percentage percentage:1.0"/>
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:MinSize"/>
</ez:Row>
<ez:Row CellOption="composeType:MinSize">
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:Percentage percentage:1.0"/>
  <ez:Column CellOption="composeType:MinSize">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <Button Text="OK">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0" PreferredMinSize="x:60"/>
        <ev.Clicked-eval><![CDATA[{ self.Ready = true; self.Close(); }]]></ev.Clicked-eval>
      </Button>
    </Bounds>
  </ez:Column>
  <ez:Column CellOption="composeType:MinSize">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <Button Text="Cancel">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0" PreferredMinSize="x:60"/>
        <ev.Clicked-eval><![CDATA[{ self.Ready = false; self.Close(); }]]></ev.Clicked-eval>
      </Button>
    </Bounds>
  </ez:Column>
</ez:Row>
```

In the original spec `TODO_Task_EazyLayout.md` there are two rules:
- ez:Fill@Direction is only needed when the layout has only ez:Fill at a certain level and only one is needed. Here we have ez:Top as its sibling node, so Direction="Vertical" should be inferred.
- When rows over columns (vice vesa), for every column position, only one ez:Column needs to specify `CellOption`. Here we repeated everything. By the way, since the default value is MinSize, it means when multiple ez:Column on the same column position conflicts, all MinSize will be ignored.

Basically the XML is technically correct, but I would like you to omit whatever you can, so that it could be used as a test to see if omitting actually works or not. And tutorials in Release folder are samples, people are going to learn from them, keep them simple would be one of the goal. Here my request is that, update all XML UI in `GacUI_Controls.sln` to omit ez:* properties that are not needed, rebuild the solution, starts every affected apps, and make sure the visible layouts are not changed.

By the way, in newly added test cases running in remote protocol based unit test framework, there are some tests to see if `BuildLayout` crashes. I don't think it is appropriate to put them here because the goal for remote protocol based unit test framework is to see how UI actually layouted in each frame. To test the error, we should move them to `TestCompositions_*.cpp`, it is better to create a dedicated file for such cases `TestCompositions_EazyLayoutFailures.cpp`.

## UI Automation

### Responding to WM_GETOBJECT

The current implementation is too proactive. In order to improve the performance when no UIA client is asking for it:
- Any UIA interface implementation (UIA object) should be created when it is requested, do not create any UIA object ahead of time.
- COM is based on reference counting so it should be easy to do: when any references from UIA client is lost, those objects should be released directly, as well as detaching all listening events. Since GuiWindow exposes events to tell you when a composition is removed from the control, verify did the code already handle these event to release those objects and detecthing all listening events.
  - This is important, as currently I found that when a sub window is created and then deleted, closing the app causing a crash in UIA. I have not debugged into it, but I doubt this could be a reason.
- When UIA is running the GacUI itself seems to be very slow, I will come back and check this later to see if the lazy UIA creation resolves this issue or not.
  - The most easy observation is the progress bar tutoail in `GacUI_Controls`, when the document is completed downloaded after clicking the button and waiting for the progress bar to reach the end, maximizing the window takes a lot of time in debug mode, it did not happen before implementing UIA.

Improving UiaListApp tool:
- In `UiaListApp`, right click UIA node to popup a context menu for "Inspect" is technically working, but you have to pass the cursor position to show the context menu in a correct position, currently it is popup at the middle of the top border, which is not good in terms of user experience.
