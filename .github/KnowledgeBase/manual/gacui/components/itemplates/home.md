# Item Templates

[Item Templates](../../.././gacui/kb/itemplates.md)offer customization of list item appearances.

## Item Templates

Here are all predefined item templates:
- GuiTemplate for[<RepeatStack/>, <RepeatFlow/>](../../.././gacui/components/compositions/repeat.md)and[<ComboBox/>](../../.././gacui/components/controls/list/combobox.md)
  - **\<ListItemTemplate/\>**
    - **\<TextListItemTemplate/\>**for[<TextList/>](../../.././gacui/components/controls/list/textlist.md),[<BindableTextList/>](../../.././gacui/components/controls/list/bindabletextlist.md)and[<BindableRibbonGalleryList/>](../../.././gacui/components/controls/ribbon/gallerylist.md)
      - **\<TreeItemTemplate/\>**: for[<TreeView/>](../../.././gacui/components/controls/list/treeview.md)and[<BindableTreeView/>](../../.././gacui/components/controls/list/bindabletreeview.md)
  - \<ControlTemplate/\>
    - GuiGridCellTemplate
      - **\<GridVisualizerTemplate/\>**for[<BindableDataGrid/>](../../.././gacui/components/controls/list/bindabledatagrid.md)
      - **\<GridEditorTemplate/\>**for[<BindableDataGrid/>](../../.././gacui/components/controls/list/bindabledatagrid.md)

## Properties

All item templates has three kinds of properties:
- **Input Property**: Such property is assigned by the control, to communicate with the item template.
- **Output Property**: Such property is assigned by the item template, to communicate with the control.
- **Exchange Property**: Such property is assigned by both the item template and the control, to communicate with each other.

Properties of**GuiTemplate**are all**input property**:
- **Font**: Always sync to the**Font**property of the list control.
- **Context**: Always sync to the**Context**property of the[list control](../../.././gacui/components/controls/list/home.md)or[the repeat composition](../../.././gacui/components/compositions/repeat.md).
- **VisuallyEnabled**: Always sync to the**VisuallyEnabled**property of the list control.
- **Text**: The text representation of a list item.

**IMPORTANT**: All input/exchange properties of item templates will only be assigned after it is created. Property change events of these properties are expected to be subscribed in order to correctly behave when these properties are initialized or updated. In GacUI XML Resource, data binding is recommended on these properties to reduce the complexity of creating item templates.

## When to Implement Item Templates

The item template of**\<ListView/\>**and**\<BindableListView/\>**are not recommended to change. If you need a customizable list with items organized as boxes, you are recommended to use**\<BindableTextList/\<**with[predefined item arrangers.](../../.././gacui/components/controls/list/guilistcontrol.md)If you need a customizable grid, you are recommended to use**\<BindableDataGrid/\>**.

Typically, a**\<TextList/\>**or**\<BindableTextList/\>**is used with**\<ComboBox/\>**together. Since**\<TextListItemTemplate/\>**indirectly derives from**\<GuiTemplate/\>**, the combo box and the list control could share the same**ItemTemplate**.

When creating item templates for**\<RepeatStack/\>**and**\<RepeatFlow/\>**compositions, anything derives from**GuiTemplate**could be used. But since they are compositions instead of controls, no property of the created item template instance will be assigned.

