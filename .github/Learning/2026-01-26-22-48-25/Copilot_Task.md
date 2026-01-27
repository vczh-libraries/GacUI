# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.2: TEST_CATEGORY for file dialog open + folder navigation + listing

Add dialog-driven unit tests (opening the real `GuiOpenFileDialog`) to validate folder navigation and file listing, using `FileSystemMock` to control dialog contents.

### what to be done

- Extend `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` by adding a `TEST_CATEGORY` for folder navigation / listing.
- Reuse the existing patterns in the prepared sample:
	- Launch via `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)`.
	- Find the opened dialog window by title `L"FileDialog"` (matching `GuiOpenFileDialog::Title` from the resource).
	- Access key controls by name through `filePickerControl`:
		- Data grid: `FindObjectByName<GuiBindableDataGrid>(window, L"filePickerControl", L"dataGrid")`.
		- Text box: `FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox")`.
		- Filter combo box: `FindObjectByName<GuiComboBoxListControl>(window, L"filePickerControl", L"comboBox")`.
- Add subcases that validate listing / navigation through the UI:
	- Root listing contains expected items from `CreateFileItemRoot()` (e.g. `root.txt`, `README`, `A`, `B`).
	- Double-clicking a folder in the data grid navigates into that folder (e.g. `DbClickFile(protocol, L"A")`), and the new listing matches expected children (e.g. `a.txt`, `noext`, `AA`).
	- Navigating deeper (e.g. `AA`) updates listing accordingly.
	- Cancelling closes the dialog with `GetFileNames().Count() == 0`.
- When asserting listing, prefer reading from `dataGrid->GetItemProvider()` and comparing `GetTextValue(i)` results, following the sample `ClickFileInternal` pattern.

### rationale

- The file dialog view model depends on localized resources and control templates; driving the real dialog keeps tests representative and avoids brittle “test-only” APIs.
- Navigation and listing are the foundation for all later selection/confirmation tests; keeping them in a dedicated category keeps failures easy to diagnose.

# UPDATES

## UPDATE

# Update
remember to read item text in datagrid to ensure it has expected filtered content: alphabetical ordered folder followed by filtered and alphabetical ordered files. reading texts from items are enough, you don't need to tell if each one is folder or file.

# INSIGHTS AND REASONING

## What already exists to build on

- `Test/GacUISrc/UnitTest/TestControls.h` already provides `FileItemMock`, `FileSystemMock`, and injection via `vl::filesystem::InjectFileSystemImpl`, which gives deterministic folder/file enumeration for the dialog.
- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` already contains a working dialog-driven sample:
	- Launches the app with embedded XML via `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)`.
	- Finds the dialog window by `GuiWindow::GetText() == L"FileDialog"`.
	- Operates the UI using `UnitTestRemoteProtocol` and list helpers (`LClickListItem`, `LDBClickListItem`) from `Test/GacUISrc/UnitTest/TestControls_List.h`.
	- Reads rendered rows via `GuiBindableDataGrid::GetItemProvider()` + `IItemProvider::GetTextValue(i)` (see also `.github/KnowledgeBase/manual/gacui/components/controls/list/itemprovider.md`).

## Expected listing behavior (for stable assertions)

- The dialog view model fills the file list in `FileDialogViewModel::RefreshFiles()` by appending folders first and then files, preserving the order returned by `filesystem::Folder::GetFolders()` and `filesystem::Folder::GetFiles()` (`Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`).
- `FileSystemMock::GetFolders()` / `GetFiles()` enumerate `FileItemMock::children` in key order (a `collections::Dictionary<WString, ...>`), and split by `isFile` (`Test/GacUISrc/UnitTest/TestControls.h`). This means the list is deterministic and can be asserted as:
	- all folders in alphabetical order, followed by all files in alphabetical order.
- Filter visibility in the dialog UI is applied at the data grid layer via `BindableDataGrid.AdditionalFilter` calling `IFileDialogFilter::FilterFile` (`Source/Utilities/FakeServices/Dialogs/FileDialog.xml`):
	- folder rows are always visible (not filtered),
	- file rows are filtered by the selected filter,
	- the ordering remains “folders first, then (remaining) files”.

## Proposed `TEST_CATEGORY` structure

- Add/extend a dedicated category for navigation & listing focused on the dialog UI contract:
	- **Root listing**: open the dialog and read all `itemProvider->GetTextValue(i)`; assert the exact sequence equals “alphabetical folders, then alphabetical files” from `CreateFileItemRoot()`.
	- **Root filter listing**: choose `Text Files (*.txt)` and read all `itemProvider->GetTextValue(i)`; assert the exact sequence equals “alphabetical folders, then alphabetical `*.txt` files”.
	- **Navigate into folder**: double-click `A`, wait for listing refresh, and assert the grid text sequence equals `A`’s “alphabetical folders, then alphabetical files”; then repeat the filter assertion inside `A`.
	- **Navigate deeper**: double-click `AA`, then assert the grid text sequence equals `AA`’s expected content (e.g. `deep.bin`).
	- **Cancel**: press `Cancel` and assert `GuiOpenFileDialog::GetFileNames().Count() == 0`.

## Handling async refresh in UI-driven tests

- Navigation triggers an async load cycle in the view model (placeholder row + `InvokeAsync` + main-thread update). Tests should place listing assertions only after the grid has refreshed.
- Practical approach in `UnitTestRemoteProtocol` terms:
	- Insert an extra `OnNextIdleFrame(...)` stage after any navigation (or repeatedly schedule idle frames) before reading `itemProvider->Count()` / `GetTextValue(i)`.
	- Optionally treat any “loading placeholder” row (if present) as “not ready yet” and keep waiting until the expected count/items appear.

# !!!FINISHED!!!
