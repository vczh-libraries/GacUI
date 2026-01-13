# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Confirm target controls and existing helper patterns

- Target controls in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp:
  - GuiDocumentTextBox
  - GuiDocumentLabel
  - GuiDocumentViewer
- Confirm existing helper patterns to mirror:
  - RunTextBoxKeyTestCases_Singleline<TTextBox>(...) (structure, frame naming, clipboard usage)
  - RunTextBoxKeyTestCases_Multiline<TTextBox>(...) (uses GetDocument() + GetTextForReading())

Why this step is necessary:
- The new helper must match the existing test style (GacUIUnitTest_SetGuiMainProxy + OnNextIdleFrame) so logs are readable and tests are stable.
- The task requires all new content assertions to use the document model (GetDocument + GetTextForReading), which the multiline helper already demonstrates.

## STEP 2: Add a Paragraph-mode helper for ENTER/CRLF semantics

Change:
- Implement a new helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp:
  - Name: RunTextBoxKeyTestCases_MultiParagraph
  - Form: a template helper (same style as existing helpers) so it can be reused for the three Paragraph-mode controls.

Proposed signature:

    template<typename TTextBox>
    void RunTextBoxKeyTestCases_MultiParagraph(const wchar_t* resource, const WString& controlName)
    	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)

Why this change is necessary:
- The task scope is “ENTER/CRLF behaviors only” for Paragraph-mode controls, and it explicitly asks for a dedicated helper (RunTextBoxKeyTestCases_MultiParagraph).
- A template helper keeps the three control categories consistent and avoids copy/paste divergence.

## STEP 3: Add ENTER vs CTRL+ENTER tests (Paragraph mode)

Change:
- In RunTextBoxKeyTestCases_MultiParagraph, add a TEST_CATEGORY(L"Enter/CRLF (Paragraph)") with at least these cases.
- All assertions about content must go through:
  - auto document = textBox->GetDocument();
  - document->paragraphs.Count()
  - document->paragraphs[i]->GetTextForReading()

Planned test case: Enter creates a new paragraph

    TEST_CATEGORY(L"Enter/CRLF (Paragraph)")
    {
    	TEST_CASE(L"Enter_CreatesNewParagraph")
    	{
    		TooltipTimer timer;
    		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    		{
    			protocol->OnNextIdleFrame(L"Ready", [=]()
    			{
    				auto window = GetApplication()->GetMainWindow();
    				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    				textBox->SetFocused();
    				protocol->TypeString(L"ABCDE");
    			});

    			protocol->OnNextIdleFrame(L"Typed ABCDE", [=]()
    			{
    				auto window = GetApplication()->GetMainWindow();
    				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    				textBox->SetCaret(TextPos(0, 2), TextPos(0, 2));
    				protocol->KeyPress(VKEY::KEY_RETURN);

    				auto document = textBox->GetDocument();
    				TEST_ASSERT(document->paragraphs.Count() == 2);
    				TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB");
    				TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"CDE");
    			});

    			protocol->OnNextIdleFrame(L"Pressed Enter", [=]()
    			{
    				auto window = GetApplication()->GetMainWindow();
    				window->Hide();
    			});
    		});

    		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    			WString::Unmanaged(L"Controls/Editor/")
    			+ controlName
    			+ WString::Unmanaged(L"/Key/EnterCRLF_Paragraph_Enter_CreatesNewParagraph"),
    			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    			resource
    		);
    	});
    }

Planned test case: Ctrl+Enter inserts a line break inside the paragraph

    TEST_CASE(L"CtrlEnter_InsertsLineBreakInParagraph")
    {
    	TooltipTimer timer;
    	GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    	{
    		protocol->OnNextIdleFrame(L"Ready", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			textBox->SetFocused();
    			protocol->TypeString(L"ABCDE");
    		});

    		protocol->OnNextIdleFrame(L"Typed ABCDE", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			textBox->SetCaret(TextPos(0, 2), TextPos(0, 2));
    			protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);

    			auto document = textBox->GetDocument();
    			TEST_ASSERT(document->paragraphs.Count() == 1);
    			TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB\r\nCDE");
    		});

    		protocol->OnNextIdleFrame(L"Pressed Ctrl+Enter", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			window->Hide();
    		});
    	});

    	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    		WString::Unmanaged(L"Controls/Editor/")
    		+ controlName
    		+ WString::Unmanaged(L"/Key/EnterCRLF_Paragraph_CtrlEnter_InsertsLineBreakInParagraph"),
    		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    		resource
    	);
    }

Why this change is necessary:
- In Paragraph mode, ENTER and CTRL+ENTER are intentionally different behaviors. These are the main semantics that are not covered by the existing multiline tests.
- Using GetDocument() validates the underlying model (paragraph boundaries vs in-paragraph line breaks) without depending on any normalized GetText().

## STEP 4: Add paste parsing tests for CRLF sequences (Paragraph mode)

Change:
- Add a TEST_CATEGORY(L"Clipboard (Paragraph)") in RunTextBoxKeyTestCases_MultiParagraph.
- Use clipboard writer->SetText(...) to feed exact CRLF sequences.
- Paste using protocol->KeyPress(VKEY::KEY_V, true, false, false).
- Verify the resulting document structure with GetDocument() + GetTextForReading().

Planned test cases:

1) Single CRLF stays inside a paragraph

    TEST_CASE(L"CtrlV_SingleCRLF_StaysInParagraph")
    {
    	TooltipTimer timer;
    	GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    	{
    		protocol->OnNextIdleFrame(L"Ready", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			auto clipboard = GetCurrentController()->ClipboardService();
    			textBox->SetFocused();

    			{
    				auto writer = clipboard->WriteClipboard();
    				writer->SetText(L"a\r\nb");
    				TEST_ASSERT(writer->Submit());
    			}

    			textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
    			protocol->KeyPress(VKEY::KEY_V, true, false, false);
    		});

    		protocol->OnNextIdleFrame(L"Pasted", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			auto document = textBox->GetDocument();
    			TEST_ASSERT(document->paragraphs.Count() == 1);
    			TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a\r\nb");
    		});

    		protocol->OnNextIdleFrame(L"Verified", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			window->Hide();
    		});
    	});
    	...
    }

2) Double CRLF creates a paragraph break
- Input: L"a\r\n\r\nb"
- Expect:
  - paragraphs.Count() == 2
  - paragraph[0] == L"a"
  - paragraph[1] == L"b"

3) Three consecutive CRLF follows the parser rule
- Input: L"a\r\n\r\n\r\nb"
- Expect:
  - paragraphs.Count() == 2
  - paragraph[0] == L"a"
  - paragraph[1] == L"\r\nb" (second paragraph begins with an empty line, then b)

Why this change is necessary:
- Paragraph-mode controls treat CRLF sequences differently than multiline controls:
  - Single CRLF is an in-paragraph line break.
  - Double CRLF is a paragraph break.
- These tests ensure the formatting/parsing logic used by paste aligns with the documented behavior and stays correct across refactors.

## STEP 5: Add copy/paste roundtrip test that preserves paragraph and line-break structure

Change:
- Add a test that constructs a mixed structure using keyboard input (so it is independent from clipboard parsing):
  - Create an in-paragraph line break using CTRL+ENTER.
  - Create a paragraph break using ENTER.
  - Select all, copy, delete, paste.
  - Verify the post-paste document model matches the pre-copy model.

Proposed structure (core assertions):

    TEST_CASE(L"CtrlC_CtrlV_Roundtrip_PreservesParagraphAndLineBreakStructure")
    {
    	TooltipTimer timer;
    	GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    	{
    		protocol->OnNextIdleFrame(L"Ready", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			textBox->SetFocused();
    			protocol->TypeString(L"AB");
    			protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
    			protocol->TypeString(L"CD");
    			protocol->KeyPress(VKEY::KEY_RETURN);
    			protocol->TypeString(L"EF");
    		});

    		protocol->OnNextIdleFrame(L"Typed 2 paragraphs with a line break", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

    			auto before = textBox->GetDocument();
    			TEST_ASSERT(before->paragraphs.Count() == 2);
    			TEST_ASSERT(before->paragraphs[0]->GetTextForReading() == L"AB\r\nCD");
    			TEST_ASSERT(before->paragraphs[1]->GetTextForReading() == L"EF");

    			protocol->KeyPress(VKEY::KEY_A, true, false, false);
    			protocol->KeyPress(VKEY::KEY_C, true, false, false);
    			protocol->KeyPress(VKEY::KEY_DELETE);
    		});

    		protocol->OnNextIdleFrame(L"Copied and Cleared", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			protocol->KeyPress(VKEY::KEY_V, true, false, false);

    			auto after = textBox->GetDocument();
    			TEST_ASSERT(after->paragraphs.Count() == 2);
    			TEST_ASSERT(after->paragraphs[0]->GetTextForReading() == L"AB\r\nCD");
    			TEST_ASSERT(after->paragraphs[1]->GetTextForReading() == L"EF");
    		});

    		protocol->OnNextIdleFrame(L"Pasted Again", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			window->Hide();
    		});
    	});

    	...
    }

Why this change is necessary:
- It validates the control’s copy format / paste logic preserves both:
  - paragraph boundaries (ENTER)
  - in-paragraph line breaks (CTRL+ENTER)
- It catches regressions where copy/paste would flatten structure into plain text or misinterpret CRLF sequences.

## STEP 6: Wire the helper into the three Paragraph-mode control categories

Change:
- In the TEST_FILE section of Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp, update these categories to call the new helper:
  - GuiDocumentTextBox
  - GuiDocumentLabel
  - GuiDocumentViewer

Planned calls:

    TEST_CATEGORY(L"GuiDocumentTextBox")
    {
    	RunTextBoxKeyTestCases<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
    	RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
    }

    TEST_CATEGORY(L"GuiDocumentLabel")
    {
    	RunTextBoxKeyTestCases<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
    	RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
    }

    TEST_CATEGORY(L"GuiDocumentViewer")
    {
    	RunTextBoxKeyTestCases<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
    	RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
    }

Why this change is necessary:
- The task explicitly requires extending coverage to “the other three editor controls” in this file.
- Centralizing via RunTextBoxKeyTestCases_MultiParagraph ensures identical ENTER/CRLF expectations across these controls.
- Using the correct template parameter for each category ensures FindObjectByName<TTextBox>(...) resolves the actual control type created by the XML resource.

## STEP 7: Validation plan (unit tests)

- Build and run unit tests using the existing VS Code tasks:
  - Build Unit Tests
  - Run Unit Tests
- If a failure happens, inspect frame logs to identify which semantics regressed:
  - ENTER paragraph split vs CTRL+ENTER line break
  - paste parsing (1/2/3 CRLF)
  - copy/paste roundtrip

Why this step is necessary:
- These tests are UI-driven with frame capture; validation must happen via the normal unit-test runner to ensure timing and protocol behavior are correct.

# !!!FINISHED!!!
