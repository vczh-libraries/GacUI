# !!!LEARNING!!!

# Orders

- `vl::collections::ObservableList<T>` element access and replacement [2]
- `TreeViewItemBindableRootProvider::UpdateBindingProperties` is root-scoped [1]
- Validate `tree::NodeItemProvider::RequestNode` indices [1]
- `tree::NodeItemProvider::CalculateNodeVisibilityIndex` returns `-1` for invisible nodes [1]
- Use `CHECK_ERROR` + `ERROR_MESSAGE_PREFIX` for invalid `NodeItemProvider` inputs [1]
- Prefer `TreeViewItemRootProvider::GetTreeViewData` over `GetData().Cast<TreeViewItem>()` [1]
- Keep header changes comment-free [1]
- Place helpers in the primary-responsibility namespace [1]
- `DiffRuns` must not drop old ranges (use `CHECK_ERROR`) [1]
- Compare `IGuiGraphicsParagraph::TextStyle` flags against `(TextStyle)0` [1]
- Extract helpers to remove duplicated conversion blocks [1]
- `GuiDocumentCommonInterface::ProcessKey` ignores Enter in `GuiDocumentParagraphMode::Singleline` [1]
- `FakeClipboardWriter` initializes a fresh `FakeClipboardReader` for `WriteClipboard()` [1]
- Use `id == -1` as paragraph registration state [1]
- Register/unregister paragraphs on the render target [1]
- Capture inline-object background element id before inserting runs [1]
- Ensure protocol element descriptors carry `id` fields [1]
- Make sync helpers return `bool` and early-out [1]
- In document protocol handlers, element id is in arguments/arguments.id [1]
- Keep trivial helpers header-only when appropriate [1]
- Add include guards to shared headers (macros, not `#pragma once`) [1]

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

## `GuiDocumentCommonInterface::ProcessKey` ignores Enter in `GuiDocumentParagraphMode::Singleline`

In `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey`, treat `VKEY::KEY_RETURN` (including Ctrl+Enter) as a no-op when `config.paragraphMode == GuiDocumentParagraphMode::Singleline`.

Enter must not mutate text and must not delete an existing selection in singleline mode.

## `FakeClipboardWriter` initializes a fresh `FakeClipboardReader` for `WriteClipboard()`

In `Source/Utilities/FakeServices/GuiFakeClipboardService.cpp`, ensure `FakeClipboardService::WriteClipboard()` returns a writer that can deterministically populate (and clear) clipboard state for unit tests.

`FakeClipboardWriter` should initialize its staging `reader` to a fresh `FakeClipboardReader` in the constructor, so `SetText(...)` + `Submit()` works and `Submit()` without setting text can represent “empty clipboard” (so `ReadClipboard()->ContainsText()` becomes `false`).

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
