# !!!PLANNING!!!

# UPDATES

## UPDATE

Moving caret does not update the UI as hijacking the current time using `TooltipTimer` causing time to stop therefore no caret animation. So if a test case only moves caret, only one frame is needed. Changing selection or typing updates the UI, new frames can be started after these actions.

# EXECUTION PLAN

## STEP 1: Add `TEST_CATEGORY(L"Navigation")` to the single-line textbox key tests

Change:
- Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- In `RunSinglelineTextBoxTestCases(...)`, add a new `TEST_CATEGORY(L"Navigation")` after the existing `Scaffold` category.

Why:
- This groups all caret movement and keyboard-driven selection extension tests in one place, matching the task requirement.
- It provides a stable category name so future key-related coverage can grow without mixing concerns.

Proposed code (shape only):

	static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
	{
		TEST_CATEGORY(L"Scaffold")
		{
			// (existing tests)
		});

		TEST_CATEGORY(L"Navigation")
		{
			// (new test cases)
		});
	}

## STEP 2: Add navigation-only caret movement tests (Left/Right/Home/End)

Change:
- Add the following `TEST_CASE`s in `TEST_CATEGORY(L"Navigation")`.
- Use `GuiDocumentLabel` public APIs from `GuiDocumentCommonInterface` to assert state:
	- `GetCaretBegin()`, `GetCaretEnd()`
	- `GetSelectionText()`
	- `GetText()`
- Use the required log identity:
	`WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Navigation_<CASE>")`.

Why:
- These cases validate the baseline `GuiDocumentCommonInterface::ProcessKey` movement semantics before higher-level editing operations.
- Explicitly asserting edges (Home/End) guards the special-case logic in `ProcessKey`.
- Because `TooltipTimer` hijacks current time (caret animation does not tick), caret-only movement is not expected to produce additional UI updates; keep these cases in a single `UnitTestRemoteProtocol::OnNextIdleFrame` callback and hide the window at the end.

Proposed code (to add under `TEST_CATEGORY(L"Navigation")`):

		TEST_CASE(L"ArrowKeys_MoveCaret")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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

## STEP 3: Add Shift-based selection extension tests

Change:
- Add `TEST_CASE`s that press navigation keys with `shift=true` and verify:
	- `GetSelectionText()` matches the selected substring
	- `GetCaretBegin()` / `GetCaretEnd()` reflect selection direction (anchor vs active end)

Why:
- This validates the most important modifier behavior: Shift extends selection while navigation keys change the caret.
- It provides foundational coverage needed before delete/cut/copy/paste tests can be trusted.
- Shift-driven selection changes update the UI, so adding a follow-up frame (e.g. to keep the last logged frame minimal) remains appropriate.

Proposed code:

		TEST_CASE(L"ShiftArrow_ExtendsSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_HOME, false, true, false);
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 5));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetSelectionText() == L"01234");

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

## STEP 4: Add “no effect” tests for single-line behavior (Up/Down/PageUp/PageDown)

Change:
- Add `TEST_CASE`s ensuring these keys do not change caret/selection/text for a single-line textbox:
	- `KEY_UP` / `KEY_DOWN`
	- `KEY_PRIOR` / `KEY_NEXT`

Why:
- These keys have explicit handling in `GuiDocumentCommonInterface::ProcessKey`; asserting the single-line no-op behavior prevents accidental regressions.
- These are caret-only/no-op cases; keep them in a single idle frame for the same reason as STEP 2.

Proposed code:

		TEST_CASE(L"UpDown_NoEffectInSingleline")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_UP);
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
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));

					protocol->KeyPress(VKEY::KEY_PRIOR);
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

## STEP 5: Add Ctrl-modifier spot-check to ensure navigation behavior is unchanged

Change:
- Add a `TEST_CASE` that compares behavior of `Ctrl+KEY_LEFT` (or `Ctrl+KEY_HOME`) with the unmodified key for this control.

Why:
- `GuiDocumentCommonInterface::ProcessKey` first checks Ctrl-based shortcuts via `internalShortcutKeyManager`, but Ctrl+navigation is not registered there.
- This protects the intended behavior guarantee: Ctrl should not change caret navigation semantics here.
- This is still a caret-only movement check; keep it in a single idle frame.

Proposed code:

		TEST_CASE(L"CtrlWithNavigation_DoesNotChangeBehavior")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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

# TEST PLAN

- Build and run unit tests using the existing tasks: `Build and Run Unit Tests`.
- Confirm the new tests appear under `GuiSinglelineTextBox/Navigation/*` and each produces a log path matching:
	`Controls/Editor/GuiSinglelineTextBox/Key/Navigation_<CASE>`.
- Verify key behaviors:
	- Left/Right/Home/End update caret columns correctly with empty selection.
	- Shift+navigation extends selection and `GetSelectionText()` matches expected substrings.
	- Up/Down/PageUp/PageDown do not change caret/selection/text.
	- Ctrl+navigation behaves the same as non-Ctrl navigation.

# !!!FINISHED!!!
