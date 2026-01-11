# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.7: Add TEST_CATEGORY for undo/redo shortcuts

Add test cases for Ctrl+Z/Ctrl+Y and validate undo/redo behavior across representative edits (typing, deletion, paste) in a single-line textbox.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones (frame titles describe what happened in the previous frame); if an input is issued, add one more final frame whose only job is `window->Hide();` so the last logged frame stays minimal; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/UndoRedo_<CASE>")`.
- When asserting that a `WString` contains no line breaks, use `WString::IndexOf(L'\r')` and `WString::IndexOf(L'\n')` (character literals) instead of `L"\r"` / `L"\n"` to match the correct overload.
- If additional state is helpful to inspect in frame logs (e.g. `CanUndo()` / `CanRedo()` transitions), set `window->SetText(...)` in the action frame, and keep the last frame minimal (hide-only) by scheduling a final callback.
- Add `TEST_CATEGORY(L"UndoRedo")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Keep adding cases inside the existing template-based shared test function (i.e. continue using `FindObjectByName<TTextBox>` in test code, and do not hardcode `GuiDocumentLabel`).
- Add multiple `TEST_CASE`s:
  - `CtrlZ_UndoesTyping_AndCtrlY_Redoes`: type a short string, press `Ctrl+Z` to undo, assert text restored; press `Ctrl+Y`, assert text re-applied.
  - `CtrlZ_UndoesDeletion`: perform Backspace/Delete, then `Ctrl+Z` restores the deleted text.
  - `CtrlZ_UndoesPaste_FlattenedText`: paste multi-line clipboard text (flattened), then undo/redo and assert consistent results.
  - `UndoRedo_ModifiedFlags`: optionally assert `GetModified()`, `CanUndo()`, `CanRedo()` transitions are reasonable after edits and after undo back to the saved point (when `NotifyModificationSaved()` is used by initialization).

### rationale

- Undo/redo is a first-class keyboard feature (registered shortcuts) and must work with all major edit paths (char input, key deletions, paste normalization).
- These tests provide coverage for `GuiDocumentUndoRedoProcessor` integration without requiring rich text features.

# UPDATES

## UPDATE

`ClearUndoRedo()` delete all editing history in the text box. You should test it carefully. You should also test `Undo()` and `Redo()` when it fails because reaching the end of the history. Check `CanUndo()` and `CanRedo()` before calling `Undo()` and `Redo()` and after finishing all calling to consecutive `Undo()` and `Redo()` to make sure the state is correct.

There is a `LoadTextAndClearUndoRedo` function, include it in the test.

Please be awared that, if you set `Text` property in XML or call `SetText` in C++, it creates an editing history. Calling TypeString create history per character. But calling other functions like edit series or paste only generates one editing history. Keep this in mind when creating test cases.

# INSIGHTS AND REASONING

## What we are validating

This task is about proving that `GuiSinglelineTextBox` (and, by template reuse, other `GuiDocumentCommonInterface`-based controls later) correctly supports:

- Ctrl+Z (Undo) and Ctrl+Y (Redo) shortcuts.
- Correct state transitions: `CanUndo()`, `CanRedo()`, and `GetModified()`.
- Undo/redo correctness across the three key edit paths already covered by other categories:
  - Char input (`protocol->TypeString(...)`).
  - Editing keys (Backspace/Delete).
  - Paste via clipboard (with singleline flattening rules).

Additionally (per the latest update), we must validate the *API-level* undo/redo contracts exposed by `vl::presentation::controls::GuiDocumentCommonInterface`:

- `ClearUndoRedo()` truly deletes all editing history (both undo-able and redo-able steps).
- `Undo()` / `Redo()` return `false` when history ends are reached, and do not corrupt state.
- `CanUndo()` / `CanRedo()` transitions are correct:
	- Before each call to `Undo()` / `Redo()`.
	- After exhausting all consecutive `Undo()` calls.
	- After exhausting all consecutive `Redo()` calls.
- `LoadTextAndClearUndoRedo(...)` can be used to load text while ensuring a clean undo/redo baseline.

## Evidence from source code (why this should work)

### Ctrl+Z / Ctrl+Y are actual registered shortcuts

`vl::presentation::controls::GuiDocumentCommonInterface::GuiDocumentCommonInterface` registers shortcuts using `AddShortcutCommand`:

- `AddShortcutCommand(VKEY::KEY_Z, ...)` binds Undo.
- `AddShortcutCommand(VKEY::KEY_Y, ...)` binds Redo.

`GuiDocumentCommonInterface::AddShortcutCommand` creates a shortcut with `(ctrl=true, shift=false, alt=false)`, so the unit tests must send Ctrl-modified key presses (as existing Clipboard tests already do for Ctrl+C/X/V).

### Undo/redo availability and modified tracking are explicit and testable

`GuiDocumentCommonInterface` exposes:

- `CanUndo()` / `CanRedo()` which gate on `editMode == GuiDocumentEditMode::Editable` and forward to `GuiDocumentUndoRedoProcessor`.
- `Undo()` / `Redo()` which simply call `undoRedoProcessor->Undo()` / `Redo()` when allowed.
- `GetModified()` / `NotifyModificationSaved()` forwarding to the undo/redo processor.

`GuiGeneralUndoRedoProcessor` (base of `GuiDocumentUndoRedoProcessor`) defines modified as `firstFutureStep != savedStep` and updates these counters on Push / Undo / Redo. This makes `GetModified()` behavior deterministic as long as the test creates a known "saved point".

### Undo/redo edits are backed by ReplaceModel steps

Document edits are captured as undo steps via `GuiDocumentUndoRedoProcessor::OnReplaceModel(...)` (called from `GuiDocumentCommonInterface::EditTextInternal` when `clearUndoRedo == false`). Undo/redo replays by calling `GuiDocumentCommonInterface::EditRun(...)` and restoring caret.

This is why we should cover representative edits that go through this path:

- Typing inserts characters.
- Backspace/Delete removes a run.
- Paste replaces selection with clipboard text (flattened in singleline mode).

## Test structure and design choices

### Follow the existing key-test scaffold

The file `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` already uses a consistent pattern:

- Per test case, create a local `TooltipTimer timer;` to keep time deterministic (required by earlier tasks).
- `Init` frame: focus the textbox and seed deterministic text via `protocol->TypeString(L"0123456789")`.
- One action/assertion frame: perform input and assert immediately in the same callback.
- A final hide-only frame when the action frame issues input, keeping the last logged frame minimal.

Undo/redo tests should match this to keep logs comparable.

### Make undo/redo state deterministic

Typing the initial text creates undo steps (and `protocol->TypeString(...)` creates one step per character). For most shortcut tests we still want a deterministic baseline so the tested edit is the only meaningful undo step.

Two compatible approaches are worth documenting:

1) **Baseline by `ClearUndoRedo()` (keep “seed via TypeString”):**
	- Seed initial text via `protocol->TypeString(...)` in the setup frame.
	- Immediately call `textBox->ClearUndoRedo();` so the baseline has no history.
	- Optionally call `textBox->NotifyModificationSaved();` if the test also asserts `GetModified()`.

2) **Baseline by `LoadTextAndClearUndoRedo(...)` (preferred for history-sensitive cases):**
	- In the setup frame, call `textBox->LoadTextAndClearUndoRedo(L"0123456789");`.
	- This avoids creating seed history from XML `Text` or `SetText`, and avoids the per-character step behavior of `TypeString`.
	- This approach is especially suitable when the test needs to reason about how many undo steps exist (e.g. “3 chars -> 3 undos”).

Both approaches keep the test independent from internal history-merging rules for non-typing edits (paste/edit series should be exactly one step).

### Log-friendly state capture

When it helps diagnose failures, set `window->SetText(...)` inside the action frame to surface key booleans and/or current content in the frame logs (e.g. include `CanUndo()` / `CanRedo()` / `GetModified()` values). Keep the last frame hide-only.

## Proposed test cases (high-level)

Add `TEST_CATEGORY(L"UndoRedo")` under `RunSinglelineTextBoxTestCases<TTextBox>` and implement the following cases:

1. `CtrlZ_UndoesTyping_AndCtrlY_Redoes`
	- Baseline: `0123456789`.
	- Edit: move caret to a known position and `protocol->TypeString(L"X")`.
	- Undo: `protocol->KeyPress(VKEY::KEY_Z, true, false, false)` restores baseline.
	- Redo: `protocol->KeyPress(VKEY::KEY_Y, true, false, false)` restores edited text.
	- Also assert `CanUndo()` / `CanRedo()` before/after each shortcut as lightweight state checks.

2. `CtrlZ_UndoesDeletion`
	- Baseline: `0123456789`.
	- Edit: place caret and press `VKEY::KEY_BACK` (or `VKEY::KEY_DELETE`) to remove one known character.
	- Undo restores baseline; redo re-applies the deletion.
	- Include `CanUndo()` / `CanRedo()` assertions around the operations.

3. `CtrlZ_UndoesPaste_FlattenedText`
	- Baseline: `0123456789`.
	- Edit: write clipboard text containing line breaks (e.g. `L"12\r\n34\n"`), paste at a known caret position.
	- Assert result contains no `\r` or `\n` using `IndexOf(L'\r')` / `IndexOf(L'\n')` and matches expected flattened insertion.
	- Undo restores baseline; redo restores the same flattened result.
	- Include `CanUndo()` / `CanRedo()` assertions around the operations.

4. `ClearUndoRedo_DeletesAllHistory`
	- Build a non-trivial history (e.g. baseline via `LoadTextAndClearUndoRedo`, then `protocol->TypeString(L"ABC")` to create 3 steps, optionally do one Undo to create a redo step).
	- Call `textBox->ClearUndoRedo();`.
	- Assert:
		- `CanUndo() == false` and `CanRedo() == false`.
		- `textBox->Undo()` returns `false` and leaves text unchanged.
		- `textBox->Redo()` returns `false` and leaves text unchanged.
	- This pins the key contract that ClearUndoRedo removes *all* history, not just undo steps.

5. `UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect`
	- Use `LoadTextAndClearUndoRedo(L"")` for a clean baseline.
	- Perform `protocol->TypeString(L"ABC")` and expect multiple steps (one per character).
	- In the action frame, repeatedly call `Undo()` while `CanUndo()` is true; assert `Undo()` returns true each time.
	- After the loop:
		- Assert `CanUndo() == false`.
		- Call one extra `Undo()` and assert it returns `false`.
		- Assert `CanUndo() == false` still, and `CanRedo() == true`.
	- Then repeatedly call `Redo()` while `CanRedo()` is true; assert `Redo()` returns true each time.
	- After the loop:
		- Assert `CanRedo() == false`.
		- Call one extra `Redo()` and assert it returns `false`.
		- Assert `CanRedo() == false` still, and `CanUndo() == true`.
	- This explicitly validates “fail at ends” behavior and the correctness of `CanUndo()` / `CanRedo()` gating before and after consecutive operations.

4. `UndoRedo_ModifiedFlags` (optional, but valuable)
	- After baseline setup + `NotifyModificationSaved()`: assert `GetModified() == false` and `!CanUndo()`.
	- After one edit: assert `GetModified() == true`, `CanUndo() == true`, `CanRedo() == false`.
	- After undo back to saved point: assert `GetModified() == false`, `CanUndo()` state matches "no more undo", and `CanRedo() == true`.
	- After redo: assert `GetModified() == true`, `CanRedo() == false`.

## Risks / dependencies

- These tests assume `GuiSinglelineTextBox` is in `GuiDocumentEditMode::Editable` in the resource under test (already true for the existing key test resource). If a future control reuses these tests with ViewOnly mode, `CanUndo()` and edits will be gated.
- Undo/redo operates on discrete undo steps. Clearing undo/redo after seeding is the key design choice to keep tests stable and independent from how `TypeString` batches keystrokes.

## Notes about history creation (to keep tests robust)

Per the update, the following behaviors should be treated as *test invariants* (or explicitly avoided unless a test is validating them):

- Setting `Text` in XML or calling `SetText(...)` creates an undo step.
- `protocol->TypeString(...)` creates one undo step per character.
- Paste / edit series produce one undo step.

Therefore:

- Avoid putting `Text="..."` in the XML resource for these tests unless the test explicitly wants to validate that this generates history.
- Prefer `LoadTextAndClearUndoRedo(...)` to establish a clean baseline when the test needs predictable undo-step counting.

# !!!FINISHED!!!
