# \<ToolstripButton\>

- **C++/Workflow**: (vl::)presentation::controls::GuiToolstripButton* - **Template Tag**: \<ToolstripButtonTemplate/\> - **\<ToolstripButton/\>** - **Template Name**: ToolstripButton - **\<MenuBarButton/\>** - **Template Name**: MenuBarButton - **\<MenuItemButton/\>** - **Template Name**: MenuItemButton - **\<ToolstripDropdownButton/\>** - **Template Name**: ToolstripDropdownButton - **\<ToolstripSplitButton/\>** - **Template Name**: ToolstripSplitButton

**\<ToolstripButton/\>** is the button control for toolstrips and ribbons.

## Connecting to a \<ToolstripCommand/\>

Multiple toolstrip buttons could be connected to the same toolstrip command by assigning the command to the **Command** property of the button. When a toolstrip button is clicked, the **Executed** event in the connected toolstrip command will be raised.

The following properties in **\<ToolstripButton/\>** is automatically synced to the assigned command: - **LargeImage (LargeImageChanged)**: The large version of icon of the button. Used in ribbons. - **Image (ImageChanged)**: The icon of the button. Used in toolstrips and ribbons. - **ShortcutText (ShortcutTextChanged)**: The text representation of the key combination, rendered in menus. - **Text (TextChanged)**: The title of the menu. - **Enabled (EnabledChanged)** - **Selected (SelectedChanged)** Synchronizable properties in **\<ToolstripButton/\>** and **\<ToolstripCommand/\>** shares the same name, except that **ShortcutBuilder** in **\<ToolstripCommand/\>** controls **ShortcutText** in **\<ToolstripButton/\>**.

When a **\<ToolstripCommand/\>** is connected to a **\<ToolstripButton/\>**, **DO NOT** update these properties the **\<ToolstripButton/\>** directly, **DO** update these properties in the **\<ToolstripCommand/\>**, property values will be synced to all connected toolstrip buttons. For example, after disabling a toolstrip command, all connected toolstrip buttons will also be disabled.

## Shortcut Key

Setting a correct key combination to **\<ToolstripCommand/\>**::**ShortcutBuilder** will actually combine the command to this shortcut key in the current window. While setting **\<ToolstripButton/\>**::**ShortcutText** only affect rendering.

The value of the **ShortcutBuilder** property must be in this format: ``` Ctrl+Shift+Alt+KEY ``` **Ctrl+**, **Shift+** and **Alt+** are optional. And **KEY** must be one of a value returning from **vl::presentation::GetCurrentController()-\>InputService()-\>GetKeyName(VKEY)**.

A valid **KEY** must be one of the following values: - A to Z - 0 to 9 - F1 to F12 - Num +, Num -, Num *, Num Del, Num 0 to Num 9 - Home, End, Insert, Delete, Page Up, Page Down - Tab, Caps Lock, Backspace, Space - Esc, Enter, Pause, Scroll Lock, Sys Req (the name of Print Screen) - Left, Right, Up, Down

## Global Shortcut Key

If you want to bind a **\<ToolstripCommand/\>** to a global shortcut key, which affects the entire operating system instead of just the application, add **global:** before the shortcut key combination.

For example: ``` <ToolstripCommand Shortcut="global:Ctrl+Shift+C"/> ``` It binds the command to the global shortcut key **Ctrl+Shift+C**. Whenever you press this key combination, regardless your application has the focus or not, the command will be executed.

## Accessing SubMenu

The **ToolstripSubMenu** property returns the attached sub menu of the button. It is **null** by default, until **EnsureToolstripSubMenu** or **CreateToolstripSubMenu** is called.

In GacUI XML Resource, accessing the **SubMenu** property automatically get **EnsureToolstripSubMenu** called, so there is always a valid **\<ToolstripMenu/\>**.

Sometimes you might want to add other controls instead of menu items. The following methods are for this scenario: - **IsSubMenuExists** - **CreateSubMenu** - **DestroySubMenu** - **SubMenu** property - **SubMenuHost** property - **SubMenuOpening** property - **PreferredMenuClientSize** property When calling **SetSubMenu**, set the second argument (which is remembered by the **OwnedSubMenu** property) to **true** so that the sub menu is owned by the toolstrip button. In this case, when the toolstrip button is deleted, the sub menu will also be deleted.

**NOTE**: **CreateSubMenu**, **EnsureToolstripSubMenu** and **CreateToolstripSubMenu** make the created sub menu **owned**.

## \<ToolstripMenu/\> v.s. \<Menu/\>

The biggest difference between these two controls is that, like other toolstrip button containers, toolstrip controls put in the **\<ToolstripMenu/\>::ToolstripItems** property get a proper layout. Although you can put same controls in **\<Menu/\>**, but you need to worry about the layout by yourself.

Please check out [ this page ](../../../.././gacui/components/controls/toolstrip/grouping.md) for details about **ToolstripItems**.

## Reacting to ALT sequence

When a toolstrip button is executed by an **ALT** sequence, if it has a sub menu, the sub menu will be opened instead of executing the button.

This will make split buttons inaccessible. For keeping a good accessibility for your application, please make sure that the same command can be executed by another **ALT** sequence, e.g. the sub menu has a menu item for the same task, or it can be accessed from the menu bar.

