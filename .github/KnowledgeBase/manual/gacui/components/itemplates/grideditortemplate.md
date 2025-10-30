# \<GridEditorTemplate\>

- **\<GridEditorTemplate/\>** - **C++/Workflow**: (vl::)presentation::templates::GuiGridEditorTemplate*

**\<GridEditorTemplate/\>** derives from **\<ControlTemplate/\>**. It has the following properties additional to its base class:

## Input Property

### PrimaryTextColor

This property is assigned by the list control. It comes from the **PrimaryTextColor** property of **\<ListViewTemplate/\>** that applied to the list control.

The list control does not track the changing of the **PrimaryTextColor** property of **\<ListViewTemplate/\>**. If it is changed, **PrimaryTextColor** of item templates are not changed automatically. But when a list control is assigned with a new control template, **PrimaryTextColor** of item templates will be updated as well.

### SecondaryTextColor

This property is assigned by the list control. It comes from the **SecondaryTextColor** property of **\<ListViewTemplate/\>** that applied to the list control.

The list control does not track the changing of the **SecondaryTextColor** property of **\<ListViewTemplate/\>**. If it is changed, **SecondaryTextColor** of item templates are not changed automatically. But when a list control is assigned with a new control template, **SecondaryTextColor** of item templates will be updated as well.

### ItemSeparatorColor

This property is assigned by the list control. It comes from the **ItemSeparatorColor** property of **\<ListViewTemplate/\>** that applied to the list control.

The list control does not track the changing of the **ItemSeparatorColor** property of **\<ListViewTemplate/\>**. If it is changed, **ItemSeparatorColor** of item templates are not changed automatically. But when a list control is assigned with a new control template, **ItemSeparatorColor** of item templates will be updated as well.

### LargeImage

This property is assigned by the list control. It is the large image of the item.

### SmallImage

This property is assigned by the list control. It is the small image of the item.

### RowValue

This property is assigned by the list control. This is the row object for the item.

## Output Property

### CellValueSaved

This property is assigned by the item template. The default value is **true**. When a cell is being editing but the value has not been updated to the **CellValue** property, update this property to **false**. When a new value is updated to the **CellValue** property, update this property to **true**.

### FocusControl

This property is assigned by the item template. The default value is **null**.

When an editor is opened for a cell, if **FocusControl** is not **null**, **FocusControl** will receive the focus.

## Exchange Property

### CellValue

This property is assigned by the list control and the item template. This is the cell object for the cell.

When an editor believes a new value is ready for this cell, update this property with the new value. For example, an editor with a text box could update this property whenever the text in the text box is changed.

## Sample

- Source code: [control_datagrid_editor](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_editor/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_datagrid_editor.gif)

- Source code: [Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_controls_datagrid.gif)

