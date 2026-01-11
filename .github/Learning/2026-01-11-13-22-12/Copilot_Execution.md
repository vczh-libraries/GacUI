# !!!EXECUTION!!!

# UPDATES

## UPDATE

For a redo or undo shortcut, if it changes the text box, I would like a new frame after assertion

# EXECUTION PLAN

## STEP 1: Add `TEST_CATEGORY(L"UndoRedo")` under `RunSinglelineTextBoxTestCases` [DONE]

Edit file: `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.

Location: inside `RunSinglelineTextBoxTestCases<TTextBox>(...)`, insert the new category between `TEST_CATEGORY(L"Clipboard")` and `TEST_CATEGORY(L"Deletion")`.

	TEST_CATEGORY(L"UndoRedo")
	{
		// TEST_CASEs will be added here
	});

## STEP 2: Add shortcut tests for Ctrl+Z / Ctrl+Y across representative edits [DONE]

Insert the following code inside `TEST_CATEGORY(L"UndoRedo")`.

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
				TEST_ASSERT(textBox->GetText() == L"0123456789X");
				TEST_ASSERT(textBox->CanUndo());
				TEST_ASSERT(!textBox->CanRedo());
				TEST_ASSERT(textBox->GetModified());
				protocol->KeyPress(VKEY::KEY_Z, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Undid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				TEST_ASSERT(textBox->GetText() == L"0123456789");
				TEST_ASSERT(!textBox->CanUndo());
				TEST_ASSERT(textBox->CanRedo());
				TEST_ASSERT(!textBox->GetModified());
			});

			protocol->OnNextIdleFrame(L"RedoShortcut", [=]()
			{
				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Redid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				TEST_ASSERT(textBox->GetText() == L"0123456789X");
				TEST_ASSERT(textBox->CanUndo());
				TEST_ASSERT(!textBox->CanRedo());
				TEST_ASSERT(textBox->GetModified());

				window->SetText(textBox->GetText());
			});

			protocol->OnNextIdleFrame(L"UndoRedo", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
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
				textBox->ClearUndoRedo();
			});

			protocol->OnNextIdleFrame(L"Typing", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
				protocol->KeyPress(VKEY::KEY_BACK);
				TEST_ASSERT(textBox->GetText() == L"012356789");
				protocol->KeyPress(VKEY::KEY_Z, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Undid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"0123456789");
			});

			protocol->OnNextIdleFrame(L"RedoShortcut", [=]()
			{
				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Redid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"012356789");
			});

			protocol->OnNextIdleFrame(L"UndoRedo", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
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
				TEST_ASSERT(textBox->GetText() == L"1234560123456789");
				TEST_ASSERT(textBox->GetText().IndexOf(L'\r') == -1);
				TEST_ASSERT(textBox->GetText().IndexOf(L'\n') == -1);
				protocol->KeyPress(VKEY::KEY_Z, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Undid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"0123456789");
			});

			protocol->OnNextIdleFrame(L"RedoShortcut", [=]()
			{
				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
			});

			protocol->OnNextIdleFrame(L"Redid", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"1234560123456789");
				TEST_ASSERT(textBox->GetText().IndexOf(L'\r') == -1);
				TEST_ASSERT(textBox->GetText().IndexOf(L'\n') == -1);
			});

			protocol->OnNextIdleFrame(L"UndoRedo", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
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

## STEP 3: Add API-level tests for `ClearUndoRedo()`, `LoadTextAndClearUndoRedo(...)`, and history-end behavior [DONE]

Insert the following code inside `TEST_CATEGORY(L"UndoRedo")`.

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
			});

			protocol->OnNextIdleFrame(L"Typing", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				protocol->TypeString(L"ABC");
				TEST_ASSERT(textBox->GetText() == L"ABC");
				TEST_ASSERT(textBox->CanUndo());
				TEST_ASSERT(!textBox->CanRedo());

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

			protocol->OnNextIdleFrame(L"Cleared", [=]()
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
			});

			protocol->OnNextIdleFrame(L"Typing", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				protocol->TypeString(L"ABC");
				TEST_ASSERT(textBox->GetText() == L"ABC");
				TEST_ASSERT(textBox->CanUndo());
				TEST_ASSERT(!textBox->CanRedo());

				while (textBox->CanUndo())
				{
					TEST_ASSERT(textBox->Undo());
				}
				TEST_ASSERT(!textBox->CanUndo());
				TEST_ASSERT(textBox->CanRedo());
				TEST_ASSERT(textBox->GetText() == L"");
				TEST_ASSERT(!textBox->Undo());

				while (textBox->CanRedo())
				{
					TEST_ASSERT(textBox->Redo());
				}
				TEST_ASSERT(!textBox->CanRedo());
				TEST_ASSERT(textBox->CanUndo());
				TEST_ASSERT(textBox->GetText() == L"ABC");
				TEST_ASSERT(!textBox->Redo());

				window->SetText(textBox->GetText());
			});

			protocol->OnNextIdleFrame(L"Exhausted", [=]()
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

## STEP 4: Integrate `GetModified()` saved-point checks where it adds diagnostic value [DONE]

No additional code changes beyond STEP 2 (already contains `NotifyModificationSaved()` and `GetModified()` assertions).

## STEP 5: Validation build (no unit tests) [DONE]

Run the VS Code task `Build Unit Tests`.

# FIXING ATTEMPTS

## Fixing attempt No.1

Build failed with `error C1075: '(': no matching token found` pointing at `TEST_CATEGORY(L"UndoRedo")` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp(646,2)`. This is consistent with a missing macro terminator for `TEST_CATEGORY`.

Changed the end of the `UndoRedo` category from `}` to `});` right before `TEST_CATEGORY(L"Deletion")`.

This should fix the build because `TEST_CATEGORY` (like `TEST_CASE`) must end with `});`; leaving it as `}` keeps the macro expansion unterminated and breaks parsing at the category start.

## Fixing attempt No.2

Unit tests failed in `GuiSinglelineTextBox / UndoRedo / CtrlZ_UndoesTyping_AndCtrlY_Redoes` with:
`UnitTestRemoteProtocolFeatures::LogRenderingResult()#The last frame didn't trigger UI updating. The action registered by OnNextIdleFrame should always make any element or layout to change.`

The original test structure had idle frames (e.g. `Undid`) that only performed `TEST_ASSERT(...)` without any UI-changing operation, so the harness treated the frame as invalid. It also performed text-changing inputs (`TypeString`, Ctrl+Z/Ctrl+Y) and assertions in the same frame, which violates the repo rule that assertions should happen in the idle frame after the input.

Changed `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` UndoRedo shortcut tests to:
- Split frames so each text-changing input happens in its own frame, with assertions moved to the following frame.
- Ensure every assertion-only frame triggers a UI update by calling `window->SetText(textBox->GetText());`.
- Keep the last frame hide-only.

This should fix the failure because every `OnNextIdleFrame` callback now causes a UI change and the input/verification ordering matches the harness expectations.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

No user edit found
