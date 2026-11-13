# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

A task to bring new folder, delete folder and delete file to file dialog box.
It includes two file dialog for `GuiFakeDialogService`, one for GUI, another one for TUI.

In GUI's file dialog, above the tree view and list view there is a long label for viewing folder path.
You are going to add a tool bar which only has two buttons, put it at the very right after the label.
The two images are already added to `Source/Utilities/FakeServices/Dialogs/ToolbarIcons`.
One for adding a new folder in the folder displaying in the list view.
One for deleting selected items in the list view.

In TUI's file dialog, there is the same structure, but instead of using toolbar, use two buttons directly.
Their text will be "New Folder" and "Delete" in English, make sure they are localizable like other text.

Both buttons in GUI/TUI file dialog has the same feature,
so you can implement the feature in their view model implementation,
and you should use the file system classes from `VlppOS` so that the implementation is naturally cross platform.

When "New Folder" is clicked,
it shows a modal window asking for the folder name, implemented for both GUI/TUI:
A label for "Enter the folder name".
A text box below it.
Two buttons below it aligned to the right say "OK" and "Cancel".

When clicking OK, it tries to create a folder, when failed, print a message box to tell the user:
- The folder already exists.
- A general failure message.
As the file system class does not return the reason. So you should check folder name existing first and then create the folder.
Only when succeeded the window is closed, otherwise it stays open.

When clicking Cancel, it just closes.

When "Delete" is clicked,
it first ask if the user would like to delete all selected items in a OKCancel message box,
and then delete them when OK is clicked.
File system supports deleting them one by one, be awared that a selected folder might have content, everything will be deleted.
When any failure happens do not stop, just remember what can't be deleted and displayed in a message box.
When there is no failure, nothing needs to prompt.

Here are message box contents:

```plaintext
In folder "xxxx", "yyyy" folder cannot be created because it already exists.
```

```plaintext
In folder "xxxx", "yyyy" folder cannot be created.
```

```plaintext
Are you sure to delete the following items in folder "xxxx"?
abc
def
ghi
```

```plaintext
The following items failed to delete in folder "xxxx":
abc
def
ghi
```

These actions might affect content in tree view and list view, they need to be updated.
Both controls are data binded so changing containers directly should work.

All text in new UI created in this task should be localizable.

To verify, make sure you don't delete wrong content. You can create a temporary folder in GacUI/Test/FileDialogPlayground, do anything you want in this folder for testing, and delete FileDialogPlayground before committing.

It is enough to only test against `CppTest` and `CppTest_Tui`.

No need to update `DebugRemoteProtocolSOP.md`.

## DETAILS

### Shared implementation and command targets

- The shared implementation is `FileDialogViewModel` in `<GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, with its interfaces in `<GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h`. Both GUI and TUI Open/Save dialogs use this model. Keep filesystem operations and refresh behavior here, with presentation and input collection in the corresponding XML resources.
- Creating a folder acts on the folder currently displayed in the list. Deleting acts on the actual selected list items, obtained through the item provider so sorting and filtering do not change their identities. Do not use the editable file-name text box or `ParseDisplayString` to determine deletion targets; that text intentionally supports arbitrary paths for Open/Save.
- Capture the current folder and the selected item paths before opening a confirmation prompt. The confirmed names and the deletion targets must remain the same even if asynchronous enumeration finishes while the modal is open. Preserve the existing single/multiple-selection option; do not enable multiple selection just for deletion.
- Disable commands while their folder/list state is unavailable or loading, and prevent duplicate submissions while a mutation is pending. Delete is disabled with no selected real items; loading placeholders are never targets. The parent file dialog stays open after either command.
- The Windows synthetic root is a drive list, not a writable directory. Disable both commands there and reject drive rows as deletion targets in the shared model as well as the UI. Checking only the row's folder type is insufficient: drives are ordinary folder rows, and `Folder::Delete(true)` can delete their contents before failing to remove the drive. Do not disable normal child operations merely because the containing directory is a filesystem root: POSIX `/` is a real directory, unlike the Windows empty-path root.

### New-folder behavior

- Accept a single child name, not a path. Reject empty names, `.` and `..`, rooted inputs and the active filesystem's path delimiters before performing a mutation. Use `filesystem::FilePath` normalization and verify that the result remains an immediate child of the captured directory. Preserve valid spaces and Unicode characters; let the filesystem report other unsupported names through the general failure message.
- Invalid or overlong user input can make Windows path construction throw `ArgumentException` before `Create` returns a result. Translate that expected input failure into the localized general creation message and keep the modal open; do not suppress unrelated exceptions.
- Check `filesystem::Folder(candidate).Exists()` first for the specified already-exists message, then call `Create(false)` so no missing parent hierarchy is created. A file occupying that name uses the general failure message. A failed create after the existence check also uses the general failure message; the precheck is not an atomic reservation.
- The modal title is localizable `New Folder`. Focus its text box initially. Cancel and title-bar close perform no filesystem operation. A failed attempt leaves the entered name and modal available for correction; only success closes the new-folder modal and refreshes the parent view. Creating a folder does not automatically navigate into it.

### Deletion and refresh

- Use `filesystem::File::Delete()` for selected files and `filesystem::Folder::Delete(true)` for ordinary selected directories. This is permanent deletion, with the specified OK/Cancel confirmation before any mutation. Cancel and prompt close do nothing. Keep selected-item order in both confirmation and failure lists.
- Recursive deletion can partially remove a directory before returning false. Continue with every remaining selected item, collect failures by selected top-level name, and show one combined failure prompt afterward. Do not promise rollback or a separate failure entry for every descendant. A disappeared target that cannot be deleted is also a failed selected item.
- The current imported recursive deletion follows directory symlinks/junctions, including links nested inside an ordinary folder. For this task, preflight each selected directory using VlppOS enumeration and `FileInfo::isSymbolicLink` / `isReparsePoint`; reject the selected directory as a failure before deleting any of its contents if it or a descendant directory is linked, or if traversal cannot be completed. Continue with other selected items. Do not recurse into link targets or introduce native filesystem calls in the view model. This conservative policy does not claim protection against another process replacing entries concurrently; general race-proof filesystem deletion is outside this task.
- Refresh after successful creation and after every confirmed deletion batch, including partial failure. Reconcile both the file list and the affected tree children against the filesystem. The existing `RefreshFiles()` adds missing tree nodes but does not remove deleted nodes: update `children`, `childrenByName`, parent relationships and surviving child `Index` values together, preserving unaffected folder objects and expansion state where possible.
- Invalidate obsolete tree enumeration and file-list results using the existing task/generation mechanism so queued results cannot restore deleted entries. Keep observable collections, properties, message boxes and window operations on the UI thread. Reuse the existing worker queue for filesystem work, particularly recursive traversal, without changing its locked scheduling model. Completion must not touch disposed dialogs or replace the list of a different folder after navigation.
- Preserve the current folder, active filter and normal Open/Save behavior. Clear selection references to removed rows and synchronize selection-derived filename text; do not erase an independently typed save filename merely because the contents were refreshed. Expanded, collapsed and previously unloaded tree branches must all show the current contents when visited.

### Resources, localization and generation

- Author the header controls and new modal instances in `<GacUI repo>/Source/Utilities/FakeServices/Dialogs/FileDialog.xml` and `<GacUI repo>/Source/Utilities/FakeServices/TuiDialogs/FileDialog.xml`. Keep the path label flexible and the two actions at its right. Use GUI toolbar buttons with the provided images and ordinary TUI buttons with cell-based spacing. Long paths must not obscure the actions.
- Register `<GacUI repo>/Source/Utilities/FakeServices/Dialogs/ToolbarIcons/NewFolder.png` and `<GacUI repo>/Source/Utilities/FakeServices/Dialogs/ToolbarIcons/Delete.png` in `<GacUI repo>/Source/Utilities/FakeServices/Dialogs/Resource.xml`. Include their actual dimensions in `UnitTestConfig/ImageData`; the existing snapshot renderer requires entries for every image it encounters.
- Add all new captions, modal titles, labels, GUI action tooltips/accessibility text and complete parameterized messages to both flavors' `Resource.xml`. Use parameters for folder paths, child names and failure lists so translated sentences can reorder them. Retain the requested English message wording, and reuse the existing localized OK/Cancel captions.
- Extend the existing Chinese string injections in `<GacUI repo>/Test/Resources/App/FullControlTest/LocalizedDialogsTabPage.xml` and `<GacUI repo>/Test/Resources/App/TuiControlTest/LocalizedDialogsTabPage.xml`. Updating only the English base resources would leave the new behavior incompletely localized in the two verification applications.
- Use `FakeDialogServiceBase::ShowMessageBox` for confirmation and failure prompts, preserving modal ownership and the active GUI/TUI flavor. Own the new-folder modal by the file dialog and its failure prompt by the new-folder modal. Keep tab traversal and focus restoration usable through repeated open, failure, retry and cancel operations.
- Register changed view-model interfaces in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp`. Follow `<GacUI repo>/Project.md` for metadata generation, resource compilation and rebuilding consumers in architecture order, including metadata regeneration after `GacUI_Compiler`. Regenerate both dialog flavors and affected showcase outputs; do not hand-edit generated files. Keep authored and generated changes in separate commits when executing this task.

## VERIFICATION

These are acceptance steps for implementing the task; this review does not execute them.

### Preparation and execution scope

- Use only `<GacUI repo>/Test/FileDialogPlayground` for mutable fixtures. Verify its resolved location before setup or recursive cleanup. Include ordinary files, empty and populated directories, spaces/CJK names, an existing-name collision and an unselected sentinel. Any link fixture and its target must both remain inside this playground, with the target outside the selected subtree.
- Complete required metadata/resource generation and architecture builds through the repository wrappers, inspect completed logs and check for resource compiler error files. Runtime feature verification is limited to `CppTest` and `CppTest_Tui` as requested; do not expand the application matrix or update the remote-protocol SOP.
- Use the existing localized file-dialog showcase pages and set their Directory fields to the playground. Cover Open and Save dialogs and enable multiple selection where the showcase supports it. For `CppTest`, use automation-service IO/state first, then the corresponding Windows UIA operations. Launch `CppTest_Tui` through the documented interactive CLI wrapper; if desktop terminal access fails, continue through an interactive session or owned pseudoconsole and distinguish replay/cell evidence from physical input and displayed appearance.

### Acceptance cases

| Case | Required observation |
| --- | --- |
| Header and modal layout | GUI shows the two supplied icons; TUI shows New Folder/Delete. Long paths and Chinese text leave both actions usable. Check normal/reduced GUI sizes and TUI at 120x40 and 80x25. The modal has the label, text box and right-aligned OK/Cancel; keyboard focus and tab order are usable. |
| Create and cancel | Cancel and title close create nothing. A valid space/CJK child name creates exactly one directory in the displayed folder, closes only the new-folder modal and updates both list and tree. The current directory and filter remain unchanged. |
| Existing names and retry | An existing folder produces the already-exists message. An existing file produces the general failure message. The modal retains its input; correcting the name succeeds without reopening it. |
| Invalid names | Empty, dot, parent, rooted, separator-containing and overlong inputs create no directory and cannot escape the displayed folder. They follow the general failure behavior and leave the modal open without an unhandled path-construction exception. |
| Selection and cancellation | Delete is disabled with no real selection and during unavailable/loading state. After sorting/filtering, the confirmation lists precisely the selected rows. Typed filename text cannot redirect deletion. Cancel leaves all fixtures unchanged. |
| Successful deletion | Delete a file, an empty folder, a populated folder and a mixed multi-selection. Only the confirmed items disappear; recursive contents are removed, the sentinel remains, and no success prompt appears. Repeat in single-selection Open and Save modes. |
| Partial failure | Hold a fixture file open without delete sharing from a separate process, then confirm a batch containing that file and deletable items. Later items are still attempted, successful removals are reflected, and one prompt lists only failed selected names. Also exercise a populated folder with a locked descendant and verify its actual partial contents after refresh. Release the handle and retry successfully. |
| Root and link boundaries | Inspect the synthetic Windows drive list and verify both commands are disabled without invoking deletion on a drive. Select a playground directory link/junction and then an ordinary directory containing one; each is reported as a failed selected item, the target sentinel survives, and an unrelated selected ordinary item can still be deleted. |
| Tree coherence and pending work | Create/delete with the affected tree branch expanded, collapsed and previously unloaded. Navigate away/back and expand remaining siblings; no deleted node reappears and every surviving node navigates to the correct folder. Rapid refresh/navigation and repeated commands must not apply an obsolete list or execute a mutation twice. |
| Localization and continued use | Repeat representative success, duplicate-name, confirmation and partial-failure flows in English and Chinese in both applications. Check titles, actions and complete messages, then complete ordinary Open/Save with the existing filter. Reopen/cancel the new modal repeatedly and verify parent focus and normal shutdown. |

- Inspect the filesystem as well as UI state after each destructive case. Preserve unselected sentinels, inspect logs for failures, and record any unavailable UIA/terminal coverage explicitly. Windows runs do not establish Linux/macOS runtime verification.
- Close test applications, release held handles, remove fixture links without following their targets, then remove the verified playground before committing the implementation. Inspect the final diff for temporary instrumentation, fixture paths and unintended generated changes.

## REVIEW COMMENTS

No unresolved review comments. The implementation details and verification cases above resolve the identified gaps without changing the requested GUI/TUI scope.

# UPDATES

# TEST [CONFIRMED]

Use the acceptance matrix above, with mutable fixtures confined to <GacUI repo>/Test/FileDialogPlayground. First establish that both existing dialog resources lack the requested commands. Add focused regression coverage for shared mutation rules and refresh behavior, then verify the generated CppTest and CppTest_Tui consumers. Required generation and metadata checks remain in scope. Record unavailable desktop coverage explicitly.

Baseline: the Debug x64 solution build succeeded with zero errors and two pre-existing Unicode/code-page warnings in <GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Caret.cpp. The repository UnitTest wrapper exited 1 at the new `Folder mutation commands are available in an ordinary directory` case in <GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp: `FindControlByText` could not find New Folder. CppTest automation independently opened Misc / Dialogs / Open/Save FileDialog and observed zero toolbar controls and no New Folder/Delete actions in the picker. The test application was then closed normally.

# PROPOSALS

- No.1 Shared queued mutations with captured item identities and reconciled directory views [CONFIRMED]

## No.1 Shared queued mutations with captured item identities and reconciled directory views

Extend `IFileDialogViewModel` in <GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase.h with availability/progress notifications and asynchronous create/delete operations. Keep their implementation in `FileDialogViewModel` in <GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp, using the unchanged locked worker queue. The UI supplies complete localized creation messages and parameterized deletion formatters. Capture selected item identities and current directory before the confirmation prompt; never parse the editable filename for deletion.

Creation validates one immediate child using the active filesystem delimiters and normalized `FilePath`, translates expected path-construction `ArgumentException`, checks existing directories, then uses `Folder::Create(false)`. Deletion rejects synthetic-root/drive rows, preflights directory links and traversal, attempts every selected top-level item and reports one ordered failure list. Check owner disposal before touching dialogs in completions.

Reconcile existing folder children and their name lookup, parent links and indices while preserving surviving objects. Invalidate obsolete enumeration generations, refresh the affected subtree after mutations and only replace a list belonging to the same selected directory. Preserve independently entered filenames when refreshed selection is cleared.

Author both headers and input modals in the existing GUI/TUI XML files, register the supplied 16x16 icons, and add matching English/Chinese strings. Register the interface changes and regenerate metadata, dialog resources and affected showcase consumers in the documented architecture order. Use CppTest and CppTest_Tui for runtime feature verification, alongside required focused unit/metadata checks.

### CODE CHANGE

Implemented the shared interface and reflection registrations, command availability, captured-target mutation operations and asynchronous lifetime checks in the sources identified above. Creation validates one immediate child, translates expected path-construction failures, distinguishes existing directories, and calls Folder::Create(false). Deletion materializes the selected provider rows before confirmation, preflights selected and descendant directories for link metadata and complete enumeration, attempts every selected top-level target, and reports one ordered failure list. Synthetic-root and drive rows are rejected in the model. The existing locked worker queue remains unchanged; observable state and window operations stay on the UI thread.

Directory refresh reconciles child containers, name lookup, parent relationships and surviving indices while retaining unaffected objects. Generation checks discard obsolete results. Completion refreshes the affected subtree and a current list in that subtree, while preserving a different directory's pending generation. Reconciliation reannounces the current folder so the existing tree-location handler restores selection after item-count changes. Mutation-triggered tree completions do the same after navigation. Text-only TUI placeholder rows now start lazy enumeration through GetName(), matching the existing GetType() trigger used by GUI image bindings.

The authored <GacUI repo>/Source/Utilities/FakeServices/Dialogs/FileDialog.xml and <GacUI repo>/Source/Utilities/FakeServices/TuiDialogs/FileDialog.xml resources contain the new header actions and owned new-folder modal. GUI buttons use the supplied 16x16 icons, with registered snapshot image dimensions; TUI uses localized ordinary buttons. The flexible path label leaves the actions usable at reduced sizes. The modal focuses its input, disables tab insertion, supports Enter, retains failed input, and closes only after success or cancellation. English base resources and both Chinese showcase injections contain complete parameterized captions and messages. Deletion formatters supply an explicit newline parameter because the XML parser does not decode numeric character entities. Selection tracking preserves independently typed filenames while clearing names derived from removed list selections.

Extended the existing filesystem mock and added four rendered regression cases in <GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp. They cover retry, invalid names, Unicode creation, cancellation, recursive deletion, sorted/filtered row identity, ordered combined failures, selected/nested links, synthetic-root guards, duplicate-command rejection, and navigation/refresh while confirmation remains open. The pending-navigation case also verifies detached parent/index values and actual tree selection. Native generated-control access is used for the test view-model helper.

Added the hosted FileDialogMutations scenario to <GacUI repo>/Test/UIA_CppTest.ps1 and its shared client, and documented it in <GacUI repo>/Project.md. It runs automation IO before UIA, owns all fixtures and native lock holders, checks both locales and Open/Save modes, validates geometry and keyboard traversal, and explicitly unlinks fixture junctions before cleanup. Generated dialog/showcase sources, metadata and snapshots are retained in separate commits. Compiler-produced reordering is preserved. Snapshot changes outside the file-dialog cases are image-identifier shifts from registering the two additional images.

### CONFIRMED

The proposal is confirmed. Both applications expose the requested commands, and native filesystem checks agree with the displayed confirmation and failure lists. Typed filename text cannot redirect deletion. Invalid creation leaves the modal and input available for correction, partial failure does not prevent later targets from being attempted, and refreshed tree/list state preserves the current directory and independent filename.

| Validation | Result |
| --- | --- |
| Generation and builds | Completed the required Win32 build/metadata generation, x64 build/metadata generation/metadata validation, GacUI_Compiler, then the required architecture-ordered rebuild and metadata sequence. The compiler completed both architectures and merged GUI/TUI sources; no resource error files remained. Final builds after the selection correction passed with zero warnings and errors in both architectures. |
| Full unit suite | Debug Win32 UnitTest passed 97/97 test files and 1,997/1,997 test cases through the repository wrapper. The completed <GacUI repo>/.github/Scripts/Execute.log contains no skipped files or appended memory-leak dump. All four new rendered cases and existing Open/Save/filter/selection cases passed. |
| Final GUI replay | Debug x64 CppTest passed 2,929 assertions through automation IO and Windows UIA, in English and Chinese, for Open and Save. Coverage includes normal/reduced picker sizes, header bounds, initial input focus, Tab/Shift+Tab, cancel/title close, duplicate-folder/file collisions, empty/dot/parent/rooted/separator/overlong names, Unicode retry, exact messages, recursive deletion, independent filename retention and ordinary filtered Open/Save. |
| Native failures and boundaries | Separate processes held top-level files and recursive descendants without delete sharing. Later selected files, empty folders and populated folders were removed despite earlier failures. The partial directory retained only its locked descendant. Selected and nested junctions were rejected before deleting their contents, and both target/subtree sentinels survived. Ordered failure reports and successful retries after releasing handles passed in both applications and languages. The GUI also inspected the Windows drive list and disabled commands without invoking drive deletion. |
| Tree and pending work | Unit tests cover sorted/filtered selection identity, repeated refresh, duplicate submissions, captured confirmation targets after navigation, stale-result rejection, surviving indices and detached nodes. GUI checks cover collapsed partial branches and surviving-sibling navigation. TUI checks cover visible lazy placeholders, a previously unloaded populated branch, expansion, deletion, navigation away/back and retained current-directory selection after creation/deletion. |
| TUI replay | CppTest_Tui ran through the interactive CLI wrapper in an owned pseudoconsole at 120x40 and 80x25. English/Chinese create/error/retry/confirmation/failure flows, Unicode names, single/multiple selection, ordinary Open/Save and reduced-size layout were checked using console input records, captured cells and filesystem state. Rebuilt consumers specifically repeated newline formatting and Tab/Shift+Tab/Enter checks. Final x64 cell attributes confirmed retained tree selection after reconciliation. |
| Shutdown and cleanup | GUI runs closed normally and released their automation endpoint. TUI runs exited 0 and restored their original console input/output modes. Native holders exited; fixture links were unlinked without following targets; sentinels were verified. The entire resolved <GacUI repo>/Test/FileDialogPlayground was removed before implementation commits. Temporary project user arguments were restored, and no probe/helper files remain. |

Validation found and corrected several implementation issues before acceptance: numeric newline entities were displayed literally; Tab initially inserted text; TUI tree enumeration depended on a GUI-only image binding; and child removal cleared tree selection without reannouncing the unchanged current directory. CDB also identified test-helper failures involving virtual filesystem injection during derived construction and an invalid generated-control reflection lookup; the mock/helper now use the existing supported mechanisms. An earlier x64 incremental link reported LNK1163; an identical wrapper retry passed without source or manual artifact changes. Both final architecture builds passed normally.

Desktop terminal access was unavailable, so TUI evidence establishes native console replay and cell output, not physical keyboard/mouse input or displayed terminal appearance. GUI coverage uses automation/UIA state and geometry. The TUI size checks reopened the picker at each target size; shrinking an already-open picker retained its earlier placement, and live viewport repositioning was not changed. Linux/macOS runtime behavior was not verified. Recursive deletion may remain partial after failure, and link preflight does not claim protection against concurrent filesystem replacement, as specified in the task.
