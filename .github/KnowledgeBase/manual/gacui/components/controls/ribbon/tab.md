# \<RibbonTab\>


- **\<RibbonTab/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiRibbonTab*
  - **Template Tag**: \<RibbonTabTemplate/\>
  - **Template Name**: RibbonTab

A**\<RibbonTab/\>**is a**\<Tab/\>**with two more properties:
- **BeforeHeaders**: A composition as a container that is put before tab page headers.
- **AfterHeaders**: A composition as a container that is put after tab page headers.Multiple**\<RibbonTabPage/\>**are expected in its**Pages**property.

Usually, a**\<RibbonTab/\>**is put on the top of a window.

## Sample

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon/UI)for details of ribbon controls.

Here is a simplified version about how to build a ribbon application:
- Source code:[control_ribbon_empty](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_empty/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_ribbon_empty.gif)

