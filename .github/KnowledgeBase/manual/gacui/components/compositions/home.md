# Compositions

[Composition](../../.././gacui/kb/compositions.md)offer layout algorithms based on constraints. Usually, developers using GacUI places nested controls and compositions in a window, by defining constraints between compositions, the window knows its minimum size, and how to change sizes of child controls when the window is resizing.

There are a lot of predefined compositions as follows, each defines a kind of constraint:
- GuiGraphicsComposition
  - GuiWindowComposition
  - \<Bounds\>
    - \<Stack\>
      - \<RepeatStack\>
    - \<Flow\>
      - \<RepeatFlow\>
    - \<Table\>
    - \<SharedSizeRoot\>
    - \<SharedSizeItem\>
    - GuiResponsiveCompositionBase
      - \<ResponsiveView\>
      - \<ResponsiveFixed\>
      - \<ResponsiveStack\>
      - \<ResponsiveGroup\>
    - \<ResponsiveShared\>
    - \<ResponsiveContainer\>
  - \<StackItem\>
  - \<FlowItem\>
  - \<Cell\>
  - GuiTableSplitterComposition
    - \<RowSplitter\>
    - \<ColumnSplitter\>
  - \<SideAligned\>
  - \<PartialView\>
  - GuiVirtualRepeatCompositionBase
    - \<RepeatFreeHeightItem\>
    - \<RepeatFixedHeightItem\>
    - \<RepeatFixedSizeMultiColumnItem\>
    - \<RepeatFixedHeightMultiColumnItem\>

Here we introduce common constraints and configuration of all compositions first.

## TransparentToMouse property

The default value is**false**.

If it is set to**true**, this composition becomes transparent to the mouse. When a mouse clicks on this composition, the click just go through this composition and hit a non-transparent one under this composition.

**Note**: by setting it to**true**, it doesn't makes child compositions become transparent to the mouse. If you want a whole composition tree to be transparent to the mouse, every single composition in this tree must have this property set to**true**.

## AssociatedCursor property

This property defines how the cursor should look like when the cursor is over this composition.

If this property is not set, it will ask its parent composition for a value.

In**GacUI XML Resource**, the type of this property is**presentation::INativeCursor::SystemCursorType**. The value could be any item of this enum.

## AssociatedHitTestResult property

The property defines how the window should behave when the mouse is interacting with this composition.

The type of this property is**presentation::INativeWindowListener::HitTestResult**. The value could be any item of this enum.

Usually this property is used when implementing a control template for**\<Window/\>**. For example, the window template will have a close button and**ButtonClose**is set to the**AssociatedHitTestResult**property of this button's**BoundsComposition**. When the mouse clicks this button, the OS will know that this button is for closing the window, and then raise a sequence of events and close the window if it is not interrupted. No code is needed for the**Clicked**event of this button.

## Visible property

The default value is**true**.

If it is set to**false**, anything inside this composition will not be rendered,**but they still affect the sizing**as they are visible.

## Box Model

![](https://gaclib.net/doc/gacui/com_BoxModel.png)

**DO NOT**control layout by code,**DO**control layout by assigning proper values to properties mentioned below. the layout will be fixed as soon as possible after properties of elements are changed, and it is fast enough, but the timing is not guaranteed.

Any attempts to read the position of size of composition by programming could lead to unexpected result.

### Minimum size of the composition

The minimum size is controlled by the following properties:
- **PreferredMinSize**
- **OwnedElement**when**MinSizeLimitation**is**LimitToElement**or**LimitToElementAndChildren**
- **Children**when**MinSizeLimitation**is**LimitToElementAndChildren**And also by the following properties that only exist in**GuiBoundsComposition**and its derived classes:
- **ExpectedBounds**'s size
- Children's**AlignmentToParent**when**MinSizeLimitation**is**LimitToElementAndChildren**

A composition has a default minimum size to 0.

By assigning an element to the**OwnedElement**property, the element is binded to this composition. The element is rendered using the composition's position and size.

Sometimes the element requires a minimum size to render its full content (typically for**SolidLabel**without enabling trailing ellipses). To assign the minimum size to this composition, simply just set**MinSizeLimitation**to**LimitToElement**or**LimitToElementAndChildren**.

If this composition has children, there will also be a minimum size to render all children. To assign the minimum size to this composition, simply just set**MinSizeLimitation**to**LimitToElementAndChildren**.

You can also assign a minimum size to this composition using the**PreferredMinSize**property.

### Position of the composition

The position is controlled by the following properties:
- **AlignmentToParent**, defaults to (-1,-1,-1,-1). Any component of**AlignmentToParent**must be -1 or non-negative.**AlignmentToParent**is the gap between this composition and its parent compositon.
- **ExpectedBounds**, defaults to (0,0,0,0).
- **InternalMargin**of the parent composition, defaults to (0,0,0,0). Any component of**InternalMargin**must be non-negative.**InternalMargin**adds a gap between this component and its children compositions. A component of**InternalMargin**is considered when the corresponding component of**AlignmentToParent**of a child composition is**-1**.

Considering**left**and**right**of**AlignmentToParent**.
- If both are not -1, the gap between this composition and its parent is defined by**AlignmentToParent**.
- If**left**is -1 and**right**is not, the right gap is defined by**AlignmentToParent**, the left gap is decided by the component's width.
- If**left**is not -1 and**right**is not, the left gap is defined by**AlignmentToParent**, the right gap is decided by the component's width.
- If both are -1, the left gap is defined by**ExpectedBounds**combining with its parent's**InternalMargin**, the right gap is decided by the component's width.**top**,**bottom**and height are similar to what is described above.

**AlignmentToParent**also affects the minimum size of the parent composition if**MinSizeLimitation**of the parent composition is set to**LimitToElementAndChildren**.

**NOTE**: some compositions like**\<SideAligned/\>**or**\<PartialView/\>**ignores**AlignmentToParent**.

### Keeping a button at the right-bottom corner of the window

This is very straightforward:
```
<Button>
  <att.BoundsComposition AlignmentToParent="left:-1 top:-1 right:5 bottom:5"/>
</Button>
```


By doing this, the button will keep its right 5 pixel (96 dpi) away from its parent's right, also its bottom 5 pixel (96 dpi) away from its parent's bottom. When the size of its parent changes, the button will stick to the right-bottom corner.

**NOTE**: you are recommended to use**\<Table/\>**when you can, since the size of the button could change because it has different text under different OS language configuration.**\<Table/\>**helps to organize the button and the content above the button, or when there are multiple button on the right-bottom size.

## Data Binding on Positioning Properties

Just**DON'T DO THIS**.

**IT IS DANGEROUS**to bind one positining properties to another. For example, you may want to keep a composition takes exactly the left half of its parent composition. But if you bind**AlignmentToParent**of this composition to its size, it could results in keeping growing its parent composition.

When the parent composition grows (e.g. dragging the window), the child composition also grows because it wants to keep the gap exactly to**AlignmentToParent**. After that**AlignmentToParent**also grows because the child composition grows, and it makes the parent composition to grow again because the gap increases. This process never stops, the window will keep growing when it is resizing, until forever.

## Other Properties

### AssociatedControl

This property returns the control, whose**BoundsComposition**is this composition.

This property could return**null**when this composition is not a**BoundsComposition**of a control.

### RelatedControl

This property returns the most inner control that contains this composition.

This property could return**null**when this composition has not been put in a control yet.

### RelatedControlHost

This property returns the window that contains this composition.

This property could return**null**when this composition has not been put in a control yet. Or the control has not been put in a window yet.

## The timing of layout calculation

GacUI will trigger layout calculation automatically, once on each frame until calculation becomes table, and it stops.

It can also be triggered by calling the**ForceCalculateSizeImmediately**method on a composition you want to process. It calculates the layout of the composition and all its direct and indirect child compositions.

Only when the calculation is completed, the following properties of a composition are updated:
- **CachedMinSize**, triggering the**CachedMinSizeChanged**property.
- **CachedMinClientSize**, triggering the**CachedMinSizeChanged**property.
- **CachedBounds**, triggering the**CachedBoundsChanged**property.
- **CachedClientArea**, triggering the**CachedBoundsChanged**property.

### ForceCalculateSizeImmediately Method

This function makes this property refresh its size based on constraints immediately. But if elements in this composition have not been rendered yet, or have not been rendered since some properties of elements are changed, this function could return an inaccurate result. This depends on the implementation of element renderers.

### CachedMinSize property

The minimum size of a composition.

### CachedMinClientSize property

The minimum client size of a composition, which is**CachedMinSize**but the area of**InternalMargin**is excluded.

### CachedBounds property

The position and actual size of a composition in its parent's coordinate space.

### CachedClientArea property

The position and actual size of a composition's client area in its parent's coordinate space, which is**CachedBounds**but the area of**InternalMargin**is excluded.

