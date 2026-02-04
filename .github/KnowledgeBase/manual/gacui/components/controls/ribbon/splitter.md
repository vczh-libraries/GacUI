# \<RibbonSplitter\>


- **\<RibbonSplitter/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiControl*
  - **Template Tag**: \<ControlTemplate/\>
  - **Template Name**: RibbonSplitter

**\<RibbonSplitter/\>**splits controls in**\<RibbonGroup/\>**.

For example,**\<RibbonButtons/\>**groups every three buttons together, but there is no splitter between buttons. In order to create a visible splitter in a**\<RibbonButtons/\>**of six buttons, a**\<RibbonSplitter/\>**is put between two**\<RibbonButtons/\>**, each having three buttons.

If a button never changes into smaller size,**\<RibbonLargeButton/\>**,**\<RibbonLargeDropdownButton/\>**and**\<RibbonLargeSplitButton/\>**are recommended to put in**\<RibbonGroup/\>**directly, instead of putting multiple**\<ToolstripButton/\>**in one**\<RibbonButtons/\>**, having both**MaxSize**and**MinSizee**set to**Large**.

Typically,**\<RibbonSplitters/\>**is designed for split buttons, since**\<RibbonToolstrips/\>**and**\<BindableRibbonGalleryList/\>**are recommended to have exclusive**\<RibbonGroup/\>**for each of them.

## Sample

Here is a simplified version about how to build a ribbon application:
- Source code:[control_ribbon_empty](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_empty/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_ribbon_empty.gif)

