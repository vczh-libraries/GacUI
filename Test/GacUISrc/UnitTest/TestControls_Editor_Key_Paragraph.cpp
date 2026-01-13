#include "TestControls.h"

using namespace gacui_unittest_template;

template<typename TTextBox>
void RunTextBoxKeyTestCases_MultiParagraph(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
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
		});
	});

	TEST_CATEGORY(L"Clipboard (Paragraph)")
	{
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
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_SingleCRLF_StaysInParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_DoubleCRLF_CreatesNewParagraph")
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
						writer->SetText(L"a\r\n\r\nb");
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
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"b");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_DoubleCRLF_CreatesNewParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_TripleCRLF_FollowsParserRule")
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
						writer->SetText(L"a\r\n\r\n\r\nb");
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
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"\r\nb");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_TripleCRLF_FollowsParserRule"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

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
					textBox->SetFocused();
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Pasted Again", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto after = textBox->GetDocument();
					TEST_ASSERT(after->paragraphs.Count() == 2);
					TEST_ASSERT(after->paragraphs[0]->GetTextForReading() == L"AB\r\nCD");
					TEST_ASSERT(after->paragraphs[1]->GetTextForReading() == L"EF");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlC_CtrlV_Roundtrip_PreservesParagraphAndLineBreakStructure"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Navigation (Paragraph)")
	{
		TEST_CASE(L"LeftRight_Boundary_JumpsAcrossParagraphs")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCC");
				});

				protocol->OnNextIdleFrame(L"Typed 3 paragraphs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"|");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,0) and [LEFT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p0 = document->paragraphs[0]->GetTextForReading();
					textBox->SetCaret(TextPos(0, p0.Length()), TextPos(0, p0.Length()));

					protocol->KeyPress(VKEY::KEY_RIGHT);
					protocol->TypeString(L"^");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"^BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at end of (0) and [RIGHT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_LeftRight_Boundary_JumpsAcrossParagraphs"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"UpDown_MovesAcrossLinesAndParagraphs")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					// Paragraph 0: "AAA\r\nBBB"
					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					// Paragraph 1: "CCC\r\nDDD"
					protocol->TypeString(L"CCC");
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					protocol->TypeString(L"DDD");
				});

				protocol->OnNextIdleFrame(L"Typed 2 paragraphs with line breaks", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p0 = document->paragraphs[0]->GetTextForReading();
					auto crlf0 = p0.IndexOf(L"\r\n");
					TEST_ASSERT(crlf0 != -1);

					// Place caret on the second line of paragraph 0.
					textBox->SetCaret(TextPos(0, crlf0 + 2 + 1), TextPos(0, crlf0 + 2 + 1));
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->TypeString(L"^");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 2);
					// Marker should be inserted into the first line (before the CRLF).
					auto updated0 = document->paragraphs[0]->GetTextForReading();
					TEST_ASSERT(updated0.IndexOf(L"^") != -1);
					TEST_ASSERT(updated0.IndexOf(L"^") < updated0.IndexOf(L"\r\n"));
				});

				protocol->OnNextIdleFrame(L"Caret in p0 line2 and [UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p0 = document->paragraphs[0]->GetTextForReading();
					auto crlf0 = p0.IndexOf(L"\r\n");
					TEST_ASSERT(crlf0 != -1);

					// Place caret on the last line of paragraph 0.
					textBox->SetCaret(TextPos(0, crlf0 + 2 + 1), TextPos(0, crlf0 + 2 + 1));
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->TypeString(L"v");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 2);
					// Marker should be inserted in paragraph 1 (moved across paragraph boundary).
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading().IndexOf(L'v') != -1);
				});

				protocol->OnNextIdleFrame(L"Caret in p0 line2 and [DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_UpDown_MovesAcrossLinesAndParagraphs"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Home_Escalation_Line_Paragraph_Document")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					// Paragraph 0: "P0"
					protocol->TypeString(L"P0");
					protocol->KeyPress(VKEY::KEY_RETURN);

					// Paragraph 1: "AB\r\nCD"
					protocol->TypeString(L"AB");
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					protocol->TypeString(L"CD");
					protocol->KeyPress(VKEY::KEY_RETURN);

					// Paragraph 2: "P2"
					protocol->TypeString(L"P2");
				});

				protocol->OnNextIdleFrame(L"Typed 3 paragraphs (middle has 2 lines)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p1 = document->paragraphs[1]->GetTextForReading();
					TEST_ASSERT(p1.IndexOf(L"\r\n") != -1);

					// HOME (1): end of paragraph -> start of last line.
					textBox->SetCaret(TextPos(1, p1.Length()), TextPos(1, p1.Length()));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"|");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"AB\r\n|CD");
				});

				protocol->OnNextIdleFrame(L"Caret at end of p1 and [HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p1 = document->paragraphs[1]->GetTextForReading();
					auto crlf = p1.IndexOf(L"\r\n");
					TEST_ASSERT(crlf != -1);

					// HOME (2): at start of line -> escalate to paragraph start.
					textBox->SetCaret(TextPos(1, crlf + 2), TextPos(1, crlf + 2));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"!");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading().Left(1) == L"!");
				});

				protocol->OnNextIdleFrame(L"Caret at start of p1 and [HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					// HOME (3): at paragraph start -> escalate to document start.
					textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"$");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading().Left(1) == L"$");
				});

				protocol->OnNextIdleFrame(L"Pressed [HOME] at paragraph start", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_Home_Escalation_Line_Paragraph_Document"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"End_Escalation_Line_Paragraph_Document")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					// Paragraph 0: "P0"
					protocol->TypeString(L"P0");
					protocol->KeyPress(VKEY::KEY_RETURN);

					// Paragraph 1: "AB\r\nCD"
					protocol->TypeString(L"AB");
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					protocol->TypeString(L"CD");
					protocol->KeyPress(VKEY::KEY_RETURN);

					// Paragraph 2: "P2"
					protocol->TypeString(L"P2");
				});

				protocol->OnNextIdleFrame(L"Typed 3 paragraphs (middle has 2 lines)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					// END (1): middle of first line -> end of that line.
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"^");

					auto document = textBox->GetDocument();
					auto p1 = document->paragraphs[1]->GetTextForReading();
					auto crlf = p1.IndexOf(L"\r\n");
					TEST_ASSERT(crlf != -1);
					TEST_ASSERT(p1.IndexOf(L'^') != -1);
					TEST_ASSERT(p1.IndexOf(L'^') < crlf);
				});

				protocol->OnNextIdleFrame(L"Caret in p1 line1 and [END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p1 = document->paragraphs[1]->GetTextForReading();
					auto crlf = p1.IndexOf(L"\r\n");
					TEST_ASSERT(crlf != -1);

					// END (2): at end of line -> escalate to paragraph end.
					textBox->SetCaret(TextPos(1, crlf), TextPos(1, crlf));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"$");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading().Right(1) == L"$");
				});

				protocol->OnNextIdleFrame(L"Caret at end of p1 and [END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto document = textBox->GetDocument();
					auto p1 = document->paragraphs[1]->GetTextForReading();

					// END (3): at paragraph end -> escalate to document end (paragraph 2 end).
					textBox->SetCaret(TextPos(1, p1.Length()), TextPos(1, p1.Length()));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"#");

					document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading().Right(1) == L"#");
				});

				protocol->OnNextIdleFrame(L"Pressed [END] at paragraph end", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_End_Escalation_Line_Paragraph_Document"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlHomeEnd_JumpsToDocumentEdges")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCC");
				});

				protocol->OnNextIdleFrame(L"Typed 3 paragraphs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
					protocol->TypeString(L"|");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"|AAA");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,1) and [CTRL+HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_END, true, false, false);
					protocol->TypeString(L"#");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading().Right(1) == L"#");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,1) and [CTRL+END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_CtrlHomeEnd_JumpsToDocumentEdges"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"PageUpPageDown_MovesVerticallyByViewport")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					for (vint i = 0; i < 40; i++)
					{
						protocol->TypeString(WString::Unmanaged(L"P") + itow(i));
						if (i != 39)
						{
							protocol->KeyPress(VKEY::KEY_RETURN);
						}
					}
				});

				protocol->OnNextIdleFrame(L"Typed 40 paragraphs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(30, 1), TextPos(30, 1));
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->TypeString(L"^");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 40);

					vint index = -1;
					for (vint i = 0; i < document->paragraphs.Count(); i++)
					{
						if (document->paragraphs[i]->GetTextForReading().IndexOf(L'^') != -1)
						{
							index = i;
							break;
						}
					}
					TEST_ASSERT(index != -1);
					TEST_ASSERT(index < 30);
				});

				protocol->OnNextIdleFrame(L"Caret at (30,1) and [PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(5, 1), TextPos(5, 1));
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->TypeString(L"v");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 40);

					vint index = -1;
					for (vint i = 0; i < document->paragraphs.Count(); i++)
					{
						if (document->paragraphs[i]->GetTextForReading().IndexOf(L'v') != -1)
						{
							index = i;
							break;
						}
					}
					TEST_ASSERT(index != -1);
					TEST_ASSERT(index > 5);
				});

				protocol->OnNextIdleFrame(L"Caret at (5,1) and [PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationParagraph_PageUpPageDown_MovesVerticallyByViewport"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}

template void RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentLabel>(const wchar_t* resource, const WString& controlName);
template void RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentViewer>(const wchar_t* resource, const WString& controlName);