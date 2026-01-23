# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.1: Add FileItemMock + FileSystemMock (IFileSystemImpl)

Add an in-memory read-only file-system representation for unit tests, to deterministically control what `GuiFileDialog` / `FileSystemViewModel` would â€œseeâ€ during directory enumeration and existence checks.

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

# UPDATES

## UPDATE

After calling Initialize, all fullpath passed to other function to this interface will be alrealdy normalized, so you can do it simpler in other functions.

You can also use vl::regex::Regex::Split to help you find out all path components, remember in this regex implementation, "/" and "\" both escape other characters, and "." means dot meanwhile "\." or "/." means anything. You can escape characters using "/" as this would be simpler in C++ string literals. Just pass false to keepEmptyMatch and it would automatically delete zero-size path components, which automatically implements treating "//" as "/". After getting all components, just join the using "/" (and remember the path should begins with "/" if it is not the root) 

# INSIGHTS AND REASONING

## Existing Architecture Evidence

- `vl::filesystem::FilePath`, `vl::filesystem::File`, and `vl::filesystem::Folder` delegate all filesystem behaviors to an injected `vl::filesystem::IFileSystemImpl` (see `Import/VlppOS.h` for the interface and `Import/VlppOS.cpp` for `InjectFileSystemImpl`/`EjectFileSystemImpl` and the delegation in `FilePath::Initialize`, `File::Delete`, `Folder::GetFiles`, etc.).
- `vl::filesystem::FilePath::Initialize()` calls `vl::filesystem::IFileSystemImpl::Initialize(fullPath)` and the other `FilePath` delegations pass the already-initialized `fullPath` directly (see `Import/VlppOS.cpp`), so `IFileSystemImpl::IsFile/IsFolder/IsRoot/GetRelativePathFor` can assume normalized `fullPath` when invoked through `vl::filesystem::FilePath`.
- `vl::regex` uses a non-standard escaping rule where both `\\` and `/` escape the next character, and `.` means a literal dot while `\\.` means “any character” (see the grammar comment in `Import/VlppRegex.h`), which affects how a “split on / or \\” pattern should be written.
- `vl::presentation::FileDialogViewModel` (in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`) exercises the exact APIs the mock must satisfy:
	- Background folder enumeration: `filesystem::Folder(path).GetFolders(...)` and `filesystem::Folder(path).GetFiles(...)` in `RefreshFiles()`.
	- Existence checks: `filesystem::File(path).Exists()` and `filesystem::Folder(path).Exists()` in `TryConfirm(...)`.
	- Error/report formatting: `wd.GetRelativePathFor(paths[index])` in `TryConfirm(...)`.
- The unit-test helper `gacui_unittest_template::TooltipTimer` (in `Test/GacUISrc/UnitTest/TestControls.h`) shows the preferred “test-only helper type + feature injection” pattern: derive from `vl::feature_injection::FeatureImpl<...>`, inject in the constructor, and eject in the destructor.

## Proposed High-Level Design

### Data Model (`FileItemMock`)

- Add `FileItemMock` in `Test/GacUISrc/UnitTest/TestControls.h` (namespace `gacui_unittest_template`) exactly as specified:
	- `bool isFile = false;`
	- `vl::collections::Dictionary<vl::WString, vl::Ptr<FileItemMock>> children;`
- Interpretation:
	- A node with `isFile == false` is a folder (including the root).
	- A node with `isFile == true` is a file and must not be enumerated as a folder.
	- `children` keys are per-segment names (no separators).

### Filesystem Adapter (`FileSystemMock`)

- Implement `FileSystemMock` as a test-only injected filesystem backend:
	- Class location: `Test/GacUISrc/UnitTest/TestControls.h` (next to `TooltipTimer`).
	- Namespace: `gacui_unittest_template`.
	- Inheritance: `vl::feature_injection::FeatureImpl<vl::filesystem::IFileSystemImpl>` (matching the `TooltipTimer` pattern).
	- Construction:
		- Takes `vl::Ptr<FileItemMock>` root (stored).
		- Calls `vl::filesystem::InjectFileSystemImpl(this)` so all `vl::filesystem::*` calls in the view model route to the mock.
	- Destruction:
		- Calls `vl::filesystem::EjectFileSystemImpl(nullptr)` to restore the previous filesystem implementation (matching the existing “eject all” pattern used by `TooltipTimer`).

### Path Normalization Rules

- `Initialize(vl::WString& fullPath)` is the single source of truth for normalization because it is automatically applied by `vl::filesystem::FilePath::Initialize()` (`Import/VlppOS.cpp`), and all other `IFileSystemImpl` methods receive `fullPath` values originating from `FilePath`.
- Normalization algorithm (separator-agnostic, no re-normalization needed elsewhere):
	- Use `vl::regex::Regex::Split` to split `fullPath` into path components using a regex that matches one-or-more path separators (`/` and `\\`), and pass `keepEmptyMatch = false` so empty components are dropped automatically (therefore `//` behaves like `/`, and leading/trailing separators produce no empty segments).
	- Rebuild `fullPath` by joining components with `/`:
		- If there are no components, normalized `fullPath` is the root representation `""`.
		- Otherwise, normalized `fullPath` begins with `/` and contains no trailing `/` (e.g. `/ABC/DEF`).
- Notes for writing the split regex:
	- Because `vl::regex` treats both `\\` and `/` as escaping characters, the pattern must be written carefully (prefer `/`-escaping in the regex text to reduce C++ string literal escaping).

### Resolving a Path to a Node

- Add an internal helper in `FileSystemMock` (conceptually):
	- Assume `fullPath` is already normalized when invoked through `vl::filesystem::FilePath` (per the `FilePath::Initialize()` delegation).
	- Split `fullPath` into components using the same `vl::regex::Regex::Split(..., keepEmptyMatch = false, ...)` rule (for root `""`, the component list is empty).
	- Traverse `children` from the root node; if a segment is missing, resolution fails.
- This helper drives:
	- `IsFile(fullPath)`, `IsFolder(fullPath)`, and `IsRoot(fullPath)`.
	- Folder enumeration (`GetFolders`/`GetFiles`).

### Required `IFileSystemImpl` Behaviors

- `IsRoot(fullPath)`: true iff normalized path is `""`.
- `IsFile(fullPath)`: true iff resolution succeeds and the node is `isFile == true`.
- `IsFolder(fullPath)`: true iff resolution succeeds and the node is `isFile == false` (including root).
- `GetFolders(folderPath, folders)` / `GetFiles(folderPath, files)`:
	- Resolve `folderPath.GetFullPath()` to a folder node; return false if missing or points to a file.
	- Enumerate `children` and create `vl::filesystem::Folder` or `vl::filesystem::File` objects for each child based on `isFile`.
	- The returned items must have stable full paths consistent with the normalization rules so that:
		- `folder.GetFilePath().GetName()` matches the dictionary key (used by `FileDialogViewModel`).
		- subsequent `Exists()` checks and `GetRelativePathFor` calculations work.
- `GetRelativePathFor(fromPath, toPath)`:
	- This is primarily used for presenting a relative path inside `TryConfirm(...)` error messages.
	- Minimal expected behavior for these tests:
		- If `fromPath` is `""` (root), return `toPath` without a leading `/`.
		- If `toPath` is inside `fromPath`, return the remaining suffix without a leading `/`.
		- Otherwise, fall back to returning `toPath` as-is (tests should not rely on cross-tree `..` semantics).
- Read-only / unsupported operations:
	- For the mutation APIs listed in the task (`FileDelete`, `FileRename`, `CreateFolder`, `DeleteFolder`, `FolderRename`) and any other write-paths accidentally invoked by the view model under test, immediately `CHECK_FAIL(L"Not Implemented!");` to keep the test surface minimal and highlight unexpected dependencies.
	- `GetFileStreamImpl(...)` should also `CHECK_FAIL(L"Not Implemented!");` unless a later test task explicitly needs file content I/O.

## Why This Solves the Testability Problem

- The file dialog view model logic is written against `vl::filesystem::FilePath` / `File` / `Folder`, which are already abstracted behind `IFileSystemImpl` via feature injection (`Import/VlppOS.cpp`). Injecting a deterministic implementation lets unit tests control enumeration and existence checks without touching the host filesystem.
- Keeping the mock read-only and intentionally incomplete (fail-fast on mutations/streams) reduces risk of tests accidentally depending on behaviors not required by the dialog/view model, and makes new requirements visible when they appear.

# !!!FINISHED!!!
