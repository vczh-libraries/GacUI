# \<BindableListView\>


- **\<BindableListView/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiBindableListView*
  - **Template Tag**: \<ListViewTemplate/\>
  - **Template Name**: ListView

**\<BindableListView/\>**is very simlar to**\<ListView/\>**, but it accepts any classes or interfaces instead of just**list::ListViewItem**.

The**SelectedItem (SelectionChanged)**property returns the selected item in**ItemSource**. But if no item is selected, or multiple items are selected, it returns**null**.

**LargeImageProperty (LargeImagePropertyChanged)**defines a readable property in an item, to replace**list::ListViewItem::LargeImage**.

**SmallImageProperty (SmallImagePropertyChanged)**defines a readable property in an item, to replace**list::ListViewItem::SmallImage**.

In each**list::ListViewColumn**in**Columns**, there is a**TextProperty**property. This property takes no effect on**\<ListView/\>**, it is for**\<BindableListView/\>**. This property replaces**list::ListViewItem::Text**and**list::ListViewItem::SubItems**, depending on the position of each column.

It is fine not to put any**list::ListViewColumn**in**\<ListView/\>**, but this is required for**\<BindableListView/\>**even if you never use the**Detail**view. You must put at least one**list::ListViewColumn**in**\<BindableListView/\>**, because the**TextProperty**in the first column means the text representation for an item, which is used in every views.

## NotifyItemDataModified function

When properties in an item is changed, if an item template has anything binded to them directly, the item template will be updated automatically.

But if the item template has anything binded to the**Text**property of**\<TextListItemTemplate/\>**, when the property described by**LargeImageProperty**or**SmallImageProperty**or any column's**TextProperty**in the associated item is changed, the item template is not updated since no notification get sent out.

In this case, The**NotifyItemDataModified**function needs to be called. It notices a range of item templates to update itself.

## Sample


- Source code:[control_list_listview_bindable](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_listview_bindable/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_list_listview_bindable.gif)

