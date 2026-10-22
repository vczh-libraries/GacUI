# GuiScrollView and \<ScrollContainer\>

- **\<ScrollContainer/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiScrollContainer*
  - **Template Tag**: \<ScrollViewTemplate/\>
  - **Template Name**: ScrollView Making scroll bars dynamically visible and invisible and resizing the content area could be done using **\<Table/\>**. But such things are commonly required by container controls, list controls and editor controls. **\<CommonScrollViewLook/\>** is prepared for controlling scroll bars to help implementing control templates easier.

**GuiScrollView** is the base class of all controls that with a **\<HScroll/\>** and a **\<VScroll/\>**. **\<ScrollView/\>** is a ready-to-use container control that automatically shows scroll bars when content takes more spaces than the container itself.

## Touch Scrolling

`GuiScrollView` enables `AcceptPointEvents` by default and consumes the built-in `GuiPointGesture_Scrolling` gesture. This behavior is inherited by scroll containers, list controls and other scroll-view subclasses. Windows and GacJS support touch input; the native wGac and iGac providers do not. GacJS browser touch remains supported on Linux and macOS. Set `AcceptPointEvents` to `false` to disable this control's default acceptance; custom Started handlers and ancestor controls can still accept the sequence.

Single-contact movement beyond a radius of eight logical units locks the gesture to its dominant axis, with horizontal winning a tie. A scroll view consumes an unhandled scrolling offer only when it owns the sequence at its bounds, is visually enabled, and the scrollbar for the locked axis exists, is visible and is visually enabled. It subtracts the signed movement from the current view coordinate through the normal range-clamped scrolling API, so content follows the contact and reversing direction responds immediately at a boundary.

The default scroll view does not consume `GuiPointGesture_DraggingAndRotation`: free two-dimensional translation, two-contact panning, pinch-to-zoom and rotation require a custom consumer. It provides no inertia or overscroll effect. A second contact stops scrolling offers for the rest of the sequence, even after that contact lifts. An unavailable axis does not switch to the other axis. Neither an unavailable axis nor reaching the scroll limit transfers the sequence to a parent scroll view.

The nearest accepting child owns the entire sequence; point gestures do not bubble. Enabled selectable or editable document text marks the Started query handled without accepting it, preventing default ancestor scroll-view acceptance and allowing immediate mouse-compatible text selection. View-only text and scrollbar or surrounding control chrome retain scrolling behavior. A custom Started handler can still explicitly accept a handled query.

An accepted single-contact tap can still activate a control through the automatic click fallback; an eligible stationary hold produces a right click on successful release. A selected scrolling gesture does not click on release. See [Touch and Built-in Gestures](../../../.././gacui/components/compositions/events.md) for acceptance, capture, cancellation, gesture payloads and custom manipulation.

## GuiScrollView Properties

### ViewSize

This property represents the size of the visible part.

### ViewPosition

This property represents the position of the visible part. Components of **ViewPosition** are non-negative. When users scroll towards down or right, the position becomes larger.

This property could be assigned to change the position, which could cause scroll sliders move automatically.

### ViewBounds

This property is the combination of **ViewPosition** and **ViewSize**.

### HorizontalScroll and VerticalScroll

These properties return the **\<HScroll/\>** and the **\<VScroll/\>** of the container.

### HorizontalAlwaysVisible

The default value for this property is **true**.

When this property is **false**, the **\<HScroll/\>** is not visible when the width of the container is large enough to display the content.

### VerticalAlwaysVisible

The default value for this property is **true**.

When this property is **false**, the **\<VScroll/\>** is not visible when the height of the container is large enough to display the content.

## \<ScrollContainer\> Properties

### ExtendToFullWidth or ExtendToFullHeight

The default value for these properties is **false**.

When **ExtendToFullWidth** is **false**, **ContainerComposition** shinks in width to contain its content. When **ExtendToFullWidth** is **true**, width of **ContainerComposition** extends to the **ViewSize** when the total width of its content doesn't exceed the width of the control.

This feature is very useful when you want to create a layout with limited width but unlimited height. Just set **ExtendToFullWidth** to **true** and **HorizontalAlwaysVisible** to **false**, with correct compositions to make content fills the whole space in width but has a very small minimum width (e.g. fill things in a **\<Flow/\>**, or multiple **\<Flow/\>** in a vertical **\<Stack/\>**). When **\<ScrollContainer\>** shrinks in width, its content reorganizes to shrink in width but grow in height.

**ExtendToFullHeight** works in the same way.

## Sample

- Source code: [control_container_scrollcontainer](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_container_scrollcontainer/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_container_scrollcontainer.gif)

