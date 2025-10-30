# \<BindableTextList\>

- **\<BindableTextList/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiBindableTextList* - **Template Tag**: \<TextListTemplate/\> - **Template Name**: TextList

**\<BindableTextList/\>** is very simlar to **\<TextList/\>**, but it accepts any classes or interfaces instead of just **list::TextItem**.

The **SelectedItem (SelectionChanged)** property returns the selected item in **ItemSource**. But if no item is selected, or multiple items are selected, it returns **null**.

**TextProperty (TextPropertyChanged)** defines a readable property in an item, to replace **list::TextItem::Text**.

**CheckedProperty (CheckedPropertyChanged)** defines a writable property in an item, to replace **list::TextItem::Checked**.

## NotifyItemDataModified function

When properties in an item is changed, if an item template has anything binded to them directly, the item template will be updated automatically.

But if the item template has anything binded to the **Text** property of **\<TextListItemTemplate/\>**, when the property described by **TextProperty** or **CheckedProperty** in the associated item is changed, the item template is not updated since no notification get sent out.

In this case, The **NotifyItemDataModified** function needs to be called. It notices a range of item templates to update itself.

## Sample

- Source code: [control_list_textlist_bindable](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_textlist_bindable/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_list_textlist_bindable.gif)

