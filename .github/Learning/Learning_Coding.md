# !!!LEARNING!!!

# Orders

- `vl::collections::ObservableList<T>` element access and replacement [2]
- Extract helpers to remove duplicated conversion blocks [2]
- Keep remote JSON channel code pure data processing [1]
- Renderer channel dispatch belongs in async renderer layer [1]
- Cache renderer packages until main-thread invoker exists [1]
- Deliver fatal remote-channel errors before transport shutdown [1]
- `TreeViewItemBindableRootProvider::UpdateBindingProperties` is root-scoped [1]
- Validate `tree::NodeItemProvider::RequestNode` indices [1]
- `tree::NodeItemProvider::CalculateNodeVisibilityIndex` returns `-1` for invisible nodes [1]
- Use `CHECK_ERROR` + `ERROR_MESSAGE_PREFIX` for invalid `NodeItemProvider` inputs [1]
- Use full namespace `ERROR_MESSAGE_PREFIX` in remote renderer `CHECK_ERROR` messages [1]
- Prefer `TreeViewItemRootProvider::GetTreeViewData` over `GetData().Cast<TreeViewItem>()` [1]
- Keep header changes comment-free [1]
- Place helpers in the primary-responsibility namespace [1]
- `DiffRuns` must not drop old ranges (use `CHECK_ERROR`) [1]
- Compare `IGuiGraphicsParagraph::TextStyle` flags against `(TextStyle)0` [1]
- `GuiDocumentCommonInterface::ProcessKey` ignores Enter in `GuiDocumentParagraphMode::Singleline` [1]
- `GuiDocumentCommonInterface::ProcessKey` Enter/Ctrl+Enter depends on `GuiDocumentParagraphMode` [1]
- `FakeClipboardWriter` initializes a fresh `FakeClipboardReader` for `WriteClipboard()` [1]
- Fake file dialogs: use `FakeDialogServiceBase::ShowMessageBox` for prompts [1]
- `<DocumentTextBox/>` is an XML virtual type backed by `GuiDocumentLabel` [1]
- Use `id == -1` as paragraph registration state [1]
- Register/unregister paragraphs on the render target [1]
- Capture inline-object background element id before inserting runs [1]
- Ensure protocol element descriptors carry `id` fields [1]
- Make sync helpers return `bool` and early-out [1]
- In document protocol handlers, element id is in arguments/arguments.id [1]
- Keep trivial helpers header-only when appropriate [1]
- Add include guards to shared headers (macros, not `#pragma once`) [1]
- Remote document paragraphs cache full run state across render-target recreation [1]
- DocumentParagraph handlers use explicit no-timeout main-thread invocation [1]
- Iterate document run maps with `Keys()`/`Values()` loops [1]
- Remote renderer partial DOM diffs preserve element nodes [1]
- `GuiRemoteRendererSingle::RequestRendererEndRendering` refreshes every completed frame [1]
- Document paragraph layouts are keyed by valid caret positions [1]
- `text.Length()` is a caret but not a drawable layout entry [1]
- `Impl_DocumentParagraph_GetNearestCaretFromTextPos` honors `frontSide` [1]
- Null-check graphics resource manager before paragraph creation [1]
- Encapsulate remote inline-object run properties behind query helpers [1]

# Refinements

## `TreeViewItemBindableRootProvider::UpdateBindingProperties` is root-scoped

`TreeViewItemBindableRootProvider::UpdateBindingProperties(true)` only unprepares and re-prepares the root node's immediate children. It does not recursively process prepared descendants (grandchildren and deeper), so tests and features should not expect descendant callbacks from this API.

## `vl::collections::ObservableList<T>` element access and replacement

If a field is typed as `ObservableList<Ptr<T>>`, indexing returns `Ptr<T>` (by reference) directly; do not use `UnboxValue` (which is for `Value`) on `children[i]`.

`ObservableList<T>` does not support copy assignment for "replace whole list" scenarios; replace list contents with `Clear()` + `Add()` (or equivalent per-element operations) instead of `operator=`.

## Validate `tree::NodeItemProvider::RequestNode` indices

For index-based APIs that return a pointer-like value (e.g. `tree::NodeItemProvider::RequestNode(vint index)`), validate `index < 0 || index >= Count()` and return `nullptr` early. This avoids accidental root-node leaks / off-by-one bugs and makes boundary behavior explicit for callers and tests.

## `tree::NodeItemProvider::CalculateNodeVisibilityIndex` returns `-1` for invisible nodes

`tree::NodeItemProvider::CalculateNodeVisibilityIndex(INodeProvider* node)` normalizes all “not visible” results to `-1` as part of its public contract, even if the internal implementation uses other negative sentinel values. Tests should assert `-1` for any node in a collapsed subtree (or otherwise invisible), not internal sentinel values.

## Use `CHECK_ERROR` + `ERROR_MESSAGE_PREFIX` for invalid `NodeItemProvider` inputs

For `vl::presentation::controls::tree::NodeItemProvider`, treat out-of-range indices and foreign nodes as programming errors and fail fast using `CHECK_ERROR`, following the `ERROR_MESSAGE_PREFIX` convention (full namespace + method signature + trailing `#`) for consistent, actionable diagnostics.

Differentiate invalid inputs from normal sentinel-return cases: invisibility / “not visible” uses `-1` in `CalculateNodeVisibilityIndex`, while misuse (e.g. foreign node pointers, invalid item indices in data retrieval APIs) should trigger `CHECK_ERROR`.

## Use full namespace `ERROR_MESSAGE_PREFIX` in remote renderer `CHECK_ERROR` messages

For `vl::presentation::remote_renderer::GuiRemoteRendererSingle` and related remote-renderer helpers, `CHECK_ERROR` messages should use the complete namespace + type + method signature + trailing `#` form (for example `vl::presentation::remote_renderer::GuiRemoteRendererSingle::Method#...`). Avoid abbreviated prefixes; full prefixes keep diagnostics searchable and consistent with tests that validate error text.

## Prefer `TreeViewItemRootProvider::GetTreeViewData` over `GetData().Cast<TreeViewItem>()`

When working with nodes whose data is known to be `TreeViewItem`, use `TreeViewItemRootProvider::GetTreeViewData(node)` to obtain `Ptr<TreeViewItem>` instead of calling `node->GetData().Cast<TreeViewItem>()`. This is the intended API, keeps code cleaner, and avoids repeating casts.

## Keep header changes comment-free

Avoid adding explanatory comments in header-file code changes unless they are required for correctness. Prefer keeping headers clean and moving rationale to `.cpp` files or task documentation when needed.

## Place helpers in the primary-responsibility namespace

When implementing helpers that bridge subsystems, place them in the namespace of their primary responsibility / usage site, and use explicit namespace prefixes for types from other subsystems to keep boundaries clear.

## `DiffRuns` must not drop old ranges (use `CHECK_ERROR`)

`vl::presentation::elements::DiffRuns(...)` should treat “an old run range becomes uncovered by `newRuns`” as an error (it means part of the paragraph state was updated to nothing). Detect this and report via `CHECK_ERROR` (not `CHECK_FAIL`), following the `ERROR_MESSAGE_PREFIX` convention with full namespace + signature + trailing `#` so tests can validate it with `TEST_ERROR(...)`.

## Compare `IGuiGraphicsParagraph::TextStyle` flags against `(TextStyle)0`

When checking `IGuiGraphicsParagraph::TextStyle` bitflags, use a zero-value cast for comparisons (e.g. `(style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0`). Don’t assume a `TextStyle::None` enumerator exists.

## Extract helpers to remove duplicated conversion blocks

When the same conversion logic is repeated in multiple branches (e.g. converting override structs to full protocol properties inside `MergeRuns`), extract a small helper function and replace duplicated blocks with calls. This improves maintainability and reduces bug surface during later semantic changes.

Use the same judgment for repeated dictionary update patterns such as `Contains()`/`Set()` vs `Add()` blocks for inline-object bounds, properties, or ranges. Extract a tiny helper when the repeated code carries behavior; keep it inline only when the helper would hide more than it clarifies.

## Keep remote JSON channel code pure data processing

`GuiRemoteProtocol_Channel_Json.(h|cpp)` should stay focused on JSON/protocol package conversion and dispatch. Do not put GacUI application scheduling, renderer-thread decisions, or `GetApplication()` usage in this layer; keep those responsibilities in the async/channel integration layer or in `GuiMain`.

## Renderer channel dispatch belongs in async renderer layer

Renderer-side remote protocol packages must reach `GuiRemoteRendererSingle` on the GacUI UI thread. Put this in `GuiRemoteProtocol_Channel_AsyncRenderer.(h|cpp)` with an invoker injected from renderer `GuiMain`, rather than making JSON conversion code know about GacUI threading.

## Cache renderer packages until main-thread invoker exists

During renderer startup, network/channel packages can arrive before `GetApplication()` and the main-window registration are ready. The renderer async channel should cache all incoming packages until `SetInvokeInMainThread` installs the invoker, then drain them through `InvokeInMainThread` so startup commands are not lost.

## Deliver fatal remote-channel errors before transport shutdown

When `RemotingTest_Core` broadcasts a fatal error, deliver the `!Error` package through `IChannelServer::BroadcastError` and give HTTP/named-pipe transports enough shutdown ordering to let clients consume it. Browser and renderer clients should observe the fatal package rather than only seeing the transport close.

## `GuiDocumentCommonInterface::ProcessKey` ignores Enter in `GuiDocumentParagraphMode::Singleline`

In `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey`, treat `VKEY::KEY_RETURN` (including Ctrl+Enter) as a no-op when `config.paragraphMode == GuiDocumentParagraphMode::Singleline`.

Enter must not mutate text and must not delete an existing selection in singleline mode.

## `GuiDocumentCommonInterface::ProcessKey` Enter/Ctrl+Enter depends on `GuiDocumentParagraphMode`

In `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey` (`VKEY::KEY_RETURN`):

- `GuiDocumentParagraphMode::Multiline`: `ENTER` and `CTRL+ENTER` should do the same thing (create a new line, represented structurally rather than embedding `L"\r\n"` inside a paragraph).
- `GuiDocumentParagraphMode::Paragraph`: keep the distinction: `ENTER` creates a new paragraph; `CTRL+ENTER` inserts an in-paragraph line break (`L"\r\n"`). Don’t gate this on `pasteAsPlainText` for plain-text paragraph-mode controls.

## `FakeClipboardWriter` initializes a fresh `FakeClipboardReader` for `WriteClipboard()`

In `Source/Utilities/FakeServices/GuiFakeClipboardService.cpp`, ensure `FakeClipboardService::WriteClipboard()` returns a writer that can deterministically populate (and clear) clipboard state for unit tests.

`FakeClipboardWriter` should initialize its staging `reader` to a fresh `FakeClipboardReader` in the constructor, so `SetText(...)` + `Submit()` works and `Submit()` without setting text can represent “empty clipboard” (so `ReadClipboard()->ContainsText()` becomes `false`).

## Fake file dialogs: use `FakeDialogServiceBase::ShowMessageBox` for prompts

In fake dialog services, show prompts (e.g. “File(s) not exist”) using the fake dialog service’s GUI-rendered message box API, so UI-driven unit tests can locate and close the prompt window. Avoid calling the controller’s native `DialogService()->ShowMessageBox(...)` from fake view model code, since native message boxes may not appear in `GetApplication()->GetWindows()` or produce rendering changes detectable by the UI-test harness.

If `FakeDialogServiceBase::ShowMessageBox(...)` requires a modal options argument, pass `INativeDialogService::ModalWindow` to preserve modal behavior.

## `<DocumentTextBox/>` is an XML virtual type backed by `GuiDocumentLabel`

`GuiDocumentTextBox` is an XML virtual type registered by the instance loader. At runtime, `<DocumentTextBox/>` creates a `GuiDocumentLabel`-backed control (with a different theme), so C++ code and tests should treat it as `GuiDocumentLabel` when a concrete runtime type is required.

## Use `id == -1` as paragraph registration state

For remote paragraphs, treat `id == -1` as the single source of truth for “not registered / not available”, and avoid maintaining a separate `registered` flag.

## Register/unregister paragraphs on the render target

If only the render target is responsible for creating/destroying remote renderers, keep the paragraph id→paragraph map in the render target and expose `RegisterParagraph`/`UnregisterParagraph` there. This avoids relying on a resource-manager map for paragraph-only ids.

## Capture inline-object background element id before inserting runs

When `SetInlineObject` needs a `backgroundElementId`, obtain it from the remote renderer (`GetID()`) before inserting into the inline-object run map. Don’t attempt to mutate a `Dictionary` value via a `const&` returned from `operator[]`; instead build the full `remoteprotocol::DocumentInlineObjectRunProperty` first, then call `AddInlineObjectRun`, and only after success call `renderer->SetRenderTarget(renderTarget)` if needed.

## Ensure protocol element descriptors carry `id` fields

Remote update requests typically accept a single descriptor struct that includes the element id (e.g. `ElementDesc_*` has an `id` field). If a descriptor lacks `id`, call sites may incorrectly try to pass the id separately. Keep protocol schemas consistent and regenerate the C++ schema when descriptor fields change.

## Make sync helpers return `bool` and early-out

For methods that require a synced remote paragraph state, implement a synchronization helper that returns `bool` so callers can bail out quickly and return conservative defaults when sync or submission fails (e.g. disconnection).

## In document protocol handlers, element id is in arguments/arguments.id

For document/paragraph-aware protocol requests, ensure the element id is carried by the request arguments (either as the argument itself when it’s a primitive, or via an `arguments.id` field when it’s a struct). Don’t introduce ad-hoc extra id parameters at call sites; follow the schema.

## Keep trivial helpers header-only when appropriate

For very small helpers that are simple wrappers (e.g. an IO helper that just loops and emits events), prefer keeping the implementation inline in the header to reduce indirection and make the helper easy to discover and maintain.

## Add include guards to shared headers (macros, not `#pragma once`)

When a test helper is moved into a shared header, ensure the header has macro-based include guards to prevent redefinition errors during compilation. Follow the repo convention of `#ifndef/#define/#endif` guards rather than `#pragma once`.

## Remote document paragraphs cache full run state across render-target recreation

Remote document paragraph protocol updates are incremental. A wrapper element such as `GuiRemoteDocumentParagraphElement` must cache the full text, text-run properties, inline-object runs, merged runs, caret state, and callback-related inline-object state needed to rebuild its `IGuiGraphicsParagraph` when `SetRenderTarget` destroys/recreates the paragraph. Applying `runsDiff` directly to the live paragraph without caching loses styling after DPI/device/render-target changes.

## DocumentParagraph handlers use explicit no-timeout main-thread invocation

In `Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Document.cpp`, document paragraph request handlers should call `InvokeInMainThreadAndWait(window, proc, -1)` when synchronously querying or mutating UI state. Keep the explicit `-1` timeout argument rather than relying on an overload, matching the existing handler style.

## Iterate document run maps with `Keys()`/`Values()` loops

For document paragraph run maps in remote-renderer code, follow the existing `Keys()`/`Values()` indexed-loop pattern instead of switching to structured bindings. These map types expose stable key/value views in the local style, and matching that style avoids accidental API assumptions.

## Remote renderer partial DOM diffs preserve element nodes

`GuiRemoteRendererSingle::RequestRendererRenderDomDiff(...)` can receive partial-frame DOM streams. When pre-filtering diffs, ignore `Deleted` diffs for element nodes and virtual parents (`domId % 4 == 0/1`), convert `Created` to `Modified` when such a node already exists client-side, and keep hit-test nodes (`domId % 4 == 2/3`) deletable. This lets DOM diffs manage structure while `RequestRendererDestroyed` and `availableElements` decide actual renderability, preventing stale "zombie" rendering without dropping omitted-but-still-valid elements.

## `GuiRemoteRendererSingle::RequestRendererEndRendering` refreshes every completed frame

At the end of a completed rendering cycle, `GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)` should set `needRefresh = true` unconditionally so the client repaints even when the DOM diff is empty or only measurement/element updates arrived.

## Document paragraph layouts are keyed by valid caret positions

For document paragraph layout/caret logic, separate text positions from renderable layout entries. Inline objects can span multiple text positions but collapse to one drawable item, so arrays indexed by layout-entry position must not be accessed with `DocumentParagraphLineInfo::{startPos,endPos}` text positions. Store character layouts keyed by valid caret positions and keep any sorted key list synchronized with the layout dictionary.

## `text.Length()` is a caret but not a drawable layout entry

In document paragraph geometry and navigation, `text.Length()` remains a valid caret position for editing/navigation but should not have a drawable layout entry because there is no glyph after it. Geometry APIs need an explicit end-of-text path, typically using the end of the last drawable item.

## `Impl_DocumentParagraph_GetNearestCaretFromTextPos` honors `frontSide`

`Impl_DocumentParagraph_GetNearestCaretFromTextPos` should prefer the nearest caret on the requested side: `frontSide == true` prefers a caret `<= textPos`, while `frontSide == false` prefers a caret `> textPos`. If the preferred side is unavailable, fall back to the other side and handle empty paragraphs explicitly.

## Null-check graphics resource manager before paragraph creation

Before creating a paragraph through `GetGuiGraphicsResourceManager()->GetLayoutProvider()->CreateParagraph()`, check that the resource manager and layout provider are available. Remote paragraph wrapper code can be called during setup/teardown or unusual render-target transitions, so null checks produce clearer behavior than crashes.

## Encapsulate remote inline-object run properties behind query helpers

Remote paragraph query handlers should not directly inspect wrapper internals such as inline-object property dictionaries. Add a small public helper like `TryGetInlineObjectRunProperty(callbackId, outProp)` on the wrapper to preserve encapsulation and keep query behavior stable when cached state changes.
