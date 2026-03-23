# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Identify insertion point in existing tests

Change file:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Locate the existing category nesting:
- `TEST_CATEGORY(L"Styles")`
  - `TEST_CATEGORY(L"EditStyleName")`
    - `TEST_CATEGORY(L"SingleParagraph")`

Add a sibling category right after `SingleParagraph`:
- `TEST_CATEGORY(L"MultiParagraph")`

Why:
- Existing coverage only exercises row 0. `vl::presentation::controls::GuiDocumentCommonInterface::EditStyleName` delegates to the document element which iterates paragraph-by-paragraph; we need explicit cross-row coverage.

## STEP 2: Implement MultiParagraph unit tests for EditStyleName

### 2.1 Add: Apply a style across paragraphs

Insert the following block under `TEST_CATEGORY(L"EditStyleName")` (as a sibling of `SingleParagraph`) and reuse the existing helpers in this file (`RegisterStyle`, `SummarizeName`).

```cpp
			TEST_CATEGORY(L"MultiParagraph")
			{
				TEST_CASE(L"AcrossParagraphs")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();
							textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 5), TextPos(2, 5), WString::Unmanaged(L"MyBold"));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							// Paragraph 0: [0,5) unstyled, [5,10) styled
							TEST_ASSERT(!SummarizeName(textBox, 0, 0, 5));
							auto p0 = SummarizeName(textBox, 0, 5, 10);
							TEST_ASSERT(p0);
							TEST_ASSERT(p0.Value() == WString::Unmanaged(L"MyBold"));

							// Paragraph 1: fully styled
							auto p1 = SummarizeName(textBox, 1, 0, 10);
							TEST_ASSERT(p1);
							TEST_ASSERT(p1.Value() == WString::Unmanaged(L"MyBold"));

							// Paragraph 2: [0,5) styled, [5,10) unstyled
							auto p2h = SummarizeName(textBox, 2, 0, 5);
							TEST_ASSERT(p2h);
							TEST_ASSERT(p2h.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(!SummarizeName(textBox, 2, 5, 10));

							// Cross-paragraph summarization sanity checks
							auto mid = textBox->SummarizeStyleName(TextPos(0, 5), TextPos(2, 5));
							TEST_ASSERT(mid);
							TEST_ASSERT(mid.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(!textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10)));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_AcrossParagraphs"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

```

Why:
- This asserts head/tail preservation across paragraph boundaries and verifies the middle paragraph becomes fully covered, matching how multi-row edits should behave.

### 2.2 Add: Apply a style to entire multi-paragraph document

In the same `TEST_CATEGORY(L"MultiParagraph")`, add:

```cpp
				TEST_CASE(L"FullDocument")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();
							textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(2, 10), WString::Unmanaged(L"MyBold"));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							for (vint row = 0; row < 3; row++)
							{
								auto s = SummarizeName(textBox, row, 0, 10);
								TEST_ASSERT(s);
								TEST_ASSERT(s.Value() == WString::Unmanaged(L"MyBold"));
							}

							auto all = textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10));
							TEST_ASSERT(all);
							TEST_ASSERT(all.Value() == WString::Unmanaged(L"MyBold"));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_FullDocument"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});
```

Why:
- This ensures the implementation handles the full multi-row range and that style summarization remains uniform for the entire document.

Notes / guardrails:
- Avoid empty ranges (`begin == end`) because summarize APIs return empty for empty ranges.
- When asserting “unstyled”, use `TEST_ASSERT(!SummarizeName(...))` (nullable-empty) instead of comparing to a string.

## STEP 3: Verification (test plan)

1. Build `REPO-ROOT\Test\GacUISrc` with `Debug|x64` following `REPO-ROOT\.github\Guidelines\Building.md` (use the repo’s wrapper scripts, do not invoke msbuild directly).
2. Run UnitTest project `UnitTest` with `Debug|x64` following `REPO-ROOT\.github\Guidelines\Running-UnitTest.md`.
3. Confirm the two new tests execute and pass:
   - `Styles/EditStyleName/MultiParagraph/AcrossParagraphs`
   - `Styles/EditStyleName/MultiParagraph/FullDocument`
4. If failures occur, use paragraph-local assertions first (already in the proposed tests) to localize which row/range is incorrect.

# !!!FINISHED!!!
