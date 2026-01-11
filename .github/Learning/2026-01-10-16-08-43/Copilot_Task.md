# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.4: Add TEST_CATEGORY for navigation and selection keys

Add test cases for caret movement and keyboard-driven selection extension in a single-line document.

Keyboard operations covered by this category (from `GuiDocumentCommonInterface::ProcessKey` + modifiers): `Left/Right/Home/End/Up/Down/PageUp/PageDown`, and Shift/Ctrl combinations (Ctrl does not change caret semantics here; Shift extends selection).

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified): in `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones; keep the final callback minimal so the last logged frame is easy to inspect; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Navigation_<CASE>")`.
- Add `TEST_CATEGORY(L"Navigation")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s (examples; final names can be adjusted to match existing naming style):
	- `ArrowKeys_MoveCaret`: start with known text, put caret at end, press `KEY_LEFT`/`KEY_RIGHT` and assert caret positions change as expected.
	- `HomeEnd_MoveCaretToEdges`: place caret mid-text, press `KEY_HOME` then `KEY_END`, assert caret at column 0 / end column.
	- `ShiftArrow_ExtendsSelection`: place caret, press `Shift+KEY_LEFT/RIGHT`, assert `GetSelectionText()` matches the selected substring and caret begin/end reflect selection direction.
	- `ShiftHomeEnd_SelectToEdges`: from mid-text, `Shift+KEY_HOME` selects to start, `Shift+KEY_END` selects to end; assert selection text.
	- `UpDown_NoEffectInSingleline`: assert `KEY_UP/KEY_DOWN` do not change caret/selection in a single-line textbox.
	- `PageKeys_NoEffectInSingleline`: assert `KEY_PRIOR/KEY_NEXT` do not change caret/selection in a single-line textbox.
	- `CtrlWithNavigation_DoesNotChangeBehavior`: spot-check `Ctrl+KEY_LEFT` (and/or `Ctrl+KEY_HOME`) behaves the same as without Ctrl for this control.

### rationale

- Caret movement and selection extension are the foundation for all other keyboard edit operations (delete/cut/copy/paste/undo), so they should be validated first.
- Explicitly asserting that Up/Down and Page keys have no effect in singleline prevents accidental behavior regressions if paragraph/caret calculation changes.

# UPDATES

# INSIGHTS AND REASONING

## What “navigation and selection keys” mean in this codebase

Keyboard navigation for document-based editor controls is implemented centrally in `vl::presentation::controls::GuiDocumentCommonInterface` (Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h/.cpp). The key handling path relevant to this task is:

- `GuiDocumentCommonInterface::OnKeyDown(...)` calls `GuiDocumentCommonInterface::ProcessKey(VKEY code, bool shift, bool ctrl)`.
- `ProcessKey` first checks `internalShortcutKeyManager->TryGetShortcut(ctrl, shift, false, code)`.
	- This is where Ctrl-based shortcuts are handled.
	- In `GuiDocumentCommonInterface` constructor, only Ctrl+Z/Y/A/X/C/V are registered via `AddShortcutCommand(...)` (Undo/Redo/SelectAll/Cut/Copy/Paste). There is no shortcut registered for Ctrl+Left/Home/etc., so Ctrl+navigation should fall through to the same caret-move logic.
- If no shortcut is found, `ProcessKey` handles movement keys in a `switch (code)` and calls `documentElement->CalculateCaret(...)` plus `Move(newCaret, shift, frontSide)`.
	- `Move(...)` is the key reason selection extension happens: when `shift==true`, it preserves the current selection begin and only updates the end, effectively extending (or shrinking / reversing) selection.
	- When `shift==false`, begin is reset to the new caret, clearing any selection.

The caret/selection state is externally observable via the public API in `GuiDocumentCommonInterface`:

- `GetCaretBegin()` / `GetCaretEnd()` return `vl::presentation::TextPos` (row/column).
- `GetSelectionText()` returns the selected substring.

In a single-line textbox, caret positions should remain in `TextPos.row == 0` and only `TextPos.column` changes.

## Frame-based test constraints and how to apply them

The unit tests run in remote mode with `UnitTestRemoteProtocol::OnNextIdleFrame(...)` being the frame boundary. The knowledge base explicitly describes this as frame-based testing (see .github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md, Unit Testing Integration).

For this repo’s preferred test style (as clarified in the scrum updates and already reflected in the current scaffold file):

- Each callback should both perform the input(s) and assert the resulting state in the same callback.
- Only schedule another frame when the current callback triggers a UI change that will be visible next frame.
- Keep the last frame minimal (just hide the window).
- Frame titles describe what happened in the previous frame.

## Proposed test architecture for TASK No.4

### Where to add tests

Add a `TEST_CATEGORY(L"Navigation")` inside `RunSinglelineTextBoxTestCases(...)` in Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp, alongside the existing scaffold category. This matches the reuse goal (tests are callable for other `GuiDocumentLabel` subclasses later).

### Shared initial state

All cases in this category should use the same deterministic setup, to make assertions stable and easy to read:

- Ensure the control has focus (`SetFocused()`), because `ProcessKey` is wired to the focusable composition.
- Start from a known text buffer (e.g. `L"0123456789"`).
- For navigation-only tests, assert that the text content is unchanged after key presses.

To avoid relying on implicit caret placement rules after typing, explicitly place the caret using `GuiDocumentCommonInterface::SetCaret(TextPos begin, TextPos end)` in the setup frame.

### What to assert

Prefer validating caret and selection state through public APIs:

- `GetCaretBegin()` / `GetCaretEnd()` for selection geometry.
- `GetSelectionText()` for selection content.
- `GetText()` as a guard that navigation keys do not mutate text.

This keeps the tests aligned with “keyboard operations” rather than internal rendering details.

## Proposed test cases (Navigation category)

The following cases cover the key paths in `GuiDocumentCommonInterface::ProcessKey` that are relevant to navigation and selection extension.

### 1) ArrowKeys_MoveCaret

Goal: Validate `KEY_LEFT` and `KEY_RIGHT` move the caret by one column and do not create selections.

Setup:

- Text = `0123456789`.
- Caret = `(row=0, column=10)` (end of text).

Actions and assertions:

- Press `KEY_LEFT`.
	- Expect caret begin/end both become `(0, 9)`.
	- Expect `GetSelectionText()` is empty.
- Press `KEY_RIGHT`.
	- Expect caret returns to `(0, 10)`.
	- Text remains unchanged.

Rationale: This validates the most basic `CalculateCaret(...CaretMoveLeft/Right...)` path and that `shift==false` collapses selection.

### 2) HomeEnd_MoveCaretToEdges

Goal: Validate `KEY_HOME` and `KEY_END` move caret to edges, including the fallback logic in `ProcessKey` (line-first/last to document-first/last).

Setup:

- Text = `0123456789`.
- Caret = `(0, 5)`.

Actions and assertions:

- Press `KEY_HOME`.
	- Expect caret `(0, 0)`.
- Press `KEY_END`.
	- Expect caret `(0, 10)`.

Rationale: Exercises the `KEY_HOME/KEY_END` special-case logic in `ProcessKey` and ensures correct edge behavior in single-line.

### 3) ShiftArrow_ExtendsSelection

Goal: Validate Shift extends selection and that selection text matches the expected substring.

Setup:

- Text = `0123456789`.
- Caret = `(0, 5)` with no selection (begin=end).

Actions and assertions:

- Press `Shift+KEY_LEFT`.
	- Expect a non-empty selection.
	- Expect `GetSelectionText()` equals `L"4"` (selects the character immediately to the left of the original caret).
	- Expect caret begin/end reflect selection direction (begin remains at column 5, end becomes column 4).

Optional extension (still within one test if it stays readable):

- Press `Shift+KEY_RIGHT` to shrink/clear or reverse selection and assert selection geometry updates correctly.

Rationale: Directly validates `Move(newCaret, shift=true, ...)` preserving selection begin.

### 4) ShiftHomeEnd_SelectToEdges

Goal: Validate Shift+Home/End selects to edges.

Setup:

- Text = `0123456789`.
- Caret = `(0, 5)`.

Actions and assertions:

- Press `Shift+KEY_HOME`.
	- Expect `GetSelectionText()` equals `L"01234"`.
- Reset to caret `(0, 5)` (or use a separate `TEST_CASE` to keep steps minimal).
- Press `Shift+KEY_END`.
	- Expect `GetSelectionText()` equals `L"56789"`.

Rationale: Covers Shift interaction with `KEY_HOME/KEY_END` branches and validates selection text correctness, not just caret positions.

### 5) UpDown_NoEffectInSingleline

Goal: Guard against unintended vertical navigation in a single-line control.

Setup:

- Text = `0123456789`.
- Caret = `(0, 5)`.

Actions and assertions:

- Press `KEY_UP`, then `KEY_DOWN`.
	- Expect caret begin/end unchanged.
	- Expect selection remains empty.
	- Expect text unchanged.

Rationale: `ProcessKey` has explicit cases for Up/Down, so this ensures `CalculateCaret(...MoveUp/MoveDown...)` is a no-op in singleline and stays that way.

### 6) PageKeys_NoEffectInSingleline

Goal: Validate `KEY_PRIOR` / `KEY_NEXT` do not change state.

Evidence: In `GuiDocumentCommonInterface::ProcessKey`, `KEY_PRIOR` and `KEY_NEXT` cases are currently empty.

Setup and assertions:

- Same as the Up/Down test.
- Press `KEY_PRIOR`, `KEY_NEXT`.
	- Expect caret/selection/text unchanged.

Rationale: Prevents future changes from accidentally introducing page-navigation semantics into single-line controls.

### 7) CtrlWithNavigation_DoesNotChangeBehavior

Goal: Spot-check that Ctrl modifier does not change caret semantics for navigation keys in this control.

Evidence: Ctrl is only used for shortcut dispatch via `internalShortcutKeyManager`; the caret-move branches in `ProcessKey` do not consult `ctrl` for navigation keys.

Setup:

- Text = `0123456789`.
- Caret = `(0, 5)`.

Actions and assertions:

- Press `Ctrl+KEY_LEFT`.
	- Expect caret behaves like plain `KEY_LEFT` (caret becomes `(0, 4)`) and selection stays empty.

Rationale: This test defends the intended “Ctrl does not change caret semantics here” guarantee stated in the scrum task.

## Notes on reuse for other GuiDocumentLabel subclasses

All assertions rely on `GuiDocumentCommonInterface` public APIs (`GetCaretBegin`, `GetCaretEnd`, `GetSelectionText`, `GetText`) and the control lookup uses `FindObjectByName<GuiDocumentLabel>`, so the same test cases should be reusable for other controls inheriting from `GuiDocumentLabel` without changing test logic.

# !!!FINISHED!!!
