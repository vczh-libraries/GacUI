# !!!EXECUTION!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add `TEST_CATEGORY(L"Navigation")` in `RunSinglelineTextBoxTestCases` [DONE]

Edit file: `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.

In `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)`, insert a new `TEST_CATEGORY(L"Navigation")` after the existing `TEST_CATEGORY(L"Scaffold")` block and before the closing `}` of `RunSinglelineTextBoxTestCases`.

Insert this code block:

```
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
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->SetText(L"0123456789");
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
	}
```

# TEST PLAN

- Build only: run the `Build Unit Tests` task and check `.github/TaskLogs/Build.log`.

# FIXING ATTEMPTS

## Fixing attempt No.1

### Why the original change did not work

`TEST_CATEGORY` is a macro that must be closed with `});`. The inserted `Navigation` category was mistakenly closed with `}` only, causing `error C2143` at the end of `RunSinglelineTextBoxTestCases`.

### What I need to do

Fix the closing of `TEST_CATEGORY(L"Navigation")` from `}` to `});`.

### Why this should solve the build break

This matches the required `TEST_CATEGORY` / `TEST_CASE` macro structure used everywhere else in the unit tests, restoring correct syntax.

# Comparing to User Edit

- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, initialization was changed from `textBox->SetText(...)` to `protocol->TypeString(...)`, and split into a dedicated setup frame (e.g. `Init`) followed by a second frame (often titled `Typing`) for caret setup, key presses, and assertions.
- Tests consistently call `textBox->SetFocused()` before typing.
- Several cases set caret to `(0, 10)` before setting the intended start caret (e.g. `(0, 5)`), suggesting an extra caret initialization step is sometimes needed after typing.
- Learning: for editor control key tests, seed text via `protocol->TypeString` and treat “type then assert/operate” as requiring a frame boundary; avoid relying on `SetText(...)` for initial content unless the test is specifically about programmatic text assignment.

# !!!FINISHED!!!
