# Grouping Multiple \<ToolstripButton\>


- **C++/Workflow**: (vl::)presentation::controls::GuiControl*
- **Template Tag**: \<ControlTemplate/\>
- **\<MenuSplitter/\>**
  - **Template Name**: MenuSplitter
- **\<ToolstripSplitter/\>**
  - **Template Name**: ToolstripSplitter


- **\<ToolstripGroupContainer/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiToolstripGroupContainer*
  - **Template Tag**: \<ControlTemplate/\>
  - **Template Name**: CustomControl


- **\<ToolstripGroup/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiToolstripGroup*
  - **Template Tag**: \<ControlTemplate/\>
  - **Template Name**: CustomControl

All toolstrip button containers have a**ToolstripItems**property. This property is the default property for them in GacUI XML Resource, which means direct child controls inside these containers are all added to this property.

Invisible toolstrip items in**ToolstripItems**are not rendered and take zero space, but they are in the toolstrip button container.

Predefined toolstrip button containers are:
- **\<ToolstripMenu/\>**
- **\<ToolstripMenuBar/\>**
- **\<ToolstripToolBar/\>**
- **\<ToolstripGroupContainer/\>**
- **\<ToolstripGroup/\>**Recommended controls in these containers are:
- **\<ToolstripButton/\>**or proper virtual controls
- **\<ToolstripGroupContainer/\>**
- **\<ToolstripGroup/\>**

A**\<ToolstripGroup/\>**organizes toolstrip items horizontally or vertically according to which toolstrip button container it is in.

A**\<ToolstripGroupContainer/\>**organizes toolstrip items horizontally or vertically according to which toolstrip button container it is in. It will choose a proper theme to create splitters between items, unless**SplitterTemplate**is assigned.

When a toolstrip button is put inside a toolstrip button container, their relation is built, and features are automatically available, like**ALT**sequence handling, or open sub menu when moving the mouse cursor while a ancestor toolstrip button is clicked and activated.

So it doesn't need to worry about how a toolstrip button is put inside a toolstrip button container. For example, you could wrap a toolstrip button in an item template, and assign this item template to a**\<RepeatFlow/\>**inside a**\<CustomControl/\>**, and finally add the**\<CustomControl/\>**to the**ToolstripItems**property of a toolstrip button container (in GacUI XML Resource you don't need to use**ToolstripItems**explicitly). Now you created a bindable menu item list in a menu!

**IMPORTANT**:**\<ToolstripGroupContainer/\>**will always create splitters no matter whether some toolstrip items are visible or invisible. When there is an item in**\<ToolstripGroupContainer/\>**that consist only bindable menu item list, you need to remove it from**\<ToolstripGroupContainer/\>**if it contains no item, just setting its**Visible**to**false**doesn't work.

## Sample


- Source code:[control_toolstrip_menu_binding](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_toolstrip_menu_binding/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_toolstrip_menu_binding.gif)

