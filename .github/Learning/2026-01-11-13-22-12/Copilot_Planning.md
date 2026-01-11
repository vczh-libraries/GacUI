# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add `TEST_CATEGORY(L"UndoRedo")` under `RunSinglelineTextBoxTestCases`

Add a new test category in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` inside `RunSinglelineTextBoxTestCases<TTextBox>(...)`.

Why this is necessary:

- The file currently covers navigation, clipboard, deletion, etc., but it has no undo/redo coverage.
- The task explicitly requires adding `TEST_CATEGORY(L"UndoRedo")` under the same template-based shared test function so that the tests can be reused for other `GuiDocumentCommonInterface`-based controls in the future.

Planned insertion (structure only, cases added in later steps):

	TEST_CATEGORY(L"UndoRedo")
	{
		// TEST_CASEs will be added here
	});

Test plan for this step:

- Compile to ensure the new category is syntactically valid.

## STEP 2: Add shortcut tests for Ctrl+Z / Ctrl+Y across representative edits

Implement three shortcut-focused cases in the new `UndoRedo` category. Each test case follows the established scaffold pattern in this file:

- `Init` frame: focus control and seed deterministic text (via `protocol->TypeString(...)`), then normalize undo baseline.
- `Typing` frame: issue the shortcut key inputs and assert final state in the same callback.
- Final frame: hide-only.

Why this is necessary:

- Ctrl+Z / Ctrl+Y are first-class keyboard shortcuts that must work across typing, deletion, and paste.
- These tests validate integration between shortcut handling and `GuiDocumentUndoRedoProcessor` through the real UI path.

Planned code (insert inside `TEST_CATEGORY(L"UndoRedo")`):

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
				TEST_ASSERT(textBox->GetText() == L"0123456789");
				TEST_ASSERT(!textBox->CanUndo());
				TEST_ASSERT(textBox->CanRedo());
				TEST_ASSERT(!textBox->GetModified());

				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
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
				TEST_ASSERT(textBox->GetText() == L"0123456789");

				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
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
				TEST_ASSERT(textBox->GetText() == L"0123456789");

				protocol->KeyPress(VKEY::KEY_Y, true, false, false);
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

Test plan for this step:

- Run the unit test suite and verify the three new cases pass.
- Ensure each case uses the required log identity prefix `.../Key/UndoRedo_...`.

## STEP 3: Add API-level tests for `ClearUndoRedo()`, `LoadTextAndClearUndoRedo(...)`, and history-end behavior

Add dedicated cases that validate undo/redo contracts independent of shortcut dispatch.

Why this is necessary:

- The update explicitly requires careful validation of:
  - `ClearUndoRedo()` deleting both undo and redo history.
  - `Undo()` / `Redo()` failing (returning `false`) at the ends of history.
  - Correct `CanUndo()` / `CanRedo()` transitions before and after exhausting consecutive operations.
  - `LoadTextAndClearUndoRedo(...)` as a baseline initializer.

Planned code (insert inside `TEST_CATEGORY(L"UndoRedo")`):

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

				TEST_ASSERT(textBox->CanUndo());
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

Test plan for this step:

- Verify that the two new cases pass and that they explicitly assert both the return value of `Undo()` / `Redo()` and the `CanUndo()` / `CanRedo()` state before/after exhaustion.
- Confirm `LoadTextAndClearUndoRedo(...)` is exercised (and not replaced by `SetText(...)`).

## STEP 4: Integrate `GetModified()` saved-point checks where it adds diagnostic value

Enhance `CtrlZ_UndoesTyping_AndCtrlY_Redoes` (already planned in Step 2) to verify `GetModified()` transitions around a known “saved point” created via `NotifyModificationSaved()`.

Why this is necessary:

- The task suggests (optionally) validating modified flags to ensure undo back to the saved point behaves as expected.
- This gives high-signal regression coverage without adding a completely separate case.

Planned checks (already included in Step 2 code):

- In `Init`: after `ClearUndoRedo()` + `NotifyModificationSaved()`, assert `GetModified() == false`.
- After typing `X`: assert `GetModified() == true`.
- After undo: assert `GetModified() == false`.
- After redo: assert `GetModified() == true`.

Test plan for this step:

- Ensure the modified-flag asserts remain stable across runs.

## STEP 5: Validation run (build + unit tests)

Run the existing unit test tasks for `Test/GacUISrc`.

Why this is necessary:

- The changes are purely test additions, so success criteria is compile + all unit tests passing.

Test plan for this step:

- Run the VS Code task `Build and Run Unit Tests`.

# !!!FINISHED!!!
