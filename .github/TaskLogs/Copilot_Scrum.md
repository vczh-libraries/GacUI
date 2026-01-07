# !!!SCRUM!!!

# DESIGN REQUEST

# Problem

There are 4 controls inheriting from `GuiDocumentCommonInterface`. There differences are just twisting some rendering and user input. They share the same core design and implementation. At the moment we are focusing on designing test cases for `GuiSinglelineTextBox`.

In  [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp) there are already some basic test cases. But I would like you to add more test cases to the currently empty  `TestControls_Editor_GuiSinglelineTextBox_Key.cpp. It stores test cases for keyboard operations and typing.

You need to analyse how many keyboard operations does `GuiSinglelineTextBox` supports. Categorize all of them and design multiple test cases covering all of them. Keyboard operations including key strokes combinations, predefined shortcut like ctrl+c etc, and typing. 

`GuiSinglelineTextBox` is a text box for single line of plain text. Although it has rich text functions in its base class, but the constructor already configures all of them to perform plain text operations only. For example, changing font or text color will be ignores, pasting styled text will not apply styles, CrLf will be removed, etc.

You need to do analysis right now, get the result, and categorize all keyboard operations test cases into multiple TEST_CATEGORY of multiple TEST_CASE. Each TEST_CATEGORY becomes a new task following the first one.  Analysing itself should not be a task in the scrum document because it does not edit source code.

# UPDATES

# TASKS

- [ ] TASK No.1: Create GuiSinglelineTextBox key test scaffold
- [ ] TASK No.2: Add TEST_CATEGORY for navigation and selection keys
- [ ] TASK No.3: Add TEST_CATEGORY for deletion and Enter keys
- [ ] TASK No.4: Add TEST_CATEGORY for clipboard shortcuts
- [ ] TASK No.5: Add TEST_CATEGORY for undo/redo shortcuts
- [ ] TASK No.6: Add TEST_CATEGORY for typing and char-input filtering

## TASK No.1: Create GuiSinglelineTextBox key test scaffold

Create a shared test window/resource and minimal helper flow in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` so later categories can focus on keyboard behaviors without repeating boilerplate.

### what to be done

- Implement `TEST_FILE { ... }` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add a minimal resource XML (similar to `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`) containing a `Window` with a `SinglelineTextBox ref.Name="textBox"`.
- Ensure each test case:
  - Brings focus to the textbox (`SetFocused()`).
  - Starts from a known text value (preferably something index-friendly like `L"0123456789"` or `L"Hello World"`).
  - Uses `UnitTestRemoteProtocol` helpers (`KeyPress`, `TypeString`) and asserts via `GetText()`, `GetSelectionText()`, `GetCaretBegin()/GetCaretEnd()` where needed.

### rationale

- All subsequent tasks add `TEST_CATEGORY` blocks to the same file; a shared scaffold keeps those tasks small and consistent.
- Using an index-friendly initial text makes it easy to assert caret/selection changes precisely.

## TASK No.2: Add TEST_CATEGORY for navigation and selection keys

Add test cases for caret movement and keyboard-driven selection extension in a single-line document.

Keyboard operations covered by this category (from `GuiDocumentCommonInterface::ProcessKey` + modifiers): `Left/Right/Home/End/Up/Down/PageUp/PageDown`, and Shift/Ctrl combinations (Ctrl does not change caret semantics here; Shift extends selection).

### what to be done

- Add `TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - Navigation")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
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

## TASK No.3: Add TEST_CATEGORY for deletion and Enter keys

Add test cases for editing keys handled in `ProcessKey`: Backspace, Delete, Enter, and Ctrl+Enter, including behavior with and without a selection.

In `GuiSinglelineTextBox`, Enter is processed but line breaks are flattened by configuration (`paragraphMode=Singleline`, `spaceForFlattenedLineBreak=false`), so no newline should remain in the resulting text.

### what to be done

- Add `TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - Deletion")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s:
  - `Backspace_DeletesPreviousChar`: set caret after a known character; press `KEY_BACK`; assert one character removed and caret moved appropriately.
  - `Delete_DeletesNextChar`: set caret before a known character; press `KEY_DELETE`; assert one character removed.
  - `Backspace_DeletesSelection` and `Delete_DeletesSelection`: create a selection via `Shift+Arrow` or `SelectAll()`, then press Backspace/Delete; assert selection removed.
  - `BackspaceAtStart_NoChange` and `DeleteAtEnd_NoChange`: boundary behavior should not change text.
  - `Enter_DoesNotInsertNewline_ButCanDeleteSelection`: with no selection, press `KEY_RETURN` and assert text unchanged; with a selection, press `KEY_RETURN` and assert selection removed.
  - `CtrlEnter_DoesNotInsertNewline_ButCanDeleteSelection`: same as above but using `KeyPress(KEY_RETURN, ctrl=true, ...)`.

### rationale

- Backspace/Delete/Enter are the primary editing keys beyond typing; they have subtle selection-dependent behavior.
- Single-line newline-flattening is a key “plain text” guarantee of `GuiSinglelineTextBox` and needs coverage.

## TASK No.4: Add TEST_CATEGORY for clipboard shortcuts

Add test cases for Ctrl+C/Ctrl+X/Ctrl+V, using the fake clipboard service in unit tests and validating single-line plain-text normalization on paste (CR/LF removed, styles ignored).

### what to be done

- Add `TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - Clipboard")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Use `GetCurrentController()->ClipboardService()->WriteClipboard()` / `ReadClipboard()` in test code to set up and verify clipboard content.
- Add multiple `TEST_CASE`s:
  - `CtrlC_CopiesSelection_TextUnchanged`: create a selection, press `Ctrl+C`, assert textbox text unchanged and clipboard text equals the selection.
  - `CtrlX_CutsSelection_TextUpdated`: create a selection, press `Ctrl+X`, assert textbox text removed selection and clipboard text equals the removed substring.
  - `CtrlV_PastesAtCaret`: pre-fill clipboard text, place caret, press `Ctrl+V`, assert insertion at caret.
  - `CtrlV_ReplacesSelection`: pre-fill clipboard, create selection, press `Ctrl+V`, assert selection replaced.
  - `CtrlV_FlattensLineBreaks`: set clipboard text containing `\r\n` and/or `\n` (and optional double line breaks); paste and assert resulting textbox text contains no line breaks and joins segments without adding spaces (since `spaceForFlattenedLineBreak=false`).
  - `CtrlV_EmptyClipboard_NoChange`: ensure clipboard has no text, press `Ctrl+V`, assert text unchanged.

### rationale

- Clipboard shortcuts are core keyboard operations and are explicitly registered as shortcuts in `GuiDocumentCommonInterface`.
- Singleline + paste-as-plain-text behavior is easy to regress when document/clipboard handling changes; these tests pin expected normalization.

## TASK No.5: Add TEST_CATEGORY for undo/redo shortcuts

Add test cases for Ctrl+Z/Ctrl+Y and validate undo/redo behavior across representative edits (typing, deletion, paste) in a single-line textbox.

### what to be done

- Add `TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - UndoRedo")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s:
  - `CtrlZ_UndoesTyping_AndCtrlY_Redoes`: type a short string, press `Ctrl+Z` to undo, assert text restored; press `Ctrl+Y`, assert text re-applied.
  - `CtrlZ_UndoesDeletion`: perform Backspace/Delete, then `Ctrl+Z` restores the deleted text.
  - `CtrlZ_UndoesPaste_FlattenedText`: paste multi-line clipboard text (flattened), then undo/redo and assert consistent results.
  - `UndoRedo_ModifiedFlags`: optionally assert `GetModified()`, `CanUndo()`, `CanRedo()` transitions are reasonable after edits and after undo back to the saved point (when `NotifyModificationSaved()` is used by initialization).

### rationale

- Undo/redo is a first-class keyboard feature (registered shortcuts) and must work with all major edit paths (char input, key deletions, paste normalization).
- These tests provide coverage for `GuiDocumentUndoRedoProcessor` integration without requiring rich text features.

## TASK No.6: Add TEST_CATEGORY for typing and char-input filtering

Add test cases for the `OnCharInput` path: normal typing, tab input, control-modified typing being ignored, and newline characters being flattened/ignored as appropriate for single-line text.

### what to be done

- Add `TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - Typing")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s:
  - `TypeString_InsertsPlainText`: use `protocol->TypeString(L\"...\")` and assert text updates.
  - `TypeString_InsertsTab_WhenAcceptTabInput`: type `L\"\\t\"` and assert a tab character appears in text.
  - `TypeString_IgnoresWhenCtrlPressed`: use `protocol->_KeyDown(KEY_CONTROL)` + `TypeString(...)` + `_KeyUp(KEY_CONTROL)` and assert no text change.
  - `TypeString_NewlineCharsFlattened`: type a string containing `\\n` and assert the resulting text contains no line breaks (flattened by formatting); also verify `\\r` (KEY_RETURN) is ignored by char input.
  - `TypeString_ReplacesSelection`: create a selection (e.g. Shift+Arrow or `SelectAll()`), then `TypeString(...)` and assert replacement.

### rationale

- `GuiSinglelineTextBox` is configured to accept only plain text; char-input filtering + flattening is the core guarantee.
- Explicitly testing tab and ctrl-modified typing prevents regressions in `OnCharInput` gating logic.

# Impact to the Knowledge Base

## None

# !!!FINISHED!!!
