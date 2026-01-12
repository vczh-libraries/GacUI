# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Refactor RunTextBoxKeyTestCases signature (remove singleline)

Change the shared helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp to remove the `singleline` parameter, and delete all `if (singleline)` branches from this helper.

Why: the baseline key-test scaffold should represent behavior shared by all textbox-like controls. The boolean flag couples unrelated behavior to the baseline, and it will not scale as multiline / paragraph-specific cases grow.

Planned code change (shape):

    template<typename TTextBox>
    void RunTextBoxKeyTestCases(const wchar_t* resource, const WString& controlName)
    	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
    {
    	// keep existing categories and test cases
    	// remove all `if (singleline)` blocks entirely
    }

Notes:

- Keep every existing `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` log identity string unchanged.
- This step must not rename any test cases or change any `OnNextIdleFrame` sequences.

## STEP 2: Introduce RunTextBoxKeyTestCases_Singleline for singleline-only behaviors

Add a dedicated helper in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp which contains only the cases that used to be guarded by `if (singleline)`.

Why: making singleline-only behavior explicit avoids hidden branching in the baseline helper, and makes future multiline / paragraph work additive (by adding new helpers) instead of conditional.

Planned code change (shape):

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

Notes:

- Category names in this helper must use the postfix " (Singleline)" (e.g. `Clipboard (Singleline)`), matching the clarified requirement.
- All moved test cases must keep the same per-test log identity string (the `Controls/Editor/<controlName>/Key/...` path) as before.
- No behavior change beyond removing the `if (singleline)` branch point.

## STEP 3: Update TEST_FILE call sites (GuiSinglelineTextBox calls both helpers)

Update the bottom `TEST_FILE` section in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp.

Why: the baseline suite should run for all controls, but the singleline-only suite must only run for `GuiSinglelineTextBox`.

Planned code change (shape):

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

Notes:

- Keep `controlName` passed to both helpers exactly the same string as before for each control.
- Do not append " (Singleline)" to the outer `TEST_CATEGORY(L"GuiSinglelineTextBox")`; only the categories inside `RunTextBoxKeyTestCases_Singleline` get the postfix.

## STEP 4: Compile-time and runtime verification (no behavior regressions)

After implementing the refactor, verify that the project still compiles and the unit tests execute.

Why: the refactor changes template signatures and rearranges test registration; it must not break compilation, resource mapping, or test discovery.

Expected compile-time checks:

- All call sites updated to the new `RunTextBoxKeyTestCases` signature.
- `RunTextBoxKeyTestCases_Singleline` compiles under the same `requires(...)` constraint.

Expected runtime checks:

- For `GuiSinglelineTextBox`, both baseline categories and the new `(... (Singleline))` categories appear and execute.
- For non-singleline controls, no `(... (Singleline))` categories appear.
- The four moved test cases keep identical log identity strings (folder paths) to their pre-refactor versions.

# TEST PLAN

## Build

- Run the existing "Build Unit Tests" task.
- Confirm there are no compilation errors in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp after signature changes.

## Unit tests

- Run the existing "Run Unit Tests" task.
- Confirm `GuiSinglelineTextBox` runs:
	- `Clipboard_CtrlV_FlattensLineBreaks`
	- `UndoRedo_CtrlZ_UndoesPaste_FlattenedText`
	- `Deletion_Enter_Ignored_NoSelection_NoChange`
	- `Deletion_Enter_Ignored_SelectionPreserved`
	- `Deletion_CtrlEnter_Ignored_NoSelection_NoChange`
	- `Deletion_CtrlEnter_Ignored_SelectionPreserved`
- Confirm `GuiMultilineTextBox`, `GuiDocumentTextBox`, `GuiDocumentLabel`, `GuiDocumentViewer` do not register any " (Singleline)" categories.

## Log path stability

- Inspect that the log identity strings passed to `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>` are unchanged for the moved cases, especially:
	- `Controls/Editor/GuiSinglelineTextBox/Key/Clipboard_CtrlV_FlattensLineBreaks`
	- `Controls/Editor/GuiSinglelineTextBox/Key/UndoRedo_CtrlZ_UndoesPaste_FlattenedText`
	- `Controls/Editor/GuiSinglelineTextBox/Key/Deletion_...`

# !!!FINISHED!!!
