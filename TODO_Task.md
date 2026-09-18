`Tools\UiaList\Planning.md` stores the last completed mission to create the `UiaList` tool. But when I am running it manually I notice some issues:
- When opening the detail window for an UI element, IInvokeProvider seems never appear in `Actions` tab.
- It seems almost all UI element implements `ILegacyIAccessibleProvider`, but clicking `DoDefaultAction` always crash.
- In `Actions`, getter functions without side effects should just display the result directly instead of making a button.

Implement UIA support for GacUI on Windows
- `GuiWindow` should expose a event `ChildCompositionUpdated`:
  - New types introduced by this event should be declared just above `GuiWindow`.
  - Argument has `CompositionUpdateType updateType` which has Added/Removed/Reordered.
  - Argument had two GuiGraphicsComposition pointer field `parent` and `child`.
- Controls destroys, connects to or disconnects from a top level window report.
  - If a control has not been hooked, hook all necessary events.
  - There will be no unhook, but callbacks should he ignored when:
    - Reported of being destructed.
    - Not connected to top level window.
- Since UIA must know exact control type so a dynamic_cast list is expected.
- Adding a internel property to a composition extends the UIA tree, if its associated control exists then the control's implementation is replaced.
- Dealing with data grid's editor, or any list/grid item template, it should replaces the visualizer in a cell.
