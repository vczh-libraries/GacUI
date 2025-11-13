# !!!TASK!!!
# !!!TASK!!!
# PROBLEM DESCRIPTION
Next

## TASK No.13: Implement `GuiRemoteGraphicsParagraph` class

### description

Implement all methods of `GuiRemoteGraphicsParagraph` class in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`.

**Learning from Task 6 and Task 7**: When working with complex protocol types, start with basic functionality first rather than trying to implement everything comprehensively. The protocol schema may use different types than initially expected (e.g., `Ptr<List<>>` instead of `Array<>`, structs instead of variants). Verify types by reading the protocol schema carefully before implementation.

**Step 1: Add protected members and helper functions**

Add protected members:
- `DocumentRunPropertyMap runs` - stores text formatting and inline objects
- `bool wrapLine = false`
- `vint maxWidth = -1`
- `Alignment paragraphAlignment = Alignment::Left`
- `Size cachedSize = Size(0, 0)`
- `bool needUpdate = true` - tracks if paragraph needs resending to remote
- `vint id = -1` - paragraph ID for protocol communication

Add protected conversion functions:
- `vint NativeTextPosToRemoteTextPos(vint textPos)` - currently returns `textPos` directly
- `vint RemoteTextPosToNativeTextPos(vint textPos)` - currently returns `textPos` directly

**Step 2: Categorize IGuiGraphicsParagraph methods**

Group methods into three categories:

*Category A: Property getters/setters (affect ElementDesc_DocumentParagraph)*:
- `GetWrapLine`, `SetWrapLine`
- `GetMaxWidth`, `SetMaxWidth`
- `GetParagraphAlignment`, `SetParagraphAlignment`

*Category B: Run manipulation (modify runs map)*:
- `SetFont`, `SetSize`, `SetStyle`, `SetColor`, `SetBackgroundColor`
- `SetInlineObject`, `ResetInlineObject`

*Category C: Query operations (send messages)*:
- `GetSize` - sends `RendererUpdateElement_DocumentParagraph`, caches result
- `GetCaret` - sends `DocumentParagraph_GetCaret`
- `GetCaretBounds` - sends `DocumentParagraph_GetCaretBounds`
- `GetCaretFromPoint` - sends `DocumentParagraph_GetCaretBounds` with point
- `GetInlineObjectFromPoint` - sends `DocumentParagraph_GetInlineObjectFromPoint`
- `GetNearestCaretFromTextPos` - sends `DocumentParagraph_GetNearestCaretFromTextPos`
- `IsValidCaret` - sends `DocumentParagraph_IsValidCaret`
- `IsValidTextPos` - handled locally
- `OpenCaret` - sends `DocumentParagraph_OpenCaret`
- `CloseCaret` - sends `DocumentParagraph_CloseCaret`

**Step 3: Implement update strategy**

- Category A and B methods set `needUpdate = true`
- Before any Category C message, if `needUpdate == true`, send `RendererUpdateElement_DocumentParagraph` first with runs converted to `runsDiff` array, then set `needUpdate = false`
- `GetSize()` caches the size returned from `RendererUpdateElement_DocumentParagraph`
- Constructor allocates paragraph ID via `remote->AllocateParagraphId()`
- Destructor sends paragraph destruction message

**Step 4: Implement each method**

- `GetProvider`, `GetRenderTarget`, `IsValidTextPos` - simple implementations
- Property methods - update members and set `needUpdate = true`
- Run manipulation methods - use `AddRun`, `ResetInlineObjectRun` with text position conversion
- Query methods - ensure update is sent, then send corresponding protocol message with proper text position conversion
- `Render` - triggers rendering via render target

### what to be done

1. Add protected members and helper conversion functions to the class
2. Categorize all IGuiGraphicsParagraph methods
3. Implement the lazy update strategy with `needUpdate` flag
4. Implement all Category A methods (property getters/setters)
5. Implement all Category B methods (run manipulation)
6. Implement all Category C methods (query operations)
7. Ensure proper lifecycle management (constructor/destructor)

### how to test it

Testing will be covered in subsequent tasks (Task 12-14) with GacUI test cases. This task focuses on implementation.

### file locations

Implementation: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

# UPDATES

# INSIGHTS AND REASONING

## Understanding the current renderer expectations
- The existing desktop paragraph pipeline applies fonts, colors, styles, and inline objects by calling `IGuiGraphicsParagraph` repeatedly during `SetPropertiesVisitor::SetProperty` (`Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp:454`). Each leaf run comes through helpers such as `ApplyStyle` / `ApplyColor` and `Visit(DocumentImageRun*)` which invoke `SetFont`, `SetSize`, `SetStyle`, `SetColor`, `SetBackgroundColor`, and `SetInlineObject` on the paragraph (`Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp:56`, `Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp:142`, `Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp:202`). The remote paragraph must accept the same sequence without assuming higher-level batching.
- Remote run utilities already consolidate formatting and inline objects: `MergeRuns` combines text overrides with inline objects into a single `DocumentRunPropertyMap`, and `DiffRuns` produces deltas plus created/removed inline object callback IDs (`Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp:312`, `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp:419`). The new paragraph should reuse these helpers so we stay consistent with the unit tests from Task 1–12.

## Proposed members and helper structures
- Keep the task-specified members (`wrapLine`, `maxWidth`, `paragraphAlignment`, `cachedSize`, `needUpdate`, `id`) and extend them with:
  - `DocumentTextRunPropertyMap textRuns` and `DocumentInlineObjectRunPropertyMap inlineObjectRuns` holding staged overrides/inlines.
  - `DocumentRunPropertyMap currentRuns` representing the last merged state and `DocumentRunPropertyMap committedRuns` representing the last state acknowledged by the remote side (needed for `DiffRuns`).
  - `bool textDirty` to track whether the main `text` string must be re-sent.
  - A lightweight `struct InlineObjectState` (cached range, `InlineObjectProperties`, remote `backgroundElementId`, `callbackId`) stored in `Dictionary<CaretRange, InlineObjectState>` plus an index by `callbackId`. This allows us to return the original `InlineObjectProperties` in `GetInlineObjectFromPoint` and to route render callbacks later.
- Add small helpers:
  - `EnsureParagraphRegistered()` to lazily allocate an element id via `renderTarget->AllocateNewElementId()` (`Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp:323`) and queue a `RendererCreated` message with `RendererType::DocumentParagraph` (`Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp:83`).
  - `EnsureRemoteParagraphSynced()` performing merge/diff, building `ElementDesc_DocumentParagraph`, sending `RequestRendererUpdateElement_DocumentParagraph`, and capturing the returned size.
  - Conversion helpers for text positions (identity today but routed through `NativeTextPosToRemoteTextPos` / `RemoteTextPosToNativeTextPos`), alignment mapping (`Alignment` → `ElementHorizontalAlignment`), and color/style packing.

## Lifecycle and registration
- Constructor: capture parameters, stash the `callback`, allocate an element id, register the paragraph id with `GuiRemoteGraphicsResourceManager` so the resource manager can find us when `DocumentParagraph_RenderInlineObject` arrives (`Source/PlatformProviders/Remote/GuiRemoteGraphics.h:115`). Initial state should mark `needUpdate = true` and `textDirty = true` so the first query pushes all data.
- Destructor: notify the resource manager and emit `RendererDestroyed` for our id to release remote resources. We should guard against already disconnected controllers by checking the `Submit` return flag.
- Resource manager: store a `Dictionary<vint, GuiRemoteGraphicsParagraph*>` so `OnDocumentParagraph_RenderInlineObject` can forward events in Task 14. Registration/deregistration happens alongside creation/destruction.

## Run and property updates
- Category A setters simply update the cached value, flip `needUpdate`, and in some cases reset `cachedSize` (e.g., changing wrap/max width invalidates the cached measurement). `GetWrapLine`, `GetMaxWidth`, and `GetParagraphAlignment` just return the cached values.
- Category B uses the new maps:
  - For text formatting calls, build a `DocumentTextRunPropertyOverrides` with only the requested fields populated and call `AddTextRun` on `textRuns`. We ignore zero-length requests (return true) to match desktop behavior.
  - For inline objects, translate `InlineObjectProperties` to `DocumentInlineObjectRunProperty` (`Source/PlatformProviders/Remote/Protocol/Generated/GuiRemoteProtocolSchema.h:344`). We must obtain `backgroundElementId` by dynamic_casting the background element’s renderer to `elements_remoteprotocol::IGuiRemoteProtocolElementRender` and pulling the remote id; if the renderer is attached to a different render target we call `SetRenderTarget(renderTarget)` first so it gets registered before we reference the id. `AddInlineObjectRun` returns false on overlap, which we propagate.
  - `ResetInlineObject` uses `ResetInlineObjectRun` and updates the inline-state dictionaries accordingly.
  - Every mutation sets `needUpdate = true` and invalidates `cachedSize`.

## Lazy synchronization with the remote protocol
- `EnsureRemoteParagraphSynced()` executes when `needUpdate` or `textDirty` is true:
  1. Run `MergeRuns(textRuns, inlineObjectRuns, currentRuns)`; propagate any `false` return as a `CHECK_FAIL` because it indicates inconsistent override usage (unit tests already enforce the required invariants).
  2. Build `remoteprotocol::ElementDesc_DocumentParagraph`:
     - Always fill `wrapLine`, `maxWidth`, and mapped alignment.
     - Set `text` only when `textDirty` is true; otherwise leave it empty so the remote side keeps the previous string.
     - Call `DiffRuns(committedRuns, currentRuns, desc)` to populate `runsDiff`, `createdInlineObjects`, and `removedInlineObjects`.
  3. Send `RequestRendererUpdateElement_DocumentParagraph(id, desc)` (`Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering.cpp:387`), submit, and retrieve the resulting size. Convert the remote size through `remote->remoteWindow.Convert()` if required to stay in host coordinates.
  4. Set `cachedSize`, copy `currentRuns` into `committedRuns`, and clear the dirty flags.
- `GetSize()` runs the synchronization routine the first time (or after any change) and returns `cachedSize` afterwards without hitting the network.

## Query and rendering operations
- Before issuing any protocol request, ensure the paragraph is registered and synchronized (so `runsDiff` and text are current).
- Requests that expect responses follow the `Request -> Submit -> Retrieve` flow already used elsewhere (`Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp:110`). Examples:
  - `GetCaret`: send `RequestDocumentParagraph_GetCaret(id, { caret, position })`, submit, retrieve response, convert the caret index back via `RemoteTextPosToNativeTextPos`.
  - `GetCaretBounds`/`GetCaretFromPoint`/`GetNearestCaretFromTextPos`: similar, wrapping the point or caret in the generated request structs.
  - `GetInlineObjectFromPoint`: remote returns an optional `DocumentRun`; translate the run range into `InlineObjectState` via the dictionaries and return the stored `InlineObjectProperties`.
  - `IsValidCaret`: delegate entirely to `DocumentParagraph_IsValidCaret`; if the controller disconnects mid-flight we fail gracefully by returning `false`.
  - `IsValidTextPos`: perform a local clamp check against `text.Length()` because the protocol lacks a dedicated text-position validator; optional remote verification can reuse `IsValidCaret` if future conversions diverge.
- `OpenCaret` and `CloseCaret` are fire-and-forget messages; we still call `Submit` to flush them and return `false` if the connection dropped.
- `Render(Rect bounds)` mimics `GuiRemoteProtocolElementRenderer::Render`: pull the clipped area from the render target and send `RequestRendererRenderElement({ id, bounds, clipper })`, updating the paragraph’s cached rendering batch id so repeated renders within one frame can detect whether the remote side changed min-size data.

## Inline object management
- `InlineObjectState` preserves the original `InlineObjectProperties`, the range, and remote identifiers. We update this state whenever we add or remove inline objects so later lookups and callbacks are O(log n).
- When a background image is present, we ensure its renderer is remote-aware and registered (call `RegisterRenderer` through the render target if needed) so the `backgroundElementId` stays valid for reuse across diffs.
- We record `callbackId` in the state map; Task 14 can then locate the right paragraph and invoke `callback->OnRenderInlineObject` by id without scanning all paragraphs.

## Error handling and disconnection
- Every protocol round-trip checks the `Submit` flag; on disconnection we stop short (return neutral values) instead of dereferencing missing responses.
- Protocol helpers guard against malformed input by using `CHECK_ERROR`/`CHECK_FAIL` where the existing run utilities already validate assumptions (e.g., `DiffRuns` verifying that new runs fully cover the old ones).

## Dependencies and follow-up work
- `GuiRemoteGraphicsResourceManager` needs a paragraph map (id → paragraph) and helper methods so `OnDocumentParagraph_RenderInlineObject` can locate the target paragraph and forward the request; Task 14 will consume it.
- The document renderer that owns the paragraph already triggers creation/destruction of inline background elements, but we should document that inline background renderers must be created through the same resource manager so their remote ids match the ones we capture.
- Future tasks can extend the position conversion helpers once remote text layout diverges (Task guidance already calls out that they currently return the argument directly).

# !!!FINISHED!!!
