# List Controls

List controls organize multiple similar items in a particular order.

## Overview of Classes

**GuiListControl** is the base class of all list controls. **\<ComboBox/\>**, which is a virtual control of  **GuiComboBoxListControl**, is a drop down control with a list control in the drop down.

**GuiListControl** is a **GuiScrollView**.

The **ItemTemplate** property to control how an item looks like. All list controls have its own default **ItemTemplate**.

The **Arranger** and the **Axis** property controls how items are organized.

Each visible item has its own instance of **ItemTemplate**. When an item becomes invisible (typically because it is scrolled out of the view port), its item template instance will be deleted. When an item becomes visible again, a new item template will be created for this item.

Items in a list control are not required to use the same item template class, this is very easy to customize for bindable version of list controls.

All mentioned types in **(vl::)presentation::controls::** namespace are without a full name.

### Text List

**GuiVirtualTextList** is the base class of all list controls displaying items as texts. Items could be optionally checkable. This control doesn't maintain items, instead it requires an **list::IItemProvider** when created, which must support a **list::ITextItemView** view.

**\<TextList/\>** is a **GuiVirtualTextList** that maintains items in its **Items** property. This property an **observe (list::TextItem^) []**.

**\<BindableTextList/\>** is a **GuiVirtualTextList** that display all items from a given item source object. Each item is required to inherit from a specified interface or class. If the provided item source object is an [ observable collection ](../../../.././workflow/lang/type.md), the list control will automatically updates itself when items are added to or removed from this item source object.

### Tree View

**GuiVirtualTreeListControl** is the base class of all list controls displaying cascading items. Items are expandable if they have child items. This control doesn't maintain items, instead it requires an **INodeRootProvider** when created. It will create an item provider which supports the **tree::INodeItemView** view.

The root node is invisible, the control displays child nodes of the root node as visible root nodes.

**GuiVirtualTreeView** is a **GuiVirtualTreeListControl** for all items that has an icon and a title. It requires the root provider to support a **ITreeViewItemView** view.

**\<TreeView/\>** is a **GuiVirtualTreeView** that maintains items in its **Nodes** property. All direct or indirect child nodes in **Nodes** must be a **tree::MemoryNodeProvider^**, whose **Data** must be a **tree::TreeViewItem^**

**\<BindableTreeView/\>** is a **GuiVirtualTreeView** that display all items from a given item source object. Each item is required to inherit from a specified interface or class. If the provided item source object is an [ observable collection ](../../../.././workflow/lang/type.md), the list control will automatically updates itself when items are added to or removed from this item source object. The base type of items is required to have a property for getting child nodes, this property could also be an observable collection.

### List View

**GuiVirtualListView** is the base class of all list controls displaying items as texts with icons. This control doesn't maintain items, instead it requires an **list::IItemProvider** when created, which must support a **list::IListViewItemView** and a **list::ListViewColumnItemArranger::IColumnItemView** view.

A list view control could organizes items in different way, one of them is to display items in a table with a column header.

**\<ListView/\>** is a **GuiVirtualListView** that maintains items in its **Items** property. This property an **observe (list::ListViewItem^) []**.

**\<BindableListView/\>** is a **GuiVirtualListView** that display all items from a given item source object. Each item is required to inherit from a specified interface or class. If the provided item source object is an [ observable collection ](../../../.././workflow/lang/type.md), the list control will automatically updates itself when items are added to or removed from this item source object.

### Data Grid

**GuiVirtualDataGrid** is the base class of all list controls displaying items as a table. This control doesn't maintain items, instead it requires an **list::IItemProvider** when created, which must support a **list::IDataGridView** view and all other views required by **GuiVirtualListView**.

A data grid is a list view control with powerful features when the list view control is rendering a table. A data grid provide customizable visualizing and editing experience for each cell in the table. It also provide basic filtering and sorting on items.

**\<BindableDataGrid/\>** is a **GuiVirtualDataGrid** that display all items from a given item source object. Each item is required to inherit from a specified interface or class. If the provided item source object is an [ observable collection ](../../../.././workflow/lang/type.md), the list control will automatically updates itself when items are added to or removed from this item source object. Data in different columns in an item could be connected to different properties in the item.

## Rendering issues about adding items

Adding items could cause flashing during rendering for about 1 frame. To workaround this issue, call **GuiControlHost::ForceCalculateSizeImmediately** after finishing editing all items in a rendering cycle, in either C++ or Workflow. A control host is a top level window like window or menu. You could also call the same function in `ContainerComposition` of any list controls, but this function is only available to C++.

