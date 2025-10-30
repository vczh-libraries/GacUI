# \<ListItemTemplate\>

- **\<ListItemTemplate/\>** - **C++/Workflow**: (vl::)presentation::templates::GuiListItemTemplate*

**\<ListItemTemplate/\>** derives from **GuiTemplate**. It has the following properties additional to its base class:

## Input Property

### Selected

This property is assigned by the list control. When the item is selected, this property becomes **true**. When the item is unselected, this property becomes **false**.

### Index

This property is assigned by the list control. The number is the position of this item in the list control. The **Index** of the first item is **0**.

This property could be changed when items before this item are inserted to or removed from the list control.

