#include "TestControls.h"

using namespace gacui_unittest_template;

template<typename TTextBox>
void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
	TEST_CATEGORY(L"Scaffold")
	{
		TEST_CASE(L"SmokeTest")
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
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"X");
					TEST_ASSERT(textBox->GetText() == L"012345678X9");
				});

				protocol->OnNextIdleFrame(L"Moved Cursor and Edited", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Scaffold_SmokeTest"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Navigation")
	{
		TEST_CASE(L"ArrowKeys_MoveCaret")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_LEFT);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 9));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 9));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_RIGHT);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_ArrowKeys_MoveCaret"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"HomeEnd_MoveCaretToEdges")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_HOME);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 0));

					protocol->KeyPress(VKEY::KEY_END);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_HomeEnd_MoveCaretToEdges"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"ShiftArrow_ExtendsSelection")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_LEFT, false, true, false);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetSelectionText() == L"4");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Extended Selection", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_ShiftArrow_ExtendsSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"ShiftHomeEnd_SelectToEdges")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_HOME, false, true, false);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetSelectionText() == L"01234");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					protocol->KeyPress(VKEY::KEY_END, false, true, false);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"56789");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Selected To Edges", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_ShiftHomeEnd_SelectToEdges"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"UpDown_NoEffectInSingleline")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_UP);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_DOWN);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_UpDown_NoEffectInSingleline"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"PageKeys_NoEffectInSingleline")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_PageKeys_NoEffectInSingleline"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlWithNavigation_DoesNotChangeBehavior")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_LEFT, true, false, false);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Navigation_CtrlWithNavigation_DoesNotChangeBehavior"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Typing")
	{
		TEST_CASE(L"TypeString_InsertsPlainText")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"abc");
				});

				protocol->OnNextIdleFrame(L"Typed abc", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"abc");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Typing_TypeString_InsertsPlainText"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"TypeString_InsertsTab_WhenAcceptTabInput")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					TEST_ASSERT(textBox->GetAcceptTabInput());
					protocol->TypeString(L"\t");
				});

				protocol->OnNextIdleFrame(L"Typed Tab", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"\t");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Typing_TypeString_InsertsTab_WhenAcceptTabInput"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"TypeString_IgnoresWhenCtrlPressed")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"012345");
				});

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					protocol->_KeyDown(VKEY::KEY_CONTROL);
					protocol->TypeString(L"abc");
					protocol->_KeyUp(VKEY::KEY_CONTROL);
					TEST_ASSERT(textBox->GetText() == L"012345");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Typing_TypeString_IgnoresWhenCtrlPressed"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"TypeString_ReplacesSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"012345");
				});

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 6), TextPos(0, 6));
					protocol->KeyPress(VKEY::KEY_LEFT, false, true, false);
					protocol->KeyPress(VKEY::KEY_LEFT, false, true, false);
					protocol->TypeString(L"X");
					TEST_ASSERT(textBox->GetText() == L"0123X");
				});

				protocol->OnNextIdleFrame(L"Selected and typed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Typing_TypeString_ReplacesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Clipboard")
	{
		TEST_CASE(L"CtrlC_CopiesSelection_TextUnchanged")
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

					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_C, true, false, false);

					auto reader = clipboard->ReadClipboard();
					window->SetText(reader->GetText());
					TEST_ASSERT(reader->ContainsText());
					TEST_ASSERT(reader->GetText() == L"234");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Copied", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlC_CopiesSelection_TextUnchanged"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlX_CutsSelection_TextUpdated")
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

					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");
					protocol->KeyPress(VKEY::KEY_X, true, false, false);

					auto reader = clipboard->ReadClipboard();
					window->SetText(reader->GetText());
					TEST_ASSERT(reader->ContainsText());
					TEST_ASSERT(reader->GetText() == L"234");
					TEST_ASSERT(textBox->GetText() == L"0156789");
				});

				protocol->OnNextIdleFrame(L"Cut", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlX_CutsSelection_TextUpdated"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_PastesAtCaret")
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
						writer->SetText(L"XX");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetCaret(TextPos(0, 3), TextPos(0, 3));
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"012XX3456789");
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
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_PastesAtCaret"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_ReplacesSelection")
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
						writer->SetText(L"XX");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"01XX56789");
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
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_ReplacesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

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

		TEST_CASE(L"CtrlV_EmptyClipboard_NoChange")
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
						TEST_ASSERT(writer->Submit());
					}
					TEST_ASSERT(!clipboard->ReadClipboard()->ContainsText());

					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_EmptyClipboard_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"UndoRedo")
	{
		TEST_CASE(L"CtrlZ_UndoesTyping_AndCtrlY_Redoes")
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
					textBox->NotifyModificationSaved();

					TEST_ASSERT(!textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());
					TEST_ASSERT(!textBox->GetModified());
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));
					protocol->TypeString(L"X");
				});

				protocol->OnNextIdleFrame(L"Type X", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					TEST_ASSERT(textBox->GetText() == L"0123456789X");
					TEST_ASSERT(textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());
					TEST_ASSERT(textBox->GetModified());
					protocol->KeyPress(VKEY::KEY_Z, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Undo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(!textBox->CanUndo());
					TEST_ASSERT(textBox->CanRedo());
					TEST_ASSERT(!textBox->GetModified());
					protocol->KeyPress(VKEY::KEY_Y, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Redo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					TEST_ASSERT(textBox->GetText() == L"0123456789X");
					TEST_ASSERT(textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());
					TEST_ASSERT(textBox->GetModified());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/UndoRedo_CtrlZ_UndoesTyping_AndCtrlY_Redoes"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlZ_UndoesDeletion")
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
					textBox->ClearUndoRedo();
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					protocol->KeyPress(VKEY::KEY_BACK);
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"012356789");
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
					TEST_ASSERT(textBox->GetText() == L"012356789");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/UndoRedo_CtrlZ_UndoesDeletion"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

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

		TEST_CASE(L"ClearUndoRedo_DeletesAllHistory")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();

					textBox->LoadTextAndClearUndoRedo(L"");
					TEST_ASSERT(!textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());

					protocol->TypeString(L"ABC");
					TEST_ASSERT(textBox->GetText() == L"ABC");
					TEST_ASSERT(textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					TEST_ASSERT(textBox->Undo());
					TEST_ASSERT(textBox->GetText() == L"AB");
					TEST_ASSERT(textBox->CanRedo());

					textBox->ClearUndoRedo();
					TEST_ASSERT(!textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());

					TEST_ASSERT(!textBox->Undo());
					TEST_ASSERT(!textBox->Redo());
					TEST_ASSERT(textBox->GetText() == L"AB");
				});

				protocol->OnNextIdleFrame(L"Undo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/UndoRedo_ClearUndoRedo_DeletesAllHistory"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"");

					protocol->TypeString(L"ABC");
					TEST_ASSERT(textBox->GetText() == L"ABC");
					TEST_ASSERT(textBox->CanUndo());
					TEST_ASSERT(!textBox->CanRedo());
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					while (textBox->CanUndo())
					{
						TEST_ASSERT(textBox->Undo());
					}
					TEST_ASSERT(!textBox->CanUndo());
					TEST_ASSERT(textBox->CanRedo());
					TEST_ASSERT(textBox->GetText() == L"");
					TEST_ASSERT(!textBox->Undo());
				});

				protocol->OnNextIdleFrame(L"Undo until the End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					while (textBox->CanRedo())
					{
						TEST_ASSERT(textBox->Redo());
					}
					TEST_ASSERT(!textBox->CanRedo());
					TEST_ASSERT(textBox->CanUndo());
					TEST_ASSERT(textBox->GetText() == L"ABC");
					TEST_ASSERT(!textBox->Redo());
				});

				protocol->OnNextIdleFrame(L"Redo until the end", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/UndoRedo_UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Deletion")
	{
		TEST_CASE(L"Backspace_DeletesPreviousChar")
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
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"012356789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetSelectionText() == L"");
				});

				protocol->OnNextIdleFrame(L"SetCaret and [BACKSPACE]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Backspace_DeletesPreviousChar"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Delete_DeletesNextChar")
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
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"012346789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");
				});

				protocol->OnNextIdleFrame(L"SetCaret and [DELETE]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Delete_DeletesNextChar"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"BackspaceAtStart_NoChange")
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
					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_BackspaceAtStart_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"DeleteAtEnd_NoChange")
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
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_DeleteAtEnd_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Backspace_DeletesSelection")
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
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");
				});

				protocol->OnNextIdleFrame(L"SetCaret and [BACKSPACE]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Backspace_DeletesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Delete_DeletesSelection")
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
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");
				});

				protocol->OnNextIdleFrame(L"SetCaret and [DELETE]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Delete_DeletesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

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

TEST_FILE
{
	const auto resource_SinglelineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Key Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentTextBox Test" ClientSize="x:480 y:320">
        <DocumentTextBox ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </DocumentTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentTextBox")
	{
		RunSinglelineTextBoxTestCases<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	});
}
