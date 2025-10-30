# \<GridVisualizerTemplate\>

- **\<GridVisualizerTemplate/\>** - **C++/Workflow**: (vl::)presentation::templates::GuiGridVisualizerTemplate*

**\<GridVisualizerTemplate/\>** derives from **\<ControlTemplate/\>**. It has the following properties additional to its base class:

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

### CellValue

This property is assigned by the list control. This is the cell object for the cell.

This property could be changed especially when this cell is edited.

### Selected

This property is assigned by the list control. When the cell is selected, this property becomes **true**. When the cell is unselected, this property becomes **false**.

Only zero or one cell of an selected row will be selected. No cell will be selected of an unselected row.

## Output Property

### ContainerComposition

This property is assigned by the item template. It is derives from **\<ControlTemplate/\>**.

This property is very important. When an embedded visualizer is put inside this visualizer, it is added to the **ContainerComposition** property of this visualizer.

In this case, if this property is not assigned during construction, it will crash.

## Sample

- Source code: [control_datagrid_visualizer](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_visualizer/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_datagrid_visualizer.gif)

- Source code: [Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_controls_datagrid.gif)

