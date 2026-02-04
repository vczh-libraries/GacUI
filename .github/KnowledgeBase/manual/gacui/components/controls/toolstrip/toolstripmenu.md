# \<ToolstripMenu\>


- **\<ToolstripMenu/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiToolstripMenu*
  - **Template Tag**: \<MenuTemplate/\>
  - **Template Name**: Menu

A menu is a popup toolstrip button containers for menu items.

Multiple**\<ToolstripGroupContainer/\>**in the menu will be separated by horizontal splitters.

Expected button types inside this control are:
- **\<MenuItemButton/\>**Such toolstrip buttons render icons, titles, shortcut keys. If there are attached sub menus, sub menus are opened at the right side of buttons if there are enough spaces.

Please check out[this page](../../../.././gacui/components/controls/toolstrip/grouping.md)for details about**ToolstripItems**, which provides automatic layout for toolstrip buttons inside this control.

