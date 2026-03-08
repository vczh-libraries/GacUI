# !!!KNOWLEDGE BASE!!!

# DESIGN REQUEST

You are going to research about the remote protocol mode. Remote protocol delegates rendering and OS provider to a command-based protocol. But in this topic, ignore actual protocol serialization/deserialization, ignore GuiSingleRendererSingle, ignore unit test that builds on top of remote protocol. You are going to finish a design explanation of how remote protocol is implemented, focus on the starting/stopping, connection establishing, and other details as well as the API design. Those "ignored" parts will be covered in another future topic, here let us focus on the core part.

# INSIGHT

## Overview

Remote protocol mode separates GacUI into two sides: the **core side** runs all application logic and framework code, while the **renderer side** (client) handles actual rendering and OS services. They communicate through `IGuiRemoteProtocol`. The core side is always wrapped in hosted mode (`GuiHostedController` wraps `GuiRemoteController`).

Implementation lives in `Source/PlatformProviders/Remote/`.

## Why Remote Protocol Mode Exists

In native modes (Windows Direct2D, GDI), GacUI couples directly to OS window management, rendering APIs, and input handling. Remote protocol decouples all of this: the core side becomes platform-independent and only speaks a protocol. This enables:
- Running GacUI applications in a web browser, terminal, or any custom renderer
- Unit testing without a real window system
- Reconnection — the core side can survive renderer disconnection and restore full visual state on reconnection

## Protocol Design: Messages vs Events

The protocol has a strict directional split:
- **Messages** flow core → renderer (defined via `IGuiRemoteProtocolMessages`). They are requests, some expecting responses.
- **Events** flow renderer → core (defined via `IGuiRemoteProtocolEvents`). They notify the core of user input, connection changes, and window changes.
- **Responses** also flow renderer → core, but are tied to specific request IDs.

This is defined in `Protocol/*.txt` files (e.g. `Protocol_Controller.txt`, `Protocol_Renderer.txt`) and code-generated into `GuiRemoteProtocolSchema.h`. The generated macros `GACUI_REMOTEPROTOCOL_MESSAGES` and `GACUI_REMOTEPROTOCOL_EVENTS` carry drop annotations (`[@DropRepeat]`, `[@DropConsecutive]`) that the filter layer uses for traffic optimization.

## Request/Response Synchronization Pattern

`GuiRemoteMessages` wraps `IGuiRemoteProtocol` with auto-incrementing request IDs. The fundamental pattern is:
1. Queue one or more requests via `RequestNAME(...)` 
2. Call `Submit()` — this is blocking: it flushes all queued messages and waits for all responses before returning
3. Retrieve responses by ID via `RetrieveNAME(id)`

This means the core side operates synchronously from its own perspective, despite the underlying protocol potentially being async. Batching multiple requests before a single `Submit()` is the primary way to reduce round-trips.

`GuiRemoteEvents` receives responses and events from the renderer side. Responses are stored for later retrieval by `GuiRemoteMessages`. Events are dispatched to the appropriate handler (`GuiRemoteController`, `GuiRemoteWindow`, etc.).

## GuiRemoteController: Virtual Native Controller

`GuiRemoteController` implements `INativeController` and all its sub-services (resource, input, screen, window) as virtual stubs that communicate via protocol. The key design decisions:
- **Single window only**: `CreateNativeWindow` can only be called once (enforced by `CHECK_ERROR`). Hosted mode handles sub-windows.
- **Services returning `nullptr`** (clipboard, dialog) cause GacUI to fall back to built-in fakes — this is intentional, not a limitation.
- **Key state queries** (`IsKeyPressing`, `IsKeyToggled`) are synchronous request-submit-retrieve round-trips — expensive but rarely called.
- **Global shortcut keys** are tracked core-side and batch-sent to the renderer on update.

## Connection Lifecycle

### Startup (SetupRemoteNativeController)

The entry point creates a layered stack on the call stack:
1. `GuiRemoteController` (implements `INativeController`)
2. `GuiHostedController` (wraps it for hosted mode sub-window support)
3. `GuiRemoteGraphicsResourceManager` (implements `GuiGraphicsResourceManager` + `IGuiGraphicsLayoutProvider`)
4. `GuiHostedGraphicsResourceManager` (wraps for hosted mode rendering)

Initialize order: remote → resource manager → hosted. Finalize order: reverse. Then `GuiApplicationMain()` runs.

### Connection Establishing

Triggered by the renderer side firing `OnControllerConnect(ControllerGlobalConfig)`. The core side:
1. Acknowledges via `RequestControllerConnectionEstablished()`
2. Fetches `FontConfig` and `ScreenConfig` in one batched submit
3. Propagates to `remoteWindow`, `imageService`, `resourceManager` — each restores its state

`GuiRemoteWindow::OnControllerConnect()` re-sends all current window styles if `applicationRunning` is true, enabling seamless reconnection.

### Disconnection and Exit

- **Disconnect**: Renderer fires `OnControllerDisconnect()`. Each subsystem marks itself disconnected.
- **Graceful exit**: Renderer fires `OnControllerRequestExit()` → `remoteWindow.Hide(true)` → `BeforeClosing`/`AfterClosing` listener chain → scheduled `DestroyNativeWindow`.
- **Forced exit**: `connectionForcedToStop` bypasses the `BeforeClosing` cancellation check.

### Run Loop

`RunOneCycle()` drives the core side:
1. Process incoming events via `IGuiRemoteEventProcessor::ProcessRemoteEvents()`
2. `Submit()` flushes queued messages
3. If timer enabled: `InvokeGlobalTimer()` — this triggers hosted controller rendering
4. Execute async tasks

The timer-driven rendering is crucial: `InvokeGlobalTimer()` is what causes `GuiHostedController` to decide whether to render, which calls into `GuiRemoteGraphicsRenderTarget`.

## GuiRemoteWindow

Implements `INativeWindow` for one virtual window. Key design points:
- `IsActivelyRefreshing()` returns `true` — forces hosted mode to render every cycle
- Bounds and sizing use `remoteWindowSizingConfig` from the renderer side; the `sizingConfigInvalidated` flag triggers synchronous re-fetch when bounds are read
- Style changes use `SET_REMOTE_WINDOW_STYLE` / `SET_REMOTE_WINDOW_STYLE_INVALIDATE` macros — they check for value change before sending, and the `_INVALIDATE` variant marks sizing as stale
- `RedrawContent()` is a no-op because rendering is timer-driven, not on-demand
- Reconnection re-sends all window state, making the renderer side experience a fresh window setup

## Rendering Pipeline

This is the most complex part of remote protocol. The rendering system bridges the gap between GacUI's composition-tree rendering model and the remote protocol's message-based communication.

### Why Rendering Is Complex

In native mode, rendering is immediate: traverse the composition tree, call graphics APIs. In remote mode, each element exists as an ID on both sides. The core side must:
- Track which elements changed since the last frame
- Batch element updates efficiently
- Send rendering commands that the renderer can replay
- Receive measurement feedback (font heights, min sizes, image metadata, inline object bounds) and feed it back into layout

### Element Lifecycle

Every visual element has a core-side renderer implementing `IGuiRemoteProtocolElementRender`. The lifecycle:
1. **Creation**: When a renderer's render target is set (`RenderTargetChangedInternal`), it allocates an ID via `AllocateNewElementId()` and registers with `GuiRemoteGraphicsRenderTarget`. The ID is added to `createdRenderers`.
2. **Notification to renderer side**: `EnsureRequestedRenderersCreated()` (called at the start of each frame) batches pending creates and destroys into `RequestRendererCreated` / `RequestRendererDestroyed` messages.
3. **Destruction**: `FinalizeInternal()` calls `UnregisterRenderer()`, which moves the ID to `destroyedRenderers`.

The renderer side (`GuiRemoteRendererSingle`) receives `RequestRendererCreated` and creates matching native elements (e.g., `GuiSolidLabelElement`, `GuiSolidBorderElement`) keyed by ID.

### Frame Rendering Flow

A single rendering frame follows this sequence:

**`StartRenderingOnNativeWindow()`**:
1. Capture current canvas size from `remoteWindow.GetClientSize()`
2. Increment `renderingBatchId` (used to track which elements rendered this frame)
3. `EnsureRequestedRenderersCreated()` — flush element create/destroy batches
4. Build `ElementBeginRendering` with all updated element descriptions:
   - **Normal mode**: iterate all renderers, collect only those where `IsUpdated()` is true
   - **Reconnect mode** (`needFullElementUpdateOnNextFrame`): collect ALL renderers with `fullContent=true`
5. Each renderer's `SendUpdateElementMessages()` serializes its current state into the update list
6. For renderers needing min size from cache (`NeedUpdateMinSizeFromCache()`), add to `renderersAskingForCache`
7. Send `RequestRendererBeginRendering` with the frame ID and all updates

**During rendering** (composition tree traversal):
- Each element's `Render(bounds)` sends `RequestRendererRenderElement` with the element ID, bounds, and clipped area
- Clipper push/pop sends `RequestRendererBeginBoundary` / `RequestRendererEndBoundary` with hit test results and cursor types per composition

**`StopRenderingOnNativeWindow()`**:
1. Send `RequestRendererEndRendering`, `Submit()`, retrieve `ElementMeasurings` response
2. Process four categories of feedback (see Measurement Feedback Loop below)
3. If any min sizes changed, call `hostedController->RequestRefresh()` to trigger another frame
4. Return `ResizeWhileRendering` if canvas size changed during rendering

### Element Update Mechanism

Each element renderer tracks its own dirty state:
- `updated` flag: set by `OnElementStateChanged()` (called when element properties change)
- `renderTargetChanged` flag: set when the render target is first assigned
- `SendUpdateElementMessages(fullContent, updatedElements)`: serializes the element's current visual state. The `fullContent` parameter controls whether to include all properties or just changed ones.

For example, `GuiSolidLabelElementRenderer::SendUpdateElementMessages()` includes font and text only when they actually changed (comparing `lastFont`/`lastText`), unless `fullContent` or `renderTargetChanged` is true. This diff-based update minimizes protocol traffic.

### Measurement Feedback Loop

The core side cannot measure text or images locally — it must ask the renderer side. This creates a feedback loop:

1. **Font height**: `GuiSolidLabelElementRenderer` includes a `measuringRequest` field in its update message (either `FontHeight` or `TotalSize`). The renderer side measures and returns `fontHeights` in `ElementMeasurings`. These are cached in `renderTarget->fontHeights` keyed by `(fontFamily, fontSize)`.

2. **Min sizes**: The renderer side returns `minSizes` per element ID. `StopRenderingOnNativeWindow()` calls `UpdateMinSize()` on each renderer.

3. **Image metadata**: Newly created images return metadata (dimensions, format) via `createdImages`. `GuiRemoteGraphicsImage::UpdateFromImageMetadata()` stores this.

4. **Inline object bounds**: For `DocumentParagraph` elements with inline objects, the renderer returns layout bounds per callback ID via `inlineObjectBounds`.

**The retry mechanism**: If any min size changed, `hostedController->RequestRefresh()` schedules another rendering frame. The layout engine will use the new min sizes, potentially changing element positions, requiring another render. This converges quickly because min sizes stabilize.

**Font height caching**: `TryFetchMinSizeFromCache()` in `GuiSolidLabelElementRenderer` checks whether the needed font height is already in `renderTarget->fontHeights`. If so, it uses the cached value and skips the measurement request. This avoids redundant round-trips for fonts already measured.

### Rendering Boundary Tracking

As the composition tree is traversed, clipper operations emit boundary messages:
- `AfterPushedClipper()`: inspects the composition generating the clip for hit test results and cursor types. If either is set and differs from the current stack top, sends `RequestRendererBeginBoundary` with the composition's `remoteId`, bounds, and clipped area.
- `AfterPoppedClipper()`: sends `RequestRendererEndBoundary` for the matching boundary.

This allows the renderer side to know which composition each pixel belongs to for hit testing and cursor display, without needing the full composition tree.

## GuiRemoteGraphicsParagraph (IGuiGraphicsParagraph)

`GuiRemoteGraphicsParagraph` is the remote implementation of `IGuiGraphicsParagraph`, handling rich text layout for document elements. It is the most protocol-intensive part of the rendering system because text layout is inherently complex and cannot be done core-side.

### Why Paragraphs Are Special

Unlike ordinary elements (borders, backgrounds, labels) that simply send their visual state and let the renderer draw them, paragraphs need bidirectional communication:
- The core side defines the text content, styling, and inline objects
- The renderer side performs text layout (line breaking, glyph positioning)
- The core side needs the layout results (sizes, caret positions, inline object positions) for hit testing, caret navigation, and selection

### Paragraph Lifecycle

Paragraphs are created by `GuiRemoteGraphicsResourceManager::CreateParagraph()`, which constructs a `GuiRemoteGraphicsParagraph`. The paragraph gets its own element ID (shared ID space with element renderers), registered via `RegisterParagraph()` instead of `RegisterRenderer()`. The creation is batched alongside element creates in `pendingParagraphCreations`.

### Run Property System

Paragraph styling uses a run-based system with three layers:
1. **`DocumentTextRunPropertyMap textRuns`**: text-level properties (font, size, color, style) by `CaretRange`
2. **`DocumentInlineObjectRunPropertyMap inlineObjectRuns`**: inline objects (images, controls embedded in text) by `CaretRange`
3. **`DocumentRunPropertyMap stagedRuns` / `committedRuns`**: merged result used for diff computation

`AddTextRun()` handles overlapping ranges by splitting and merging, supporting incremental property changes. `AddInlineObjectRun()` requires exact range matches (inline objects cannot be split). `MergeRuns()` combines both maps, with inline objects taking priority over text runs.

### Synchronization with Renderer Side (EnsureRemoteParagraphSynced)

This is the core synchronization method, called before any query:
1. `EnsureRequestedRenderersCreated()` — ensures the paragraph element exists on the renderer side
2. Merge text and inline object runs into `stagedRuns`
3. `DiffRuns(committedRuns, stagedRuns, desc)` — compute diff between last committed and current state
4. Send `RequestRendererUpdateElement_DocumentParagraph(desc)` with the diff
5. `Submit()` synchronously — the response includes the new `documentSize`
6. Store `cachedSize` from response, swap `stagedRuns` to `committedRuns`

The first sync sends full text content; subsequent syncs send only run property diffs. This is efficient for scenarios like syntax highlighting where only styling changes.

### Layout Queries (Delegated to Renderer Side)

All layout-dependent operations require synchronous round-trips:
- **`GetSize()`**: returns `cachedSize` from the last sync
- **`GetCaret(comparingCaret, position, preferFrontSide)`**: sends `RequestDocumentParagraph_GetCaret` — the renderer performs text layout-aware caret navigation
- **`GetCaretBounds(caret, frontSide)`**: uses `GetCaretBoundsInternal()` which lazily fetches ALL caret bounds (front and back arrays) in one request via `RequestDocumentParagraph_GetCaretBounds` and caches them in `cachedCaretBounds`. Subsequent calls use the cache.
- **`GetCaretFromPoint(point)`**: iterates all caret positions locally using cached bounds from `GetCaretBoundsInternal()`, finding the nearest by Manhattan distance — no additional remote call if bounds are cached
- **`GetInlineObjectFromPoint(point)`**: sends `RequestDocumentParagraph_GetInlineObjectFromPoint` for hit-test, then looks up `inlineObjectProperties` locally
- **`GetNearestCaretFromTextPos(textPos, frontSide)`**: sends `RequestDocumentParagraph_GetNearestCaretFromTextPos`
- **`IsValidCaret(caret)`**: sends `RequestDocumentParagraph_IsValidCaret`
- **`IsValidTextPos(textPos)`**: purely local — just checks bounds against `text.Length()`

### Caret Display

- `EnableCaret()` sends `RequestDocumentParagraph_OpenCaret` with position, color, front/back side
- `DisableCaret()` sends `RequestDocumentParagraph_CloseCaret`
- `BlinkCaret()` returns false — blinking is handled renderer-side

### Paragraph Rendering

`GuiRemoteGraphicsParagraph::Render(bounds)`:
1. `EnsureRemoteParagraphSynced()` — ensure current state sent to renderer
2. For each inline object with cached bounds, call `callback->OnRenderInlineObject()` — if the inline object's size changed, update `inlineObjectRuns` and mark dirty
3. Send `RequestRendererRenderElement` — same as ordinary elements

### Dirty Tracking

`MarkParagraphDirty(invalidateSize, invalidateCaretBoundsCache)` sets:
- `needUpdate = true` always
- `cachedSize = {0,0}` if `invalidateSize` is true (forcing re-measurement)
- `needUpdateCaretBoundsCache = true` if `invalidateCaretBoundsCache` is true (forcing caret bounds refetch)

Property setters (`SetFont`, `SetSize`, `SetStyle`, `SetColor`, `SetBackgroundColor`, `SetInlineObject`, `ResetInlineObject`) and layout properties (`SetWrapLine`, `SetMaxWidth`, `SetParagraphAlignment`) all call `MarkParagraphDirty`. Size-affecting changes always set both flags; color-only changes skip size invalidation.

## DOM Diff Layer

### Why DOM Diff Exists

Without DOM diff, each frame sends per-element rendering commands (`RenderElement`, `BeginBoundary`, `EndBoundary`). For a typical GacUI application with hundreds of elements, most of which don't move between frames, this is wasteful. The DOM diff layer converts the per-frame command stream into a tree structure and diffs it against the previous frame, sending only changes.

### How It Works

`GuiRemoteProtocolDomDiffConverter` wraps `IGuiRemoteProtocol`:
- During rendering, `RenderingDomBuilder` builds a `RenderingDom` tree from the intercepted boundary and element commands
- On `RequestRendererEndRendering`:
  - First frame: sends `RequestRendererRenderDom` with the full tree
  - Subsequent frames: `DiffDom(lastDom, lastDomIndex, newDom, newDomIndex, diffs)` computes structural diffs, sends `RequestRendererRenderDomDiff`
- `lastDom` is stored for next frame; `OnControllerConnect` clears it to force a full DOM send on reconnection

DOM node IDs encode their type: element IDs use `(elementId << 2) + 0`, hit test compositions use `(compositionId << 2) + 2`, with parent variants at `+1` and `+3`.

## Protocol Combinator Layer

The combinator pattern allows composing protocol transformations as a pipeline. `GuiRemoteProtocolCombinator<TEvents>` wraps an `IGuiRemoteProtocol*` and interposes a `TEvents` event combinator. This creates a chain where messages flow core → combinator → target protocol, and events flow target protocol → event combinator → core.

Passing-through variants (`GuiRemoteProtocolCombinator_PassingThrough`) forward everything unchanged, allowing subclasses to selectively override just the messages they care about. The filter and DOM diff layers both use this pattern.

## Protocol Filter Layer

`GuiRemoteProtocolFilter` optimizes traffic by dropping redundant messages/events:
- `[@DropRepeat]`: drop messages/events with identical arguments to the previous send
- `[@DropConsecutive]`: drop events with identical arguments to the immediately preceding event

The filter queues messages internally and applies drop logic in `ProcessRequests()` before `Submit()`.

## Channel Layer

For real remote deployment, `IGuiRemoteProtocolChannel<T>` provides a bidirectional channel abstraction. The typical stack:
1. `GuiRemoteProtocolFromJsonChannel` adapts a JSON channel to `IGuiRemoteProtocol`
2. `GuiRemoteProtocolAsyncChannelSerializer<Ptr<JsonObject>>` runs channel IO on a separate thread

The async serializer's key design: the UI thread batches messages and blocks on `Submit()` until the channel thread completes the round-trip. Events from the channel thread are queued and processed on the UI thread during `ProcessRemoteEvents()`. Connection/disconnection is tracked via a `connectionCounter` to handle race conditions.

## Image Service

`GuiRemoteGraphicsImageService` creates `GuiRemoteGraphicsImage` objects with core-side binary copies. Image metadata (dimensions, format) is lazily fetched from the renderer side via `ImageCreated` messages. On reconnection, all existing images are re-registered.

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

## QUESTION

I would like you to improve the insight, the first thing is to remove pure code translation to natural language. They are not necessary because they can be just read from the source code directly. By saying "insight" I would like to see more why/what/how/summarization than pure translation. And pay more attention to the rendering part, the rendering part is complex, how elements update get pushed to the client side, how measuremeng happens, how IGuiGraphicsParagraph is processed. They should take a significant part in the document

### ANSWER

The `# INSIGHT` has been rewritten with the following improvements:

**Removed code translations**: Eliminated sections that were essentially restating what functions do line-by-line (e.g., the old `INativeResourceService Implementation`, `INativeInputService Implementation`, `INativeScreenService Implementation`, `INativeWindowService Implementation` subsections that just listed what each method returns). Also removed the exhaustive event dispatch listing from `GuiRemoteEvents` and the member-by-member listing of `GuiRemoteController`.

**Added insight-level content**:
- "Why Remote Protocol Mode Exists" — explains the motivation (platform independence, testability, reconnection support)
- "Why Rendering Is Complex" — explains the fundamental challenge of deferred rendering with measurement feedback
- "Why DOM Diff Exists" — explains the performance motivation
- "Why Paragraphs Are Special" — explains why `GuiRemoteGraphicsParagraph` needs bidirectional communication unlike simple elements

**Expanded rendering section** (now the largest part of the document):
- **Element Lifecycle**: how elements are created (ID allocation on render target assignment), batched for notification (`EnsureRequestedRenderersCreated`), and destroyed
- **Frame Rendering Flow**: the complete `StartRenderingOnNativeWindow` → composition traversal → `StopRenderingOnNativeWindow` sequence, explaining the two modes (normal incremental vs reconnect full update)
- **Element Update Mechanism**: how dirty tracking works (`updated`/`renderTargetChanged` flags, `OnElementStateChanged`), and how `SendUpdateElementMessages` does diff-based serialization (e.g., `GuiSolidLabelElementRenderer` only sends font/text when they changed)
- **Measurement Feedback Loop**: the four categories of feedback (font heights, min sizes, image metadata, inline object bounds) and how the retry mechanism (`RequestRefresh`) drives convergence
- **Font Height Caching**: how `TryFetchMinSizeFromCache` uses the shared `fontHeights` cache to avoid redundant measurements

**Expanded paragraph section** (now a major section):
- **Run Property System**: the three-layer run map architecture (`textRuns`, `inlineObjectRuns`, merged `committedRuns`/`stagedRuns`) and how `AddTextRun` handles range splitting/merging
- **Synchronization**: how `EnsureRemoteParagraphSynced` computes diffs and sends incremental updates (first sync sends full text, subsequent syncs send only run property changes)
- **Layout Queries**: how each query delegates to the renderer side, with emphasis on the caret bounds caching strategy (one-shot fetch of all bounds, then local iteration in `GetCaretFromPoint`)
- **Dirty Tracking**: how `MarkParagraphDirty` controls what gets invalidated (size vs caret bounds cache)
- **Inline Object Interaction**: how `Render()` processes inline object bounds and updates sizes via callback

# DRAFT

## DRAFT REQUEST

## IMPROVEMENTS

## (API|DESIGN) EXPLANATION

## DOCUMENT
