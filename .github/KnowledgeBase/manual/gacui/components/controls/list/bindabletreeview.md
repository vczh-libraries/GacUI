# \<BindableTreeView\>


- **\<BindableTreeView/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiBindableTreeView*
  - **Template Tag**: \<TreeViewTemplate/\>
  - **Template Name**: TreeView

**\<BindableTreeView/\>**is very simlar to**\<TreeView/\>**, but it accepts any classes or interfaces instead of just**tree::TreeViewItem**in**tree::MemoryNodeProvider**.

**IMPORTANT**: Unlike other list controls, the**ItemSource**in**\<BindableTreeView/\>**accepts an root node object, instead of a collection of nodes. The type of the root node object is expected to be what is specified in**env.ItemType**or any of its sub classes or interfaces.

The**SelectedItem (SelectionChanged)**property returns the selected item in**ItemSource**. But if no item is selected, or multiple items are selected, it returns**null**.

**TextProperty (TextPropertyChanged)**defines a readable property in an item, to replace**tree::TreeViewItem::text**.

**ImageProperty (ImagePropertyChanged)**defines a readable property in an item, to replace**tree::TreeViewItem::image**.

**ChildrenProperty (ChildrenPropertyChanged)**defines a readable property in an item, to replace**tree::MemoryNodeProvider::Children**. If it reads a observable collection out of an item, data binding applies. In this case, when this observable collection is changed,**\<BindableTreeView/\>**knows that child nodes for this node is changed.

## ReverseMappingProperty property and NotifyNodeDataModified function

When properties in an item is changed, if an item template has anything binded to them directly, the item template will be updated automatically.

But if the item template has anything binded to the**Text**or**Image**property of**\<TextListItemTemplate/\>**, when the property described by**TextProperty**or**ImageProperty**in the associated item is changed, the item template is not updated since no notification get sent out.

In this case, The**NotifyNodeDataModified**function needs to be called. It notices an item template to update by its associated item. This function will throw an exception if:
- The**ReverseMappingProperty**property is not specified.
- The given item is**null**.
- The given item is not binded to this control.
- The given item is the**ItemSource**object.

The**ReverseMappingProperty**is specified in the constructor when using C++. It lets the control stores something to the item to create a 1:1 mapping from tree nodes to items, using a property of**vl::reflection::description::Value**in C++ or**object**in Workflow script. When this property is used, items in the item source should be exclusively binded to this control. When an item get unbinded from the control, the specified property of an item will be set to**null**again.

## Sample


- Source code:[control_list_treeview_bindable](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_treeview_bindable/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_list_treeview_bindable.gif)

