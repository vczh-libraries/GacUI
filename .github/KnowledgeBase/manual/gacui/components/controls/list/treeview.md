# \<TreeView\>

- **\<TreeView/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiTreeView* - **Template Tag**: \<TreeViewTemplate/\> - **Item Template Tag**: \<TreeItemTemplate/\> - **Template Name**: TreeView - **Default Item Arranger**: list::FixedHeightItemArranger - **Default Item Template**: list::DefaultTreeItemTemplate

The **TreeItemExpander** theme is for child item expander bullets, which is a **\<SelectableButtonTemplate/\>**. But it will be overrides by **ExpandingDecoratorTemplate** in **\<TreeViewTemplate/\>**.

**\<TreeView/\>** stores a tree of **tree::TreeViewItem** and display them. In **tree::TreeViewItem** there are **text**, **image** and **tag** fields. Changing them doesn't cause the tree view to refresh itself. After a **tree::TreeViewItem** is modified, call **tree::MemoryNodeProvider::NotifyDataModified** to update the tree view.

The **Nodes** property is the root node of the tree view. It is important to remember that the tree view doesn't renderer the root node, insteads it renders direct children of the root node as multiple root nodes in the list control.

Each node is a **tree::MemoryNodeProvider**. In order to display a **tree::TreeViewItem**, **tree::MemoryNodeProvider** must be created first, and put **tree::TreeViewItem** in it using its **Data** property. **Children** property is provided to access child nodes of a **tree::MemoryNodeProvider**, changing this property immediately causes changing in the list control.

**NOTE**: In GacUI XML Resource, **tree::MemoryNodeProvider** is not needed, the virtual type **tree::TreeNode** combines the two, with property **Text**, **Image**, **Tag** and the default property for child nodes.

The **SelectedItem** property returns the currently selected item in **tree::TreeViewItem**. If no item is selected, or multiple items are selected, this property returns **null**.

## Sample

- Source code: [control_list_treeview](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_treeview/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_list_treeview.gif)

