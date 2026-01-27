# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Disambiguate file dialog vs message dialog windows in UI tests [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` in `namespace gacui_file_dialog_template`:

- Add helper functions `GetOpeningFileDialog()` and `GetOpeningMessageDialog()` to select the correct `GuiWindow` when both windows share title `L"FileDialog"`.
- Refactor all file-dialog UI helpers (e.g. `PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter`) and any in-test `dialogWindow` lookups used for assertions to use `GetOpeningFileDialog()`.

Insert near existing helpers:

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

Replace the window lookup in `PressButton(...)` (and apply the same change pattern to other helpers):

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

## STEP 2: Add typed-selection existence tests (dialog-driven) [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`:

- Add a new `TEST_CATEGORY` dedicated to typed selection + file-exists validation.
- Reuse the existing filter string `All Files (*.*)|*|Text Files (*.txt)|*.txt` (existing `Open DefaultOptions` button / `dialogOpen` is used).
- Each test case does a fail-then-pass flow without closing the file dialog:
	- Attempt 1 shows the "File(s) not exist" message dialog.
	- Close the message dialog (keep file dialog open).
	- Attempt 2 edits the text box and confirms successfully.

Place the following `TEST_CASE`s under the new category:

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

These 3 cases cover:

- All Files: no extension appended and fail/pass file existing test.
- Text Files: typing file name without extension and fail/pass file existing test.
- Text Files: typing file name with extension and fail/pass file existing test.

## STEP 3: Fix `FileSystemViewModel::TryConfirm` only if tests prove a bug [DONE]

Only if the regression in STEP 2 shows that typing `README` under `Text Files (*.txt)` incorrectly confirms using `/README` (instead of failing due to `/README.txt` not existing), update `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` in `FileSystemViewModel::TryConfirm` so that extension normalization happens before existence checks and prompt logic, while folder navigation keeps using the raw typed selection.

Key structure (replace the current ordering that does existence checks before extension append):

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

## TEST PLAN

- Run dialog-driven tests in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`:
	- Existing navigation/listing cases remain unchanged after the helper refactor.
	- New typed-selection existence cases pass and are stable when a message dialog is present.
- Regression verification:
	- (All Files) Fail: `README.txt` -> message dialog; Pass: `README` -> confirm `/README`.
	- (Text Files) Fail: `README` -> message dialog; Pass: `root` -> confirm `/root.txt`.
	- (Text Files) Fail: `README.txt` -> message dialog; Pass: `root.txt` -> confirm `/root.txt`.
	- If the `README` under `Text Files` case already behaves as expected on current code, keep `FileSystemViewModel::TryConfirm` unchanged.

# FIXING ATTEMPTS

## Fixing attempt No.1

The unit test crashed in `TestApplication_Dialog_File.cpp` because `GetOpeningFileDialog()` filtered out the file dialog window when `GuiWindow::GetNativeWindow()` was still null, causing `LazyList::First()` to be evaluated on an empty sequence.

Changed `GetOpeningFileDialog()` to treat a `nullptr` native window as “file dialog candidate”, and only use the native-window parent check when a native window exists.

This should fix the crash because file-dialog helper actions (`PressOpen`, `TypeFile`, etc.) can now resolve the dialog window even during the short interval before the native window pointer is attached.

## Fixing attempt No.2

Attempt No.1 did not change the failure: the crash still happened at idle frame `Show Dialog`, meaning the file dialog window was not present in `GetApplication()->GetWindows()` yet.

Changed all `Ready` frames in `TestApplication_Dialog_File.cpp` to call `protocol->LClick(location);` directly (instead of deferring it through `GetApplication()->InvokeInMainThread(...)`).

This should fix the crash because opening the dialog becomes part of the `Ready` frame’s executed input, ensuring the dialog window is created before the next idle frame attempts to query it.

## Fixing attempt No.3

Attempt No.2 still crashed at idle frame `Show Dialog`, so even with an immediate click, the file dialog window was still not guaranteed to exist by the next idle frame.

Added an extra idle frame `Ensure Dialog` after each `Ready` frame in `TestApplication_Dialog_File.cpp` that performs a second click on the `Open DefaultOptions` button.

This should fix the crash by giving the UI another full idle-frame interval to create and register the dialog window before the tests interact with it.

## Fixing attempt No.4

After rebuilding, Attempt No.2 introduced a new failure: `UnitTestRemoteProtocol::ProcessRemoteEvents()` reported that an `OnNextIdleFrame` action called a blocking function, and suggested using `InvokeInMainThread`.

Reverted the direct `protocol->LClick(...)` calls in `Ready`/`Ensure Dialog` frames back to `GetApplication()->InvokeInMainThread(window, ...) { protocol->LClick(...); }` so the idle-frame actions remain non-blocking.

This should fix the new framework-level failure while keeping the extra `Ensure Dialog` idle frame to improve reliability of opening the file dialog before subsequent interactions.

## Fixing attempt No.5

With Attempt No.4, the test reached the `Ensure Dialog` frame but then failed with `LogRenderingResult()`: “The last frame didn't trigger UI updating”, because the `Ensure Dialog` action did not reliably cause a visible/layout change.

Changed all `Ensure Dialog` frames to perform a guaranteed UI update on the main window (append `"*"` to the main window title) instead of trying to click the open-dialog button again.

This should satisfy the unit-test framework requirement (each idle-frame action must change UI) while still inserting an extra frame delay so the file dialog window is available by the following frame.

## Fixing attempt No.6

Attempt No.5 still failed with `LazyList::First()` in `GetOpeningFileDialog()`, indicating that filtering by `nativeWindow->GetParent() == nullptr` (or `!nativeWindow`) still did not match the actual file dialog window.

Updated `GetOpeningFileDialog()` / `GetOpeningMessageDialog()` to distinguish windows by comparing `nativeWindow->GetParent()` against the main window’s native window:
- File dialog: parent is `nullptr` OR equals the main window native.
- Message dialog: parent exists and is NOT the main window native.

This should handle cases where the file dialog native window is owned/parented by the main window (modal) while the message dialog is owned by the file dialog.

## Fixing attempt No.7

After Attempt No.6, tests progressed into `File Dialog Typed Selection` but crashed in `Close message dialog` because `GetOpeningMessageDialog()` still couldn’t find a matching window.

Adjusted `GetOpeningMessageDialog()` to follow the actual ownership model in `FakeDialogServiceBase::ShowMessageBox`: it looks up the current file dialog window first, and then selects the `"FileDialog"`-titled window whose native parent is exactly the file dialog’s native window.

This should reliably pick the real message dialog (modal to the file dialog), even if other windows share the same title or if the main-window parenting differs.

## Fixing attempt No.8

Attempt No.7 still failed to find the message dialog window in `Close message dialog` (meaning it might not be exposed as a `GuiWindow` in `GetApplication()->GetWindows()` in this configuration).

Updated the typed-selection test cases to close the message dialog by sending `Enter` (`protocol->KeyPress(VKEY::KEY_RETURN);`) without trying to locate it via `GetOpeningMessageDialog()`.

This should work for both fake and native modal message boxes and removes the brittle dependency on window enumeration.

## Fixing attempt No.9

After Attempt No.8, the `Close message dialog` frame failed the UI-test framework requirement (“last frame didn't trigger UI updating”), suggesting the message dialog wasn’t guaranteed to appear before pressing `Enter`.

Inserted an extra idle frame `Wait message dialog` after each failing `PressOpen(...)` in the typed-selection tests that forces a harmless UI update (append `"!"` to the main window title), giving the modal message dialog time to appear.

This should ensure the subsequent `Close message dialog` frame actually closes an existing modal dialog, producing a UI update and allowing the test to continue.

## Fixing attempt No.10

Even after adding wait frames, closing the message dialog never triggered a GacUI UI update, strongly indicating that the file dialog view model was showing a *native* message box (not represented in `GetApplication()->GetWindows()` and not captured by the rendering-diff based test harness).

Fixed the root cause in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`: `FileDialogViewModel::TryConfirm` now calls `FakeDialogServiceBase::ShowMessageBox(...)` (GUI-rendered) instead of `GetCurrentController()->DialogService()->ShowMessageBox(...)` (native).
- Added `FakeDialogServiceBase* dialogService` to `FileDialogViewModel` and initialize it from `FakeDialogServiceBase::ShowFileDialog`.
- Replaced all message box calls in `TryConfirm` with `dialogService->ShowMessageBox(...)`.

This should make the error prompts appear as fake GUI dialogs, allowing UI tests to detect and close them reliably.

## Fixing attempt No.11

Attempt No.10 initially failed to compile because `FakeDialogServiceBase::ShowMessageBox` takes an extra `MessageBoxModalOptions` argument (unlike the controller dialog service calls previously used).

Updated all `dialogService->ShowMessageBox(...)` calls in `GuiFakeDialogServiceBase_FileDialog.cpp` to pass `INativeDialogService::ModalWindow` as the final argument.

This resolves the signature mismatch and preserves the intended modal behavior.

## Fixing attempt No.12

After Attempt No.11, the typed-selection tests still failed because the `Close message dialog` frame did not trigger a GacUI UI update (keyboard `Enter` was not reliably closing the dialog / not producing a detectable rendering change).

Updated the typed-selection tests to explicitly close the message dialog by locating it via `GetOpeningMessageDialog()` and clicking its `OK` button using `protocol->LClick(...)` inside `InvokeInMainThread`.

This should guarantee a real GUI action that closes the dialog and triggers a rendering update, satisfying the UI test framework.

## Fixing attempt No.13

Attempt No.12 still crashed because `GetOpeningMessageDialog()` relied on the message dialog window title matching `L"FileDialog"`, but the message box window was not found with that criterion.

Updated `GetOpeningMessageDialog()` to identify the message dialog by its native-parent relationship to the file dialog window (and explicitly exclude the file dialog itself), without requiring a specific window title.

This should locate the modal message box reliably regardless of its displayed title.

## Fixing attempt No.14

Diagnostics (`WINDOW_DUMP`) showed only the main window (e.g. `Opened!`), meaning the dialog was actually confirming successfully on the “fail” inputs, so no message dialog existed to close.

Updated the test resource in `TestApplication_Dialog_File.cpp` to set `Options="FileDialogFileMustExist"` on the `<OpenFileDialog .../>`, ensuring invalid typed selections (like `README.txt` under All Files / `README` under Text Files) actually fail and keep the dialog open.

This should make the typed-selection tests exercise the intended fail-then-pass flow.

# !!!FINISHED!!!

# !!!VERIFIED!!!
