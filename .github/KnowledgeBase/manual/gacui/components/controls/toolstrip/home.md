# Toolstrip Controls

Toolstrip Controls contains toolbars and menus.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/DocumentEditorToolstrip.xml)for using toolstrip controls in GacUI XML Resource.

## Menu Buttons

**\<ToolstripButton/\>**the button control for menu bars, menus, toolbars and ribbon controls.

It has other four virtual controls:
- **\<MenuBarButton/\>**
- **\<MenuItemButton/\>**
- **\<ToolstripDropdownButton/\>**
- **\<ToolstripSplitButton/\>**

One[<ToolstripCommand/>](../../../.././gacui/components/controls/toolstrip/toolstripbutton.md)can be connected to multiple toolstrip buttons. A toolstrip command could also be assigned a shortcut key. Whenever the shortcut key is activated, or any connected toolstrip button is executed, the**Execute**event in the toolstrip command raises.

The**ShortcutText**property in**\<ToolstripButton/\>**is only for displaying purpose. Only the**Shortcut**property in**\<ToolstripCommand/\>**makes a command react to a key combination. When a**\<ToolstripCommand/\>**is connected to a**\<ToolstripButton/\>**, properties in**\<ToolstripButton/\>**get synced to the**\<ToolstripCommand/\>**, e.g., a proper**ShortcutText**in the**\<ToolstripButton/\>**is generated according to the**Shortcut**in**\<ToolstripCommand/\>**.

## Menu Bars

**\<ToolstripMenuBar/\>**is a toolstrip button container. Usually it is installed at the top of a window.

Expected button types inside this control are:
- **\<MenuBarButton/\>**

## Menus

**\<ToolstripMenu/\>**is a popup toolstrip button container.

The**SubMenu**property in**\<ToolstripButton/\>**is also a**\<ToolstripMenu/\>**.

Expected button types inside this control are:
- **\<MenuItemButton/\>**

Expected splitter types inside this control are:
- **\<MenuSplitter/\>**

## Toolbars

**\<ToolstripToolBar/\>**is a toolstrip button container.

Usually it is installed at the top of a window, and below the menu bar if exists.

Expected button types inside this control are:
- **\<ToolstripButton/\>**: A button.
- **\<ToolstripDropdownButton/\>**: A button which doesn't execute a command but show a menu when it is clicked.
- **\<ToolstripSplitButton/\>**: A button which could execute a command and also show a menu when different parts of it are clicked.

Expected splitter types inside this control are:
- **\<ToolstripSplitter/\>**

## Sample

In this demo there are all kinds of toolstrip controls. It also demos about how to group buttons, and how to connect buttons to commands.

There are three check boxes controlling the**Enabled**property of three commands. When a command is disabled, all connected buttons are also disabled.

The**Alt**property is set for all buttons. By pressing**Alt**and release, the window enters a special mode, waiting for a key series to navigate to anything that is executable.

The**Alt**property could contains multiple keys as a sequence. For buttons, executing means raising the**Clicked**event. For tab pages, executing means switching to that page. For most of other controls, executing means setting its focus.


- Source code:[control_toolstrip_button](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_toolstrip_button/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_toolstrip_button.gif)

