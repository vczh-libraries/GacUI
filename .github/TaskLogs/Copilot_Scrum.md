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

# TASKS

- [ ] TASK No.1: Add FileItemMock + FileSystemMock (IFileSystemImpl)
- [ ] TASK No.2: Make FileDialogViewModel testable (options + message box + factory) and add empty test file
- [ ] TASK No.3: TEST_CATEGORY for folder navigation + file listing
- [ ] TASK No.4: TEST_CATEGORY for filter visibility + default extension behavior
- [ ] TASK No.5: TEST_CATEGORY for ParseDisplayString + multi-selection normalization
- [ ] TASK No.6: TEST_CATEGORY for TryConfirm message box interactions
- [ ] TASK No.7: TDD for TryConfirm existence checks with auto-appended extension

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

## TASK No.2: Make FileDialogViewModel testable (options + message box + factory) and add empty test file

Refactor the file dialog view model to enable unit tests to control message box outcomes and to observe confirmation results via `IFileDialogViewModel`, without exposing `FileDialogViewModel`’s concrete type in headers.

### what to be done

- Introduce `IFileSystemViewModelMessageBox` with a `ShowMessageBox` signature identical to the current usage inside `TryConfirm`.
	- Store `Ptr<IFileSystemViewModelMessageBox>` in `FileSystemViewModel` (or the component that currently calls message boxes during confirmation).
	- Ensure `TryConfirm` routes its single message-box decision through this interface (and only calls it once).
- Introduce `FileSystemViewModelOptions` containing (at least) the fields specified in the request, plus the message box implementation pointer.
	- Update `FileDialogViewModel` to store the options object instead of duplicating these fields as separate members.
	- Fix all resulting build breaks in the places that construct/configure the view model.
- Update fake services to wire a real message-box implementation in production/fake-dialog contexts:
	- `Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`
	- `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`
- Add test-only hooks to `IFileDialogViewModel` and update reflection:
	- `bool IsConfirmed();`
	- `const ...& GetConfirmedSelection();`
	- `void ResetConfirmation();`
- Add a cpp-only factory function (defined in the implementation `.cpp`) that returns `Ptr<IFileDialogViewModel>` to create a `FileDialogViewModel` instance for tests.
	- Keep the concrete `FileDialogViewModel` type hidden from headers; unit tests only see `IFileDialogViewModel`.
- Create an empty `TestFileDialogViewModel.cpp` placed alongside `Test/GacUISrc/UnitTest/TestDocumentConfig.cpp` and in the same solution explorer folder.
	- Add it to the correct project file (`*.vcxproj` / `*.vcxitems`) and its `*.filters` entry so it builds (even if it contains no tests yet).

### rationale

- `TryConfirm` behavior depends on user prompts; injecting a message-box interface is the minimal way to make these branches unit-testable without driving UI.
- Consolidating options into `FileSystemViewModelOptions` prevents the test factory from needing to reach into a growing list of constructor parameters or ad-hoc setters, and reduces future churn.
- Exposing confirmation state and selection through the interface is necessary because the tests exercise `IFileDialogViewModel` directly.
- Creating the test `.cpp` early ensures the plumbing (project inclusion, linkage for the factory function) is correct before investing in multiple test categories.

## TASK No.3: TEST_CATEGORY for folder navigation + file listing

Add a focused test category validating that `SetSelectedFolder` drives file listing correctly (root by default, and switching folders updates `GetFiles()`), using `FileSystemMock` to control the tree.

### what to be done

- In `Test/GacUISrc/UnitTest/TestFileDialogViewModel.cpp`, add a `TEST_CATEGORY` dedicated to folder navigation / listing.
- Create a representative `FileItemMock` tree including at least:
	- Root files and folders.
	- Nested folders with both files and subfolders.
	- Names that include extension and no extension (to be reused by later tasks).
- Create the view model via the cpp-only factory, passing a `FileSystemViewModelOptions` instance and the `FileSystemMock` root.
- Validate expected behavior:
	- Default selected folder is root (path `""`) and `SetSelectedFolder` updates `GetSelectedFolder()`.
	- After `SetSelectedFolder`, loop on `GetIsLoadingFiles()` until it completes, then assert:
		- `GetRootFolder()` and `GetSelectedFolder()` are consistent.
		- `GetFiles()` reflects children of the selected folder with correct file/folder distinction and full path formatting.

### rationale

- Navigation and listing are the foundation for all later selection and confirmation tests; separating them keeps failure diagnosis straightforward.

## TASK No.4: TEST_CATEGORY for filter visibility + default extension behavior

Add tests validating `SetSelectedFilter` affects file visibility and determines the “expected extension” used by selection normalization.

### what to be done

- Implement simple `IFileDialogFilter` instances in the test file:
	- A filter that accepts all files (no default extension name).
	- A filter that accepts only `*.txt` (default extension name is `txt`).
- Before calling `SetSelectedFilter`, add all filters to the collection returned by `GetFilters()`.
- In the test category:
	- Select different folders and verify `GetFiles()` changes when the filter changes (files outside filter become invisible).
	- Validate `GetSelectedFilter()` reflects the current filter selection.
	- Cover extension-appending prerequisites:
		- When a filter has an extension name, it must be the expected extension.
		- When filter has no extension name, `defaultExtension` from options must become the expected extension (if non-empty).

### rationale

- Filtering is core dialog behavior and is tightly coupled to how selection strings are interpreted; establishing these invariants enables later selection/confirmation tests to be precise.

## TASK No.5: TEST_CATEGORY for ParseDisplayString + multi-selection normalization

Add tests around `ParseDisplayString` behavior for one or multiple selections, including extension appending and full-path normalization.

### what to be done

- In a dedicated `TEST_CATEGORY`, create scenarios that call `ParseDisplayString` with:
	- A single filename (relative to selected folder).
	- Multiple filenames separated by `;`.
	- Inputs containing both `/` and `\\` to ensure path normalization rules are applied consistently.
	- Filenames with and without extensions, under both:
		- Selected filter with extension name.
		- Selected filter without extension name but options `defaultExtension` is non-empty.
- After `ParseDisplayString`, call `TryConfirm` and assert via `IsConfirmed()` / `GetConfirmedSelection()` that:
	- Selected items have the expected extension appended (when required).
	- Confirmed selection contains full paths and matches folder + filename resolution rules.
- Use `ResetConfirmation()` between subcases to avoid cross-case contamination.

### rationale

- `ParseDisplayString` is the primary entry point for typed selections and multi-select strings; keeping these cases in a dedicated category ensures coverage without mixing in message-box driven behaviors.

## TASK No.6: TEST_CATEGORY for TryConfirm message box interactions

Add tests validating `TryConfirm` branches that depend on prompt/result interactions, using a test fake for `IFileSystemViewModelMessageBox` that can preset the return value and record whether it was called.

### what to be done

- Implement a minimal test fake `IFileSystemViewModelMessageBox`:
	- Configurable “next result” return value.
	- A flag/counter recording whether `ShowMessageBox` has been called.
- Create multiple subcases, each configuring `FileSystemViewModelOptions` and filesystem state, then calling `ParseDisplayString` + `TryConfirm`:
	- `fileMustExist` / `folderMustExist` failure path should trigger message box exactly once and return false (or follow the returned choice if applicable).
	- `promptCreateFile` behavior when file does not exist.
	- `promptOverriteFile` behavior when file exists.
	- Confirm that when a message box is not needed, the fake is not called.
- Assert that the view model’s confirmation and selection state matches each scenario and can be reset using `ResetConfirmation()`.

### rationale

- The message-box driven branches are otherwise untestable and the largest source of combinatorial behavior; isolating them in a single category ensures comprehensive coverage while keeping the rest of the tests free from UI branching logic.

## TASK No.7: TDD for TryConfirm existence checks with auto-appended extension

Add a focused regression unit test proving `TryConfirm` checks existence using the final normalized filename (after extension auto-append), and then fix `FileSystemViewModel::TryConfirm` so that any required extension appending (from selected filter or from `FileSystemViewModelOptions::defaultExtension`) happens before checking `filesystem::File(path).Exists()` / `filesystem::Folder(path).Exists()` and before deciding `files` / `folders` / `unexistings`.

This prevents incorrect behavior such as treating `abc` as an existing file, then returning a confirmed selection `abc.txt` that was never checked for existence.

### what to be done

- First, in `Test/GacUISrc/UnitTest/TestFileDialogViewModel.cpp`, add a dedicated `TEST_CATEGORY` with a small mocked filesystem containing both:
	- A file `abc.txt` (but no `abc`), and optionally a folder `abc` for edge-case priority validation.
- Configure the view model so that extension appending is required (either by selecting a filter whose default extension is `txt`, or by leaving filter extension empty but setting `FileSystemViewModelOptions::defaultExtension` to `txt`).
- Add at least these subcases:
	- **Existence is checked on appended name**: with `fileMustExist = true`, input `abc` should succeed (no message box) and confirm `abc.txt` when `abc.txt` exists.
	- **Raw-name existence must not mask missing appended name**: with `fileMustExist = true`, if `abc` exists but `abc.txt` does not, `TryConfirm` must fail (and trigger the appropriate message box path), and must not confirm `abc.txt`.
- Assert `IsConfirmed()` / `GetConfirmedSelection()` and message box fake call count to ensure the branch decisions are driven by the normalized target name.
- Then, in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, adjust `FileSystemViewModel::TryConfirm` so that:
	- It still keeps the existing "single selection navigates into folder" behavior using the raw (non-appended) path.
	- For file confirmation, it computes the expected extension (from `selectedFilter->GetDefaultExtension()` or options `defaultExtension`) and normalizes each candidate file path accordingly before existence checks.
	- It uses the normalized paths for:
		- Deciding membership in `files` / `unexistings`.
		- Prompting (create/overwrite) messages (the relative paths shown should match the normalized target files).
		- Populating `confirmedSelection` (so no late post-processing is needed to append extension after prompts).
	- It does not append an extension to paths that resolve to folders (folder navigation and folder error reporting should remain folder-oriented).

### rationale

- Writing the failing regression test first locks in the intended behavior and prevents future refactors from reintroducing the "check-before-append" ordering.
- `TryConfirm` must validate and prompt against the actual final target paths; doing extension normalization after existence checks breaks correctness for `fileMustExist`, `promptCreateFile`, and `promptOverriteFile` branches.

# Impact to the Knowledge Base

## GacUI

- Add a knowledge base note describing recommended patterns for unit-testing dialog/view-model logic:
	- Implementing a deterministic `IFileSystemImpl` mock using a tree model (root `""`, normalized `/` paths).
	- Injecting UI prompts (message boxes) through a small interface to make view-model branching testable.
	- Using an options struct to avoid brittle constructors and to keep test factories stable.
	- Ensuring selection normalization (e.g. default extension appending) happens before existence checks in confirm logic (to keep `fileMustExist` and prompt behaviors correct).
