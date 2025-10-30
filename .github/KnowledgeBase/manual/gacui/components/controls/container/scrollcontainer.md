# GuiScrollView and \<ScrollContainer\>

- **\<ScrollContainer/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiScrollContainer* - **Template Tag**: \<ScrollViewTemplate/\> - **Template Name**: ScrollView Making scroll bars dynamically visible and invisible and resizing the content area could be done using **\<Table/\>**. But such things are commonly required by container controls, list controls and editor controls. **\<CommonScrollViewLook/\>** is prepared for controlling scroll bars to help implementing control templates easier.

**GuiScrollView** is the base class of all controls that with a **\<HScroll/\>** and a **\<VScroll/\>**. **\<ScrollView/\>** is a ready-to-use container control that automatically shows scroll bars when content takes more spaces than the container itself.

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

- Source code: [control_container_scrollcontainer](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_container_scrollcontainer/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_container_scrollcontainer.gif)

