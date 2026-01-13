#include "TestControls.h"

using namespace gacui_unittest_template;

template<typename TTextBox>
void RunTextBoxKeyTestCases_Multiline(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
	TEST_CATEGORY(L"Enter/CRLF (Multiline)")
	{
		TEST_CASE(L"Enter_SplitsIntoTwoLines")
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
				+ WString::Unmanaged(L"/Key/EnterCRLF_Enter_SplitsIntoTwoLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlEnter_BehavesSameAsEnter")
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
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"CDE");
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
				+ WString::Unmanaged(L"/Key/EnterCRLF_CtrlEnter_BehavesSameAsEnter"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Clipboard (Multiline)")
	{
		TEST_CASE(L"CtrlV_PreservesMultipleLines")
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
						writer->SetText(L"12\r\n34\n\r\n56");
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
					TEST_ASSERT(document->paragraphs.Count() == 4);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"12");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"34");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"");
					TEST_ASSERT(document->paragraphs[3]->GetTextForReading() == L"56");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_PreservesMultipleLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlC_CtrlV_Roundtrip_MultipleLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto clipboard = GetCurrentController()->ClipboardService();
					{
						auto writer = clipboard->WriteClipboard();
						writer->SetText(L"12\r\n34\n\r\n56");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetFocused();
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Pasted", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
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
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 4);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"12");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"34");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"");
					TEST_ASSERT(document->paragraphs[3]->GetTextForReading() == L"56");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlC_CtrlV_Roundtrip_MultipleLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Navigation (Multiline)")
	{
		TEST_CASE(L"LeftRight_Boundary_JumpsAcrossLines")
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

				protocol->OnNextIdleFrame(L"Typed 3 lines", [=]()
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
					textBox->SetCaret(TextPos(1, 3), TextPos(1, 3));
					protocol->KeyPress(VKEY::KEY_RIGHT);
					protocol->TypeString(L"|");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"|CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,3) and [RIGHT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_LeftRight_Boundary_JumpsAcrossLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"UpDown_MovesAcrossLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAAAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBBBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCCCC");
				});

				protocol->OnNextIdleFrame(L"Typed 3 lines", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->TypeString(L"^");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AA^AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBBBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCCCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,2) and [UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->TypeString(L"v");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AA^AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBBBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCvCCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,2) and [DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_UpDown_MovesAcrossLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"HomeEnd_Escalation_Line_Paragraph_Document")
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

				protocol->OnNextIdleFrame(L"Typed 3 lines", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"|");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"|BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,1) and [HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"!");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"!AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"|BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,0) and [HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"^");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"!AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"|BBB^");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,2) and [END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 5), TextPos(1, 5));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"$");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"!AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"|BBB^");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC$");
				});

				protocol->OnNextIdleFrame(L"Caret at (1,5) and [END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_HomeEnd_Escalation_Line_Paragraph_Document"),
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

				protocol->OnNextIdleFrame(L"Typed 3 lines", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
					protocol->TypeString(L"|");

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"|AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
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
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"|AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC#");
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
				+ WString::Unmanaged(L"/Key/NavigationMultiline_CtrlHomeEnd_JumpsToDocumentEdges"),
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
						protocol->TypeString(WString::Unmanaged(L"L") + itow(i));
						if (i != 39)
						{
							protocol->KeyPress(VKEY::KEY_RETURN);
						}
					}
				});

				protocol->OnNextIdleFrame(L"Typed 40 lines", [=]()
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
				+ WString::Unmanaged(L"/Key/NavigationMultiline_PageUpPageDown_MovesVerticallyByViewport"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}

template void RunTextBoxKeyTestCases_Multiline<GuiMultilineTextBox>(const wchar_t* resource, const WString& controlName);