# !!!SCRUM!!!

# DESIGN REQUEST

# Problem

There are 4 controls inheriting from `GuiDocumentCommonInterface`. There differences are just twisting some rendering and user input. They share the same core design and implementation. At the moment we are focusing on designing test cases for `GuiSinglelineTextBox`.

In  [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp) there are already some basic test cases. But I would like you to add more test cases to the currently empty  `TestControls_Editor_GuiSinglelineTextBox_Key.cpp. It stores test cases for keyboard operations and typing.

You need to analyse how many keyboard operations does `GuiSinglelineTextBox` supports. Categorize all of them and design multiple test cases covering all of them. Keyboard operations including key strokes combinations, predefined shortcut like ctrl+c etc, and typing. 

`GuiSinglelineTextBox` is a text box for single line of plain text. Although it has rich text functions in its base class, but the constructor already configures all of them to perform plain text operations only. For example, changing font or text color will be ignores, pasting styled text will not apply styles, CrLf will be removed, etc.

You need to do analysis right now, get the result, and categorize all keyboard operations test cases into multiple TEST_CATEGORY of multiple TEST_CASE. Each TEST_CATEGORY becomes a new task following the first one.  Analysing itself should not be a task in the scrum document because it does not edit source code.

# UPDATES

## UPDATE

I would like to update the plan for sharing test cases to other editor control. In [TestControls_Editor_GuiSinglelineTextBox_Key.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp) , the file structure should look like

static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName){...}

TEST_FILE
{
    const auto resourceSinglelineTextBox = LR"GacUISrc(...)GacUISrc";
    TEST_CATEGORY(L"GuiSinglelineTextBox")
    {
        RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
    }
}

The first argument will be the XML description of the UI under test. The second will be part of the path, so that every test cases are logging using:
WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/CATEGORY_CASE");, CATEGORY and CASE will be replaced with the actual name.

In the `RunSinglelineTextBox` function, code like
auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
should be updated to
auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
since my future plan is to share these test cases to all GuiDocumentLabel subclasses. But testing other controls are not included in the current document.

As there will be a big TEST_CATEGORY in TEST_FILE, so your proposed
TEST_CATEGORY(L"GuiSinglelineTextBox (Key) - Navigation")
could be simplified to
TEST_CATEGORY(L"Navigation"), applies to all other proposed category names.

After fixing existing tasks, add one task that before all other task, to fix [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp)  following the above way.

## UPDATE

Firstly, `- Follow the established scaffold from TASK No.2 xxx` lines are added to all unfinished tasks. I would like to clarify that, performing change and assertion should be in one single frame. A frame can only be ended (except for the last one) when any UI element would be changed. A frame title reflects what has been done in the last frame (except for the first one). These are in the knowledge base. And I would like to keep the last frame as simple as possible, so that I can view generated logs easily.

Secondly, please insert one task before all unfinished tasks. That's said to hijack the current time reading. There is a `TooltipTimer` in [TestApplication_Tooltip.cpp](Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp) . I would like you to move the class to [TestControls.h](Test/GacUISrc/UnitTest/TestControls.h) so it could be shared with text box test cases. And there is also one `extern` in that cpp file, delete it as it is not needed anymore, I forgot to do that. That cpp file demoed how to use `TooltipTimer`. Use it in every test cases in [TestControls_Editor_GuiSinglelineTextBox_Key.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp)  and [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp) 

## UPDATE

I made manual changes:
- TestControls_Editor_GuiSinglelineTextBox.cpp has been renamed to TestControls_Editor.cpp
- Two test case functions in #file:TestControls_Editor.cpp  and #file:TestControls_Editor_GuiSinglelineTextBox_Key.cpp has been changed from static to template function, preferring FindObjectByName<TTextBox> other than the previous GuiDocumentLabel.

I would like you to do:
- Update the scrum document to reflect the change in unfinished tasks.
- Add a new task at the end, to update #file:TestControls_Editor.cpp to run `RunTextBoxSmokeTest` for 4 other text box types:
  - <MultilineTextBox/>: GuiMultilineTextBox
  - <DocumentTextBox/>: GuiDocumentLabel
  - <DocumentLabel/>: GuiDocumentLabel
  - <DocumentViewer/>: GuiDocumetViewer
  - Need to set `EditMode="Editable"` for the last 3 text box types

# TASKS

- [x] TASK No.1: Refactor existing GuiSinglelineTextBox tests for reuse
- [x] TASK No.2: Create GuiSinglelineTextBox key test scaffold
- [x] TASK No.3: Hijack current-time reading in editor unit tests
- [x] TASK No.4: Add TEST_CATEGORY for navigation and selection keys
- [x] TASK No.5: Add TEST_CATEGORY for deletion and Enter keys
- [x] TASK No.6: Add TEST_CATEGORY for clipboard shortcuts
- [ ] TASK No.7: Add TEST_CATEGORY for undo/redo shortcuts
- [ ] TASK No.8: Add TEST_CATEGORY for typing and char-input filtering
- [ ] TASK No.9: Run smoke tests for more editor text boxes

## TASK No.1: Refactor existing GuiSinglelineTextBox tests for reuse

Restructure `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` to follow the same “shared test cases + outer control category” pattern as the key test file, so the same test cases can be reused for other `GuiDocumentLabel` subclasses in the future.

### what to be done

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`.
- Move existing `TEST_CASE(L"Basic")` and `TEST_CASE(L"Typing")` into `RunSinglelineTextBoxTestCases` (keeping their behavior the same).
- Introduce (or update) a shared `RunSinglelineTextBox` helper so each test case logs to `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/CATEGORY_CASE")` (CATEGORY and CASE replaced by actual names, consistent with the key test plan but without the `/Key/` segment).
- In `RunSinglelineTextBox`, change `FindObjectByName<GuiSinglelineTextBox>(window, L"textBox")` to `FindObjectByName<GuiDocumentLabel>(window, L"textBox")`.
- Update `TEST_FILE` to contain only the resource XML + one outer `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resource, WString::Unmanaged(L"GuiSinglelineTextBox")); }`.

### rationale

- This keeps non-key and key tests consistent and makes it straightforward to reuse the same test cases for other editor controls inheriting from `GuiDocumentLabel` later.
- Switching lookup to `GuiDocumentLabel` is a prerequisite for sharing without rewriting all test steps.

## TASK No.2: Create GuiSinglelineTextBox key test scaffold

Create a shared test window/resource and minimal helper flow in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` so later categories can focus on keyboard behaviors without repeating boilerplate, and so the same test cases can be reused by other `GuiDocumentLabel` subclasses in the future.

### what to be done

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` to `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Implement `TEST_FILE` using the structure:
  - `const auto resourceSinglelineTextBox = LR"GacUISrc( ... )GacUISrc";`
  - An outer `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox")); }`.
- Add (or update) a shared `RunSinglelineTextBox` helper that:
  - Loads the UI from `resource` (XML) and starts the test using `GacUIUnitTest_StartFast_WithResourceAsText`.
  - Builds per-test log paths as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/CATEGORY_CASE")` (CATEGORY and CASE replaced by actual names).
  - Finds the target control by `FindObjectByName<GuiDocumentLabel>(window, L"textBox")`.
- Ensure each test case establishes a consistent initial state (focus + known text like `L"0123456789"`) before exercising key operations via `UnitTestRemoteProtocol` (`KeyPress`, `TypeString`).

### rationale

- All subsequent tasks add `TEST_CATEGORY` blocks in `RunSinglelineTextBoxTestCases`; a shared scaffold keeps those tasks small and consistent.
- Using an index-friendly initial text makes it easy to assert caret/selection changes precisely.

## TASK No.3: Hijack current-time reading in editor unit tests

Move the existing `TooltipTimer` (feature injection for `vl::IDateTimeImpl`) to a shared test header and use it in all single-line textbox unit tests, to ensure time-dependent behaviors (e.g. cursor blink / delayed behaviors) do not read the real system clock and cause flaky logs.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule): in `UnitTestRemoteProtocol::OnNextIdleFrame`, treat each rendered frame as "the UI change done in the previous callback + the assertions done in the current callback"; do not end a callback (except the last one) unless it performs an input that will change UI in the next frame; choose frame titles that describe what happened in the previous callback; keep the final callback minimal so the last logged frame is easy to inspect.
- Move `TooltipTimer` from `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` to `Test/GacUISrc/UnitTest/TestControls.h` so it can be shared by other test files.
- Remove the unnecessary `extern` declaration from `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` after moving `TooltipTimer`.
- Update `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` to include and use the shared `TooltipTimer` from `TestControls.h` (no duplicated definition).
- Use `TooltipTimer` in every `TEST_CASE` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` and `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` (construct a local `TooltipTimer` at the beginning of each test case so time is hijacked for the whole test).

### rationale

- `UnitTestRemoteProtocol` logs and assertions are frame-based; making time deterministic reduces flakiness and makes frame-by-frame logs easier to compare.
- Centralizing `TooltipTimer` in `TestControls.h` avoids copy/paste and keeps time-hijacking consistent across editor test files.

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

## TASK No.5: Add TEST_CATEGORY for deletion and Enter keys

Add test cases for editing keys handled in `ProcessKey`: Backspace, Delete, Enter, and Ctrl+Enter, including behavior with and without a selection.

In `GuiSinglelineTextBox`, Enter is processed but line breaks are flattened by configuration (`paragraphMode=Singleline`, `spaceForFlattenedLineBreak=false`), so no newline should remain in the resulting text.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones; keep the final callback minimal so the last logged frame is easy to inspect; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Deletion_<CASE>")`.
- Add `TEST_CATEGORY(L"Deletion")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s:
  - `Backspace_DeletesPreviousChar`: set caret after a known character; press `KEY_BACK`; assert one character removed and caret moved appropriately.
  - `Delete_DeletesNextChar`: set caret before a known character; press `KEY_DELETE`; assert one character removed.
  - `Backspace_DeletesSelection` and `Delete_DeletesSelection`: create a selection via `Shift+Arrow` or `SelectAll()`, then press Backspace/Delete; assert selection removed.
  - `BackspaceAtStart_NoChange` and `DeleteAtEnd_NoChange`: boundary behavior should not change text.
  - `Enter_DoesNotInsertNewline_ButCanDeleteSelection`: with no selection, press `KEY_RETURN` and assert text unchanged; with a selection, press `KEY_RETURN` and assert selection removed.
  - `CtrlEnter_DoesNotInsertNewline_ButCanDeleteSelection`: same as above but using `KeyPress(KEY_RETURN, ctrl=true, ...)`.

### rationale

- Backspace/Delete/Enter are the primary editing keys beyond typing; they have subtle selection-dependent behavior.
- Single-line newline-flattening is a key "plain text" guarantee of `GuiSinglelineTextBox` and needs coverage.

## TASK No.6: Add TEST_CATEGORY for clipboard shortcuts

Add test cases for Ctrl+C/Ctrl+X/Ctrl+V, using the fake clipboard service in unit tests and validating single-line plain-text normalization on paste (CR/LF removed, styles ignored).

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones; keep the final callback minimal so the last logged frame is easy to inspect; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`.
- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones (frame titles describe what happened in the previous frame); if an input is issued, add one more final frame whose only job is `window->Hide();` so the last logged frame stays minimal; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`.
- Add `TEST_CATEGORY(L"Clipboard")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Keep adding cases inside the existing template-based shared test function (i.e. continue using `FindObjectByName<TTextBox>` in test code, and do not hardcode `GuiDocumentLabel`).
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

## TASK No.7: Add TEST_CATEGORY for undo/redo shortcuts

Add test cases for Ctrl+Z/Ctrl+Y and validate undo/redo behavior across representative edits (typing, deletion, paste) in a single-line textbox.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones (frame titles describe what happened in the previous frame); if an input is issued, add one more final frame whose only job is `window->Hide();` so the last logged frame stays minimal; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/UndoRedo_<CASE>")`.
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

## TASK No.8: Add TEST_CATEGORY for typing and char-input filtering

Add test cases for the `OnCharInput` path: normal typing, tab input, control-modified typing being ignored, and newline characters being flattened/ignored as appropriate for single-line text.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones (frame titles describe what happened in the previous frame); if an input is issued, add one more final frame whose only job is `window->Hide();` so the last logged frame stays minimal; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Typing_<CASE>")`.
- Add `TEST_CATEGORY(L"Typing")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Keep adding cases inside the existing template-based shared test function (i.e. continue using `FindObjectByName<TTextBox>` in test code, and do not hardcode `GuiDocumentLabel`).
- Add multiple `TEST_CASE`s:
  - `TypeString_InsertsPlainText`: use `protocol->TypeString(L\"...\")` and assert text updates.
  - `TypeString_InsertsTab_WhenAcceptTabInput`: type `L\"\\t\"` and assert a tab character appears in text.
  - `TypeString_IgnoresWhenCtrlPressed`: use `protocol->_KeyDown(KEY_CONTROL)` + `TypeString(...)` + `_KeyUp(KEY_CONTROL)` and assert no text change.
  - `TypeString_NewlineCharsFlattened`: type a string containing `\\n` and assert the resulting text contains no line breaks (flattened by formatting); also verify `\\r` (KEY_RETURN) is ignored by char input.
  - `TypeString_ReplacesSelection`: create a selection (e.g. Shift+Arrow or `SelectAll()`), then `TypeString(...)` and assert replacement.

### rationale

- `GuiSinglelineTextBox` is configured to accept only plain text; char-input filtering + flattening is the core guarantee.
- Explicitly testing tab and ctrl-modified typing prevents regressions in `OnCharInput` gating logic.

## TASK No.9: Run smoke tests for more editor text boxes

Extend the existing editor smoke tests in #file:TestControls_Editor.cpp so the same `RunTextBoxSmokeTest<TTextBox>(resource, controlName)` flow (Basic + Typing) runs against the other `GuiDocumentCommonInterface`-based editor controls.

This task is only about expanding the smoke-test coverage (reusing the existing shared test-case helper) and preparing the XML resources for each control type so failures can be compared via consistent log paths.

### what to be done

- In #file:TestControls_Editor.cpp, add additional resource XML strings (or a single combined resource that can host one control at a time) and add outer `TEST_CATEGORY` blocks to call `RunTextBoxSmokeTest` for:
  - `<MultilineTextBox/>` mapped to `GuiMultilineTextBox`.
  - `<DocumentTextBox/>` mapped to `GuiDocumentLabel`.
  - `<DocumentLabel/>` mapped to `GuiDocumentLabel`.
  - `<DocumentViewer/>` mapped to `GuiDocumetViewer`.
- Ensure the last 3 types have `EditMode="Editable"` set in the XML so the existing typing-based smoke tests are meaningful.
- Keep log paths consistent with the existing pattern in `RunTextBoxSmokeTest`, so the output folders differentiate control types cleanly (e.g. `Controls/Editor/<ControlName>/Basic` and `.../Typing`).

### rationale

- These editor controls share a large portion of `GuiDocumentCommonInterface` behavior; running the same smoke tests across them quickly detects regressions in shared code paths.
- Doing this after the key-test categories are planned keeps scope contained: it reuses a proven smoke-test helper and adds only the minimum XML/control scaffolding needed.

# Impact to the Knowledge Base

## None

# !!!FINISHED!!!

