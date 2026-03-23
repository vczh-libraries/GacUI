# !!!TASK!!!

# PROBLEM DESCRIPTION
## TASK No.4: Test EditStyleName (MultiParagraph)

Write test cases for `EditStyleName` spanning multiple paragraphs, under `TEST_CATEGORY(L"EditStyleName")` / `TEST_CATEGORY(L"MultiParagraph")`.

### what to be done

- **Test: Apply a style across paragraphs** — Load multi-paragraph text (e.g., `"0123456789\r\n\r\nabcdefghij"`), register a style, apply it across paragraph boundaries (e.g., `TextPos(0,5)` to `TextPos(2,5)`), verify via `SummarizeStyleName` on each paragraph.
- **Test: Apply a style to entire multi-paragraph document** — Apply a single style to the full range spanning all paragraphs and verify every paragraph has the style.

### rationale

- Multi-paragraph is important because the document model operates per-paragraph (`DocumentParagraphRun`), and `EditStyleName` on `GuiDocumentCommonInterface` delegates through `documentElement->EditStyleName` which iterates over paragraphs.
- The `TestControls_Editor_RichText.cpp` follows the same SingleParagraph/MultiParagraph split pattern.

# UPDATES

# INSIGHTS AND REASONING

## Goal / Scope

This design covers **TASK No.4** only: extend the existing style-name tests in `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp` by adding multi-paragraph coverage for `vl::presentation::controls::GuiDocumentCommonInterface::EditStyleName`.

Out of scope (covered by other Scrum tasks):
- Any new semantics of `RemoveStyleName`, `SummarizeStyleName` edge-cases, `RenameStyle`.
- Undo/redo behavior.
- Any changes to production code.

## Evidence from implementation (why multi-paragraph matters)

### EditStyleName is paragraph-aware (model side)

- `vl::presentation::DocumentModel::EditStyleName` is implemented via `DocumentModel::EditContainer(...)`, which explicitly handles both single-row and multi-row ranges by iterating `for (vint i = begin.row; i <= end.row; i++)` and calling the supplied editor for each affected paragraph (`Source\Resources\GuiDocument_Edit.cpp`, `DocumentModel::EditContainer`, `DocumentModel::EditStyleName`).
- `DocumentModel::EditStyleName` uses `document_editor::AddStyleName(...)` to add a named style container inside each affected paragraph range (`Source\Resources\GuiDocument_Edit.cpp`, `DocumentModel::EditStyleName`).

### EditStyleName is exposed through GuiDocumentCommonInterface

- `vl::presentation::controls::GuiDocumentCommonInterface::EditStyleName` early-returns when `config.pasteAsPlainText` is enabled, otherwise delegates through `EditStyleInternal(begin, end, ...)` into `documentElement->EditStyleName(begin, end, styleName)` (`Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp`, around `GuiDocumentCommonInterface::EditStyleName`).

### How unit tests should create multiple paragraphs

- `GuiDocumentCommonInterface` uses `config.doubleLineBreaksBetweenParagraph` to split `LoadTextAndClearUndoRedo(...)` input into paragraphs; it uses a `"\r\n\r\n"` joiner when converting a document back to text (`Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp`, around `UserInput_FormatText` and `UserInput_ConvertDocumentToText`).
- Existing rich-text tests already rely on the same convention: multi-paragraph text literals use `"...\r\n\r\n..."` (e.g. `TestControls_Editor_RichText.cpp` in `TEST_CATEGORY(L"MultiParagraph")`).

## Proposed test design (high-level)

### Where to add tests

Add new tests to the existing file:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Under the existing hierarchy:
- `TEST_CATEGORY(L"Styles")`
  - `TEST_CATEGORY(L"EditStyleName")`
    - **add** `TEST_CATEGORY(L"MultiParagraph")` sibling to `SingleParagraph`.

### Test harness and helpers

Reuse the existing harness pattern already used in this file:
- `TooltipTimer timer;`
- `GacUIUnitTest_SetGuiMainProxy(...)` with two `protocol->OnNextIdleFrame(...)` callbacks.
- `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` with the existing `resource_DocumentViewer`.

Reuse existing helpers already present in `TestControls_Editor_Styles.cpp`:
- `RegisterStyle(...)`
- `SummarizeName(textBox, row, begin, end)` for paragraph-local assertions.

### Stable log identities

Use stable log identities consistent with the existing naming in this file:
- `Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_AcrossParagraphs`
- `Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_FullDocument`

## Test case details

All test cases should:
- `textBox->SetFocused();`
- `textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");` (3 paragraphs, 10 chars each; aligns with the multi-paragraph patterns in `TestControls_Editor_RichText.cpp`).
- Register the style name being applied (e.g. `MyBold`) using `RegisterStyle(...)` so the style name is "real" (even though storage of the name is independent of registration).

### 1) Apply a style across paragraphs

- Apply `MyBold` across paragraph boundaries:
  - `EditStyleName(TextPos(0, 5), TextPos(2, 5), L"MyBold")`
- Verify per-paragraph ranges (paragraph-local first):
  - Paragraph 0: `[0,5)` returns empty / null; `[5,10)` returns `MyBold`.
  - Paragraph 1: `[0,10)` returns `MyBold`.
  - Paragraph 2: `[0,5)` returns `MyBold`; `[5,10)` returns empty / null.

Cross-paragraph summarization assertions:
- `textBox->SummarizeStyleName(TextPos(0, 5), TextPos(2, 5))` returns `MyBold` (the entire queried range is uniformly styled).
- `textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10))` returns empty / null (the full document includes unstyled regions).

Rationale:
- This mirrors the `MultiParagraph_*` rich-text tests pattern: ensure head/tail regions outside the edited range remain unchanged, while the middle paragraph becomes fully covered.

### 2) Apply a style to the entire multi-paragraph document

- Apply `MyBold` to the full range:
  - `EditStyleName(TextPos(0, 0), TextPos(2, 10), L"MyBold")`
- Verify:
  - For each paragraph row 0..2: `SummarizeName(textBox, row, 0, 10)` returns `MyBold`.
  - `textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10))` returns `MyBold`.

## Risks / guardrails

- When validating "not styled", assert nullable-empty (e.g. `!SummarizeName(...)`) instead of checking inequality against `L"MyBold"`.
- Avoid empty-range summarization (`begin == end`) because `DocumentModel::SummarizeStyleName` returns empty when the range is empty (`Source\Resources\GuiDocument_Edit.cpp`, `DocumentModel::SummarizeStyleName`).
- Keep all assertions paragraph-local first (using `SummarizeName(textBox, row, ...)`) so failures are easy to diagnose and aren’t overly dependent on multi-paragraph summarization behavior.
- Known risk: `DocumentModel::SummarizeStyleName` in `Source\Resources\GuiDocument_Edit.cpp` appears to use `end.column` in the middle-paragraph branch where `range.end` is likely intended; with the proposed 10-character-per-paragraph test text this may not manifest, and any production fix should remain out of scope for this test task.

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# !!!FINISHED!!!

