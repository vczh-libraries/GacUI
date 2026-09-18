`Tools\UiaList\Planning.md` stores the last completed mission to create the `UiaList` tool. But when I am running it manually I notice some issues:
- When opening the detail window for an UI element, IInvokeProvider seems never appear in `Actions` tab.
- It seems almost all UI element implements `ILegacyIAccessibleProvider`, but clicking `DoDefaultAction` always crash.
- In `Actions`, getter functions without side effects should just display the result directly instead of making a button.

Implement UIA support for GacUI on Windows:
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
- Besides `ChildCompositionUpdated` all source code should be added to new files under `Source\PlatformProviders\Windows\UIAutomation` folder.
  - An `WM_GETOBJECT` message or whatever the correct name needs to be handled, its handler should be a class implementing `INativeControllerListener` and `INativeMessageHandler`:
    - It is called `WindowsUIAutomationListener`.
    - The following function defines `WindowsUIAutomationListener` as a variable called `uiaListener`:
      - `SetupWindowsGDIRenderer`
      - `SetupWindowsDirect2DRenderer`
      - `SetupHostedWindowsGDIRenderer`
      - `SetupHostedWindowsDirect2DRenderer`
      - They already have `listener` variables, renamed them to `nativeListener`, and those are the place for `uiaListener`.
    - `WindowsUIAutomationListener` will install `INativeMessageHandler` when a native window is created.
  - When hosted mode is used, sub window is treated as control of main window. The easiest way to determine is to pass a `isHostedMode` to `WindowsUIAutomationListener`'s constructor.

Verification:
- The research report for implementing UIA for GacUI is in `Todo\Task_UIA.md`.
- `FullControlTest` already has all controls in use, you could run `CppTest` for hosted mode and `CppTest_Metaonly` for normal mode.
- Use `UIAList` to view `CppTest` and `CppTest_Metaonly` and make sure every control is exposed expectedly.
  - Every test app including `UiaListApp`, which already runs an automation service, should be able to receive a `/AsPort:xxxx` to specify the http port for automation service.
    - The default value is `8888` when it is not specified.
    - Some of them already have such cli arguments, change it to `/AsPort` and update documents where is saying this argument. This should include `Project.md` to say that which test app handle `/AsPort`, emphasis that the purpose is to enable running multiple test apps at the same time, currently later launches crash because the port is taken.
    - Also update `Tools\UiaList\README.md`.
- Since `CppTest` and `CppTest_Metaonly` is stable, powershell scripts `Test\UIA_CppTest.ps1` and `Test\CppTest_Metaonly.ps1` should be created to call UIA (probably in C#) and traverse all controls and make sure expected control types and providers are implemented, as well as reading important properies like text (but exclude position related properties as they are not stable).
  - In these scripts, call `copilotBuild.ps1` on `GacUISrc.sln` for debug x64, launch a corresponding test app.
  - Run them to make sure they work.
  - Exit the test app.
  - These powershell scripts should be organized like a unit test program so that each test target will be printed before executing them.
  - Main provider actions should also be used and expect UI changing.
