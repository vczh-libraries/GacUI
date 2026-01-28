# !!!SCRUM!!!

# DESIGN REQUEST

# Problem
create test cases for `GuiFileDialog`

Task: implement `IFileSystemImpl`. In order to predict what a file dialog would display, we need to fake a file system. It is done by implementing `IFileSystemImpl`.

In [TestControls.h](Test/GacUISrc/UnitTest/TestControls.h) you need to create a simple data structure:
```
class FileItemMock : public Object
{
public:
  bool isFile = false;
  Dictionary<WString, Ptr<FileItemMock>> children;
};
```

You can fake the whole file system by making a `FileItemMock` tree. The root node will be a root. Its path is "". A valid path for non-root item would look like /ABC/DEF/GHI... You need to treat "/" and "\" the same.

Following `TooltipTimer` to create a `FileSystemMock` implementing `IFileSystemImpl, its constructor takes a `FileItemMock` as root. Because this is a read only file system, so the following functions should just `CHECK_FAIL(L"Not Implemented!");`:
- FileDelete
- FileRename
- CreateFolder
- DeleteFolder
- FolderRename
- GetFileSystemImpl

The Initialize function normalizes the path to use "/" and delete the finaly "/" in the path if exists. WString is a readonly string class.

Multiple tasks: Design test cases for `IFileDialogViewModel`. It is implemented by `FileDialogViewModel`. But this is not visible in the header file so you need to add a function in the cpp file just to create it, returning `Ptr<IFileDialogViewModel>`.

You need to create a TestFileDialogViewModel.cpp, put it in the same folder and same solution explorer folder as [TestDocumentConfig.cpp](Test/GacUISrc/UnitTest/TestDocumentConfig.cpp) . extern the function you just added in this file and you can continue to create test cases.

And then you are going to test `IFileDialogViewModel` directly, using the FileSystemMock created in the previous task. In this interface, `SetSelectedFolder` display files in the folder, the default one should be root. `SetSelectedFilter` make some files invisible. You need to implement `IFileDialogFilter` by yourself, but keep it simple, for example, making one filter let all files go (therefore no default extension name) and another just let *.txt go (therefore the default extension name is "txt"). You need to add all filters to `GetFilters()` before calling `SetSelectedFilter`.

After calling these two functions, you can loop GetIsLoadingFiles and when it turns true, test GetRootFolder/GetSelectedFilter/GetSelectedFolder/GetFiles, for file navigation

For file selection, whenever you want to choose one file or multiple file, call `ParseDisplayString` and then `TryConfirm`.

The parameter to `ParseDisplayString` is one or multiple files separated by ";". When the selected filter has an extension name, or when it has not but defaultExtension of the view model is not empty, any selection without the expected extension name will get it appended.

When `TryConfirm` returns true, `confirmedSelection` will stores selected files with its full path fixed according to extension. This is not accessible via the interface so you have to add:
bool IsConfirmed();
const...& GetConfirmedSelection();
void ResetConfirmation(){confirmed=false; confirmedSelection.Clear();}

Remember to update the reflection for the interface

Since there are a lot of bool options controlling how TryConfirm will call and react to message boxes, this is not testable so you must make it testable.

Add an interface says `IFileSystemViewModelMessageBox` with an identical ShowMessageBox function, store an Ptr<> to `FileSystemViewModel`. In `FakeDialogServiceBase` it will set an implementation that actually calls the dialog service, but in the unit test you can make a fake one to control the return value. You can preset a result and record if it has been called, as `TryConfirm` will only call `ShowMessageBox` once. So testing if it has been called, and test how it react to the return value is an important way to see if `TryConfirm` reacts correctly with those bool options.

That says when you expose a function creating `FileSystemViewMode`, you'd better have a struct calls `FileSystemViewModelOptions` covers the message box mock and all these:
```
			WString						title;
			bool						enabledMultipleSelection = false;
			bool						fileMustExist = false;
			bool						folderMustExist = false;
			bool						promptCreateFile = false;
			bool						promptOverriteFile = false;
			WString						defaultExtension;
```
and you can stores the options directly to `FileDialogViewModel` instead of storing these options directly. This will cause multiple build breaks and you need to fix it.

So the next task is to make any necessary mentioned change to [GuiFakeDialogServiceBase.h](Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h)  and [GuiFakeDialogServiceBase_FileDialog.cpp](Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp) with an empty test file `TestFileDialogViewModel.cpp`, following by multiple tasks each create a test category to test the behavior of `FileSystemViewModel` using the `FileSystemMock` to control the content of the it.

# UPDATES

## UPDATE

there is a bug in `FileSystemViewModel::TryConfirm`, when the extension to automatically append is txt, and user enter abc, abc will be used to test if the file exist and then fix the file name to be abc.txt, this is clearly wrong. You need to cover this case and also fix the bug.

## UPDATE

you can merge the last two tasks together, complete the unit test first and then fix the function.

## UPDATE

The idea of testing against FileDialogViewModel is totally not working, because it requires resources from GacUI environment. So I would like you to:

1. Remove Task No.2 as it is completely not necessary now
2. Replan No.3 to No.7. Previously they are testing against navigation and filtering and anything else, now you need to actually open a file dialog.

I have prepared you some sample test in [TestApplication_Dialog_File.cpp](Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp) . It already has some helper function teaching you how to actually access controls in the opened file dialog.

The definition of the file dialog is in [FileDialog.xml](Source/Utilities/FakeServices/Dialogs/FileDialog.xml) 
meanwhile all localized text that working in the unit test can be found in [Resource.xml](Source/Utilities/FakeServices/Dialogs/Resource.xml) 
For example, when the dialog is opened, you need to find the datagrid:

firstly
```
		auto window = From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
			.First();
```
The title of the file dialog is defined here
```
<OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
```

Secondly, there is a `filePickerControl` in the window (via ref.Name="filePickerControl"), inside it there is a dataGrid, that is a <BindableDataGrid>. So this is how
```
auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"filePickerControl", L"dataGrid");
```
works.

Now you know how to iterate items in the datagrid by text according to `ClickFileInternal`, and now how to operate the data grid using helper functions in [TestControls_List.h](Test/GacUISrc/UnitTest/TestControls_List.h) 

Please reuse the test case design in the scrum document but rework them to work on the file dialog.

## UPDATE

it looks like Task No.2 already covers some filtering, please examine Task 3 carefully, I guess we can remove some tests in Task 3 which are already covered in Test 2, and merge the rest with Task 6 (therefore there will be no Task6 after merging). I believe Task 6 is also about extension behavior, so merging it into 3 is reasonable.

## UPDATE

some hint, when a file dialog opens a message dialog, this is the way to get the object of both: before opening a message dialog, just like what it was done in the test ``` auto dialogWindow = From(GetApplication()->GetWindows()) 	.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; }) 	.First(); ``` after ``` auto dialogWindow = From(GetApplication()->GetWindows()) 	.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; && w->GetNativeWindow()->GetParent() == nullptr; })  	.First(); ``` Both file dialog and message dialog have the same title, the one with null parent will be file dialog, otherwise will be message dialog. Refer to [TestApplication_Dialog_Message.cpp](Test/GacUISrc/UnitTest/TestApplication_Dialog_Message.cpp)  about how to react to a message dialog

so I would like you to add a small job to task 3, add GetOpeningFileDialog and GetOpeningMessageDialog, and clean up the current file to use the two functions. By the way, no matter the message dialog is opened or not, file dialog will always have a null native parent, so you can always add that parent check.

## UPDATE

backgrouhd: although GuiOpenFileDialog and GuiSaveFileDialog share the same implementation, but some options or flags is only valid on GuiSaveFileDialog, so the last task is going to add a save dialog anyway. But in order not to disturb other test cases' log files, I would like you to insert a new task before the current Task No.5, to add that save dialog and the button. So the current Task No.5 (which will become Task No.6) do not have to change the UI

## UPDATE

add a new task before No.6. The finished No.5 made two duumy test cases for save dialog. You need to change the first one to type an unexisting file name and click OK, another one (with overriding prompt enabled) to click an existing file, click OK and solve the prompt.

# TASKS

- [x] TASK No.1: Add FileItemMock + FileSystemMock (IFileSystemImpl)
- [x] TASK No.2: TEST_CATEGORY for file dialog open + folder navigation + listing
- [x] TASK No.3: TEST_CATEGORY for filter visibility + extension behavior (in dialog)
- [x] TASK No.4: TEST_CATEGORY for typed selection + multiple selection (in dialog)
- [x] TASK No.5: Add save dialog + open buttons (in test UI)
- [ ] TASK No.6: Update save dialog test cases (replace placeholders)
- [ ] TASK No.7: TEST_CATEGORY for dialog message box interactions (open/save options)

## TASK No.1: Add FileItemMock + FileSystemMock (IFileSystemImpl)

Add an in-memory read-only file-system representation for unit tests, to deterministically control what `GuiFileDialog` / `FileSystemViewModel` would “see” during directory enumeration and existence checks.

### what to be done

- In `Test/GacUISrc/UnitTest/TestControls.h`, add the `FileItemMock` data structure exactly as requested.
- Following the style of `TooltipTimer` (test-only helper types), add a `FileSystemMock` implementing `IFileSystemImpl`.
	- Constructor takes `Ptr<FileItemMock>` as the root.
	- Root path is `""`; non-root item paths are `/ABC/DEF/...`.
	- Treat `/` and `\\` as equivalent in all path inputs.
	- Implement `Initialize` to normalize paths to use `/` and remove trailing `/` if exists.
	- For write/mutation APIs (read-only mock), implementation must be `CHECK_FAIL(L"Not Implemented!");`:
		- `FileDelete`, `FileRename`, `CreateFolder`, `DeleteFolder`, `FolderRename`, `GetFileSystemImpl`.
- Provide the minimal read-only operations needed by `FileSystemViewModel` and/or dialog view models to:
	- Resolve a path to a `FileItemMock` node.
	- Enumerate children (files/folders) in a folder.
	- Query file/folder existence and file/folder distinction.

### rationale

- The file dialog UI is driven by filesystem queries; a deterministic in-memory implementation is the simplest way to write stable unit tests without depending on the host machine filesystem.
- Making the mock explicitly read-only keeps scope tight and prevents accidental reliance on mutation behaviors that are not needed by the dialog view model tests.

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

## TASK No.3: TEST_CATEGORY for filter visibility + extension behavior (in dialog)

Add dialog-driven tests validating that changing the filter updates the listing, and that the selected filter affects typed selection / extension normalization.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` dedicated to filter behaviors.
- Add helper functions for selecting the correct opened dialog window when both file dialog and message dialog share the same title:
	- `GetOpeningFileDialog`: find `GuiWindow::GetText() == L"FileDialog"`, and treat `w->GetNativeWindow() == nullptr` as a candidate; when native window exists, accept `GetParent() == nullptr` **or** equals the main window native.
	- `GetOpeningMessageDialog`: prefer selecting a window whose native parent is the file dialog native window; do not rely on the message dialog title, and always exclude the file dialog window itself.
	- Refactor existing helper functions (e.g. `PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter`) to use `GetOpeningFileDialog` instead of duplicating `From(GetApplication()->GetWindows())...`.
- Reuse the test dialog’s `Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt"` and drive the UI (avoid re-testing basic folder navigation and listing already covered by TASK No.2; only assert deltas caused by filter/extension logic):
	- Switch filters via `ChooseFilter(protocol, filterIndex)`.
	- Add a minimal visibility assertion (e.g. in `/A`, `a.txt` remains visible in `Text Files`, while a known non-`.txt` entry becomes invisible).
- Add typed-selection subcases that demonstrate the chosen filter’s expected extension behavior:
	- Navigate into `/A`, choose `Text Files (*.txt)`, type `L"a"` and click `Open`; assert the returned filename is `/A/a.txt` (extension auto-append).
	- Choose `All Files (*.*)`, type `L"README"` at root and click `Open`; assert the returned filename is `/README` (no auto-append under the all-files filter).
	- Regression: choose `Text Files (*.txt)`, type `L"README"` at root and click `Open`; expected behavior is the dialog stays open and shows the “File(s) not exist” message because the real target is `README.txt` (auto-append happens before existence checks). Use `GetOpeningMessageDialog` to locate the message dialog and close it (following patterns in `Test/GacUISrc/UnitTest/TestApplication_Dialog_Message.cpp`). If the test fails, fix `FileSystemViewModel::TryConfirm` in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` so that extension auto-append happens before existence/prompt checks, while folder navigation still uses the raw, non-appended name when the selection resolves to a folder.
	- Ensure the dialog option triggers the intended prompt (e.g. `Options="FileDialogFileMustExist"` for open dialogs), otherwise the “fail” inputs won’t produce a message dialog.

### rationale

- Filter correctness is core dialog behavior and is tightly coupled to selection parsing and extension normalization; validating it through the UI ensures `FileDialog.xml` bindings and localized text initialization are also exercised.
- Extension auto-append influences both the returned selection and validation logic (existence checks, prompts); the regression case ensures these stay consistent under a restrictive filter.

## TASK No.4: TEST_CATEGORY for typed selection + multiple selection (in dialog)

Add dialog-driven tests that focus on the selection string in the file name text box, including multiple selection behavior when enabled.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` for typed selection and multi-selection.
- For typed selection:
	- Navigate into a folder, set a specific filter, call `TypeFile(protocol, ...)`, and click `Open` / `Save`.
	- Assert dialog output via `GuiOpenFileDialog::GetFileNames()` (or `GuiSaveFileDialog::GetFileName()` when using save dialog).
	- Keep “type” and “confirm” in separate idle frames when needed (so each frame produces UI updates and remains non-blocking).
- For multiple selection:
	- Add a second dialog instance (or a second button) that enables multiple selection by setting `GuiFileDialogBase::Options` to include `INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection`.
	- Add a case that selects multiple files in the data grid using helpers from `Test/GacUISrc/UnitTest/TestControls_List.h` (e.g. ctrl-click) and then presses `Open`.
	- Assert that `GetFileNames()` returns all expected full paths.

### rationale

- The file dialog’s user-facing contract is the selection string plus final selected file list; testing it through the actual dialog ensures `FilePickerControl`’s `Selection` logic (based on data grid selection and text box input) is covered.
- Selection outcomes depend on the active filter, which persists across navigation; tests should explicitly set the filter before selecting or typing file names.

## TASK No.5: Add save dialog + open buttons (in test UI)

Add a `GuiSaveFileDialog` entry point to the unit-test UI so save-only options (prompt create / prompt overwrite) can be exercised without changing the UI in later tasks.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, extend the `resourceFileDialogs` test resource:
	- Add a `<SaveFileDialog ref.Name="dialogSave" .../>` alongside the existing `<OpenFileDialog ref.Name="dialogOpen" .../>` (reuse the same `Title`, `Filter`, and `Directory` defaults so existing helper functions keep working).
	- Add button(s) that open the save dialog with the options needed by later message-box tests, using the same `Clicked` handler pattern as the existing open buttons (set `dialogSave.Options`, call `dialogSave.ShowDialog()`, and write the joined result to `self.Text`):
		- A button that enables `INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile`.
		- A button that enables `INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile`.
- Keep existing open-dialog buttons unchanged (text and behavior) so previously completed navigation / selection tests remain stable.

### rationale

- Some `INativeDialogService::FileDialogOptions` are meaningful only for `GuiSaveFileDialog`; introducing it as an explicit, isolated task avoids mixing UI wiring changes with message-box behavior assertions.
- Keeping UI changes out of the message-box test task reduces churn in later task logs and keeps failures attributable to logic rather than resource changes.

## TASK No.6: Update save dialog test cases (replace placeholders)

Replace the placeholder save-dialog tests introduced during TASK No.5 with meaningful coverage that matches intended user interactions.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, locate the two placeholder save-dialog test cases introduced during TASK No.5 and update them:
	- **Type an unexisting file name**: type a non-existing file name in the file-name text box and click `OK`.
	- **Overwrite prompt (click existing file)**: enable overwrite prompting, click an existing file in the file list, click `OK`, and solve the overwrite prompt so the dialog closes with that selection.
- Keep this task focused on making the save-dialog test cases realistic; comprehensive message-box interaction coverage stays in the next task.

### rationale

- The placeholder save-dialog tests from TASK No.5 are not aligned with the intended interaction sequences (typed new name vs. clicking an existing file and handling overwrite prompts).
- A dedicated adjustment task keeps later message-box interaction tests focused on prompt logic instead of compensating for earlier placeholder behaviors.

## TASK No.7: TEST_CATEGORY for dialog message box interactions (open/save options)

Add dialog-driven tests for error/prompt message boxes triggered by `TryConfirm`, by causing invalid selections and answering prompts in the UI.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` for message box interactions.
- When adding proxy frames (`UnitTestRemoteProtocol::OnNextIdleFrame`), name each frame by what the previous frame has done, and ensure each frame makes a visible UI change (split “type”, “click confirm”, “close message box”, “assert output” into separate frames when needed).
- Keep assertions deterministic (avoid `A || B`); when asserting selection display strings, control the selection order and assert the exact expected string.
- When reading the file name text box (`filePickerControl.textBox`), expect `FileDialogViewModel::GetDisplayString(...)` to always quote file names; multi-selection display strings are formatted like `"a";"b"` (and single selection like `"a"`).
- Reuse localized button texts from `Source/Utilities/FakeServices/Dialogs/Resource.xml` when finding buttons to click (`OK`, `Cancel`, `Yes`, `No`), so tests remain aligned with the dialog resources.
- Locate and close message dialogs by native-parent relationships (`GetOpeningMessageDialog`) instead of relying on titles or sending `Enter`; closing should be a real UI click (so the test harness observes a UI update).
- Cover at least these scenarios (reusing the existing open/save buttons from `resourceFileDialogs` to open dialogs with the intended options):
	- **File must exist (open dialog)**: click the open button that enables `FileDialogFileMustExist`, type a non-existing file name and press `Open`; assert a message box appears and that clicking `OK` dismisses it while keeping the file dialog open.
	- **Prompt create file (save dialog)**: click the save button that enables `FileDialogPromptCreateFile`, type a non-existing file name and press `Save`; assert the create prompt appears and validate both `Yes` (confirm selection) and `No` (stay in dialog) flows.
	- **Prompt overwrite file (save dialog)**: click the save button that enables `FileDialogPromptOverwriteFile`, type an existing file name and press `Save`; assert the overwrite prompt and validate both `Yes`/`No` flows.
	- **Multi-selection not enabled**: open a dialog without `FileDialogAllowMultipleSelection`, attempt to confirm multiple selections and assert the “Multiple selection is not enabled” message box and dismissal behavior.

### rationale

- These behaviors are user-visible and driven by localized dialog resources; asserting them through the UI is the most reliable way to ensure the wiring between `FileDialog.xml`, `Resource.xml`, and `TryConfirm` stays correct.

# Impact to the Knowledge Base

## GacUI

- Add a knowledge base note describing recommended patterns for unit-testing fake dialog UI behavior:
	- Implementing a deterministic `IFileSystemImpl` mock using a tree model (root `""`, normalized `/` paths) so dialogs enumerate predictable content.
	- Using `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>` + `UnitTestRemoteProtocol` to drive real dialogs instead of testing `FileDialogViewModel` directly (resource/template dependent).
	- Finding dialog windows via helper functions (e.g. `GetOpeningFileDialog` / `GetOpeningMessageDialog`) that consider native parenting (file dialog may be parented to the main window; message dialogs are parented to the file dialog), and accessing dialog internals by `ref.Name` via `FindObjectByName(...)` (e.g. `filePickerControl`, `dataGrid`, `textBox`, `comboBox`).
	- Driving list controls using helpers from `Test/GacUISrc/UnitTest/TestControls_List.h` instead of re-implementing mouse/keyboard logic.
	- Ensuring selection normalization (e.g. default extension appending) happens before existence checks in confirm logic (so file-must-exist and prompt behaviors remain correct).

# !!!FINISHED!!!
