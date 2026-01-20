# !!!EXECUTION!!!

# UPDATES

## UPDATE

Still need more cases to cover multiple EditStyle call overlapps just like the single paragraph test category.

## User Update Spotted

- The note above says more overlap cases are still needed, but `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` already contains additional multi-paragraph overlap test cases under `TEST_CATEGORY(L"MultiParagraph")`, including:
	- `Overlap_SameProperty_FontFace`
	- `Overlap_DifferentProperties_BoldThenColor`
	- `Overlap_SameProperty_BoldTrueThenFalse`

# EXECUTION PLAN

## STEP 1: Add a multi-paragraph Summarize helper overload [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

In the `namespace { ... }` helper section, keep the existing helper:

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(0, begin), TextPos(0, end));
	}

Add an overload right after it:

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(row, begin), TextPos(row, end));
	}

## STEP 2: Add a dedicated `MultiParagraph` test category [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Add a new `TEST_CATEGORY(L"MultiParagraph")` after the existing `TEST_CATEGORY(L"SingleParagraph")` (i.e. before the end of `TEST_FILE`):

	TEST_CATEGORY(L"MultiParagraph")
	{
		TEST_CASE(L"Font_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");
					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeFontFaceAndSizeStyle(WString::Unmanaged(L"Arial"), 16));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(!s0h->face);
						TEST_ASSERT(!s0h->size);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->face);
						TEST_ASSERT(s0t->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s0t->size);
						TEST_ASSERT(s0t->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->face);
						TEST_ASSERT(s1->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s1->size);
						TEST_ASSERT(s1->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->face);
						TEST_ASSERT(s2h->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s2h->size);
						TEST_ASSERT(s2h->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(!s2t->face);
						TEST_ASSERT(!s2t->size);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Font_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"FontStyle_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");
					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeTextStyle(true, true, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(!s0h->bold);
						TEST_ASSERT(!s0h->italic);
						TEST_ASSERT(!s0h->underline);
						TEST_ASSERT(!s0h->strikeline);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->bold);
						TEST_ASSERT(s0t->bold.Value() == true);
						TEST_ASSERT(s0t->italic);
						TEST_ASSERT(s0t->italic.Value() == true);
						TEST_ASSERT(s0t->underline);
						TEST_ASSERT(s0t->underline.Value() == false);
						TEST_ASSERT(s0t->strikeline);
						TEST_ASSERT(s0t->strikeline.Value() == false);
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->bold);
						TEST_ASSERT(s1->bold.Value() == true);
						TEST_ASSERT(s1->italic);
						TEST_ASSERT(s1->italic.Value() == true);
						TEST_ASSERT(s1->underline);
						TEST_ASSERT(s1->underline.Value() == false);
						TEST_ASSERT(s1->strikeline);
						TEST_ASSERT(s1->strikeline.Value() == false);
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->bold);
						TEST_ASSERT(s2h->bold.Value() == true);
						TEST_ASSERT(s2h->italic);
						TEST_ASSERT(s2h->italic.Value() == true);
						TEST_ASSERT(s2h->underline);
						TEST_ASSERT(s2h->underline.Value() == false);
						TEST_ASSERT(s2h->strikeline);
						TEST_ASSERT(s2h->strikeline.Value() == false);
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(!s2t->bold);
						TEST_ASSERT(!s2t->italic);
						TEST_ASSERT(!s2t->underline);
						TEST_ASSERT(!s2t->strikeline);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_FontStyle_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Color_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");
					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(!s0h->color);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->color);
						TEST_ASSERT(s0t->color.Value() == Color(255, 0, 0));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->color);
						TEST_ASSERT(s1->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->color);
						TEST_ASSERT(s2h->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(!s2t->color);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Color_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"AllProperties_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					auto style = Ptr(new DocumentStyleProperties);
					style->face = WString::Unmanaged(L"Arial");
					style->size = DocumentFontSize(16, false);
					style->bold = true;
					style->italic = false;
					style->underline = true;
					style->strikeline = false;
					style->color = Color(255, 0, 0);
					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), style);
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(!s0h->face);
						TEST_ASSERT(!s0h->size);
						TEST_ASSERT(!s0h->bold);
						TEST_ASSERT(!s0h->italic);
						TEST_ASSERT(!s0h->underline);
						TEST_ASSERT(!s0h->strikeline);
						TEST_ASSERT(!s0h->color);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->face);
						TEST_ASSERT(s0t->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s0t->size);
						TEST_ASSERT(s0t->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s0t->bold);
						TEST_ASSERT(s0t->bold.Value() == true);
						TEST_ASSERT(s0t->italic);
						TEST_ASSERT(s0t->italic.Value() == false);
						TEST_ASSERT(s0t->underline);
						TEST_ASSERT(s0t->underline.Value() == true);
						TEST_ASSERT(s0t->strikeline);
						TEST_ASSERT(s0t->strikeline.Value() == false);
						TEST_ASSERT(s0t->color);
						TEST_ASSERT(s0t->color.Value() == Color(255, 0, 0));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->face);
						TEST_ASSERT(s1->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s1->size);
						TEST_ASSERT(s1->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s1->bold);
						TEST_ASSERT(s1->bold.Value() == true);
						TEST_ASSERT(s1->italic);
						TEST_ASSERT(s1->italic.Value() == false);
						TEST_ASSERT(s1->underline);
						TEST_ASSERT(s1->underline.Value() == true);
						TEST_ASSERT(s1->strikeline);
						TEST_ASSERT(s1->strikeline.Value() == false);
						TEST_ASSERT(s1->color);
						TEST_ASSERT(s1->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->face);
						TEST_ASSERT(s2h->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s2h->size);
						TEST_ASSERT(s2h->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s2h->bold);
						TEST_ASSERT(s2h->bold.Value() == true);
						TEST_ASSERT(s2h->italic);
						TEST_ASSERT(s2h->italic.Value() == false);
						TEST_ASSERT(s2h->underline);
						TEST_ASSERT(s2h->underline.Value() == true);
						TEST_ASSERT(s2h->strikeline);
						TEST_ASSERT(s2h->strikeline.Value() == false);
						TEST_ASSERT(s2h->color);
						TEST_ASSERT(s2h->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(!s2t->face);
						TEST_ASSERT(!s2t->size);
						TEST_ASSERT(!s2t->bold);
						TEST_ASSERT(!s2t->italic);
						TEST_ASSERT(!s2t->underline);
						TEST_ASSERT(!s2t->strikeline);
						TEST_ASSERT(!s2t->color);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_AllProperties_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});
	}

## STEP 3: Add MultiParagraph overlap EditStyle cases [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

In the `TEST_CATEGORY(L"MultiParagraph")` block added in STEP 2, insert the following `TEST_CASE`s after `TEST_CASE(L"AllProperties_OneCall")` and before the closing `}` of the category:

		TEST_CASE(L"Overlap_SameProperty_FontFace")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 6), MakeFontFaceStyle(WString::Unmanaged(L"FontA")));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 9), MakeFontFaceStyle(WString::Unmanaged(L"FontB")));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0 = Summarize(textBox, 0, 1, 10);
						TEST_ASSERT(s0->face);
						TEST_ASSERT(s0->face.Value() == WString::Unmanaged(L"FontA"));
					}
					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->face);
						TEST_ASSERT(s1a->face.Value() == WString::Unmanaged(L"FontA"));
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->face);
						TEST_ASSERT(s1b->face.Value() == WString::Unmanaged(L"FontB"));
					}
					{
						auto s1m = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(!s1m->face);
					}
					{
						auto s2 = Summarize(textBox, 2, 0, 9);
						TEST_ASSERT(s2->face);
						TEST_ASSERT(s2->face.Value() == WString::Unmanaged(L"FontB"));
					}
					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						TEST_ASSERT(!s2t->face);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_SameProperty_FontFace"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_DifferentProperties_BoldThenColor")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 6), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 9), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->bold);
						TEST_ASSERT(s1a->bold.Value() == true);
						TEST_ASSERT(!s1a->color);
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->bold);
						TEST_ASSERT(s1b->bold.Value() == true);
						TEST_ASSERT(s1b->color);
						TEST_ASSERT(s1b->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2a = Summarize(textBox, 2, 0, 6);
						TEST_ASSERT(s2a->bold);
						TEST_ASSERT(s2a->bold.Value() == true);
						TEST_ASSERT(s2a->color);
						TEST_ASSERT(s2a->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2b = Summarize(textBox, 2, 6, 9);
						TEST_ASSERT(!s2b->bold);
						TEST_ASSERT(s2b->color);
						TEST_ASSERT(s2b->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2m = Summarize(textBox, 2, 0, 10);
						TEST_ASSERT(!s2m->bold);
						TEST_ASSERT(!s2m->color);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_DifferentProperties_BoldThenColor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_SameProperty_BoldTrueThenFalse")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 9), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 6), MakeTextStyle(false, false, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0 = Summarize(textBox, 0, 1, 10);
						TEST_ASSERT(s0->bold);
						TEST_ASSERT(s0->bold.Value() == true);
					}
					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->bold);
						TEST_ASSERT(s1a->bold.Value() == true);
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->bold);
						TEST_ASSERT(s1b->bold.Value() == false);
					}
					{
						auto s2a = Summarize(textBox, 2, 0, 6);
						TEST_ASSERT(s2a->bold);
						TEST_ASSERT(s2a->bold.Value() == false);
					}
					{
						auto s2b = Summarize(textBox, 2, 6, 9);
						TEST_ASSERT(s2b->bold);
						TEST_ASSERT(s2b->bold.Value() == true);
					}
					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						TEST_ASSERT(!s2t->bold);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_SameProperty_BoldTrueThenFalse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

## STEP 4: Verify log paths match conventions [DONE]

All new test cases must keep the log path prefix exactly:

	Controls/Editor/Features/RichText/MultiParagraph_<CaseName>

# FIXING ATTEMPTS

## Fixing attempt No.1

The run crashed in `MultiParagraph::Font_OneCall` on `TEST_ASSERT(!s0h->face)`. The test assumed that regions outside the edited range would have `Nullable<>` style fields unset (e.g. `face`/`size` are null). However, `SummarizeStyle` can legitimately return baseline/default font properties for unedited text, so asserting null is too strict and can fail depending on the theme/default font resolution.

I updated the multi-paragraph tests to capture baseline (pre-edit) style values for the "outside" ranges in the Init frame, and then assert those values remain unchanged after `EditStyle`. The styled ranges still assert the requested explicit properties (e.g. `Arial` + `16`). This keeps the tests validating the editing behavior without depending on whether the baseline style is represented as null or a concrete default value.

## Fixing attempt No.2

After rebuilding, unit tests still failed in `MultiParagraph::Overlap_SameProperty_FontFace` on `TEST_ASSERT(!s2t->face)`. This was the same root cause as attempt No.1: the tests assumed unedited/outside ranges always produce null `Nullable<>` fields from `SummarizeStyle`, but they can legitimately report baseline/default values.

I updated the overlap-focused multi-paragraph tests to capture baseline values for the properties that were being asserted as “unset” (e.g. paragraph-2 tail face, paragraph-1 pre-overlap color, paragraph-2 non-bold region bold flag) in the Init frame, and then assert those baseline values remain unchanged after the overlapping edits. Assertions for explicitly-edited ranges (FontA/FontB, color red, bold true/false) remain unchanged.

This should make the overlap tests stable across environments/themes while still validating that `EditStyle` only affects the intended ranges.

# !!!FINISHED!!!

# !!!VERIFIED!!!



# Comparing to User Edit

- When asserting â€œoutside edited rangeâ€ behavior, do not assume `SummarizeStyle(...)` returns `null` for every unset field; capture baseline summaries before applying edits and assert those baselines remain unchanged.
- When an UPDATE asks for â€œmore overlap casesâ€, align new coverage with the existing single-paragraph patterns (same kinds of overlap, but adapted to multi-paragraph ranges) and confirm what already exists before adding more to avoid duplicate/overlapping intent.
