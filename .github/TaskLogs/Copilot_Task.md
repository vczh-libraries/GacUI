# !!!TASK!!!

# PROBLEM DESCRIPTION
## TASK No.6: Test Undo/Redo for Style Operations

Write test cases for undo/redo behavior of `EditStyleName`, `RemoveStyleName`, and `RenameStyle`, each under their own `TEST_CATEGORY` hierarchy. This task covers the following TEST_CATEGORYs:
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"EditStyleName")`
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RemoveStyleName")`
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RenameStyle")`

### what to be done

**Under `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"EditStyleName")` and `TEST_CATEGORY(L"RemoveStyleName")`:**

- **Test: Undo EditStyleName** — Apply a style name, verify `CanUndo()` is true, call `Undo()`, verify `SummarizeStyleName` returns null (original state).
- **Test: Redo EditStyleName** — After undoing, verify `CanRedo()` is true, call `Redo()`, verify `SummarizeStyleName` returns the style name again.
- **Test: Undo RemoveStyleName** — Apply a style name, then remove it, then undo the removal. Verify the style name is restored.
- **Test: Redo RemoveStyleName** — After undoing a removal, redo it. Verify the style name is removed again.
- **Test: Multiple edits then undo all** — Apply style, then remove style, then undo both in sequence. Verify we return to the original unstyled state.
- **Test: Undo/Redo with CanUndo/CanRedo checks** — Verify `CanUndo()` and `CanRedo()` return correct values at each step of the undo/redo chain.

**Under `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RenameStyle")`:**

- **Test: Undo RenameStyle** — Register a style, apply it, rename it, then undo. Verify:
  - The style name in text runs reverts to the old name.
  - The styles dictionary reverts to the old name.
- **Test: Redo RenameStyle** — After undoing, redo. Verify everything is renamed again.
- **Test: Undo RenameStyle with parent references** — Register parent and child styles, rename parent, undo. Verify the child's `parentStyleName` reverts to the original parent name.
- **Test: Multiple renames then undo all** — Rename `"A"` to `"B"`, then `"B"` to `"C"`, then undo both. Verify we return to `"A"`.

### rationale

- `EditStyleName` and `RemoveStyleName` both use `EditStyleInternal` which takes a before/after snapshot of the document model and submits to `undoRedoProcessor->OnReplaceModel()`. This is the same mechanism used by `EditStyle` and is confirmed in `GuiDocumentCommonInterface.cpp`.
- `RenameStyle` uses a separate undo/redo path: `undoRedoProcessor->OnRenameStyle(arguments)` with `RenameStyleStruct`, instead of the `ReplaceModelStruct` used by `EditStyleName`/`RemoveStyleName`. The `RenameStyleStep::Undo()` and `Redo()` in `GuiTextUndoRedo.cpp` (lines 136, 145) swap old/new names.
- The undo/redo pattern from `TestControls_Editor_Key_Shared.cpp` shows checking `CanUndo/CanRedo` and calling `Undo/Redo` in loops — this same pattern should be used for style operations.
- All undo/redo tests are grouped into one task while keeping separate TEST_CATEGORYs for each operation, since undo/redo is a cross-cutting concern and can be implemented together.

# UPDATES

# INSIGHTS AND REASONING

## Goal / scope

This design covers **TASK No.6** only: add unit tests to `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp` verifying undo/redo behavior for:
- `vl::presentation::controls::GuiDocumentCommonInterface::EditStyleName`
- `vl::presentation::controls::GuiDocumentCommonInterface::RemoveStyleName`
- `vl::presentation::controls::GuiDocumentCommonInterface::RenameStyle`

Out of scope:
- Any production-code change.
- Restoring full UnitTest run (TASK No.7).

## Evidence / behavior constraints from source code

### Undo/redo stacks are step-based and redo history is dropped on new edits

- `GuiGeneralUndoRedoProcessor::PushStep` truncates future steps (`RemoveRange(firstFutureStep, ...)`) when a new step is pushed after an undo, meaning redo history is discarded on any fresh edit.
- During undo/redo execution, `performingUndoRedo=true` prevents `PushStep` from recording new steps; this is essential for `RenameStyleStep::Undo/Redo` which calls back into `GuiDocumentCommonInterface::RenameStyle`.

(See `Source\Controls\TextEditorPackage\EditorCallback\GuiTextUndoRedo.cpp`.)

### EditStyleName / RemoveStyleName use model snapshot replacement steps

- Both `GuiDocumentCommonInterface::EditStyleName` and `RemoveStyleName` call `EditStyleInternal(begin, end, ...)`.
- `EditStyleInternal` snapshots the document before and after the edit using `documentElement->GetDocument()->CopyDocument(begin, end, true)` and submits a `GuiDocumentUndoRedoProcessor::ReplaceModelStruct` via `undoRedoProcessor->OnReplaceModel(arguments)`.

Implications for tests:
- Each call to `EditStyleName` / `RemoveStyleName` should produce exactly one undo step.
- Undo should restore the original style-name runs for the edited range; redo should reapply the post-edit runs.

(See `Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp`, `EditStyleInternal` and the style-name methods.)

### ReplaceModelStep changes both document content and caret selection

- `GuiDocumentUndoRedoProcessor::ReplaceModelStep::Undo/Redo` uses `ci->EditRun(...)` with the saved model and then calls `ci->SetCaret(...)`.

Implications for tests:
- The primary assertions should be on externally observable model state (style summaries and style registry), not on caret position (caret changes are incidental for these style-only edits).

(See `Source\Controls\TextEditorPackage\EditorCallback\GuiTextUndoRedo.cpp`.)

### RenameStyle uses a dedicated rename step (not a model snapshot)

- `GuiDocumentCommonInterface::RenameStyle` directly calls `documentElement->RenameStyle(oldStyleName, newStyleName)` and then submits a `RenameStyleStruct` to `undoRedoProcessor->OnRenameStyle(arguments)`.
- `GuiDocumentUndoRedoProcessor::RenameStyleStep::Undo` calls `ci->RenameStyle(new, old)` and `Redo` calls `ci->RenameStyle(old, new)`.

Implications for tests:
- Undo/redo for rename must validate both:
  - The style name stored in text runs (via `SummarizeStyleName`).
  - The style registry / parent references (`textBox->GetDocument()->styles`).

(See `Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp` and `Source\Controls\TextEditorPackage\EditorCallback\GuiTextUndoRedo.cpp`.)

## Proposed changes (test-only, high-level)

### Where to add tests

Update existing file:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

### Category structure

Under the existing `TEST_CATEGORY(L"Styles")`, add:
- `TEST_CATEGORY(L"UndoRedo")`
  - `TEST_CATEGORY(L"EditStyleName")`
  - `TEST_CATEGORY(L"RemoveStyleName")`
  - `TEST_CATEGORY(L"RenameStyle")`

These categories should follow the existing style-test harness pattern in this file (resource XML, `GacUIUnitTest_SetGuiMainProxy`, `Init` + `Verify` frames).

### Test harness requirements (must-follow)

- **Stable test log paths**: every new `GacUIUnitTest_StartFast_WithResourceAsText(...)` must pass a stable log path string under:
  - `Controls/Editor/Features/Styles/UndoRedo/EditStyleName/<CaseName>`
  - `Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/<CaseName>`
  - `Controls/Editor/Features/Styles/UndoRedo/RenameStyle/<CaseName>`

- **Frame choreography**: every `protocol->OnNextIdleFrame(...)` callback must trigger an observable UI update.
  - Do not schedule verification-only frames that only query `CanUndo()` / `CanRedo()`.
  - Anchor frames on UI-changing actions: `EditStyleName`, `RemoveStyleName`, `RenameStyle`, `Undo()`, `Redo()`, and the final `window->Hide()`.
  - Keep frame titles consistent with the file’s existing convention (title describes what happened in the previous frame).

- **Verified clean baseline**: after `LoadTextAndClearUndoRedo(...)`, the first assertions in every test must be:
  - `textBox->CanUndo() == false`
  - `textBox->CanRedo() == false`

  Any required style registration / setup must be done before the baseline is cleared (i.e. before `LoadTextAndClearUndoRedo(...)`) to avoid accidental undo steps.

### Test design (what to assert)

All undo/redo tests should:
- Ensure the operation actually changes state before asserting undo/redo behavior (especially important for `RemoveStyleName` and `RenameStyle`).
- Explicitly assert `CanUndo()` / `CanRedo()` transitions around each UI-changing action.
- Verify both text runs and the style registry where applicable:
  - Use presence checks like `textBox->GetDocument()->styles.Keys().Contains(L"Name")` (do not copy-assign the dictionary).
  - For parent relationship checks, verify the style object field (e.g. `textBox->GetDocument()->styles[L"Child"]->parentStyleName`, or the confirmed equivalent) in addition to run summaries.
- Confirm `SummarizeStyleName` return type before encoding “null” expectations.
  - If it returns `Nullable<WString>`, assert absence via `!result` / `result == Nullable<WString>()` (avoid string comparisons).

Concrete test cases (each one is a scenario test; no standalone “CanUndo/CanRedo-only” test):

- `UndoRedo/EditStyleName`:
  - Basic undo/redo: apply a style name on `[0,10)`; verify summary present; `Undo()` restores absence; `Redo()` re-applies.
  - **Redo truncation**: apply style name → `Undo()` → apply a different style name → assert `CanRedo() == false`.
  - **End-of-history boundary**: drain undo and redo with loops and assert boundaries:
    - When exhausted: `CanUndo() == false` and `Undo()` returns `false`.
    - After redoing to the end: `CanRedo() == false` and `Redo()` returns `false`.

- `UndoRedo/RemoveStyleName`:
  - Apply style name then remove it; `Undo()` restores; `Redo()` removes again.
  - Multi-step chain with **intermediate state** checks:
    - apply → remove → `Undo()` (must restore to post-apply state) → `Undo()` (must restore original unstyled state).

- `UndoRedo/RenameStyle`:
  - Basic rename undo/redo: register `OldName`, apply, rename to `NewName`; undo/redo must validate both run summaries and registry key presence.
  - Parent reference: `Parent` + `Child(parent=Parent)`; rename parent; `Undo()` must restore `styles[L"Child"]->parentStyleName` to `Parent`.
  - Multiple renames with **intermediate state** checks: `A→B→C`, then `Undo()` (verify `B`), then `Undo()` (verify `A`) in both runs and registry.

## Risks / guardrails

- Undo/redo is gated by `editMode == GuiDocumentEditMode::Editable` in `CanUndo/CanRedo`; ensure all tests run with `EditMode="Editable"` (already used by the existing style test resource).
- `RenameStyle` always submits an undo step regardless of whether the underlying rename succeeds; for deterministic undo/redo tests, only perform renames that are guaranteed to succeed (old exists, new does not) and avoid testing failure cases under undo/redo here.
- Avoid empty ranges (`begin == end`) since summary and snapshot behavior can differ (and some APIs intentionally return empty in that case).

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# !!!FINISHED!!!
