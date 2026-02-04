# \<CommonScrollBehavior\>

This is a helper class for implementing a**\<DatePickerTemplate/\>**. It inherits from**GuiComponent**.

This helper class is a component. The control template must take care of the rendering of the control. After all compositions and controls are ready, call the following methods of the helper class to deliver user inputs to scroll control:
- **AttachScrollTemplate**: Make the helper class has access to the**Commands**property of the control template.
- **AttachDecreaseButton**: Let the helper class handles the**Clicked**event of the decrease button in a scroll bar.
- **AttachIncreaseButton**: Let the helper class handles the**Clicked**event of the increase button in a scroll bar.
- **AttachHorizontalScrollHandle**: Let the helper class handle mouse draggings of the handle. This method is only for horizontal scroll bars.
- **AttachVerticalScrollHandle**: Let the helper class handle mouse draggings of the handle. This method is only for vertical scroll bars.
- **AttachHorizontalTrackerHandle**: Let the helper class handle mouse draggings of the handle. This method is only for horizontal trackers.
- **AttachVerticalTrackerHandle**: Let the helper class handle mouse draggings of the handle. This method is only for vertical trackers.

The handle must be a**\<PartialView/\>**composition.

All methods should not be called more than once.**AttachScrollTemplate**must be called in all scroll bars and trackers.

Here is an example of a control template of horizontal scroll bars:
```
<Instance ref.CodeBehind="false" ref.Class="darkskin::HScrollTemplate" ref.Styles="res://DarkSkin/Style">
  <ref.Ctor>
    <![CDATA[
      {
        behavior.AttachScrollTemplate(self);
        behavior.AttachDecreaseButton(buttonDecrease);
        behavior.AttachIncreaseButton(buttonIncrease);
        behavior.AttachHorizontalScrollHandle(handle);
      }
    ]]>
  </ref.Ctor>
  <ScrollTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
    <CommonScrollBehavior ref.Name="behavior"/>

    <SolidBackground Color="#3D3D42"/>
    <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0">
      <SolidBorder ref.Style="controlBorder"/>
    </Bounds>

    <SideAligned MaxLength="20" MaxRatio="0.5" Direction="Left">
      <Button AutoFocus="false" ref.Name="buttonDecrease" ControlTemplate="darkskin::LeftScrollButtonTemplate">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
      </Button>
    </SideAligned>
    <SideAligned MaxLength="20" MaxRatio="0.5" Direction="Right">
      <Button AutoFocus="false" ref.Name="buttonIncrease" ControlTemplate="darkskin::RightScrollButtonTemplate">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
      </Button>
    </SideAligned>
    <Bounds ref.Name="handleContainer" AlignmentToParent="left:20 top:0 right:20 bottom:0" MinSizeLimitation="NoLimit">
      <PartialView ref.Name="handle">
        <att.WidthPageSize-bind>(cast double self.PageSize) / self.TotalSize</att.WidthPageSize-bind>
        <att.WidthRatio-bind>(cast double self.Position) / self.TotalSize</att.WidthRatio-bind>
        <Button AutoFocus="false" ControlTemplate="darkskin::HScrollHandleTemplate">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
        </Button>
      </PartialView>
    </Bounds>
  </ScrollTemplate>
</Instance>
```


