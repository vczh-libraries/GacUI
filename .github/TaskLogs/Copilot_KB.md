# !!!KNOWLEDGE BASE!!!

# DESIGN REQUEST

Draft a research topic about GuiRemoteRendererSingle, as well as how protocol are serialized and deserialized. You need to also check out two project in GacUI, one is the remote protocol core application, another is the windows rendering test that uses GuiRemoteRendererSingle. They are a demo in a pair. But do not focus on actual protocol implementation because the http and named pipe are not offered by GacUI directly, they are supposed to be implemented by the user. This will become a design explanation.

# INSIGHT

## GuiRemoteRendererSingle Overview

`GuiRemoteRendererSingle` (in `Source/PlatformProviders/RemoteRenderer/`) is a renderer-side implementation that bridges the GacUI remote protocol to an actual native window with real graphics rendering. It implements `IGuiRemoteProtocol` so it can receive protocol messages (requests from the core side) and translates them into real native window operations and graphics element rendering. It also implements `INativeWindowListener` and `INativeControllerListener` to capture OS-level events (mouse, keyboard, window lifecycle) and forward them as protocol events back to the core side.

### Class Hierarchy and Interfaces

`GuiRemoteRendererSingle` inherits from:
- `Object` — standard base
- `IGuiRemoteProtocol` (virtual) — receives all protocol messages from the core side
- `INativeWindowListener` (protected virtual) — listens to native window events
- `INativeControllerListener` (protected virtual) — listens to global timer and shortcut key events

It is NOT a full `INativeController`. It relies on a real platform provider (e.g., Windows Direct2D) already running via `SetupRawWindowsDirect2DRenderer()`. The class registers itself as a listener on a native window and translates between the protocol message interface and native operations.

### Key Members

- `window` / `screen` / `events`: The native window, current screen, and the `IGuiRemoteProtocolEvents` callback to fire events to the core side.
- `availableElements`: A `Dictionary<vint, Ptr<IGuiGraphicsElement>>` mapping element IDs to actual graphics elements created in the real rendering engine (e.g., `GuiSolidBorderElement`, `GuiSolidLabelElement`, etc.).
- `availableImages`: A `Dictionary<vint, Ptr<INativeImage>>` mapping image IDs to actual loaded images.
- `renderingDom` / `renderingDomIndex`: The rendering DOM tree received from the core side, used for actual on-screen rendering and hit testing.
- `solidLabelMeasurings` / `fontHeightMeasurings`: Caches and measurement tracking for text elements, enabling the measurement feedback loop.
- `pendingMouseMove`, `pendingHWheel`, `pendingVWheel`, `pendingKeyAutoDown`, `pendingWindowBoundsUpdate`: Accumulation fields for high-frequency events that are batched before sending.

### Source File Organization

- `GuiRemoteRendererSingle.cpp`: Construction, destruction, main window registration, connection lifecycle (`Opened`, `BeforeClosing`, `Closed`), screen/config management.
- `GuiRemoteRendererSingle_Controller.cpp`: Handles controller-level requests (`RequestControllerGetFontConfig`, `RequestControllerGetScreenConfig`, `RequestControllerConnectionEstablished`, `RequestControllerConnectionStopped`).
- `GuiRemoteRendererSingle_MainWindow.cpp`: Handles window style notifications (`RequestWindowNotifySetBounds`, `RequestWindowNotifySetTitle`, `RequestWindowNotifySetEnabled`, `RequestWindowNotifyShow`, etc.).
- `GuiRemoteRendererSingle_IO.cpp`: Handles IO requests (global shortcuts, mouse capture, key state queries) and converts native input events to protocol events. Contains `SendAccumulatedMessages()` which batches high-frequency events.
- `GuiRemoteRendererSingle_Rendering.cpp`: Core rendering pipeline — element creation/destruction (`RequestRendererCreated`, `RequestRendererDestroyed`), begin/end rendering, DOM rendering (`Render` recursive traversal), hit testing, and the `GlobalTimer`/`Paint` driven refresh.
- `GuiRemoteRendererSingle_Rendering_Elements.cpp`: Updates properties on ordinary graphics elements (solid border, sink border, splitter, background, gradient, inner shadow, polygon).
- `GuiRemoteRendererSingle_Rendering_Label.cpp`: Handles solid label measurement and property updates.
- `GuiRemoteRendererSingle_Rendering_Image.cpp`: Handles image creation, metadata, and image frame element updates.
- `GuiRemoteRendererSingle_Rendering_Document.cpp`: Handles the complex document paragraph element with `GuiRemoteDocumentParagraphElement` inner class.

### Rendering Pipeline

When the core sends `RequestRendererBeginRendering(ElementBeginRendering)`, it contains `updatedElements` — a list of `OrdinaryElementDescVariant`. Each variant is dispatched via `Apply(Overloading(...))` to the corresponding `RequestRendererUpdateElement_*` helper. These helpers look up the element by ID in `availableElements` and set the element's properties.

The core sends `RequestRendererRenderDom` or `RequestRendererRenderDomDiff` to update the rendering DOM tree. The old command-based rendering (`RequestRendererBeginBoundary`/`RequestRendererEndBoundary`/`RequestRendererRenderElement`) is disabled and raises `CHECK_FAIL` — the DOM-diff approach is required.

Actual rendering happens in `GlobalTimer()`: if `needRefresh` is true, it calls `Render(renderingDom, rt)` which recursively traverses the DOM tree, rendering each element in order with clipping. After rendering, it checks if label measurements changed and populates `elementMeasurings`. The response is returned via `RequestRendererEndRendering` → `RespondRendererEndRendering(id, elementMeasurings)`.

### Event Forwarding

For input events, `GuiRemoteRendererSingle` implements all `INativeWindowListener` mouse/keyboard callbacks. Discrete events (button clicks) are sent immediately. High-frequency events (mouse move, wheel, key auto-repeat) are accumulated and coalesced:
- `pendingMouseMove`: Only the latest mouse position is kept.
- `pendingHWheel`/`pendingVWheel`: Wheel deltas are summed.
- `pendingKeyAutoDown`: Only the latest auto-repeat key is kept.
- `SendAccumulatedMessages()` is called from `GlobalTimer()` to flush these.

Window lifecycle events (`Opened`, `BeforeClosing`, `Moved`, `DpiChanged`, etc.) are translated to protocol events like `OnControllerConnect`, `OnControllerRequestExit`, `OnWindowBoundsUpdated`, etc.

### Hit Testing

Hit testing is performed locally in the renderer by traversing the rendering DOM tree (`HitTestInternal`). Each DOM node may have `hitTestResult` and `cursor` attributes set by the core side. The renderer walks the tree and finds the matching node for a given hit test point. This avoids round-trips for hit testing.

## Protocol Serialization and Channel Architecture

### Layered Channel Design

The serialization system uses a layered channel architecture with three key interfaces:
- `IGuiRemoteProtocolChannel<TPackage>`: A bidirectional channel that can `Write` packages and receives packages via `IGuiRemoteProtocolChannelReceiver<TPackage>::OnReceive`.
- `IGuiRemoteProtocol`: The high-level typed protocol interface with named methods (`RequestNAME`, `OnNAME`, `RespondNAME`).

The layers transform between typed protocol calls and serialized data:

1. **IGuiRemoteProtocol ↔ JsonObject channel**: Two adapter classes handle this conversion:
   - `GuiRemoteProtocolFromJsonChannel`: Wraps a `IGuiRemoteProtocolChannel<Ptr<JsonObject>>` and implements `IGuiRemoteProtocol`. When a `RequestNAME(arguments)` is called, it serializes the arguments to JSON via `ConvertCustomTypeToJson()`, packs them into a JSON envelope with `JsonChannelPack()`, and calls `channel->Write(package)`. When it receives a JSON package via `OnReceive`, it unpacks the envelope with `JsonChannelUnpack()`, dispatches to the appropriate event or response handler by name, deserializes arguments via `ConvertJsonToCustomType()`, and calls the corresponding `events->OnNAME()` or `events->RespondNAME()`.
   - `GuiRemoteJsonChannelFromProtocol`: The reverse direction — wraps an `IGuiRemoteProtocol` and implements `IGuiRemoteProtocolChannel<Ptr<JsonObject>>`. When it receives `Write(jsonPackage)`, it unpacks the envelope, deserializes arguments, and calls the protocol's `RequestNAME()`. When events or responses fire, it serializes and sends JSON packages to the receiver.

2. **JsonObject ↔ WString channel**: `JsonToStringSerializer` handles this:
   - `Serialize`: Converts `Ptr<JsonObject>` to `WString` using `JsonToString` with compact formatting.
   - `Deserialize`: Parses `WString` to `Ptr<JsonObject>` using `JsonParse`.
   - `GuiRemoteJsonChannelStringSerializer` and `GuiRemoteJsonChannelStringDeserializer` are type aliases for `GuiRemoteProtocolChannelSerializer<JsonToStringSerializer>` and `GuiRemoteProtocolChannelDeserializer<JsonToStringSerializer>`.

3. **WString ↔ transport**: This layer is user-implemented. The user provides an `IGuiRemoteProtocolChannel<WString>` that sends and receives strings over any transport (named pipe, HTTP, WebSocket, etc.).

4. **UTF string conversion**: `GuiRemoteUtfStringChannelSerializer`/`GuiRemoteUtfStringChannelDeserializer` can convert between `WString` and other UTF string types if needed.

### Channel Transformer Pattern

`GuiRemoteProtocolChannelTransformerBase<TFrom, TTo>` is a base class that bridges two channel types. It implements `IGuiRemoteProtocolChannel<TFrom>` while wrapping `IGuiRemoteProtocolChannel<TTo>`:
- `GuiRemoteProtocolChannelSerializer<TSerialization>`: Calls `TSerialization::Serialize` on `Write` and `TSerialization::Deserialize` on `OnReceive`.
- `GuiRemoteProtocolChannelDeserializer<TSerialization>`: The reverse — calls `Deserialize` on `Write` and `Serialize` on `OnReceive`.

The `TSerialization` concept requires: `SourceType`, `DestType`, `ContextType`, and static `Serialize`/`Deserialize` methods.

### JSON Envelope Format

Every package is a JSON object with fields:
- `"semantic"`: One of `"Message"`, `"Request"`, `"Response"`, `"Event"`.
- `"id"`: (optional) An integer request/response ID, present only for `Request` and `Response`.
- `"name"`: The message/event name (e.g., `"ControllerGetFontConfig"`, `"IOMouseMoving"`).
- `"arguments"`: (optional) The serialized arguments as a JSON value.

Messages (no response expected) use semantic `"Message"`. Requests (response expected) use `"Request"` with an ID. Responses use `"Response"` with the matching ID. Events use `"Event"`.

### JSON Serialization of Protocol Types

Protocol types are code-generated from `Protocol/*.txt` files into `GuiRemoteProtocolSchema.h`/`.cpp`. Each struct gets a `JsonHelper<T>` specialization with `ToJson` and `FromJson` methods. The shared infrastructure in `GuiRemoteProtocolSchemaShared.h` provides:
- Primitive type serializers: `bool`, `vint`, `float`, `double`, `WString`, `wchar_t`, `VKEY`, `Color`, `Ptr<MemoryStream>` (Base64-encoded).
- Generic container serializers: `Nullable<T>`, `Ptr<T>`, `List<T>`, `ArrayMap<K,V,F>`, `Dictionary<K,V>`.
- `Variant` types are serialized with a type discriminator field.
- `ConvertCustomTypeToJsonField` adds a named field to a JSON object.

### Async Channel

`GuiRemoteProtocolAsyncChannelSerializer<TPackage>` provides thread separation:
- A **channel thread** handles all `Write`/`OnReceive` calls on the underlying channel (IO operations).
- A **UI thread** runs the GacUI application logic.
- `Start(channel, uiMainProc, startingProc)` launches both threads. The `startingProc` is responsible for creating the threads.
- Events received asynchronously are queued and dispatched on the UI thread during `Submit()`.
- Responses are matched to pending requests using a `PendingRequestGroup` with `connectionCounter` for handling disconnection/reconnection safely.
- `ExecuteInChannelThread()` allows queueing work to the channel thread from any thread.

## Demo Project Pair

### RemotingTest_Core (Console Application)

Located at `Test/GacUISrc/RemotingTest_Core/`. This is the core side that runs the GacUI application logic headlessly.

**Entry point** (`Main.cpp`): A console application that accepts `/Pipe` or `/Http` arguments to start either a named pipe server or HTTP server.

**GuiMain.cpp**: Contains `GuiMain()` which creates a GacUI window with DarkSkin theme. The `StartServer<TServer>` template function sets up the full protocol stack:
1. Creates `CoreChannel` wrapping the transport server.
2. Creates `GuiRemoteJsonChannelStringSerializer` for JSON↔String conversion.
3. Creates `GuiRemoteProtocolAsyncJsonChannelSerializer` for async channel/UI thread separation.
4. Starts via `asyncChannelSender.Start()` with a UI main proc that builds the protocol stack: `GuiRemoteProtocolFromJsonChannel` → `GuiRemoteProtocolFilter` → `GuiRemoteProtocolDomDiffConverter` → `SetupRemoteNativeController`.
5. Waits for a renderer client to connect, then waits for the async channel to stop, then waits for disconnection.

**CoreChannel**: Implements `IGuiRemoteProtocolChannel<WString>`. It bridges between the async channel and the network transport:
- `Write(WString)`: Accumulates messages in a pending list.
- `Submit()`: Dispatches accumulated messages via `networkProtocol->SendStringArray()`.
- `OnReadStringThreadUnsafe()`: Called when strings arrive from the network, queues them for processing in the channel thread via `asyncChannel->ExecuteInChannelThread()`.
- Detects `ControllerConnect` event JSON to track connection state.
- On reconnection (`OnReconnectedUnsafe`), injects a `ControllerDisconnect` event.

### RemotingTest_Rendering_Win32 (Windows Application)

Located at `Test/GacUISrc/RemotingTest_Rendering_Win32/`. This is the renderer side that displays the actual window and renders graphics.

**Entry point** (`Main.cpp`): A Windows application (WinMain) that accepts `/Pipe` or `/Http` arguments to start as a named pipe or HTTP client.

**GuiMain.cpp**: Contains `GuiMain()` which creates a native window, sets initial size, then registers the renderer channel and `GuiRemoteRendererSingle` on the window. The `StartClient<TClient>` template function sets up the renderer-side protocol stack:
1. Creates `GuiRemoteRendererSingle` — the core renderer that implements `IGuiRemoteProtocol`.
2. Creates `GuiRemoteJsonChannelFromProtocol` wrapping the renderer — converts protocol calls to JSON.
3. Creates `GuiRemoteJsonChannelStringDeserializer` for String↔JSON conversion.
4. Creates `RendererChannel` connecting the renderer, transport, and channel.
5. Hooks `BeforeWrite`/`BeforeOnReceive` events to handle request/response caching in `RendererChannel`.
6. Calls `SetupRawWindowsDirect2DRenderer()` to run the native window event loop.

**RendererChannel**: Bridges between the network transport and the JSON channel:
- `OnReadStringThreadUnsafe()`: Receives strings from the network, dispatches to the UI thread via `InvokeInMainThread()`. Handles error strings (prefixed with `!`) by showing a `MessageBox` and calling `renderer->ForceExitByFatelError()`.
- `OnReceive(WString)`: Called by the channel when the renderer side produces output. Uses a caching mechanism (`isCaching`/`cachedPackages`) to batch responses:
  - When a `Request` semantic is detected (via `BeforeWrite`), caching is enabled.
  - When a `Response` semantic is detected (via `BeforeOnReceive`), caching is disabled and all cached packages are sent as a batch.

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

# ASKS

# DRAFT

## DRAFT REQUEST

Based on the insight, draft a design explanation document covering GuiRemoteRendererSingle, protocol serialization and channel architecture, and the demo project pair.

## IMPROVEMENTS

## DESIGN EXPLANATION (GacUI)

Title: Remote Protocol Renderer and Serialization

This adds a new section under `### Design Explanation` in the GacUI project in `Index.md`, placed after the existing `#### Remote Protocol Core Architecture` entry.

The document file will be `KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md`.

## DOCUMENT

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

## DESIGN EXPLANATION (GacUI)

This document covers the **renderer side** of the remote protocol architecture and the **channel/serialization layer** that enables real remote deployment. It is a companion to the existing "Remote Protocol Core Architecture" design explanation, which covers the core side (`GuiRemoteController`, `GuiRemoteMessages`, rendering pipeline, DOM diff, etc.). This document specifically covers:
- `GuiRemoteRendererSingle` — the renderer-side protocol endpoint that translates protocol messages into real native window rendering and captures OS events back as protocol events.
- The layered channel architecture — `IGuiRemoteProtocolChannel<TPackage>`, serializer/deserializer templates, JSON envelope format, async thread separation, and UTF string conversion.
- The demo project pair (`RemotingTest_Core` and `RemotingTest_Rendering_Win32`) that demonstrates a complete real remote deployment.

Place as a new `#### Remote Protocol Renderer and Channel Architecture` under `### Design Explanation` in the GacUI section, after the existing `#### Remote Protocol Core Architecture`.

## DOCUMENT

# Remote Protocol Renderer and Channel Architecture

The remote protocol separates GacUI into a **core side** (application logic, layout, composition tree) and a **renderer side** (real OS window, graphics rendering, input capture). The core side is covered in `KB_GacUI_Design_RemoteProtocolCoreArchitecture.md`. This document covers the renderer side implementation (`GuiRemoteRendererSingle`), the serialization and channel layers that connect both sides, and the demo projects that demonstrate real remote deployment.

## GuiRemoteRendererSingle

`GuiRemoteRendererSingle` (in `Source/PlatformProviders/RemoteRenderer/`) is the renderer-side endpoint. It receives protocol messages from the core side (rendering commands, window style notifications, IO requests) and translates them into real native window operations. It also captures OS-level events (mouse, keyboard, window lifecycle) and forwards them as protocol events back to the core side.

### Class Hierarchy

`GuiRemoteRendererSingle` inherits from:
- `Object` — standard base class
- `IGuiRemoteProtocol` (virtual) — receives all protocol messages from the core side
- `INativeWindowListener` (protected virtual) — listens to native window events (mouse, keyboard, painting, hit testing, window lifecycle)
- `INativeControllerListener` (protected virtual) — listens to global timer and shortcut key events

It is NOT a full `INativeController`. It relies on a real platform provider already running (e.g., Windows Direct2D via `SetupRawWindowsDirect2DRenderer()`). The class registers itself as a listener on a native window created by that platform provider, then translates between the `IGuiRemoteProtocol` message interface and real native operations.

### Key Data Members

- `window` / `screen` / `events`: The native window, current screen, and the `IGuiRemoteProtocolEvents` callback for firing events to the core side.
- `availableElements` (`Dictionary<vint, Ptr<IGuiGraphicsElement>>`): Maps element IDs to real graphics element instances (e.g., `GuiSolidBorderElement`, `GuiSolidLabelElement`, `GuiImageFrameElement`).
- `availableImages` (`Dictionary<vint, Ptr<INativeImage>>`): Maps image IDs to loaded native images.
- `renderingDom` / `renderingDomIndex`: The current rendering DOM tree received from the core side, used for on-screen rendering and hit testing.
- `solidLabelMeasurings` / `fontHeightMeasurings`: Caches and measurement tracking for text elements, supporting the measurement feedback loop with the core side.
- `pendingMouseMove`, `pendingHWheel`, `pendingVWheel`, `pendingKeyAutoDown`, `pendingWindowBoundsUpdate`: Accumulation fields for high-frequency events that are coalesced before sending.
- `globalShortcuts` (`Dictionary<vint, GlobalShortcutKey>`): Global keyboard shortcut registrations received from the core side.
- `focusedParagraphElements`: Tracks paragraph elements with active carets for caret blink notifications.

### Source File Organization

The implementation is split across multiple files by responsibility:

- `GuiRemoteRendererSingle.cpp`: Construction, destruction, `RegisterMainWindow`/`UnregisterMainWindow`, connection lifecycle (`Opened`, `BeforeClosing`, `Closed`), screen and configuration management.
- `GuiRemoteRendererSingle_Controller.cpp`: Controller-level requests — `RequestControllerGetFontConfig`, `RequestControllerGetScreenConfig`, `RequestControllerConnectionEstablished`, `RequestControllerConnectionStopped`. These query the real platform's resource service and screen service.
- `GuiRemoteRendererSingle_MainWindow.cpp`: Window style notifications — `RequestWindowNotifySetBounds`, `RequestWindowNotifySetTitle`, `RequestWindowNotifySetEnabled`, `RequestWindowNotifyShow`, `RequestWindowNotifySetCustomFrameMode`, etc. Each maps directly to the corresponding `INativeWindow` method on the real window.
- `GuiRemoteRendererSingle_IO.cpp`: IO requests (global shortcuts, mouse capture, key state queries) and native input event forwarding. Contains `SendAccumulatedMessages()` which flushes coalesced high-frequency events.
- `GuiRemoteRendererSingle_Rendering.cpp`: Core rendering pipeline — element creation/destruction (`RequestRendererCreated`, `RequestRendererDestroyed`), begin/end rendering (`RequestRendererBeginRendering`, `RequestRendererEndRendering`), DOM rendering (`Render` recursive traversal), hit testing, and the `GlobalTimer`/`Paint`-driven refresh.
- `GuiRemoteRendererSingle_Rendering_Elements.cpp`: Updates properties on ordinary graphics elements — solid border, sink border, splitter, solid background, gradient background, inner shadow, polygon.
- `GuiRemoteRendererSingle_Rendering_Label.cpp`: Solid label measurement and property updates. Measures font heights and label sizes using the real rendering engine.
- `GuiRemoteRendererSingle_Rendering_Image.cpp`: Image creation from `Ptr<MemoryStream>`, metadata extraction, and image frame element updates.
- `GuiRemoteRendererSingle_Rendering_Document.cpp`: Document paragraph element handling via `GuiRemoteDocumentParagraphElement` inner class.

### Element Lifecycle on the Renderer Side

1. The core side sends `RequestRendererCreated` with a list of `RendererCreation` items (each containing an `id` and `type`).
2. The renderer creates the corresponding real `IGuiGraphicsElement` instance (e.g., `GuiFocusRectangleElement`, `GuiSolidBorderElement`, `GuiSolidLabelElement`, `GuiImageFrameElement`) and stores it in `availableElements[id]`.
3. On `RequestRendererDestroyed`, the element is removed from `availableElements` and released.

### Rendering Pipeline

The rendering pipeline is driven by `GlobalTimer()` and `Paint()`:

1. **Element update**: When `RequestRendererBeginRendering` arrives with an `ElementBeginRendering` containing `updatedElements` (a list of `OrdinaryElementDescVariant`), each variant is dispatched via `Apply(Overloading(...))` to the corresponding `RequestRendererUpdateElement_*` helper. These helpers look up the element by ID in `availableElements` and set its properties on the real graphics element.
2. **DOM update**: The core sends `RequestRendererRenderDom` (full DOM) or `RequestRendererRenderDomDiff` (incremental diff) to update the rendering DOM tree stored in `renderingDom`. The old command-based rendering (`RequestRendererBeginBoundary`/`RequestRendererEndBoundary`/`RequestRendererRenderElement`) is disabled and raises `CHECK_FAIL`.
3. **Actual rendering**: In `GlobalTimer()`, if `needRefresh` is true, `Render(renderingDom, rt)` recursively traverses the DOM tree, rendering each element with proper clipping via the native render target. This draws the actual pixels on screen.
4. **Measurement feedback**: After rendering, the renderer checks if label measurements changed and populates `elementMeasurings` with `fontHeights`, `minSizes`, `createdImages` metadata, and `inlineObjectBounds`. The response is sent via `RespondRendererEndRendering(id, elementMeasurings)`.

### Event Forwarding

`GuiRemoteRendererSingle` implements all `INativeWindowListener` mouse and keyboard callbacks. Events are forwarded to the core side via `IGuiRemoteProtocolEvents`:

- **Discrete events** (button clicks, key press/release): Sent immediately as `OnIOButtonDown`, `OnIOButtonUp`, `OnIOKeyDown`, `OnIOKeyUp`, `OnIOChar`, etc.
- **High-frequency events** (mouse move, wheel scroll, key auto-repeat): Accumulated and coalesced to reduce protocol traffic:
  - `pendingMouseMove`: Only the latest mouse position is kept.
  - `pendingHWheel` / `pendingVWheel`: Wheel deltas are summed.
  - `pendingKeyAutoDown`: Only the latest auto-repeat key info is kept.
  - `SendAccumulatedMessages()` is called from `GlobalTimer()` to flush all accumulated events in a single batch.
- **Window lifecycle events**: `Opened` → `OnControllerConnect`, `BeforeClosing` → `OnControllerRequestExit`, `Moved` → `OnWindowBoundsUpdated`, `DpiChanged` → `OnControllerScreenUpdated`, `RenderingAsActivated`/`RenderingAsDeactivated` → `OnWindowActivatedUpdated`.

### Hit Testing

Hit testing is performed locally in the renderer by traversing the rendering DOM tree via `HitTestInternal`. Each DOM node carries `hitTestResult` and `cursor` attributes set by the core side. The renderer walks the tree to find the matching node for a given screen point. This avoids network round-trips for every mouse move — the renderer resolves hit testing locally and only sends the final mouse event with the resolved position.

### Caret Handling

Paragraph elements with active carets are tracked in `focusedParagraphElements`. The `GlobalTimer()` method handles caret blinking by checking elapsed time against `CaretInterval` (500ms) and calling `OnCaretNotify()` on focused paragraph elements.

## Protocol Serialization and Channel Architecture

The channel layer provides a composable, layered architecture for transforming typed protocol calls into serialized data suitable for network transport. All channel types are defined in `Source/PlatformProviders/Remote/`.

### Core Interfaces

- `IGuiRemoteProtocolChannelReceiver<TPackage>`: Receives packages via `OnReceive(const TPackage& package)`.
- `IGuiRemoteProtocolChannel<TPackage>`: A bidirectional channel with `Initialize(receiver)`, `Write(package)`, `Submit(disconnected)`, `GetReceiver()`, `GetExecutablePath()`, and `GetRemoteEventProcessor()`.

These interfaces are generic over `TPackage`, enabling type-safe composition of channel transformers.

### Channel Transformer Pattern

`GuiRemoteProtocolChannelTransformerBase<TFrom, TTo>` bridges two channel types. It implements `IGuiRemoteProtocolChannel<TFrom>` while wrapping `IGuiRemoteProtocolChannel<TTo>`. Two specializations handle the direction of conversion:

- `GuiRemoteProtocolChannelSerializer<TSerialization>`: On `Write`, calls `TSerialization::Serialize` to convert from `SourceType` to `DestType` and writes to the inner channel. On `OnReceive`, calls `TSerialization::Deserialize` to convert back and forwards to the outer receiver.
- `GuiRemoteProtocolChannelDeserializer<TSerialization>`: The reversed direction — calls `Deserialize` on `Write` and `Serialize` on `OnReceive`.

The `TSerialization` concept requires a struct with:
- `SourceType` — the outer type
- `DestType` — the inner type
- `ContextType` — additional context (can be `std::nullptr_t`)
- `static void Serialize(const ContextType&, const SourceType&, DestType&)`
- `static void Deserialize(const ContextType&, const DestType&, SourceType&)`

### Layer 1: IGuiRemoteProtocol ↔ JSON Channel

Two adapter classes convert between typed `IGuiRemoteProtocol` calls and `IGuiRemoteProtocolChannel<Ptr<JsonObject>>`:

**`GuiRemoteProtocolFromJsonChannel`** (in `GuiRemoteProtocol_Channel_Json.h`):
- Implements `IGuiRemoteProtocol` and `IJsonChannelReceiver`
- Wraps an `IJsonChannel*` (i.e., `IGuiRemoteProtocolChannel<Ptr<JsonObject>>`)
- When a `RequestNAME(arguments)` method is called, it serializes the arguments to JSON via `ConvertCustomTypeToJson()`, packs them into a JSON envelope with `JsonChannelPack()`, and calls `channel->Write(package)`
- When `OnReceive(jsonPackage)` is called, it unpacks the envelope with `JsonChannelUnpack()`, dispatches by name to the appropriate event or response handler, deserializes via `ConvertJsonToCustomType()`, and calls the corresponding `events->OnNAME()` or `events->RespondNAME()`
- Exposes `BeforeWrite` and `BeforeOnReceive` events for external hooks (used by the demo projects for caching)

**`GuiRemoteJsonChannelFromProtocol`** (in `GuiRemoteProtocol_Channel_Json.h`):
- The reverse adapter — implements `IJsonChannel` and `IGuiRemoteProtocolEvents`
- Wraps an `IGuiRemoteProtocol*`
- When `Write(jsonPackage)` is called, it unpacks the envelope, deserializes arguments, and calls the protocol's `RequestNAME()`
- When protocol events or responses fire, it serializes them to JSON and calls the receiver's `OnReceive()`
- Also exposes `BeforeWrite` and `BeforeOnReceive` events

### JSON Envelope Format

Every package is wrapped in a JSON object by `JsonChannelPack()` / `JsonChannelUnpack()` with the following fields:
- `"semantic"` (string): One of `"Message"`, `"Request"`, `"Response"`, `"Event"`, `"Unknown"`
- `"id"` (number, optional): Present only when `id != -1`; used for request/response correlation
- `"name"` (string): The message name (e.g., `"ControllerGetFontConfig"`, `"IOMouseMoving"`)
- `"arguments"` (object, optional): The serialized arguments, present only when provided

Semantic meanings:
- `"Message"`: A one-way notification with no response expected
- `"Request"`: A request expecting a response, paired with an `"id"`
- `"Response"`: A response to a previous request, paired with the matching `"id"`
- `"Event"`: An asynchronous event from the other side

`ChannelPackageSemanticUnpack()` extracts the semantic and ID from any package without full deserialization — this is used by the async channel for response routing.

### JSON Serialization of Protocol Types

Protocol types are code-generated from `Protocol/*.txt` files into `GuiRemoteProtocolSchema.h`/`.cpp`. Each struct gets a `JsonHelper<T>` specialization with `ToJson` and `FromJson` methods. The shared infrastructure in `GuiRemoteProtocolSchemaShared.h` provides:
- Primitive type serializers: `bool`, `vint`, `float`, `double`, `WString`, `wchar_t`, `VKEY`, `Color`, `Ptr<MemoryStream>` (Base64-encoded)
- Generic container serializers: `Nullable<T>`, `Ptr<T>`, `List<T>`, `ArrayMap<K,V,F>`, `Dictionary<K,V>`
- `Variant` types serialized with a type discriminator field
- `ConvertCustomTypeToJsonField` for adding named fields to a JSON object

### Layer 2: JSON ↔ String Channel

`JsonToStringSerializer` handles `Ptr<JsonObject>` ↔ `WString` conversion:
- `SourceType = Ptr<glr::json::JsonObject>`
- `DestType = WString`
- `ContextType = Ptr<glr::json::Parser>` (the JSON parser instance)
- `Serialize`: Converts JSON to compact string via `JsonToString`
- `Deserialize`: Parses string to JSON via `JsonParse`

Type aliases:
- `GuiRemoteJsonChannelStringSerializer` = `GuiRemoteProtocolChannelSerializer<JsonToStringSerializer>`
- `GuiRemoteJsonChannelStringDeserializer` = `GuiRemoteProtocolChannelDeserializer<JsonToStringSerializer>`

### Layer 3: String ↔ Transport (User-Implemented)

The user provides an `IGuiRemoteProtocolChannel<WString>` that sends and receives strings over any transport mechanism (named pipe, HTTP, WebSocket, etc.). GacUI does not provide transport implementations directly — the demo projects show how to build them.

### Optional: UTF String Conversion

`UtfStringSerializer<TFrom, TTo>` (defined in `GuiRemoteProtocol_Channel_Shared.h`) converts between different `ObjectString` character types using `ConvertUtfString()`. This enables transport over channels with different string encodings:
- `GuiRemoteUtfStringChannelSerializer<TFrom, TTo>` = `GuiRemoteProtocolChannelSerializer<UtfStringSerializer<TFrom, TTo>>`
- `GuiRemoteUtfStringChannelDeserializer<TFrom, TTo>` = `GuiRemoteProtocolChannelDeserializer<UtfStringSerializer<TFrom, TTo>>`

### Async Channel (Thread Separation)

`GuiRemoteProtocolAsyncChannelSerializer<TPackage>` (in `GuiRemoteProtocol_Channel_Async.h`) provides dual-thread separation between UI logic and IO operations. The JSON-specialized type alias is `GuiRemoteProtocolAsyncJsonChannelSerializer`.

#### Dual-Thread Architecture

- **UI thread** (`UIThreadProc`): Executes the user-provided `uiMainProc` callback, which sets up the GacUI application. All `Write()` calls from the UI thread queue packages in `uiPendingPackages`. The `Submit()` call transfers queued packages to the channel thread and blocks until all expected responses arrive.
- **Channel thread** (`ChannelThreadProc`): Runs in a loop, executing queued IO tasks. All actual `channel->Write()` and `channel->Submit()` calls to the underlying channel happen here.
- `Start(channel, uiMainProc, startingProc)` initializes and launches both threads. The `startingProc` callback is responsible for creating the actual threads (allowing the caller to choose the threading mechanism).

#### Request/Response Correlation

When `Submit()` is called on the UI thread:
1. All pending packages are grouped into a `PendingRequestGroup` containing all request IDs and the current `connectionCounter`.
2. The batch is queued for the channel thread to write.
3. The UI thread blocks on `eventAutoResponses` (an `EventObject`).
4. When `OnReceive()` is called from the channel thread:
   - Events (semantic `"Event"`) are added to `queuedEvents` (protected by `lockEvents`).
   - Responses (semantic `"Response"`) are added to `queuedResponses` map by ID (protected by `lockResponses`).
5. `AreCurrentPendingRequestGroupSatisfied()` checks whether all request IDs in the current group have responses, or whether the connection has been lost.
6. When satisfied, `eventAutoResponses` is signaled, unblocking the UI thread.
7. The UI thread collects all responses for the current request IDs and forwards them to the receiver.

#### Connection Tracking

`connectionCounter` is incremented on each connection/disconnection event. If a disconnection occurs while requests are pending, `AreCurrentPendingRequestGroupSatisfied()` detects the counter mismatch and unblocks `Submit()`, preventing deadlocks.

#### Cross-Thread Execution

`ExecuteInChannelThread(func)` queues a lambda to be executed on the channel thread. This is used by transport implementations that receive data on a network thread and need to route it to the channel thread for processing.

## Demo Project Pair

Two demo projects demonstrate a complete real remote deployment using named pipes or HTTP transport. They are not part of the GacUI library itself but serve as reference implementations.

### RemotingTest_Core (Core Side — Console Application)

Located at `Test/GacUISrc/RemotingTest_Core/`.

#### Protocol Stack Setup

The `StartServer<TServer>` template function in `GuiMain.cpp` builds the core-side protocol stack:

```
SetupRemoteNativeController(protocol)
  → GuiRemoteProtocolDomDiffConverter(&filteredProtocol)
    → GuiRemoteProtocolFilter(&channelSender)
      → GuiRemoteProtocolFromJsonChannel(asyncChannel)
        → GuiRemoteProtocolAsyncJsonChannelSerializer
          → GuiRemoteJsonChannelStringSerializer(&serverCoreChannel, jsonParser)
            → CoreChannel (IGuiRemoteProtocolChannel<WString>)
              → INetworkProtocol (named pipe / HTTP server)
```

Messages flow from `SetupRemoteNativeController` outward through the stack to the network. Events and responses flow inward from the network through the same stack in reverse.

#### CoreChannel

`CoreChannel` implements `IGuiRemoteProtocolChannel<WString>` and `INetworkProtocolCoreCallback`:
- `Write(WString)`: Accumulates messages in `pendingMessages`.
- `Submit()`: Dispatches all accumulated messages to the network via `networkProtocol->SendStringArray()`.
- `OnReadStringThreadUnsafe()`: Called when strings arrive from the network on a network thread. Routes them to the channel thread via `asyncChannel->ExecuteInChannelThread()`.
- Detects `ControllerConnect` event JSON (by string matching `"ControllerConnect"`) to track connection state.
- On reconnection (`OnReconnectedUnsafe`): Injects a synthetic `ControllerDisconnect` event to notify the core side of the previous disconnection before the new connection is established.

### RemotingTest_Rendering_Win32 (Renderer Side — Windows Application)

Located at `Test/GacUISrc/RemotingTest_Rendering_Win32/`.

#### Protocol Stack Setup

The `StartClient<TClient>` template function in `GuiMain.cpp` builds the renderer-side protocol stack:

```
INetworkProtocol (named pipe / HTTP client)
  → RendererChannel
    → GuiRemoteJsonChannelStringDeserializer(&channelReceiver, jsonParser)
      → GuiRemoteJsonChannelFromProtocol(&remoteRenderer)
        → GuiRemoteRendererSingle (IGuiRemoteProtocol)
          → Native window rendering (via real platform provider)
```

Messages arrive from the network and flow inward to `GuiRemoteRendererSingle`. Events and responses flow outward from `GuiRemoteRendererSingle` through the same stack to the network.

#### RendererChannel

`RendererChannel` bridges between the network transport and the JSON string channel:
- `OnReadStringThreadUnsafe()`: Receives strings from the network on a network thread, dispatches to the UI thread via `InvokeInMainThread()`. Handles error strings (prefixed with `!`) by showing a `MessageBox` and calling `renderer->ForceExitByFatelError()`.
- `OnReceive(WString)`: Called by the string channel when the renderer side produces output. Implements a response-caching mechanism to batch network sends.

#### Response Caching Mechanism

`RendererChannel` hooks into `GuiRemoteJsonChannelFromProtocol`'s `BeforeWrite` and `BeforeOnReceive` events to implement caching:
1. `BeforeWrite(info)`: When a `Request` semantic is about to be written to the inner protocol, sets `isCaching = true`.
2. While caching is active, all outgoing packages from `OnReceive()` are accumulated in `cachedPackages` instead of being sent immediately.
3. `BeforeOnReceive(info)`: When a `Response` semantic is about to be delivered to the inner protocol, sets `isCaching = false`.
4. On the next `OnReceive()` after caching is disabled, all cached packages plus the current package are sent as a single batch via `networkProtocol->SendStringArray()`.

This batching ensures that a request and all its side-effect messages (responses, notifications) are sent together, reducing network round-trips.

### Shared Network Protocol Interface

Both demo projects share `Shared/ProtocolCallback.h` defining:
- `INetworkProtocol`: Interface with `SendStringArray(msgs)` for sending and a callback for receiving strings.
- `INetworkProtocolCoreCallback` / `INetworkProtocolCallback`: Callbacks for receiving strings and lifecycle events (reconnection, errors).

Transport implementations (named pipe server/client, HTTP server/client) are in `Shared/` and implement `INetworkProtocol`. These are demonstration implementations and not part of the GacUI library.

### Key Design Point

The transport layer (`INetworkProtocol`) is intentionally decoupled from GacUI. GacUI provides everything above the `IGuiRemoteProtocolChannel<WString>` level. Users implement their own `IGuiRemoteProtocolChannel<WString>` (or `IGuiRemoteProtocolChannel<ObjectString<char>>` with UTF conversion) to plug in any transport mechanism. The demo projects show how to do this for named pipes and HTTP.
