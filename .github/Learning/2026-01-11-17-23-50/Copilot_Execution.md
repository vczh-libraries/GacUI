# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Refactor RunTextBoxKeyTestCases signature (remove singleline) [DONE]

Update the shared helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp:

- Remove the `singleline` parameter from `RunTextBoxKeyTestCases`.
- Delete all `if (singleline)` branches from this helper.

    template<typename TTextBox>
    void RunTextBoxKeyTestCases(const wchar_t* resource, const WString& controlName)
    	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
    {
    	// keep existing categories and test cases
    	// remove all `if (singleline)` blocks entirely
    }

## STEP 2: Introduce RunTextBoxKeyTestCases_Singleline for singleline-only behaviors [DONE]

Add a dedicated helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp which contains only the cases that used to be guarded by `if (singleline)`.

- Category names in this helper must use the postfix " (Singleline)".
- All moved test cases must keep the same per-test log identity string (the `Controls/Editor/<controlName>/Key/...` path) as before.

    template<typename TTextBox>
    void RunTextBoxKeyTestCases_Singleline(const wchar_t* resource, const WString& controlName)
    	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
    {
    	TEST_CATEGORY(L"Clipboard (Singleline)")
    	{
    		// moved verbatim from: TEST_CATEGORY(L"Clipboard") { if (singleline) { ... } }
    		TEST_CASE(L"CtrlV_FlattensLineBreaks")
    		{
    			// (keep all frames and asserts)
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_FlattensLineBreaks"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}
    	}

    	TEST_CATEGORY(L"UndoRedo (Singleline)")
    	{
    		// moved verbatim from: TEST_CATEGORY(L"UndoRedo") { if (singleline) { ... } }
    		TEST_CASE(L"CtrlZ_UndoesPaste_FlattenedText")
    		{
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/UndoRedo_CtrlZ_UndoesPaste_FlattenedText"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}
    	}

    	TEST_CATEGORY(L"Deletion (Singleline)")
    	{
    		// moved verbatim from: TEST_CATEGORY(L"Deletion") { if (singleline) { ... } }
    		TEST_CASE(L"Enter_Ignored_NoSelection_NoChange")
    		{
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_NoSelection_NoChange"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}

    		TEST_CASE(L"Enter_Ignored_SelectionPreserved")
    		{
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_SelectionPreserved"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}

    		TEST_CASE(L"CtrlEnter_Ignored_NoSelection_NoChange")
    		{
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_NoSelection_NoChange"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}

    		TEST_CASE(L"CtrlEnter_Ignored_SelectionPreserved")
    		{
    			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    				WString::Unmanaged(L"Controls/Editor/")
    				+ controlName
    				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_SelectionPreserved"),
    				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    				resource
    			);
    		}
    	}
    }

## STEP 3: Update TEST_FILE call sites (GuiSinglelineTextBox calls both helpers) [DONE]

Update the bottom `TEST_FILE` section in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp:

- `GuiSinglelineTextBox` calls both helpers.
- Other controls call only `RunTextBoxKeyTestCases`.
- Keep `controlName` passed to both helpers exactly the same string as before.

    TEST_CATEGORY(L"GuiSinglelineTextBox")
    {
    	RunTextBoxKeyTestCases<GuiSinglelineTextBox>(
    		resource_SinglelineTextBox,
    		WString::Unmanaged(L"GuiSinglelineTextBox")
    	);
    	RunTextBoxKeyTestCases_Singleline<GuiSinglelineTextBox>(
    		resource_SinglelineTextBox,
    		WString::Unmanaged(L"GuiSinglelineTextBox")
    	);
    }

    TEST_CATEGORY(L"GuiMultilineTextBox")
    {
    	RunTextBoxKeyTestCases<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
    }

    TEST_CATEGORY(L"GuiDocumentTextBox")
    {
    	RunTextBoxKeyTestCases<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
    }

    TEST_CATEGORY(L"GuiDocumentLabel")
    {
    	RunTextBoxKeyTestCases<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
    }

    TEST_CATEGORY(L"GuiDocumentViewer")
    {
    	RunTextBoxKeyTestCases<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
    }

## STEP 4: Compile-time and runtime verification (no behavior regressions)

- Build: run the existing "Build Unit Tests" task.
- Unit tests: run the existing "Run Unit Tests" task.
- Verify `GuiSinglelineTextBox` registers and executes the moved cases under " (Singleline)" categories.
- Verify other controls do not register any " (Singleline)" categories.
- Verify log identity strings (paths passed to `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>`) remain unchanged for moved cases.

# FIXING ATTEMPTS

## Fixing attempt No.1
- Why original change did not work: UnitTest project still referenced a deleted/renamed file, causing `error C1083: Cannot open source file: 'TestControls_Editor_GuiSinglelineTextBox_Key.cpp'`.
- What you did: Updated `Test/GacUISrc/UnitTest/UnitTest.vcxproj` to compile `TestControls_Editor_Key.cpp` (keeping the existing `/bigobj` additional options) instead of the missing `TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Why it should solve the build break: `TestControls_Editor_Key.cpp` exists in `Test/GacUISrc/UnitTest`, and the stale compile item was the only reported build error.

# !!!FINISHED!!!

# Comparing to User Edit

No user edit found
