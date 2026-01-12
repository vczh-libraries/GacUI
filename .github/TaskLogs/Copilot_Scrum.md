# !!!SCRUM!!!

# DESIGN REQUEST

# Problem
In #file:TestControls_Editor_Key.cpp  there is a function `RunTextBoxKeyTestCases`. When the last parameter is true, it runs extra test cases. I would like to propose 2 tasks:

1. Remove the parameter. Move those extra test cases into a new function `RunTextBoxKeyTestCases_Singleline`. Log path of the test case should not change, but append " (Singleline)" after test category names.
2. Add a `RunTextBoxKeyTestCases_Multiline` for `GuiMultilineTextBox`. This text box control accepts ENTER or CTRL+ENTER, they do the same thing, which inserts CRLF to the text. And it doesn't flatten any pasted or input text with CRLF.
3. Add a `RunTextBoxKeyTestCases_MultiParagraphs` for all other 3 text boxes. This text box controls accepts ENTER (or double CRLF in input text) to create a new paragraph, or CTRL+ENTER (or single CRLF in input text) to create a new line in a paragraph.

In new test cases, you should not use GetText to assert the content of the control, you should first check how many parametes in GetDocument, and then check each parameter's content by calling GetTextForReading. Design these new test cases according to what is included in `RunTextBoxKeyTestCases_Singleline`.

# UPDATES

## UPDATE

I would like to propose two tasks, to add more cursor/caret navigation test by keyboard into `RunTextBoxKeyTestCases_Multiline` and `RunTextBoxKeyTestCases_MultiParagraphs`. To test that:
- Pressing left/right from the beginning/ending of line/paragrpah jumpst to the end/beginning of previous/next line/paragrpah.
- Pressing top/down or page up/page down move caret vertically.
- Pressing HOME/END jumps to the beginning/ending of the current line. If the caret is already in that position therefore causing the caret not actually moved, it jumps to the beginning/ending of the current paragraph. If the caret is still not moving, jumps to the beginning/ending of the whole document. Pressing CTRL+HOME/CTRL+END jumps to the beginning/ending of the whole document. This part may not been fully implemented yet, fix the implementation in #file:GuiTextCommonInterface.cpp when necessary
- Any other navigation operations you found that could be affected by GuiDocumentParagraphMode::Multiline/Paragraph.

In order to demo actual caret moving between frames, you can type a character after moving, so that it will cause UI to change and the position of the caret will be logged in an visual way, therefore frames are well organized as well.

 Some cases may be affected by line wrapping. But I don't want to cover this case right now, please make sure each line or paragraph is short enough so that it doesn't trigger line wrapping. You can do some guess because according to the implementation of #file:GuiUnitTestProtocol_Rendering.cpp , each character only takes 0.6*font size in width. Keep each line taking no more than half of the actual window width. The reason behind this is that, line wrapping is implemented in #file:GuiUnitTestProtocol_Rendering.cpp , this is a stub implementation only for unit test, which is not the actual layout algorithm. I want new test cases mentioned above not to touch this part too much. But you can't actually avoid it because it also implements how caret moves. So when you see test case fails, you need to also take a look into this file.

 Please be awared that, the two functions mentioned above are for two different paragraph mode, so behavior might be a little bit difference, refer details in the early part of the scrum document.

## UPDATE

I would like you to adjust and switch part of content between Task 2 and Task 3, Task 2 will focus on ENTER and CRLF behaviors, And Task 3 will focus on caret navigation. And then split Task 4 into Task 4 and Task 5 following above.

## UPDATE

In Task 4 there are test cases invokcing copy/paste of multiple lines of text. Please make sure this also included in Task 2. And be awared that, in multiline mode (task 2), CRLF separates lines. In paragraph mode (task 4), double CRLF separates paragraphs and inside each paragraph CRLF separates lines. So when 3 consecutive CRLF happens, like a\r\n\r\n\r\nb, the first paragrpah consist one line a, the second paragraph consist of one emtpy line and one line b

## UPDATE

To clarify, `RunTextBoxKeyTestCases_Multiline` only applies to GuiMultilineTextBox because this is the only control that has Multiline paragraph mode. And `RunTextBoxKeyTestCases_MultiParagraph` only applies to the other 3 because those are in Paragraph mode. Control can't switch mode until created, so we only test their default options mentioned here.

## UPDATE

More clarification, since Task 2/3 is for `RunTextBoxKeyTestCases_Multiline`, so these 2 task should only focus on GuiMultilineTextBox, and update the GuiMultilineTextBox test category to call this function. Task 4/5 is for RunTextBoxKeyTestCases_MultiParagraph with other 3 controls. I see these two functions are mentioned in all 4 tasks, please clean the content of the task.

## UPDATE

You will still need to add test cases in Task 2/3, otherwise the `RunTextBoxKeyTestCases_Multiline` is empty.

# TASKS

- [x] TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline
- [x] TASK No.2: Fix ENTER/CRLF semantics for multiline mode (GuiMultilineTextBox)
- [ ] TASK No.3: Fix caret navigation semantics for multiline mode (GuiMultilineTextBox)
- [ ] TASK No.4: Add Enter/CRLF tests with GetDocument assertions (Paragraph mode controls)
- [ ] TASK No.5: Add keyboard caret navigation tests (Paragraph mode controls)

## TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline

Refactor the shared key-test scaffold in the editor unit tests so that singleline-only behaviors are isolated in a dedicated helper, while keeping log paths stable.

This task removes the `singleline` flag from `RunTextBoxKeyTestCases` and moves all currently-conditional tests into a new `RunTextBoxKeyTestCases_Singleline` helper.

### what to be done

- Update `RunTextBoxKeyTestCases` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp to remove the `singleline` parameter.
- Introduce `RunTextBoxKeyTestCases_Singleline` and move all tests that were guarded by `if (singleline)` into that function.
- Update `TEST_FILE` to:
	- Call `RunTextBoxKeyTestCases_Singleline` only for `GuiSinglelineTextBox`.
  - Keep the outer `TEST_CATEGORY(L"GuiSinglelineTextBox")` unchanged; in `RunTextBoxKeyTestCases_Singleline`, append " (Singleline)" to test category names (e.g. `Clipboard (Singleline)`).
- Ensure the moved test cases keep the same per-test log identity string (the `Controls/Editor/<controlName>/Key/...` path) as before.
- Fix any call-site or template-parameter mismatches exposed by the refactor so the file still compiles and the tests still find the correct `<... ref.Name="textBox"/>` control instance.

### rationale

- The `singleline` flag currently couples unrelated behaviors into one monolithic helper and makes it hard to add multiline/paragraph-specific coverage cleanly.
- Preserving the log path while changing category names is important for existing log consumers and for comparing historical test runs.
- Making singleline-only behaviors explicit creates a stable baseline for the next two tasks to build multiline and multi-paragraph semantics without adding more conditionals.

## TASK No.2: Fix ENTER/CRLF semantics for multiline mode (GuiMultilineTextBox)

Add multiline-mode ENTER/CRLF coverage into `RunTextBoxKeyTestCases_Multiline` and fix implementation when necessary so `GuiMultilineTextBox` behaves consistently with the expected semantics described in the DESIGN REQUEST.

Per the latest UPDATE, this task must include real test cases so `RunTextBoxKeyTestCases_Multiline` is not empty.

### what to be done

- Implement (or fill in) `RunTextBoxKeyTestCases_Multiline` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp for `GuiMultilineTextBox`.
	- Update the `GuiMultilineTextBox` test category in the same file to call `RunTextBoxKeyTestCases_Multiline`.
- Add ENTER/CRLF test cases (modeled after the structure and assertion approach used by `RunTextBoxKeyTestCases_Singleline`) that validate multiline mode semantics:
	- `ENTER` and `CTRL+ENTER` do the same thing.
	- Both insert a CRLF into the text model.
	- Any pasted or typed text containing CRLF is preserved and not flattened.
		- CRLF separates lines (including multi-line clipboard paste and multi-line typing input).
	- Include copy/paste cases that involve multiple lines of text (as clarified in UPDATE).
- For every new assertion about text/document content:
	- Do not call `GetText()`.
	- Validate paragraph count in the `DocumentModel` returned by `GetDocument()`.
	- Validate each paragraph’s content using `DocumentParagraphRun::GetTextForReading()`.
- If any new multiline-mode test fails due to behavior mismatch, fix the underlying implementation in #file:GuiTextCommonInterface.cpp (or the document-editing code it calls) until the tests pass.

### rationale

- This task is scoped to `GuiDocumentParagraphMode::Multiline` only, matching the clarified design (multiline helper applies only to `GuiMultilineTextBox`).
- Adding the ENTER/CRLF test cases here prevents `RunTextBoxKeyTestCases_Multiline` from staying empty and immediately locks down the editing semantics differences for multiline mode.
- Validating via `GetDocument()` + `GetTextForReading()` ensures tests check the underlying model rather than any normalized string produced by `GetText()`.

## TASK No.3: Fix caret navigation semantics for multiline mode (GuiMultilineTextBox)

Add multiline-mode caret-navigation coverage into `RunTextBoxKeyTestCases_Multiline` and fix implementation when necessary so that keyboard navigation works consistently for `GuiDocumentParagraphMode::Multiline` in `GuiMultilineTextBox`.

Per the latest UPDATE, this task must include real test cases so `RunTextBoxKeyTestCases_Multiline` is not empty beyond ENTER/CRLF.

### what to be done


- Extend `RunTextBoxKeyTestCases_Multiline` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp with keyboard caret-navigation test cases that cover:
	- `LEFT` / `RIGHT` boundary movement across line boundaries in multiline mode.
	- `UP` / `DOWN` and `PAGE UP` / `PAGE DOWN` vertical movement.
	- `HOME` / `END` behavior per the UPDATE (line → paragraph → document), and `CTRL+HOME` / `CTRL+END` (document).
	- Any other navigation operations that are mode-sensitive for `GuiDocumentParagraphMode::Multiline` as discovered from existing tests and implementation.
- In order to make caret movement visible in logs and to ensure each frame produces UI change:
	- After each navigation key (or small sequence), type a character (or insert a marker) so the caret position becomes visually reflected and the frame is valid.
	- When a step already causes a visible UI change (e.g. editing text), it is fine to put `window->Hide();` in a final dedicated frame to log the post-action UI state cleanly.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to avoid wrapping in the unit-test renderer, so the tests primarily validate caret navigation rules rather than the stub wrapping algorithm.

- Review keyboard navigation handling in #file:GuiTextCommonInterface.cpp (especially `KEY_HOME`, `KEY_END`, and boundary behaviors for `KEY_LEFT`/`KEY_RIGHT`).
- Ensure the requested HOME/END semantics:
	- `HOME` / `END` first targets the beginning/ending of the current line.
	- If that would not move the caret (already at that position), it should target the beginning/ending of the current paragraph.
	- If that still would not move the caret, it should target the beginning/ending of the whole document.
	- `CTRL+HOME` / `CTRL+END` always targets the beginning/ending of the whole document.
	- Ensure `LEFT`/`RIGHT` boundary movement matches expectations for multiline mode (line-level boundaries).
- If the new tests reveal mismatches, fix the implementation in #file:GuiTextCommonInterface.cpp until they pass.
- If failures are caused by the unit-test rendering stub (caret bounds / caret movement), adjust #file:GuiUnitTestProtocol_Rendering.cpp minimally so caret navigation in tests reflects the intended behavior, while keeping line-wrapping out of scope.

### rationale

- Scoping this task to multiline mode matches the clarified design: Task 2/3 focus on `GuiMultilineTextBox` and `RunTextBoxKeyTestCases_Multiline`.
- Adding navigation test cases here ensures `RunTextBoxKeyTestCases_Multiline` contains real multiline-specific coverage beyond basic editing semantics.
- The unit-test rendering layer participates in caret queries; when failures are caused by stub behavior, it should be fixed in isolation and as minimally as possible.

## TASK No.4: Add Enter/CRLF tests with GetDocument assertions (Paragraph mode controls)

Add and/or extend a key-test helper so ENTER/CRLF behaviors are covered by unit tests for the Paragraph-mode editor controls (the three non-singleline, non-multiline controls in this test file).

- `RunTextBoxKeyTestCases_MultiParagraph` for the other three editor controls.

This task focuses on ENTER/CRLF behaviors only (no caret-navigation coverage in this task).

Control paragraph mode can’t be switched until the control is created, so this task only tests each Paragraph-mode control’s default paragraph mode.

Note: earlier request text used `RunTextBoxKeyTestCases_MultiParagraphs`; this document follows the latest update and uses `RunTextBoxKeyTestCases_MultiParagraph` as the helper name.

All new assertions for content must use `GetDocument()` and `GetTextForReading()` rather than `GetText()`.

### what to be done

- Implement `RunTextBoxKeyTestCases_MultiParagraph` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp.
	- Update the other three editor-control test categories in the same file to call `RunTextBoxKeyTestCases_MultiParagraph`.
- Add/extend test cases (modeled after the structure and coverage approach used by `RunTextBoxKeyTestCases_Singleline`) that validate:
	- Multi-paragraph (Paragraph mode) semantics:
		- `ENTER` creates a new paragraph (paragraph count increases).
		- `CTRL+ENTER` inserts a line break inside the current paragraph (paragraph count unchanged).
			- This behavior should be decided by `paragraphMode == Paragraph` (not by `pasteAsPlainText`), because there are plain-text paragraph-mode controls that still need in-paragraph line breaks.
			- Typing/pasting with single CRLF results in a line break within a paragraph.
			- Typing/pasting with double CRLF results in a paragraph break.
			- Typing/pasting with 3 consecutive CRLF matches the expected parsing rule (e.g. `a\r\n\r\n\r\nb` => 2 paragraphs; the second paragraph begins with an empty line and then `b`).
			- Copy/paste roundtrip for multi-line / multi-paragraph selections preserves the paragraph/line structure.
- For every new assertion about text/document content:
	- Validate paragraph count in the `DocumentModel` returned by `GetDocument()`.
	- Validate each paragraph’s content using `DocumentParagraphRun::GetTextForReading()`.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to stay within half of the window width based on the unit-test renderer’s simplified width estimate (each character is approximately `0.6 * fontSize`).

### rationale

- ENTER/CRLF semantics are the main behavioral differences between paragraph modes; covering them first keeps later navigation tests smaller and easier to debug.
- Using `GetDocument()` + `GetTextForReading()` verifies the underlying model directly and avoids relying on a potentially-normalized `GetText()` string.
- Keeping wrapping out of scope reduces sensitivity to the unit-test renderer’s stub layout, while still validating the intended editing semantics.

## TASK No.5: Add keyboard caret navigation tests (Paragraph mode controls)

Add keyboard caret navigation coverage into `RunTextBoxKeyTestCases_MultiParagraph` to validate Paragraph-mode boundary behaviors and HOME/END semantics for the three Paragraph-mode controls.

This task focuses on navigation only (ENTER/CRLF behaviors should already be covered by TASK No.4).

As clarified in the latest update, only the default paragraph mode for each Paragraph-mode control is covered here (no mode switching within a control instance).

Note: earlier request text used `RunTextBoxKeyTestCases_MultiParagraphs`; this document follows the latest update and uses `RunTextBoxKeyTestCases_MultiParagraph` as the helper name.

### what to be done

- Add new test cases in `RunTextBoxKeyTestCases_MultiParagraph` that cover:
	- `LEFT` / `RIGHT` boundary movement:
		- Multi-paragraph mode: beginning/ending of paragraph jumps to previous/next paragraph when appropriate.
	- `UP` / `DOWN` vertical caret movement.
	- `PAGE UP` / `PAGE DOWN` vertical caret movement by page.
	- `HOME` / `END` behavior per the UPDATE (line → paragraph → document), and `CTRL+HOME` / `CTRL+END` (document).
	- Any other navigation operations that are mode-sensitive for `GuiDocumentParagraphMode::Paragraph` as discovered from existing tests and implementation.
- In order to make caret movement visible in logs and to ensure each frame produces UI change:
	- After each navigation key (or small sequence), type a character (or insert a marker) so the caret position becomes visually reflected and the frame is valid.
	- When a step already causes a visible UI change, prefer a final hide-only frame (`window->Hide();`) to capture the post-action UI state in logs.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to avoid wrapping in the unit-test renderer, so the tests primarily validate caret navigation rules rather than the stub wrapping algorithm.

### rationale

- Caret navigation is mode-sensitive and easy to regress; dedicated tests protect these behaviors when caret/line calculations change.
- Splitting navigation tests from ENTER/CRLF tests keeps failures localized and helps diagnose whether a regression is in editing semantics or navigation semantics.
- Typing after moving keeps frame logs readable and prevents “no UI change” frame crashes.

# Impact to the Knowledge Base

## GacUI

- Consider adding a short knowledge base topic (or extending an existing unit testing topic) explaining:
	- How to assert editor control content via `GuiDocumentCommonInterface::GetDocument()` and per-paragraph `DocumentParagraphRun::GetTextForReading()`.
	- How to structure keyboard navigation tests so each frame introduces UI change (e.g. typing a marker after navigation to make caret location visible in rendering logs).
	- Practical guidance for keeping test text short to avoid line wrapping in the unit-test renderer when wrapping behavior is intentionally out of scope.

# !!!FINISHED!!!

