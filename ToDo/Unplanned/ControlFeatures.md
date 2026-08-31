## Control Features

- `IColumnItemView`.
  - Change column size from `int` to `{minSize:int, columnOption:Absolute|Percentage, absolute:int, percentage:float}`
  - Column drag and drop.
    - ListView: only raises an event, developers need to update column headers and data by themselves.
    - DataGrid: swap column object, cells are changed due to binding.
- `GuiBindableDataGrid`:
  - `IDataGridView::GetColumnSpan`.
  - Add customizable row visualizer.
  - The default (or `nullptr`) row visualizer displays cell visualizer and editor.
- `GuiBindableTreeDataGrid`:
  - Offer a default group header row visualizer when users only need one level of collapsable `GuiBindableDataGrid`.
  - Replace the new `list::DataProvider`'s base class with `tree::NodeItemProvider` offering `GuiBindableTreeView::ItemSource` to make a useful data source.
- Touch support.
