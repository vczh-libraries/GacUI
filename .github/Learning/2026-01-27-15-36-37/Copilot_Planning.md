# !!!PLANNING!!!

# UPDATES

## UPDATE

name of a frame describe what the previous frame has done, and each frame should make visible UI changes. Please review your test cases. And only OpenFileDialog able to choose multiple files (therefore get file names using FileNames or GetFileNames()), so when you cover these test cases, use the OpenFileDialog, Leave SaveFileDialog for single file selection scenarios.

You can update the Open DefaultOptions button to print multiple file names separating with ";" to the window text, so file names can be logged.

## UPDATE

# UPDATE
you don't need another dialogOpenMulti, you can assign the option in the button's clicked event, just remember to set in both. And Workflow uses & instead of + to join strings

## UPDATE

instead of making a new ClickFile, you should just add ctrl/shift/alt to ClickFileInternal and pass falses to them in ClickFile and DbClickFile. And then you can make a new ClickFile wrapper. Reuse code reasonably.

for test coverage, save dialog and open dialog only different in options, you do not need to make the same test case run against both save and open dialog. And you also don't need to test navigate+filter given that navigate and filter are already tested. typed selection seem to be already covered also.

please focus on testing multi selection with or without filter. so I think you can just delete the save dialog from the xml

## UPDATE

# UPDATE
i dont like this
```

    			TEST_ASSERT(
    				selectionText == WString::Unmanaged(L"\"README\";\"root.txt\"") ||
    				selectionText == WString::Unmanaged(L"\"root.txt\";\"README\"")
    			);
```
just pick one, you still have chance to make the failed test pass if you make a wrong guess.

you need another test case to try to ctrl+click in a single selection open dialog box, where ctrl+click is just click

# EXECUTION PLAN

## STEP 1: Extend the test window UI to host open/multi-select dialogs

**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

The existing tests share one `resourceFileDialogs` XML that only creates `dialogOpen` and one button (`Open DefaultOptions`). To enable multi-selection from a separate button without disturbing existing tests, extend the XML by adding:

- a new button that invokes the dialog, and ensure both open buttons set `dialogOpen.Options` before calling `ShowDialog()`

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

**Why**:
- Existing test cases keep using the same `dialogOpen` and `Open DefaultOptions` button text, so no existing test logic needs to change.
- Multi-selection is enabled by assigning `dialogOpen.Options` in the `Open MultipleSelection` button before calling `ShowDialog()`, so no extra dialog instance is required.

## STEP 2: Extend ClickFileInternal to support modifier keys

**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` (`namespace gacui_file_dialog_template`)

`ClickFileInternal(...)` only supports normal click and double-click. Extend it to take `(ctrl, shift, alt)` so multi-selection can reuse the same “find row by file name” logic.

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

**Why**:
- Multi-selection requires ctrl-click at the UI layer, and the tests should drive the dialog the same way a user does.
- Reusing the existing “find row by file name” logic keeps tests resilient to row index changes caused by filters and navigation.

## STEP 3: Add a new file in the mock filesystem to enable filter + multi-selection

**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` (`CreateFileItemRoot()`)

Add one more `*.txt` file at root so selecting filter "Text Files" still leaves at least 2 selectable files:

    AddFile(root, L"root2.txt");

Update existing expected listings that enumerate root files (e.g. “Listing: Root + Filter” and “Navigation + Listing: Root -> A -> AA”) to include `root2.txt`.

**Why**:
- The existing mock filesystem only has one `*.txt` at root (`root.txt`), so a “filter + multi-selection” test cannot select multiple files without changing mock data.
- Keeping this at root avoids introducing navigation steps into the new multi-selection tests.

## STEP 4: Add a new TEST_CATEGORY focused on multiple selection

**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

Add a new category after `TEST_CATEGORY(L"File Dialog Typed Selection")`:

    TEST_CATEGORY(L"File Dialog Multiple Selection")
    {
    	// test cases here
    });

**Why**:
- Typed-selection behavior is already covered by `TEST_CATEGORY(L"File Dialog Typed Selection")`.
- This category only adds coverage for multi-selection, which is currently missing.

## STEP 5: Add multi-selection tests (without filter, and with filter)

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

**Why**:
- Covers multi-selection behavior both with and without an active filter, without duplicating existing navigation/filter/typed-selection coverage.
- Asserts both the selection string (textbox) and the final confirmed output (`GetFileNames()`), so the end-to-end behavior is validated.

## STEP 6: Add a ctrl-click test for single-selection dialogs

**Location**: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

Ctrl-click should behave as a normal click when multiple selection is not enabled. Add a test case that uses `Open DefaultOptions` (no `INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection` in `dialogOpen.Options`) and then performs a ctrl-click in the data grid.

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

**Why**:
- Ensures ctrl-click does not create multiple selection when `FileDialogAllowMultipleSelection` is not enabled.

# TEST PLAN

## Test Strategy

- Build unit tests using the existing “Build Unit Tests” task in the solution.
- Run unit tests using the existing “Run Unit Tests” task.

## Expected New Test Coverage

- `Application/Dialog_File/MultipleSelection_Root_AllFiles`
  - Open multi-select dialog, select `README` and ctrl-select `root.txt`, assert text box shows `"README";"root.txt"`, confirm `Open`, assert `GetFileNames()` contains both full paths.
- `Application/Dialog_File/MultipleSelection_Root_TextFiles`
  - Open multi-select dialog, set filter to “Text Files”, select `root.txt` and ctrl-select `root2.txt`, assert text box shows `"root.txt";"root2.txt"`, confirm `Open`, assert `GetFileNames()` contains both full paths.
- `Application/Dialog_File/SingleSelection_CtrlClickBehavesLikeClick`
  - Open default dialog, select `README` and ctrl-click `root.txt`, assert text box shows `root.txt`, confirm `Open`, assert `GetFileNames()` contains exactly `/root.txt`.

## Regression Checks

- Re-run the existing categories:
  - `File Dialog Navigation`
  - `File Dialog Typed Selection`
- Ensure they continue to locate and click `Open DefaultOptions` and access `dialogOpen` unchanged.

# !!!FINISHED!!!
