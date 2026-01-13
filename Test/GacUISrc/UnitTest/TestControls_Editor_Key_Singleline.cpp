#include "TestControls.h"

using namespace gacui_unittest_template;

template<typename TTextBox>
void RunTextBoxKeyTestCases_Singleline(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
	TEST_CATEGORY(L"Clipboard (Singleline)")
	{
		TEST_CASE(L"CtrlV_FlattensLineBreaks")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto clipboard = GetCurrentController()->ClipboardService();

					{
						auto writer = clipboard->WriteClipboard();
						writer->SetText(L"12\r\n34\n\r\n56");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"1234560123456789");
					TEST_ASSERT(textBox->GetText().IndexOf(L'\r') == -1);
					TEST_ASSERT(textBox->GetText().IndexOf(L'\n') == -1);
				});

				protocol->OnNextIdleFrame(L"Pasted", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_FlattensLineBreaks"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"UndoRedo (Singleline)")
	{
		TEST_CASE(L"CtrlZ_UndoesPaste_FlattenedText")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					protocol->TypeString(L"0123456789");
					textBox->ClearUndoRedo();
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto clipboard = GetCurrentController()->ClipboardService();
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
					TEST_ASSERT(textBox->GetText() == L"1234560123456789");
					TEST_ASSERT(textBox->GetText().IndexOf(L'\r') == -1);
					TEST_ASSERT(textBox->GetText().IndexOf(L'\n') == -1);
					protocol->KeyPress(VKEY::KEY_Z, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Undo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_Y, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Redo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"1234560123456789");
					TEST_ASSERT(textBox->GetText().IndexOf(L'\r') == -1);
					TEST_ASSERT(textBox->GetText().IndexOf(L'\n') == -1);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/UndoRedo_CtrlZ_UndoesPaste_FlattenedText"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Deletion (Singleline)")
	{
		TEST_CASE(L"Enter_Ignored_NoSelection_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_NoSelection_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Enter_Ignored_SelectionPreserved")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_SelectionPreserved"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlEnter_Ignored_NoSelection_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_NoSelection_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlEnter_Ignored_SelectionPreserved")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_SelectionPreserved"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}

template void RunTextBoxKeyTestCases_Singleline<GuiSinglelineTextBox>(const wchar_t* resource, const WString& controlName);