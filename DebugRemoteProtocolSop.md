# GacUI End-to-End UI Operation SOP

This document contains only the renderer-independent UI actions and observable
results used to catch end-to-end regressions. Use
[`DebugRemoteProtocolWithGacUI.md`](DebugRemoteProtocolWithGacUI.md) or
[`DebugRemoteProtocolWithNativeRenderer.md`](DebugRemoteProtocolWithNativeRenderer.md) to
establish, drive, inspect, replace, and close the renderer session.

## Expected Behavior of Remote Protocol

- Test Apps:
  - `RemotingTest_Core` in `GacUI` repo is a test app for remote protocol.
  - `RemotingTest_Rendering_Win32` in `GacUI` repo is a test app for remote protocol client using Windows native renderer.
  - `RemotingTest_Rendering_macOS` in `iGac` repo is a test app for remote protocol client using macOS native renderer.
  - `RemotingTest_Rendering_Wayland` in `wGac` repo is a test app for remote protocol client using Linux/Wayland native renderer.
  - `Gaclib/website/entry` npmjs package in `GacJS` repo is a http service, hosting a website as a remoting protocol client running in a Browser.
  - Core and client test apps are expected to run in the same computer.
- Expected Behavior:
  - Core is supposed to run with one or without renderer connected. When using the automation service from core to perform UI operation, anything should just work, don't assume there must be a renderer to handle any request/message.
  - At most one renderer can connect to core.
    - The first event from a renderer is expected to be `ControllerConnect`.
    - No matter there was an active renderer or not, after core receiving `ControllerConnect`, the renderer sent this take over the rendering.
    - The `ControllerConnectionEstablished` message will be sent from core to acknowledge the new renderer.
    - Core allows switching between any renderer, even between `GacJS` and a native renderer.
    - If the rendering is already taken over, any events from the old renderer will be ignored.
  - When any user operation on renderer causing core to exit normally (at the user's will), a message `ControllerConnectionStopped` will be sent from core to distinguish from a fatal error stopping core from running.
  - Upon core sending a fatal error, any event sending from the renderer will be ignored.
  - Once a renderer receiving `ControllerConnectionStopped`, any fatal error from core or from local should be ignored, as that might due to unstability of network connection after core initiating the finalization.
- Disconnection:
  - We should assume that connection is unreliable, core can't really know if the renderer is suddenly gone or not. So the remote protocol design also respect such assumption, core should not expect `ControllerDisconnect` event will always sent from a renderer.
    - Core should always assume the renderer could suddenly gone without saying anything.
    - Core will be remain running if an explicit signal of exiting doesn't happen.
  - For HTTP renderer channel clients, a failed active `/Request` or `/Response`, including HTTP 404 after renderer replacement or Core termination, is a fatal local channel error.
    - The raw VlppOS network protocol reports each failed HTTP exchange as a nonfatal local error and retains its transport-specific retry policy.
    - The VlppOS `IChannelClient` implementation promotes every local error received after the channel connection is established to fatal, because channel delivery is no longer reliable, and stops the network connection before it retries.
    - The renderer acts on the fatal local-error callback directly and enters its normal disconnected state without waiting for a later `OnDisconnected` callback.
    - A fatal local channel error does not display a fatal prompt or overlay. Only a Core-authored `!Error` package does.
  - `ControllerRequestExit` event asks core to close the window, but the actual app could reject the request. That's similar to clicking the "X" button don't guarantee the app is going to close.
  - `ControllerForceExit` event asks core to exit the app and don't give the actual app any chance to reject the request. That's similar to killing the process.
  - Core app requesting exiting, `ControllerRequestExit` and `ControllerForceExit` result in `ControllerConnectionStopped` being sent to the renderer because it is deterministic, and it is the only scenario of deterministic disconnection.

## Expected Behavior of Workflow RPC

- Test Apps:
  - `RemotingTest_Core /RVMT` and `CppTest_Rvm` running the `RemoteViewModelTest` app requesting a remote `IViewModel` to be implemented.
  - `RemotingTest_RvmHost` implemented that `IViewModel` and connect to above test apps via the Workflow RPC feature, they are blocked until `RemotingTest_RvmHost` successfully connected.
- Expected Behavior:
  - Only one `RemotingTest_RvmHost` is allowed to connect.
  - If it disconnects before the main window is closed in any reason, this is considered a fatal error inside `RemotingTest_Core` and `CppTest_Rvm`, terminating them directly.
  - Terminating `RemotingTest_Core` in this way also causing such fatal error to be sent to `RemotingTest_Renderer` so it could render the error information.

## Rules for Every Operation

1. Read the current visible UI before acting. Use the active enclosing control,
   menu item, or dialog button rather than matching hidden or historical text.
2. Send the input through the renderer surface. For text input, focus the
   intended editor and type with the keyboard; do not paste or inject text
   through the core.
3. After a tab, menu, dialog, or renderer transition, inspect the new visible UI
   and locate the controls again before continuing.
4. After every action, require the exact visible state change. Content that
   should close or clear must disappear from the active UI.
5. After every action except the final close, require the application to remain
   responsive with no startup mask, unexpected modal dialog, alert, error or
   fatal overlay, or disconnect.

## Remote Protocol Test (`/RPT`)

Use a fresh application state. Keep the same application session through the
renderer replacement and intentional close steps.

### 1. Verify the Initial UI

1. Require the exact window title `Remote Protocol Test`.
2. Require the `Home`, `DataGrid`, and `Document` tabs and the `File` menu.
3. Require the active `Home` view to contain `Click Me!`.
4. Require no startup mask, error overlay, unexpected modal dialog, or
   disconnected renderer.

### 2. Click the Home Button

1. Make `Home` active and locate the active `Click Me!` button.
2. Activate the button through the renderer.
3. Require its text to become exactly `You have clicked!` and require
   `Click Me!` to be absent from the active button.
4. Do not click the button again. Use `You have clicked!` as the
   state-continuity marker during renderer replacement.

### 3. Add and Clear DataGrid Rows

1. Activate `DataGrid`.
2. Require the `Name`, `Title`, and `Description` headers, the `Add 3 Rows` and
   `Clear` buttons, and no populated data rows.
3. Activate `Add 3 Rows` exactly once.
4. Require exactly three data rows. Require every row to contain a nonempty
   value under each of the three headers.
5. Activate `Clear`.
6. Require all three rows and their cell values to disappear while the grid,
   headers, `Add 3 Rows`, and `Clear` remain visible and usable.

### 4. Open and Close the Document Dialog

1. Activate `Document` and require document content containing the interactive
   text `RIGHT NOW`.
2. Activate `RIGHT NOW`.
3. Require an active modal dialog containing exactly
   `Pretend to be starting!`. Require the base window to remain visible and
   connected.
4. Activate the `OK` button belonging to that dialog.
5. Require the dialog and its text to disappear. Require the `Document` view to
   be active and responsive again.

### 5. Replace the Renderer and Verify State Continuity

1. Close the first renderer without closing the application session.
2. Open a second renderer using the same renderer configuration and connect it
   to the existing application session.
3. Require `Remote Protocol Test` and live application content to appear.
4. Activate `Home` and require `You have clicked!` without clicking the button
   again.
5. Keep the second renderer open and open a third renderer using the same
   configuration.
6. Require the third renderer to take over the application, show live content,
   and retain `You have clicked!` without another click.
7. Require the second renderer to be detached and unable to drive the
   application. Require it to settle without a fatal error or retry loop.
8. Inspect the third renderer's current UI and locate all controls again before
   continuing.

### 6. Close the Application Through the File Menu

Perform these steps in the third renderer.

1. Activate `File` and require its menu to be visible.
2. In the active menu, activate exactly
   `self.Close() (InvokeInMainThread)`.
3. Require an active confirmation dialog containing exactly
   `Do you want to exit?`. Require the application to remain responsive while
   the dialog is active.
4. Activate the `OK` button belonging to the confirmation dialog.
5. Require the application session to end. Require the active renderer to close,
   settle, or visibly enter a terminal state without a fatal alert or retry
   loop. A frozen, apparently active application is a failure.

## Complete Control Showcase (`/FCT`)

Use a fresh application state.

### 1. Verify the Initial UI

1. Require the exact window title `Complete Control Showcase`.
2. Require the top-level `List`, `Control`, `Misc`, and `Window Manager` tabs.
3. Require live controls with no startup mask, alert, error or fatal overlay, or
   disconnect.

### 2. Add and Clear Both Lists

1. Activate `List` and its default `TextList` page.
2. Require two visible list controls and the `Add 10 items` and `Clear` buttons.
   Require numbered items `0` through `9` to be absent.
3. Activate `Add 10 items` exactly once.
4. Require the complete sequence `0` through `9` in each of the two lists.
5. Activate `Clear`.
6. Require all ten numbered items to be absent from both lists while both list
   controls, `Add 10 items`, and `Clear` remain visible and usable.

### 3. Type and Preserve Text in Two Editors

1. Activate the top-level `Control` tab, then activate
   `Document Editor (Ribbon)` if it is not already active.
2. Require `Search:`, its associated text box, and the large central rich-edit
   surface.
3. Focus the text box associated with `Search:`. Type a unique, short, printable
   marker containing the renderer and transport names.
4. Require the exact marker to be visible in the search box with no missing,
   duplicated, or reordered characters.
5. Focus the large central rich-edit surface, not the search box. Type a
   different unique, short, printable marker.
6. Require the exact second marker to be visible in the rich-edit surface with
   no missing, duplicated, or reordered characters.
7. Activate the top-level `List` tab and require its content to render.
8. Return to `Control`, restore `Document Editor (Ribbon)` if necessary, and
   require both markers to remain unchanged without retyping them.
9. Require the application to remain connected and responsive throughout the
   typing and tab changes.
