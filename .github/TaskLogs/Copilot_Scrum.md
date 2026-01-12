# !!!SCRUM!!!

# DESIGN REQUEST

# Problem
In #file:TestControls_Editor_Key.cpp  there is a function `RunTextBoxKeyTestCases`. When the last parameter is true, it runs extra test cases. I would like to propose 2 tasks:

1. Remove the parameter. Move those extra test cases into a new function `RunTextBoxKeyTestCases_Singleline`. Log path of the test case should not change, but append " (Singleline)" after test category names.
2. Add a `RunTextBoxKeyTestCases_Multiline` for `GuiMultilineTextBox`. This text box control accepts ENTER or CTRL+ENTER, they do the same thing, which inserts CRLF to the text. And it doesn't flatten any pasted or input text with CRLF.
3. Add a `RunTextBoxKeyTestCases_MultiParagraphs` for all other 3 text boxes. This text box controls accepts ENTER (or double CRLF in input text) to create a new paragraph, or CTRL+ENTER (or single CRLF in input text) to create a new line in a paragraph.

In new test cases, you should not use GetText to assert the content of the control, you should first check how many parametes in GetDocument, and then check each parameter's content by calling GetTextForReading. Design these new test cases according to what is included in `RunTextBoxKeyTestCases_Singleline`.

# UPDATES

# TASKS

- [ ] TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline
- [ ] TASK No.2: Add GuiMultilineTextBox Enter/CRLF tests with GetDocument assertions
- [ ] TASK No.3: Add multi-paragraph Enter/CRLF semantics tests with GetDocument assertions

## TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline

Refactor the shared key-test scaffold in the editor unit tests so that singleline-only behaviors are isolated in a dedicated helper, while keeping log paths stable.

This task removes the `singleline` flag from `RunTextBoxKeyTestCases` and moves all currently-conditional tests into a new `RunTextBoxKeyTestCases_Singleline` helper.

### what to be done

- Update `RunTextBoxKeyTestCases` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp to remove the `singleline` parameter.
- Introduce `RunTextBoxKeyTestCases_Singleline` and move all tests that were guarded by `if (singleline)` into that function.
- Update `TEST_FILE` to:
	- Call `RunTextBoxKeyTestCases_Singleline` only for `GuiSinglelineTextBox`.
	- Append " (Singleline)" to the outer `TEST_CATEGORY` name for `GuiSinglelineTextBox`, while keeping `controlName` used in log paths unchanged so the log path stays stable.
- Ensure the moved test cases keep the same per-test log identity string (the `Controls/Editor/<controlName>/Key/...` path) as before.
- Fix any call-site or template-parameter mismatches exposed by the refactor so the file still compiles and the tests still find the correct `<... ref.Name="textBox"/>` control instance.

### rationale

- The `singleline` flag currently couples unrelated behaviors into one monolithic helper and makes it hard to add multiline/paragraph-specific coverage cleanly.
- Preserving the log path while changing category names is important for existing log consumers and for comparing historical test runs.
- Making singleline-only behaviors explicit creates a stable baseline for the next two tasks to build multiline and multi-paragraph semantics without adding more conditionals.

## TASK No.2: Add GuiMultilineTextBox Enter/CRLF tests with GetDocument assertions

Add a dedicated `RunTextBoxKeyTestCases_Multiline` helper for `GuiMultilineTextBox` to cover multiline-specific CRLF handling:

- ENTER and CTRL+ENTER behave the same.
- Both insert CRLF into the text.
- Pasted or typed text containing CRLF is not flattened.

All new assertions for content must use `GetDocument()` and `GetTextForReading()` rather than `GetText()`.

### what to be done

- Implement `RunTextBoxKeyTestCases_Multiline` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp and call it only for `GuiMultilineTextBox`.
- Add new test cases for multiline behaviors, modeled after the structure and coverage approach used by `RunTextBoxKeyTestCases_Singleline` (same style of setup frames, key injection, and validation).
- In the new tests:
	- Validate the document structure by checking the paragraph count in the `DocumentModel` returned by `GetDocument()`.
	- Validate each paragraph’s content using `DocumentParagraphRun::GetTextForReading()`.
- Include coverage for:
	- `KEY_RETURN` inserts a CRLF (and does not create a new paragraph model if multiline uses a single paragraph).
	- `Ctrl+KEY_RETURN` matches `KEY_RETURN`.
	- `Ctrl+V` pastes text containing CRLF without flattening.
	- `TypeString` with CRLF (or sequences including CRLF) preserves the CRLF semantics (not flattened).

### rationale

- The multiline text box has explicitly different user input semantics from singleline; mixing these in the same helper would reintroduce the conditional complexity we’re removing.
- `GuiDocumentCommonInterface` exposes the underlying `DocumentModel` (`GetDocument()`), and `DocumentModel` stores paragraphs as a list of `DocumentParagraphRun` objects; using `GetTextForReading()` verifies the model content directly rather than relying on a potentially-normalized `GetText()` string.
- Testing both keyboard ENTER and clipboard/text input ensures all user input entry points share the same formatting rules.

## TASK No.3: Add multi-paragraph Enter/CRLF semantics tests with GetDocument assertions

Add a dedicated `RunTextBoxKeyTestCases_MultiParagraphs` helper for the other three text boxes (the non-`GuiMultilineTextBox` controls) to verify paragraph vs line semantics:

- ENTER (or double CRLF in input text) creates a new paragraph.
- CTRL+ENTER (or single CRLF in input text) creates a new line inside the current paragraph.

All new assertions for content must use `GetDocument()` and `GetTextForReading()` rather than `GetText()`.

### what to be done

- Implement `RunTextBoxKeyTestCases_MultiParagraphs` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp.
- Call `RunTextBoxKeyTestCases_MultiParagraphs` for the remaining three editor controls (all non-singleline and non-multiline in this test file).
- Add new test cases that verify:
	- ENTER creates a new paragraph (document paragraph count increases, and each paragraph’s `GetTextForReading()` matches expected content).
	- CTRL+ENTER inserts a line break inside a paragraph without splitting into multiple paragraphs.
	- Typing/pasting with single CRLF results in a line break within a paragraph.
	- Typing/pasting with double CRLF results in a paragraph break (two paragraphs).
- Keep new test scaffolding consistent with the frame-based testing guidance (each `OnNextIdleFrame` corresponds to one rendered frame and should contain the input + assertions for that phase), per the knowledge base’s frame-based testing guidance.

### rationale

- These controls are explicitly different from `GuiMultilineTextBox`: they model a multi-paragraph rich document rather than a simple multiline plain-text editor.
- `DocumentModel::paragraphs` is the authoritative representation of paragraph structure; validating paragraph count + per-paragraph text via `GetTextForReading()` ensures the tests match the intended semantics rather than a flattened serialization.
- Separating this helper keeps the shared baseline tests stable and makes paragraph rules explicit and maintainable.

# Impact to the Knowledge Base

## GacUI

- Consider adding a short knowledge base topic (or extending an existing unit testing topic) explaining how to assert editor control content via `GuiDocumentCommonInterface::GetDocument()` and per-paragraph `DocumentParagraphRun::GetTextForReading()`, especially for distinguishing singleline, multiline, and multi-paragraph controls.

# !!!FINISHED!!!
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

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): structure each interaction as an input frame followed by a verification frame.
  - Input frame: issue one or more inputs (`TypeString`, `KeyPress`, `_KeyDown`, `_KeyUp`, etc) and do not assert.
  - Verification frame: perform all assertions for the result *and* ensure the callback triggers a UI update (e.g. `window->SetText(textBox->GetText());`) so the harness accepts the frame.
  - Schedule another frame (except the last one) only when the current frame issues an input that will change UI in the next frame.
  - Keep frame titles short, and let titles describe what happened in the previous frame.
  - Keep the last frame minimal and hide-only (`window->Hide();`).
  - Build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Typing_<CASE>")`.
- When asserting that a `WString` contains no line breaks, use `WString::IndexOf(L'\r')` and `WString::IndexOf(L'\n')` (character literals) instead of `L"\r"` / `L"\n"` to match the correct overload.
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
- Follow the latest frame learnings (from TASK No.8 / Typing) when adding new smoke-test invocations: merge focus + input when possible; avoid adding `window->SetText(...)` just to force a render; if a step doesn’t naturally trigger a render, merge it into a frame that performs real input (or restructure so each intermediate frame does something that will change UI); keep the last frame hide-only when practical, but merge it when it avoids artificial UI changes.
- Ensure the last 3 types have `EditMode="Editable"` set in the XML so the existing typing-based smoke tests are meaningful.
- Keep log paths consistent with the existing pattern in `RunTextBoxSmokeTest`, so the output folders differentiate control types cleanly (e.g. `Controls/Editor/<ControlName>/Basic` and `.../Typing`).

### rationale

- These editor controls share a large portion of `GuiDocumentCommonInterface` behavior; running the same smoke tests across them quickly detects regressions in shared code paths.
- Doing this after the key-test categories are planned keeps scope contained: it reuses a proven smoke-test helper and adds only the minimum XML/control scaffolding needed.

# Impact to the Knowledge Base

## None

# LEARNINGS

- Treat XML virtual types as first-class test targets: it is OK (and sometimes preferred) to run smoke tests with the runtime C++ type (e.g. `GuiDocumentLabel`) while keeping the category/log folder name aligned with the XML concept (e.g. `GuiDocumentTextBox`).
- Prefer deterministic caret positioning via key operations (e.g. `Ctrl+Home`) rather than changing layout just to make a mouse click land reliably.
- When embedding XML in C++ raw string literals for tests, use 2 spaces for indentation.

# !!!FINISHED!!!

