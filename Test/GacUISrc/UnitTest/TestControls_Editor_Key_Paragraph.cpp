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
}

template void RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentLabel>(const wchar_t* resource, const WString& controlName);
template void RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentViewer>(const wchar_t* resource, const WString& controlName);