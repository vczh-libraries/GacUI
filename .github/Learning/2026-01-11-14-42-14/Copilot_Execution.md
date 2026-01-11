# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add Typing test category to RunSinglelineTextBoxTestCases [DONE]

- File to edit: Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp
- Target function: `template<typename TTextBox> void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)`
- Insert location: right before `TEST_CATEGORY(L"Clipboard")` (i.e. after the ending `});` of `TEST_CATEGORY(L"Navigation")`).

Insert this new `TEST_CATEGORY(L"Typing")` block:

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

## STEP 2: Build and run unit tests [DONE]

- Build: run the “Build Unit Tests” task. (Succeeded: 0 warnings, 0 errors)
- Execute: skipped (per execution constraints: do not run unit tests)
- Validate: confirm these log folders exist and contain no failures:
	- Controls/Editor/<controlName>/Key/Typing_TypeString_InsertsPlainText
	- Controls/Editor/<controlName>/Key/Typing_TypeString_InsertsTab_WhenAcceptTabInput
	- Controls/Editor/<controlName>/Key/Typing_TypeString_IgnoresWhenCtrlPressed
	- Controls/Editor/<controlName>/Key/Typing_TypeString_ReplacesSelection

# FIXING ATTEMPTS

- None.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

No user edit found.
