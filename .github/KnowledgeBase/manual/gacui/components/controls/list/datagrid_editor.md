# Editor

The **EditorFactory** property is a **list::IDataEditorFactory**. Please check out [ this page ](../../../.././gacui/xmlres/instance/properties.md) for using this property in GacUI XML Resource.

To create a **list::IDataEditorFactory** in C++, it is recommended to use **list::DataEditorFactory** as the implementation for the interface. The constructor has one argument, which is an [ item template property value ](../../../.././gacui/xmlres/instance/properties.md) to create a **GuiGridEditorTemplate**.

Cells in different columns could have different editors. When a cell is clicked, the previous editor (if exists) is closed and deleted, the new editor is created and activated on that cell.

Editor objects are not cached, each time when an editable cell is clicked, a new editor is created.

Please check out [Item Templates](../../../.././gacui/components/itemplates/home.md) for details about **GuiGridEditorTemplate**.

## Step-by-Step Sample

In this demo, cell editors are applied to all columns: ``` <att.TitleColumn-set EditorFactory="sample::TextEditor"/> <att.AuthorColumn-set EditorFactory="sample::TextEditor"/> <att.DatePublishedColumn-set EditorFactory="sample::DateEditor"/> <att.SummaryColumn-set EditorFactory="sample::SummaryEditor"/> ``` All editors are defined in the same resource file.

**sample::TextEditor** allows users to edit text in the data grid.

**sample::DateEditor** allows users to edit a date in a data grid.

**sample::SummaryEditor** shows a button. When it is clicked, a new window is displayed. By modifying the text and clicked the **OK** button, the modification applies.

Updating the **CellValue** of a **\<GridEditorTemplate/\>** submits the modification. It is written to the row object using the **ValueProperty** proeprty of the column header.

In GacUI XML Resource, [ only one editor ](../../../.././gacui/xmlres/instance/properties.md) could be assigned to the **EditorFactory** property. When a cell is clicked, the associated editor is activated.

- Source code: [control_datagrid_editor](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_editor/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_datagrid_editor.gif)

