# \<ListView\>


- **\<ListView/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiListView*
  - **Template Tag**: \<ListViewTemplate/\>
  - **Item Template Tag**: \<ListItemTemplate/\>
  - **Template Name**: ListView
  - **Predefined Item Template**:
    - **SetView(Detail)**(default): list::DetailListViewItemTemplate + list::ListViewColumnItemArranger (which has**ColumnButtons**and**ColumnSplitters**properties if control objects of them are needed)
    - **SetView(BigIcon)**: list::BigIconListViewItemTemplate + list::FixedSizeMultiColumnItemArranger
    - **SetView(SmallIcon)**: list::SmallIconListViewItemTemplate + list::FixedSizeMultiColumnItemArranger
    - **SetView(List)**: list::ListListViewItemTemplate + list::FixedHeightMultiColumnItemArranger
    - **SetView(Tile)**: list::TileListViewItemTemplate + list::FixedSizeMultiColumnItemArranger
    - **SetView(Information)**: list::InformationListViewItemTemplate + list::FixedHeightItemArranger

## Items

**\<ListView/\>**stores a list of**list::ListViewItem**and display them. In**list::ListViewItem**there are**Text**,**SmallImage**,**LargeImage**,**Tag**and**SubItems**properties, changing these properties also immediately causes changing in the list control.

You can add, remove or change items in the**Items**property to change content in the list control.

The**SelectedItem**property returns the currently selected item in**list::ListViewItem**. If no item is selected, or multiple items are selected, this property returns**null**.

## Views

The**View**property defines all predefined view in**\<ListView/\>**. Valid values are listed at the beginning of this page. You could ignore this property and change**ItemTemplate**and**Arranger**by assigning new values to these properties or calling the protected method**SetStyleAndArranger**to set them at the same time. Doing this makes**View**become**Unknown**.

The**Detail**view renders a table, column headers are defined by**list::ListViewColumn**objects in the**Columns**property. Changing**Columns**immediately causes the list control to display different columns. In each row, the first column is always**Text**in**list::ListViewItem**, other columns are stored in**SubItems[0]**,**SubItems[1]**,**SubItems[2]**...

## Columns

In**list::ListViewColumn**there are**Text**,**Size**,**OwnPopup**,**DropdownPopup**and**SortingState**properties, changing these properties also immediately causes changing in the list control headers:
- **Text**: The column header.
- **Size**: The size of the column header. Column sizes could also be changed by dragging them by mouse.
- **OwnPopup**: The default value is**true**. When the list control is deleted, if this property is**true**, the**DropdownPopup**of this column will also be deleted.
- **DropdownPopup**: A popup attaching to the window. This could be either**\<Popup/\>**,**\<ToolstripMenu/\>**or anything that is compatible. If**DropdownPopup**is not**null**, a small dropdown button will be displayed in the column header.
- **SortingState**: The default value is**NotSorted**. Changing it to**Ascending**or**Descending**causes a sorting mark to appear in the column header. It is usually a triangle towards up (Ascending) or down (Descending).The**TextProperty**is for**\<BindableListView/\>**.

## Sub Items and Data Columns

The**Tile**and**Information**views render**Text**with selected**SubItems**. Selected**SubItems**are defined by**DataColumns**in the list control.**DataColumns**stores indices of**SubItems**. It also require enough column headers in**Columns**.

The relationship between these properties are:
- **list::ListViewItem::Text**matches**Columns[0]**.
- **list::ListViewItem::SubItems[X]**matches**DataColumns[X]**.
- **list::ListViewItem::SubItems[X]**matches**Columns[X + 1]**.

If**SubItems**in**list::ListViewItem**contains not enough values but they are required in**Columns**or**DataColumns**in the list control, the list control renders empty strings for these sub columns.

If**Columns**contains not enough values but they are required in**DataColumns**in the list control, the list control renders empty strings for these column headers.

## Sample


- Source code:[control_list_listview](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_listview/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_list_listview.gif)

