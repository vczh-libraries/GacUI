# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add `GuiSaveFileDialog` Resource Entry [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` in the `resourceFileDialogs` resource text.

Insert a save dialog entry right after the existing open dialog entry (reuse the same `Title`, `Filter`, and `Directory`):

```
				<OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
				<SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
```

## STEP 2: Add Buttons to Open Save Dialog With Save-only Options [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` in the same `resourceFileDialogs` `<Table ...>` so existing open buttons remain unchanged, and add two more rows and two more buttons.

1) Replace the `<att.Rows>` block so two new `MinSize` rows are inserted before the final `Percentage` row:

```
					<att.Rows>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:MinSize</_>
						<_>composeType:Percentage percentage:1.0</_>
					</att.Rows>
```

2) Insert the following `<Cell>` blocks after the existing open-dialog `<Cell>` blocks (keep existing open-dialog button texts/handlers unchanged):

```
					<Cell Site="row:2 column:0">
						<Button Text="Save PromptCreateFile">
							<att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
							<ev.Clicked-eval><![CDATA[{
								dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
								if (dialogSave.ShowDialog())
								{
									self.Text = dialogSave.FileName;
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
									self.Text = dialogSave.FileName;
								}
							}]]></ev.Clicked-eval>
						</Button>
					</Cell>
```

## STEP 3: Add Smoke Tests for Save Dialog Entry Points [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to add minimal tests that each new main-window button opens the save dialog and can be cancelled.

Add the following `TEST_CASE`s under a suitable existing category (e.g. `TEST_CATEGORY(L"File Dialog Navigation")`), following the existing proxy-frame patterns:

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
					TEST_ASSERT(dialog->GetFileName().Length() == 0);
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
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName().Length() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/SavePromptOverwriteFile_OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		}
```

## STEP 4: Regression and Stability Checks [DONE]

- Run existing `TestApplication_Dialog_File.cpp` categories to ensure open-dialog tests remain unchanged and stable.
- Confirm `Title="FileDialog"` stays identical for both dialogs so `GetOpeningFileDialog()` continues to locate the active dialog window without branching.

# FIXING ATTEMPTS

## Fixing attempt No.1

- Why the original change didn't work: The new save-dialog tests called `vl::presentation::controls::GuiSaveFileDialog::GetFileNames()`, but `GuiSaveFileDialog` only supports a single file name via `GuiFileDialogBase::GetFileName()`.
- What I changed: Updated the two cancel-path assertions in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to use `dialog->GetFileName().Length() == 0` instead of `dialog->GetFileNames().Count() == 0`.
- Why it should solve the build break: The code now calls an API that exists for `GuiSaveFileDialog`, so the unit test project should compile while still asserting that cancelling leaves no selected file.

# !!!FINISHED!!!

# !!!VERIFIED!!!
