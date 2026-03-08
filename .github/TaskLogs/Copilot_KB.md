# !!!KNOWLEDGE BASE!!!

# DESIGN REQUEST

You are going to research about the remote protocol mode. Remote protocol delegates rendering and OS provider to a command-based protocol. But in this topic, ignore actual protocol serialization/deserialization, ignore GuiSingleRendererSingle, ignore unit test that builds on top of remote protocol. You are going to finish a design explanation of how remote protocol is implemented, focus on the starting/stopping, connection establishing, and other details as well as the API design. Those "ignored" parts will be covered in another future topic, here let us focus on the core part.

# INSIGHT

## Overview

Remote protocol mode allows a GacUI application to delegate rendering and OS-level services (window management, input, screen) to the renderer side through a command-based protocol. The GacUI application logic runs on the core side, but all visual output and user input are communicated through `IGuiRemoteProtocol`. Remote mode always runs on top of hosted mode (`GuiHostedController` wraps `GuiRemoteController`).

The implementation lives in `Source/PlatformProviders/Remote/` with these key files:
- `GuiRemoteProtocol_Shared.h` — defines `IGuiRemoteProtocol`, `IGuiRemoteProtocolEvents`, `IGuiRemoteProtocolMessages`, `IGuiRemoteProtocolConfig`, `IGuiRemoteEventProcessor`, combinator base classes
- `GuiRemoteProtocol.h` — umbrella include for all protocol layers
- `GuiRemoteController.h/.cpp` — the core `GuiRemoteController` implementing `INativeController` via protocol
- `GuiRemoteControllerSetup.cpp` — `SetupRemoteNativeController()` entry point
- `GuiRemoteWindow.h/.cpp` — `GuiRemoteWindow` implementing `INativeWindow` via protocol messages
- `GuiRemoteEvents.h/.cpp` — `GuiRemoteMessages` (request/response tracking) and `GuiRemoteEvents` (event dispatching)
- `GuiRemoteGraphics.h/.cpp` — `GuiRemoteGraphicsRenderTarget` and `GuiRemoteGraphicsResourceManager`
- `GuiRemoteGraphics_ImageService.h/.cpp` — `GuiRemoteGraphicsImageService` and `GuiRemoteGraphicsImage`
- `GuiRemoteProtocol_Filter.h/.cpp` — `GuiRemoteProtocolFilter` for dropping repeated/consecutive messages and events
- `GuiRemoteProtocol_FilterVerifier.h/.cpp` — `GuiRemoteProtocolFilterVerifier` for verifying filter correctness
- `GuiRemoteProtocol_DomDiff.h/.cpp` — `GuiRemoteProtocolDomDiffConverter` for converting rendering commands to DOM diff
- `GuiRemoteProtocol_Channel_Shared.h` — `IGuiRemoteProtocolChannel<T>` and channel transformer base
- `GuiRemoteProtocol_Channel_Json.h/.cpp` — `GuiRemoteProtocolFromJsonChannel` / `GuiRemoteJsonChannelFromProtocol` for JSON-based protocol
- `GuiRemoteProtocol_Channel_Async.h/.cpp` — `GuiRemoteProtocolAsyncChannelSerializer<T>` for running protocol IO on a separate thread
- `GuiRemoteProtocolSchemaShared.h` — JSON serialization helpers, `ArrayMap`
- `Protocol/` — protocol definition text files and generated schema code

## Protocol Definition

Protocol messages, events, and data types are defined in `Protocol/*.txt` files:
- `Protocols.txt` — lists all protocol files to include
- `Protocol_Controller.txt` — controller lifecycle messages: `ControllerGetFontConfig`, `ControllerGetScreenConfig`, `ControllerConnectionEstablished`, `ControllerConnectionStopped`; and data types `FontConfig`, `ScreenConfig`, `ControllerGlobalConfig`
- `Protocol_MainWindow.txt` — window management messages: `WindowGetBounds`, `WindowNotifySet*` (Title, Enabled, TopMost, ShowInTaskBar, CustomFrameMode, MaximizedBox, MinimizedBox, Border, SizeBox, IconVisible, TitleBar, Bounds, ClientSize), `WindowNotifyActivate`, `WindowNotifyShow`, `WindowNotifyMinSize`, `WindowNotifySetCaret`; and events `WindowBoundsUpdated`, `WindowActivatedUpdated`
- `Protocol_IO.txt` — IO messages: `IOUpdateGlobalShortcutKey`, `IORequireCapture`, `IOReleaseCapture`, `IOIsKeyPressing`, `IOIsKeyToggled`; and IO events: `IOButtonDown`, `IOButtonDoubleClick`, `IOButtonUp`, `IOHWheel`, `IOVWheel`, `IOMouseMoving`, `IOMouseEntered`, `IOMouseLeaved`, `IOKeyDown`, `IOKeyUp`, `IOChar`, `IOGlobalShortcutKey`
- `Protocol_Renderer.txt` — renderer messages: `RendererCreated`, `RendererDestroyed`, `RendererBeginRendering`, `RendererEndRendering`, `RendererBeginBoundary`, `RendererEndBoundary`, `RendererRenderElement`; plus DOM messages `RendererRenderDom`, `RendererRenderDomDiff`
- `Protocol_SyncDom.txt` — defines `RenderingDom`, `RenderingDom_Diff`, `RenderingDom_DiffsInOrder`

The generated code is in `Protocol/Generated/GuiRemoteProtocolSchema.h` which produces:
- `GACUI_REMOTEPROTOCOL_MESSAGES(HANDLER)` macro — lists all messages with `HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)` format. `REQTAG` is `REQ`/`NOREQ`, `RESTAG` is `RES`/`NORES`, `DROPTAG` is `NODROP`/`DROPREP` (drop repeat)
- `GACUI_REMOTEPROTOCOL_EVENTS(HANDLER)` macro — lists all events with `HANDLER(NAME, REQUEST, REQTAG, DROPTAG)` format. `DROPTAG` is `NODROP`/`DROPREP` (drop repeat)/`DROPCON` (drop consecutive)

Messages are **requests from the core side to the renderer side** (optionally with responses). Events are **notifications from the renderer side to the core side**.

## Core Protocol Interfaces

### IGuiRemoteProtocolMessages

Generated via `GACUI_REMOTEPROTOCOL_MESSAGES`, has pure virtual methods for each message:
- `RequestNAME()` or `RequestNAME(id)` or `RequestNAME(arguments)` or `RequestNAME(id, arguments)` — depending on whether the message has a request payload and/or expects a response (id is needed for matching responses)

### IGuiRemoteProtocolEvents

Generated via `GACUI_REMOTEPROTOCOL_EVENTS` and `GACUI_REMOTEPROTOCOL_MESSAGES`, has:
- `OnNAME()` or `OnNAME(arguments)` for each event
- `RespondNAME(id, arguments)` for each message that has a response — this is how the renderer side delivers responses back

### IGuiRemoteProtocolConfig

- `GetExecutablePath()` — returns the executable path, used by `INativeController::GetExecutablePath()`

### IGuiRemoteProtocol

Combines `IGuiRemoteProtocolConfig` and `IGuiRemoteProtocolMessages`, plus:
- `Initialize(IGuiRemoteProtocolEvents* events)` — gives the protocol the event callback interface
- `Submit(bool& disconnected)` — flushes all queued messages and waits for responses; sets `disconnected` on failure
- `GetRemoteEventProcessor()` — returns `IGuiRemoteEventProcessor` (or `nullptr`), whose `ProcessRemoteEvents()` is called to pump incoming events

### IGuiRemoteEventProcessor

- `ProcessRemoteEvents()` — processes any pending incoming events from the renderer side; called from `GuiRemoteController::RunOneCycle()`

## GuiRemoteMessages and GuiRemoteEvents

### GuiRemoteMessages

Wraps `IGuiRemoteProtocol` with an auto-incrementing `id` counter for request/response matching:
- For messages with response: `RequestNAME(...)` allocates an `id`, calls `remoteProtocol->RequestNAME(id, ...)`, returns the `id`
- For messages without response: `RequestNAME(...)` calls `remoteProtocol->RequestNAME(...)`
- `Submit(bool& disconnected)` calls `remoteProtocol->Submit(disconnected)`
- `RespondNAME(id, arguments)` stores the response in a `Dictionary<vint, RESPONSE>`
- `RetrieveNAME(id)` removes and returns the response

The call pattern is: queue one or more requests → `Submit()` → retrieve responses by id. `Submit()` is a blocking synchronous call that flushes all queued messages and waits for all responses.

### GuiRemoteEvents

Implements `IGuiRemoteProtocolEvents`:
- Response methods delegate to `GuiRemoteMessages::RespondNAME(id, arguments)`
- Event methods dispatch to the appropriate handlers:
  - `OnControllerConnect(globalConfig)` → calls `remoteMessages.RequestControllerConnectionEstablished()`, then `remote->OnControllerConnect(globalConfig)`, then `Submit()`
  - `OnControllerDisconnect()` → `remote->OnControllerDisconnect()`
  - `OnControllerRequestExit()` → `remote->OnControllerRequestExit()`
  - `OnControllerForceExit()` → `remote->OnControllerForceExit()`
  - `OnControllerScreenUpdated(screenConfig)` → `remote->OnControllerScreenUpdated(screenConfig)`
  - `OnWindowBoundsUpdated(sizingConfig)` → `remote->remoteWindow.OnWindowBoundsUpdated(sizingConfig)`
  - `OnWindowActivatedUpdated(activated)` → `remote->remoteWindow.OnWindowActivatedUpdated(activated)`
  - `OnIOGlobalShortcutKey(id)` → `remote->callbackService.InvokeGlobalShortcutKeyActivated(id)`
  - `OnIOButtonDown/DoubleClick/Up(info)` → dispatches to `remoteWindow.listeners` based on `info.button` (Left/Middle/Right)
  - `OnIOHWheel/VWheel/MouseMoving/MouseEntered/MouseLeaved(info)` → dispatches to `remoteWindow.listeners`
  - `OnIOKeyDown/KeyUp/Char(info)` → dispatches to `remoteWindow.listeners`
- `ClearResponses()` is available to clear all pending responses

## GuiRemoteController Architecture

`GuiRemoteController` is the core class. It inherits from:
- `Object`
- `INativeController` — to serve as the native controller for the GacUI framework
- `INativeResourceService` (protected) — font and cursor services
- `INativeInputService` (protected) — timer, key state, global shortcut keys
- `INativeScreenService` / `INativeScreen` (protected) — virtual single screen
- `INativeWindowService` (protected) — single-window creation and run loop

Key members:
- `IGuiRemoteProtocol* remoteProtocol` — the underlying protocol implementation
- `GuiRemoteMessages remoteMessages` — request/response tracking wrapper
- `GuiRemoteEvents remoteEvents` — event dispatcher (implements `IGuiRemoteProtocolEvents`)
- `GuiRemoteWindow remoteWindow` — the single virtual native window
- `GuiRemoteGraphicsResourceManager* resourceManager` — set after construction
- `SharedCallbackService callbackService` — framework callback service (timer, window lifecycle, shortcut)
- `SharedAsyncService asyncService` — async task execution service
- `GuiRemoteGraphicsImageService imageService` — remote image service
- `bool applicationRunning` — true during `Run()` message loop
- `bool connectionForcedToStop` — set by `OnControllerForceExit()`
- `bool connectionStopped` — set by `DestroyNativeWindow()`, ends the run loop
- `ControllerGlobalConfig remoteGlobalConfig` — received from `OnControllerConnect`, contains `documentCaretFromEncoding`
- `FontConfig remoteFontConfig` — fetched from `ControllerGetFontConfig` during connect
- `ScreenConfig remoteScreenConfig` — fetched from `ControllerGetScreenConfig` during connect

### Service Delegation

`GuiRemoteController` implements `INativeController` with service dispatch:
- `CallbackService()` → core side `SharedCallbackService`
- `ResourceService()` → itself (implements `INativeResourceService`)
- `AsyncService()` → core side `SharedAsyncService`
- `ClipboardService()` → `nullptr` (use `FakeClipboardService`)
- `ImageService()` → core side `GuiRemoteGraphicsImageService`
- `InputService()` → itself (implements `INativeInputService`)
- `DialogService()` → `nullptr` (use `FakeDialogServiceBase`)
- `ScreenService()` → itself (implements `INativeScreenService`)
- `WindowService()` → itself (implements `INativeWindowService`)

Unlike hosted mode with a real native controller underneath, everything in remote mode is virtual. Services that return `nullptr` cause GacUI to fall back to built-in fake implementations (`FakeDialogServiceBase`, `FakeClipboardService`).

### INativeResourceService Implementation

- `GetSystemCursor(type)` — creates and caches `GuiRemoteCursor` objects (core side stubs, only `SystemCursorType` stored)
- `GetDefaultFont()` / `SetDefaultFont()` — reads/writes `remoteFontConfig.defaultFont`
- `EnumerateFonts(fonts)` — copies from `remoteFontConfig.supportedFonts`

### INativeInputService Implementation

- `StartTimer()` / `StopTimer()` / `IsTimerEnabled()` — toggles core side `timerEnabled` flag
- `IsKeyPressing(code)` / `IsKeyToggled(code)` — synchronous request/submit/retrieve pattern via `remoteMessages.RequestIOIsKeyPressing` / `RequestIOIsKeyToggled` with immediate `Submit()`
- Key name mapping: lazy-initialized `keyNames` / `keyCodes` dictionaries from `GUI_DEFINE_KEYBOARD_WINDOWS_NAME` macro
- `RegisterGlobalShortcutKey(ctrl, shift, alt, key)` — adds to core side `hotKeySet` / `hotKeyIds`, sends `RequestIOUpdateGlobalShortcutKey`, submits
- `UnregisterGlobalShortcutKey(id)` — removes from core side tracking, sends update, submits

### INativeScreenService / INativeScreen Implementation

- `GetScreenCount()` returns 1
- `GetScreen()` returns itself
- `GetBounds()` / `GetClientBounds()` return `remoteScreenConfig.bounds` / `.clientBounds`
- `GetScalingX()` / `GetScalingY()` return `remoteScreenConfig.scalingX` / `.scalingY`

### INativeWindowService Implementation

- `GetMainWindowFrameConfig()` / `GetNonMainWindowFrameConfig()` — both return `NativeWindowFrameConfig::Default` (since hosted mode handles frame configs)
- `CreateNativeWindow(windowMode)` — can only be called once (CHECK_ERROR). Sets `windowCreated`, assigns `windowMode` to `remoteWindow`, fires `InvokeNativeWindowCreated`, returns `&remoteWindow`
- `DestroyNativeWindow(window)` — can only be called once (CHECK_ERROR). Fires `Closed`, `Destroying`, `InvokeNativeWindowDestroying`, `Destroyed` on listeners. Sets `connectionStopped = true`
- `GetMainWindow()` — returns `&remoteWindow` if created and not destroyed
- `Run(window)` — sets `applicationRunning = true`, calls `window->Show()`, loops on `RunOneCycle()`, executes remaining async tasks, sets `applicationRunning = false`
- `RunOneCycle()` — if not `connectionStopped`: calls `processor->ProcessRemoteEvents()` if available, calls `remoteMessages.Submit()`, if `timerEnabled` and not disconnected calls `callbackService.InvokeGlobalTimer()`, executes async tasks. Returns `!connectionStopped`

## Connection Lifecycle

### Entry Point: SetupRemoteNativeController (GuiRemoteControllerSetup.cpp)

```
int SetupRemoteNativeController(IGuiRemoteProtocol* protocol)
```

1. Creates `GuiRemoteController remoteController(protocol)` on stack
2. Creates `GuiHostedController hostedController(&remoteController)` wrapping it
3. Creates `GuiRemoteGraphicsResourceManager remoteResourceManager(&remoteController, &hostedController)` — remote render target
4. Creates `GuiHostedGraphicsResourceManager hostedResourceManager(&hostedController, &remoteResourceManager)` — hosted graphics wrapper
5. Sets globals: `SetNativeController(&hostedController)`, `SetGuiGraphicsResourceManager(&hostedResourceManager)`, `SetHostedApplication(hostedController.GetHostedApplication())`
6. Initialization sequence: `remoteController.Initialize()` → `remoteResourceManager.Initialize()` → `hostedController.Initialize()`
7. Runs `GuiApplicationMain()`
8. Finalization sequence: `hostedController.Finalize()` → `remoteResourceManager.Finalize()` → `remoteController.Finalize()`
9. Clears globals and returns 0

### Initialize

`GuiRemoteController::Initialize()`:
- Calls `remoteProtocol->Initialize(&remoteEvents)` — gives the protocol the event handler
- Calls `imageService.Initialize()`

### Connection Establishing (OnControllerConnect)

The renderer side fires `OnControllerConnect(ControllerGlobalConfig)` event. `GuiRemoteEvents::OnControllerConnect()`:
1. Sends `RequestControllerConnectionEstablished()` notification to the renderer side
2. Calls `remote->OnControllerConnect(globalConfig)`

`GuiRemoteController::OnControllerConnect(globalConfig)`:
1. Stores `remoteGlobalConfig = globalConfig` (contains `documentCaretFromEncoding`)
2. Calls `UpdateGlobalShortcutKey()` to resend hotkey registrations
3. Requests `ControllerGetFontConfig` and `ControllerGetScreenConfig` in the same batch
4. Calls `Submit()` to flush and receive responses
5. Stores `remoteFontConfig` and `remoteScreenConfig` from responses
6. Calls `remoteWindow.OnControllerConnect()`, `imageService.OnControllerConnect()`, `resourceManager->OnControllerConnect()`
7. Calls `callbackService.InvokeEnvironmentChanged()`

`GuiRemoteWindow::OnControllerConnect()`:
- Clears `controllerDisconnected` flag
- Sets `sizingConfigInvalidated = true`
- Sends `RequestWindowNotifySetBounds`, then `RequestGetBounds()` (synchronous submit + retrieve)
- Fires `DpiChanged(true)` / `DpiChanged(false)` on listeners (workaround for `GuiWindow::UpdateCustomFramePadding`)
- If `applicationRunning`: re-sends all window style properties (title, enabled, topMost, border options, etc.) and capture state

### Disconnection (OnControllerDisconnect)

The renderer side fires `OnControllerDisconnect()`. This calls:
- `remoteWindow.OnControllerDisconnect()` — sets `controllerDisconnected = true`
- `imageService.OnControllerDisconnect()`
- `resourceManager->OnControllerDisconnect()`

### Graceful Exit (OnControllerRequestExit)

The renderer side fires `OnControllerRequestExit()`. This calls `remoteWindow.Hide(true)`, which:
1. Fires `BeforeClosing(cancel)` on listeners — if any cancels, returns without closing
2. Fires `AfterClosing()` on listeners
3. Schedules `DestroyNativeWindow` via `AsyncService()->InvokeInMainThread()`

### Forced Exit (OnControllerForceExit)

The renderer side fires `OnControllerForceExit()`. Sets `connectionForcedToStop = true`, then calls `remoteWindow.Hide(true)`. The `connectionForcedToStop` flag bypasses the `BeforeClosing` listener check in `Hide()`.

### Finalize

`GuiRemoteController::Finalize()`:
- Calls `imageService.Finalize()`
- Sends `RequestControllerConnectionStopped()` message to the renderer side
- Calls `Submit()` to flush

### Run Loop

`GuiRemoteController::Run(window)`:
1. Sets `applicationRunning = true`
2. Calls `window->Show()` — triggers `ShowWithSizeState()` which sends `WindowNotifyShow` and fires `Opened()` and `SetActivated()` on the window
3. Loops on `RunOneCycle()` until `connectionStopped`
4. Executes remaining async tasks
5. Sets `applicationRunning = false`

`RunOneCycle()`:
1. If `connectionStopped`, returns false
2. If `remoteProtocol->GetRemoteEventProcessor()` exists, calls `processor->ProcessRemoteEvents()`
3. Calls `remoteMessages.Submit(disconnected)` — flushes all queued messages
4. If `timerEnabled` and not disconnected, calls `callbackService.InvokeGlobalTimer()` — this triggers hosted controller's global timer which drives rendering
5. Calls `asyncService.ExecuteAsyncTasks()`
6. Returns `!connectionStopped`

## GuiRemoteWindow

`GuiRemoteWindow` implements `INativeWindow` for the single remote window. The `GuiHostedController` wrapping `GuiRemoteController` sees `GuiRemoteWindow` as a native window.

Key characteristics:
- `IsActivelyRefreshing()` returns `true` — tells hosted mode to always render
- `GetRenderingOffset()` returns `{0,0}` — rendering offset is handled by hosted mode
- Coordinate conversion uses `scalingX`/`scalingY` received from the renderer side screen config
- `GetBounds()` and `GetClientSize()` use `remoteWindowSizingConfig` (received from the renderer side), and if `sizingConfigInvalidated` is true, triggers synchronous `RequestGetBounds()`
- `SetBounds()` / `SetClientSize()` update core side config and send `WindowNotifySetBounds` / `WindowNotifySetClientSize`, setting `sizingConfigInvalidated = true`
- Style changes (title, border, enabled, etc.) use `SET_REMOTE_WINDOW_STYLE` / `SET_REMOTE_WINDOW_STYLE_INVALIDATE` macros that check for value change and send the corresponding `WindowNotifySet*` message. The `_INVALIDATE` variant also sets `sizingConfigInvalidated = true` for style changes that affect window bounds.
- `SetParent()`, `EnableActivate()`, `DisableActivate()` → `CHECK_FAIL` — not supposed to be called, since hosted controller manages these
- `Show()` / `ShowMaximized()` / `ShowMinimized()` / `ShowRestored()` / `ShowDeactivated()` → call `ShowWithSizeState()` which sends `WindowNotifyShow` with activate flag and size state, then fires `Opened()` and `SetActivated()` on the core side
- `Hide(true)` — unless `connectionForcedToStop`, iterates `BeforeClosing(cancel)` / `AfterClosing()` on listeners, then schedules `DestroyNativeWindow` via async main thread invocation
- `RequireCapture()` / `ReleaseCapture()` — toggle `statusCapturing`, send `IORequireCapture` / `IOReleaseCapture` with immediate `Submit()`
- `SetCaretPoint(point)` — stores on the core side and sends `WindowNotifySetCaret`
- `RedrawContent()` — no-op (rendering is driven by hosted controller's global timer)

### Reconnection Support

`GuiRemoteWindow::OnControllerConnect()` resends all current window state to the renderer side if `applicationRunning` is true. This enables reconnection scenarios where the renderer side reconnects and the core side restores its visual state.

## GuiRemoteGraphicsResourceManager

`GuiRemoteGraphicsResourceManager` inherits from `GuiGraphicsResourceManager` and `IGuiGraphicsLayoutProvider`. Key members:
- `GuiRemoteController* remote`
- `GuiRemoteGraphicsRenderTarget renderTarget` — the remote render target
- `GuiHostedController* hostedController` — for refresh requests

Service methods:
- `GetRenderTarget(window)` — returns `&renderTarget` (single render target)
- `RecreateRenderTarget(window)` / `ResizeRenderTarget(window)` — no-ops (managed by hosted controller)
- `GetLayoutProvider()` — returns itself
- `CreateRawElement()` — creates a `GuiRemoteGraphicsElement`

The `GuiHostedGraphicsResourceManager` in the chain wraps this, providing the same single render target for all hosted windows.

## GuiRemoteGraphicsRenderTarget

Inherits from `GuiGraphicsRenderTarget`. Manages remote rendering state:
- `canvasSize` — current canvas size from `remoteWindow.GetClientSize()`
- `usedFrameIds` / `usedElementIds` / `usedCompositionIds` — auto-incrementing ID counters
- `renderers` — map of element ID to `IGuiRemoteProtocolElementRender*`
- `createdRenderers` / `destroyedRenderers` — pending create/destroy batches
- `renderersAskingForCache` — renderers needing min size from the renderer side
- `needFullElementUpdateOnNextFrame` — set on reconnect
- `fontHeights` — cached font height measurements from the renderer side
- `hitTestResults` / `cursors` — stacks for tracking rendering boundaries

`StartRenderingOnNativeWindow()`:
1. Gets canvas size from remote window
2. Increments `usedFrameIds`, builds `ElementBeginRendering` with updated elements
3. If `needFullElementUpdateOnNextFrame` (reconnect mode), collects all elements' updates
4. Otherwise collects only changed elements
5. Sends `RequestRendererBeginRendering`

`StopRenderingOnNativeWindow()`:
1. Sends `RequestRendererEndRendering`, submits, retrieves `ElementMeasurings` response
2. Processes `fontHeights`, `minSizes`, `createdImages`, `inlineObjectBounds` from response
3. If min sizes changed, calls `hostedController->RequestRefresh()`
4. Returns `ResizeWhileRendering` if canvas size changed during rendering, otherwise `None`

Clipper push/pop methods (`AfterPushedClipper`, `AfterPoppedClipper`) send `RequestRendererBeginBoundary` / `RequestRendererEndBoundary` messages tracking hit test results and cursor types per composition boundary.

`OnControllerConnect()` — on reconnect, re-registers all existing renderers and paragraphs via `RequestRendererCreated`, sets `needFullElementUpdateOnNextFrame = true`, marks all paragraphs dirty.

## Protocol Combinator Layer

### GuiRemoteEventCombinator

Base class with `IGuiRemoteProtocolEvents* targetEvents`. Subclasses override event/response methods and optionally forward to `targetEvents`.

### GuiRemoteProtocolCombinator<TEvents>

Template wrapping `IGuiRemoteProtocol* targetProtocol` with a `TEvents eventCombinator`. `Initialize()` chains: `eventCombinator.targetEvents = _events`, then `targetProtocol->Initialize(&eventCombinator)`. This creates a pipeline where events flow through the combinator before reaching the final handler.

### Passing-Through Variants

`GuiRemoteEventCombinator_PassingThrough` and `GuiRemoteProtocolCombinator_PassingThrough<TEvents>` — forward all messages and events unchanged. Subclasses selectively override specific methods.

## Protocol Filter Layer (GuiRemoteProtocol_Filter.h)

### GuiRemoteProtocolFilter

Wraps `IGuiRemoteProtocol` to optimize protocol traffic by dropping redundant messages:
- Messages annotated `[@DropRepeat]` in protocol definitions: if the same message is sent with the same arguments, the repeated one is dropped. Tracked via `lastDropRepeatRequest*` indices.
- Events annotated `[@DropRepeat]` or `[@DropConsecutive]`: repeated or consecutive duplicate events are dropped before delivery. Tracked via `lastDropRepeatEvent*` / `lastDropConsecutiveEvent*` indices.

Uses `FilteredRequest` / `FilteredResponse` / `FilteredEvent` structs that store messages in a variant-typed queue with drop tracking.

`Submit()` calls `ProcessRequests()` first (filters, then forwards non-dropped requests), then delegates to `targetProtocol->Submit()`.

### GuiRemoteProtocolFilterVerifier

A separate combinator that verifies the filter's correctness. It tracks whether messages/events should have been dropped and validates.

## Protocol DOM Diff Layer (GuiRemoteProtocol_DomDiff.h)

### GuiRemoteProtocolDomDiffConverter

Wraps `IGuiRemoteProtocol`. Intercepts rendering messages and converts them from per-frame rendering commands into a DOM diff format:
- Overrides `RequestRendererBeginRendering`, `RequestRendererEndRendering`, `RequestRendererBeginBoundary`, `RequestRendererEndBoundary`, `RequestRendererRenderElement`
- Uses `RenderingDomBuilder` to build a `RenderingDom` tree during rendering
- On `RequestRendererEndRendering`:
  - If first frame (no `lastDom`): sends `RequestRendererRenderDom` with the full DOM
  - Otherwise: computes diff using `DomIndex` and sends `RequestRendererRenderDomDiff` with `RenderingDom_DiffsInOrder`
- Stores `lastDom` and `lastDomIndex` for next frame comparison
- On `OnControllerConnect`: clears `lastDom` and `lastDomIndex` to force full DOM send on reconnect

## Channel Layer (IGuiRemoteProtocolChannel<T>)

### IGuiRemoteProtocolChannel<T>

Generic bidirectional communication channel interface:
- `Initialize(IGuiRemoteProtocolChannelReceiver<T>* receiver)` — sets up the receiving end
- `GetReceiver()` — returns the receiver
- `Write(const T& package)` — sends a package
- `GetExecutablePath()` — returns executable path
- `Submit(bool& disconnected)` — flushes the channel
- `GetRemoteEventProcessor()` — returns event processor

### GuiRemoteProtocolChannelTransformerBase<TFrom, TTo>

Bridges two channel types. Implements `IGuiRemoteProtocolChannel<TFrom>` by wrapping `IGuiRemoteProtocolChannel<TTo>`. Used by `GuiRemoteProtocolChannelSerializer` and `GuiRemoteProtocolChannelDeserializer` to transform between serialized and deserialized package formats.

### JSON Channel (GuiRemoteProtocol_Channel_Json.h)

`GuiRemoteProtocolFromJsonChannel` — adapts `IGuiRemoteProtocolChannel<Ptr<JsonObject>>` to `IGuiRemoteProtocol`. Converts protocol messages to JSON objects for writing and parses incoming JSON back to typed protocol events/responses. Uses handler maps (`onReceiveEventHandlers`, `onReceiveResponseHandlers`) for efficient dispatch.

`GuiRemoteJsonChannelFromProtocol` — adapts `IGuiRemoteProtocol` to `IGuiRemoteProtocolChannel<Ptr<JsonObject>>` (the reverse direction). Intercepts protocol events and converts them to JSON for channel transport.

`JsonToStringSerializer` — provides `Serialize`/`Deserialize` between `Ptr<JsonObject>` and `WString`. Used by `GuiRemoteJsonChannelStringSerializer` / `GuiRemoteJsonChannelStringDeserializer` type aliases.

### Async Channel (GuiRemoteProtocol_Channel_Async.h)

`GuiRemoteProtocolAsyncChannelSerializer<TPackage>` — runs protocol IO on a separate channel thread while GacUI runs on the UI thread. Inherits from `GuiRemoteProtocolAsyncChannelSerializerBase` (which provides cross-thread task queues).

Threading model:
- **UI thread**: runs `uiMainProc` which calls `SetupRemoteNativeController` — the core side application loop
- **Channel thread**: runs a loop waiting for tasks from the UI thread, executing channel IO operations

`Start(_channel, _uiMainProc, startingProc)`:
1. Creates auto events (`eventAutoResponses`, `eventAutoChannelTaskQueued`) and manual events (`eventManualChannelThreadStopped`, `eventManualUIThreadStopped`)
2. `startingProc` is called with two callables: one for the channel thread, one for the UI thread. The caller is responsible for launching both threads.

`Write(package)` — called from UI thread, accumulates packages in `uiPendingPackages`

`Submit(disconnected)` — called from UI thread:
1. Checks connection availability; if disconnected, returns immediately
2. Groups all pending packages into a `PendingRequestGroup` (tracks request IDs)
3. Queues channel thread task: writes all packages to channel, calls `channel->Submit()`
4. Blocks on `eventAutoResponses` waiting for all responses
5. If disconnect detected (by `connectionCounter` mismatch), sets `disconnected`
6. Collects and delivers responses to `receiver->OnReceive()`

`ProcessRemoteEvents()` — called from UI thread:
1. Queues `channel->GetRemoteEventProcessor()->ProcessRemoteEvents()` to channel thread
2. Fetches and executes UI thread tasks
3. Processes queued events from channel thread
4. Special handling for `ControllerConnect` event: updates `connectionCounter` and `connectionAvailable`

`OnReceive(package)` — called from any thread (typically the channel thread):
- If response: stores in `queuedResponses` under lock; if all pending responses satisfied, signals `eventAutoResponses`
- If event: stores in `queuedEvents` under lock

The async channel uses `ChannelPackageSemanticUnpack()` (argument-dependent lookup) to classify incoming packages as events or responses.

Shutdown sequence:
1. `UIThreadProc` finishes → sets `stopping = true` → signals `eventAutoChannelTaskQueued`
2. Channel thread processes remaining tasks → signals `eventManualChannelThreadStopped`
3. UI thread processes remaining tasks → signals `eventManualUIThreadStopped`

The typical protocol stack for async remote operation:
`GuiRemoteProtocolDomDiffConverter` over `GuiRemoteProtocolFilter` over `GuiRemoteProtocolFromJsonChannel` over `GuiRemoteProtocolAsyncChannelSerializer<Ptr<JsonObject>>`

## Image Service

`GuiRemoteGraphicsImageService` implements `INativeImageService` remotely:
- `CreateImageFromStream(stream)` — creates a `GuiRemoteGraphicsImage` with a binary copy, assigns an ID
- `CreateImageFromFile(path)` — reads file into stream, calls `CreateImageFromStream`
- Each `GuiRemoteGraphicsImage` lazily requests its metadata from the renderer side via `ImageCreated` message when first accessed (`EnsureMetadata()`)
- `OnControllerConnect()` — re-registers all existing images on reconnect via `ImageCreated` messages
- `OnControllerDisconnect()` — resets metadata state to `Uninitialized`

## Screen Config Updates

When the renderer side notifies `OnControllerScreenUpdated(ScreenConfig)`:
- `GuiRemoteController` stores the new `remoteScreenConfig`
- `GuiRemoteWindow::OnControllerScreenUpdated()` — if scaling changed, updates `scalingX`/`scalingY` and fires `DpiChanged(true)` / `DpiChanged(false)` on listeners

# ASKS

## QUESTION

I would like you to use formal terms through out the document. Core means the side that actually running the full functionality GacUI features. Renderer or client means the rendering

### ANSWER

Throughout the document, the following formal terminology should be used consistently:

- **Core side**: The side that runs the full GacUI application logic and framework. This is where `GuiRemoteController`, `GuiRemoteWindow`, `GuiRemoteGraphicsResourceManager`, and all GacUI framework code execute. In class terms, `GuiRemoteController` and everything it owns runs on the core side.
- **Renderer side** (or **client side**): The side that performs actual rendering and provides OS-level services (window management, input, screen, image decoding). The renderer side communicates with the core side through `IGuiRemoteProtocol`.

In the protocol direction:
- **Messages** (`IGuiRemoteProtocolMessages`) are requests sent **from the core side to the renderer side**. Examples: `WindowNotifySetTitle`, `RendererBeginRendering`, `ControllerGetFontConfig`.
- **Events** (`IGuiRemoteProtocolEvents`) are notifications sent **from the renderer side to the core side**. Examples: `OnControllerConnect`, `OnWindowBoundsUpdated`, `OnIOKeyDown`.
- **Responses** are delivered **from the renderer side to the core side** via `RespondNAME(id, arguments)` methods on `IGuiRemoteProtocolEvents`, matching request IDs from messages that expect responses.

The core side drives the application loop (`GuiRemoteController::Run()`), sends rendering commands and window management messages to the renderer side, and processes events and responses received from the renderer side. The renderer side is responsible for actual pixel output, OS window creation, and forwarding user input events back to the core side.

# DRAFT

## DRAFT REQUEST

## IMPROVEMENTS

## (API|DESIGN) EXPLANATION

## DOCUMENT
