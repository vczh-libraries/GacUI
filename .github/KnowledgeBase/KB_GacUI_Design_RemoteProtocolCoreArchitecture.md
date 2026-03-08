Remote Protocol Core Architecture

Remote protocol mode separates GacUI into two sides: the **core side** runs all application logic and framework code, while the **renderer side** (client) handles actual rendering and OS services. They communicate through `IGuiRemoteProtocol`. The core side is always wrapped in hosted mode — `GuiHostedController` wraps `GuiRemoteController`.

Implementation lives in `Source/PlatformProviders/Remote/`.

## Motivation

In native modes (Windows Direct2D, GDI), GacUI couples directly to OS window management, rendering APIs, and input handling. Remote protocol decouples all of this: the core side becomes platform-independent and only speaks a protocol. This enables:
- Running GacUI applications in a web browser, terminal, or any custom renderer
- Unit testing without a real window system
- Reconnection support — the core side can survive renderer disconnection and restore full visual state when a new renderer connects

## Protocol Design: Messages, Events, and Responses

The protocol has a strict directional split defined by three interface layers in `GuiRemoteProtocol.h`:

### IGuiRemoteProtocolMessages (Core Side → Renderer Side)

Messages are requests sent from the core side to the renderer side. They cover controller configuration queries, window operations, rendering commands, and image management. Some messages expect responses (indicated by `_RES` tag in the protocol definition).

### IGuiRemoteProtocolEvents (Renderer Side → Core Side)

Events are notifications sent from the renderer side to the core side. They include connection lifecycle (`OnControllerConnect`, `OnControllerDisconnect`, `OnControllerRequestExit`), user input (`OnIOKeyDown`, `OnIOKeyUp`, `OnIOMouseMoving`, etc.), and window state changes (`OnWindowBoundsUpdated`).

Responses are also delivered through `IGuiRemoteProtocolEvents` via `RespondNAME(id, arguments)` methods, matched to their originating request by request ID.

### Protocol Definition Files

Protocol messages and events are defined in `Protocol/*.txt` files (e.g., `Protocol_Controller.txt`, `Protocol_Renderer.txt`, `Protocol_IO.txt`) and code-generated into `GuiRemoteProtocolSchema.h`. The generated macros `GACUI_REMOTEPROTOCOL_MESSAGES` and `GACUI_REMOTEPROTOCOL_EVENTS` carry drop annotations (`[@DropRepeat]`, `[@DropConsecutive]`) used by the filter layer for traffic optimization.

## Request/Response Synchronization (GuiRemoteMessages)

`GuiRemoteMessages` wraps `IGuiRemoteProtocol` to provide a synchronous request-response model with batching:

1. **Queue requests**: Each `RequestNAME(...)` call enqueues a message. If the message expects a response, an auto-incrementing request ID (`++id`) is assigned and returned.
2. **Submit**: `Submit()` flushes all queued messages to the renderer side via `IGuiRemoteProtocol::Submit()`. This call is **blocking** — it waits until all responses have been received.
3. **Retrieve responses**: `RetrieveNAME(id)` fetches the response for a given request ID.

The core side operates synchronously from its own perspective despite the underlying protocol potentially being asynchronous. Batching multiple requests before a single `Submit()` is the primary mechanism for reducing round-trip overhead.

`GuiRemoteEvents` implements `IGuiRemoteProtocolEvents` on the core side. It receives both responses (stored for later retrieval by `GuiRemoteMessages`) and events (dispatched to `GuiRemoteController`, `GuiRemoteWindow`, etc.).

## GuiRemoteController: Virtual Native Controller

`GuiRemoteController` implements `INativeController` and all sub-services as virtual stubs that communicate via protocol. It inherits from:
- `Object`
- `INativeController`
- `INativeResourceService`
- `INativeInputService`
- `INativeScreenService` / `INativeScreen`
- `INativeWindowService`

### Service Design Decisions

- **Single window only**: `CreateNativeWindow` can only be called once (enforced by `CHECK_ERROR`). Multiple sub-windows are managed by `GuiHostedController` through hosted mode.
- **Intentionally null services**: `ClipboardService()` and `DialogService()` return `nullptr`, causing GacUI to fall back to built-in fakes (`FakeDialogServiceBase`). This is intentional — the core side cannot access real OS clipboard or dialogs.
- **Synchronous key state queries**: `IsKeyPressing()` and `IsKeyToggled()` perform synchronous request-submit-retrieve round-trips. This is expensive but rarely called.
- **Global shortcut keys**: Tracked core-side and batch-sent to the renderer side on update.

## Connection Lifecycle

### Startup (SetupRemoteNativeController)

`SetupRemoteNativeController(IGuiRemoteProtocol* protocol)` creates a layered stack:
1. `GuiRemoteController` — implements `INativeController`, wraps the protocol
2. `GuiHostedController` — wraps `GuiRemoteController` for hosted mode sub-window support
3. `GuiRemoteGraphicsResourceManager` — implements `GuiGraphicsResourceManager` and `IGuiGraphicsLayoutProvider`
4. `GuiHostedGraphicsResourceManager` — wraps the above for hosted mode rendering

Initialization order: remote → resource manager → hosted controller. Finalization order: reverse. Then `GuiApplicationMain()` runs.

### Connection Establishing

A connection begins when the renderer side fires `OnControllerConnect(ControllerGlobalConfig)`. The core side responds:
1. Acknowledges via `RequestControllerConnectionEstablished()`
2. Fetches `FontConfig` and `ScreenConfig` in one batched `Submit()`
3. Propagates the connection event to `remoteWindow`, `imageService`, and `resourceManager`, each of which restores its state

`GuiRemoteWindow::OnControllerConnect()` re-sends all current window styles when `applicationRunning` is true, enabling seamless reconnection — the renderer side experiences a fresh complete window state setup.

### Disconnection

The renderer side fires `OnControllerDisconnect()`. Each subsystem (`GuiRemoteWindow`, `GuiRemoteGraphicsImageService`, `GuiRemoteGraphicsResourceManager`) marks itself disconnected and suspends protocol communication until reconnection.

### Graceful Exit

The renderer side fires `OnControllerRequestExit()`:
1. `remoteWindow.Hide(true)` triggers the close sequence
2. `BeforeClosing` / `AfterClosing` listener chain fires
3. `DestroyNativeWindow` is scheduled

`connectionForcedToStop` bypasses the `BeforeClosing` cancellation check for forced shutdown.

### Run Loop (RunOneCycle)

`GuiRemoteController::RunOneCycle()` drives the core side:
1. Process incoming events via `IGuiRemoteEventProcessor::ProcessRemoteEvents()`
2. `Submit()` flushes queued messages
3. If timer enabled: `InvokeGlobalTimer()` — triggers hosted controller rendering
4. Execute async tasks

The timer-driven rendering is crucial: `InvokeGlobalTimer()` causes `GuiHostedController` to decide whether to render, which calls into `GuiRemoteGraphicsRenderTarget`.

## GuiRemoteWindow: Virtual Native Window

`GuiRemoteWindow` implements `INativeWindow` for one virtual window.

### Timer-Driven Rendering

`IsActivelyRefreshing()` returns `true`, forcing hosted mode to render every cycle. `RedrawContent()` is a no-op because rendering is driven by the timer in `RunOneCycle`, not on-demand.

### Size and Bounds

Bounds and sizing use `remoteWindowSizingConfig` obtained from the renderer side. A `sizingConfigInvalidated` flag triggers a synchronous re-fetch when bounds are read — this ensures that style changes that affect the window frame are reflected before the next layout pass.

### Style Change Protocol

Two macros manage window style synchronization:
- `SET_REMOTE_WINDOW_STYLE(STYLE, VALUE)` — checks for value change, then sends `RequestWindowNotifySetSTYLE` if changed
- `SET_REMOTE_WINDOW_STYLE_INVALIDATE(STYLE, VALUE)` — same as above, but also marks `sizingConfigInvalidated = true` for styles that affect window sizing (e.g., border, title bar)

Managed properties: Title, CustomFrameMode, ShowInTaskBar, MaximizedBox, MinimizedBox, Border, SizeBox, IconVisible, TitleBar, TopMost.

### Reconnection

On reconnection (`OnControllerConnect`), `GuiRemoteWindow` re-sends all current window styles to the renderer side, so the renderer side can reconstruct the window exactly as it was.

## Rendering Pipeline

The rendering system bridges GacUI's composition-tree rendering model and the remote protocol's message-based communication. In native mode, rendering is immediate: traverse the composition tree, call graphics APIs. In remote mode, each element exists as an ID on both sides. The core side must track changes, batch element updates, send rendering commands, and receive measurement feedback.

### Element Lifecycle

Every visual element has a core-side renderer implementing `IGuiRemoteProtocolElementRender`, with a base class template `GuiRemoteProtocolElementRenderer<TElement, TRenderer, _RendererType>`. The concrete renderer classes include:
- `GuiFocusRectangleElementRenderer`
- `GuiRawElementRenderer`
- `GuiSolidBorderElementRenderer`, `Gui3DBorderElementRenderer`, `Gui3DSplitterElementRenderer`
- `GuiSolidBackgroundElementRenderer`, `GuiGradientBackgroundElementRenderer`
- `GuiInnerShadowElementRenderer`
- `GuiSolidLabelElementRenderer`
- `GuiImageFrameElementRenderer`
- `GuiPolygonElementRenderer`

Element lifecycle:
1. **Creation**: When a renderer's render target is set (`RenderTargetChangedInternal`), it allocates an ID via `AllocateNewElementId()` and registers with `GuiRemoteGraphicsRenderTarget`. The ID is added to `createdRenderers`.
2. **Notification**: `EnsureRequestedRenderersCreated()` (called at the start of each frame) batches pending creates and destroys into `RequestRendererCreated` / `RequestRendererDestroyed` messages to the renderer side.
3. **Destruction**: `FinalizeInternal()` calls `UnregisterRenderer()`, moving the ID to `destroyedRenderers`.

### Frame Rendering Flow

A single rendering frame follows this sequence:

**StartRenderingOnNativeWindow()**:
1. Capture current canvas size from `remoteWindow.GetClientSize()`
2. Increment `renderingBatchId` (tracks which elements rendered this frame)
3. `EnsureRequestedRenderersCreated()` — flush element create/destroy batches
4. Build `ElementBeginRendering` with updated element descriptions:
   - **Normal mode**: iterate all renderers, collect only those where `IsUpdated()` returns true
   - **Reconnect mode** (`needFullElementUpdateOnNextFrame`): collect ALL renderers with `fullContent=true`, ensuring the new renderer side gets complete state
5. Each renderer's `SendUpdateElementMessages()` serializes its current state into the update list
6. For renderers needing min size from cache (`NeedUpdateMinSizeFromCache()`), add to `renderersAskingForCache`
7. Send `RequestRendererBeginRendering` with the frame ID and all updates

**Composition tree traversal**:
- Each element's `Render(bounds)` sends `RequestRendererRenderElement` with the element ID, bounds, and clipped area
- Clipper push/pop sends `RequestRendererBeginBoundary` / `RequestRendererEndBoundary` with hit test results and cursor types per composition

**StopRenderingOnNativeWindow()**:
1. Send `RequestRendererEndRendering`, `Submit()`, retrieve `ElementMeasurings` response
2. Process four categories of measurement feedback (see below)
3. If any min sizes changed, call `hostedController->RequestRefresh()` to trigger another frame
4. Return `ResizeWhileRendering` if canvas size changed during rendering

### Element Update Mechanism (Diff-Based)

Each element renderer tracks its own dirty state:
- `updated` flag: set by `OnElementStateChanged()` when element properties change
- `renderTargetChanged` flag: set when the render target is first assigned

`SendUpdateElementMessages(fullContent, updatedElements)` serializes the element's current visual state. The `fullContent` parameter controls whether to include all properties or just changed ones.

For example, `GuiSolidLabelElementRenderer::SendUpdateElementMessages()` only includes font and text when they actually changed (comparing `lastFont`/`lastText`), unless `fullContent` or `renderTargetChanged` is true. This diff-based update minimizes protocol traffic — for a typical frame, most elements are unchanged.

### Measurement Feedback Loop

The core side cannot measure text or images locally — it must ask the renderer side. This creates a feedback loop processed in `StopRenderingOnNativeWindow()`:

1. **Font heights**: `GuiSolidLabelElementRenderer` includes a `measuringRequest` field (either `FontHeight` or `TotalSize`). The renderer side measures and returns `fontHeights` in `ElementMeasurings`, cached in `renderTarget->fontHeights` keyed by `(fontFamily, fontSize)`.

2. **Min sizes**: The renderer side returns `minSizes` per element ID. `StopRenderingOnNativeWindow()` calls `UpdateMinSize()` on each renderer.

3. **Image metadata**: Newly created images return metadata (dimensions, format) via `createdImages`. `GuiRemoteGraphicsImage::UpdateFromImageMetadata()` stores this.

4. **Inline object bounds**: For `DocumentParagraph` elements with inline objects, the renderer returns layout bounds per callback ID via `inlineObjectBounds`.

**Convergence**: If any min size changed, `hostedController->RequestRefresh()` schedules another rendering frame. The layout engine uses the new min sizes, potentially repositioning elements, requiring another render. This converges quickly because min sizes stabilize after one or two iterations.

**Font height caching**: `TryFetchMinSizeFromCache()` in `GuiSolidLabelElementRenderer` checks whether the needed font height is already in `renderTarget->fontHeights`. If cached, it uses the value directly and skips the measurement request, avoiding redundant round-trips.

### Rendering Boundary Tracking

During composition tree traversal, clipper operations emit boundary messages to enable renderer-side hit testing and cursor display:
- `AfterPushedClipper()`: inspects the composition for hit test results and cursor types. If either is set and differs from the current stack top, sends `RequestRendererBeginBoundary` with the composition's `remoteId`, bounds, and clipped area.
- `AfterPoppedClipper()`: sends `RequestRendererEndBoundary` for the matching boundary.

This allows the renderer side to know which composition each pixel belongs to without needing the full composition tree.

## GuiRemoteGraphicsParagraph (IGuiGraphicsParagraph)

`GuiRemoteGraphicsParagraph` is the remote implementation of `IGuiGraphicsParagraph`, handling rich text layout for document elements. It is the most protocol-intensive part of the rendering system.

### Why Paragraphs Are Special

Unlike ordinary elements (borders, backgrounds, labels) that send their visual state and let the renderer side draw them, paragraphs need **bidirectional communication**:
- The core side defines text content, styling, and inline objects
- The renderer side performs text layout (line breaking, glyph positioning)
- The core side needs layout results (sizes, caret positions, inline object positions) for hit testing, caret navigation, and selection

### Lifecycle

Paragraphs are created by `GuiRemoteGraphicsResourceManager::CreateParagraph()`. Each paragraph gets its own element ID (shared ID space with element renderers), registered via `RegisterParagraph()` instead of `RegisterRenderer()`. Creations are batched in `pendingParagraphCreations` alongside element creates in `EnsureRequestedRenderersCreated()`.

### Run Property System

Paragraph styling uses a run-based system with three layers:
1. **`DocumentTextRunPropertyMap textRuns`**: text-level properties (font, size, color, style) by `CaretRange`
2. **`DocumentInlineObjectRunPropertyMap inlineObjectRuns`**: inline objects (images, controls embedded in text) by `CaretRange`
3. **`DocumentRunPropertyMap stagedRuns` / `committedRuns`**: merged result used for diff computation

`AddTextRun()` handles overlapping ranges by splitting and merging, supporting incremental property changes. `AddInlineObjectRun()` requires exact range matches (inline objects cannot be split). `MergeRuns()` combines both maps, with inline objects taking priority over text runs.

### Core Synchronization (EnsureRemoteParagraphSynced)

This is the core synchronization method, called before any query:
1. `EnsureRequestedRenderersCreated()` — ensures the paragraph element exists on the renderer side
2. Merge text and inline object runs into `stagedRuns`
3. `DiffRuns(committedRuns, stagedRuns, desc)` — compute diff between last committed and current state
4. Send `RequestRendererUpdateElement_DocumentParagraph(desc)` with the diff
5. `Submit()` synchronously — the response includes the new `documentSize`
6. Store `cachedSize` from response, swap `stagedRuns` to `committedRuns`

The first sync sends full text content; subsequent syncs send only run property diffs. This is efficient for scenarios like syntax highlighting where only styling changes between frames.

### Layout Queries (Delegated to Renderer Side)

All layout-dependent operations require synchronous round-trips or cached data:
- **`GetSize()`**: returns `cachedSize` from the last sync
- **`GetCaret(comparingCaret, position, preferFrontSide)`**: sends `RequestDocumentParagraph_GetCaret` — the renderer side performs text layout-aware caret navigation
- **`GetCaretBounds(caret, frontSide)`**: uses `GetCaretBoundsInternal()` which lazily fetches ALL caret bounds (front and back arrays) in one request via `RequestDocumentParagraph_GetCaretBounds` and caches them in `cachedCaretBounds`. Subsequent calls use the cache.
- **`GetCaretFromPoint(point)`**: iterates all caret positions locally using cached bounds from `GetCaretBoundsInternal()`, finding the nearest by Manhattan distance — no additional remote call if bounds are cached
- **`GetInlineObjectFromPoint(point)`**: sends `RequestDocumentParagraph_GetInlineObjectFromPoint` for hit-test, then looks up `inlineObjectProperties` locally
- **`GetNearestCaretFromTextPos(textPos, frontSide)`**: sends `RequestDocumentParagraph_GetNearestCaretFromTextPos`
- **`IsValidCaret(caret)`**: sends `RequestDocumentParagraph_IsValidCaret`
- **`IsValidTextPos(textPos)`**: purely local — checks bounds against `text.Length()`

### Caret Display

- `EnableCaret()` sends `RequestDocumentParagraph_OpenCaret` with position, color, and front/back side
- `DisableCaret()` sends `RequestDocumentParagraph_CloseCaret`
- `BlinkCaret()` returns false — blinking is handled renderer-side

### Paragraph Rendering

`GuiRemoteGraphicsParagraph::Render(bounds)`:
1. `EnsureRemoteParagraphSynced()` — ensure current state sent to renderer side
2. For each inline object with cached bounds, call `callback->OnRenderInlineObject()` — if the inline object's size changed, update `inlineObjectRuns` and mark dirty
3. Send `RequestRendererRenderElement` — same as ordinary elements

### Dirty Tracking

`MarkParagraphDirty(invalidateSize, invalidateCaretBoundsCache)` sets:
- `needUpdate = true` always
- `cachedSize = {0,0}` if `invalidateSize` is true (forcing re-measurement from renderer side)
- `needUpdateCaretBoundsCache = true` if `invalidateCaretBoundsCache` is true (forcing caret bounds refetch)

Size-affecting changes (font, size, text content, wrap line, max width) set both invalidation flags. Color-only changes skip size invalidation because they don't affect layout.

## DOM Diff Layer

### Motivation

Without DOM diff, each frame sends per-element rendering commands (`RenderElement`, `BeginBoundary`, `EndBoundary`). For a typical GacUI application with hundreds of elements, most of which don't move between frames, this is wasteful. The DOM diff layer converts the per-frame command stream into a tree structure and diffs against the previous frame, sending only structural changes.

### How It Works

`GuiRemoteProtocolDomDiffConverter` extends `GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>` and wraps `IGuiRemoteProtocol`:
- During rendering, `RenderingDomBuilder` builds a `RenderingDom` tree from the intercepted boundary and element commands
- On `RequestRendererEndRendering`:
  - First frame: sends `RequestRendererRenderDom` with the full tree
  - Subsequent frames: `DiffDom(lastDom, lastDomIndex, newDom, newDomIndex, diffs)` computes structural diffs, sends `RequestRendererRenderDomDiff`
- `lastDom` is stored for next frame; `OnControllerConnect` clears it to force a full DOM send on reconnection

DOM node IDs encode their type: element IDs use `(elementId << 2) + 0`, hit test compositions use `(compositionId << 2) + 2`, with parent variants at `+1` and `+3`.

## Protocol Combinator Layer

The combinator pattern allows composing protocol transformations as a pipeline. `GuiRemoteProtocolCombinator<TEvents>` (in `GuiRemoteProtocol_Shared.h`) wraps an `IGuiRemoteProtocol*` and interposes a `TEvents` event combinator. This creates a chain where messages flow core → combinator → target protocol, and events flow target protocol → event combinator → core.

`GuiRemoteProtocolCombinator_PassingThrough` (specialization with `void` events plus `GuiRemoteEventCombinator_PassingThrough`) forwards everything unchanged, allowing subclasses to selectively override only the messages they care about. Both the filter layer and DOM diff layer use this pattern.

## Protocol Filter Layer

`GuiRemoteProtocolFilter` wraps `IGuiRemoteProtocol` via `GuiRemoteProtocolCombinator<GuiRemoteEventFilter>` and optimizes traffic by dropping redundant messages/events:
- `[@DropRepeat]`: drop messages or events with identical arguments to the previous send, regardless of what happened in between
- `[@DropConsecutive]`: drop events with identical arguments to the immediately preceding event of the same type

The filter queues messages internally and applies drop logic in `ProcessRequests()` before `Submit()`.

## Channel Layer

For real remote deployment, `IGuiRemoteProtocolChannel<T>` provides a bidirectional channel abstraction. The typical stack:
1. `GuiRemoteProtocolFromJsonChannel` adapts a JSON channel to `IGuiRemoteProtocol`
2. `GuiRemoteProtocolAsyncChannelSerializer<Ptr<JsonObject>>` (aliased as `GuiRemoteProtocolAsyncJsonChannelSerializer`) runs channel IO on a separate thread

The async serializer's design: the UI thread batches messages and blocks on `Submit()` until the channel thread completes the round-trip. Events from the channel thread are queued and processed on the UI thread during `ProcessRemoteEvents()`. Connection/disconnection is tracked via a `connectionCounter` to handle race conditions when the channel thread delivers events after disconnection.

## Image Service

`GuiRemoteGraphicsImageService` implements `INativeImageService` and creates `GuiRemoteGraphicsImage` objects with core-side binary copies. Image metadata (dimensions, format) is lazily fetched from the renderer side via `ImageCreated` messages in the `ElementMeasurings` response.

On reconnection (`OnControllerConnect`), all existing images are re-registered with the renderer side so it can recreate its local image objects. On disconnection (`OnControllerDisconnect`), the service suspends sending and resets metadata that may have become stale.
