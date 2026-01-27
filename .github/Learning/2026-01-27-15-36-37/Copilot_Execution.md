# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN
## STEP 1: Extend the test window UI to host open/multi-select dialogs [DONE]
**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`
**Replace the dialog and table section in `resourceFileDialogs` with**:
    <Window ref.Name="self" Text="Dialog File Test" ClientSize="x:640 y:480">
      <OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
      <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
        <att.Rows>
          <_>composeType:MinSize</_>
          <_>composeType:MinSize</_>
          <_>composeType:Percentage percentage:1.0</_>
        </att.Rows>
        <att.Columns>
          <_>composeType:MinSize</_>
          <_>composeType:Percentage percentage:1.0</_>
        </att.Columns>
        <Cell Site="row:0 column:0">
          <Button Text="Open DefaultOptions">
            <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            <ev.Clicked-eval><![CDATA[{
              dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogFileMustExist;
              if (dialogOpen.ShowDialog())
              {
                var output:string = "";
                for (fileName in dialogOpen.FileNames)
                {
                  if (output != "") output = output & ";";
                  output = output & fileName;
                }
                self.Text = output;
              }
            }]]></ev.Clicked-eval>
          </Button>
        </Cell>
        <Cell Site="row:1 column:0">
          <Button Text="Open MultipleSelection">
            <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            <ev.Clicked-eval><![CDATA[{
              dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogFileMustExist | INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection;
              if (dialogOpen.ShowDialog())
              {
                var output:string = "";
                for (fileName in dialogOpen.FileNames)
                {
                  if (output != "") output = output & ";";
                  output = output & fileName;
                }
                self.Text = output;
              }
            }]]></ev.Clicked-eval>
          </Button>
        </Cell>
      </Table>
    </Window>
## STEP 2: Extend ClickFileInternal to support modifier keys [DONE]
**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` (`namespace gacui_file_dialog_template`)
**Replace `ClickFileInternal(...)` and update wrappers to**:
    void ClickFileInternal(UnitTestRemoteProtocol* protocol, const WString& fileName, bool ctrl, bool shift, bool alt, bool doubleClick)
    {
    	auto window = GetOpeningFileDialog();
    	auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"filePickerControl", L"dataGrid");
    	auto itemProvider = dataGrid->GetItemProvider();
    	vint fileItemIndex = -1;
    	for (vint i = 0; i < itemProvider->Count(); i++)
    	{
    		if (itemProvider->GetTextValue(i) == fileName)
    		{
    			fileItemIndex = i;
    			break;
    		}
    	}
    	TEST_ASSERT(fileItemIndex != -1);
    	if (doubleClick)
    	{
    		LDBClickListItem(protocol, dataGrid, fileItemIndex);
    	}
    	else
    	{
    		LClickListItem(protocol, dataGrid, ctrl, shift, alt, fileItemIndex);
    	}
    }
    void ClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
    {
    	ClickFileInternal(protocol, fileName, false, false, false, false);
    }
    void DbClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
    {
    	ClickFileInternal(protocol, fileName, false, false, false, true);
    }
    void ClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName, bool ctrl, bool shift, bool alt)
    {
    	ClickFileInternal(protocol, fileName, ctrl, shift, alt, false);
    }
## STEP 3: Add a new file in the mock filesystem to enable filter + multi-selection [DONE]
**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` (`CreateFileItemRoot()`)
**Insert after `AddFile(root, L"root.txt");`**:
	AddFile(root, L"root2.txt");
## STEP 4: Add a new TEST_CATEGORY focused on multiple selection [DONE]
**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`
**Insert before the last `});` in the `TEST_FILE` block (before the final `}`)**:
	TEST_CATEGORY(L"File Dialog Multiple Selection")
	{
		// test cases in STEP 5 and STEP 6
	}
## STEP 5: Add multi-selection tests (without filter, and with filter) [DONE]
**Add these test cases inside `TEST_CATEGORY(L"File Dialog Multiple Selection")`**:
    TEST_CASE(L"Multiple selection: All Files (root) -> selection string -> output list")
    {
    	Ptr<FileSystemMock> fsMock;
    	GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    	{
    		protocol->OnNextIdleFrame(L"Ready", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto button = FindControlByText<GuiButton>(window, L"Open MultipleSelection");
    			auto location = protocol->LocationOf(button);
    			GetApplication()->InvokeInMainThread(window, [=]()
    			{
    				protocol->LClick(location);
    			});
    		});
    		protocol->OnNextIdleFrame(L"Clicked: Open MultipleSelection", [=]()
    		{
    			ClickFile(protocol, L"README");
    		});
    		protocol->OnNextIdleFrame(L"Selected: README", [=]()
    		{
    			ClickFile(protocol, L"root.txt", true, false, false);
    		});
    		protocol->OnNextIdleFrame(L"Ctrl-Selected: root.txt", [=]()
    		{
    			auto dialogWindow = GetOpeningFileDialog();
    			auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
    			auto selectionText = textBox->GetText();
    			TEST_ASSERT(selectionText == WString::Unmanaged(L"\"README\";\"root.txt\""));
    			PressOpen(protocol);
    		});
    		protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
    			auto&& fileNames = dialog->GetFileNames();
    			TEST_ASSERT(fileNames.Count() == 2);
    			TEST_ASSERT(fileNames.Contains(FilePath(L"/README").GetFullPath()));
    			TEST_ASSERT(fileNames.Contains(FilePath(L"/root.txt").GetFullPath()));
    			window->Hide();
    		});
    	});
    	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    		WString::Unmanaged(L"Application/Dialog_File/MultipleSelection_Root_AllFiles"),
    		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    		resourceFileDialogs,
    		CreateInstaller(fsMock)
    	);
    }
    TEST_CASE(L"Multiple selection: Text Files filter (root) -> selection string -> output list")
    {
    	Ptr<FileSystemMock> fsMock;
    	GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    	{
    		protocol->OnNextIdleFrame(L"Ready", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto button = FindControlByText<GuiButton>(window, L"Open MultipleSelection");
    			auto location = protocol->LocationOf(button);
    			GetApplication()->InvokeInMainThread(window, [=]()
    			{
    				protocol->LClick(location);
    			});
    		});
    		protocol->OnNextIdleFrame(L"Clicked: Open MultipleSelection", [=]()
    		{
    			ChooseFilter(protocol, 1);
    		});
    		protocol->OnNextIdleFrame(L"Selected Filter: Text Files", [=]()
    		{
    			ClickFile(protocol, L"root.txt");
    		});
    		protocol->OnNextIdleFrame(L"Selected: root.txt", [=]()
    		{
    			ClickFile(protocol, L"root2.txt", true, false, false);
    		});
    		protocol->OnNextIdleFrame(L"Ctrl-Selected: root2.txt", [=]()
    		{
    			auto dialogWindow = GetOpeningFileDialog();
    			auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
    			auto selectionText = textBox->GetText();
    			TEST_ASSERT(selectionText == WString::Unmanaged(L"\"root.txt\";\"root2.txt\""));
    			PressOpen(protocol);
    		});
    		protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
    			auto&& fileNames = dialog->GetFileNames();
    			TEST_ASSERT(fileNames.Count() == 2);
    			TEST_ASSERT(fileNames.Contains(FilePath(L"/root.txt").GetFullPath()));
    			TEST_ASSERT(fileNames.Contains(FilePath(L"/root2.txt").GetFullPath()));
    			window->Hide();
    		});
    	});
    	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    		WString::Unmanaged(L"Application/Dialog_File/MultipleSelection_Root_TextFiles"),
    		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    		resourceFileDialogs,
    		CreateInstaller(fsMock)
    	);
    }
## STEP 6: Add a ctrl-click test for single-selection dialogs [DONE]
**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`
**Add this test case inside `TEST_CATEGORY(L"File Dialog Multiple Selection")`**:
    TEST_CASE(L"Ctrl-click behaves like click when multiple selection is disabled")
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
    			ClickFile(protocol, L"README");
    		});
    		protocol->OnNextIdleFrame(L"Selected: README", [=]()
    		{
    			ClickFile(protocol, L"root.txt", true, false, false);
    		});
    		protocol->OnNextIdleFrame(L"Ctrl-Clicked: root.txt", [=]()
    		{
    			auto dialogWindow = GetOpeningFileDialog();
    			auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
    			auto selectionText = textBox->GetText();
    			TEST_ASSERT(selectionText == WString::Unmanaged(L"root.txt"));
    			PressOpen(protocol);
    		});
    		protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
    		{
    			auto window = GetApplication()->GetMainWindow();
    			auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
    			auto&& fileNames = dialog->GetFileNames();
    			TEST_ASSERT(fileNames.Count() == 1);
    			TEST_ASSERT(fileNames[0] == FilePath(L"/root.txt").GetFullPath());
    			window->Hide();
    		});
    	});
    	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    		WString::Unmanaged(L"Application/Dialog_File/SingleSelection_CtrlClickBehavesLikeClick"),
    		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    		resourceFileDialogs,
    		CreateInstaller(fsMock)
    	);
    }

# FIXING ATTEMPTS

## Fixing attempt No.1

Unit tests failed before running due to XML resource precompile errors in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

- Why it failed: The Workflow script inside `<ev.Clicked-eval>` used `var output:string = "";`, which is not valid Workflow syntax (it expects spaces around `:` like `var output : string = "";`). This caused the XML compiler to stop at token `output`.
- What I changed: Updated both occurrences to `var output : string = "";`.
- Why this should work: This matches Workflow syntax used elsewhere in the repo, allowing the XML resource to precompile and the related unit tests to execute.

## Fixing attempt No.2

After fixing XML precompile, one new test case failed on an assertion in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

- Why it failed: The `FileDialogViewModel::GetDisplayString(...)` implementation always formats selected file names as quoted strings (e.g. `"root.txt"`), even when only one file is selected.
- What I changed: Updated the assertion in `Ctrl-click behaves like click when multiple selection is disabled` to expect `"root.txt"` instead of `root.txt`.
- Why this should work: The assertion now matches the dialog’s existing display-string format while still validating that ctrl-click changes the selection to `root.txt` in single-selection mode.

# !!!FINISHED!!!

# !!!VERIFIED!!!
