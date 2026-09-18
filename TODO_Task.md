`Tools\UiaList\Planning.md` stores the last completed mission to create the `UiaList` tool. But when I am running it manually I notice some issues:
- When opening the detail window for an UI element, IInvokeProvider seems never appear in `Actions` tab.
- It seems almost all UI element implements `ILegacyIAccessibleProvider`, but clicking `DoDefaultAction` always crash.
- In `Actions`, getter functions without side effects should just display the result directly instead of making a button.

Implement UIA support for GacUI on Windows
- `GuiWindow` should expose a event `ChildCompositionUpdated`:
  - New types introduced by this event should be declared just above `GuiWindow`.
  - Argument has `CompositionUpdateType updateType` which has Inserted/Removed/Moved.
  - Argument had two GuiGraphicsComposition pointer field `parent` and `child`.
  - In `GuiGraphicsCompositokn::(Insert|Remove|Move)Child`:
    - Check if the current composition is attached to a root window.
    - When attached raise that event.
    - No recursive event needed, for example, when a composition is added making it appears on a window, the event should not be raised for all indirect children.
- Since UIA must know exact control type so a dynamic_cast list is expected.
- Dealing with data grid's editor, or any list/grid item template, it should replaces the visualizer in a cell.
- Not caring about how users could extend the UIA implementation yet.
- When hosted mode is used, sub window is treated as control of main window.
- Besides `ChildCompositionUpdated` all source code should be added to new files under the windows platform provider, in a `UIAutomation` folder.
  - 
