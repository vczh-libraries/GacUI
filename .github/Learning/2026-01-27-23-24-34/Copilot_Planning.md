# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Replace "Save PromptCreateFile" placeholder with typed new name

### What to change

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` and replace the test case `Save PromptCreateFile: Open and Close` with a realistic save confirmation flow: open save dialog, type a non-existing file name, click `Save`, and assert `GuiSaveFileDialog::GetFileName()` is the selected path.

Proposed replacement:

	TEST_CASE(L"Save PromptCreateFile: Open and Close")
	{
		Ptr<FileSystemMock> fsMock;
		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Save PromptCreateFile");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]()
				{
					protocol->LClick(location);
				});
			});
			protocol->OnNextIdleFrame(L"Show Dialog", [=]()
			{
				TypeFile(protocol, L"new.txt");
			});
			protocol->OnNextIdleFrame(L"Typed", [=]()
			{
				PressSave(protocol);
			});
			protocol->OnNextIdleFrame(L"Confirmed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
				TEST_ASSERT(dialog->GetFileName() == FilePath(L"/new.txt").GetFullPath());
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/SavePromptCreateFile_OpenAndClose"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	});

### Why this change is necessary

- The existing placeholder only clicks `Cancel`, so it never validates the save dialog’s intended “type a new name then confirm” workflow.
- Per `Copilot_Task.md`’s update, no message box is expected for saving to a non-existing file; “file not exist” errors apply to open dialogs (`INativeDialogService::FileDialogFileMustExist`), not this save-dialog case.

## STEP 2: Replace "Save PromptOverwriteFile" placeholder with overwrite prompt flow

### What to change

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` and replace the test case `Save PromptOverwriteFile: Open and Close` with a realistic overwrite flow: open save dialog, click an existing file in the list, click `Save`, accept the overwrite prompt, and assert `GuiSaveFileDialog::GetFileName()` is that existing file.

Proposed replacement:

	TEST_CASE(L"Save PromptOverwriteFile: Open and Close")
	{
		Ptr<FileSystemMock> fsMock;
		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Save PromptOverwriteFile");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]()
				{
					protocol->LClick(location);
				});
			});
			protocol->OnNextIdleFrame(L"Show Dialog", [=]()
			{
				ClickFile(protocol, L"root.txt");
			});
			protocol->OnNextIdleFrame(L"Selected", [=]()
			{
				PressSave(protocol);
			});
			protocol->OnNextIdleFrame(L"Popped up overwrite prompt", [=]()
			{
				auto messageWindow = GetOpeningMessageDialog();
				auto buttonOk = FindControlByText<GuiButton>(messageWindow, L"OK");
				auto location = protocol->LocationOf(buttonOk);
				GetApplication()->InvokeInMainThread(messageWindow, [=]()
				{
					protocol->LClick(location);
				});
			});
			protocol->OnNextIdleFrame(L"Confirmed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
				TEST_ASSERT(dialog->GetFileName() == FilePath(L"/root.txt").GetFullPath());
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/SavePromptOverwriteFile_OpenAndClose"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	});

### Why this change is necessary

- The existing placeholder only clicks `Cancel`, so it never validates the intended save dialog interaction: “select existing file -> confirm -> handle overwrite prompt”.
- Clicking an existing file is deterministic because `gacui_unittest_template::CreateFileItemRoot()` always creates `root.txt` at the root.
- This covers the minimum overwrite prompt interaction needed for save-dialog realism, without expanding into full message-box branching coverage (deferred to the next task).

## STEP 3: Test plan (verify the new behaviors)

### What to test

- `Save PromptCreateFile: Open and Close`:
	- The dialog closes after `PressSave(protocol)` with a typed non-existing name.
	- No message dialog is involved.
	- `dialogSave->GetFileName()` equals `FilePath(L"/new.txt").GetFullPath()`.
- `Save PromptOverwriteFile: Open and Close`:
	- After selecting `root.txt` and pressing `Save`, an overwrite message dialog appears.
	- Clicking `OK` closes the save dialog and confirms the selection.
	- `dialogSave->GetFileName()` equals `FilePath(L"/root.txt").GetFullPath()`.

### How to run

- Build the solution (writes to `.github/TaskLogs/Build.log`):
	- `& C:\Code\VczhLibraries\GacUI\.github\TaskLogs\copilotBuild.ps1`
- Execute unit tests (writes to `.github/TaskLogs/Execute.log`):
	- `& C:\Code\VczhLibraries\GacUI\.github\TaskLogs\copilotExecute.ps1 UnitTest`

# !!!FINISHED!!!
