#### Remote Protocol Renderer and Serialization

The remote protocol renderer side receives protocol messages from the core side and translates them into native window operations and graphics rendering. The serialization and channel infrastructure provides composable layers that convert between typed protocol calls and transport-ready strings, enabling GacUI applications to run across process boundaries over any user-provided transport (named pipe, HTTP, WebSocket, etc.).

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

### Source File Organization

- `GuiRemoteRendererSingle.cpp`: Construction, destruction, main window registration, connection lifecycle (`Opened`, `BeforeClosing`, `Closed`), screen/config management.
- `GuiRemoteRendererSingle_Controller.cpp`: Controller-level requests — `RequestControllerGetFontConfig`, `RequestControllerGetScreenConfig`, `RequestControllerConnectionEstablished`, `RequestControllerConnectionStopped`.
- `GuiRemoteRendererSingle_MainWindow.cpp`: Window style notifications — `RequestWindowNotifySetBounds`, `RequestWindowNotifySetTitle`, `RequestWindowNotifySetEnabled`, `RequestWindowNotifyShow`, etc.
- `GuiRemoteRendererSingle_IO.cpp`: IO requests (global shortcuts, mouse capture, key state queries) and native-to-protocol input event conversion. Contains `SendAccumulatedMessages()` for batching high-frequency events.
- `GuiRemoteRendererSingle_Rendering.cpp`: Core rendering pipeline — element creation/destruction (`RequestRendererCreated`, `RequestRendererDestroyed`), begin/end rendering, DOM rendering (`Render` recursive traversal), hit testing, and `GlobalTimer`/`Paint` driven refresh.
- `GuiRemoteRendererSingle_Rendering_Elements.cpp`: Property updates on ordinary graphics elements (solid border, sink border, splitter, background, gradient, inner shadow, polygon).
- `GuiRemoteRendererSingle_Rendering_Label.cpp`: Solid label measurement and property updates.
- `GuiRemoteRendererSingle_Rendering_Image.cpp`: Image creation, metadata, and image frame element updates.
- `GuiRemoteRendererSingle_Rendering_Document.cpp`: Complex document paragraph element rendering via `GuiRemoteDocumentParagraphElement` inner class.

### Rendering Pipeline

When the core sends `RequestRendererBeginRendering(ElementBeginRendering)`, the `updatedElements` field contains a list of `OrdinaryElementDescVariant`. Each variant is dispatched via `Apply(Overloading(...))` to the corresponding `RequestRendererUpdateElement_*` helper, which looks up the element by ID in `availableElements` and sets its properties.

The core sends `RequestRendererRenderDom` or `RequestRendererRenderDomDiff` to update the rendering DOM tree. The legacy command-based rendering path (`RequestRendererBeginBoundary`/`RequestRendererEndBoundary`/`RequestRendererRenderElement`) is disabled with `CHECK_FAIL` — the DOM-diff approach is required.

Actual painting happens in `GlobalTimer()`: if `needRefresh` is true, it calls `Render(renderingDom, rt)` which recursively traverses the DOM tree, rendering each element in order with clipping. After rendering, it checks if label measurements changed and populates `elementMeasurings`. The response is returned via `RespondRendererEndRendering(id, elementMeasurings)`.

### Event Forwarding

`GuiRemoteRendererSingle` implements all `INativeWindowListener` mouse/keyboard callbacks to forward OS events as protocol events:
- **Discrete events** (button clicks, key presses): Sent immediately via `events->OnIOMouseLeft`, `events->OnIOKeyDown`, etc.
- **High-frequency events** (mouse move, wheel, key auto-repeat): Accumulated and coalesced:
  - `pendingMouseMove`: Only the latest mouse position is kept.
  - `pendingHWheel` / `pendingVWheel`: Wheel deltas are summed across frames.
  - `pendingKeyAutoDown`: Only the latest auto-repeat key is kept.
  - `SendAccumulatedMessages()` is called from `GlobalTimer()` to flush these accumulated events.
- **Window lifecycle events** (`Opened`, `BeforeClosing`, `Moved`, `DpiChanged`): Translated to protocol events like `OnControllerConnect`, `OnControllerRequestExit`, `OnWindowBoundsUpdated`.

### Hit Testing

Hit testing is performed locally in the renderer by traversing the rendering DOM tree via `HitTestInternal`. Each DOM node may have `hitTestResult` and `cursor` attributes set by the core side. The renderer walks the tree and finds the matching node for a given point. This avoids round-trips — hit testing stays entirely renderer-side.

## Protocol Serialization and Channel Architecture

The serialization system uses a layered channel architecture to convert between typed protocol calls and transport-ready data. Each layer is a composable building block.

### Core Interfaces

- `IGuiRemoteProtocolChannel<TPackage>`: A bidirectional channel that can `Write` packages and receives packages via `IGuiRemoteProtocolChannelReceiver<TPackage>::OnReceive`.
- `IGuiRemoteProtocol`: The high-level typed protocol interface with named methods (`RequestNAME`, `OnNAME`, `RespondNAME`).

### Layer 1: IGuiRemoteProtocol ↔ JsonObject Channel

Two adapter classes handle bidirectional conversion between typed protocol calls and JSON objects:

- `GuiRemoteProtocolFromJsonChannel`: Wraps `IGuiRemoteProtocolChannel<Ptr<JsonObject>>` and implements `IGuiRemoteProtocol`. When `RequestNAME(arguments)` is called, it serializes arguments to JSON via `ConvertCustomTypeToJson()`, packs them into a JSON envelope with `JsonChannelPack()`, and calls `channel->Write(package)`. When it receives a JSON package via `OnReceive`, it unpacks with `JsonChannelUnpack()`, dispatches by name, deserializes via `ConvertJsonToCustomType()`, and calls `events->OnNAME()` or `events->RespondNAME()`.
- `GuiRemoteJsonChannelFromProtocol`: The reverse — wraps `IGuiRemoteProtocol` and implements `IGuiRemoteProtocolChannel<Ptr<JsonObject>>`. Converts incoming JSON packages to protocol calls, and outgoing events/responses to JSON packages.

### Layer 2: JsonObject ↔ WString Channel

`JsonToStringSerializer` handles JSON-to-string conversion:
- `Serialize`: Converts `Ptr<JsonObject>` to `WString` using `JsonToString` with compact formatting.
- `Deserialize`: Parses `WString` to `Ptr<JsonObject>` using `JsonParse`.
- Type aliases: `GuiRemoteJsonChannelStringSerializer` and `GuiRemoteJsonChannelStringDeserializer` for `GuiRemoteProtocolChannelSerializer<JsonToStringSerializer>` and `GuiRemoteProtocolChannelDeserializer<JsonToStringSerializer>`.

### Layer 3: WString ↔ Transport (User-Implemented)

The user provides an `IGuiRemoteProtocolChannel<WString>` implementation that sends and receives strings over any transport mechanism (named pipe, HTTP, WebSocket, etc.). GacUI does not provide transport implementations directly.

### Layer 4: UTF String Conversion (Optional)

`GuiRemoteUtfStringChannelSerializer` / `GuiRemoteUtfStringChannelDeserializer` can convert between `WString` and other UTF string types if the transport requires a specific encoding.

### Channel Transformer Pattern

`GuiRemoteProtocolChannelTransformerBase<TFrom, TTo>` bridges two channel types:
- `GuiRemoteProtocolChannelSerializer<TSerialization>`: Calls `TSerialization::Serialize` on `Write` and `TSerialization::Deserialize` on `OnReceive`.
- `GuiRemoteProtocolChannelDeserializer<TSerialization>`: The reverse — calls `Deserialize` on `Write` and `Serialize` on `OnReceive`.

The `TSerialization` concept requires: `SourceType`, `DestType`, `ContextType`, and static `Serialize`/`Deserialize` methods.

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

`GuiRemoteProtocolAsyncChannelSerializer<TPackage>` provides thread separation for real remote deployment:
- A **channel thread** handles all `Write`/`OnReceive` calls on the underlying channel (IO operations).
- A **UI thread** runs the GacUI application logic.
- `Start(channel, uiMainProc, startingProc)` launches both threads. The `startingProc` is responsible for creating the threads.
- Events received asynchronously are queued and dispatched on the UI thread during `Submit()`.
- Responses are matched to pending requests using a `PendingRequestGroup` with `connectionCounter` for safe handling of disconnection/reconnection races.
- `ExecuteInChannelThread()` allows queueing work from any thread to the channel thread.

## Demo Project Pair

Two projects in `Test/GacUISrc/` demonstrate a full remote protocol deployment. They are paired: one is the core side (console application) and the other is the renderer side (Windows application).

### RemotingTest_Core (Console Application)

Located at `Test/GacUISrc/RemotingTest_Core/`. Accepts `/Pipe` or `/Http` arguments to start either a named-pipe server or HTTP server.

**Protocol stack setup** (`StartServer<TServer>` in `GuiMain.cpp`):
1. Creates `CoreChannel` wrapping the transport server — implements `IGuiRemoteProtocolChannel<WString>`.
2. Creates `GuiRemoteJsonChannelStringSerializer` for JSON ↔ String conversion.
3. Creates `GuiRemoteProtocolAsyncJsonChannelSerializer` for async channel/UI thread separation.
4. Starts via `asyncChannelSender.Start()` with a UI main proc that builds the protocol stack: `GuiRemoteProtocolFromJsonChannel` → `GuiRemoteProtocolFilter` → `GuiRemoteProtocolDomDiffConverter` → `SetupRemoteNativeController`.
5. Waits for a renderer client to connect, then waits for the async channel to stop, then waits for disconnection.

**CoreChannel** bridges async channel and network transport:
- `Write(WString)`: Accumulates messages in a pending list.
- `Submit()`: Dispatches accumulated messages via `networkProtocol->SendStringArray()`.
- `OnReadStringThreadUnsafe()`: Called when strings arrive from the network, queues them for channel thread processing via `asyncChannel->ExecuteInChannelThread()`.
- Detects `ControllerConnect` event JSON to track connection state.
- On reconnection (`OnReconnectedUnsafe`), injects a `ControllerDisconnect` event.

### RemotingTest_Rendering_Win32 (Windows Application)

Located at `Test/GacUISrc/RemotingTest_Rendering_Win32/`. Accepts `/Pipe` or `/Http` arguments to start as a named-pipe or HTTP client.

**Protocol stack setup** (`StartClient<TClient>` in `GuiMain.cpp`):
1. Creates `GuiRemoteRendererSingle` — the renderer implementing `IGuiRemoteProtocol`.
2. Creates `GuiRemoteJsonChannelFromProtocol` wrapping the renderer — converts protocol calls to JSON.
3. Creates `GuiRemoteJsonChannelStringDeserializer` for String ↔ JSON conversion.
4. Creates `RendererChannel` connecting renderer, transport, and channel.
5. Hooks `BeforeWrite`/`BeforeOnReceive` events for request/response caching in `RendererChannel`.
6. Calls `SetupRawWindowsDirect2DRenderer()` to run the native window event loop.

**RendererChannel** bridges network transport and JSON channel:
- `OnReadStringThreadUnsafe()`: Receives strings from network, dispatches to UI thread via `InvokeInMainThread()`. Handles error strings (prefixed with `!`) by displaying a `MessageBox` and calling `renderer->ForceExitByFatelError()`.
- `OnReceive(WString)`: Uses a caching mechanism (`isCaching`/`cachedPackages`) to batch responses — when a `Request` semantic is detected (via `BeforeWrite`), caching is enabled; when a `Response` semantic is detected (via `BeforeOnReceive`), caching is disabled and all cached packages are sent as a batch.

### Protocol Stack Direction

**Core side** (messages flow outward):
```
SetupRemoteNativeController
  → GuiRemoteProtocolDomDiffConverter
    → GuiRemoteProtocolFilter
      → GuiRemoteProtocolFromJsonChannel
        → GuiRemoteProtocolAsyncJsonChannelSerializer
          → GuiRemoteJsonChannelStringSerializer
            → CoreChannel (IGuiRemoteProtocolChannel<WString>)
              → Network transport (user-implemented)
```

**Renderer side** (messages flow inward):
```
Network transport (user-implemented)
  → RendererChannel
    → GuiRemoteJsonChannelStringDeserializer
      → GuiRemoteJsonChannelFromProtocol
        → GuiRemoteRendererSingle (IGuiRemoteProtocol)
          → Native window rendering
```

Events flow in the reverse direction through the same stack.
