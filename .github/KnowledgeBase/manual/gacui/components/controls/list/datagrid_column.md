# Column Configuration

Just like**\<ListView/\>**,**\<BindableDataGrid/\>**has**Columns**and**DataColumns**. There is no difference between**DataColumns**in**\<ListView/\>**and**\<BindableDataGrid/\>**.

In**Columns**,**\<ListView/\>**uses**list::ListViewColumn**, but**\<BindableDataGrid/\>**uses**list::DataColumm**. There is not difference between the following properties in**list::ListViewColumn**and**list::DataColumm**:
- **Text**
- **Size**
- **OwnPopup**
- **DropdownPopup**in**list::ListViewColumn**and**Popup**in**list::DataColumm**There is no**SortingState**in**list::DataColumm**, because the sorting state of the column is managed by**\<BindableDataGrid/\>**using**Sorter**in**list::DataColumn**.

## Defining a Column in an Item

**TextProperty (TextPropertyChanged)**defines a readable property in an item. It reads a text from an item for this particular column. This property is used in predefined**Visualizer**for this column. It is also used in other**View**.

**ValueProperty (ValuePropertyChanged)**defines a writable property in an item. It reads and writes the cell object for this particular column. This property is used in user-defined**Visualizer**and**Editor**for this column.

Ideally,**TextProperty**and**ValueProperty**needs to be semantically equal. For example, if**ValueProperty**returns a**DataTime**from an item, but this item doesn't have a property for the text representation for it, then**TextProperty**could use a**Workflow**expression to compute the text representation inline:
```
<BindableDataGrid env.ItemType="MyDataItem^" env.ItemName="itemToBind" ...>
  <att.Columns>
    <DataColumn Text="Birthday" ...>
      <att.ValueProperty>Birthday</att.ValueProperty>
      <att.TextProperty>$'$(itemToBind.Birthday.month)/$(itemToBind.Birthday.day)/$(itemToBind.Birthday.year)'</att.TextProperty>
    </DataColumn>
  </att.Columns>
  ...
</BindableDataGrid>
```


In this example, the**ItemSource**receives a collection of multiple**MyDataItem^**, which is defined in**env.ItemType**.**env.ItemName**defines a name that can be used to define properties, representing an item in**ItemSource**, which is obviously in type**MyDataItem^**.

The**ValueProperty**matches to the**Birthday**property in**MyDataItem**.

The**TextProperty**defines an expression to format the**Birthday**. In fact, you could also define a static function[in the GacUI XML Resource](../../../.././gacui/xmlres/tag_script.md)and use it here to reuse code.

**IMPORTANT**:**Property and Writable Property**is not data binding, so changing**Birthday**in**MyDataItem**doesn't refresh the**\<BindableDataGrid/\>**. Please check out[Updating Items](../../../.././gacui/components/controls/list/bindable.md)for details.

## Accessing Processed Items

The**GetCellText**method is based on**TextProperty**. The**GetCellValue**and**SetCellValue**methods is based on**ValueProperty**.

The**row**property is the position of the item in**\<BindableDataGrid/\>**. It is not the index of item in**ItemSource**.**\<BindableDataGrid/\>**could be applied with multiple**Filter**and a**Sorter**. The displaying item order could be changed, and some items may be filtered out.

These methods operate data based on items in the UI. To get the item itself instead of data defined for columns, use**GetBindingValue**in**ItemProvider**.

## DataColumn Properties

The following properties defines column-based features in a**\<BindableDataGrid/\>**:
- **[Filter](../../../.././gacui/components/controls/list/datagrid_filter.md)**: Assigning a filter to a column. The filter receives the item object instead of the cell object for this column. An item must satisfy all filters (including the**AdditinalFilter**in the control) in order to appear in the control.
- **[Sorter](../../../.././gacui/components/controls/list/datagrid_sorter.md)**Assigning a sorter to a column. The sorter will be activated by clicking the column header, but only one sorter in all columns can be activated.
- **[VisualizerFactory](../../../.././gacui/components/controls/list/datagrid_visualizer.md)**: This property is the creator of visualizers for cells in this column.
- **[EditorFactory](../../../.././gacui/components/controls/list/datagrid_editor.md)**: This property is the creator of editors for cells in this column. The editor will be activated by clicking a cell, but only one editor in all cells can be activated.

