# Child Instances

Some UI objects are allowed to have child instances, or child objects.

For example, in the root**\<Window/\>**, you can add controls or compositions to make them displayed in the window. You can also add components like radio button group controllers or system dialogs, which are not displayed in the window, instead they gain access to the window, and their life cycles are controlled by the window.

Adding different types of UI object as child instances to a parent object behavios differently. For example, adding**\<StackItem/\>**to**\<Stack/\>**makes it be aligned with other**\<StackItem/\>**, but adding other compositions or controls to**\<Stack/\>**will only layout it in a standard way.

Here is the list of objects that are allowed to have child instances. Please check out[GacUI Components](../../.././gacui/components/home.md)for details.
- **Any Composition**:
  - Control: Put the control inside the parent composition.
  - Composition: Put the composition inside the parent composition.
  - Element: Assign the element to the parent composition. A composition can be assigned with at most one element.
- **Any Control**:
  - Control: Put the control inside the parent control.
  - Composition: Put the composition inside the parent control.
  - Component (for root instance only): Add the component to the parent control. The component will gain access to the parent control, and the life cycle will be controlled.
- **\<Table/\>**:
  - **\<Cell/\>**: Align the cell according to its**Site**property.
  - **\<RowSplitter/\>**: Align the splitter according to its**RowsToTheTop**property.
  - **\<ColumnSplitter/\>**: Align the splitter according to its**ColumnsToTheLeft**property.
- **\<Stack/\> and \<RepeatStack/\>**:
  - **\<StackItem/\>**: Align the item according to its position in all items.
- **\<Flow/\> and \<RepeatFlow/\>**:
  - **\<FlowItem/\>**: Align the item according to its position in all items.
- **\<SharedSizeRoot/\>**:
  - **\<SharedSizeItem/\>**: Align the item according to its**Group**,**SharedWidth**and**SharedHeight**. The item is not required to be a direct child instance.
- **\<DocumentItem/\>**:
  - Control: Put the control inside a document according to the**Name**property of this document item.
  - Composition: Put the composition inside a document according to the**Name**property of this document item.
- **\<DocumentViewer/\> and \<DocumentLabel/\>**:
  - **\<DocumentItem/\>**: Register a document item. Please check out[<Doc>](../../.././gacui/xmlres/tag_doc.md)for details.
- **\<ToolstripToolBar/\>**,**\<ToolstripToolBar/\>**and**\<ToolstripMenu/\>**, typically also for**SubMenu**property of toolstrip buttons:
  - **\<ToolstripGroupContainer/\>**: Put the group containers in the toolstrip menu.
  - **\<ToolstripButton/\>**or its virtual classes: Put the button inside the parent control.
  - **\<ToolstripSplitter/\>**or**\<ToolstripSplitterInMenu/\>**: Put the splitter inside the parent control.
- **\<ToolstripGroupContainer/\>**, typically also for**SubMenu**property of toolstrip items:
  - **\<ToolstripGroup/\>**: Put the group inside this group container.**\<ToolstripSplitter\>**or**\<ToolstripSplitterInMenu/\>**will be automatically inserted between groups.
- **\<ToolstripGroup/\>**:
  - **\<ToolstripButton/\>**or its virtual classes: Put the button inside the parent control.
  - **\<ToolstripSplitter/\>**or**\<ToolstripSplitterInMenu/\>**: Put the splitter inside the parent control.

