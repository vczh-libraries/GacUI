# \<BindableRibbonGalleryList\>

- **\<BindableRibbonGalleryList/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiBindableRibbonGalleryList* - **Template Tag**: \<RibbonGalleryListTemplate/\> - **Item Template Tag**: \<TextListItemTemplate/\> - **Template Name**: RibbonGalleryList - **Gallery List in the control** - **Template Tag**: \<TextListTemplate/\> - **Template Name**: RibbonGalleryItemList - **SubMenu** - **Template Tag**: \<RibbonToolstripMenuTemplate/\> - **Template Name**: RibbonToolstripMenu

**\<BindableRibbonGalleryList/\>** is a huge combo box button displaying a list control and a dropdown menu. When the mouse hovering above an item, it previews the effect of this item. When an item is selected, it applies the effect of this item.

In the dropdown menu, there is a bigger list containing the same content, followed by menu items. Menu items could be visually grouped by inserting **\<RibbonToolstripHeader/\>**.

The list in the dropdown menu is not a list control, it is a **\<RepeatStack/\>** containing multiple **\<RepeatFlow/\>** for each group.

## Control Properties

### ItemTemplate

The default value of this property is empty, but this property must be set, otherwise **\<BindableRibbonGalleryList/\>** renders nothing.

The item template should return a **\<TextListItemTemplate/\>** or one of its derived classes.

### MinCount and MaxCount

The default value of these properties are **0**, but this property must be set, otherwise **\<BindableRibbonGalleryList/\>** shrinks to its smallest size and show no item.

As a ribbon control, **\<BindableRibbonGalleryList/\>** automatically shinks when there is no space. **MinCount** and **MaxCount** define the minimum and maximum size of this control, so that it renders an expected number of items whatever the available space for this control is.

### SelectedIndex and SelectedItem

These properties returns the selected item. When no item is selected, **SelectedIndex** is **-1**, and **SelectedItem** is **null**.

### VisibleItemCount

The default value of this property is **1**.

This property defines the minimum number of items that must be visible in the dropdown list.

### ScrollUpEnabled and ScrollDownEnabled

The default value of this property is **true**.

You don't have to open the dropdown menu to view all contents in the list. **\<BindableRibbonGalleryList/\>** provides a pair of navigation buttons, and these properties control the **Enabled** property of these buttons.

## Item Source Properties

### ItemSource

This property works exactly as [ the ItemSource property ](../../../.././gacui/components/controls/list/bindable.md) in other bindable list controls.

### GroupTitleProperty and GroupChildrenProperty

These are [ readable properties ](../../../.././gacui/xmlres/instance/properties.md). Grouping is enabled only when both of these properties are set.

When grouping is enabled, items in **ItemSource** become group headers. **GroupTitleProperty** defines the titie of each group. **GroupChildrenProperty** defines the collection of items in each group. **ItemTemplate** is created to renderer items in groups. In this case, **env.ItemType** is the type of a group.

When grouping is disabled, **ItemTemplate** is created to renderer items in **ItemSource**. **env.ItemType** is the type of an item.

## Methods

### IndexToGalleryPos and GalleryPosToIndex

These methods convert between index of items and index of items in groups.

**(vl::)presentation::controls::GalleryPos** defines the index of items in groups.

When grouping is enabled, the **group** field is the position of the group in **ItemSource**, and the **item** field is the position of the item in this group.

When grouping is disabled, the **group** field is always **0**, and the **item** field is the position of the item in **ItemSource**.

"Index" defines the position of items in a virtual list, this virtual list merges items of all groups together.

### SelectItem

This method selects an item.

### ApplyItem

This method selects an item, and apply the effect of this item.

## Events

- **SelectionChanged**: Raises when an item is selected by **SelectItem**. - **PreviewStarted**: Raises when the mouse enters an item. - **PreviewStopped**: Raises when the mouse leaves an item. - **ItemApplied**: Raises when an item is selected by **ApplyItem** or by user input.

## Sample

- Source code: [control_ribbon_gallery](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_gallery/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_ribbon_gallery.gif)

