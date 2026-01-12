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

- [x] TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline
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
  - Keep the outer `TEST_CATEGORY(L"GuiSinglelineTextBox")` unchanged; in `RunTextBoxKeyTestCases_Singleline`, append " (Singleline)" to test category names (e.g. `Clipboard (Singleline)`).
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

