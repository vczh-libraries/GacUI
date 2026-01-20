# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Prepare a single-paragraph editable GuiDocumentLabel test scaffold

Update `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`:

- Replace the current empty `TEST_CATEGORY(L"RichText")` with a resource XML string hosting a `DocumentLabel` in `EditMode="Editable"` (same layout pattern as `resource_DocumentLabel` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`).
- Create exactly one `TEST_CATEGORY` that contains all test cases in this task (single paragraph, non-hyperlink rich-text style editing tests for `GuiDocumentLabel`).
- Use log paths that replace the `.../Key/...` segment used by key-navigation tests with `.../RichText/...`, e.g. `Controls/Editor/GuiDocumentLabel/RichText/SingleParagraph/<CaseName>`.

Why:

- A dedicated file + a single category keeps the test surface focused and makes log grouping predictable.
- Using `GuiDocumentLabel` in editable mode ensures `GuiDocumentCommonInterface::EditStyle` is effective for rich-text.
- Using `.../RichText/...` in the log path aligns with existing editor test log naming (`.../Key/...`) while keeping rich-text tests clearly separated.

Proposed code shape (sketch):

	const auto resource_DocumentLabel = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiDocumentLabel RichText Test" ClientSize="x:480 y:320">
	        <DocumentLabel ref.Name="textBox" EditMode="Editable">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
	        </DocumentLabel>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	TEST_CATEGORY(L"GuiDocumentLabel_SingleParagraph_RichText")
	{
		// all TEST_CASE in this task
	}

	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Editor/GuiDocumentLabel/RichText/SingleParagraph/<CaseName>"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resource_DocumentLabel
	);

## STEP 2: Add minimal helpers for style editing + summary assertions

In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, add file-local helpers to keep each test case to “setup → one or more EditStyle calls → summarize → assert”:

- A helper to create `Ptr<vl::presentation::DocumentStyleProperties>` for each property group (font / style / color) without touching unrelated fields.
- A helper that loads a known single-paragraph content using `GuiDocumentCommonInterface::LoadTextAndClearUndoRedo`, applies `EditStyle(begin, end, style)`, and returns `SummarizeStyle(begin, end)`.
- A small assertion helper that checks only the expected fields (e.g. `face`, `size`, `bold`, `color`) and avoids asserting unrelated fields that can vary with theme/default styles.

Why:

- Keeping each test case declarative reduces noise and makes it easier to audit that “one test per property” truly uses only one editing call.
- Using `SummarizeStyle` avoids fragile checks that depend on run tree shapes unless overlap tests require deeper validation.

Proposed code shape (sketch):

	using namespace vl;
	using namespace vl::presentation;
	using namespace vl::presentation::controls;

	namespace
	{
		Ptr<DocumentStyleProperties> MakeFontStyle(const WString& face, double size)
		{
			auto style = Ptr(new DocumentStyleProperties);
			style->face = face;
			style->size = DocumentFontSize(size, false);
			return style;
		}

		Ptr<DocumentStyleProperties> MakeTextStyle(bool bold, bool italic, bool underline, bool strikeline)
		{
			auto style = Ptr(new DocumentStyleProperties);
			style->bold = bold;
			style->italic = italic;
			style->underline = underline;
			style->strikeline = strikeline;
			return style;
		}

		Ptr<DocumentStyleProperties> MakeColorStyle(Color color)
		{
			auto style = Ptr(new DocumentStyleProperties);
			style->color = color;
			return style;
		}

		Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint begin, vint end)
		{
			auto b = TextPos(0, begin);
			auto e = TextPos(0, end);
			return textBox->SummarizeStyle(b, e);
		}
	}

## STEP 3: Implement the required “single paragraph” test cases using EditStyle + SummarizeStyle

In the single `TEST_CATEGORY` in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, add the required tests. All tests:

- Use `LoadTextAndClearUndoRedo(L"...")` with a single-paragraph string (no CRLF).
- Use `TextPos(0, begin/end)` ranges.
- Drive edit + verification inside `GacUIUnitTest_SetGuiMainProxy` idle frames, then `window->Hide()` to end the run.

Why:

- This isolates style-run behavior to a single paragraph and ensures overlap boundaries are only column-based.

### 3.1 One-call-per-property tests

- `Font_OneCall`: One `EditStyle` call that sets `DocumentStyleProperties::face` and/or `size` on a subrange; verify summary matches.
- `FontStyle_OneCall`: One `EditStyle` call that sets one or more of `bold/italic/underline/strikeline` on a subrange; verify summary matches.
- `Color_OneCall`: One `EditStyle` call that sets `color` on a subrange; verify summary matches.

Proposed assertions (sketch):

	auto summary = textBox->SummarizeStyle(TextPos(0, 2), TextPos(0, 5));
	TEST_ASSERT(summary->face);
	TEST_ASSERT(summary->face.Value() == WString::Unmanaged(L"Arial"));
	TEST_ASSERT(summary->size);
	TEST_ASSERT(summary->size.Value() == DocumentFontSize(16, false));
	TEST_ASSERT(summary->color);
	TEST_ASSERT(summary->color.Value() == Color(255, 0, 0));
	TEST_ASSERT(summary->bold);
	TEST_ASSERT(summary->bold.Value() == true);

### 3.2 One-range, multiple-properties test (one call per property)

- `AllProperties_SameRange`: On the same subrange, call `EditStyle` once for font, once for font style, once for color; verify the final `SummarizeStyle` result contains all edited fields.

Why:

- Ensures partial style updates combine correctly across successive edits (without requiring a single “set everything” call).

### 3.3 Overlap tests (small set)

Add a small set of overlap tests using multiple `EditStyle` calls where ranges overlap. Each overlap test should verify:

- The final style in each overlap region using `SummarizeStyle` on subranges that are expected to be uniform.
- A superset summary across multiple differently styled subranges yields `null` (empty `Nullable<T>`) for fields that are intentionally not uniform.

Proposed overlap cases:

1) `Overlap_SameProperty_FontFace`:
   - Edit face on `[1, 6)` to `FontA`
   - Edit face on `[4, 9)` to `FontB`
   - Verify `[1, 4)` is `FontA`, `[4, 9)` is `FontB`, and summarizing `[1, 9)` returns empty `face` (not uniform)

2) `Overlap_DifferentProperties_BoldThenColor`:
   - Edit `bold=true` on `[1, 6)`
   - Edit `color=Red` on `[4, 9)`
   - Verify:
     - `[1, 4)` has `bold=true` and no asserted `color`
     - `[4, 6)` has `bold=true` and `color=Red`
     - `[6, 9)` has `color=Red` and no asserted `bold`
   - Verify summarizing `[1, 9)` yields empty `bold` and empty `color`

3) `Overlap_SameProperty_BoldTrueThenFalse`:
   - Edit `bold=true` on `[1, 9)`
   - Edit `bold=false` on `[4, 6)`
   - Verify `[1, 4)` is `bold=true`, `[4, 6)` is `bold=false`, `[6, 9)` is `bold=true`

## STEP 4: Add run-level validation only if SummarizeStyle cannot prove correct splitting

If any overlap case cannot be adequately proven using `SummarizeStyle` (e.g. summary fields are empty on the minimal range due to implementation details), add a fallback check that inspects the underlying `Ptr<vl::presentation::DocumentModel>` from `GuiDocumentCommonInterface::GetDocument()` and validates that style-application runs are segmented at expected column boundaries.

Why:

- `SummarizeStyle` is the preferred stable API-level assertion.
- Run inspection should remain an “escape hatch” to avoid coupling most tests to internal run-tree shapes.

# TEST PLAN

All tests live in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` under one `TEST_CATEGORY`, and all log paths use `Controls/Editor/GuiDocumentLabel/RichText/SingleParagraph/...`.

- Font_OneCall: edit `[2,5)` with `face=Arial` and `size=16`; assert `SummarizeStyle([2,5))` has uniform `face/size`.
- FontStyle_OneCall: edit `[2,5)` with `bold=true` (and optionally `italic=true`); assert summary contains the edited flags.
- Color_OneCall: edit `[2,5)` with `color=Red`; assert summary contains uniform `color`.
- AllProperties_SameRange: apply font, then style, then color on `[2,5)`; assert final summary contains all edited fields.
- Overlap_SameProperty_FontFace: verify per-subrange face and that a spanning range produces empty `face`.
- Overlap_DifferentProperties_BoldThenColor: verify three regions and that spanning range produces empty `bold` and empty `color`.
- Overlap_SameProperty_BoldTrueThenFalse: verify three regions with explicit `bold=false` in the middle.

# !!!FINISHED!!!
