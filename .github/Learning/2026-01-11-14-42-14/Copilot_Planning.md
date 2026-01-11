# !!!PLANNING!!!

# UPDATES

## UPDATE

There is no need to clear text and then TypeString, because the text box has no text
You don't need to do `window->SetText;` just to make the UI change for no reason.For example, if you just can't keep the last frame clean with only a Hide call, then merge with the previous frame. This also applies to frames in the middle.
`TypeString_NewlineCharsFlattened` could be removed because it is covered before.

## UPDATE

Add one more example
```
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
				});

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					protocol->TypeString(L"abc");
				});
```
This could just be one frame. Check other test cases.

# EXECUTION PLAN

## STEP 1: Add a new Typing test category to the shared test function

Add a new `TEST_CATEGORY(L"Typing")` inside `RunSinglelineTextBoxTestCases<TTextBox>(...)` in Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp.

The log identity for each test case must follow:

    WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Typing_<CASE>")

Why:

- This keeps all typing-related coverage grouped and discoverable in logs.
- The `Typing_...` prefix makes it easy to filter and compare in regression analysis.

## STEP 2: Implement `TypeString_InsertsPlainText`

Add a `TEST_CASE(L"TypeString_InsertsPlainText")` that validates the positive `OnCharInput` path inserts normal characters.

Implementation sketch:

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
    }

Why:

- This is the baseline behavior of `GuiDocumentCommonInterface::OnCharInput` when not filtered.
- Avoids artificial UI updates: the input frame itself produces the render needed for the verification+hide frame.

## STEP 3: Implement `TypeString_InsertsTab_WhenAcceptTabInput`

Add a `TEST_CASE(L"TypeString_InsertsTab_WhenAcceptTabInput")` that verifies a tab character is accepted and inserted when `GetAcceptTabInput()` is true.

Implementation sketch:

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

Why:

- Tab has competing semantics (focus navigation) and is explicitly gated in `OnCharInput`.
- This prevents regressions where Tab silently stops being insertable for editor controls.

## STEP 4: Implement `TypeString_IgnoresWhenCtrlPressed`

Add a `TEST_CASE(L"TypeString_IgnoresWhenCtrlPressed")` that verifies char input is ignored when Ctrl is held.

Important detail:

- If Ctrl-modified typing is ignored, it may not trigger any rendering. Avoid artificial UI updates (e.g. `window->SetText(...)`). Instead, merge input+verification+hide into a single frame after initialization for this case.

Implementation sketch:

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

Why:

- `GuiDocumentCommonInterface::OnCharInput` explicitly ignores char input when `arguments.ctrl` is true.
- This test guards against regressions where Ctrl-modified typing accidentally becomes “normal typing”.

## STEP 5: Implement `TypeString_ReplacesSelection`

Add a `TEST_CASE(L"TypeString_ReplacesSelection")` that selects a range via keyboard selection extension and then types text to replace it.

Implementation sketch (selection via `Shift+Left`):

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
    		});

			protocol->OnNextIdleFrame(L"Selected and typed", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
    			TEST_ASSERT(textBox->GetText() == L"0123X");
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

Why:

- Replacement is the key interaction between typing and selection state.
- Using keyboard-driven selection extension ensures we cover the same path users take.

## STEP 6: Test plan (build, run, and validate logs)

Run the unit tests and validate the new Typing category is exercised and stable.

- Build: run the existing “Build Unit Tests” task.
- Execute: run the existing “Run Unit Tests” task.
- Validate: confirm these test log folders exist and contain no failures:
	- `Controls/Editor/<controlName>/Key/Typing_TypeString_InsertsPlainText`
	- `Controls/Editor/<controlName>/Key/Typing_TypeString_InsertsTab_WhenAcceptTabInput`
	- `Controls/Editor/<controlName>/Key/Typing_TypeString_IgnoresWhenCtrlPressed`
	- `Controls/Editor/<controlName>/Key/Typing_TypeString_ReplacesSelection`

# !!!FINISHED!!!
