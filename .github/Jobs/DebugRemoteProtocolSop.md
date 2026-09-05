# GacUI End-to-End UI Operation SOP

This document contains only the feature operations, error injections, and
observable results used to catch end-to-end regressions. It does not define the
test matrix or process setup. Use
[`DebugRemoteProtocolWithGacJS.md`](DebugRemoteProtocolWithGacJS.md) or
[`DebugRemoteProtocolWithNativeRenderer.md`](DebugRemoteProtocolWithNativeRenderer.md) to
establish, drive, inspect, replace, and close the renderer session.

## Expected Behavior of Remote Protocol

- Test Apps:
  - `RemotingTest_Core` in `GacUI` repo is a test app for remote protocol.
  - `RemotingTest_Rendering_Win32` in `GacUI` repo is a test app for remote protocol client using Windows native renderer.
  - `RemotingTest_Rendering_macOS` in `iGac` repo is a test app for remote protocol client using macOS native renderer.
  - `RemotingTest_Rendering_Wayland` in `wGac` repo is a test app for remote protocol client using Linux/Wayland native renderer.
  - `Gaclib/website/entry` in the `GacJS` repo is the browser renderer website/package; its checked-in static server hosts the built output for testing.
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
  - `RemotingTest_RvmHost` and GacJS `rvmhost` implement that `IViewModel`. A host connects through a manually selected network transport, the GacJS browser host, or auto-launched stdio `/Cli`. Requesters block until the accepted host completes the Workflow RPC service handshake and the required service is acquired and held; `Ready` alone is not sufficient.
- Expected Behavior:
  - Only one native or GacJS view-model host is allowed to connect.
  - `CppTest_Rvm /Cli:<path>` auto-launches `<path> /Cli`. `RemotingTest_Core /RVMT <renderer-transport> /Cli:<path>` keeps renderer traffic on its selected network server and auto-launches the host on a separate stdio-only server.
  - `/Cli:<path>` and the single literal argument `/Cli:"<path>"` name the same executable path. Exactly one balanced quote pair is removed; the npm bin, a JavaScript file, or `node <script>` is not a Core-launchable path.
  - If the accepted host disconnects before the main window is closed for any
    reason, this is a fatal error inside `RemotingTest_Core` and `CppTest_Rvm`.
    - When an in-flight or subsequent RPC observes host loss, `CppTest_Rvm`
      lets the injected `rpc_controller::RpcInjectedException` escape. Its
      nonzero crash is the required direct termination; no graceful-close
      handler or recovery is needed.
    - `RemotingTest_Core` catches the failure only to send the Core-authored
      fatal error to `RemotingTest_Renderer` before terminating.

## Rules for Every Operation

1. Read the current visible UI before acting. Use the active enclosing control,
   menu item, or dialog button rather than matching hidden or historical text.
2. Send the input through the visible application surface. For a Core target,
   use the renderer rather than Core `/IO`; for a standalone `CppTest_Rvm`
   target, use its local native UI. For text input, focus the intended editor
   and type with the keyboard; do not paste or inject text through the core.
3. After a tab, menu, dialog, or renderer transition, inspect the new visible UI
   and locate the controls again before continuing.
4. After every action, require the exact visible state change. Content that
   should close or clear must disappear from the active UI.
5. After every action except the final close, require the application to remain
   responsive with no startup mask, unexpected modal dialog, alert, error or
   fatal overlay, or disconnect.
   In GacJS, Mouse4 and Mouse5 must not navigate browser history or change the page URL.

## Shared Input Payload Checks

On the mouse-result label in either `/RPT` Home or `/FCT` Window Manager:

1. Require the separate initial readout `Alt: 0; Super: 0`.
2. For Left, Middle, Right, Mouse4 and Mouse5, press and release with neither
   modifier, Alt alone, Super alone, and both. Require the ordinary exact
   button down/up text and respectively `Alt: 0; Super: 0`, `Alt: 1; Super: 0`,
   `Alt: 0; Super: 1`, and `Alt: 1; Super: 1`. Also combine Ctrl and Shift
   with Alt and Super.
3. Over the same label, repeat the modifier combinations while moving,
   double-clicking, and scrolling in both directions on both wheel axes.
   Require the modifier readout to match each event. The readout uses
   reflected C++ mouse event arguments, so observing it through the renderer
   verifies both input delivery and rendered output.
4. Repeat after renderer replacement; pending input from the detached renderer
   must not alter the new session.
5. In an editable control, type `Hello[Ab]{Cd}` with the keyboard and require
   the exact text. Check outgoing browser KeyDown values `0xDB` and `0xDD`
   for the left and right bracket physical keys, including shifted braces.

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

### 3. Verify Shortcuts and Mouse Buttons

1. On `Home`, require the three shortcut labels to be exactly `Ctrl+Q`,
   `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+Q}` on Windows.
   Inspect the visible native-renderer DOM or GacJS page text after the initial
   connection; `osSuper` must never remain in either displayed shortcut.
   On macOS require `Command` in place of `Win`; on Linux require `Super`.
2. Press `Ctrl+Q` and require a distinct dialog containing exactly
   `You pressed Ctrl+Q!`. Close that dialog.
3. Press `Ctrl+Alt+Win+Q` and require a distinct dialog containing exactly
   `You pressed Ctrl+Alt+Win+Q!`. Close that dialog.
4. Press `Ctrl+Shift+Alt+Win+Q` through the real Windows global-hot-key path and
   require a distinct dialog containing exactly
   `You pressed Ctrl+Shift+Alt+Win+Q!`. Close that dialog.
5. Over the visible mouse-result label, press and release Left, Middle, Right,
   XBUTTON1, and XBUTTON2 through the renderer. For each button, require the
   label to change first to exactly `<button> button down!` and then to exactly
   `<button> button up!`, where `<button>` is respectively `Left`, `Middle`,
   `Right`, `Mouse4`, or `Mouse5`.
6. Require the mouse-result label to use the same theme font and text color as
   the shortcut labels, both initially and after mouse input.

### 4. Add and Clear DataGrid Rows

1. Activate `DataGrid`.
2. Require the `Name`, `Title`, and `Description` headers, the `Add 3 Rows` and
   `Clear` buttons, and no populated data rows.
3. Activate `Add 3 Rows` exactly once.
4. Require exactly three data rows. Require every row to contain a nonempty
   value under each of the three headers.
5. Activate `Clear`.
6. Require all three rows and their cell values to disappear while the grid,
   headers, `Add 3 Rows`, and `Clear` remain visible and usable.

### 5. Open and Close the Document Dialog

1. Activate `Document` and require document content containing the interactive
   text `RIGHT NOW`.
2. Activate `RIGHT NOW`.
3. Require an active modal dialog containing exactly
   `Pretend to be starting!`. Require the base window to remain visible and
   connected.
4. Activate the `OK` button belonging to that dialog.
5. Require the dialog and its text to disappear. Require the `Document` view to
   be active and responsive again.

### 6. Replace the Renderer and Verify State Continuity

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
9. On the third renderer, require the three shortcut labels to remain exactly
   `Ctrl+Q`, `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+Q}`. Repeat all three shortcut
   activations from section 3 and dismiss each expected dialog.
   Use the replacement renderer's canonical `Win`, `Command`, or `Super` name,
   and reject a stale name from the previous renderer or the literal `osSuper`.
10. Repeat the Mouse4 and Mouse5 down/up checks from section 3 through the third
    renderer and require the same exact label text without a fatal error or
    disconnect.

### 7. Close the Application Through the File Menu

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

### 4. Verify Shortcuts and Mouse Buttons

1. Activate `Window Manager` and require the three shortcut labels to be
   exactly `Ctrl+Q`, `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+Q}` on Windows.
   Inspect the visible native-renderer DOM or GacJS page text after the initial
   connection; `osSuper` must never remain in either displayed shortcut.
   On macOS require `Command` in place of `Win`; on Linux require `Super`.
2. Press `Ctrl+Q` and require a distinct dialog containing exactly
   `You pressed Ctrl+Q!`. Close that dialog.
3. Press `Ctrl+Alt+Win+Q` and require a distinct dialog containing exactly
   `You pressed Ctrl+Alt+Win+Q!`. Close that dialog.
4. Press `Ctrl+Shift+Alt+Win+Q` through the real Windows global-hot-key path and
   require a distinct dialog containing exactly
   `You pressed Ctrl+Shift+Alt+Win+Q!`. Close that dialog.
5. Over the visible mouse-result label, press and release Left, Middle, Right,
   XBUTTON1, and XBUTTON2 through the native Windows provider. For each button,
   require the label to change first to exactly `<button> button down!` and then
   to exactly `<button> button up!`, where `<button>` is respectively `Left`,
   `Middle`, `Right`, `Mouse4`, or `Mouse5`.
6. Require the mouse-result label to use the same theme font and text color as
   the shortcut labels, both initially and after mouse input.
7. Replace the renderer while keeping Core alive, return to `Window Manager`,
   and repeat the exact label, shortcut activation, and Mouse4/Mouse5 checks.
   Use the replacement renderer's canonical platform name and require no fatal
   error or unexpected disconnect.

### 5. Close the Application

1. In GacJS, use the renderer's visible `Force Exit` control. In a native
   renderer, activate the application's top-level `Exit` tab and then activate
   `self.Close() (InvokeInMainThread)` through the renderer.
2. Require the application session to end. Require the renderer to close,
   settle, or visibly enter a terminal disconnected state without a fatal
   prompt, error mask, reconnect, or retry loop. A frozen, apparently active
   application is a failure.

## Remote View Model Test (`/RVMT`)

Use a fresh application state. Follow
`DebugRemoteProtocolWithGacJS.md` or
`DebugRemoteProtocolWithNativeRenderer.md` to select and establish the required
requester, host, transport, and renderer topology, and wait for that guide's
readiness condition before performing the operations below. This SOP does not
redefine the test matrix or process-launch sequence. A standalone requester
uses its local native UI; a `RemotingTest_Core /RVMT` target uses its selected
renderer.

### 1. Verify the Initial UI

1. Require the exact window title `Remote View Model Test`.
2. Require one single-line text box and the exact initial greeting `Hello, !`.
3. Require no startup mask, error, fatal overlay, or disconnect.

### 2. Verify Workflow RPC

1. Focus the text box and type a unique, short, printable marker.
2. Require the greeting to become exactly
   `Hello, <marker>!`.
3. Require the application to remain connected and responsive.

### 3. Reject a Second View-Model Host

This externally started second-host operation applies to non-CLI targets. A
`/Cli` target owns only its auto-launched stdio child, so do not claim this
coverage unless an explicit test path calls `ConnectNewClient` again. Starting
a manual host on a Core `/Cli` renderer transport instead verifies that the
renderer-only server rejects non-renderer channels; it does not create a second
stdio host.

1. Keep the accepted view-model host and requester running after the successful
   `Translate` above.
2. Start a second view-model host using another instance of the target's
   non-CLI host mode and the same transport.
3. Require the second host not to be admitted: it must not replace the accepted
   host, report `Ready` to the requester, or change the visible greeting.
   Remaining blocked while it is rejected is acceptable; taking over the
   service is not.
4. Stop only the rejected second host. Type a different marker through the
   application surface and require the greeting to become exactly
   `Hello, <different-marker>!`, proving that the original host still owns the
   service and the requester remains responsive.

### 4. Close the Application

1. Close the application through the active UI surface.
2. Require the application session to end without a fatal error or retry loop.

# Fatal Regression Addendum

The following operations extend the procedures above without changing their
normal-path steps. Run each operation in a fresh session.

## Remote Protocol Test (`/RPT`): Verify the Core-Authored Fatal Error

1. Activate `Home`, locate the visible `Fatel Error` button (the intentional
   test label for the Fatal Error operation), and activate it through the
   renderer.
2. Require Core to send exactly one `!Error` carrying exactly
   `This is a fatel error!` and then terminate nonzero. A renderer-side local
   transport failure or ordinary disconnect without this Core-authored package
   is a failure.
3. In a Windows or macOS native renderer, require the `ERROR from GacUI Core`
   fatal prompt to contain the exact message. Choose `No` when asked whether to
   close the renderer so the fatal state remains inspectable. The raw Linux
   Wayland renderer has no `GuiApplication` for this prompt; require it to enter
   the retained fatal state directly instead.
4. In a native renderer, require renderer `Dom.fatalError` to equal exactly
   `This is a fatel error!`. Require the retained renderer to reject ordinary
   input without retrying or reconnecting while still accepting exact `!Exit`.
5. In GacJS, require the visible error mask, not the ordinary disconnect success
   mask, to contain exactly `This is a fatel error!`. The page rethrows this
   Core-authored error after displaying it, so one matching page error is
   expected. Require no additional error, reconnect, retry loop, or apparently
   live UI.
6. For a native renderer, send exact `!Exit` to close the retained renderer.
   Close the GacJS page after inspecting its terminal mask. Bound every wait and
   require no Core, renderer, listener, native prompt, or crash dialog to remain.

## Remote View Model Test (`/RVMT`): Terminate the Accepted View-Model Host

Run this operation for both requester shapes: `CppTest_Rvm` with an accepted
host, and `RemotingTest_Core /RVMT` with an accepted host and a matching
renderer. First complete the Workflow RPC check above. For Core, keep the
renderer connected throughout the failure.

1. Stop only the accepted view-model host through the failure injection
   appropriate to that host mode. Force-terminate a native, Node, or SEA host
   process through the operating-system process tool. For a browser `?rvmhost`
   mode, first replace its renderer with a separate ordinary renderer, then call
   `window.__gacui_rvmhost_session.host.stop()` to stop only the browser host
   session. Do not close the host through its graceful application path, and do
   not terminate the requester, Core, or retained renderer.
   A `/Cli` requester observes child termination directly as stdio EOF; no
   replacement poll or HTTP/MiniHTTP timeout applies, and the fatal outcome must
   begin promptly. A `/Pipe` requester must also observe the broken connection
   directly, even while idle. `/Http` and `/MiniHttp` have no heartbeat or explicit disconnect
   exchange, so they may remain unaware while only an empty `/Request` is
   pending. When the first post-loss `IViewModel` call caused by typing needs
   to send a real message, a successful response to that pending `/Request`
   starts a five-second deadline for the replacement `/Request`. Receiving the
   replacement acknowledges delivery. Missing it means that the connection is
   lost: unregister its connection GUID, reject any later `/Request` carrying
   that retired GUID as unknown, and promote the loss to a fatal disconnect.
   The call must finish within the bound and the UI must not hang.
2. Run two variants with fresh processes:
   - Idle-next-call: terminate the host after one successful `Translate`, then
     focus the text box and type a different marker to trigger the next real
     `IViewModel::Translate` RPC. `/Cli` records host loss promptly from EOF;
     the next `Translate` exposes the injected exception. For `/Http` and
     `/MiniHttp`, this operation is the latest permitted point for discovering
     the idle peer loss.
   - Delivery-acknowledgement loss: start a second `Translate` and terminate the
     host while that call is blocked. In `/Cli`, stdio EOF must release the
     caller promptly. In `/Http` or `/MiniHttp`, terminate it after the server
     delivers through the pending `/Request` but before the host submits the
     replacement `/Request`; the blocked caller must be released within the
     five-second bound. Once the replacement poll arrives, the transport has
     acknowledged delivery. With no heartbeat, a later crash during
     already-acknowledged request execution is not detectable until the server
     next needs to send a real message and that delivery is not acknowledged.
3. Require `CppTest_Rvm` to terminate nonzero from an unhandled
   `rpc_controller::RpcInjectedException`, without retry, recovery, or a
   graceful-close adapter. The resulting crash is valid direct termination.
4. For Core, require exactly one Core-authored `ErrorChannel` package carrying exactly
   `RemotingTest_RvmHost disconnected.` before Core terminates nonzero. A local
   renderer transport error alone is not sufficient.
5. In a Windows or macOS native renderer, require the fatal prompt to contain
   that exact message and choose `No` once so the retained renderer can be
   inspected. The raw Linux Wayland renderer has no `GuiApplication` for this
   prompt and must enter the retained fatal state directly. In every native
   renderer, require `Dom.fatalError` to equal the same text, then send exact
   `!Exit` to close it.
6. In GacJS, require the visible error mask, not the ordinary disconnect success
   mask, to contain exactly `RemotingTest_RvmHost disconnected.`. One matching
   page error from the page's deliberate rethrow is expected. Require no
   additional error, reconnect, retry loop, or apparently live UI, then close
   the page.
7. Bound every wait and require no requester, Core, host, renderer, listener,
   native prompt, or crash dialog to remain. Normal requester shutdown and
   renderer replacement must not produce this host-loss error.
