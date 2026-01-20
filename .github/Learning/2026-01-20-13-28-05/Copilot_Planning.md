# !!!PLANNING!!!

# UPDATES

## UPDATE

Just to clarify, by saying edit styles across different paragraphs, I mean one call to edit multiple paragraphs at the same time, not just editing a piece of text inside one but different paragraph

# EXECUTION PLAN

## STEP 1: Align with existing rich-text test patterns

Add the new multi-paragraph tests into `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` following the same structure already used by `TEST_CATEGORY(L"SingleParagraph")`:

- Use `TooltipTimer`, `GacUIUnitTest_SetGuiMainProxy`, `OnNextIdleFrame(L"Init", ...)`, `OnNextIdleFrame(L"Verify", ...)`, and `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)`.
- Keep log path prefix exactly `Controls/Editor/Features/RichText/...`.

This keeps the new category consistent with TASK No.8 style tests and avoids inventing a new test harness shape.

## STEP 2: Add a multi-paragraph Summarize helper overload

Extend the local helper `Summarize(...)` so tests can summarize style within any paragraph (not just `row==0`) without duplicating `TextPos(row, ...)` everywhere.

Proposed code change (keeps existing `Summarize(textBox, begin, end)` intact for `SingleParagraph`):

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(row, begin), TextPos(row, end));
	}

This is necessary because multi-paragraph validations must query sub-ranges inside paragraph 0/1/2 separately (the existing helper hard-codes `TextPos(0, ...)`).

## STEP 3: Add a dedicated MultiParagraph test category

Add a new `TEST_CATEGORY(L"MultiParagraph")` next to `SingleParagraph` in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`.

- Category name: `MultiParagraph` (concise, as required).
- Log paths: `Controls/Editor/Features/RichText/MultiParagraph_<CaseName>` (no extra folder segments).
- Document content: load a single string containing multiple paragraphs using double CRLF to ensure paragraph splitting under the document label default config:

	textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

This separation is necessary to validate the paragraph-spanning logic in a focused way without mixing it into the existing single-paragraph scenarios.

## STEP 4: Add representative multi-paragraph EditStyle(...) test cases

Add a small set of cases that repeat the non-hyperlink rich-text style scenarios (font/font-style/color), but each validates **one** `EditStyle(begin, end, style)` call that spans multiple paragraphs (i.e. `begin.row != end.row`). Do not implement this as multiple `EditStyle(...)` calls targeting different paragraphs.

For all cases below, use a paragraph-spanning range that forces partial coverage on the begin/end paragraphs and full coverage on the middle paragraph, e.g.:

	const auto begin = TextPos(0, 2);
	const auto end   = TextPos(2, 5);

### Case A: MultiParagraph_Font_OneCall

Init:

	textBox->EditStyle(begin, end, MakeFontFaceAndSizeStyle(WString::Unmanaged(L"Arial"), 16));

Verify (sub-ranges inside each paragraph):

- Outside edited head of paragraph 0 stays unchanged:

	auto s0h = Summarize(textBox, 0, 0, 2);
	TEST_ASSERT(!s0h->face);
	TEST_ASSERT(!s0h->size);

- Tail of paragraph 0 (within edit) has font face+size:

	auto s0t = Summarize(textBox, 0, 2, 10);
	TEST_ASSERT(s0t->face && s0t->face.Value() == WString::Unmanaged(L"Arial"));
	TEST_ASSERT(s0t->size && s0t->size.Value() == DocumentFontSize(16, false));

- Whole paragraph 1 (fully covered) has font face+size:

	auto s1 = Summarize(textBox, 1, 0, 10);
	TEST_ASSERT(s1->face && s1->face.Value() == WString::Unmanaged(L"Arial"));
	TEST_ASSERT(s1->size && s1->size.Value() == DocumentFontSize(16, false));

- Head of paragraph 2 (within edit) has font face+size, tail stays unchanged:

	auto s2h = Summarize(textBox, 2, 0, 5);
	TEST_ASSERT(s2h->face && s2h->face.Value() == WString::Unmanaged(L"Arial"));
	TEST_ASSERT(s2h->size && s2h->size.Value() == DocumentFontSize(16, false));

	auto s2t = Summarize(textBox, 2, 5, 10);
	TEST_ASSERT(!s2t->face);
	TEST_ASSERT(!s2t->size);

Why: validates begin/end paragraph cutting and middle-paragraph full coverage for a non-boolean style property.

### Case B: MultiParagraph_FontStyle_OneCall

Init:

	textBox->EditStyle(begin, end, MakeTextStyle(true, true, false, false));

Verify:

- For each paragraph, summarize a fully edited sub-range and assert all four properties match the requested values.
- Also summarize outside sub-ranges (`p0[0,2)` and `p2[5,10)`) and assert the properties are not present (i.e. `!summary->bold`, etc), to confirm no leakage beyond the cut points.

Why: validates the same paragraph-spanning logic for multiple boolean fields in the same style object.

### Case C: MultiParagraph_Color_OneCall

Init:

	textBox->EditStyle(begin, end, MakeColorStyle(Color(255, 0, 0)));

Verify:

- Edited sub-ranges in paragraph 0/1/2 report `summary->color.Value() == Color(255, 0, 0)`.
- Outside sub-ranges do not report `color`.

Why: validates a different scalar property type (color) across paragraph boundaries.

### Case D: MultiParagraph_AllProperties_OneCall (update requirement)

To cover “EditStyle updates multiple properties in one call”, include one test where a single `DocumentStyleProperties` instance contains multiple independent fields, and it is applied by exactly one `EditStyle(...)` call.

Init:

	auto style = Ptr(new DocumentStyleProperties);
	style->face = WString::Unmanaged(L"Arial");
	style->size = DocumentFontSize(16, false);
	style->bold = true;
	style->italic = false;
	style->underline = true;
	style->strikeline = false;
	style->color = Color(255, 0, 0);
	textBox->EditStyle(begin, end, style);

Verify:

- Use `Summarize(textBox, row, ...)` on edited sub-ranges in all involved paragraphs and assert all populated fields match.
- Use `Summarize(...)` on outside sub-ranges (`p0[0,2)`, `p2[5,10)`) and assert these fields are not present, confirming the edit is correctly bounded.

Why: ensures the multi-paragraph edit path handles a multi-field style object correctly in one operation (not relying on multiple `EditStyle` calls like `SingleParagraph_AllProperties_SameRange`).

## STEP 5: Ensure log paths match TASK No.8 conventions

For each `TEST_CASE(L"...")` in the new category, set the log path to:

	WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_<CaseName>")

This is necessary so the new tests appear alongside TASK No.8 results and satisfy the “no extra folder segments” requirement.

# TEST PLAN

- Run the unit test suite that includes `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` and confirm the new `Controls/Editor/Features/RichText/MultiParagraph_*` logs are generated and all cases pass.
- For each case, confirm the assertions cover both edited and unedited sub-ranges in paragraph 0 and paragraph 2 (boundary correctness), and at least one edited sub-range in paragraph 1 (full-paragraph coverage).
- Specifically for `MultiParagraph_AllProperties_OneCall`, confirm a single `EditStyle(...)` call is used and the verify step checks multiple properties (`face`, `size`, `bold`, `underline`, `color`) within each involved paragraph.

# !!!FINISHED!!!
