# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.4: Add Enter/CRLF tests with GetDocument assertions (Paragraph mode controls)

Add and/or extend a key-test helper so ENTER/CRLF behaviors are covered by unit tests for the Paragraph-mode editor controls (the three non-singleline, non-multiline controls in this test file).

- `RunTextBoxKeyTestCases_MultiParagraph` for the other three editor controls.

This task focuses on ENTER/CRLF behaviors only (no caret-navigation coverage in this task).

Control paragraph mode can't be switched until the control is created, so this task only tests each Paragraph-mode control's default paragraph mode.

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
	- Validate each paragraph's content using `DocumentParagraphRun::GetTextForReading()`.
- Organize `protocol->OnNextIdleFrame(...)` frames per `## Organizing Frames` in `.github/copilot-instructions.md`:
	- First frame is always `Ready`, and each subsequent frame name describes what the previous frame did.
	- Prefer action + verification in the same frame, and keep a final hide-only frame (`window->Hide();`) when possible.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to stay within half of the window width based on the unit-test renderer's simplified width estimate (each character is approximately `0.6 * fontSize`).

### rationale

- ENTER/CRLF semantics are the main behavioral differences between paragraph modes; covering them first keeps later navigation tests smaller and easier to debug.
- Using `GetDocument()` + `GetTextForReading()` verifies the underlying model directly and avoids relying on a potentially-normalized `GetText()` string.
- Keeping wrapping out of scope reduces sensitivity to the unit-test renderer's stub layout, while still validating the intended editing semantics.

# UPDATES

# INSIGHTS AND REASONING

## Scope and target controls

This task adds Paragraph-mode specific ENTER/CRLF and paste semantics coverage for the three non-singleline, non-multiline editor controls in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`:

- `GuiDocumentTextBox`
- `GuiDocumentLabel`
- `GuiDocumentViewer`

These three controls share the `GuiDocumentCommonInterface` core and, by default, run in `GuiDocumentParagraphMode::Paragraph` with `doubleLineBreaksBetweenParagraph = true` (see `Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp` `GetDocumentLabelDefaultConfig()` and `GetDocumentViewerDefaultConfig()`).

## Relevant implementation points (evidence)

### ENTER vs. CTRL+ENTER in Paragraph mode

`GuiDocumentCommonInterface::ProcessKey(...)` (in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`) handles `VKEY::KEY_RETURN` as:

- `paragraphMode == Singleline`: ignore Enter entirely (including preserving selection).
- `ctrl && paragraphMode == Paragraph`: insert a line break inside the current paragraph by editing text with a single `L\"\\r\\n\"` fragment.
- otherwise: insert a paragraph break by editing text with two fragments (causing a new paragraph to be created).

This matches the intended behavior that Paragraph mode differentiates `ENTER` (new paragraph) and `CTRL+ENTER` (new line inside paragraph), independent of `pasteAsPlainText`.

### Plain-text paste parsing and paragraph splitting

`GuiDocumentCommonInterface::Paste()` pastes plain text via `SetSelectionText(reader->GetText())` when document data is not used.

`GuiDocumentCommonInterface::SetSelectionText(...)` uses `UserInput_FormatText(value, paragraphTexts, config)` to split the pasted string into paragraph fragments, then calls `EditText(..., text, true)` with these fragments.

In Paragraph-mode document controls, `doubleLineBreaksBetweenParagraph = true` means `UserInput_FormatText` treats:

- single CRLF: a line break inside a paragraph
- double CRLF: a paragraph break
- multiple CRLF sequences: split into paragraph fragments while keeping in-paragraph line breaks as CRLF within the fragment

The design of tests for `a\\r\\n\\r\\n\\r\\nb` should follow this parser behavior and verify the exact per-paragraph text produced.

## Proposed test-helper design

Add `RunTextBoxKeyTestCases_MultiParagraph(const wchar_t* resource, const WString& controlName)` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, following the structure of:

- `RunTextBoxKeyTestCases_Singleline` (singleline-specific behavior)
- `RunTextBoxKeyTestCases_Multiline` (multiline paragraph-mode behavior)

The helper should be called in addition to the shared scaffold (`RunTextBoxKeyTestCases`) for:

- `GuiDocumentTextBox`
- `GuiDocumentLabel`
- `GuiDocumentViewer`

Within this helper, keep coverage limited to ENTER/CRLF + paste/copy semantics; do not add caret navigation cases (reserved for TASK No.5).

## Proposed test cases (high level)

All new assertions must use `GetDocument()` and `DocumentParagraphRun::GetTextForReading()`; avoid `GetText()` to prevent accidental normalization from masking issues.

### 1) ENTER creates a new paragraph

- Arrange: type a short ASCII string (no wrapping risk), place caret in the middle.
- Act: press `VKEY::KEY_RETURN` (no modifiers).
- Assert: paragraph count increases by 1, and the two resulting paragraph texts match the split around the caret.

### 2) CTRL+ENTER inserts a line break in the paragraph

- Arrange: type a short ASCII string, place caret in the middle.
- Act: press `VKEY::KEY_RETURN` with `ctrl = true`.
- Assert: paragraph count unchanged, and the single paragraph contains exactly one `\\r\\n` at the caret position (e.g. `AB\\r\\nCDE`).

### 3) Paste: single CRLF stays within a paragraph

- Arrange: write plain text to clipboard containing `a\\r\\nb` (or a mix of `\\n` and `\\r\\n` to confirm normalization).
- Act: `CTRL+V`.
- Assert: `document->paragraphs.Count() == 1` and paragraph 0 text contains `\\r\\n` between `a` and `b`.

### 4) Paste: double CRLF creates a paragraph break

- Arrange: clipboard text `a\\r\\n\\r\\nb`.
- Act: `CTRL+V`.
- Assert: `document->paragraphs.Count() == 2`, with paragraph texts `a` and `b`.

### 5) Paste: 3 consecutive CRLF rule

- Arrange: clipboard text `a\\r\\n\\r\\n\\r\\nb`.
- Act: `CTRL+V`.
- Assert: `document->paragraphs.Count() == 2`, and paragraph 1 begins with an empty line followed by `b` (expected per-paragraph `GetTextForReading()` to include a leading `\\r\\n` before `b`).

### 6) Copy/paste roundtrip preserves paragraph/line structure

- Arrange: construct a document containing:
	- at least 2 paragraphs (created via `ENTER`)
	- at least one in-paragraph line break (created via `CTRL+ENTER`)
- Act: select across multiple paragraphs (e.g. `CTRL+A`), `CTRL+C`, clear selection (e.g. `DELETE`), `CTRL+V`.
- Assert: the resulting `GetDocument()` structure matches the pre-copy structure (paragraph count and each paragraph’s `GetTextForReading()`), ensuring both paragraph boundaries and in-paragraph line breaks roundtrip.

## Frame organization and stability constraints

- Follow `.github/copilot-instructions.md` `## Organizing Frames`: first frame `Ready`, subsequent frames named for the previous frame’s action, and keep a final `window->Hide();` frame.
- Keep inserted strings short to avoid line wrapping in the unit-test renderer, so failures isolate paragraph splitting vs. wrapping behavior.

# !!!FINISHED!!!
