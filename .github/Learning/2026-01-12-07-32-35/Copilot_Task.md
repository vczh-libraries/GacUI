# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

# INSIGHTS AND REASONING

## Scope / interpretation

- The “multiline mode” in this repo corresponds to `GuiDocumentParagraphMode::Multiline`, used by `GuiMultilineTextBox` via `GuiDocumentConfig::GetMultilineTextBoxDefaultConfig()`.
- The task requires:
	- New unit tests (in the editor key test file) for ENTER / CTRL+ENTER and CRLF preservation.
	- Fixing the underlying behavior if those tests fail.
	- New assertions must validate the document model via `GetDocument()` and per-paragraph `GetTextForReading()`, not `GetText()`.

## Evidence from current implementation

### Default config confirms “Multiline + plain text + CRLF delimiter”

- `GuiDocumentConfig::GetMultilineTextBoxDefaultConfig()` sets:
	- `pasteAsPlainText = true`
	- `paragraphMode = GuiDocumentParagraphMode::Multiline`
	- `doubleLineBreaksBetweenParagraph = false`
	- (see Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp)

Implications:

- In multiline mode, “paragraph delimiter” used by `UserInput_ConvertDocumentToText()` is `\r\n` (because `doubleLineBreaksBetweenParagraph == false`).
- Pasting text uses `SetSelectionText()` which parses plain text via `UserInput_FormatText(value, ...)`, and then inserts an array of paragraph texts.

### Current Enter handling is inconsistent for multiline

- In `GuiDocumentCommonInterface::ProcessKey()` (Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp):
	- `KEY_RETURN` ignores Enter for `GuiDocumentParagraphMode::Singleline`.
	- For non-singleline:
		- `CTRL+ENTER` inserts `text[0] = L"\r\n"` (single string containing a CRLF).
		- `ENTER` inserts `Array<WString>(2)` (two paragraphs), which is effectively “split / new paragraph”.

Why this is a problem in multiline mode:

- Multiline’s user-input normalization explicitly removes line breaks *inside a paragraph* (`UserInput_JoinLinesInsideParagraph`) when `paragraphMode != Paragraph`.
- Therefore inserting a literal `"\r\n"` inside one paragraph (the current CTRL+ENTER path) is at best redundant and at worst will be flattened into nothing (or a space if `spaceForFlattenedLineBreak` were enabled).
- The requested behavior for multiline is that `ENTER` and `CTRL+ENTER` do the same thing and both create a new line (CRLF-separated line).

## Design proposal

### 1) Update Enter semantics for multiline mode

Goal: In `GuiDocumentParagraphMode::Multiline`, `ENTER` and `CTRL+ENTER` should both create a new line.

Proposed change (high-level):

- In `GuiDocumentCommonInterface::ProcessKey()` handling `VKEY::KEY_RETURN`:
	- Keep current `Singleline` behavior (ignore Enter entirely, selection unchanged).
	- If `paragraphMode == Multiline`:
		- Treat `CTRL+ENTER` the same as `ENTER`.
		- Use the same insertion path as current “paragraph break” insertion (the `Array<WString>(2)` path) so that a new “line” is represented structurally, not as a CRLF embedded inside a paragraph.
	- If `paragraphMode == Paragraph`:
		- Keep existing distinction: `ENTER` creates a new paragraph, `CTRL+ENTER` creates a line break within the paragraph.

Rationale:

- It matches the task’s requested semantics for multiline.
- It avoids producing “CRLF embedded in a paragraph”, which is intentionally normalized away for non-Paragraph modes.
- It preserves the existing Paragraph-mode behavior needed by later tasks.

### 2) Add `RunTextBoxKeyTestCases_Multiline` and model-level assertions

Where:

- Add a new helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp: `RunTextBoxKeyTestCases_Multiline`.
- Update the `TEST_CATEGORY(L"GuiMultilineTextBox")` section to call it (in addition to the existing shared scaffold `RunTextBoxKeyTestCases`).

Test organization (suggested):

- `TEST_CATEGORY(L"Enter/CRLF (Multiline)")` or similar within `RunTextBoxKeyTestCases_Multiline`.

Test cases (minimum set to satisfy task):

1) `Enter_InsertsNewLine`
	- Arrange: focus, type a short single-line string.
	- Act: place caret in middle and press `ENTER`.
	- Assert (document model):
		- `textBox->GetDocument()->paragraphs.Count()` increases (typically becomes 2).
		- `paragraphs[0]->GetTextForReading()` equals text before caret.
		- `paragraphs[1]->GetTextForReading()` equals text after caret.

2) `CtrlEnter_SameAsEnter`
	- Same setup, but use `CTRL+ENTER`.
	- Assert the same structural effects as (1) (not a flattened no-op).

3) `Paste_PreservesCRLF_AsMultipleLines`
	- Arrange: put multi-line text into the clipboard (include mixed `\r\n`, `\n`, and empty lines).
	- Act: paste with `CTRL+V`.
	- Assert via `GetDocument()`:
		- Paragraph count equals expected line count.
		- Each paragraph content matches expected.

4) `CopyPaste_Roundtrip_MultipleLines`
	- Arrange: create a multi-line document (via paste or repeated Enter), select all.
	- Act: `CTRL+C`, clear, `CTRL+V`.
	- Assert that the document’s paragraph count and paragraph texts are preserved.

Notes on assertions:

- New tests should not call `GetText()`.
- Use `GetDocument()` and then inspect `document->paragraphs[i]` using `DocumentParagraphRun::GetTextForReading()`.
- This aligns with the task requirement to validate the underlying model rather than a potentially-normalized string.

## Expected impact / risk

- Behavior change is narrowly scoped to `KEY_RETURN` handling when `config.paragraphMode == Multiline`.
- Paragraph-mode (`GuiDocumentParagraphMode::Paragraph`) behavior must remain unchanged because Task 4 relies on a meaningful distinction between `ENTER` (paragraph break) and `CTRL+ENTER` (line break).
- Existing unit tests currently use `GetText()` heavily in the shared helper; this task only requires new tests to use model-level assertions, so the change should be additive.

## Open questions / things to verify during implementation

- Confirm how `TestControls` / `UnitTestRemoteProtocol::TypeString()` produces newlines (whether it can emit Enter key presses or only character input). If it cannot emit Enter, rely on clipboard paste and `KeyPress(KEY_RETURN)` to create lines.
- Confirm that “line == paragraph” is the intended representation for multiline mode in the document model (it appears consistent with `doubleLineBreaksBetweenParagraph == false` and `UserInput_ConvertDocumentToText()` using `\r\n`).

# !!!FINISHED!!!
