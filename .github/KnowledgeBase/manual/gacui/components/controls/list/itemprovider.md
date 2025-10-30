# Accessing Items via IItemProvider

List controls provide programmatic access to their items through provider interfaces. These interfaces allow you to query, navigate, and manipulate items in different ways depending on the control type.

## Interfaces Required by Controls

Different list controls expose different provider interfaces for accessing their items. Use **dynamic_cast** with **RequestView** to obtain specific view interfaces.

### GuiListControl

**GuiListControl** provides basic item access through **IItemProvider**: - **GetItemProvider()**: Returns the **IItemProvider** interface - **Count()**: Get the number of items - **GetTextValue(itemIndex)**: Get the text representation of an item - **GetBindingValue(itemIndex)**: Get the boxed value of an item

To access more detailed view interfaces:

**dynamic_cast\<IMyView*\>(l.GetItemProvider()-\>RequestView(WString::Unmanaged(IMyView::Identifier)))**

This allows accessing items in their rendering order.

### GuiVirtualTextList

**GuiVirtualTextList** requires the item provider to implement **list::ITextItemView**: - **GetChecked(itemIndex)**: Get the checked state of an item - **SetChecked(itemIndex, bool)**: Set the checked state of an item

### GuiVirtualListView

**GuiVirtualListView** requires the item provider to implement **list::IListViewItemView** for accessing list view item data: - **GetSmallImage(itemIndex)**: Get the small icon for an item - **GetLargeImage(itemIndex)**: Get the large icon for an item - **GetText(itemIndex)**: Get the text of an item - **GetSubItem(itemIndex, subItemIndex)**: Access sub-item data - **GetDataColumnCount()**: Get the number of data columns - **GetDataColumn(index)**: Access a user-defined projection of text and sub items - **GetColumnCount()**: Get the number of column headers - **GetColumnText(index)**: Access column header text

The **ListViewColumnItemArranger::IColumnItemView** interface provides column-specific operations: - **GetColumnSize(index)**: Get column header width - **SetColumnSize(index, value)**: Set column header width - **GetDropdownPopup(index)**: Access the dropdown menu attached to a column header - **GetSortingState(index)**: Access the sorting state of a column header (rendering effect only, does not reflect actual item order)

### GuiVirtualTreeListControl

**GuiVirtualTreeListControl** requires the item provider to implement **tree::INodeItemView** (also accessible via **GetNodeItemView()**): - **RequestNode(index)**: Convert item index to **Ptr\<INodeProvider\>** - **CalculateNodeVisibilityIndex(node)**: Convert **INodeProvider*** to index (returns **-1** if collapsed by any ancestor)

**GetNodeRootProvider()** returns **tree::INodeRootProvider**: - **GetRootNode()**: Get the root node (root node is not rendered, its children are the top-level nodes) - **GetTextValue(node)**: Get the text representation of a node - **GetBindingValue(node)**: Get the boxed value of a node

To access more detailed view interfaces:

**dynamic_cast\<IMyView*\>(l.GetNodeRootProvider()-\>RequestView(WString::Unmanaged(IMyView::Identifier)))**

This allows accessing nodes in their logical tree structure.

### GuiVirtualTreeView

**GuiVirtualTreeView** requires the node provider to implement **tree::ITreeViewItemView**: - **GetNodeImage(node)**: Access the icon for a tree view node

### GuiVirtualDataGrid

**GuiVirtualDataGrid** requires the item provider to implement **list::IDataGridView** for cell-level access: - **GetBindingCellValue(row, column)**: Access cell data - **SetBindingCellValue(row, column, value)**: Set cell data (must be writable if an editor factory is assigned for the column) - **GetCellSpan()**: No effect (merging cells is not supported yet)

**IMPORTANT**: The item provider of the data grid represents items in rendering order, which means it only contains items that passed all filters, sorted by the assigned sorter.

## Predefined IItemProvider or INodeRootProvider

Each list and tree control uses a specific provider implementation:

**List Controls and View Implementations:** - **GuiTextList**: **list::TextItemProvider** (BindingValue: **Ptr\<TextItem\>**) - **GuiListView**: **list::ListViewItemProvider** (BindingValue: **Ptr\<ListViewItem\>**) - **GuiBindableTextList**: **list::TextItemBindableProvider** (BindingValue: **description::Value** in ItemSource) - **GuiBindableListView**: **list::ListViewItemBindableProvider** (BindingValue: **description::Value** in ItemSource) - **GuiVirtualTreeListControl**: **tree::NodeItemProvider** - **GuiBindableDataGrid**: **list::DataProvider**

**Tree Controls and View Implementations:** - **GuiTreeView**: **tree::TreeViewItemRootProvider** (BindingValue: **Ptr\<TreeViewItem\>**) - **GuiBindableTreeView**: **tree::TreeViewItemBindableRootProvider** (BindingValue: **description::Value** in ItemSource)

## Manipulating Items

Different controls provide different ways to manipulate their items programmatically.

### GuiTextList

**GuiTextList** provides direct access to its items: - **GetItems()**: Returns a writable list of **Ptr\<TextItem\>** - **GetSelectedItem()**: Returns the selected **Ptr\<TextItem\>** (null if multiple or none selected)

### GuiListView

**GuiListView** provides access to columns and items: - **GetDataColumns()**: Returns a writable list of data column indices - **GetColumns()**: Returns a writable list of **Ptr\<ListViewColumn\>** - **GetItems()**: Returns a writable list of **Ptr\<ListViewItem\>** - **GetSelectedItem()**: Returns the selected **Ptr\<ListViewItem\>** (null if multiple or none selected)

### GuiTreeView

**GuiTreeView** provides tree-based access through **Nodes()**: - **GetRootNode()**: Returns **Ptr\<INodeProvider\>** of the root node - **GetMemoryNode(node)**: Converts **INodeProvider*** to **MemoryNodeProvider***

**MemoryNodeProvider** provides: - **Children()**: A writable list of **Ptr\<MemoryNodeProvider\>** - **GetData()**: Access the actual data of a node (must be **Ptr\<TreeViewItem\>**) - **SetData(value)**: Set the data of a node

### GuiBindableTextList

**GuiBindableTextList** works with data binding: - **GetItemSource()**: Get the enumerable object representing all items - **SetItemSource(value)**: Set the item source (if it implements **IValueReadonlyList**, the implementation will be more efficient; if it implements **IValueObservableList**, changing the list will update the list control automatically) - **GetSelectedItem()**: Returns the selected **description::Value** (null if multiple or none selected) - **NotifyItemDataModified(start, count)**: Call this function if properties inside any item are changed when there is no update to the item source

### GuiBindableListView

**GuiBindableListView** combines columns with data binding: - **GetDataColumns()**: Returns a writable list of data column indices - **GetColumns()**: Returns a writable list of **Ptr\<ListViewColumn\>** - **GetItemSource()**: Get the enumerable object representing all items - **SetItemSource(value)**: Set the item source (if it implements **IValueReadonlyList**, the implementation will be more efficient; if it implements **IValueObservableList**, changing the list will update the list control automatically) - **GetSelectedItem()**: Returns the selected **description::Value** (null if multiple or none selected) - **NotifyItemDataModified(start, count)**: Call this function if properties inside any item are changed when there is no update to the item source

### GuiBindableTreeView

**GuiBindableTreeView** accepts an optional **reverseMappingProperty** constructor argument, which represents a property of the node data in the item source for accessing **Ptr\<INodeProvider\>** for that node.

Available methods: - **GetItemSource()**: Get the object for the root node - **SetItemSource(value)**: Set the root node object (if its Children property implements **IValueReadonlyList**, the implementation will be more efficient; if its Children property implements **IValueObservableList**, changing the list will update the tree view automatically) - **GetSelectedItem()**: Returns the selected **description::Value** (null if multiple or none selected) - **NotifyNodeDataModified(value)**: Call this function if properties inside any node data are changed (this method requires **reverseMappingProperty** to be specified; there is no way to notify children changes if its Children property does not implement **IValueObservableList**)

### GuiBindableDataGrid

**GuiBindableDataGrid** provides cell-based selection with data binding: - **GetItemSource()**: Get the enumerable object representing all rows - **SetItemSource(value)**: Set the row data (if it implements **IValueReadonlyList**, the implementation will be more efficient; if it implements **IValueObservableList**, changing the list will update the data grid automatically) - **GetSelectedCell()**: Returns the selected **description::Value** for the cell - **SelectCell(value, openEditor)**: Select a cell, and open its editor if **openEditor** is **true**

