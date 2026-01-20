# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.9: Add multi-paragraph rich text style editing tests (GuiDocumentLabel)

Add a second rich-text test category that repeats the non-hyperlink rich-text style scenarios from TASK No.8, but validates a single editing call spanning multiple paragraphs.

Per the UPDATE, this task creates a new test category.

### what to be done

- In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, add a new `TEST_CATEGORY` dedicated to multi-paragraph style editing for `GuiDocumentLabel`:
	- Use a concise category name (e.g. `MultiParagraph`) since `RichText` and `GuiDocumentLabel` are already implied by the file name and log path.
	- Keep the log path consistent with TASK No.8 (no extra folder segments for control/category), e.g. `Controls/Editor/Features/RichText/MultiParagraph_<CaseName>`.
- Create document content with multiple paragraphs and select a range that spans paragraph boundaries.
- Add representative test cases to validate that a single `EditStyle(...)` call correctly applies the requested property across multiple paragraphs.
	- Keep the number of cases small; focus on correctness across paragraph boundaries.
- Verify the resulting style using `SummarizeStyle(...)` over sub-ranges within each paragraph (and/or by inspecting runs) to ensure the style is applied consistently across paragraphs.
- Keep the log path prefix `Controls/Editor/Features/RichText/...` consistent with TASK No.8.

### rationale

- Multi-paragraph editing is a distinct behavior surface: range-to-run mapping must correctly split and apply styles across paragraph boundaries.
- A dedicated category keeps the single-paragraph tests focused and avoids mixing concerns, matching the UPDATE's requirement for a new category.

# UPDATES

## UPDATE

You also need to add test cases to cover when EditStyle updates multiple properties in one call

# INSIGHTS AND REASONING

## Scope

This task only adds unit test coverage in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` for `vl::presentation::controls::GuiDocumentLabel` multi-paragraph style editing, without adding hyperlink scenarios (those are TASK No.10).

## Why a dedicated multi-paragraph category is required

The existing tests in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` only validate style editing inside a single paragraph using `TextPos(0, ...)`. That does not exercise the code paths that:

- Split the begin paragraph and end paragraph at different caret positions.
- Apply the same operation to fully-covered middle paragraphs.
- Aggregate style summaries across multiple paragraphs.

These behaviors are explicitly implemented in `vl::presentation::documents::DocumentModel`:

- Multi-paragraph style editing is implemented by `vl::presentation::documents::DocumentModel::EditStyle(...)` delegating to `DocumentModel::EditContainer(...)` in `Source/Resources/GuiDocument_Edit.cpp`.
	- `EditContainer(...)` cuts at `begin` and `end` and then applies the editor to `[begin.column, end-of-begin-paragraph)`, full ranges for middle paragraphs, and `[start-of-end-paragraph, end.column)`.
- Multi-paragraph style summarizing is implemented by `vl::presentation::documents::DocumentModel::SummarizeStyle(...)` in `Source/Resources/GuiDocument_Edit.cpp`, iterating per paragraph and aggregating via `AggregateStyle(...)`.

A dedicated `TEST_CATEGORY(L"MultiParagraph")` should validate the correctness of this paragraph-spanning logic without mixing it into the single-paragraph scenarios.

## Test data: how to reliably create multiple paragraphs in GuiDocumentLabel

`vl::presentation::controls::GuiDocumentLabel` uses `GuiDocumentConfig::GetDocumentLabelDefaultConfig()` (see `Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp`), where:

- `paragraphMode = GuiDocumentParagraphMode::Paragraph`
- `doubleLineBreaksBetweenParagraph = true`

`GuiDocumentCommonInterface::LoadTextAndClearUndoRedo(...)` (see `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`) routes the input text through `UserInput_FormatText(text, paragraphTexts, config)`, so test content should use double CRLF (`\r\n\r\n`) between paragraphs to guarantee multiple document paragraphs in this mode.

Therefore, tests can build a 3-paragraph document using a single call to `LoadTextAndClearUndoRedo(...)`, e.g.:

- Paragraph 0: `0123456789`
- Paragraph 1: `abcdefghij`
- Paragraph 2: `KLMNOPQRST`

This makes caret positions (`TextPos(row, column)`) straightforward and keeps each paragraph short to avoid line wrapping concerns.

## Proposed test structure

Add a new `TEST_CATEGORY(L"MultiParagraph")` in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, reusing the existing window resource and helper style builders (`MakeFontFaceAndSizeStyle`, `MakeTextStyle`, `MakeColorStyle`).

To keep the tests readable, the category should also avoid hard-coding `row=0` in helper functions (e.g. add an overload of the local `Summarize(...)` helper to accept `TextPos` or a `row` parameter), so assertions can be expressed per paragraph.

Each test follows the same pattern as the current SingleParagraph tests:

- `Init` frame:
	- Focus control, load multi-paragraph text via `LoadTextAndClearUndoRedo(...)`.
	- Perform exactly one `EditStyle(begin, end, style)` call where `begin.row != end.row`.
- `Verify` frame:
	- Use `SummarizeStyle(...)` on sub-ranges inside each involved paragraph to validate the expected style.
	- Also assert that unaffected sub-ranges outside the edited span do not report that style property.

Log paths must follow `Controls/Editor/Features/RichText/MultiParagraph_<CaseName>` to match TASK No.8 conventions.

## Update requirement: multi-property EditStyle in one call

`EditStyle(...)` applies a `Ptr<DocumentStyleProperties>` by wrapping affected text runs in a `DocumentStylePropertiesRun` created from `CopyStyle(style)` (see `vl::presentation::document_operation_visitors::AddStyleVisitor` in `Source/Resources/GuiDocumentEditor_AddContainer.cpp`). The knowledge base documents that `DocumentStyleProperties` fields are intended to be combined (face/size/color/bold/etc) and that `SummarizeStyle(...)` reports a “common style” where non-uniform fields become `null` (see `.github/KnowledgeBase/manual/gacui/components/controls/editor/home.md`). Therefore, multi-paragraph tests should include at least one case where the edited range is styled by a single call that sets multiple independent properties, and then validate that `SummarizeStyle(...)` reports all of them consistently.

## Representative cases (keep small, but cover paragraph boundaries)

The MultiParagraph category should mirror the intent of the SingleParagraph scenarios, but specifically ensure correctness across paragraph boundaries:

1. `MultiParagraph_Font_OneCall`
	- Apply `face + size` from a middle column in paragraph 0 to a middle column in paragraph 2.
	- Verify:
		- Styled property present in the tail of paragraph 0, all of paragraph 1, and the head of paragraph 2.
		- Styled property absent in the head of paragraph 0 and tail of paragraph 2.

2. `MultiParagraph_FontStyle_OneCall`
	- Apply `bold/italic/underline/strikeline` across multiple paragraphs with the same begin/end pattern.
	- Verify per-paragraph sub-ranges similarly.

3. `MultiParagraph_Color_OneCall`
	- Apply `color` across multiple paragraphs and verify per-paragraph segments.

4. `MultiParagraph_MultiProperty_OneCall`
	- Apply multiple independent properties in a single `EditStyle(...)` call (e.g. `face+size` plus `bold` plus `color`) across multiple paragraphs.
	- Verify each property is present in the edited sub-ranges for the tail of the begin paragraph, all middle paragraphs, and the head of the end paragraph.

5. `MultiParagraph_Overlap_SameProperty`
	- Two `EditStyle` calls where both span multiple paragraphs and overlap (e.g. first applies `FontA` across p0..p2, second applies `FontB` across a subrange spanning p1..p2).
	- Verify that overlap overrides correctly, and `SummarizeStyle(...)` returns no `face` for a range that mixes `FontA` and `FontB`.

6. `MultiParagraph_Overlap_DifferentProperties`
	- Two overlapping multi-paragraph edits for different properties (e.g. `bold=true` then `color=red`) so the overlap region has both properties while non-overlap regions have only one.
	- Verify both inside and outside overlap per paragraph.

This set keeps the test count small while still validating:

- Begin/end paragraph cutting at different columns.
- Full-paragraph coverage for middle paragraphs.
- Correct style aggregation behavior in `SummarizeStyle(...)` when ranges are uniform vs mixed.

# !!!FINISHED!!!
