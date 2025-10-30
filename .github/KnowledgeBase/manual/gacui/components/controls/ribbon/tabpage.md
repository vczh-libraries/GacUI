# \<RibbonTabPage\>

- **\<RibbonTabPage/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiRibbonTabPage* - **Template Tag**: \<ControlTemplate/\> - **Template Name**: CustomControl

A **\<RibbonTabPage/\>** is a **\<TabPage/\>** with two more properties: - **Groups**: All **\<RibbonGroup/\>** in this **\<RibbonTabPage/\>**. - **Highlighted**: Set to **true** to highlight the tab header.

Typically, a minmum height is needed for the first tab page like this: ``` <RibbonTabPage Text="First Page"> <att.ContainerComposition-set PreferredMinSize="y:110"/> </RibbonTabPage> ``` Alternatively, you could give the same minimum height to all tab pages.

## Sample

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon/UI) for details of ribbon controls.

Here is a simplified version about how to build a ribbon application: - Source code: [control_ribbon_empty](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_empty/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_ribbon_empty.gif)

