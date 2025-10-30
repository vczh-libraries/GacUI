# GuiListControl and GuiSelectableListControl

**GuiListControl** is the base class of all list controls. This control transforms items in a collection to UI object with a specified layout. When items are scrolled out of the control and becomes invisible, UI objects for them are disposed. Resources are not taken by invisible items.

**GuiSelectableListControl** makes items selectable.

The **ListItemBackground** theme is for item backgrounds, which is a **\<SelectableButtonTemplate/\>**. But it will be overrides by **BackgroundTemplate** in the control template for a list control.

## GuiListControl::Context

This property inherits from **GuiControl**.

All **Context** properties in item template instances will keep sync to **GuiListControl::Context**.

For data grids, **Context** in visualizers and editors will alsp keep sync.

**IMPORTANT**: Even for item template instances, visualizers or editors that are created after **GuiListControl::Context** is set, their **Context** properties are assigned after their constructors.

## GuiListControl::EnsureItemVisible

This function moves the scroll bar to ensure that the specified item is visible.

## Item Events

UI objects dynamically created for items, it is not easy to handling mouse events for items by [events in compositions](../../../.././gacui/components/compositions/events.md). **GuiListControl** redirects all such events to the list control itself: - ItemLeftButtonDown - ItemLeftButtonUp - ItemLeftButtonDoubleClick - ItemMiddleButtonDown - ItemMiddleButtonUp - ItemMiddleButtonDoubleClick - ItemRightButtonDown - ItemRightButtonUp - ItemRightButtonDoubleClick - ItemMouseMove - ItemMouseEnter - ItemMouseLeave**itemIndex** is added to the second argument of handlers for these events starting at **0**.

## GuiListControl Properties

### ItemProvider

This is a readonly property. It reflects how the list control understand data in its items.

This property returns an instance of **list::IItemProvider** which provide access to the text representation of each item, plus a **description::Value** representation for reflection`missing document: /vlppreflection/home.html`.

When reflection is turned off (which is recommended) by using **VCZH_DEBUG_NO_REFLECTION**, you cannot access member via reflection in each item, but you can still do type testing or conversion.

The **RequestView** method is an userful way to extend data representation. This method is usually implemented by each list control because they require more than just text representation. Here are some predefined views for certain list controls: - **\<TextList/\>** and **\<BindableTextList/\>**: **list::ITextItemView** - **\<TreeView/\>** and **\<BindableTreeView/\>**: **tree::INodeItemView** - **\<ListView/\>** and **\<BindableListView/\>**: **list::IListViewItemView** and **list::ListViewColumnItemArranger::IColumnItemView** - **\<BindableDataGrid/\>**: **list::IDataGridView** and all view for list view

### ItemTemplate (ItemTemplateChanged)

Different list controls require different types of item templates. They also provide default item templates and usually require a proper **Arranger** to display them as expected.

When default item templates don't satisfy the need, typically **\<BindableTextList/\>** and **\<BindableDataGrid/\>** is recommended because their item templates are very easy to implement.

To implement the item template for tree views, the **Level** property needs to be carefully taken care of to render the visual effect of the hierarchy.

Item templates for list views are not recommended to implement. For the **Detail** view, it is much more easier to use **\<BindableDataGrid/\>**. For other views, it is much more easier to use **\<BindableTextList/\>** with expected arrangers for these views.

**IMPORTANT**: UI object for each item consists of a background object and a foreground object. The foreground object is created from **ItemTemplate**, but the background object is created from other template properties in the control template. The background object handles the UI effect for selection and mouse operations.

The above behavior is controlled **DisplayItemBackground** property. If this property is set to **false**, background objects will not be created, and foreground objects need to handle UI effect for selection and mouse operations.

Please check out [Item Templates](../../../.././gacui/components/itemplates/home.md) for details.

### Arranger (ArrangerChanged)

Implementing an arranger is not easy, but there are predefined arrangers for common requirements: - **list::FreeHeightItemArranger**: Each item take a row, heights could be different. It is implemented using [<RepeatFreeHeightItem/>](../../../.././gacui/components/compositions/repeat_virtual_freeheight.md). - **list::FixedHeightItemArranger**: Each item take a row with the same height. It is implemented using [<RepeatFixedHeightItem/>](../../../.././gacui/components/compositions/repeat_virtual_fixedheight.md). - **list::FixedSizeMultiColumnItemArranger**: Layout items like a grid, each cell has the same size. It is implemented using [<RepeatFixedHeightMultiColumnItem/>](../../../.././gacui/components/compositions/repeat_virtual_fixedheightmc.md). - **list::FixedHeightMultiColumnItemArranger**: Layout items like a grid, but each column could have a different width. It is implemented using [<RepeatFixedSizeMultiColumnItem/>](../../../.././gacui/components/compositions/repeat_virtual_fixedsizemc.md). - **list::ListViewColumnItemArranger**: Each item take a row, contents are controlled by column headers. It is implemented using [<RepeatFixedHeightItem/>](../../../.././gacui/components/compositions/repeat_virtual_fixedheight.md).**NOTE**: the direction is controlled by **Asix**. For example, by using **DownLeft** instead of **RightDown** or the default axis, **FixedHeightItemArranger** layout items from left to right instead of from top to bottom.

It is recommended to inherited from **[GuiVirtualRepeatCompositionBase](../../../.././gacui/components/compositions/repeat_virtual.md)** when you have to create a new layout. When such new composition is ready, use **VirtualRepeatRangedItemArrangerBase\<T\>** to turn it into an **IItemArranger**.

### Axis (AxisChanged)

**Axis** in **GuiListControl** is very similar to [Axis in <Flow/>](../../../.././gacui/components/compositions/flow.md). For some arrangers like **FixedHeightItemArranger** which displays items in single dimension, **LeftDown** and **RightDown** do no difference. Same for other 3 pairs of values for **AxisDirection**.

### AdoptedSize (AdoptedSizeInvalidated)

This property suggests a best size for the drop down container in a **\<ComboBox/\>**.

### DisplayItemBackground

The default value for this property is **true**.

See **ItemTemplate** for more information.

## GuiSelectableListControl Properties

This control enables list items to be selected by mouse or keyboard operations.

### MultiSelect

The default value for this property is **false**.

Set this property to **true** makes multiple list items could be selected at the same time. **CTRL** with a mouse click or **SHIFT** with direction keys could help select and unselect multiple items.

### SelectedItems (SelectionChanged)

This property is a readonly collection storing indices of selected items staring at **0**.

### SelectedItemIndex (SelectionChanged)

This property returns the index of the selected item.

If no item is selected, or if more than one item is selected, this property returns **-1**.

### SelectedItemText (SelectionChanged)

This property returns the text representation of the selected item.

If no item is selected, or if more than one item is selected, this property returns **""**.

## GuiSelectableListControl Methods

### GetSelected and SetSelected

**GetSelected** tells if an item is selected.

**SetSelected** selects or unselects an item.

### SelectItemsByClick

This method select items by simulate mouse operations.

### SelectItemsByKey

This method select items by simulate keyboard operations.

### ClearSelection

This method clear all selection.

