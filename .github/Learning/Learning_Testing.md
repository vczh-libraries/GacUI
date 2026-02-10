# !!!LEARNING!!!

# Orders

- Keep test log paths stable during refactors [13]
- Don’t schedule redundant idle frames [7]
- Preserve existing idle-frame titles when requested [6]
- Seed key-behavior tests via `protocol->TypeString` [6]
- Remote protocol frames: actions must change UI; organize frames carefully [8]
- Add new unit test files to `UnitTest.vcxproj` and `.filters` [4]
- Avoid duplicate tests across related categories [3]
- Account for eager child preparation in item-provider tests [2]
- Isolate callbacks per test case (fresh log + callback) [2]
- Prefer comments that name the exercised interface [2]
- Use `TEST_ERROR` for invalid bindings and out-of-range access [2]
- `DetachCallback()` should fire `OnAttached(provider=nullptr)` [2]
- Update `UnitTest.vcxproj.user` `/F:` filters for new files [2]
- `TEST_CATEGORY` / `TEST_CASE` blocks must end with `});` [2]
- Caret navigation tests: type markers to expose caret [2]
- RichText style tests: `EditStyle` + `SummarizeStyle`, set only edited fields [2]
- GuiDocumentLabel paragraphs: use `\\r\\n\\r\\n` between paragraphs in `LoadTextAndClearUndoRedo` [2]
- Use `AssertItems`/`AssertCallbacks` for visible item lists [2]
- Skip callback plumbing when not asserting callbacks [1]
- Print actual vs expected on assertion failure [1]
- Disambiguate protocol types by namespace [1]
- Use `Ptr<List<T>>` APIs for protocol descriptor outputs [1]
- Use `TEST_ERROR(expr)` (not code blocks) [1]
- Filesystem mocks: normalize in `IFileSystemImpl::Initialize`, regex-split paths [1]
- Don’t nest `protocol->OnNextIdleFrame(...)` [1]
- Re-find GUI controls in each frame [1]
- File dialog filters persist across navigation [1]
- Dialog UI tests: close GUI message boxes via `OK` click [1]
- Prefer `TryGet...` helpers when asserting absence [1]
- Multiple selection not enabled: type `a;b` in text box [1]
- Workflow script in XML: `&` concat, `var name : type` [1]
- Don’t accept multiple outcomes in one `TEST_ASSERT` [1]
- File dialog selection text uses quoted items + `;` [1]
- `GuiSaveFileDialog` uses `GetFileName()` (not `GetFileNames()`) [1]
- Save dialog: non-existing file is allowed unless prompting [1]
- Scaffolding-only editor tests: use requested `TEST_CATEGORY` titles [1]
- Undo/redo tests: clear baselines and respect history granularity [1]
- Split singleline-only key cases into a dedicated helper [1]
- Multiline editor assertions: validate `GetDocument()` model, not `GetText()` [1]
- Hyperlink tests: `ViewOnly` interaction + merged event log + model checks [1]
- Avoid searching for CRLF in deterministic tests [1]
- Unit-test renderer `CaretLineLast` should treat CRLF as newline [1]
- Match existing test-file namespace style (avoid extra `using namespace`) [1]
- Focus the control before simulating keyboard input [1]
- Editor smoke tests: deterministic caret placement (`Ctrl+Home`) [1]
- Prefer IOChar-only typing helpers first [1]
- Prefer base-type lookups for reusable control tests [1]
- Avoid over-abstraction in test scaffolds [1]
- With hijacked time, caret-only moves may not trigger UI updates [1]
- Prepare nodes (via `GetChildCount`) before asserting nested callbacks [1]
- Empty item sources and empty `childrenProperty` only trigger `OnAttached` [1]
- Prefer single cohesive smoke tests when setup-heavy [1]

# Refinements

## Account for eager child preparation in item-provider tests

In `TreeViewItemBindableRootProvider` binding scenarios, children are prepared eagerly. Callback parameters like `count`/`newCount` reflect actual prepared child counts, not zero, and expansion is mostly a visual state toggle rather than a "prepare children" trigger. Write expectations accordingly.

## Isolate callbacks per test case (fresh log + callback)

Each test case should create its own callback log and `MockNodeProviderCallback` instance to avoid cross-test state pollution. After setup steps (especially `SetItemSource`), clear the callback log unless the test is explicitly validating setup behavior.

## Prepare nodes (via `GetChildCount`) before asserting nested callbacks

When testing nested `ObservableList` changes (grandchildren and deeper), prepare nodes to the needed depth first (typically by calling `GetChildCount()` on the relevant nodes) so the provider subscribes to list events and callbacks fire on the expected node.

## Empty item sources and empty `childrenProperty` only trigger `OnAttached`

In `TreeViewItemBindableRootProvider` tests, setting an empty item source / empty `childrenProperty` collection should only cause `OnAttached` to fire (with `newCount=0`) and should not produce `OnItemModified` noise. For null/empty children scenarios, assert the provider is safe and that callback counts reflect "nothing to prepare".

## Prefer single cohesive smoke tests when setup-heavy

For infrastructure-style test additions (new file + basic wiring), prefer a single cohesive smoke test case when it shares the same setup and the goal is “prove it compiles / links / runs”. Split into multiple test cases only when it improves diagnostics or isolates independent scenarios.

## Prefer comments that name the exercised interface

When a type implements multiple interfaces (common in item-provider tests), add brief comments that explicitly name which interface is being exercised for each operation, and keep the code direct. Avoid preemptive interface casts unless they are required to access the API.

## Use `AssertItems`/`AssertCallbacks` for visible item lists

When verifying visible items from a `NodeItemProvider` (or similar `list::IItemProvider`), prefer comparing a collected list of item texts against a `const wchar_t*[]` using existing helper(s) like `AssertCallbacks`, to keep tests compact and to get verbose diff output on failures.

This also applies to file dialog tests: read visible row texts via `GuiBindableDataGrid::GetItemProvider()->GetTextValue(i)` and assert the exact sequence, without trying to separately assert “folder vs file” row types.

If the expected list is empty, assert `Count() == 0` directly; otherwise declare a `const wchar_t* expected[]` array and pass it to `AssertItems` (which should delegate to `AssertCallbacks` instead of duplicating comparison logic).

## Skip callback plumbing when not asserting callbacks

If a test case only verifies return values / mapping logic and does not validate callback behavior, omit `callbackLog` and `Mock*Callback` setup entirely. Add callback logs only in tests that assert notifications, and clear logs after setup when the test is focused on post-setup behavior.

## Print actual vs expected on assertion failure

For custom assertions that compare complex data (e.g. run maps), follow the `PrintCallbacks`/`AssertCallbacks` pattern: first detect equality, and only on mismatch print both actual and expected using `TEST_PRINT`, then fail (e.g. `TEST_ASSERT(false)` or `TEST_ASSERT(matches)`). This keeps passing tests quiet while making failures actionable.

## Add new unit test files to `UnitTest.vcxproj` and `.filters`

When adding/removing/renaming a unit test `.cpp`, keep `Test\\GacUISrc\\UnitTest\\UnitTest.vcxproj` and `Test\\GacUISrc\\UnitTest\\UnitTest.vcxproj.filters` in sync. A stale compile item that points to a deleted file can break the build (e.g. `error C1083: Cannot open source file`).

When replacing one compile item with another, preserve any important per-file settings (e.g. additional options like `/bigobj`) that the project relies on.

## Update `UnitTest.vcxproj.user` `/F:` filters for new files

If `Test\\GacUISrc\\UnitTest\\UnitTest.vcxproj.user` uses `/F:TestFile.cpp` filters in `LocalDebuggerCommandArguments`, append new test files there so starting the unit test project under the debugger includes the new file.

When updating `LocalDebuggerCommandArguments`, also append the new full arguments string to `LocalDebuggerCommandArgumentsHistory` (preserve existing history entries and separators).

## Scaffolding-only editor tests: use requested `TEST_CATEGORY` titles

For tasks that only add placeholder editor test files, use short feature-area `TEST_CATEGORY` titles when requested (e.g. `InlineObject`, `Styles`) to keep logs readable and avoid premature categorization.

## Disambiguate protocol types by namespace

In remote protocol tests, avoid ambiguous type names (e.g. `DocumentRun` could refer to `vl::presentation::DocumentRun` or `vl::presentation::remoteprotocol::DocumentRun`). Use fully qualified names (or a single unambiguous `using`) so you are testing the schema types actually used by protocol descriptors.

## Use `Ptr<List<T>>` APIs for protocol descriptor outputs

Many protocol descriptor fields are `Ptr<collections::List<T>>` (not `Array<T>`). Tests should allocate them explicitly and validate via `->Count()`/`->Get(i)`/`->Add(...)` rather than assuming array-style APIs or initializer-list construction.

## Use `TEST_ERROR(expr)` (not code blocks)

`TEST_ERROR` expects an expression (e.g. `TEST_ERROR(DiffRuns(oldRuns, newRuns, desc))`), not a code block. Follow existing examples in the test suite when validating `CHECK_ERROR`/`CHECK_FAIL` behavior.

## Filesystem mocks: normalize in `IFileSystemImpl::Initialize`, regex-split paths

When implementing a test-only `vl::filesystem::IFileSystemImpl` mock:

- Treat `Initialize(WString& fullPath)` as the single normalization point; other `IFileSystemImpl` methods can assume `fullPath` is already normalized when invoked via `vl::filesystem::FilePath`.
- Use `vl::regex::Regex::Split(..., keepEmptyMatch=false, ...)` to split on separators and automatically drop empty components (so `//` behaves like `/`).
- Be careful with `vl::regex` escaping rules in character classes; a verified separator pattern for “`/` or `\\`” is `L"[\\/\\\\]+"` (incorrect escaping can throw `Illegal character set definition`).
- If `GetRelativePathFor(from, to)` is required, implement `..`-based relative paths (e.g. `/a/b/c` -> `/a/d` yields `../../d`) and ensure `Initialize` resolves `.`/`..` components consistently.

## Avoid duplicate tests across related categories

When adding new test categories for a feature that builds on another (e.g. `MergeRuns` nullable semantics vs `AddTextRun` nullable semantics), avoid adding cases that only re-test the other function’s behavior. Prefer scenarios that exercise the new category’s unique contract (e.g. inline-object priority and default application in `MergeRuns`).

## Match existing test-file namespace style (avoid extra `using namespace`)

For control unit tests that include `TestControls.h`, follow the existing file pattern and avoid adding extra `using namespace ...;` lines unless the surrounding tests do so. Rely on the shared test header’s conventions for namespace exposure.

## Don’t schedule redundant idle frames

In GUI protocol tests that use `OnNextIdleFrame`, avoid adding frames that don’t introduce observable changes. Only schedule a follow-up frame when the current callback actually issues an input that could change UI state.

If you want the last logged frame to be easy to inspect, keep the final callback minimal (often just `window->Hide()`) and do it in a dedicated final frame whose title describes what happened in the previous action/assert frame.

If you need to record a value in the frame (e.g. clipboard text after Ctrl+C) and also ensure UI changes, set something observable like the window title (`window->SetText(reader->GetText())`) in the action/assert frame.

Don’t call `window->SetText(...)` just to force a UI update “for no reason”. If a frame would otherwise be a no-op, merge it into the previous/next frame instead of scheduling an extra idle frame.

Similarly, avoid extra UI changes immediately after assertions (e.g. resetting caret position) just to satisfy the “UI changed” rule; instead, restructure frames or merge `window->Hide()` into the last meaningful verification frame.

## Focus the control before simulating keyboard input

Remote IO command helpers only affect the currently focused control. In typing tests, explicitly focus the target control (e.g. `textBox->SetFocused()` or a click) before calling helpers like `TypeString`/`KeyPress`.

## Prefer IOChar-only typing helpers first

For the initial typing simulation helper, emitting sequential `OnIOChar` events (`TypeString`) is sufficient and more deterministic than synthesizing key down/up for every character. Add key-event synthesis later only when a test requires modifier-accurate behavior.

## Keep test log paths stable during refactors

When refactoring an existing test file for reuse, preserve the existing `appName` path segment format for that file unless there is an explicit migration plan. Avoid introducing new `CATEGORY_CASE` path conventions if they’re intended for a different companion file.

For key-behavior tests, prefer stable category/case naming like `.../Key/<Category>_<Case>` and build paths consistently (e.g. using `WString::Unmanaged(L"...")` segments).

`TEST_CATEGORY(...)` names can change without affecting log folders when the log identity string is explicitly built from `controlName` + a hardcoded suffix. You can append postfixes like ` (Singleline)` to category names for clarity as long as `controlName` and the per-case `.../Key/...` suffix remain unchanged.

## Split singleline-only key cases into a dedicated helper

When a shared key-test scaffold starts accumulating singleline-only behavior, avoid a `singleline` boolean flag. Extract singleline-only cases into `RunTextBoxKeyTestCases_Singleline` (or a similarly named helper) and call it only for `GuiSinglelineTextBox`.

For clarity, add a ` (Singleline)` postfix to the categories contributed by the singleline helper (e.g. `Clipboard (Singleline)`), while keeping per-test log identity strings unchanged.

## Prefer base-type lookups for reusable control tests

When planning to reuse the same test steps across multiple controls, locate the target control using an appropriate base type (e.g. `GuiDocumentLabel` instead of `GuiSinglelineTextBox`) so the typed lookup remains strict but does not over-couple tests to one subclass.

## Avoid over-abstraction in test scaffolds

When there isn’t much logic shared between GUI `TEST_CASE`s, prefer inlining the setup (`SetGuiMainProxy` + `StartFast...`) inside each test case rather than extracting additional “runner” helpers. Keep the log path conventions consistent per file (e.g. key tests may use `.../Key/<CATEGORY>_<CASE>` while non-key tests keep `.../<Case>`).

## Preserve existing idle-frame titles when requested

If a test already has established `OnNextIdleFrame` title strings, keep them unchanged during refactors unless explicitly asked to rename them. Treat frame titles as stable “chapter headings” that help compare logs over time.

Follow the KB convention that a frame title describes what happened in the previous frame. Keep titles short and descriptive (e.g. `Init` then `Typing`, `Backspace`, `Delete`).

## With hijacked time, caret-only moves may not trigger UI updates

When `TooltipTimer` (or similar) freezes the clock, caret blinking/animation won’t advance, so pure caret-navigation inputs can produce no “visual” update. In that case, keep navigation-only assertions in a single frame and only add frame boundaries after actions that actually change state (selection changes, typing, deletion, etc.).

## Seed key-behavior tests via `protocol->TypeString`

For editor key-behavior tests, prefer seeding initial text through the input pipeline (`protocol->TypeString`) rather than `SetText(...)`, so subsequent key handling is exercised under the same input-driven code paths. Use a frame boundary between typing and assertions/next key actions (e.g. `Init` focuses + types, next frame performs key input + assertions).

## Use `TEST_ERROR` for invalid bindings and out-of-range access

When a bindable tree is misconfigured (e.g. missing/invalid `childrenProperty`, inconsistent children types), or when tests intentionally access nodes with invalid indices (including expand/collapse paths), validate the failure with `TEST_ERROR(...)` instead of letting the test crash or accidentally pass.

When writing such tests, ensure the corresponding production code uses `CHECK_ERROR` with the `ERROR_MESSAGE_PREFIX` convention (full namespace + method signature + trailing `#`) so failures are actionable and consistent across the codebase.

## `DetachCallback()` should fire `OnAttached(provider=nullptr)`

Detaching a callback is observable: `DetachCallback()` should invoke `OnAttached(provider=nullptr)` to signal cleanup. When the test is validating callback logs around detachment, clear logs after the detachment step (not before), and ensure subsequent operations do not reach the detached callback.

## Remote protocol frames: actions must change UI; organize frames carefully

In `UnitTestRemoteProtocol::OnNextIdleFrame` GUI tests, every idle-frame callback must introduce an observable UI change, or the test harness can fail with “The last frame didn't trigger UI updating”.

Prefer keeping frames minimal and meaningful:

- If a frame performs a UI-changing action (typing, paste, Enter, deletion, etc.), it is fine (and often better) to do action + verification in the same frame.
- Avoid scheduling “verification-only” frames that don’t change UI; either merge them into an action frame or ensure the verification frame also changes UI for a real reason (e.g. logging a diagnostic value).

Name frames so the title summarizes what the previous frame did. Create a new frame whenever you perform a visible update to the control so logs line up with actions, and avoid standalone “Verify …” frames that don’t change UI (they can crash the test framework).

## Don’t nest `protocol->OnNextIdleFrame(...)`

`protocol->OnNextIdleFrame(...)` registrations must be flat; don’t call `OnNextIdleFrame` from inside another frame callback. Schedule all frames at the same scope, matching existing dialog/editor tests.

## Re-find GUI controls in each frame

Idle-frame lambdas execute later, so don’t cache GUI control pointers outside the current frame or assume captured pointers stay valid. In each `OnNextIdleFrame(...)` body, re-find the dialog window and controls (data grid, text box, combo box, etc.) by name.

## File dialog filters persist across navigation

In file dialog navigation tests, the selected filter (e.g. `Text Files (*.txt)`) persists across folder navigation. A folder can legitimately display an empty file list under the current filter; tests should assert that empty state (and switch back to `All Files` only when needed for the next expectation).

## Dialog UI tests: close GUI message boxes via `OK` click

When a dialog workflow shows an error prompt during UI-driven tests, prefer a GUI-rendered (fake) message box and close it by clicking its `OK` button, so the test harness observes a real UI change. Native message boxes may not appear in `GetApplication()->GetWindows()` and may not trigger rendering updates detectable by the UI-test framework.

## Prefer `TryGet...` helpers when asserting absence

When a helper is used in both “must exist” and “must not exist” assertions (e.g. finding the currently-open file dialog or message dialog), provide a `TryGet...()` variant that returns `nullptr` instead of asserting. This makes post-close assertions reliable and keeps failures localized to the test’s `TEST_ASSERT`.

## Multiple selection not enabled: type `a;b` in text box

When `FileDialogAllowMultipleSelection` is not enabled, the data grid won’t allow selecting multiple rows. To trigger the “multiple selection not enabled” error path, type multiple names separated by `;` (e.g. `README;root.txt`) directly into the file-name text box and then press `Open`.

## Workflow script in XML: `&` concat, `var name : type`

In Workflow scripts embedded in XML resources:

- Use `&` (not `+`) to concatenate strings.
- Declare variables with spaces around `:` (e.g. `var output : string = "";`), otherwise the resource XML precompiler can fail.

## Don’t accept multiple outcomes in one `TEST_ASSERT`

Avoid assertions like `A == expected1 || A == expected2`. Pick the correct expected behavior and assert it directly; accepting multiple outcomes can hide bugs and let a wrong guess pass.

## File dialog selection text uses quoted items + `;`

When asserting the file dialog’s selection string in the text box, expect items to be quoted and joined with `;` (e.g. `"root.txt";"root2.txt"`). This quoting can apply even for single selection, depending on the dialog implementation.

## `GuiSaveFileDialog` uses `GetFileName()` (not `GetFileNames()`)

`GuiSaveFileDialog` is single-selection: use `GetFileName()` / `FileName` instead of `GetFileNames()` / `FileNames`. `GuiOpenFileDialog` is the dialog type that supports multi-selection and `GetFileNames()`.

## Save dialog: non-existing file is allowed unless prompting

Saving to a non-existing file name should not trigger a “File(s) not exist” error prompt; that error is for open dialogs with `FileDialogFileMustExist`. Save dialog tests should focus on typed selection confirmation and (when enabled) prompt flows like overwrite/create.

## `TEST_CATEGORY` / `TEST_CASE` blocks must end with `});`

In this repo’s unit test macros, `TEST_CATEGORY(...)` and `TEST_CASE(...)` blocks must be terminated with `});` (not just `}`), or macro expansion can break parsing and cause errors like unmatched tokens at the next category/case (e.g. `error C1075: '(': no matching token found`).

## Undo/redo tests: clear baselines and respect history granularity

For `GuiDocumentCommonInterface`-based controls, use `LoadTextAndClearUndoRedo(...)` to establish a clean baseline when tests need predictable history. `ClearUndoRedo()` deletes all editing history (both undo and redo).

When validating end-of-history behavior, loop with `CanUndo()` / `CanRedo()` and assert `Undo()` / `Redo()` return `false` when history ends are reached.

History granularity matters in expectations: setting `Text` in XML or calling `SetText(...)` creates an undo step; `protocol->TypeString(...)` creates one undo step per character; paste/edit series typically produce one undo step.

## Editor smoke tests: deterministic caret placement (`Ctrl+Home`)

When reusing shared “editor smoke test” helpers across multiple `GuiDocumentCommonInterface`-based controls (singleline + multiline + XML virtual types), make caret placement deterministic:

- For multiline editors, click near the top-left so it hits the first line (e.g. `LocationOf(..., 0.0, 0.0, 2, 8)`).
- If the click still doesn’t reliably place the caret at the beginning (notably for `<DocumentTextBox/>`), follow the click with `Ctrl+Home` (`KeyPress(VKEY::KEY_HOME, /*ctrl*/true, ...)`) before typing.

Prefer making the interaction deterministic over changing XML layout just to make a click land reliably (keep singleline layout where appropriate).

## Multiline editor assertions: validate `GetDocument()` model, not `GetText()`

For multiline-mode key tests (e.g. `GuiMultilineTextBox` with `GuiDocumentParagraphMode::Multiline`), validate content through the document model:

- Use `textBox->GetDocument()->paragraphs.Count()` for line/paragraph count.
- Use `document->paragraphs[i]->GetTextForReading()` for per-line content.

Avoid asserting on `GuiDocumentCommonInterface::GetText()` for these cases, because it can normalize/convert the model and hide structural differences that matter for multiline semantics (e.g. how `ENTER`, `CTRL+ENTER`, and CRLF-preserving paste are represented).

## RichText style tests: `EditStyle` + `SummarizeStyle`, set only edited fields

For rich-text style editing tests on `GuiDocumentLabel`, prefer:

- Initialize deterministic content with `LoadTextAndClearUndoRedo(...)` (single paragraph for single-paragraph tests).
- Apply style changes via `EditStyle(begin, end, style)`.
- Verify via `SummarizeStyle(begin, end)`. For overlap tests, expect fields to be empty (e.g. `!summary->face`, `!summary->color`) when the checked property is intentionally non-uniform across the summarized range.
- When asserting “outside edited range” behavior, capture baseline summaries before applying edits and assert those baselines remain unchanged; do not assume `SummarizeStyle(...)` returns empty fields for default/unmodified styles.

When creating a `DocumentStyleProperties` for an edit, only set the fields under test; leave unrelated fields unset so assertions don’t depend on theme/default styles.

## GuiDocumentLabel paragraphs: use `\\r\\n\\r\\n` between paragraphs in `LoadTextAndClearUndoRedo`

In `GuiDocumentParagraphMode::Paragraph` with `doubleLineBreaksBetweenParagraph == true` (document label default config), feed `LoadTextAndClearUndoRedo(...)` with double CRLF between paragraphs (e.g. `L"p0\\r\\n\\r\\n p1\\r\\n\\r\\n p2"`) so tests reliably create multiple paragraphs with predictable `TextPos(row, column)` indexing.

## Hyperlink tests: `ViewOnly` interaction + merged event log + model checks

For hyperlink tests on `GuiDocumentLabel`:

- Interaction (hover/leave/click) is implemented in `GuiDocumentEditMode::ViewOnly`, so set edit mode explicitly before mouse-driven steps.
- Don’t wrap `EditHyperlink(...)` / `RemoveHyperlink(...)` in `TEST_ASSERT(...)` if they return `void`; verify edits by inspecting `textBox->GetDocument()->GetHyperlink(...)` at representative positions.
- Record `ActiveHyperlinkChanged` / `ActiveHyperlinkExecuted` into a single `List<WString>` (so you can assert execution order), and verify it with `AssertCallbacks` from `TestItemProviders.h` before clearing the list for the next step.
- When a click (or hover) frame would otherwise only “verify”, add a real UI-changing input in the same frame (e.g. move the mouse outside to deactivate) to avoid “The last frame didn’t trigger UI updating” crashes.

## Avoid searching for CRLF in deterministic tests

In test cases, everything is deterministic. Prefer asserting exact expected strings (including `\r\n` when it is part of the model) and using fixed caret indices instead of scanning for CRLF (e.g. avoid helpers like `FindCrlf(...)`).

## Caret navigation tests: type markers to expose caret

For caret-navigation tests (especially multiline where caret movement isn’t directly observable in assertions), press the navigation key(s) and then immediately type a visible marker character. Assert the resulting `GetDocument()` paragraph texts to confirm the caret moved to the expected location.

Keep paragraphs short to avoid line wrapping, so tests validate navigation rules rather than wrapping behavior.

## Unit-test renderer `CaretLineLast` should treat CRLF as newline

In unit-test rendering stubs, line-end caret movement (e.g. END via `CaretLineLast`) must treat CRLF as a single newline terminator. If line-end trimming is needed, strip both `L'\r'` and `L'\n'` so END never lands between `\r` and `\n` (which would make typed markers insert inside the CRLF sequence).
