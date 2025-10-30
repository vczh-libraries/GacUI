# GuiScroll

- **C++/Workflow**: (vl::)presentation::controls::GuiScroll* - **Template Tag**: \<ScrollTemplate/\> - **\<HScroll/\>** - **Template Name**: HScroll - **\<VScroll/\>** - **Template Name**: VScroll - **\<HTracker/\>** - **Template Name**: HTracker - **\<VTracker/\>** - **Template Name**: VTracker - **\<ProgressBar/\>** - **Template Name**: ProgressBar Dragging the handler is not easy to implement and it is very commonly required for **GuiScroll**, so **\<CommonScrollBehavior/\>** is provided for writing such control templates. Please check out [ the default control templates ](https://github.com/vczh-libraries/GacUI/blob/master/Source/Skins/DarkSkin/Template_Scroll.xml) for details.

**HScroll** or **VScroll** have a slider with two buttons for controlling a position.

**HTracker** or **VTracker** have only a slider for controlling a position.

**ProgressBar** only displays a position.

## GuiScroll Properties

### TotalSize (TotalSizeChanged)

The default value for this property is **100**.

This property represents the total size of a logical concept.

### PageSize (PageSizeChanged)

The default value for this property is **10**. For **\<HTracker/\>**, **\<VTracker/\>** and **\<ProgressBar/\>**, this property is set to **0** right after constructors are called.

This property represents the view port size of a logical concept.

In default control templates for **HScroll** and **VScroll**, this property affect the size of the slider, it takes **PageSize/TotalSize** of the space that is not covered by the two buttons.

### Position (PositionChanged)

The default value for this property is **0**.

The property represents the position of the view port of a logical concept.

A legal value must be in **[MinPosition, MaxPosition]**.

### SmallMove (SmallMoveChanged)

The default value for this property is **1**.

This property describes how much the position will be increased or decreased by clicking the two buttons (if exist).

### BigMove (BigMoveChanged)

The default value for this property is **10**.

This property describes how much the position will be increased or decreased by clicking the remaining space of a scroll that is not covered by the two buttons and the slider.

Usually it is recommended to keep **PageSize** and **BigMove** being the same for **\<HScroll/\>** and **\<VScroll/\>**, but it is up to you.

### MinPosition

This property is always **0**.

### MaxPosition

This property is always **TotalSize - PageSize**.

### AutoFocus

The default value for this property is **true**.

When this property is **true**: - The scroll is focused when the mouse button is pressed. - The scroll is focused when it is executed by **TAB** or a **ALT** sequence.

When a scroll is focused, pressing **HOME**, **END**, **PAGE UP**, **PAGE DOWN**, **Left**, **Up**, **Right**, **Down** could change **Position**. In order to correctly transfer the focus to **GuiScroll**, buttons in this control should have **AutoFocus** set to **false**.

