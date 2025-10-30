# ALT Sequence and Control Focus

If a focusable control is put on a window, by setting the [Alt](../.././gacui/components/controls/basic/home.md) property to a value, for example "X", then it could get focused by pressing **ALT** followed by **X** (not **ALT+X**).

The **Alt** property could accept a sequence of characters, not just one. Characters in such a sequence needs to be entered to activate this control.

If **ESC** is pressed, it goes back to the previous container.

If **BACKSPACE** is pressed, it cancels the last character in the sequence.

When **ALT** is pressed, all valid **ALT** sequences will be printed on the window. All labels rendering these sequences will get changed

## Example

![](https://gaclib.net/doc/gacui/adv_alt.gif)

The operation is done by pressing the following keys: - **ALT**: Enter the ALT sequence mode. - **P**: There are two controls with the **Alt** property set to "P". It filters them out and wait for a number key. - **0**: The dropdown button is activated, a sub menu is opened. - **ESC**: Close the sub menu and go back to the previous container, which is the window. - **P**: Filter again. - **BACKSPACE**: Cancel the last character "P" in the sequence, showing all available items. - **P**: Filter again. - **1**: The paste button is activated, a piece of text get pasted to the editor. The ALT sequence mode is exited because an item is finally chosen. - **ALT**: Enter the ALT sequence mode. - **D**: Give the focus to the editor control. - **CTRL+A**: Select all text. - **DELETE**: Delete the selection.

## Behavior after being Focused

There are three kinds of behaviors when a control is focused by a **ALT** sequence. If a control is focused by **TAB**, it could behaves differently. - `empty list item` **Just being focused** Controls with complex keyboard behaviors like editors or list controls, will just get focused and do nothing else. - `empty list item` **Execute a Command** Controls with simple executable behaviors like buttons, will do what it does when clicked, after being focused by an **ALT** sequence. - `empty list item` **Just being focused** Controls that are not executable but contains executable items, like menu items with a sub menu, will render all executable items after being focused by an **ALT** sequence. A menu item with a sub menu does nothing when it is clicked. But if it is activated by an **ALT** sequence, the sub menu will be opened and wait for more keys.

## Create your own Behavior

If a new control class is created, there are multiple ways to customize the behavior about how a control should react to an **ALT** sequence:

### Using IGuiAltAction

**IGuiAltAction** is a protected base class of all controls. If a new control is created, methods could be overriden.

Properly assigning an **ALT** sequence to the **Alt** property of a control could also make these methods behave as expected easily. **IsAltEnabled** returns **true** if the control is visible and enabled. **IsAltAvailable** returns **true** if the control is focusable and the **Alt** property is not empty.

When the window is in the **ALT** sequence mode, a label rendering the sequence for this control will be put in the composition from **GetAltComposition**.

If both **IsAltEnabled** and **IsAltAvailable** returns **true**, then the result of **GetAlt()**, which is also the value from the **Alt** property by default, becomes one of an candidate. **OnActiveAlt** will be called when this control is selected by a **ALT** sequence.

### Using IGuiAltActionContainer

**IGuiAltActionContainer** is a [ service object ](../.././gacui/components/controls/basic/home.md). A service object could be attached to a control by calling **AddService** or overriding **QueryService**.

If an **IGuiAltActionContainer** instance is attached to a control, then all methods in **IGuiAltAction** are ignored. Instead, multiple **IGuiAltAction** object returned from this interface will be used.

You could now assign multiple **ALT** sequence to a control, with each sequence binded to a different behavior.

All **IGuiAltAction** returned from **IGuiAltActionContainer** must be enabled and available.

### Using IGuiAltActionHost

An **IGuiAltActionHost** instance could be attached to a control by calling **SetActivatingAltHost** or overriding **GetActivatingAltHost**.

When a control is selected by a **ALT** sequence, if **IGuiAltActionHost** is attached to this control, then the **ALT** sequence mode will not exit. Instead, this **IGuiAltActionHost** is treated as a nested container, and GacUI calls **CollectAltActions** to collect all valid **IGuiAltAction**, renders all **ALT** and wait for keyboard input.

All top level controls like a window or a menu are attached by an **IGuiAltActionHost** by default. If a sub menu is created on a menu item, **IGuiAltActionHost** is attached to this control. So when it is activated, it opens the sub menu and continue to wait for more keys, instead of executing this menu item.

**GuiAltActionHostBase** is the default implementation of **IGuiAltActionHost**. If it is attached to a control, all child controls with valid **IGuiAltAction** are not visible from the container, instead they are available when the parent control is selected by an **ALT** sequence. **GuiAltActionHostBase::SetAltControl** must be called to initialize this class, it tells this implementation where to search for child controls to collect **IGuiAltAction**.

