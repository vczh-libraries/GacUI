# \<ToolstripToolBar\>


- **\<ToolstripToolBar/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiToolstripToolBar*
  - **Template Tag**: \<ControlTemplate/\>
  - **Template Name**: ToolstripToolBar

A toolbar is a toolstrip button containers for buttons.

Multiple**\<ToolstripGroupContainer/\>**in the menu will be separated by vertical splitters.

Expected button types inside this control are:
- **\<ToolstripButton/\>**: A button.
- **\<ToolstripDropdownButton/\>**: A button which doesn't execute a command but show a menu when it is clicked.
- **\<ToolstripSplitButton/\>**: A button which could execute a command and also show a menu when different parts of it are clicked.Such toolstrip buttons only render icons. If there are attached sub menus, sub menus are opened at the bottom side of buttons if there are enough spaces.

Please check out[this page](../../../.././gacui/components/controls/toolstrip/grouping.md)for details about**ToolstripItems**, which provides automatic layout for toolstrip buttons inside this control.

