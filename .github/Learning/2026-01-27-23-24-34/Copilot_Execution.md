# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Replace "Save PromptCreateFile" placeholder with typed new name [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` and replace the test case `Save PromptCreateFile: Open and Close` with:

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

## STEP 2: Replace "Save PromptOverwriteFile" placeholder with overwrite prompt flow [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` and replace the test case `Save PromptOverwriteFile: Open and Close` with:

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

## STEP 3: Test plan (verify the new behaviors)

- Build the solution (writes to `.github/TaskLogs/Build.log`):
	- `& C:\Code\VczhLibraries\GacUI\.github\TaskLogs\copilotBuild.ps1`
- Execute unit tests (writes to `.github/TaskLogs/Execute.log`):
	- `& C:\Code\VczhLibraries\GacUI\.github\TaskLogs\copilotExecute.ps1 UnitTest`

# FIXING ATTEMPTS

# !!!FINISHED!!!
