# !!!PLANNING!!!

# UPDATES

## UPDATE

Good catch! I just realize that when paragraph mode is singleline, `GuiDocumentCommonInterface` should just ignore ENTER key (that's said, pressing ENTER with text selected should not delete selection). Please fix it and update test cases accordingly.

# EXECUTION PLAN

## STEP 1: Add a dedicated `Deletion` test category

Change [Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp) by inserting a new `TEST_CATEGORY(L"Deletion")` inside `RunSinglelineTextBoxTestCases(...)`, alongside existing `Scaffold` and `Navigation`.

Why:

- The file is already organized by keyboard-behavior group; `Deletion` keeps the new tests discoverable and consistent with the existing structure.
- The task explicitly requires `TEST_CATEGORY(L"Deletion")` so logs can be filtered and the suite stays easy to navigate.

## STEP 2: Follow the established frame scaffold (Init → Action+Assert)

For each new `TEST_CASE`:

- Frame 1: `Init`
  - Focus the control and seed text using `protocol->TypeString(L"0123456789")`.
- Frame 2: keep the label short (e.g. `Delete`, `Backspace`, `Enter`).
  - Perform the key input(s) and all assertions in the same callback.
  - Hide the window at the end of the callback.

Why:

- This matches the existing tests in the same file and the requested “frame rule” from the task description.
- It keeps logs stable: only create a follow-up frame when an input was sent that could change UI state.

## STEP 3: Add Backspace/Delete cases without selection (including boundaries)

Add the following `TEST_CASE`s under `TEST_CATEGORY(L"Deletion")`.

### 3.1 Backspace_DeletesPreviousChar

Implementation sketch:

		TEST_CASE(L"Backspace_DeletesPreviousChar")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"012356789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

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

Why:

- Validates the “no selection → extend selection by one char → delete selection” behavior implemented in `GuiDocumentCommonInterface::ProcessKey(VKEY::KEY_BACK)`.
- Asserts both text mutation and caret movement, which is the user-visible contract for Backspace.

### 3.2 Delete_DeletesNextChar

Implementation sketch (differences from Backspace are the caret position, key, expected text):

		TEST_CASE(L"Delete_DeletesNextChar")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"012346789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

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

Why:

- Covers the sibling logic path in `ProcessKey(VKEY::KEY_DELETE)` and ensures it deletes the character to the right when no selection exists.

### 3.3 BackspaceAtStart_NoChange

Implementation sketch (boundary at the start of document):

		TEST_CASE(L"BackspaceAtStart_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
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

Why:

- Ensures the boundary condition doesn’t accidentally create a selection or delete text.

### 3.4 DeleteAtEnd_NoChange

Implementation sketch (boundary at the end of document):

		TEST_CASE(L"DeleteAtEnd_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
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

Why:

- Mirrors the Backspace boundary test and protects the symmetric “end-of-text” behavior.

## STEP 4: Add Backspace/Delete cases with selection

Add two cases that prove selection-dependent deletion (the primary subtlety for these keys).

Shared selection setup:

- Use `textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));` and assert `GetSelectionText() == L"234"` before pressing the key.
- This avoids re-testing selection movement (already covered by `Navigation_ShiftArrow_ExtendsSelection` and friends) and keeps the deletion tests focused.

### 4.1 Backspace_DeletesSelection

Implementation sketch:

		TEST_CASE(L"Backspace_DeletesSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

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

### 4.2 Delete_DeletesSelection

Implementation sketch (same as above but `KEY_DELETE`):

		TEST_CASE(L"Delete_DeletesSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

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

Why:

- Confirms the branch where Backspace/Delete do not need to create a one-char selection; instead they delete the active selection.
- Prevents regressions where selection deletion works for one key but not the other.

## STEP 5: Ignore Enter / Ctrl+Enter in Singleline paragraph mode

Change `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` so that when `config.paragraphMode == GuiDocumentParagraphMode::Singleline`, pressing Enter (and Ctrl+Enter) does nothing, even if there is an active selection.

Implementation sketch (within `case VKEY::KEY_RETURN:`):

				case VKEY::KEY_RETURN:
					if (editMode == GuiDocumentEditMode::Editable)
					{
						if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
						{
							// Singleline text boxes should ignore Enter entirely.
							// In particular, Enter must not delete selection.
							return true;
						}
						if (ctrl)
						{
							Array<WString> text(1);
							text[0] = L"\r\n";
							EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						}
						else
						{
							Array<WString> text(2);
							EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						}
						return true;
					}
					break;

Why:

- With `GuiDocumentParagraphMode::Singleline`, the control is fundamentally single-line input, so Enter is not a valid editing command.
- The previous “flatten newline into nothing” approach still goes through `EditText(...)` and therefore replaces the current selection; that accidentally turns Enter into a deletion key, which is not desired in singleline mode.

## STEP 6: Add Enter and Ctrl+Enter cases (singleline: ignore Enter, selection preserved)

Add four cases to cover Enter behavior with and without a selection.

Key expectations for `GuiSinglelineTextBox`:

- With no selection, Enter / Ctrl+Enter should not change `GetText()`.
- With a selection, Enter / Ctrl+Enter should not change `GetText()` and should not clear the selection.

### 6.1 Enter_Ignored_NoSelection_NoChange

Implementation sketch:

		TEST_CASE(L"Enter_Ignored_NoSelection_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
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

### 6.2 Enter_Ignored_SelectionPreserved

Implementation sketch:

		TEST_CASE(L"Enter_Ignored_SelectionPreserved")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
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

### 6.3 CtrlEnter_Ignored_NoSelection_NoChange

Same as 6.1 but call:

				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);

and use log name `Deletion_CtrlEnter_Ignored_NoSelection_NoChange`.

### 6.4 CtrlEnter_Ignored_SelectionPreserved

Same as 6.2 but call:

				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);

and use log name `Deletion_CtrlEnter_Ignored_SelectionPreserved`.

Why:

- This verifies the behavioral contract introduced in STEP 5: Enter is ignored in singleline paragraph mode, so it must not mutate text or selection.

# TEST PLAN

## Build

- Run the existing task `Build Unit Tests`.

## Execute

- Run the existing task `Run Unit Tests`.

## What to verify in logs

- Each new test produces logs under:
  - `Controls/Editor/<controlName>/Key/Deletion_<CASE>`
- Frame labels per test are exactly two frames (`Init` then the short action label), and the last frame ends with `window->Hide();`.

## Coverage checklist (should be satisfied by the added cases)

- Backspace:
  - Deletes previous character when no selection.
  - Deletes selection when selection exists.
  - No-op at start.
- Delete:
  - Deletes next character when no selection.
  - Deletes selection when selection exists.
  - No-op at end.
- Enter / Ctrl+Enter in singleline:
	- No selection: `GetText()` unchanged.
	- With selection: `GetText()` unchanged and `GetSelectionText()` unchanged.

# !!!FINISHED!!!
