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

## IMPROVEMENTS

## (API|DESIGN) EXPLANATION

## DOCUMENT
