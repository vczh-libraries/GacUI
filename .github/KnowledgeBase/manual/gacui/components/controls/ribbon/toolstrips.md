# \<RibbonToolstrips\>


- **\<RibbonToolstrips/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiRibbonToolstrips*
  - **Template Tag**: \<RibbonToolstripsTemplate/\>
  - **Template Name**: RibbonToolstrips

**\<RibbonToolstrips/\>**stores**\<ToolstripButton/\>**in the**Groups**property. Splitters are automatically inserted between groups.

Usually buttons are organized in two rows. When there is not enough space, buttons will be organized in three rows.

The following controls are expected inside each**\<ToolstripGroup/\>**:
- **\<ToolstripButton/\>**
- **\<ToolstripDropdownButton/\>**
- **\<ToolstripSplitButton/\>**There is not need to have**\<ToolstripSplitter/\>**in groups.

## Sample


- Source code:[control_ribbon_toolstrip](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_toolstrip/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_ribbon_toolstrip.gif)

