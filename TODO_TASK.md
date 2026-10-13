In `EasyLayoutResponsiveWindow.xml` there is a trick

```xml
<ResponsiveGroup Direction="Horizontal">
  <!-- Forward both navigation widths across the scroll viewport. -->
  <ResponsiveView ref.Name="accountWidth" Direction="Horizontal">
    <att.Views>
      <ResponsiveFixed PreferredMinSize="x:230"/>
      <ResponsiveFixed PreferredMinSize="x:64"/>
    </att.Views>
  </ResponsiveView>
  <BindableTextList ref.Name="accountList" Text="Accounts" HorizontalAlwaysVisible="false" VerticalAlwaysVisiblItemType="demo::EasyResponsiveAccount^">
    ...
  </BindableTextList>
</ResponsiveGroup>
```

to configure a responsive group. Maybe this could be implemented as a feature into responsive group to be an additional layer of triggering, instead of just relying on the minimum size.
We could assign a size (int) array optionally, and the responsive group update its own `PreferredMinSize`.
The corresponding component of `PreferredMinSize` according to `Direction` will be overrided, no matter how the user specify the value.
When the size array has more elements than levels, extra numbers will be ignored.
When the size array has less elements than levels, the array will not be physically changed but any extra level will have `PreferredMinSize` increased by 1 then the previous one.
The above rules should implicitly infer that: when the size array is empty, its own `PreferredMinSize` will not be touched.

I would expect the syntax like this:

```xml
<ResponsiveGroup Direction="Horizontal" ViewSizes="{230 64}">
  <!-- or alternatively, there are two samples in the easy layout responsive window, use both syntax -->
  <!-- In this example, horizontal means x, view sizes mean y with vertical -->
  <att.ViewSizes>
    <_>230</_>
    <_>64</_>
  </att.ViewSizes>
  <BindableTextList ref.Name="accountList" Text="Accounts" HorizontalAlwaysVisible="false" VerticalAlwaysVisiblItemType="demo::EasyResponsiveAccount^">
    ...
  </BindableTextList>
</ResponsiveGroup>
```

In `TestCompositions_Responsive.cpp` add 4 test cases to cover 0, less, equal, more size array elements than levels, each one covers horizontal and vertical cases separatedly, no need to add other unit tests:
- Resize the root composition to exactly preferred size and 1 number less than the preferred size to make sure it does not change and change when shrinking, all the way from the largest to the smallest level.
- Growing should be tested.
- In the 0 and less scenario, actual min size should larger than calculated preferred min size, that's how unspecified sizes are not effecting the layout determining, in this case the testing boundary will be actual min sizes instead of calculated preferred min size, when covering levels that do not have view size specified.

Commit and push once finishing, and then update the KB pages in `Tools` repo, and website to reflect the new array size feature of responsive group, publish the website.
