# \<BindableDataGrid\>


- **\<ComboBox/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiBindableDataGrid*
  - **Template Tag**: \<ListViewTemplate/\>
  - **Template Name**: ListView
  - **Predefined Item Template**:
    - **SetViewToDefault()**(default): list::DefaultDataGridItemTemplate + list::ListViewColumnItemArranger (which has**ColumnButtons**and**ColumnSplitters**properties if control objects of them are needed)

**\<BindableDataGrid/\>**is an extension to the**Detail**view of list controls. But since it uses different item templates, the**View**property will be**Unknown**by default. You can still switch**View**to other views (including**Detail**) and**\<BindableDataGrid/\>**will just look like a list view. To re-enable data grid features, just call**SetViewToDefault**and**View**will become**Unknown**again.

## DataGrid Features

**\<BindableDataGrid/\>**provides the following features:
- `empty list item`
  **[Filtering](../../../.././gacui/components/controls/list/datagrid_filter.md)**: Each column could be attached a**list::IDataFilter**, the data grid itself could be attached a**list::IDataFilter**, all filters will be connected together using**list::DataAndFilter**which works like the C++**&&**operator.
  There are also**list::DataOrFilter**and**list::DataNotFilter**to help making a complex filter from other filters.
- `empty list item`
  **[Sorting](../../../.././gacui/components/controls/list/datagrid_sorter.md)**: Each column could be attached a**list::IDataSorter**. If a column has a sorter, it is sortable and clicking it will sort data ascendingly, descendingly or cancel the sorting.
  To sort by column or cancel sorting by code, you could request the**list::IDataGridView**view from the**ItemProvider**, and there will be**IsColumnSortable**,**SortByColumn**,**GetSortedColumn**and**IsSortOrderAscending**to help.
- `empty list item`
  **[Visualizer](../../../.././gacui/components/controls/list/datagrid_visualizer.md)**: Each column could be attached a visualizer factory. There are predefined visualizers: - MainColumnVisualizerTemplate - SubColumnVisualizerTemplate - HyperlinkVisualizerTemplateand visualizer decorators: - FocusRectangleVisualizerTemplate - CellBorderVisualizerTemplateA visualizer decorator is still a visualizer, but the data grid control will install other visualizer inside its**ContainerComposition**.
  The default visualizer factory for cells in the first column is:**MainColumnVisualizerTemplate;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate**
  The default visualizer factory for cells in the other columns is:**SubColumnVisualizerTemplate;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate**
  It is recommended to use**YOUR_VISUALIZER;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate**so that a cell automatically renders a focus rectangle and a border just like other cells.
- `empty list item`
  **[Editor](../../../.././gacui/components/controls/list/datagrid_editor.md)**: Each column could be attached an editor factory. If a column has an editor, clicking any cell in this column will activate the editor. The**OpenedEditor**property will be useful if you need to access the editor object.

## Tutorial Project

Please check out[this page](../../../.././gacui/xmlres/instance/properties.md)about GacUI XML Resource syntax for properties mentioned above.

Please also checkout[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml)about using**\<BindableDataGrid/\>**features. This project customizes visualizers and editors for different data types, defines sorters for each column, and even implements a data filter in a column drop down.

## Step-by-Step Sample

[control_datagrid_base](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_base/Resource.xml)is a sample for using**\<BindableDataGrid/\>**. It defines a window base class containing a data grid, exposing all**\<DataColumn/\>**for derived window classes to add more configuration.

By specifying**control_datagrid_base;control_datagrid_plaintext**in the[RunResource](https://github.com/vczh-libraries/Release/tree/master/SampleForDoc/GacUI/RunResource)command line argument, you could run this sample without any data grid feature enabled for a baseline.

**control_datagrid_base**is porting from**control_list_listview_bindable**, it is very easy to find all changes:
- **\<BindableListView/\>**is replaced by**\<BindableListView/\>**.
- **\<ListViewColumn/\>**is replaced by**\<DataColumn/\>**, each column is given a**ref.Name**.
- Adding**ValueProperty**for each column.
- The**SelectedItem**property is replaced by**SelectedRowValue**.
- Handling**SelectedCellChanged**instead of**SelectionChanged**.
- The combo box for changing view is removed.It is very straightforward to move your application from list view to data grid.

In**control_datagrid_plaintext**there is almost nothing, it just creates a**sample::MainWindow**inheriting from**sample::DataGridWindowBase**. In order to reference types created in another resource,**GacGenConfig/Metadata**is required in the GacUI XML Resource. Please check out[Code Generation Configuration](../../../.././gacui/xmlres/cgc.md)and[Cross XML Resource References](../../../.././gacui/xmlres/cxrr.md)for details.

In the following feature document pages, different**\<DataColumn/\>**features will be demonstrated by creating different derived window class to add configuration to columns.


- Source code:[control_datagrid_plaintext](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_plaintext/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_datagrid_plaintext.gif)

