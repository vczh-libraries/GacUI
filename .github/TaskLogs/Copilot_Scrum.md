# !!!SCRUM!!!
# DESIGN REQUEST
There are 4 controls inheriting from `GuiDocumentCommonInterface`. There differences are just twisting some rendering and user input. They share the same core design and implementation. At the moment we are focusing on designing test cases for `GuiSinglelineTextBox`.

In  [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp) there are already some basic test cases. But I would like you to create a `TestControls_Editor_GuiSinglelineTextBox_Key.cpp. It stores test cases for keyboard operations and typing.

The first task would be create that file and put it in the project and the same solution explorer folder, with an empty `TEST_FILE` block.

And then you need to analyse how many keyboard operations does `GuiSinglelineTextBox` supports. Categorize all of them and design multiple test cases covering all of them. Keyboard operations including key strokes combinations, predefined shortcut like ctrl+c etc, and typing.

`GuiSinglelineTextBox` is a text box for single line of plain text. Although it has rich text functions in its base class, but the constructor already configures all of them to perform plain text operations only. For example, changing font or text color will be ignores, pasting styled text will not apply styles, CrLf will be removed, etc.

You need to do analysis right now, get the result, and categorize all keyboard operations test cases into multiple TEST_CATEGORY of multiple TEST_CASE. Each TEST_CATEGORY becomes a new task following the first one.  Analysing itself should not be a task in the scrum document because it does not edit source code.
# UPDATES
## UPDATE
In each test case, you need to verify both text, selected text and caret range.
# TASKS
- [x] TASK No.1: Add GuiSinglelineTextBox keyboard test file stub
- [ ] TASK No.2: Add navigation keyboard tests for GuiSinglelineTextBox
- [ ] TASK No.3: Add deletion key tests for GuiSinglelineTextBox
- [ ] TASK No.4: Add typing behavior tests for GuiSinglelineTextBox
- [ ] TASK No.5: Add clipboard shortcut tests for GuiSinglelineTextBox
- [ ] TASK No.6: Add undo/redo shortcut tests for GuiSinglelineTextBox
## TASK No.1: Add GuiSinglelineTextBox keyboard test file stub
Create `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, include `TestControls.h`, and add an empty `TEST_FILE` block that will host upcoming keyboard-focused tests. Add the new file to `UnitTest.vcxproj` and its `.filters` entry under the same “Source Files\Controls\Editor” filter so it shows up next to the existing text box tests.
### what to be done
- Add the new `.cpp` with just `#include "TestControls.h"` and an empty `TEST_FILE {}` so future tasks have a compilation unit to modify.
- Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj` and `.filters` to include the file under the Controls\Editor grouping.
### how to test it
- Build the `UnitTest` project to ensure the new translation unit compiles cleanly despite being empty.
### rationale
- The new keyboard-focused tests need a dedicated compilation unit so they don't overgrow the existing general `GuiSinglelineTextBox` tests and so they can be enabled/disabled independently in Visual Studio’s test explorer folder.

## TASK No.2: Add navigation keyboard tests for GuiSinglelineTextBox
Introduce `TEST_CATEGORY(L"GuiSinglelineTextBox.Keys.Navigation")` inside the new file with cases that cover every caret-movement key path exposed by `GuiDocumentCommonInterface::ProcessKey`.
### what to be done
- Add a shared resource definition identical to the existing text box tests so both files operate on the same UI.
- Implement test cases:
  - `TEST_CASE(L"Arrow keys move caret and shift selection")` - type a known string, move caret left/right with and without `shift`, and assert `GetCaretBegin/End` track expected columns.
  - `TEST_CASE(L"Home and End clamp caret to text edges")` - place caret mid-string, press `Home`/`End` (with and without `shift`) and verify caret or selection spans the start/end positions only.
  - `TEST_CASE(L"Vertical and page keys keep caret on the only line")` - press `Up`, `Down`, `PageUp`, and `PageDown` and assert both caret positions stay unchanged because the control is single-line.
- In every navigation test case, assert the resulting text (even if unchanged), the active selection range, and the caret begin/end indices after each simulated key sequence.
### how to test it
- Run the new category through the GacUI unit-test harness (e.g., `UnitTest` project) and ensure assertions on text content, caret positions, and selection boundaries succeed.
### rationale
- Navigation keys are the entry point for most keyboard interactions; verifying they move and select text as expected (or stay put when unsupported) ensures subsequent editing or typing tests can rely on predictable caret positions.

## TASK No.3: Add deletion key tests for GuiSinglelineTextBox
Add `TEST_CATEGORY(L"GuiSinglelineTextBox.Keys.Deletion")` to exercise `Backspace`, `Delete`, and `Return` paths that call `EditText`, ensuring single-line formatting is enforced.
### what to be done
- Add test cases:
  - `TEST_CASE(L"Backspace and Delete remove characters or selection")` - cover both empty selections (one character removal) and multi-character selections, asserting resulting text and caret placement.
  - `TEST_CASE(L"Enter and Ctrl+Enter flatten to single line")` - invoke `KeyPress(VKEY::KEY_RETURN)` with and without `ctrl` in the middle of text and confirm no `\r`/`\n` remains, and caret/selection behave like an insertion of nothing.
- Ensure each deletion-focused test asserts the updated text content, the selection (e.g., `GetCaretBegin/End` before collapse), and the caret range after the key press.
### how to test it
- Run the category to ensure text, selection boundaries, and caret positions after each deletion scenario match expectations while remaining single-line.
### rationale
- These keys hit the special-case logic in `ProcessKey` plus the single-line formatting pipeline; exercising them ensures regressions in CR/LF flattening or deletion selection handling are caught quickly.

## TASK No.4: Add typing behavior tests for GuiSinglelineTextBox
Create `TEST_CATEGORY(L"GuiSinglelineTextBox.Keys.Typing")` covering ordinary character entry driven by `OnCharInput`, including tabs and replacement typing.
### what to be done
- Implement test cases:
  - `TEST_CASE(L"Typing inserts characters at the caret")` - move caret across the text with navigation keys, call `TypeString`, and verify characters land at the caret without disturbing surrounding text.
  - `TEST_CASE(L"Typing over a selection replaces it")` - select a range (e.g., via `Shift+Arrow` or `Ctrl+A`) and type to ensure the selection is overwritten and caret collapses to the insertion end.
  - `TEST_CASE(L"Tab key inserts literal tab while control characters are ignored")` - call `TypeString(L"\t")` and assert a `\t` appears, then send keys such as `Esc` to confirm they do not insert printable content.
- After each typing sequence, assert the widget text, the selected text span (if any), and the caret begin/end so the new requirement is satisfied.
### how to test it
- Execute the category via `UnitTest` and verify `GetText()` values, explicit selected range queries, and caret endpoints match expected values after each typing scenario.
### rationale
- Typing is the core scenario for end users; validating insertion, replacement, and acceptance of Tab (enabled via `SetAcceptTabInput`) ensures the control stays predictable for plain-text editing.

## TASK No.5: Add clipboard shortcut tests for GuiSinglelineTextBox
Add `TEST_CATEGORY(L"GuiSinglelineTextBox.Keys.Clipboard")` to cover `Ctrl+A/C/X/V` behaviors and plain-text enforcement during paste.
### what to be done
- Implement test cases:
  - `TEST_CASE(L"Ctrl+A selects all and Ctrl+C copies plain text")` - populate the text box, press `Ctrl+A`, ensure caret bounds cover full range, execute `Ctrl+C`, and read the fake clipboard to confirm text matches exactly.
  - `TEST_CASE(L"Ctrl+X cuts selection and updates clipboard")` - select a substring, press `Ctrl+X`, verify text shrinks and clipboard now holds the removed slice.
  - `TEST_CASE(L"Ctrl+V flattens multi-line or styled clipboard data")` - preload the clipboard with text containing CR/LF and with a styled `DocumentModel`, paste it, and assert the inserted text is single-line plain text with styles ignored.
- Each clipboard shortcut test must validate the document text, the resulting selection span (for select-all/cut/paste), and the caret positions along with clipboard expectations.
### how to test it
- Use the unit tests to assert text-box content, selected range, caret bounds, and clipboard values after each shortcut.
### rationale
- The control configures `pasteAsPlainText` and single-line formatting; verifying clipboard shortcuts honors those guarantees prevents regressions where multi-line or styled content slips through.

## TASK No.6: Add undo/redo shortcut tests for GuiSinglelineTextBox
Introduce `TEST_CATEGORY(L"GuiSinglelineTextBox.Keys.UndoRedo")` that drives `Ctrl+Z`/`Ctrl+Y` through the shortcut manager to ensure editing history works with keyboard-only workflows.
### what to be done
- Implement test cases:
  - `TEST_CASE(L"Ctrl+Z undoes recent typing and deletion")` - perform a sequence of typing and deletion steps, press `Ctrl+Z` repeatedly, and assert text/selection/`GetModified()` revert accordingly.
  - `TEST_CASE(L"Ctrl+Y redoes operations after undo chain")` - after undoing multiple steps, press `Ctrl+Y` to ensure redo reapplies edits in order.
- Throughout undo/redo sequences, verify the text snapshot, the selection (or lack thereof), and the caret begin/end after each shortcut so history restores the full editing state.
### how to test it
- Run the category to verify text state transitions, selection/caret restoration, and `GetModified()` reflect undo/redo actions, and ensure no unexpected side effects remain after closing the window.
### rationale
- Undo/redo shortcuts are essential for editors; covering them ensures the internal undo stack and shortcut routing continue to behave correctly for single-line text boxes.

# Impact to the Knowledge Base
## GacUI
No updates are required; all planned work is confined to unit tests and exercises existing documented behavior.

# !!!FINISHED!!!
