# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add `GuiSaveFileDialog` Resource Entry

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to extend `resourceFileDialogs` with a save dialog entry point.

- Change: add `<SaveFileDialog ref.Name="dialogSave" .../>` alongside the existing `<OpenFileDialog ref.Name="dialogOpen" .../>`.
- Why: later tasks need to exercise save-only options (create/overwrite prompts) without reworking the test UI; having both resources in the same window keeps interaction helpers stable.

Proposed code (inside `resourceFileDialogs`):
```
				<OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
				<SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
```

## STEP 2: Add Buttons to Open Save Dialog With Save-only Options

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to add two buttons that show the save dialog and set `dialogSave.Options` appropriately.

- Change: extend the existing `<Table>` by adding 2 new min-size rows and 2 new `<Cell>` entries.
- Constraint: keep the existing open-dialog buttons unchanged (both text and behavior) to avoid breaking existing tests that locate them by `Text`.
- Why: `INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile` and `INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile` are only meaningful in save mode; these buttons are the stable entry points for later message-box interaction tests.

Proposed code (new rows and cells only; existing open cells unchanged):
```
					<att.Rows>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:Percentage percentage:1.0</_>
					</att.Rows>
...
					<Cell Site="row:2 column:0">
						<Button Text="Save PromptCreateFile">
							<att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
							<ev.Clicked-eval><![CDATA[{
								dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
								if (dialogSave.ShowDialog())
								{
									var output : string = "";
									for (fileName in dialogSave.FileNames)
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
					<Cell Site="row:3 column:0">
						<Button Text="Save PromptOverwriteFile">
							<att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
							<ev.Clicked-eval><![CDATA[{
								dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile;
								if (dialogSave.ShowDialog())
								{
									var output : string = "";
									for (fileName in dialogSave.FileNames)
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
```

## STEP 3: Add Smoke Tests for Save Dialog Entry Points

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to add a minimal test that each new main-window button opens a save dialog that can be cancelled.

- Change: add 2 `TEST_CASE`s (or a small new `TEST_CATEGORY`) that click each new button, then dismiss the dialog by clicking `Cancel`.
- Why: verifies the UI wiring is correct (resource resolves, options assignment doesn’t break dialog creation, and the dialog can open/close) without introducing any prompt/message-box assertions in this task.

Proposed code (pattern):
```
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
			PressCancel(protocol);
		});
		protocol->OnNextIdleFrame(L"Cancel", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
			TEST_ASSERT(dialog->GetFileNames().Count() == 0);
			window->Hide();
		});
	});

	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Application/Dialog_File/SavePromptCreateFile_OpenAndClose"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resourceFileDialogs,
		CreateInstaller(fsMock)
	);
}
```

## STEP 4: Regression and Stability Checks

- Run existing `TestApplication_Dialog_File.cpp` categories to ensure open-dialog tests remain unchanged and stable.
- Confirm `Title="FileDialog"` stays identical for both dialogs so `GetOpeningFileDialog()` continues to locate the active dialog window without branching.

# !!!FINISHED!!!
