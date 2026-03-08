# !!!KNOWLEDGE BASE!!!

# DESIGN REQUEST

you are going to research about the remote protocol based unit test. You are going to create an API explanation of how the remote protocol based unit test is launched, how to start multiple test application in the unit test, how snapshots are stored, and the connection to the frame name. Note that the frame name described what happened at the previous frame, not what to happen at the current frame, it relates to how snapshots are connecting a frame to a rendering result. Also pay attention to how user interaction is simulated with unit test API.

# INSIGHT

## Overview

The remote protocol based unit test framework allows testing GacUI applications without real OS windows or rendering. It uses the same remote protocol architecture as real remote deployment, but replaces the actual renderer with a mock protocol implementation (`UnitTestRemoteProtocol`) that captures rendering results as snapshots and feeds simulated user input back through the protocol pipeline. The GacUI core application runs identically to production — composing UI, laying out controls, and issuing protocol messages — while the test harness intercepts everything.

## Unit Test Launch Mechanism

### Entry Point and Initialization

The test executable's `main` function calls `GacUIUnitTest_Initialize(&config)` with a `UnitTestFrameworkConfig` containing `snapshotFolder` and `resourceFolder` paths. This is called once per test run and:
- Sets `GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES` and `GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD` to `true`, allowing type manager and plugin reuse across tests.
- Calls `GetGlobalTypeManager()->Load()` and `GetPluginManager()->Load(true, false)` once.

After all tests, `GacUIUnitTest_Finalize()` tears down the type manager and plugin manager.

### Per-Test-Case Setup

Each test case follows a standard pattern:
1. Call `GacUIUnitTest_SetGuiMainProxy(callback)` to register the test's frame-based action callbacks.
2. Optionally call `GacUIUnitTest_LinkGuiMainProxy(linkCallback)` to wrap additional setup (resource compilation, theme registration, window creation) around the test proxy.
3. Call `GacUIUnitTest_Start(appName)`, `GacUIUnitTest_StartAsync(appName, config)`, or `GacUIUnitTest_Start_WithResourceAsText(appName, config, resource)` to launch the test.

### The Proxy Chaining Pattern

`GacUIUnitTest_SetGuiMainProxy` stores a `UnitTestMainFunc` (signature: `void(UnitTestRemoteProtocol*, IUnitTestContext*)`).

`GacUIUnitTest_LinkGuiMainProxy` captures the current proxy as `previousMainProxy` and creates a new proxy that calls the link function with the previous proxy as a parameter:
```
auto previousMainProxy = guiMainProxy;
GacUIUnitTest_SetGuiMainProxy([=](protocol, context) {
    proxy(protocol, context, previousMainProxy);
});
```

This enables decorator-style composition. When `GacUIUnitTest_StartFast_WithResourceAsText` is called, it internally calls `GacUIUnitTest_LinkGuiMainProxy` to inject theme registration, resource compilation, and window creation, then delegates to the user's test proxy via `previousMainProxy(protocol, context)` before calling `GetApplication()->Run(window)`.

### Protocol Stack in GacUIUnitTest_Start

`GacUIUnitTest_Start` constructs the full protocol stack synchronously in-process:

Renderer side:
- `UnitTestRemoteProtocol` — implements `IGuiRemoteProtocol`, simulates the renderer.
- `GuiRemoteJsonChannelFromProtocol` — converts protocol calls to JSON.
- `GuiRemoteJsonChannelStringDeserializer` — JSON ↔ String conversion.
- `GuiRemoteUtfStringChannelDeserializer<wchar_t, char8_t>` — String ↔ UTF-8.

Core side (mirrors back):
- `GuiRemoteUtfStringChannelSerializer<wchar_t, char8_t>` — UTF-8 ↔ String.
- `GuiRemoteJsonChannelStringSerializer` — String ↔ JSON.
- `GuiRemoteProtocolFromJsonChannel` — JSON ↔ typed protocol calls.

The entire protocol stack is wired together in-process. When `useChannel == UnitTestRemoteChannel::None`, the verifier protocol directly wraps `UnitTestRemoteProtocol`'s `IGuiRemoteProtocol`, bypassing the serialization layers for speed. Otherwise, the serialization channel is used for testing round-trip fidelity.

On top of the channel:
- `GuiRemoteProtocolFilterVerifier` — validates repeat-filtering invariants.
- `GuiRemoteProtocolFilter` — filters redundant messages.
- `GuiRemoteProtocolDomDiffConverter` (optional, when `useDomDiff` is true) — converts full DOM to DOM diffs.

Finally, `SetupRemoteNativeController(protocol)` creates the runtime stack:
- `GuiRemoteController` — virtual `INativeController` wrapping the protocol.
- `GuiHostedController` — hosted mode controller wrapping `GuiRemoteController`.
- Resource managers for rendering.
- Calls `GuiApplicationMain()` which calls `GuiMain()`, which invokes the registered test proxy.

### Async Mode

`GacUIUnitTest_StartAsync` uses `GuiRemoteProtocolAsyncJsonChannelSerializer` to place the core and renderer on separate threads, testing the full async channel behavior. Two threads are spawned via `RunInNewThread`: a "channel thread" for serialization I/O and a "UI thread" for the GacUI application. The call waits for `asyncChannelSender.WaitForStopped()` before proceeding to snapshot logging.

### The Main Event Loop

`GuiRemoteController::RunOneCycle()` is the main loop:
1. Calls `remoteProtocol->GetRemoteEventProcessor()->ProcessRemoteEvents()` — this is where `UnitTestRemoteProtocol::ProcessRemoteEvents()` runs, executing frame callbacks.
2. Calls `remoteMessages.Submit(disconnected)` — flushes pending protocol messages.
3. Calls `callbackService.InvokeGlobalTimer()` — triggers `GuiHostedController::GlobalTimer()`, which performs layout and rendering, generating new DOM/element data that the renderer captures.
4. Calls `asyncService.ExecuteAsyncTasks()`.

The cycle repeats until `connectionStopped` is true (the application closes).

## Starting Multiple Test Applications

Each call to `GacUIUnitTest_Start` runs one complete application lifecycle (from `GuiMain()` to window close). Usually one for each `TEST_CASE`.

However, multiple windows can be created within the same application instance:
- The `GacUIUnitTest_StartFast_WithResourceAsText` template creates a main window from a GacUI XML resource. Inside frame callbacks, additional windows can be created using `Value::Create(L"namespace::ClassName")` and shown with `Show()`, `ShowModal()`, or `ShowDialog()`.
- `GacUIUnitTest_LinkGuiMainProxy` can be called multiple times before `GacUIUnitTest_Start`, chaining multiple setup layers (e.g., one for resources, one for theme, one for additional initialization).
- To run multiple test applications across test cases, each `TEST_CASE` calls the full `GacUIUnitTest_SetGuiMainProxy` → `GacUIUnitTest_Start` sequence independently.

## How Snapshots Are Stored

### Snapshot File Structure

For a test named `Controls/Basic/GuiButton/ClickOnMouseUp`:
- **Main file**: `{snapshotFolder}/Controls/Basic/GuiButton/ClickOnMouseUp.json` — contains `UnitTest_RenderingTrace` with `createdElements`, `imageCreations`, `imageMetadatas`, and `frames` (each frame only has `frameId` and `frameName`, detailed data is cleared).
- **Frame files**: `{snapshotFolder}/Controls/Basic/GuiButton/ClickOnMouseUp/frame_0.json`, `frame_1.json`, etc. — each contains a full `UnitTest_RenderingFrame` with `frameId`, `frameName`, `windowSize`, `elements` (all rendered element descriptions keyed by ID), and `root` (the rendering DOM tree).
- **Workflow snapshot**: `ClickOnMouseUp[x64].txt` or `[x86].txt` — compiled Workflow script text for verifying script generation stability.
- **Commands log**: `ClickOnMouseUp[commands].txt` — rendering command logs per frame (when not using DOM diff).
- **Diffs log**: `ClickOnMouseUp[diffs].txt` — DOM diff logs per frame.

### Frame Capture Mechanism

**Rendering frames are captured passively**. Each time the GacUI core calls `RequestRendererBeginRendering`, `UnitTestRemoteProtocol_Rendering::Impl_RendererBeginRendering` creates a new `UnitTestLoggedFrame` and records the `frameId` (from the core's monotonic counter). When `Impl_RendererEndRendering` is called, the frame's `renderingDom` is captured. The frame is flagged as ready via `lastRenderingCommandsOpening`.

**Active frame logging** happens in `LogRenderingResult()`, called at the start of each `ProcessRemoteEvents()` cycle:
1. `TryGetLastRenderingFrameAndReset()` checks if there's a completed rendering frame since the last check. If yes, it becomes the `candidateFrame`.
2. On the next cycle where no new rendering has occurred (i.e., the UI has settled), the `candidateFrame` is committed to `loggedTrace.frames` with the current `lastElementDescs` and `sizingConfig`. This ensures the snapshot captures the final stable rendering state after all layout passes.
3. `LogRenderingResult()` returns `true`, signaling that a frame is ready. `ProcessRemoteEvents()` then:
   - Sets the `frameName` on the just-committed frame (if the frame callback provided a name).
   - Executes the next registered callback function.
4. If 100 consecutive cycles pass without any rendering change after previously rendering, the test fails with an error about no UI update.

### The Frame Name Semantics

The frame name is assigned to the **already-committed snapshot** (the result of previous rendering), then the frame callback is executed. This means:
- The `frameName` describes what **led to** the current visual state, not what the callback will do next.
- The first frame is conventionally named `"Ready"`, representing the initial rendering state after the window opens.
- Subsequent frame names describe the action taken in the previous callback (e.g., `"Hover"` means the previous callback moved the mouse over something, and this snapshot shows the hover state).
- A snapshot file `frame_N.json` with `frameName: "Clicked"` shows what the UI looks like **after** a click was performed in the callback before it.

### The Connection: Frame Name → Rendering Result

The sequence is:
1. Application renders → DOM/elements captured as `candidateFrame`.
2. UI settles (no more rendering) → `candidateFrame` committed to `loggedTrace.frames`.
3. `frameName` is set on the committed frame.
4. Frame callback executes (may trigger events → triggering more rendering).
5. Go back to 1.

So the frame name is attached to the rendering result that is the **consequence** of the previous frame's actions. The very first frame has no previous action, hence the convention to name it `"Ready"`.

### Write Strategy

`GacUIUnitTest_WriteSnapshotFileIfChanged` compares the new content with the existing file. It only writes if the content differs. This means `git diff` only shows test cases whose rendering actually changed.

After writing individual frame files, `GacUIUnitTest_LogUI` strips frame detail from the in-memory trace (keeping only `frameId` and `frameName`) before writing the main trace file. It also deletes any leftover `frame_*.json` files that no longer correspond to actual frames.

## User Interaction Simulation

### The IO Commands Layer

`UnitTestRemoteProtocol_IOCommands` provides methods to simulate all user input. It maintains virtual state:
- `mousePosition` — current mouse position (initially unset; first `MouseMove` triggers `OnIOMouseEntered`).
- `pressingKeys` — set of currently pressed keys.
- `leftPressing`, `middlePressing`, `rightPressing` — mouse button states.
- `capslockToggled` — capslock toggle state.

Each method constructs the appropriate `NativeWindowMouseInfo`, `NativeWindowKeyInfo`, or `NativeWindowCharInfo` struct and calls the corresponding event on `IGuiRemoteProtocolEvents` (obtained via `UseEvents()`).

### Location Calculation

`LocationOf(controlOrComposition, ratioX, ratioY, offsetX, offsetY)` computes the target point in **absolute screen coordinates**:
1. Gets the composition's `GetGlobalBounds()` (logical coordinates).
2. Converts to native coordinates using `nativeWindow->Convert()`.
3. Applies the ratio (default 0.5 = center) and offset.
4. Adds the window's screen position `nativeWindow->GetBounds().LeftTop()`.

### Event Flow Through the Pipeline

When a test calls `protocol->LClick(location)`:
1. `_LDown(location)` → `MouseMove(location)` (if position changed) → `UseEvents().OnIOButtonDown({Left, MakeMouseInfo()})`.
2. `_LUp(location)` → `UseEvents().OnIOButtonUp({Left, MakeMouseInfo()})`.
3. These events are received by `GuiRemoteEvents` (the `IGuiRemoteProtocolEvents` implementation in `GuiRemoteController`).
4. `GuiRemoteEvents::OnIOButtonDown()` dispatches to `listener->LeftButtonDown(info)` for all listeners on `remoteWindow`.
5. `GuiGraphicsHost` (the primary `INativeWindowListener`) receives the call.
6. `GuiGraphicsHost::LeftButtonDown()` calls `OnMouseInput(info, true, false, &leftButtonDown)`.
7. `OnMouseInput()` converts native coordinates to logical coordinates, performs hit-testing via `FindVisibleComposition()`, creates `GuiMouseEventArgs` with composition-relative coordinates, and dispatches via `RaiseMouseEvent()`.
8. The event bubbles through the composition tree, triggering control-level handlers (e.g., `GuiButton::Clicked`).

All of this happens **synchronously within a single `ProcessRemoteEvents()` call**, so by the time the frame callback returns, all side effects of the interaction have been processed.

### Blocking Function Caveat

If an IO action triggers a blocking function (like `ShowDialog`), the frame callback would never return, and the unit test framework detects this via the `frameExecuting` flag and fails. The solution is to wrap the IO action in `GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); })`, which defers the action to the next event loop iteration.

### Key and Character Input

- `KeyPress(key)` sends `OnIOKeyDown` followed by `OnIOKeyUp`.
- `KeyPress(key, ctrl, shift, alt)` wraps the key press with modifier key down/up events.
- `TypeString(text)` sends a sequence of `OnIOChar` events for each character, without synthesizing key down/up events.

### Mouse Wheel

- `WheelUp(jumps)` / `WheelDown(jumps)` send `OnIOVWheel` with delta scaled by 120 per jump.
- `HWheelLeft(jumps)` / `HWheelRight(jumps)` send `OnIOHWheel` similarly.

# ASKS

# DRAFT

## DRAFT REQUEST

## IMPROVEMENTS

## API EXPLANATION (GacUI)

Title: Remote Protocol Unit Test Framework

This document describes the API of the remote protocol based unit test framework for GacUI. It would be added under `### Choosing APIs` in the GacUI section of `Index.md`.

## DOCUMENT

# Remote Protocol Unit Test Framework

The remote protocol unit test framework allows testing GacUI applications without real OS windows or rendering. It reuses the same remote protocol architecture as production remote deployment, replacing the renderer side with a mock implementation (`UnitTestRemoteProtocol`) that captures rendering results as snapshots and feeds simulated user input through the protocol pipeline. The GacUI core side runs identically to production.

Implementation lives in `Source/UnitTestUtilities/`.

## Test Executable Initialization and Finalization

### Global Setup (GacUIUnitTest_Initialize)

Each test executable's `main` calls `GacUIUnitTest_Initialize(&config)` with a `UnitTestFrameworkConfig` containing `snapshotFolder` and `resourceFolder`. This function:
- Sets `GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES` and `GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD` to `true`, allowing the type manager and plugin system to be loaded once and reused across all test cases.
- Calls `GetGlobalTypeManager()->Load()` and `GetPluginManager()->Load(true, false)` once.

### Global Teardown (GacUIUnitTest_Finalize)

Called after all tests complete. Tears down the type manager and plugin manager.

## Per-Test-Case Setup and the Proxy Chaining Pattern

### Registering Frame Callbacks

Each test case follows a standard pattern:
1. Call `GacUIUnitTest_SetGuiMainProxy(callback)` to register a `UnitTestMainFunc` (signature: `void(UnitTestRemoteProtocol*, IUnitTestContext*)`). This is the test's frame-based action callback that registers idle frame handlers via `OnNextIdleFrame`.
2. Optionally call `GacUIUnitTest_LinkGuiMainProxy(linkCallback)` to wrap additional setup around the test proxy.
3. Call one of the start functions to launch the test.

### Proxy Chaining (GacUIUnitTest_LinkGuiMainProxy)

`GacUIUnitTest_LinkGuiMainProxy` captures the current proxy as `previousMainProxy` and installs a new proxy that calls the link function with the previous proxy as a parameter. This enables decorator-style composition — each link layer can perform setup before delegating to the inner proxy and cleanup after.

### GacUIUnitTest_StartFast_WithResourceAsText

This template function is the most commonly used entry point. It internally calls `GacUIUnitTest_LinkGuiMainProxy` to inject:
1. Sending `OnControllerConnect` with `ControllerGlobalConfig` to the protocol.
2. Registering a theme (e.g., `DarkSkin`).
3. Compiling the GacUI XML resource via `GacUIUnitTest_CompileAndLoad`.
4. Creating the main window from the resource via `Value::Create(windowTypeFullName)`.
5. Calling `previousMainProxy(protocol, context)` to let the test register its frame callbacks.
6. Running the application via `GetApplication()->Run(window)`.
7. Unregistering the theme on cleanup.

It also saves the compiled Workflow script text as a snapshot file (`[x64].txt` or `[x86].txt`) for Workflow generation stability verification.

### Start Functions

- `GacUIUnitTest_Start(appName, config)` — synchronous in-process test with optional serialization channel.
- `GacUIUnitTest_StartAsync(appName, config)` — async test with core and renderer on separate threads.
- `GacUIUnitTest_Start_WithResourceAsText(appName, config, resourceText)` — wraps resource compilation via `GacUIUnitTest_LinkGuiMainProxy`, then delegates to `GacUIUnitTest_Start` or `GacUIUnitTest_StartAsync` depending on `config.useChannel`.

## Protocol Stack Construction

### Synchronous Mode (GacUIUnitTest_Start)

`GacUIUnitTest_Start` constructs the full protocol stack in-process:

**Renderer side (deserialization direction):**
- `UnitTestRemoteProtocol` — mock `IGuiRemoteProtocol` implementation.
- `GuiRemoteJsonChannelFromProtocol` — converts protocol calls to JSON.
- `GuiRemoteJsonChannelStringDeserializer` — JSON to String.
- `GuiRemoteUtfStringChannelDeserializer<wchar_t, char8_t>` — String to UTF-8.

**Core side (serialization direction, mirrors back):**
- `GuiRemoteUtfStringChannelSerializer<wchar_t, char8_t>` — UTF-8 to String.
- `GuiRemoteJsonChannelStringSerializer` — String to JSON.
- `GuiRemoteProtocolFromJsonChannel` — JSON to typed protocol calls.

**Protocol filter layers on core side:**
- `GuiRemoteProtocolFilterVerifier` — validates repeat-filtering invariants.
- `GuiRemoteProtocolFilter` — filters redundant messages.
- `GuiRemoteProtocolDomDiffConverter` — (optional, when `useDomDiff` is true) converts full DOM to DOM diffs.

When `useChannel == UnitTestRemoteChannel::None`, the verifier directly wraps `UnitTestRemoteProtocol`'s `IGuiRemoteProtocol`, bypassing the serialization layers for speed. Otherwise, the full serialization channel is used for testing round-trip fidelity.

Finally, `SetupRemoteNativeController(protocol)` creates the runtime stack: `GuiRemoteController` → `GuiHostedController` → resource managers, then calls `GuiApplicationMain()` → `GuiMain()` → the registered test proxy.

### Async Mode (GacUIUnitTest_StartAsync)

`GacUIUnitTest_StartAsync` inserts `GuiRemoteProtocolAsyncJsonChannelSerializer` into the channel, placing the core and renderer on separate threads. Two threads are spawned via `RunInNewThread`: a channel thread for serialization I/O and a UI thread for the GacUI application. The call waits for `asyncChannelSender.WaitForStopped()` before writing snapshots.

## UnitTestRemoteProtocol Class Hierarchy

`UnitTestRemoteProtocol` inherits from `UnitTestRemoteProtocolFeatures` and `IGuiRemoteEventProcessor`. Its base classes are:
- `UnitTestRemoteProtocolBase` — holds `IGuiRemoteProtocolEvents*`, `UnitTestScreenConfig`, default `Impl_*` stubs, `Initialize`, `GetExecutablePath`.
- `UnitTestRemoteProtocol_MainWindow` — simulates window management (bounds, sizing config, styles).
- `UnitTestRemoteProtocol_IO` — simulates controller connection and IO event forwarding.
- `UnitTestRemoteProtocol_Rendering` — simulates element creation/destruction, rendering begin/end, DOM capture.
- `UnitTestRemoteProtocol_IOCommands` — provides high-level input simulation methods.
- `UnitTestRemoteProtocolFeatures` — combines all feature bases and implements `LogRenderingResult()`, the core frame capture mechanism.

`UnitTestRemoteProtocol` adds:
- `processRemoteEvents` — a list of `(Nullable<WString>, Func<void()>)` pairs representing named frame callbacks.
- `OnNextIdleFrame(callback)` and `OnNextIdleFrame(name, callback)` — register callbacks to execute after specific rendering frames settle.
- `ProcessRemoteEvents()` — the main event processing method called each cycle.

## The Main Event Loop

`GuiRemoteController::RunOneCycle()` drives the core side per cycle:
1. `remoteProtocol->GetRemoteEventProcessor()->ProcessRemoteEvents()` — this invokes `UnitTestRemoteProtocol::ProcessRemoteEvents()`.
2. `remoteMessages.Submit(disconnected)` — flushes pending protocol messages.
3. `callbackService.InvokeGlobalTimer()` — triggers `GuiHostedController::GlobalTimer()`, which performs layout and rendering, generating new DOM/element data.
4. `asyncService.ExecuteAsyncTasks()`.

The cycle repeats until `connectionStopped` is true (application closes).

## Frame Capture and Snapshot Mechanism

### Passive Rendering Capture (UnitTestRemoteProtocol_Rendering)

Each time the GacUI core calls `RequestRendererBeginRendering`, `UnitTestRemoteProtocol_Rendering::Impl_RendererBeginRendering` creates a new `UnitTestLoggedFrame` and records the `frameId` from the core's monotonic counter. When `Impl_RendererEndRendering` is called, the frame's `renderingDom` is captured from the rendering commands.

### Active Frame Logging (LogRenderingResult)

`LogRenderingResult()` is called at the start of each `ProcessRemoteEvents()` cycle in `UnitTestRemoteProtocolFeatures`:
1. `TryGetLastRenderingFrameAndReset()` checks if a rendering frame completed since the last check. If yes, it becomes the `candidateFrame`.
2. On the next cycle where no new rendering occurred (the UI has settled), the `candidateFrame` is committed to `loggedTrace.frames` with a copy of `lastElementDescs` and `sizingConfig`. This ensures the snapshot captures the final stable rendering state after all layout passes.
3. `LogRenderingResult()` returns `true`, signaling that a frame is ready.
4. In `ProcessRemoteEvents()`, the `frameName` is set on the just-committed frame (from the callback's registered name), and then the callback function executes.
5. If 100 consecutive cycles pass without any rendering change after previously rendering, the test fails with an error.

### Frame Name Semantics

The frame name is assigned to the **already-committed snapshot** (the rendering result), then the callback executes. This means:
- The `frameName` describes what **led to** the current visual state, not what the callback will do next.
- The first frame is conventionally named `"Ready"`, representing the initial rendering state after the window opens.
- Subsequent frame names describe the action taken in the previous callback (e.g., `"Hover"` means the previous callback moved the mouse, and this snapshot shows the hover state).

The sequence is:
1. Application renders → DOM/elements captured as `candidateFrame`.
2. UI settles (no more rendering) → `candidateFrame` committed to `loggedTrace.frames`.
3. `frameName` is set on the committed frame from the callback registration.
4. Frame callback executes (may trigger events → triggering more rendering).
5. Return to step 1.

### Snapshot File Structure

For a test named `Controls/Basic/GuiButton/ClickOnMouseUp`:
- **Main trace file**: `{snapshotFolder}/Controls/Basic/GuiButton/ClickOnMouseUp.json` — `UnitTest_RenderingTrace` with `createdElements`, `imageCreations`, `imageMetadatas`, and `frames` (each frame only has `frameId` and `frameName`, detail stripped).
- **Per-frame files**: `{snapshotFolder}/Controls/Basic/GuiButton/ClickOnMouseUp/frame_0.json`, `frame_1.json`, etc. — full `UnitTest_RenderingFrame` with `frameId`, `frameName`, `windowSize`, `elements` (all rendered element descriptions keyed by ID), and `root` (the rendering DOM tree).
- **Workflow snapshot**: `ClickOnMouseUp[x64].txt` or `[x86].txt` — compiled Workflow script text for generation stability verification.
- **Commands log**: `ClickOnMouseUp[commands].txt` — rendering command logs per frame (when not using DOM diff).
- **Diffs log**: `ClickOnMouseUp[diffs].txt` — DOM diff logs per frame.

### Write Strategy (GacUIUnitTest_LogUI)

`GacUIUnitTest_LogUI` performs the following:
1. Serializes the full `loggedTrace` to JSON and verifies round-trip fidelity (serialize → deserialize → re-serialize must match).
2. Writes each frame as an individual `frame_N.json` file via `GacUIUnitTest_WriteSnapshotFileIfChanged`, which only writes if content differs from the existing file (enabling clean `git diff`).
3. Strips frame detail from the in-memory trace (keeping only `frameId` and `frameName`) and writes the main trace file.
4. Deletes any leftover `frame_*.json` files that no longer correspond to actual frames.

## User Interaction Simulation

### UnitTestRemoteProtocol_IOCommands

`UnitTestRemoteProtocol_IOCommands` provides methods to simulate all user input. It maintains virtual state:
- `mousePosition` — current mouse position (`Nullable<NativePoint>`, initially unset; first `MouseMove` triggers `OnIOMouseEntered`).
- `pressingKeys` — `SortedList<VKEY>` of currently pressed keys.
- `leftPressing`, `middlePressing`, `rightPressing` — mouse button states.
- `capslockToggled` — capslock toggle state.

Each method constructs the appropriate `NativeWindowMouseInfo`, `NativeWindowKeyInfo`, or `NativeWindowCharInfo` struct (via `MakeMouseInfo()`, `MakeKeyInfo()`, `MakeCharInfo()`) and calls the corresponding event on `IGuiRemoteProtocolEvents` (obtained via `UseEvents()`).

### Location Calculation (LocationOf)

`LocationOf(controlOrComposition, ratioX, ratioY, offsetX, offsetY)` computes the target point in absolute screen coordinates:
1. Gets the composition's `GetGlobalBounds()` (logical coordinates).
2. Converts to native coordinates using `nativeWindow->Convert()`.
3. Applies the ratio (default 0.5 = center) and offset.
4. Adds the window's screen position `nativeWindow->GetBounds().LeftTop()`.

Overloads accept either `GuiGraphicsComposition*` or `GuiControl*`.

### Mouse Input Methods

- `MouseMove(location)` — sends `OnIOMouseMoving`. If mouse was previously unset, first sends `OnIOMouseEntered`.
- `_LDown(location)` / `_LUp(location)` — low-level left button down/up. `_LDown` calls `MouseMove` if position changed, then `UseEvents().OnIOButtonDown({Left, ...})`.
- `LClick(location)` — `_LDown` followed by `_LUp`.
- `LDBClick(location)` — two `LClick` calls (the framework detects double click from timing).
- Analogous `RClick`, `MClick`, `RDBClick`, `MDBClick` for right and middle buttons.
- `WheelUp(jumps)` / `WheelDown(jumps)` — `OnIOVWheel` with delta scaled by 120 per jump.
- `HWheelLeft(jumps)` / `HWheelRight(jumps)` — `OnIOHWheel` similarly.

### Key and Character Input Methods

- `KeyDown(key)` / `KeyUp(key)` — sends `OnIOKeyDown` / `OnIOKeyUp`. Tracks `pressingKeys` state. Special handling for `VKEY::KEY_CAPITAL` toggles `capslockToggled`.
- `KeyPress(key)` — `KeyDown` followed by `KeyUp`.
- `KeyPress(key, ctrl, shift, alt)` — wraps the key press with modifier key down/up events.
- `TypeString(text)` — sends a sequence of `OnIOChar` events for each character via `MakeCharInfo`, without synthesizing key down/up events.

### Event Flow Through the Pipeline

When a test calls `protocol->LClick(location)`:
1. `_LDown(location)` → `MouseMove(location)` if position changed → `UseEvents().OnIOButtonDown({Left, MakeMouseInfo()})`.
2. `_LUp(location)` → `UseEvents().OnIOButtonUp({Left, MakeMouseInfo()})`.
3. These events reach `GuiRemoteEvents` (the `IGuiRemoteProtocolEvents` implementation in `GuiRemoteController`).
4. `GuiRemoteEvents::OnIOButtonDown()` dispatches to `listener->LeftButtonDown(info)` on `remoteWindow`.
5. `GuiGraphicsHost` receives the call as `INativeWindowListener`.
6. `GuiGraphicsHost::LeftButtonDown()` calls `OnMouseInput()`, which converts native coordinates to logical coordinates, performs hit-testing via `FindVisibleComposition()`, creates `GuiMouseEventArgs`, and dispatches via `RaiseMouseEvent()`.
7. The event bubbles through the composition tree, triggering control-level handlers.

All of this happens **synchronously within a single `ProcessRemoteEvents()` call**, so by the time the frame callback returns, all side effects of the interaction have been processed.

### Blocking Function Caveat

If an IO action triggers a blocking function (like `ShowDialog`), the frame callback would never return. The unit test framework detects this via the `frameExecuting` flag and fails. The solution is to wrap the IO action in `GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); })`, which defers the action to the next event loop iteration.

## Running Multiple Test Applications

Each call to `GacUIUnitTest_Start` runs one complete application lifecycle (from `GuiMain()` to window close). Typically each `TEST_CASE` calls the full `GacUIUnitTest_SetGuiMainProxy` → `GacUIUnitTest_Start` sequence independently.

Within a single application instance, multiple windows can be created inside frame callbacks using `Value::Create(L"namespace::ClassName")` and shown with `Show()`, `ShowModal()`, or `ShowDialog()`.

`GacUIUnitTest_LinkGuiMainProxy` can be called multiple times before `GacUIUnitTest_Start`, chaining multiple setup layers (e.g., one for resources, one for theme, one for additional initialization).
