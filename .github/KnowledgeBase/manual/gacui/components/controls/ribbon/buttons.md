# \<RibbonButtons\>

- **\<RibbonButtons/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiRibbonButtons* - **Template Tag**: \<RibbonButtonsTemplate/\> - **Template Name**: RibbonButtons

**\<RibbonButtons/\>** automatically reorganize buttons and change their control templates when the size of **\<RibbonButtons/\>** changes.

**MaxSize** and **MinSize** limits how control templates of buttons are changed. Valid values of these properties are **Large**, **Small** and **Icon**. There are no default values for these properties.

Multiple **\<ToolstripButton/\>** and **\<RibbonIconLabel/\>** are expected in the **Buttons** property. You can put other controls in **Buttons**, but only **(vl::)presentation::controls::GuiToolstripButton** and **(vl::)presentation::controls::GuiRibbonIconLabel** get their control templates managed.

## Button Size and Control Templates

When the expected size is **Large**, the following control templates are applied to **\<ToolstripButton/\>** and **\<RibbonIconLabel/\>**: - **\<RibbonLargeButton/\>** - **\<RibbonLargeDropdownButton/\>** - **\<RibbonLargeSplitButton/\>** - **\<RibbonSmallIconLabel/\>**

When the expected size is **Small**, the following control templates are applied to **\<ToolstripButton/\>** and **\<RibbonIconLabel/\>**: - **\<ToolstripButton/\>** - **\<ToolstripDropdownButton/\>** - **\<ToolstripSplitButton/\>** - **\<RibbonSmallIconLabel/\>**

When the expected size is **Icon**, the following control templates are applied to **\<ToolstripButton/\>** and **\<RibbonIconLabel/\>**: - **\<RibbonSmallButton/\>** - **\<RibbonSmallDropdownButton/\>** - **\<RibbonSmallSplitButton/\>** - **\<RibbonIconLabel/\>**

These default values could be overrided by the control template of **\<RibbonButtons/\>**.

## Sample of Button Sizes

The following sample shows how **\<ToolstripButton/\>** and **\<RibbonIconLabel/\>** look like in different size.

- Source code: [control_ribbon_buttonsize](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_buttonsize/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_ribbon_buttonsize.gif)

## Auto Sizing

**\<ToolstripButton/\>** accepts size from **Large** to **Icon**. But **\<RibbonIconLabel/\>** only accepts size from **Small** to **Icon**. So they cannot be mixed in the same **\<RibbonButtons/\>** where **Large** is possible.

Typically, **\<RibbonIconLabel/\>** is much large than **\<ToolstripButton/\>** in width, so they are not recommended to be mixed in the same column in **\<RibbonButtons/\>**. **\<RibbonButtons/\>** groups every three controls in one column.

Multiple **\<RibbonButtons/\>** can be put in one **\<RibbonGroup/\>** and have different sizing configuration. A **\<RibbonSplitter/\>** is recommended but not required between two **\<RibbonButtons/\>** in the same **\<RibbonGroup/\>**

- Source code: [control_ribbon_buttonsize2](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_ribbon_buttonsize2/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_ribbon_buttonsize2.gif)

