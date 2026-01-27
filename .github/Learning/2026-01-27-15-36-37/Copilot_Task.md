# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.4: TEST_CATEGORY for typed selection + multiple selection (in dialog)

Add dialog-driven tests that focus on the selection string in the file name text box, including multiple selection behavior when enabled.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` for typed selection and multi-selection.
- For typed selection:
	- Navigate into a folder, set a specific filter, call `TypeFile(protocol, ...)`, and click `Open` / `Save`.
	- Assert dialog output via `GuiOpenFileDialog::GetFileNames()` (or `GuiSaveFileDialog::GetFileName()` when using save dialog).
	- Keep "type" and "confirm" in separate idle frames when needed (so each frame produces UI updates and remains non-blocking).
- For multiple selection:
	- Add a second dialog instance (or a second button) that enables multiple selection by setting `GuiFileDialogBase::Options` to include `INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection`.
	- Add a case that selects multiple files in the data grid using helpers from `Test/GacUISrc/UnitTest/TestControls_List.h` (e.g. ctrl-click) and then presses `Open`.
	- Assert that `GetFileNames()` returns all expected full paths.

### rationale

- The file dialog's user-facing contract is the selection string plus final selected file list; testing it through the actual dialog ensures `FilePickerControl`'s `Selection` logic (based on data grid selection and text box input) is covered.
- Selection outcomes depend on the active filter, which persists across navigation; tests should explicitly set the filter before selecting or typing file names.

# UPDATES

## UPDATE

if you need a save file dialog, you can add a new button to the window, to avoid messing up existing test cases

# INSIGHTS AND REASONING

## Current Status / Evidence in Code

- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` already uses dialog-driven UI tests (via `GacUIUnitTest_SetGuiMainProxy` + `UnitTestRemoteProtocol`) and accesses file dialog internals by `ref.Name` (`filePickerControl`, `dataGrid`, `textBox`, `comboBox`).
- In `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, `FileDialogViewModel::TryConfirm(...)`:
	- Treats single selection to an existing folder as navigation (`filesystem::Folder(path).Exists()` then `SetSelectedFolderInternal(...)`).
	- Rejects multiple selection when `enabledMultipleSelection == false` (shows `dialogErrorMultipleSelectionNotEnabled`).
	- Applies extension normalization based on the selected filter's default extension (and then `defaultExtension`) before existence checks (`filesystem::File(path).Exists()`).
  These rules define what should be asserted in dialog-driven tests.

## Proposed Test Coverage (Dialog-driven)

### 1) Typed selection (folder navigation + filter + typing + confirm)

Goal: validate the "type in file name text box -> confirm" path when the selected folder is not root, and when a filter implies an extension.

Design:
- Ensure a typed-selection test explicitly navigates to a non-root folder before typing:
	- Open dialog.
	- Navigate into `A` using double-click on the data grid.
	- Set filter to "Text Files (*.txt)" (index 1).
	- Type a file name that needs extension append to become an existing file (e.g. type `a`, expect `/A/a.txt`).
	- Confirm with `Open` (and optionally a parallel test with `Save` using a separate `GuiSaveFileDialog` instance).
- Keep "type" and "confirm" in separate idle frames in the proxy:
	- Frame N: navigation + filter selection.
	- Frame N+1: `TypeFile(protocol, ...)`.
	- Frame N+2: press `Open` / `Save`.
	- Frame N+3: assert `GuiOpenFileDialog::GetFileNames()` (or `GuiSaveFileDialog::GetFileName()`), then close the main window.

Why this is important:
- It validates that typed names are interpreted relative to the current folder and that filter-derived extension normalization happens before `FileDialogFileMustExist` checks (matching the `TryConfirm` logic).

Implementation note (test UI stability):
- Keep the existing "Open DefaultOptions" button and `dialogOpen` resource unchanged, since multiple existing test cases locate them by text and `ref.Name`.
- When save-dialog coverage is needed, add a new `SaveFileDialog` instance (e.g. `ref.Name="dialogSave"`) and a new button (e.g. "Save DefaultOptions") that calls `dialogSave.ShowDialog()` and stores `dialogSave.FileName` (or similar) to the main window; save-dialog tests should click this new button instead of reusing the open button.

### 2) Multiple selection (data grid selection -> confirm)

Goal: validate multi-selection via `FilePickerControl` selection, not by directly calling `IFileDialogViewModel`.

Design:
- Extend the test resource UI (the `resourceFileDialogs` XML in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`):
	- Add a second `OpenFileDialog` instance (e.g. `dialogOpenMulti`) that sets `GuiFileDialogBase::Options` to include `INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection` (combined with existing options using `|`, as in other XML usages in the repo).
	- Add a second button (e.g. "Open MultipleSelection") that calls `dialogOpenMulti.ShowDialog()` and stores results (same pattern as the existing button).
- Add a test case under the typed-selection category or a dedicated "Multiple Selection" category:
	- Open the multi-select dialog via the new button.
	- Navigate into `A` (so multiple files exist in the list).
	- Explicitly select a filter (recommend "All Files" to avoid extension appending causing unexpected non-existing paths).
	- Select multiple files in the data grid using `Test/GacUISrc/UnitTest/TestControls_List.h` helpers:
		- First select one item normally, then ctrl-click another (using the overload with `(ctrl, shift, alt)`).
	- Press `Open`.
	- Assert `GuiOpenFileDialog::GetFileNames()` returns all expected full paths (recommend comparing as a set / sorted list to avoid depending on selection ordering).

Why this is important:
- `FileDialogViewModel::GetDisplayString(...)` and `ParseDisplayString(...)` define how the selection text box represents multiple selections (quotes + `;`), but the real product behavior is the final selected file list; this test ensures UI selection wiring produces correct confirmation output.

## Non-goals (Deferred to TASK No.5)

- Message box interaction matrix for open/save (file-not-exist, overwrite/create prompts, and "multiple selection not enabled") should be covered as dedicated dialog-driven tests in TASK No.5, because they require deterministic interaction with dialogs created by `TryConfirm(...)`.

# !!!FINISHED!!!
