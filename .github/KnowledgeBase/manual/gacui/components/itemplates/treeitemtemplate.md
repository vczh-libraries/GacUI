# \<TreeItemTemplate\>

- **\<TreeItemTemplate/\>** - **C++/Workflow**: (vl::)presentation::templates::GuiTreeItemTemplate*

**\<TreeItemTemplate/\>** derives from **\<ListItemTemplate/\>**. It has the following properties additional to its base class:

## Input Property

### Expanding

This property is assigned by the list control. When the child item list is expanded, this property becomes **true**. When the child item list is collapsed, this property becomes **false**.

### Expandable

This property is assigned by the list control. This property is **true** when there are child items under this item. When child items are added to or removed from this item, this property could change.

### Level

This property is assigned by the list control. For all top level items rendered in the list control (which are child items under the root item), the **Level** is **0**. For all other items, their **Level** is one greater than its parent item.

### Image

This property is assigned by the list control. It is the image of the item.

