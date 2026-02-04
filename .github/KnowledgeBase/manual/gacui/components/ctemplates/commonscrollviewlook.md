# \<CommonScrollViewLook\>

This is a helper class for implementing any control templates inherit from**\<ScrollViewTemplate/\>**. It inherits from**GuiTemplate**.

This helper class is expected to be used in this way:
```
<Instance ref.CodeBehind="false" ref.Class="darkskin::ScrollViewTemplate" ref.Styles="res://DarkSkin/Style">
  <ScrollViewTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren">
    <att.ContainerComposition-eval>look.ContainerComposition</att.ContainerComposition-eval>
    <att.HorizontalScroll-eval>look.HScroll</att.HorizontalScroll-eval>
    <att.VerticalScroll-eval>look.VScroll</att.VerticalScroll-eval>

    <SolidBackground Color="#252526"/>
    <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0" InternalMargin="left:1 top:1 right:1 bottom:1">
      <SolidBorder Color="#3F3F46"/>
      <CommonScrollViewLook ref.Name="look" AlignmentToParent="left:0 top:0 right:0 bottom:0">
        <att.HScrollTemplate>darkskin::HScrollTemplate</att.HScrollTemplate>
        <att.VScrollTemplate>darkskin::VScrollTemplate</att.VScrollTemplate>
        <att.DefaultScrollSize>20</att.DefaultScrollSize>
      </CommonScrollViewLook>
    </Bounds>
  </ScrollViewTemplate>
</Instance>
```


These properties are expected to pass from**\<CommonScrollViewLook/\>**to**\<ScrollViewTemplate/\>**:
- **ContainerComposition**
- **HorizontalScroll**
- **VerticalScroll**These properties defines control templates for:
- **HScrollTemplate**: Horizontal scroll.
- **VScrollTemplate**: Vertical scroll.These properties defines the thickness of scroll bars:
- **DefaultScrollSize**: Height of the horizontal scroll, and width of the vertical scroll.

After installing this helper class properly, parameters of scroll bars will be taken care of by the control itself. For control templates of list controls, control templates don't need to worry about rendering items, they just need to provide more configurations like colors or control templates of sub objects.

