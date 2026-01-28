# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add an "Open PromptCreateFile" launcher button [DONE]

File: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

In `resourceFileDialogs` (the `LR"GacUISrc(... )GacUISrc"` string), update the `Table` to add one more row and a new launcher button.

1) Replace the existing `<att.Rows>` with:

    <att.Rows>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:MinSize</_>
      <_>composeType:Percentage percentage:1.0</_>
    </att.Rows>

2) Insert a new `Cell` after the existing `Save PromptOverwriteFile` cell (`Site="row:3 column:0"`):

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

## STEP 2: Add message dialog helpers for deterministic close/assert [DONE]

File: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

In `namespace gacui_file_dialog_template`, add non-asserting “try get” helpers and click helpers for message dialogs, and make the existing `GetOpening*` wrappers call the “try get” versions.

1) Add these helper functions (place them near the existing `GetOpeningFileDialog` / `GetOpeningMessageDialog`):

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

2) Add message dialog click helpers (place them near `PressButton`):

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

	void PressMessageOK(UnitTestRemoteProtocol* protocol)
	{
		PressMessageButton(protocol, L"OK");
	}

	void PressMessageCancel(UnitTestRemoteProtocol* protocol)
	{
		PressMessageButton(protocol, L"Cancel");
	}

## STEP 3: Add TEST_CATEGORY for TryConfirm-driven message box interactions [DONE]

File: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`

Add a new `TEST_CATEGORY(L"File Dialog Message Box Interactions")` after the existing categories.

All new `UnitTestRemoteProtocol::OnNextIdleFrame` frames must follow:

- The frame name describes what the previous frame has done.
- Each frame performs one user-visible UI action (click launcher / type / press confirm / close message dialog / cancel dialog / hide main window), plus any assertions.

Insert this whole category:

	TEST_CATEGORY(L"File Dialog Message Box Interactions")
	{
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
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
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
		});

		TEST_CASE(L"Prompt create file (open): Cancel keeps open; OK commits")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Clicked: Open PromptCreateFile", [=]()
				{
					TypeFile(protocol, L"new-created.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: new-created.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message Cancel", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open again", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() == nullptr);

					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/new-created.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_PromptCreateFile_Open_CancelThenOK"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Prompt overwrite file (save): Cancel keeps open; OK commits")
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
				protocol->OnNextIdleFrame(L"Clicked: Save PromptOverwriteFile", [=]()
				{
					TypeFile(protocol, L"root.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: root.txt", [=]()
				{
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Save", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message Cancel", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Save again", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() == nullptr);

					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_PromptOverwriteFile_Save_CancelThenOK"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Multiple selection not enabled: type a;b shows error and stays open")
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
					TypeFile(protocol, L"README;root.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: README;root.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_MultipleSelectionNotEnabled_Error_StaysOpen"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"File expected but folder selected: type README;A shows error and stays open")
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
					TypeFile(protocol, L"README;A");
				});
				protocol->OnNextIdleFrame(L"Typed: README;A", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_FileExpected_Error_StaysOpen"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});
	});

## STEP 4: Validation checklist

- Build and run unit tests; ensure `File Dialog Message Box Interactions` passes.
- Ensure every new `OnNextIdleFrame` frame name describes what the previous frame did.
- Ensure every new frame performs one UI action (plus assertions), and message dialogs are closed only by clicking `OK` / `Cancel`.

# FIXING ATTEMPTS

- None.

# !!!FINISHED!!!

# !!!VERIFIED!!!
