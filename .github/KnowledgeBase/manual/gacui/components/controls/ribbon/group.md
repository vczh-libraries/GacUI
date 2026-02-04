# \<RibbonGroup\>


- **\<RibbonGroup/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiRibbonGroup*
  - **Template Tag**: \<RibbonGroupTemplate/\>
  - **Template Name**: RibbonGroup

A**\<RibbonTabPage/\>**has three properties:
- **Expendable**: Set to**true**and a small expanding button appears at the bottom-right corner. The**ExpandButtonClicked**event raises when this button is clicked.
- **LargeImage**: Assign a large icon to this property, when there is no space to render the whole**\<RibbonTabPage/\>**in its smallest size, it becomes a button.
- **Items**: All sub controls in this**\<RibbonTabPage/\>**.The following controls are recommended in**Items**, but you could put whatever you want:
- Vertical splitters between two controls in a group:**\<RibbonSplitter/\>**.
- Large version of**\<ToolstripButton/\>**:**\<RibbonLargeButton/\>**,**\<RibbonLargeDropdownButton/\>**and**\<RibbonLargeSplitButton/\>**.
- Buttons that change their sizes automatically:**\<RibbonButtons/\>**
- Toolbars that reorganize itself automatically:**\<RibbonToolstrips/\>**
- Gallery list controls with additional menu items:**\<BindableRibbonGalleryList/\>**

## Sample

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon/UI)for details of ribbon controls.

Here is a simplified version about how to build a ribbon application:
- Source code:[control_ribbon_empty](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_empty/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_ribbon_empty.gif)

