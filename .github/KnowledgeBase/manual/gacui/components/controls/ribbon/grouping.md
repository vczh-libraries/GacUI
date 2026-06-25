# Grouping Multiple \<ToolstripButton\>

Unlike toolstrip controls, there is no need to add nested grouping in ribbon.

**\<RibbonButtons/\>** makes every three buttons in one group but without a splitters. splitters could be provided by **\<RibbonGroup/\>**.

**\<RibbonToolstrips/\>** requires buttons to be in groups. But since the area is limited, you are not expected to put too many buttons in one **\<RibbonToolstrips/\>**. Multiple **\<RibbonToolstrips/\>** could be separated by **\<RibbonGroup/\>**.

**\<RibbonSplitter/\>** could be added in **\<RibbonGroup/\>** if really needed.

