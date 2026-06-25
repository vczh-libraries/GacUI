# TAB and Control Focus

If a focusable control is put on a window, by setting the [TabPriority](../.././gacui/components/controls/basic/home.md) property to a value that is not **-1**, then it could get focused by pressing **TAB**.

Pressing **TAB** switches the focus between different controls in a window, in the order of their **TabPriority** from less to greater, when both **IsTabEnabled** and **IsTabAvailable** return **true**. If the value is less than **-1**, then they will be treated as **0xFFFFFFFFFFFFFFFF**. The order between controls with the same **TabPriority** is undefined.

## Behavior after being Focused

A focusable control will just get focused if **TAB** decides to give the focus to this control.

## Create your own Behavior

If a new control class is created, there are multiple ways to customize the behavior about how a control should react to **TAB**:

### Using IGuiTabAction

**IGuiTabAction** is a protected base class of all controls. If a new control is created, methods could be overriden.

**IsTabEnabled** returns **true** if the control is visible and enabled. **IsTabAvailable** returns **true** if the control is focusable.

The default value from **GetAcceptTabInput** is **false**. But a few controls like text controls or document controls set this value to **true**. When **GetAcceptTabInput** is **true**, it means this control defines the behavior for this key, like entering a **TAB** character, so that it doesn't want the focus to be switched to another control. Calling **SetAcceptTabInput** could change this value.

