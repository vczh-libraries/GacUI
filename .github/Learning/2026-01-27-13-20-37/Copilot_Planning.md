# !!!PLANNING!!!

# UPDATES

## UPDATE

only modify TryConfirm when there is actually a bug, just take a note in the document

## UPDATE

ensure there are test case covers that:
- all files no extension appended and pass/not pass file existing test
- txt file typing file name without extension and pass/not pass file existing test
- test file typing file name with extension and pass/not pass file existing test

you can make the file existing test fail first, close the message box, keep the file dialog open, and modify the text box, and ensure it pass, so that these 3 can be covered by only 3 test cases natually.

this will be strict enough to ensure whether we really need to modify TryConfirm

# EXECUTION PLAN

## STEP 1: Disambiguate file dialog vs message dialog windows in UI tests

Change:

- Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to centralize window selection in `namespace gacui_file_dialog_template`:
	- Add `GetOpeningFileDialog()` to pick the top-level file dialog window (title matches, native parent is `nullptr`).
	- Add `GetOpeningMessageDialog()` to pick the owned modal message dialog window (title matches, native parent is not `nullptr`).
	- Refactor `PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter` and all in-test `dialogWindow` lookups (used for data grid assertions) to use `GetOpeningFileDialog()` so a concurrently-open message dialog cannot hijack window selection.

Proposed code (to be placed near existing helpers in `namespace gacui_file_dialog_template`):

	GuiWindow* GetOpeningFileDialog()
	{
		return From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w)
			{
				auto nativeWindow = w->GetNativeWindow();
				return w->GetText() == L"FileDialog" && nativeWindow && nativeWindow->GetParent() == nullptr;
			})
			.First();
	}

	GuiWindow* GetOpeningMessageDialog()
	{
		return From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w)
			{
				auto nativeWindow = w->GetNativeWindow();
				return w->GetText() == L"FileDialog" && nativeWindow && nativeWindow->GetParent() != nullptr;
			})
			.First();
	}

	void PressButton(UnitTestRemoteProtocol* protocol, const WString& buttonText)
	{
		auto window = GetOpeningFileDialog();
		auto button = FindControlByText<GuiButton>(window, buttonText);
		auto location = protocol->LocationOf(button);
		GetApplication()->InvokeInMainThread(window, [=]()
		{
			protocol->LClick(location);
		});
	}

Why:

- `FileSystemViewModel::TryConfirm` shows message boxes using `owner->GetText()` as the dialog title, which matches the file dialog title (`L"FileDialog"`), making `From(GetApplication()->GetWindows()).Where(...).First()` ambiguous once message boxes are involved.
- Centralized window selection keeps all existing and new dialog-driven tests stable while adding the regression case that intentionally shows a message dialog.

## STEP 2: Add typed-selection existence tests (dialog-driven)

Change:

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a new `TEST_CATEGORY` dedicated to typed selection + file-exists validation (driving `TryConfirm` through UI).
	- Reuse the existing filter string: `Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt"`.
	- Each test case should do a fail-then-pass sequence without closing the file dialog:
		- Attempt 1: trigger "File(s) not exist" and show a message dialog.
		- Close the message dialog and keep the file dialog open.
		- Attempt 2: change the text box and successfully confirm a selection.

Proposed test cases (3 total):

1) All Files: type without extension, no extension appended (fail then pass):

	TEST_CASE(L"Typed selection: All Files no extension (fail then pass)")
	{
		Ptr<FileSystemMock> fsMock;
		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
			});

			protocol->OnNextIdleFrame(L"Attempt 1: All Files + README.txt (fail)", [=]()
			{
				ChooseFilter(protocol, 0);
				TypeFile(protocol, L"README.txt");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Close message dialog", [=]()
			{
				auto messageWindow = GetOpeningMessageDialog();
				TEST_ASSERT(messageWindow != nullptr);
				protocol->KeyPress(VKEY::KEY_RETURN);
			});

			protocol->OnNextIdleFrame(L"Attempt 2: All Files + README (pass)", [=]()
			{
				TypeFile(protocol, L"README");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Confirmed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
				TEST_ASSERT(dialog->GetFileNames().Count() == 1);
				TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/README").GetFullPath());
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/TypedSelection_AllFiles_NoExt_FailThenPass"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	}

2) Text Files: type without extension, auto-append + file-exists validation (fail then pass):

	TEST_CASE(L"Typed selection: Text Files no extension (fail then pass)")
	{
		Ptr<FileSystemMock> fsMock;
		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
			});

			protocol->OnNextIdleFrame(L"Attempt 1: Text Files + README (fail)", [=]()
			{
				ChooseFilter(protocol, 1);
				TypeFile(protocol, L"README");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Close message dialog", [=]()
			{
				auto messageWindow = GetOpeningMessageDialog();
				TEST_ASSERT(messageWindow != nullptr);
				protocol->KeyPress(VKEY::KEY_RETURN);
			});

			protocol->OnNextIdleFrame(L"Attempt 2: Text Files + root (pass)", [=]()
			{
				TypeFile(protocol, L"root");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Confirmed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
				TEST_ASSERT(dialog->GetFileNames().Count() == 1);
				TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root.txt").GetFullPath());
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/TypedSelection_TextFiles_NoExt_FailThenPass"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	}

3) Type with extension, validate exact existence (fail then pass):

	TEST_CASE(L"Typed selection: Text Files with extension (fail then pass)")
	{
		Ptr<FileSystemMock> fsMock;
		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
			});

			protocol->OnNextIdleFrame(L"Attempt 1: Text Files + README.txt (fail)", [=]()
			{
				ChooseFilter(protocol, 1);
				TypeFile(protocol, L"README.txt");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Close message dialog", [=]()
			{
				auto messageWindow = GetOpeningMessageDialog();
				TEST_ASSERT(messageWindow != nullptr);
				protocol->KeyPress(VKEY::KEY_RETURN);
			});

			protocol->OnNextIdleFrame(L"Attempt 2: Text Files + root.txt (pass)", [=]()
			{
				TypeFile(protocol, L"root.txt");
				PressOpen(protocol);
			});

			protocol->OnNextIdleFrame(L"Confirmed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
				TEST_ASSERT(dialog->GetFileNames().Count() == 1);
				TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root.txt").GetFullPath());
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/TypedSelection_TextFiles_WithExt_FailThenPass"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	}

4) Regression: extension normalization must happen before existence checks (typing `README` under `Text Files` should show “File(s) not exist” and keep dialog open):

This scenario is covered by case (2), no additional test case is needed.

Why:

- Confirms the UI actually updates its listing when filters change (bindings + view model filtering logic).
- Covers the extension auto-append semantics at the point it matters: when the dialog tries to confirm a typed selection.
- The regression test forces the ambiguous “two windows with the same title” scenario and ensures window selection helpers + `TryConfirm` logic are correct together.

Updated Why (supersedes previous):

- Existing listing/navigation tests already cover filter visibility changes; the missing part is typed selection validation and message dialog flows.
- Each test is stateful: fail shows a message dialog, dismissing it keeps the file dialog open, and then editing the text box can confirm successfully.
- Case (2) is strict enough to decide whether `TryConfirm` must be changed: `README` exists but `README.txt` does not, so validating before appending would incorrectly pass.

## STEP 3: Fix `FileSystemViewModel::TryConfirm` to validate normalized selections

Change:

- Only if the regression test in STEP 2 fails, update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` in `FileSystemViewModel::TryConfirm` so that:
	- Single-selection folder navigation continues to use the raw (non-appended) typed path.
	- For file confirmation, selected paths are normalized (extension auto-append) before existence checks, error messages, and prompt logic.
	- `confirmedSelection` is populated from normalized paths directly, removing the post-validation “append extensions” block.

Proposed code (key structure inside `TryConfirm`, replacing the current “classify then append extensions” ordering):

	bool TryConfirm(controls::GuiWindow* owner, Selection selectedPaths) override
	{
		auto wd = selectedFolder->folder.GetFilePath();
		List<filesystem::FilePath> rawPaths;
		CopyFrom(rawPaths, selectedPaths.Select([wd](auto path) { return wd / path; }));

		if (rawPaths.Count() == 0)
		{
			// unchanged: dialogErrorEmptySelection
			...
		}

		// folder navigation must use the raw selection (no extension append)
		if (rawPaths.Count() == 1)
		{
			auto path = rawPaths[0];
			if (filesystem::Folder(path).Exists())
			{
				SetSelectedFolderInternal(path, true);
				return false;
			}
		}
		else if (!enabledMultipleSelection)
		{
			// unchanged: dialogErrorMultipleSelectionNotEnabled
			...
		}

		Nullable<WString> extension;
		bool extensionFromFilter = false;
		if (selectedFilter)
		{
			extension = selectedFilter->GetDefaultExtension();
			extensionFromFilter = extension;
		}
		if (!extensionFromFilter && defaultExtension != WString::Empty)
		{
			extension = defaultExtension;
		}

		auto normalized = [&](filesystem::FilePath path)
		{
			if (!extension) return path;

			auto sExt = WString::Unmanaged(L".") + extension.Value();
			auto lExt = sExt.Length();
			auto full = path.GetFullPath();

			if (extensionFromFilter)
			{
				if (full.Length() >= lExt && full.Right(lExt) == sExt)
				{
					return path;
				}
			}
			else
			{
				auto selectedFileName = path.GetName();
				if (INVLOC.FindFirst(selectedFileName, WString::Unmanaged(L"."), Locale::None).key != -1)
				{
					return path;
				}
			}
			return filesystem::FilePath(full + sExt);
		};

		List<filesystem::FilePath> paths;
		CopyFrom(paths, From(rawPaths).Select([&](auto path) { return normalized(path); }));

		// classify using normalized paths
		List<vint> files, folders, unexistings;
		for (auto [path, index] : indexed(paths))
		{
			if (filesystem::File(path).Exists())
			{
				files.Add(index);
			}
			else if (filesystem::Folder(path).Exists())
			{
				folders.Add(index);
			}
			else
			{
				unexistings.Add(index);
			}
		}

		// unchanged: error/prompt logic, but all messages use `paths[index]`
		// (so “File(s) not exist” lists README.txt, not README)
		...

		CopyFrom(confirmedSelection, From(paths).Select([](auto path) { return path.GetFullPath(); }));
		confirmed = true;
		return true;
	}

Why:

- The selected filter default extension (e.g. `txt`) changes what the dialog means by a typed selection; validation must apply to the final (user-visible) selection, not the raw input.
- If the regression reproduces, fixes the case where typing `README` under `Text Files (*.txt)` can incorrectly confirm (because `README` exists) and then silently mutate to `/README.txt` after confirmation.
- Preserves folder navigation: typing `A` still navigates into folder `/A` even when a restrictive filter is active.

# TEST PLAN

- Run the dialog-driven tests in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`:
	- Existing navigation/listing cases remain unchanged in behavior after the helper refactor.
	- New typed-selection existence cases pass and are stable when a message dialog is present.
- Regression verification:
	- (All Files) Fail: `README.txt` -> message dialog; Pass: `README` -> confirm `/README`.
	- (Text Files) Fail: `README` -> message dialog; Pass: `root` -> confirm `/root.txt`.
	- (Text Files) Fail: `README.txt` -> message dialog; Pass: `root.txt` -> confirm `/root.txt`.
	- If the `README` under `Text Files` case already behaves as expected on current code, keep `FileSystemViewModel::TryConfirm` unchanged.

# !!!FINISHED!!!
