#### Remote Protocol Renderer and Serialization

The remote protocol renderer side receives protocol messages from the core side and translates them into window operations and graphics rendering. Native renderers use the platform's graphics APIs; GacJS renders HTML5 in the browser, with WebAssembly as its primary feature and intended way to run the GacUI core. The serialization and channel infrastructure converts between typed protocol calls and Parser2 JSON node packages independently of the transport, including exposed Wasm functions, named pipes, HTTP, or a user-provided transport.

## GacJS HTML5 Rendering and WebAssembly

GacJS's purpose is to run GacUI applications in the browser through WebAssembly. Its `@gaclib/renderer` package implements HTML5 rendering with DOM elements. The browser loads the core as the matching `app.mjs`, `app.wasm` and `app.worker.js` files; `@gaclib-website/remote-protocol-wasm` connects the renderer to it through exposed functions and callbacks while preserving the remote protocol and its JSON channel messages. HTTP is a parallel transport option, mainly for testing HTML5 rendering against a native core without WebAssembly.

The TypeScript view-model host also supports this Wasm transport. `startRvmHostWithChannel` in `<GacJS repo>/Gaclib/website/rvmhost/src/index.ts` hosts the view model through Workflow RPC on a separate channel, while the renderer uses the GacUI remote protocol. Both channels can share one Wasm core.

The Wasm transport is implemented in `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/`:

- `worker.ts` loads the module and owns transport state, while blocking GacUI work runs on a C++ pthread. The page exchanges messages with the worker and remains available for HTML5 rendering, input and TypeScript RPC handling.
- `index.ts` adapts the worker to the same `IChannelClient` contract used by the HTTP transport. Existing channel handshakes, the semicolon package envelope, and JSON arrays remain in use; protocol traffic does not use HTTP requests or Base64 framing.

The application supplies exported Wasm functions for startup and incoming messages, plus a callback for outgoing messages and lifecycle notifications. Connection lifetime and renderer replacement depend on that application-owned transport contract.

Loading application assets from a web server is distinct from carrying remote-protocol and view-model traffic through Wasm functions. Build and launcher instructions belong in the owning repository's project documentation.

## GuiRemoteRendererSingle

`GuiRemoteRendererSingle` (in `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/`) is the renderer-side implementation that bridges `IGuiRemoteProtocol` to a real native window with actual graphics rendering. It is not a full `INativeController` — it relies on a real platform provider (e.g., Windows Direct2D via `SetupRawWindowsDirect2DRenderer()`) already running, and registers itself as a listener on a native window.

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

- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.cpp`: Construction, destruction, main window registration, connection lifecycle (`Opened`, `BeforeClosing`, `AfterClosing`, `Closed`), screen/config management, core disconnection, and fatal-error retention.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Controller.cpp`: Controller-level requests — `RequestControllerGetFontConfig`, `RequestControllerGetScreenConfig`, `RequestControllerConnectionEstablished`, `RequestControllerConnectionStopped`.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_MainWindow.cpp`: Window style notifications — `RequestWindowNotifySetBounds`, `RequestWindowNotifySetTitle`, `RequestWindowNotifySetEnabled`, `RequestWindowNotifyShow`, etc.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_IO.cpp`: IO requests (global shortcuts, mouse capture, key state queries) and native-to-protocol input event conversion. Contains `SendAccumulatedMessages()` for batching high-frequency events.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering.cpp`: Core rendering pipeline — element creation/destruction (`RequestRendererCreated`, `RequestRendererDestroyed`), begin/end rendering, DOM rendering (`RenderDom` recursive traversal), fatal-overlay rendering, hit testing, and `GlobalTimer`/`Paint` driven refresh.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Elements.cpp`: Property updates on ordinary graphics elements (solid border, sink border, splitter, background, gradient, inner shadow, polygon).
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Label.cpp`: Solid label measurement and property updates.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Image.cpp`: Image creation, metadata, and image frame element updates.
- `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Document.cpp`: Complex document paragraph element rendering via `GuiRemoteDocumentParagraphElement` inner class.

### Rendering Pipeline

When the core sends `RequestRendererBeginRendering(ElementBeginRendering)`, the `updatedElements` field contains a list of `OrdinaryElementDescVariant`. Each variant is dispatched via `Apply(Overloading(...))` to the corresponding `RequestRendererUpdateElement_*` helper, which looks up the element by ID in `availableElements` and sets its properties.

The core sends `RequestRendererRenderDom` or `RequestRendererRenderDomDiff` to update the rendering DOM tree. The legacy command-based rendering path (`RequestRendererBeginBoundary`/`RequestRendererEndBoundary`/`RequestRendererRenderElement`) is disabled with `CHECK_FAIL` — the DOM-diff approach is required.

`RequestRendererRenderDom` installs a full `RenderingDom`, rebuilds `renderingDomIndex`, and sets `needRefresh = true`. `RequestRendererRenderDomDiff` requires an existing DOM, applies `UpdateDomInplace(renderingDom, renderingDomIndex, diffs)`, and also sets `needRefresh = true`.

Actual painting happens in either `RequestRendererEndRendering` or `GlobalTimer()`: when `needRefresh` is true and refresh is no longer suppressed, `ForceRender()` calls `RenderDom` to recursively traverse `renderingDom`, renders each element in order with clipping, optionally draws the fatal-error overlay, redraws the native window, and handles render-target resize/lost-device failures by resizing/recreating the target and scheduling another refresh. `RequestRendererEndRendering` clears suppression and can force the completed frame to render before returning `RespondRendererEndRendering(id, elementMeasurings)`. While connected, `GlobalTimer()` also flushes accumulated IO events and drives caret blinking; after disconnection it clears pending core events but can continue rendering a retained fatal-error overlay. `RequestRendererIdle()` is an idle hint and is intentionally ignored by this renderer.

### Raw Element State and Lifetime

`<Raw/>` is the recommended ownerdraw API whether or not remote protocol is in use. The Core transmits `ElementDesc_Raw` with numeric renderer `id`, application string `rawId`, string `interpretor` and string `data`. The application strings do not contain native handles or pixel frames. Native clients call their active resource manager's `CreateRawElement()` and cast to `GuiRawElementBase`, selecting `GuiDirect2DElement`, `GuiGDIElement`, `GuiWaylandElement` or `GuiCocoaElement` through the provider. TUI has no raw drawing implementation. See [native callback APIs](./KB_GacUI_Design_ImplementingIGuiGraphicsElement.md#ownerdraw-with-raw).

`GuiRemoteRendererSingle::GetRawElements()` returns `const List<Ptr<GuiRawElementBase>>&`. Its `RawElementInstalled`, `RawElementChanged` and `RawElementUninstalled` events pass `GuiRemoteRawElementEventArgs::element`, the actual shared element. All callbacks run on the renderer UI thread:

1. Creation adds the element to the element map and raw collection, then raises `RawElementInstalled`, before assigning its render target. `Id`, `Interpretor` and `Data` are initially empty because descriptors arrive later. Attach native target-lifetime handlers at installation; ID-specific painters must also inspect subsequent change events.
2. Applying a descriptor compares all three strings, sets ID and applies name/data through `SetInterpretorAndData` before raising one `RawElementChanged`, and emits nothing for an identical descriptor. Changed interpretor or data explicitly sets `needRefresh`, since remote elements have no owner composition to invalidate. An ID-only update does not independently request repaint. Core assignments are frame-batched rather than delivered as individual property events.
3. Destruction raises `RawElementUninstalled` while the object is still in the collection and its native target is usable for handler cleanup. It then detaches the render target and removes the element. The same removal path handles numeric-ID replacement and final closing. Retained disconnected/fatal pictures keep their element attachments until actual teardown.

On Windows, Wayland and Cocoa, `GuiRawElementRendererBase` owns the factory-selected interpretor independently of collection membership and target lifetime. Register factories after backend setup and before processing queued descriptors, for example during controller-related plugin loading. A retained element keeps its interpretor after removal/target detachment until its final release. Concrete native destruction finalizes while event members still exist.

For separately owned manual attachments, track attachments by object identity, not by application `Id`: empty and duplicate IDs are allowed. Reconcile on ID changes into or out of the painter's convention, enumerate existing elements for late subscription, and detach handler tokens on removal without assuming collection removal already happened.

GacJS exposes `getRawElements()`, `rawElementInstalled`, `rawElementUpdated`, and `rawElementUninstalled` on `IGacUIRenderer` in `<GacJS repo>/Gaclib/gaclib/renderer/src/interfaces.ts`. Each subscription returns an unsubscribe function. The read-only `GacUIRawElement` record provides `id` (the application string), `data` and `element` (a stable owner `<div>` for application content); the numeric protocol identity remains internal. The renderer owns that container's layout and protocol identity; consumers own children such as canvas, drawing observers and their cleanup. Plain raw elements remain visually empty unless an application painter is attached. DOM visibility is distinct from protocol element lifetime, so hiding a node must not dispose its painter. Snapshot rendering can retain raw descriptors without installing the live application's painter.

Application-defined raw drawing must use the received payload. For a clock, the Core or view model can own `HH:mm:ss` updates while native/browser painters render that value without a second timer. Browser drawing resizes its canvas for current bounds/device pixel ratio and redraws the last received state. Named painter selection remains independent of application IDs and optional manual attachments.

### Event Forwarding

`GuiRemoteRendererSingle` implements all `INativeWindowListener` mouse/keyboard callbacks to forward OS events as protocol events:
- **Discrete events** (button clicks, key presses): Sent immediately via `events->OnIOButtonDown`, `events->OnIOButtonUp`, `events->OnIOButtonDoubleClick`, `events->OnIOKeyDown`, etc. Mouse-button events carry an `IOMouseInfoWithButton` whose `button` is the mapped `NativeMouseButton` value (`Left`, `Middle`, `Right`, `Mouse4`, or `Mouse5`) and whose `info` is the complete `NativeWindowMouseInfo`, including independently captured `alt` and `osSuper` state.
- **High-frequency events** (mouse move, wheel, key auto-repeat): Accumulated and coalesced:
  - `pendingMouseMove`: Only the latest `NativeWindowMouseInfo` is kept and sent directly as `IOMouseInfo`.
  - `pendingHWheel` / `pendingVWheel`: `NativeWindowMouseInfo` wheel deltas are summed across frames and sent directly as `IOMouseInfo`; an `alt` or `osSuper` transition flushes the pending delta before accumulating the next event.
  - `pendingKeyAutoDown`: Only the latest auto-repeat key is kept.
  - `SendAccumulatedMessages()` is called from `GlobalTimer()` to flush these accumulated events.
- **Window lifecycle events** (`Opened`, `BeforeClosing`, `Moved`, `DpiChanged`): Translated to protocol events like `OnControllerConnect`, `OnControllerRequestExit`, `OnWindowBoundsUpdated`.

All outgoing callbacks and responses are guarded by `CanSendEvents()`. `DisconnectFromCore()` marks the renderer as disconnecting, releases mouse capture, unregisters global shortcuts, and clears all accumulated events so no stale input is sent afterward.

### Super-Key Label and Environment Refresh

When the native renderer opens, it sends its resource service's canonical Super-key label in `ControllerGlobalConfig::osSuperKeyName`. The remote core returns that label from `GuiRemoteController::GetOSSuperKeyName()`; before the first renderer supplies a label it returns `osSuper`, and disconnecting does not clear the last received label.

Each renderer connection invokes the controller environment-change path. `GuiApplication` forwards the change through each live `GuiWindow`, which refreshes its display font and calls `GuiComponent::EnvironmentChanged()` on installed components. `GuiToolstripCommand` overrides that hook to raise `DescriptionChanged`, so bound shortcut text adopts a replacement renderer's label. Detached commands are intentionally not notified, and commands do not register individual `INativeControllerListener` instances.

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

Protocol types are code-generated from `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/*.txt` files into `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Generated/GuiRemoteProtocolSchema.h/.cpp`. Each struct gets a `JsonHelper<T>` specialization with `ToJson` and `FromJson` methods. The shared infrastructure in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteProtocolSchemaShared.h` provides:
- Primitive type serializers: `bool`, `vint`, `float`, `double`, `WString`, `wchar_t`, `VKEY`, `Color`, `Ptr<MemoryStream>` (Base64-encoded).
- Generic container serializers: `Nullable<T>`, `Ptr<T>`, `List<T>`, `ArrayMap<K,V,F>`, `Dictionary<K,V>`.
- `Variant` types are serialized with a type discriminator field.
- `ConvertCustomTypeToJsonField` adds a named field to a JSON object.

### Async Channel

`GuiRemoteProtocolAsyncJsonChannel` is the core-side async wrapper around an `IJsonChannel`. It queues outgoing packages, queues incoming events for `ProcessRemoteEvents()`, stores incoming responses by request id, and blocks `BatchWrite(disconnected)` until the current `PendingRequestGroup` is satisfied or disconnected. `connectionCounter` and `connectionClientId` protect pending requests when channel events arrive after disconnect/reconnect boundaries.

`GuiRemoteProtocolAsyncJsonChannelRenderer` is the renderer-side async wrapper. It queues received packages and main-thread tasks, then schedules `ProcessPendingMessages()` through an `IGuiRemoteProtocolAsyncRendererInvoker`. Before `SetInvokeInMainThread(...)` is called by renderer `GuiMain`, work is cached. After the invoker is installed, it is drained in FIFO order on the renderer UI thread. `Initialize(reader)` requires a non-null reader; `Detach()` explicitly clears it, increments `messageVersion`, and drops queued work. The version check prevents callbacks queued before `Detach()` from running after detachment or a later reader installation.

## Protocol Stack Assembly

A Core composes its controller with the DOM-diff converter, protocol filter, Core channel and asynchronous JSON channel. A native renderer composes its network channel client with the asynchronous renderer channel, renderer channel and native renderer. The transport supplies `IJsonChannel` messages independently of those wrappers.

Install the native backend before creating consuming elements or processing queued renderer messages. Retain the main-thread invoker while messages can arrive; clear it and unregister the native window before stopping the connection and destroying stack-owned channel wrappers.

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
              → Named pipe, Windows HTTP, or MiniHTTP transport
```

**Renderer side** (messages flow inward):
```
Named pipe, Windows HTTP, or MiniHTTP transport
  → GuiRemoteProtocolChannelClient / IJsonChannel
    → GuiRemoteProtocolAsyncJsonChannelRenderer
      → GuiRemoteProtocolRendererChannel
        → GuiRemoteRendererSingle (IGuiRemoteProtocol)
          → Native window rendering
```

Events flow in the reverse direction through the same stack.

## Point input

Windows native remote renderers accept touch locally and forward ordered point records without mouse coalescing. Native wGac/iGac renderers do not acquire touch. GacJS supports browser touch with both HTTP and WebAssembly cores, including browsers on Linux/macOS: acquisition uses stable target capture and descendant touch-action policy, and both transports share the schema and renderer code. Unsigned 64-bit point identities/timestamps are canonical decimal strings. Core owns recognition and `GuiScrollView` behavior; renderers do not recognize separate gestures. See [Point Input, Capture and Predefined Gestures](./KB_GacUI_Design_PointInput.md) for payloads, cancellation, callback lifetime and automation examples.

## Feature capabilities and encoded image output

`GuiRemoteRendererSingle::GlobalConfig` defaults its DateTime, Locale and filesystem capabilities to true for native library consumers. Both the Core configuration and the renderer capability must enable a feature. Application startup can explicitly disable these capabilities. GacJS support is separate and is not implied.

`ImageSave` requests encoded bytes for an existing image ID and requested format.
The native renderer calls `INativeImage::SaveToStream`, so a JPEG source can become
the PNG data required by RTF. Core ensures metadata/creation on the current
renderer, checks disconnect and generation before retrieving the response, and
rejects missing/empty encoded output. Image export does not depend on filesystem
injection; a remote image can be serialized to a core-local output stream.

A named Workflow editor is an aggregate: use `SafeAggregationCast<GuiInstanceRootObject>` and scoped `FindObjectByName` to reach its native viewer; a direct C++ cast of the Workflow raw object returns null. Persistence code should depend on public document viewer APIs. Private saves can use a complete `DocumentModel::CopyDocument()` and serialize embedded images. Deep range copies retain every style definition, including unused definitions, parent names and independent authored/resolved properties. The full overload delegates to that policy. Shallow range copies alone filter referenced styles and transitive parents, sharing their objects. Reinsertion compares authored ancestry and reuses equivalent renamed definitions, keeping repeated copy/paste and undo/redo catalogs stable. RTF/HTML export uses the existing document serializers. Verify the copied byte count and finish output close before reporting synchronous save completion.

Keep a private document's complete style catalog through save and load, including styles not yet applied to any text. Application style selectors should use loaded definitions without silently restoring built-ins or replacing authored definitions.

## Native destination selection

Network clients receive their destination from application configuration. Keep the destination host and protocol transport port separate from the local automation endpoint. Named pipes accept server names or IPv4 addresses; HTTP and socket transports support the address formats exposed by their VlppOS implementations. Application command-line parsing and launch procedures belong in project instructions.

## Underlying process services

Core and a native renderer can each install their own underlying service implementations with `vl::InjectDateTimeImpl`, `vl::InjectLocaleImpl` and `vl::filesystem::InjectFileSystemImpl`. These process-local replacements are separate from the remote configuration/capability flags: installing one does not opt into remote injection. Core opt-in defaults, native-renderer capability defaults, DateTime/Locale predecessor fallback and the enabled-but-unavailable filesystem's empty root remain as described above.
