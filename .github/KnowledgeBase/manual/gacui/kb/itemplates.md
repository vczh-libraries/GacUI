# Item Templates

As what is described in the previous chapter, there are several compositions and controls that behaves like collections. By assigning an item template to replace the default one (if exists), outfit of items can be preciously controlled.

Here are a list of compositions and controls that support item template:
- **RepeatStack**
- **RepeatFlow**
- **TextList**and**BindableTextList**
- **ListView**and**BindableListView**
- **TreeView**and**BindableTreeView**
- **BindableDataGrid**
- **BindableRibbonGalleryList**
- **ComboBox**Note that**ComboBox**doesn't work like other item template, it is not a collection control, but it can use the assigned item template to visualize the selected item, which comes from a list control that is assigned to the**ListControl**property.

Different controls requires different types of item templates. For example,**ComboBox**requires**ControlTemplate**,**TreeView**requires**TreeItemTemplate**,**DataGrid**requires**GridVisualizerTemplate**and**GridEditorTemplate**, while others require**TextListItemTemplate**.

List controls are all virtual (except**RepeatStack**and**RepeatFlow**which are compositions), it means that when an item is scrolled out of the visible boundary, it doesn't require rendering resources. In this case, the associated item template instance will just be deleted.

An item template is associated to an item, the association is never changed, the item will be passed in the only constructor argument, which means a valid item template must have exactly one**\<ref.Parameter/\>**. After that, data bindings are available.

Different kinds of item template types provide different extra information. For example,**TreeItemTemplate**contains properties to tell you about, is this item selected, is this item expanded, how many levels from this item to the root item, etc. With all of these information, data bindings become very useful and convenient to render the tree view item.

Here are a few examples of item templates:


- Source code:[Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml)
- ![](https://gaclib.net/doc/gacui/kb_itemplates_colorpicker.gif)


- Source code:[Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml)
- ![](https://gaclib.net/doc/gacui/kb_controls_datagrid.gif)

