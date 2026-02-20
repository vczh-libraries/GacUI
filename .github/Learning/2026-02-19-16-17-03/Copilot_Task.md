# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.1: Remote Document Paragraph Rendering (Client)

Implement the client-side `IGuiGraphicsParagraph` integration in `GuiRemoteRendererSingle` so that the remote protocol can treat a paragraph as an element, while still behaving correctly under incremental updates (run diffs) from the core application. The result should compile and match the expected runtime behavior (even without end-to-end execution available in this environment).

### what to be done

- Fix document hygiene for this task: remove encoding/mojibake artifacts so the content is readable and searchable.
- Define acceptance criteria tied to repo tooling:
  - Must compile at least `Test/GacUISrc/GacUISrc.sln`.
  - Include `Test/GacUISrc/UnitTest/UnitTest.vcxproj` as a compile sanity check for protocol handler completeness.
- Implement a concrete wrapper design that matches the remote protocol model:
  - Create a client-side `IGuiGraphicsElement` wrapper stored in `availableElements` by element id, so all paragraph protocol handlers can resolve state from id only.
  - The wrapper owns the underlying `IGuiGraphicsParagraph` and paragraph-specific state (cached runs, inline-object bounds, caret state as needed).
  - The wrapper implements `IGuiGraphicsParagraphCallback` (at least `OnRenderInlineObject`) and is passed to paragraph creation, otherwise inline objects cannot be measured/rendered.
- Make thread-affinity explicit:
  - Paragraph creation/update and any render-target interaction must run on the UI thread.
  - If protocol handling can occur off-thread, marshal via `GetCurrentController()->AsyncService()->InvokeInMainThread(...)`.
- Specify the exact paragraph creation API and ordering:
  - Use `GetGuiGraphicsResourceManager()->GetLayoutProvider()->CreateParagraph(text, renderTarget, callback)`.
  - Clarify what happens if updates arrive before a render target exists (defer creation vs `CHECK_ERROR`) and keep the behavior consistent across all handlers.
- Define render-target lifecycle and registration rules:
  - `IGuiGraphicsParagraph` is render-target-bound; on render target changes, invalidate and recreate the paragraph (and re-register as needed).
  - Register/unregister paragraphs on the render target, and use `id == -1` as the single source of truth for "not registered / not available" (avoid parallel boolean flags).
- Expand "apply run diffs" into a concrete, mock-aligned flow in `RequestRendererUpdateElement_DocumentParagraph`:
  - Mirror behavior in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering_Document.cpp` (do not re-invent semantics) for both text runs and inline-object runs.
  - Apply known constraints/learnings: capture inline-object background element id before inserting runs; compare `IGuiGraphicsParagraph::TextStyle` flags against `(TextStyle)0`; `DiffRuns` must not drop old ranges (use `CHECK_ERROR`).
  - Be strict about protocol violations: later updates containing non-null `text` should be treated as an invariant break and fail fast with `CHECK_ERROR` (do not attempt to reinitialize).
- Replace "fill remaining functions" with an explicit checklist of handlers to implement, mapping directly to `IGuiGraphicsParagraph` APIs:
  - `RequestRendererUpdateElement_DocumentParagraph`
  - `RequestDocumentParagraph_GetCaret`
  - `RequestDocumentParagraph_GetCaretBounds`
  - `RequestDocumentParagraph_GetInlineObjectFromPoint`
  - `RequestDocumentParagraph_GetNearestCaretFromTextPos`
  - `RequestDocumentParagraph_IsValidCaret`
  - `RequestDocumentParagraph_OpenCaret`
  - `RequestDocumentParagraph_CloseCaret`
  - Plus any other paragraph queries required by the protocol.
- Make response requirements explicit:
  - Include paragraph size (`GetSize()`) and inline-object bounds (callback id -> `Rect`) so caret/hit-testing/nearest-caret queries can succeed.
  - Clarify whether an explicit layout/render step (e.g. `Render()`) is required before answering measurement-related requests.
- Reassert codebase conventions:
  - Use `vint`, `WString`, `Ptr<>`, and `vl::collections::*` (avoid `std::*`); use tabs; keep header edits comment-free; extract helpers to avoid duplicated conversion/mapping blocks.

### rationale

- The remote protocol treats `IGuiGraphicsParagraph` as if it were an `IGuiGraphicsElement`, but the actual API separation requires a wrapper element on the client to fit into the renderer's element management and rendering pipeline.
- `GuiGraphicsParagraphWrapperElement` demonstrates the basic pattern, but it is static; the remote client needs a dynamic, incremental-update version to support `RequestRendererUpdateElement_DocumentParagraph` and match core-side behavior.
- Handling creation/deletion in the central rendering file keeps ownership and lifetime rules consistent with other remote-rendered objects, reducing the chance of leaks, dangling references, or inconsistent ids.
- Implementing the remaining functions as direct mappings lowers risk and keeps behavior aligned with the underlying `IGuiGraphicsParagraph` contract, while the unit test mock provides a ground truth for expected message-level behavior.

# UPDATES

# INSIGHTS AND REASONING

## Context / Why this exists

- The remote protocol explicitly models a document paragraph as `RendererType::DocumentParagraph` and sends updates via `RendererUpdateElement_DocumentParagraph`.
  - Schema: `remoteprotocol::ElementDesc_DocumentParagraph` and `remoteprotocol::UpdateElement_DocumentParagraphResponse` in `Source\PlatformProviders\Remote\Protocol\Generated\GuiRemoteProtocolSchema.h`.
  - Protocol doc: `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_Document.txt`.
- The client implementation (`Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`) currently has all document-paragraph handlers as `CHECK_FAIL(L"Not implemented.")`.
- The protocol “treats” a paragraph like an element id (it participates in `RendererCreated/RendererDestroyed`), but `elements::IGuiGraphicsParagraph` is not an `elements::IGuiGraphicsElement`.
  - We therefore need a client-side `IGuiGraphicsElement` wrapper that owns an `IGuiGraphicsParagraph` instance.
  - Reference pattern: `GuiGraphicsParagraphWrapperElement` in `Source\UnitTestUtilities\SnapshotViewer\Application\GuiUnitTestSnapshotViewerApp.cpp`.

## Core-side behavior that drives client requirements

- Core-side remote paragraph (`elements::GuiRemoteGraphicsParagraph`) sends incremental updates using `DiffRuns` and expects a synchronous response containing:
  - the measured paragraph size
  - the bounds of inline objects keyed by callback id
  - See `GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`.
- Critical semantic detail: text is only sent on the first update.
  - `EnsureRemoteParagraphSynced()` sets `desc.text = text` only when `committedRuns.Count() == 0`.
  - Subsequent updates rely on run diffs only; the client must not assume it can recreate the paragraph every time.

## Proposed architecture (client-side)

### 1) Add a wrapper element type stored in `availableElements`

- Extend `GuiRemoteRendererSingle::RequestRendererCreated(...)` (in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`) to handle:
  - `remoteprotocol::RendererType::DocumentParagraph`
- For each created paragraph id:
  - create `Ptr<elements::IGuiGraphicsElement>` wrapper
  - store it in `availableElements` just like other renderer-created elements
  - call `wrapper->GetRenderer()->SetRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window))` so paragraph creation can bind to a real render target

#### Wrapper responsibilities

A single wrapper instance corresponds to one protocol paragraph id. Prefer introducing a dedicated wrapper type name to make code review and debugging easier (e.g. `GuiRemoteGraphicsParagraphElement`).

- Implements `elements::IGuiGraphicsElement` so it fits the existing element map.
- Owns the underlying `Ptr<elements::IGuiGraphicsParagraph>` and its render-target-bound lifecycle.
- Implements `elements::IGuiGraphicsParagraphCallback`:
  - `OnRenderInlineObject(vint callbackId, Rect location)` must:
    - record `callbackId -> location` into a `collections::Dictionary<vint, Rect>` so the update response can provide `inlineObjectBounds`.
    - return the inline object size for this `callbackId` (from cached inline-object properties that were applied from `runsDiff`).
  - Add low-cost defensive checks (`CHECK_ERROR`) for unknown / duplicated callback ids to prevent silent protocol-state divergence.
- Maintains minimal paragraph-specific state needed to apply incremental updates and answer queries:
  - `callbackId -> CaretRange` (or equivalent) so `removedInlineObjects` can be translated back to `start/length`.
  - `callbackId -> remoteprotocol::DocumentInlineObjectRunProperty` (or at least `callbackId -> Size`) so `OnRenderInlineObject` can return a stable size.
  - the latest recorded `inlineObjectBounds` (`Dictionary<vint, Rect>`), cleared when the paragraph is recreated.

This wrapper can follow the snapshot viewer structure (single class implementing element + renderer + callback) but must support *incremental* updates (runs diffs) rather than one-time initialization.

### 2) Render-target lifecycle rules

- `elements::IGuiGraphicsParagraph` is render-target bound (`CreateParagraph(text, renderTarget, callback)` takes a render target), so:
  - On `SetRenderTarget(newRt)` change, the wrapper must drop/recreate the underlying paragraph instance.
  - Clear cached inline-object bounds and callbackId state when recreating.
- If an update arrives before a valid render target is available:
  - Prefer deterministic failure: `CHECK_ERROR(renderTarget != nullptr, ...)` (this is expected to be satisfied because `RequestRendererCreated` sets the element render target immediately from `GetGuiGraphicsResourceManager()->GetRenderTarget(window)`).

### 3) Implement `RequestRendererUpdateElement_DocumentParagraph`

Design goal: mirror the unit test mock semantics (`Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering_Document.cpp`) while using the real `IGuiGraphicsParagraph` API.

High-level algorithm:

1. Resolve the wrapper from `availableElements` by `arguments.id`.
2. Paragraph creation:
   - Require a valid render target: `CHECK_ERROR(rt != nullptr, ...)`.
   - If wrapper has no paragraph yet:
     - Require `arguments.text` to exist (first update), then call
       - `GetGuiGraphicsResourceManager()->GetLayoutProvider()->CreateParagraph(text, rt, callback)`
   - If paragraph exists and `arguments.text` is non-null:
     - Treat as protocol violation and fail fast: `CHECK_ERROR(false, ...)` (text is only sent on the first update per `GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()`).
3. Always apply layout settings:
   - `SetWrapLine(arguments.wrapLine)`
   - `SetMaxWidth(arguments.maxWidth)`
   - `SetParagraphAlignment(...)` (convert `remoteprotocol::ElementHorizontalAlignment` to `elements::Alignment`)
4. Apply `arguments.removedInlineObjects` **before** `runsDiff` (match the unit test mock ordering):
   - Use the cached `callbackId -> range` map to locate the range.
   - Call `paragraph->ResetInlineObject(start, length)` and remove callbackId state (including any cached bounds).
5. Apply `arguments.runsDiff`:
   - For each `remoteprotocol::DocumentRun`:
     - `start = run.caretBegin`, `length = run.caretEnd - run.caretBegin`
     - If `DocumentTextRunProperty`:
       - map to `SetFont/SetSize/SetStyle/SetColor/SetBackgroundColor`
       - build `TextStyle` by OR-ing flags (initialize with `(TextStyle)0`, and compare flags against `(TextStyle)0`).
     - If `DocumentInlineObjectRunProperty`:
       - capture `backgroundElementId` and resolve the `IGuiGraphicsElement` **before** updating any run maps (avoid references into containers when mutating them).
       - build `IGuiGraphicsParagraph::InlineObjectProperties` (size, baseline, breakCondition, callbackId, backgroundImage)
       - call `paragraph->SetInlineObject(start, length, properties)`
       - update wrapper caches (`callbackId -> size/property`, `callbackId -> range`) so later removal / hit-test can be answered.
6. Collect response fields (per schema `UpdateElement_DocumentParagraphResponse`):
   - `documentSize = paragraph->GetSize()`
   - `inlineObjectBounds`:
     - Inline-object rectangles are only observable via `IGuiGraphicsParagraphCallback::OnRenderInlineObject`, so do one “measurement render” pass after updates to populate the callbackId->Rect map.
     - Performance note: skip the measurement render when there are no inline objects tracked.
     - Backend safety note: avoid a “guaranteed-empty” clipper that might also skip inline-object callbacks; prefer rendering with bounds that cover the paragraph (e.g. `paragraph->Render(Rect(Point(0,0), documentSize))`) while keeping any clipper consistent with those bounds.
     - Copy the recorded bounds into `inlineObjectBounds` when non-empty.

Rationale for the explicit render:
- `elements::IGuiGraphicsParagraph` does not expose inline-object bounds directly.
- Windows GDI/Uniscribe triggers `OnRenderInlineObject` during render (`GuiGraphicsUniscribe.cpp`), so without calling `Render` the client cannot reliably respond with bounds.

### 4) Implement caret / hit-test query handlers

All query handlers should:
- resolve wrapper by `arguments.id`
- ensure paragraph exists (if not, respond with safe defaults)
- forward to `IGuiGraphicsParagraph` APIs and respond via `events->Respond...`

Mappings:

- `RequestDocumentParagraph_GetCaret`:
  - call `paragraph->GetCaret(arguments.caret, arguments.relativePosition, preferFrontSide)`
  - return `remoteprotocol::GetCaretResponse{ newCaret, preferFrontSide }`
- `RequestDocumentParagraph_GetCaretBounds`:
  - call `paragraph->GetCaretBounds(arguments.caret, arguments.frontSide)`
  - respond with `Rect`
- `RequestDocumentParagraph_GetNearestCaretFromTextPos`:
  - note: schema uses `GetCaretBoundsRequest`; treat `arguments.caret` as `textPos`.
  - call `paragraph->GetNearestCaretFromTextPos(arguments.caret, arguments.frontSide)`
  - respond with `vint`
- `RequestDocumentParagraph_IsValidCaret`:
  - call `paragraph->IsValidCaret(arguments.caret)`
  - respond with `bool`
- `RequestDocumentParagraph_GetInlineObjectFromPoint`:
  - call `paragraph->GetInlineObjectFromPoint(arguments.point, start, length)`
  - if found:
    - return `remoteprotocol::DocumentRun` with `caretBegin=start`, `caretEnd=start+length`, `props=<DocumentInlineObjectRunProperty>` taken from wrapper caches
  - else respond `Nullable<DocumentRun>()`
- `RequestDocumentParagraph_OpenCaret` / `CloseCaret`:
  - forward to `paragraph->OpenCaret(...)` / `paragraph->CloseCaret()` to affect rendering output

### 5) Aligning with the unit test mock

- The mock implementation (`UnitTestRemoteProtocol_Rendering::Impl_RendererUpdateElement_DocumentParagraph`) demonstrates the intended ordering:
  1. apply optional `text`
  2. update wrap/maxWidth/alignment
  3. apply `removedInlineObjects`
  4. apply `runsDiff`
  5. compute layout
  6. respond with `documentSize` and `inlineObjectBounds`

The client implementation should keep the same ordering so that any future unit tests comparing behavior remain meaningful.

## Error handling / robustness choices

- If the wrapper element cannot be found in `availableElements` for a paragraph id:
  - treat as protocol misuse; recommended behavior is `CHECK_ERROR(false, ...)`.
- If `arguments.text` is missing on the first update:
  - fail fast: `CHECK_ERROR(arguments.text, ...)`.
- If `arguments.text` is provided after the paragraph has already been created:
  - treat as protocol violation: `CHECK_ERROR(false, ...)` (do not attempt to “reinitialize”, keep behavior strict and mock-aligned).

# AFFECTED PROJECTS

- Build the solution in folder Test\GacUISrc
  - Run Test Project UnitTest

# !!!FINISHED!!!

