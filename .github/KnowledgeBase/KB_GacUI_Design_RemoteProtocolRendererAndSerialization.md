#### Remote Protocol Renderer and Serialization

The remote protocol renderer side receives protocol messages from the core side and translates them into native window operations and graphics rendering. The serialization and channel infrastructure provides composable layers that convert between typed protocol calls and Parser2 JSON node packages, enabling GacUI applications to run across process boundaries over any user-provided transport (named pipe, HTTP, WebSocket, etc.).

## GuiRemoteRendererSingle

`GuiRemoteRendererSingle` (in `Source/PlatformProviders/RemoteRenderer/`) is the renderer-side implementation that bridges `IGuiRemoteProtocol` to a real native window with actual graphics rendering. It is not a full `INativeController` — it relies on a real platform provider (e.g., Windows Direct2D via `SetupRawWindowsDirect2DRenderer()`) already running, and registers itself as a listener on a native window.

### Class Hierarchy

`GuiRemoteRendererSingle` inherits from:
- `Object` — standard base
- `IGuiRemoteProtocol` (virtual) — receives all protocol messages from the core side
- `INativeWindowListener` (protected virtual) — listens to native window events (mouse, keyboard, window lifecycle)
- `INativeControllerListener` (protected virtual) — listens to global timer and shortcut key events

### Key Members

- `window` / `screen` / `events`: The native window handle, current screen, and `IGuiRemoteProtocolEvents` callback for firing events to the core side.
- `availableElements`: `Dictionary<vint, Ptr<IGuiGraphicsElement>>` mapping element IDs to actual graphics elements created by the real rendering engine (e.g., `GuiSolidBorderElement`, `GuiSolidLabelElement`).
- `availableImages`: `Dictionary<vint, Ptr<INativeImage>>` mapping image IDs to loaded native images.
- `renderingDom` / `renderingDomIndex`: The rendering DOM tree received from the core side, used for on-screen rendering and hit testing.
- `solidLabelMeasurings` / `fontHeightMeasurings`: Caches and measurement tracking for text elements, supporting the measurement feedback loop.
- `pendingMouseMove`, `pendingHWheel`, `pendingVWheel`, `pendingKeyAutoDown`, `pendingWindowBoundsUpdate`: Accumulation fields for coalescing high-frequency events before sending.
- `disconnectingFromCore`: Prevents further protocol requests and events after the renderer disconnects from the core.
- `stoppedByFatalError` / `fatalError` / `fatalMaskElement` / `fatalTextElement`: Retain-mode state and graphics for displaying a fatal-error overlay instead of immediately closing the renderer.

### Source File Organization

- `GuiRemoteRendererSingle.cpp`: Construction, destruction, main window registration, connection lifecycle (`Opened`, `BeforeClosing`, `AfterClosing`, `Closed`), screen/config management, core disconnection, and fatal-error retention.
- `GuiRemoteRendererSingle_Controller.cpp`: Controller-level requests — `RequestControllerGetFontConfig`, `RequestControllerGetScreenConfig`, `RequestControllerConnectionEstablished`, `RequestControllerConnectionStopped`.
- `GuiRemoteRendererSingle_MainWindow.cpp`: Window style notifications — `RequestWindowNotifySetBounds`, `RequestWindowNotifySetTitle`, `RequestWindowNotifySetEnabled`, `RequestWindowNotifyShow`, etc.
- `GuiRemoteRendererSingle_IO.cpp`: IO requests (global shortcuts, mouse capture, key state queries) and native-to-protocol input event conversion. Contains `SendAccumulatedMessages()` for batching high-frequency events.
- `GuiRemoteRendererSingle_Rendering.cpp`: Core rendering pipeline — element creation/destruction (`RequestRendererCreated`, `RequestRendererDestroyed`), begin/end rendering, DOM rendering (`RenderDom` recursive traversal), fatal-overlay rendering, hit testing, and `GlobalTimer`/`Paint` driven refresh.
- `GuiRemoteRendererSingle_Rendering_Elements.cpp`: Property updates on ordinary graphics elements (solid border, sink border, splitter, background, gradient, inner shadow, polygon).
- `GuiRemoteRendererSingle_Rendering_Label.cpp`: Solid label measurement and property updates.
- `GuiRemoteRendererSingle_Rendering_Image.cpp`: Image creation, metadata, and image frame element updates.
- `GuiRemoteRendererSingle_Rendering_Document.cpp`: Complex document paragraph element rendering via `GuiRemoteDocumentParagraphElement` inner class.

### Rendering Pipeline

When the core sends `RequestRendererBeginRendering(ElementBeginRendering)`, the `updatedElements` field contains a list of `OrdinaryElementDescVariant`. Each variant is dispatched via `Apply(Overloading(...))` to the corresponding `RequestRendererUpdateElement_*` helper, which looks up the element by ID in `availableElements` and sets its properties.

The core sends `RequestRendererRenderDom` or `RequestRendererRenderDomDiff` to update the rendering DOM tree. The legacy command-based rendering path (`RequestRendererBeginBoundary`/`RequestRendererEndBoundary`/`RequestRendererRenderElement`) is disabled with `CHECK_FAIL` — the DOM-diff approach is required.

`RequestRendererRenderDom` installs a full `RenderingDom`, rebuilds `renderingDomIndex`, and sets `needRefresh = true`. `RequestRendererRenderDomDiff` requires an existing DOM, applies `UpdateDomInplace(renderingDom, renderingDomIndex, diffs)`, and also sets `needRefresh = true`.

Actual painting happens in either `RequestRendererEndRendering` or `GlobalTimer()`: when `needRefresh` is true and refresh is no longer suppressed, `ForceRender()` calls `RenderDom` to recursively traverse `renderingDom`, renders each element in order with clipping, optionally draws the fatal-error overlay, redraws the native window, and handles render-target resize/lost-device failures by resizing/recreating the target and scheduling another refresh. `RequestRendererEndRendering` clears suppression and can force the completed frame to render before returning `RespondRendererEndRendering(id, elementMeasurings)`. While connected, `GlobalTimer()` also flushes accumulated IO events and drives caret blinking; after disconnection it clears pending core events but can continue rendering a retained fatal-error overlay. `RequestRendererIdle()` is an idle hint and is intentionally ignored by this renderer.

### Event Forwarding

`GuiRemoteRendererSingle` implements all `INativeWindowListener` mouse/keyboard callbacks to forward OS events as protocol events:
- **Discrete events** (button clicks, key presses): Sent immediately via `events->OnIOButtonDown`, `events->OnIOButtonUp`, `events->OnIOButtonDoubleClick`, `events->OnIOKeyDown`, etc. Mouse-button events carry an `IOMouseInfoWithButton` whose `button` is `IOMouseButton::Left`, `Right`, or `Middle`.
- **High-frequency events** (mouse move, wheel, key auto-repeat): Accumulated and coalesced:
  - `pendingMouseMove`: Only the latest mouse position is kept.
  - `pendingHWheel` / `pendingVWheel`: Wheel deltas are summed across frames.
  - `pendingKeyAutoDown`: Only the latest auto-repeat key is kept.
  - `SendAccumulatedMessages()` is called from `GlobalTimer()` to flush these accumulated events.
- **Window lifecycle events** (`Opened`, `BeforeClosing`, `Moved`, `DpiChanged`): Translated to protocol events like `OnControllerConnect`, `OnControllerRequestExit`, `OnWindowBoundsUpdated`.

All outgoing callbacks and responses are guarded by `CanSendEvents()`. `DisconnectFromCore()` marks the renderer as disconnecting, releases mouse capture, unregisters global shortcuts, and clears all accumulated events so no stale input is sent afterward.

### Hit Testing

Hit testing is performed locally in the renderer by traversing the rendering DOM tree via `HitTestInternal`. Each DOM node may have `hitTestResult` and `cursor` attributes set by the core side. The renderer walks the tree and finds the matching node for a given point. This avoids round-trips — hit testing stays entirely renderer-side.

### Document Paragraph Rendering

Document paragraph elements are represented by the renderer-local `GuiRemoteDocumentParagraphElement` wrapper. The wrapper is both an `IGuiGraphicsElement` and its own renderer/factory, but it delegates actual text layout to an `IGuiGraphicsParagraph` created from the active render target.

The wrapper caches full paragraph state, including text, wrapping/alignment/max-width, caret state, text runs, inline-object runs, merged runs, inline-object bounds, inline-object properties, and callback ranges. This cache is required because `SetRenderTarget(nullptr)` destroys the native paragraph object; when a new render target arrives, `TryRecreateParagraph()` creates a fresh paragraph and reapplies properties, runs, and caret state.

`ApplyUpdateAndFillResponse(arguments, response)` accepts incremental `runsDiff` updates from the core side. The first update must contain `text`; later updates must omit `text` and only update properties/runs. Removed inline objects clear cached bounds/properties/ranges and reset native paragraph ranges when the paragraph exists. Inline-object hit testing calls `TryGetInlineObjectRunProperty(callbackId, outProp)` instead of directly reading the wrapper dictionaries.

## Protocol Serialization and Channel Architecture

The channel system converts typed protocol calls to Parser2 JSON node packages and moves those packages through `VlppOS` inter-process channels. It no longer uses a separate JSON-string transformer stack for GacUI remoting.

### Core Interfaces

- `IGuiRemoteProtocol`: The high-level typed protocol interface with named methods (`RequestNAME`, `OnNAME`, `RespondNAME`).
- `channeling::JsonPackage`: Alias of `Ptr<glr::json::JsonNode>`.
- `channeling::IJsonChannel`: Alias of `inter_process::IChannel<JsonPackage>`, with `IJsonChannelReader`, `IJsonChannelClient`, and `IJsonChannelServer` aliases for the corresponding channel roles.
- `GuiRemoteProtocolNetworkChannelServer<TServerBase>`: Network channel server alias based on `inter_process::NetworkProtocolChannelServer<JsonPackage, glr::json::JsonNodeListSerializer, TServerBase>`.
- `GuiRemoteProtocolChannelClient` and `GuiRemoteProtocolLocalChannelClient`: Client helpers that expose the `GacUIRemoteProtocol` JSON channel through `GetProtocolChannel()`.

### Typed Protocol to JSON Channel

Two channel adapters bridge typed protocol calls and JSON packages:

- `GuiRemoteProtocolCoreChannel`: Implements `IGuiRemoteProtocol` and reads from an `IJsonChannel`. `RequestNAME(...)` serializes arguments with `ConvertCustomTypeToJson()`, packs an envelope with `JsonChannelPack()`, and sends it to the current renderer client. Incoming packages are unpacked with `JsonChannelUnpack()`, dispatched by name, deserialized with `ConvertJsonToCustomType()`, and delivered as `events->OnNAME(...)` or `events->RespondNAME(...)`. It also queues outgoing packages before a renderer is known and exposes `DetachRenderer(clientId)` for renderer replacement.
- `GuiRemoteProtocolRendererChannel`: Reads renderer-side JSON packages and calls the wrapped `IGuiRemoteProtocol`. Its constructor takes the renderer-side `IJsonChannel` and the `IGuiRemoteProtocol`; the channel sends renderer events and responses back to `GacUIRemoteProtocolCoreClientId`.

### Transport Layer

Named-pipe and HTTP remoting use `VlppOS` network protocol clients/servers underneath the JSON channel. The channel serializer for transport package lists is `glr::json::JsonNodeListSerializer`, shared with Parser2 JSON infrastructure. GacUI remoting code therefore passes `JsonPackage` values through channel clients and servers instead of converting each package through a dedicated `WString` protocol layer.

### JSON Envelope Format

Every package is a JSON object with fields:
- `"semantic"`: One of `"Message"`, `"Request"`, `"Response"`, `"Event"`.
- `"id"`: (optional) An integer request/response ID, present only for `Request` and `Response`.
- `"name"`: The message/event name (e.g., `"ControllerGetFontConfig"`, `"IOMouseMoving"`).
- `"arguments"`: (optional) The serialized arguments as a JSON value.

Messages (no response expected) use `"Message"`. Requests (response expected) use `"Request"` with an ID. Responses use `"Response"` with the matching ID. Events use `"Event"`.

### JSON Serialization of Protocol Types

Protocol types are code-generated from `Protocol/*.txt` files into `GuiRemoteProtocolSchema.h`/`.cpp`. Each struct gets a `JsonHelper<T>` specialization with `ToJson` and `FromJson` methods. The shared infrastructure in `GuiRemoteProtocolSchemaShared.h` provides:
- Primitive type serializers: `bool`, `vint`, `float`, `double`, `WString`, `wchar_t`, `VKEY`, `Color`, `Ptr<MemoryStream>` (Base64-encoded).
- Generic container serializers: `Nullable<T>`, `Ptr<T>`, `List<T>`, `ArrayMap<K,V,F>`, `Dictionary<K,V>`.
- `Variant` types are serialized with a type discriminator field.
- `ConvertCustomTypeToJsonField` adds a named field to a JSON object.

### Async Channel

`GuiRemoteProtocolAsyncJsonChannel` is the core-side async wrapper around an `IJsonChannel`. It queues outgoing packages, queues incoming events for `ProcessRemoteEvents()`, stores incoming responses by request id, and blocks `BatchWrite(disconnected)` until the current `PendingRequestGroup` is satisfied or disconnected. `connectionCounter` and `connectionClientId` protect pending requests when channel events arrive after disconnect/reconnect boundaries.

`GuiRemoteProtocolAsyncJsonChannelRenderer` is the renderer-side async wrapper. It queues received packages and main-thread tasks, then schedules `ProcessPendingMessages()` through an `IGuiRemoteProtocolAsyncRendererInvoker`. Before `SetInvokeInMainThread(...)` is called by renderer `GuiMain`, work is cached. After the invoker is installed, it is drained in FIFO order on the renderer UI thread. `Initialize(reader)` requires a non-null reader; `Detach()` explicitly clears it, increments `messageVersion`, and drops queued work. The version check prevents callbacks queued before `Detach()` from running after detachment or a later reader installation.

## Demo Project Pair

Two projects in `Test/GacUISrc/` demonstrate a full remote protocol deployment. They are paired: one is the core side (console application) and the other is the renderer side (Windows application).

RemoteViewModelTest-specific remoting support is organized in `Test/RemotingHelpers/Rvmt/`. `ViewModelShared.*` owns the fixed RVM constants and concrete generated-RPC dispatcher initialization, `ViewModelHostClient.*` owns only the network host client, and `ViewModelHostServer.*` owns the protected `RpcServerHelpers` implementation and the application-facing `RemoteViewModelChannelServer<TServerBase>`. The consolidated `Source_RemotingHelpers.vcxitems` inventory lists these files under its `Rvmt` filter and is imported only by remoting applications; standalone applications receive reusable automation endpoints through GacUI Core instead.

### RemotingTest_Core (Console Application)

Located at `Test/GacUISrc/RemotingTest_Core/`. Accepts `/Pipe` or `/Http` arguments to start either a named-pipe server or HTTP server.

**Protocol stack setup** (`StartServer<TServerBase>` in `GuiMain.cpp`):
1. `StartNamedPipeServer`, `StartHttpServer`, or `StartMiniHttpServer` constructs the matching `RemotingChannelServer<TServerBase>`. `/RVMT` constructs `RemoteViewModelChannelServer<TServerBase>`, which adds exact RVM-host admission while preserving the base renderer behavior.
2. The shared run function starts the server, creates a local core client with `GuiRemoteProtocolLocalChannelClient`, and connects it to the server. Local clients are accepted directly because only the owning process can call `ConnectLocalClient`.
3. It wraps the core client's protocol channel in `GuiRemoteProtocolAsyncJsonChannel` and creates `SwitchableRenderersCoreChannel<TServerBase>`. This `GuiRemoteProtocolCoreChannel` subclass verifies at submission time that its protocol renderer is still the transport server's current renderer. `Submit` evaluates this virtual guard before applying the base channel's default no-renderer behavior, so the switchable channel can report disconnection for both a stale renderer and renderer id `-1` while the default channel still treats `-1` as an idle state.
4. It builds `GuiRemoteProtocolFilter` -> `GuiRemoteProtocolDomDiffConverter` -> `SetupRemoteNativeController`. The Core remains usable before a renderer connects.
5. In `/RVMT`, the shared server owns the requester session. The exact RVM host is admitted before service acquisition, while renderer admission remains closed until the requester enters its running phase after the window is constructed.
6. On shutdown, Core clears the server's stored JSON/protocol channel pointers, finalizes the optional requester session, and stops the channel server before stack-owned wrappers are destroyed.

`RemotingChannelServer<TServerBase>::OnRemoteClientConnected` accepts replacement renderers. If a different renderer is already current, it calls `GuiRemoteProtocolCoreChannel::DetachRenderer(oldClientId)`, tries to send a raw `ControllerConnectionStopped` package to the old renderer, and disconnects the old transport only when notification fails. `RemoteViewModelChannelServer<TServerBase>` overrides this single remote-admission operation to accept the RVM host and to check the concrete requester phase before delegating renderer admission. If the RVM host disconnects, it broadcasts the business error when renderers are enabled and then terminates the requester.

### RemotingTest_Rendering_Win32 (Windows Application)

Located at `Test/GacUISrc/RemotingTest_Rendering_Win32/`. Accepts `/Pipe` or `/Http` arguments to start as a named-pipe or HTTP client.

**Protocol stack setup** (`StartClient` in `GuiMain.cpp`; this function is not a template):
1. Receives a named-pipe, Windows HTTP, or MiniHTTP `INetworkProtocolClient` and creates the shared `RemoteProtocolRendererClient`, derived from `GuiRemoteProtocolChannelClient`, over it.
2. Creates `GuiRemoteProtocolAsyncJsonChannelRenderer` over the client's protocol channel.
3. Creates `GuiRemoteRendererSingle` and `GuiRemoteProtocolRendererChannel(&asyncRendererChannel, &remoteRenderer)`.
4. Waits for the server, then calls `SetupRawWindowsDirect2DRenderer()` to run the native window event loop.
5. In `GuiMain()`, creates the native window, registers it with `GuiRemoteRendererSingle`, creates a retained `Ptr<GuiMainAsyncRendererInvoker>`, installs it through `asyncChannel->SetInvokeInMainThread(invoker)`, drains startup work with `ProcessPendingMessages()`, and runs the window service.
6. On exit, clears the invoker, unregisters the main window, stops the network connection, and clears stack-owned renderer/channel pointers.

`RemoteProtocolRendererClient` queues both protocol packages and terminal actions through the async renderer's ordered main-thread FIFO. A Core-authored `!Error` arrives through `OnReadError`, claims the first fatal error, and opens the native Yes/No prompt. Choosing Yes calls `ForceExitByFatelError()`; choosing No calls `RetainByFatalError(message)`, keeps the native renderer window open with a `[STOPPED]` title and fatal overlay, and exposes the error through renderer automation. A fatal local channel error has different UI semantics: after VlppOS's `IChannelClient` promotes a post-connection protocol error, `OnLocalError(..., true)` queues the ordinary disconnected transition directly, without showing a fatal prompt and without waiting for `OnDisconnected`. `OnDisconnected` queues the same idempotent transition when it is delivered. FIFO ordering lets an earlier `ControllerConnectionStopped` or Core `!Error` win before detach.

### Protocol Stack Direction

**Core side** (messages flow outward):
```
SetupRemoteNativeController
  → GuiRemoteProtocolDomDiffConverter
    → GuiRemoteProtocolFilter
      → GuiRemoteProtocolCoreChannel
        → GuiRemoteProtocolAsyncJsonChannel
          → GuiRemoteProtocolLocalChannelClient / IJsonChannel
            → GuiRemoteProtocolNetworkChannelServer<TServerBase>
              → Named pipe or HTTP transport
```

**Renderer side** (messages flow inward):
```
Named pipe or HTTP transport
  → GuiRemoteProtocolChannelClient / IJsonChannel
    → GuiRemoteProtocolAsyncJsonChannelRenderer
      → GuiRemoteProtocolRendererChannel
        → GuiRemoteRendererSingle (IGuiRemoteProtocol)
          → Native window rendering
```

Events flow in the reverse direction through the same stack.
