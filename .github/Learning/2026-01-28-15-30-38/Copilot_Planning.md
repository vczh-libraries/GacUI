# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add an "Open PromptCreateFile" launcher button

Add a new launcher button in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` inside `resourceFileDialogs` to open `dialogOpen` with `INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile`.

Why:

- The task requires testing the `FileDialogPromptCreateFile` branch for open dialogs; currently `resourceFileDialogs` only offers `Save PromptCreateFile`.
- Using a launcher button ensures tests stay simple (start from the main window and open the dialog with the intended option).

Proposed change (Workflow in `resourceFileDialogs`):

    <att.Rows>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:Percentage percentage:1.0</_>
    </att.Rows>

    <Cell Site="row:4 column:0">
      <Button Text="Open PromptCreateFile">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
        <ev.Clicked-eval><![CDATA[{
          dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
          if (dialogOpen.ShowDialog())
          {
            var output : string = "";
            for (fileName in dialogOpen.FileNames)
            {
              if (output != "")
              {
                output = output & ";";
              }
              output = output & fileName;
            }
            self.Text = output;
          }
        }]]></ev.Clicked-eval>
      </Button>
    </Cell>

Notes:

- Keep using string literals for button texts in tests: `Open`, `Save`, `OK`, `Cancel`.
- This UI change also makes the new tests' first action consistently "click launcher".

## STEP 2: Add message dialog helpers for deterministic close/assert

Extend `namespace gacui_file_dialog_template` in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` with:

1) Non-asserting "try get" helpers, so tests can assert "dialog exists/doesn't exist" without triggering `TEST_ASSERT` inside helpers.

    GuiWindow* TryGetOpeningFileDialog()
    {
      auto mainWindow = GetApplication()->GetMainWindow();
      auto mainNativeWindow = mainWindow ? mainWindow->GetNativeWindow() : nullptr;
      return From(GetApplication()->GetWindows())
        .Where([](GuiWindow* w)
        {
          return w->GetText() == L"FileDialog";
        })
        .Where([=](GuiWindow* w)
        {
          auto nativeWindow = w->GetNativeWindow();
          if (!nativeWindow) return true;
          auto parent = nativeWindow->GetParent();
          return parent == nullptr || parent == mainNativeWindow;
        })
        .First(nullptr);
    }

    GuiWindow* GetOpeningFileDialog()
    {
      auto openingDialog = TryGetOpeningFileDialog();
      TEST_ASSERT(openingDialog != nullptr);
      return openingDialog;
    }

    GuiWindow* TryGetOpeningMessageDialog()
    {
      auto fileDialogWindow = TryGetOpeningFileDialog();
      auto fileDialogNativeWindow = fileDialogWindow ? fileDialogWindow->GetNativeWindow() : nullptr;
      return From(GetApplication()->GetWindows())
        .Where([=](GuiWindow* w)
        {
          if (w == fileDialogWindow) return false;
          auto nativeWindow = w->GetNativeWindow();
          if (!nativeWindow) return false;
          if (fileDialogNativeWindow) return nativeWindow->GetParent() == fileDialogNativeWindow;
          return nativeWindow->GetParent() != nullptr;
        })
        .First(nullptr);
    }

    GuiWindow* GetOpeningMessageDialog()
    {
      auto openingDialog = TryGetOpeningMessageDialog();
      TEST_ASSERT(openingDialog != nullptr);
      return openingDialog;
    }

2) A helper to close message dialogs by real UI click (so the harness observes the UI update), still locating the message dialog via the native-parent relationship.

    void PressMessageButton(UnitTestRemoteProtocol* protocol, const WString& buttonText)
    {
      auto window = GetOpeningMessageDialog();
      auto button = FindControlByText<GuiButton>(window, buttonText);
      auto location = protocol->LocationOf(button);
      GetApplication()->InvokeInMainThread(window, [=]()
      {
        protocol->LClick(location);
      });
    }

    void PressMessageOK(UnitTestRemoteProtocol* protocol)     { PressMessageButton(protocol, L"OK"); }
    void PressMessageCancel(UnitTestRemoteProtocol* protocol) { PressMessageButton(protocol, L"Cancel"); }

Why:

- New tests need to assert "message dialog popped up" and "message dialog closed" deterministically.
- `GetOpeningMessageDialog()` already finds dialogs by native parent; adding `TryGet...` enables stable post-close asserts.
- Using click-based close matches the task requirement and avoids `Enter`/title matching.

## STEP 3: Add TEST_CATEGORY for TryConfirm-driven message box interactions

Add `TEST_CATEGORY(L"File Dialog Message Box Interactions")` in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

Each `UnitTestRemoteProtocol::OnNextIdleFrame` must follow these rules:

- The frame name describes what the previous frame has done.
- Each frame performs exactly one user-visible UI change (click launcher / type / press confirm / close message box / cancel dialog / assert result).

### Case A: File must exist (open dialog) shows error and stays open

Coverage goal:

- Trigger `TryConfirm` error path for `FileDialogFileMustExist`.
- Verify OK dismisses the error message dialog while keeping the file dialog open.
- Verify Cancel closes the file dialog without committing selection.

Proposed proxy frames:

    TEST_CASE(L"File must exist: Open + non-existing file shows error and stays open")
    {
      Ptr<FileSystemMock> fsMock;
      GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
      {
        protocol->OnNextIdleFrame(L"Ready", [=]()
        {
          auto window = GetApplication()->GetMainWindow();
          auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
          auto location = protocol->LocationOf(button);
          GetApplication()->InvokeInMainThread(window, [=]()
          {
            protocol->LClick(location);
          });
        });
        protocol->OnNextIdleFrame(L"Clicked: Open DefaultOptions", [=]()
        {
          TypeFile(protocol, L"not-exist.txt");
        });
        protocol->OnNextIdleFrame(L"Typed: not-exist.txt", [=]()
        {
          PressOpen(protocol);
        });
        protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
        {
          TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
          TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
        });
        protocol->OnNextIdleFrame(L"Popped up error message dialog", [=]()
        {
          PressMessageOK(protocol);
        });
        protocol->OnNextIdleFrame(L"Closed error message dialog", [=]()
        {
          TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
          TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
          PressCancel(protocol);
        });
        protocol->OnNextIdleFrame(L"Cancelled file dialog", [=]()
        {
          auto window = GetApplication()->GetMainWindow();
          auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
          TEST_ASSERT(dialog->GetFileNames().Count() == 0);
          window->Hide();
        });
      });

      GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
        WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_FileMustExist_Error_StaysOpen"),
        WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
        resourceFileDialogs,
        CreateInstaller(fsMock)
      );
    }

### Case B: Prompt create file (open dialog) Cancel keeps open; OK commits selection

Coverage goal:

- Trigger `FileDialogPromptCreateFile` prompt on `Open`.
- Verify Cancel keeps the file dialog open without committing selection.
- Verify repeating `Open` re-prompts, and OK commits and closes the dialog.

Proposed proxy frames (keep the file dialog open across both branches):

- Ready -> click `Open PromptCreateFile`
- Clicked -> type `new-created.txt`
- Typed -> press `Open`
- Pressed -> assert message dialog appears
- Popped up prompt -> click `Cancel`
- Cancelled prompt -> assert file dialog still open and no committed selection
- Pressed Open again -> assert prompt appears again
- Popped up prompt again -> click `OK`
- Confirmed prompt -> assert dialog closed and committed selection (`dialogOpen->GetFileNames()[0] == FilePath(L"/new-created.txt").GetFullPath()`)

### Case C: Prompt overwrite file (save dialog) Cancel keeps open; OK commits selection

Coverage goal:

- Trigger `FileDialogPromptOverwriteFile` prompt by typing an existing file name and pressing `Save`.
- Verify Cancel keeps the save dialog open and no committed selection.
- Verify repeating `Save` re-prompts, and OK commits and closes the dialog.

Key constraints from the task:

- Use `Save PromptOverwriteFile` launcher.
- Type `root.txt` (do not click-select).
- Close the message dialog using `GetOpeningMessageDialog()` + click `OK`/`Cancel`.

Proposed proxy frames:

- Ready -> click `Save PromptOverwriteFile`
- Clicked -> type `root.txt`
- Typed -> press `Save`
- Pressed -> assert overwrite prompt appears
- Popped up overwrite prompt -> click `Cancel`
- Cancelled overwrite prompt -> assert save dialog still open and no committed selection
- Pressed Save again -> assert prompt appears again
- Popped up overwrite prompt again -> click `OK`
- Confirmed overwrite prompt -> assert committed `dialogSave->GetFileName() == FilePath(L"/root.txt").GetFullPath()` and hide main window

### Case D: Multi-selection not enabled shows error and stays open

Coverage goal:

- Trigger `TryConfirm` multiple-selection-not-enabled error path for an open dialog without `FileDialogAllowMultipleSelection`.
- Since the data grid won't allow selecting multiple items, type multiple names directly into the text box and confirm.

Proposed proxy frames:

- Ready -> click `Open DefaultOptions`
- Clicked -> type `README;root.txt`
- Typed -> press `Open`
- Pressed -> assert error message dialog appears
- Popped up error -> click `OK`
- Closed error -> assert file dialog still open
- Cancelled file dialog -> assert `dialogOpen->GetFileNames().Count() == 0`

### Case E: File expected but folder selected shows error and stays open

Coverage goal:

- Trigger `TryConfirm` error path that rejects folders when files are expected.
- Use a dialog that enables multiple selection (`Open MultipleSelection`) and type a mixed list including a folder name.

Proposed proxy frames:

- Ready -> click `Open MultipleSelection`
- Clicked -> type `README;A`
- Typed -> press `Open`
- Pressed -> assert error message dialog appears
- Popped up error -> click `OK`
- Closed error -> assert file dialog still open
- Cancelled file dialog -> assert `dialogOpen->GetFileNames().Count() == 0`

Notes on deterministic assertions:

- Do not assert `A || B` conditions; drive UI to a single expected state then assert exactly.
- When asserting the selection display string (`filePickerControl.textBox`), assert the exact expected quoted format:
  - Single selection: `"a"`
  - Multi-selection: `"a";"b"`

## STEP 4: Test plan

All new tests are UI-driven and must be stable under the existing mock file system created by `CreateFileItemRoot()` in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

Run/verify goals:

- `File Dialog Message Box Interactions` category passes locally with deterministic frame names and one-UI-change-per-frame.
- For each case, verify both behavior and state:
  - Message dialog appears when expected (`TryGetOpeningMessageDialog() != nullptr`).
  - Closing the message dialog uses a real button click (`PressMessageOK/Cancel`) and the message dialog actually disappears afterward.
  - The file dialog stays open after an error/prompt Cancel, and closes only after user confirmation or Cancel on the file dialog itself.
  - No selection is committed when the file dialog is cancelled.
  - Selection is committed (full path) only on OK/confirm flows.

# !!!FINISHED!!!
