# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Locate the insertion point in the existing style tests

Add new undo/redo tests to:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Place the new category under the existing `TEST_CATEGORY(L"Styles")` scope, near the existing style operation tests (e.g. before the current `TEST_CATEGORY(L"RenameStyle")` is fine).

Why:
- The file already contains the style test harness (`resource_DocumentViewer`, `RegisterStyle`, `SummarizeName`) and already tests `EditStyleName`, `RemoveStyleName`, and `RenameStyle` behavior.

## STEP 2: Add `UndoRedo` category scaffolding

Insert the following structure (under `TEST_CATEGORY(L"Styles")`):

    TEST_CATEGORY(L"UndoRedo")
    {
        TEST_CATEGORY(L"EditStyleName")
        {
            // new TEST_CASEs
        }

        TEST_CATEGORY(L"RemoveStyleName")
        {
            // new TEST_CASEs
        }

        TEST_CATEGORY(L"RenameStyle")
        {
            // new TEST_CASEs
        }
    }

Why:
- Required by the task: all new tests must be grouped under `TEST_CATEGORY(L"UndoRedo")` with subcategories per operation.

Implementation note:
- Use tabs for indentation in C++ source code in this repo (the snippets below may appear space-indented; do not copy them verbatim).

## STEP 3: Add undo/redo tests for EditStyleName

### STEP 3.1: Test: Undo EditStyleName

Add this new test case under `TEST_CATEGORY(L"UndoRedo") / TEST_CATEGORY(L"EditStyleName")`:

    TEST_CASE(L"Undo_EditStyleName")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"EditStyleName", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/Undo_EditStyleName"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Validates `EditStyleName` produces an undo step and that undo restores the original “no style name” state (`SummarizeStyleName` returns empty / null).

### STEP 3.2: Test: Redo EditStyleName

Add:

    TEST_CASE(L"Redo_EditStyleName")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                TEST_ASSERT(textBox->Redo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Redo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/Redo_EditStyleName"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Confirms redo re-applies the style name after an undo.

### STEP 3.3: Test: Multiple edits then undo all (EditStyleName → RemoveStyleName)

This scenario is required under EditStyleName/RemoveStyleName in the task description; implement it under `UndoRedo/EditStyleName` (because it starts from “apply style name”) and verify both steps unwind.

Note:
- This test intentionally chains multiple operations while living under `UndoRedo/EditStyleName`, because it validates undo stack composition rather than a single operation in isolation.

Add:

    TEST_CASE(L"MultipleEdits_UndoAll_ReturnsToOriginal")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));

                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Edit then Remove", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));

                // Undo remove => should restore the applied style
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo Remove", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

                // Undo apply => should restore the original unstyled state
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo Apply", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/MultipleEdits_UndoAll_ReturnsToOriginal"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Ensures the undo stack is a chain of steps and that multiple steps can be undone back to the pristine baseline.

### STEP 3.4: Test: Undo/Redo with CanUndo/CanRedo end-of-history boundaries (EditStyleName)

Add:

    TEST_CASE(L"UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Applied", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                while (textBox->CanUndo())
                {
                    TEST_ASSERT(textBox->Undo());
                }
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                TEST_ASSERT(!textBox->Undo());
            });

            protocol->OnNextIdleFrame(L"Undo until the end", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                while (textBox->CanRedo())
                {
                    TEST_ASSERT(textBox->Redo());
                }
                TEST_ASSERT(!textBox->CanRedo());
                TEST_ASSERT(textBox->CanUndo());

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
                TEST_ASSERT(!textBox->Redo());

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Mirrors the proven pattern from `TestControls_Editor_Key_Shared.cpp` and validates the boundary behavior of `Undo()`/`Redo()` return values for style name operations.

### STEP 3.5: Test: Redo history is dropped on new edits (EditStyleName)

Add:

    TEST_CASE(L"RedoHistoryDropped_OnNewEdit")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
                RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));

                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());

                // New edit after undo must drop redo history
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleB"));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo then Edit Again", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"StyleB"));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/RedoHistoryDropped_OnNewEdit"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Validates the core undo/redo processor contract: once you undo and then perform a new edit, redo history must be truncated.

## STEP 4: Add undo/redo tests for RemoveStyleName

### STEP 4.1: Test: Undo RemoveStyleName

Add under `UndoRedo/RemoveStyleName`:

    TEST_CASE(L"Undo_RemoveStyleName")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Removed", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/Undo_RemoveStyleName"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Confirms that `RemoveStyleName` contributes a reversible undo step that restores the previous style-name runs.

### STEP 4.2: Test: Redo RemoveStyleName

Add:

    TEST_CASE(L"Redo_RemoveStyleName")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));

                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

                TEST_ASSERT(textBox->Redo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Redo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));
                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/Redo_RemoveStyleName"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Confirms redo re-applies the removal (style name becomes empty again).

### STEP 4.3: Test: Undo/Redo with CanUndo/CanRedo checks at each step (RemoveStyleName)

Add (explicitly checks transitions across a chain):

    TEST_CASE(L"CanUndoCanRedo_Transitions")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Applied then Removed", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                TEST_ASSERT(!SummarizeName(textBox, 0, 10));

                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo Remove", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

                TEST_ASSERT(textBox->Redo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
                TEST_ASSERT(!SummarizeName(textBox, 0, 10));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/CanUndoCanRedo_Transitions"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Makes `CanUndo()` / `CanRedo()` a first-class contract check around each state transition (apply → remove → undo → redo).

## STEP 5: Add undo/redo tests for RenameStyle

Important harness detail:
- Applying a style name via `EditStyleName` itself creates an undo step; for rename-specific undo/redo tests, clear history after applying so that undo/redo only covers rename operations.

### STEP 5.1: Test: Undo RenameStyle

Add under `UndoRedo/RenameStyle`:

    TEST_CASE(L"Undo_RenameStyle")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"OldName"));

                // Isolate rename from the style-application undo step
                textBox->ClearUndoRedo();
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"RenameStyle", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"NewName"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"OldName"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Undo_RenameStyle"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- `RenameStyle` uses a dedicated rename undo step; this test verifies both text runs and style registry keys revert.

### STEP 5.2: Test: Redo RenameStyle

Add:

    TEST_CASE(L"Redo_RenameStyle")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
                RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"OldName"));
                textBox->ClearUndoRedo();
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"OldName"));

                TEST_ASSERT(textBox->Redo());
                TEST_ASSERT(textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Redo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"NewName"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Redo_RenameStyle"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Confirms redo re-applies the rename and re-establishes registry + run state.

### STEP 5.3: Test: Undo RenameStyle with parent references

Add:

    TEST_CASE(L"Undo_RenameStyle_ParentReferences")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                RegisterStyle(textBox, WString::Unmanaged(L"Parent"), WString::Empty, MakeStyleWithBold(true));
                RegisterStyle(textBox, WString::Unmanaged(L"Child"), WString::Unmanaged(L"Parent"), MakeStyleWithBold(false));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"Child"));

                textBox->ClearUndoRedo();
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
                textBox->RenameStyle(WString::Unmanaged(L"Parent"), WString::Unmanaged(L"NewParent"));
                TEST_ASSERT(textBox->CanUndo());
            });

            protocol->OnNextIdleFrame(L"Rename Parent", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
                TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"NewParent"));

                TEST_ASSERT(textBox->Undo());
                TEST_ASSERT(textBox->CanRedo());
            });

            protocol->OnNextIdleFrame(L"Undo", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
                TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"Parent"));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Undo_RenameStyle_ParentReferences"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Verifies the rename operation updates and restores dependent references (`parentStyleName`) under undo.

### STEP 5.4: Test: Multiple renames then undo all

Add:

    TEST_CASE(L"MultipleRenames_UndoAll_ReturnsToA")
    {
        TooltipTimer timer;
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Init", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
                textBox->SetFocused();

                textBox->LoadTextAndClearUndoRedo(L"0123456789");
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());
                RegisterStyle(textBox, WString::Unmanaged(L"A"), WString::Empty, MakeStyleWithBold(true));
                textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"A"));

                textBox->ClearUndoRedo();
                TEST_ASSERT(!textBox->CanUndo());
                TEST_ASSERT(!textBox->CanRedo());

                textBox->RenameStyle(WString::Unmanaged(L"A"), WString::Unmanaged(L"B"));
                textBox->RenameStyle(WString::Unmanaged(L"B"), WString::Unmanaged(L"C"));
                TEST_ASSERT(textBox->CanUndo());
            });

            protocol->OnNextIdleFrame(L"A->B->C", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"C"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"C")));

                TEST_ASSERT(textBox->Undo());
            });

            protocol->OnNextIdleFrame(L"Undo C->B", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"B"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"B")));
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"C")));

                TEST_ASSERT(textBox->Undo());
            });

            protocol->OnNextIdleFrame(L"Undo B->A", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

                auto summary = SummarizeName(textBox, 0, 10);
                TEST_ASSERT(summary);
                TEST_ASSERT(summary.Value() == WString::Unmanaged(L"A"));

                auto document = textBox->GetDocument();
                TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"A")));
                TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"B")));

                window->Hide();
            });
        });

        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/MultipleRenames_UndoAll_ReturnsToA"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resource_DocumentViewer
        );
    });

Why:
- Ensures rename steps compose correctly in the undo stack and unwind to the original name.

## STEP 6: Verification plan (no new test cases beyond the above)

Follow the Windows wrapper-script workflow (do not invoke msbuild / executables directly):

- Stop any existing debugger session:

      & REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1

- Build (Debug|x64):

      cd REPO-ROOT\Test\GacUISrc
      & REPO-ROOT\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64

- Run unit tests (Debug|x64):

      cd REPO-ROOT\Test\GacUISrc
      & REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64

- Confirm all new tests log under the specified stable paths:
  - `Controls/Editor/Features/Styles/UndoRedo/EditStyleName/...`
  - `Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/...`
  - `Controls/Editor/Features/Styles/UndoRedo/RenameStyle/...`

Why:
- Ensures changes are test-only and validates behavior with the repository’s UI-driven unit test framework.

# !!!FINISHED!!!
