# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

## UPDATE

You can check the `TryConfirm` function, there should be no mesasge box for saving to a unexisting file. An error would popup when the file does not exist but it only applies to open file dialog, which does't matter because we are testing against save file dialog.

# INSIGHTS AND REASONING

## Current Test Coverage Gap

`Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` currently contains two save-dialog tests:

- `Save PromptCreateFile: Open and Close`
- `Save PromptOverwriteFile: Open and Close`

Both only open the dialog and then press Cancel (via `gacui_file_dialog_template::PressCancel`), asserting `GuiSaveFileDialog::GetFileName()` stays empty. This does not exercise any intended save-dialog confirmation flows (typing a new name, clicking an existing file, and dealing with save-only prompts).

## Constraints / Test Harness Capabilities to Reuse

- The in-test filesystem is deterministic: `gacui_unittest_template::CreateFileItemRoot()` creates a stable root folder containing `root.txt`, `root2.txt`, `README`, and subfolders (so an “existing file” selection is reliable).
- The unit test already has dialog-driving primitives for file dialogs:
	- Window discovery: `gacui_file_dialog_template::GetOpeningFileDialog()` and `gacui_file_dialog_template::GetOpeningMessageDialog()`
	- Dialog interaction: `TypeFile`, `ClickFile`, `PressSave`, `PressCancel`, `ChooseFilter`
- Dialog button texts are localized via `Source/Utilities/FakeServices/Dialogs/Resource.xml` (notably `FileDialogSave` = "Save", `OK` = "OK", `Cancel` = "Cancel"), so tests should prefer using these stable texts when finding buttons.

## Proposed Design for Replacing the Two Placeholder Save Tests

### 1) “Type an unexisting file name” (PromptCreateFile button)

Replace `Save PromptCreateFile: Open and Close` with a test that:

- Opens the save dialog using the existing UI entry point button (`"Save PromptCreateFile"`).
- Types a non-existing file name in the file name text box (via `TypeFile`).
	- Prefer typing a name with a known extension (e.g. `new.txt`) to avoid conflating this task with filter-driven default-extension behavior.
	- Alternatively (if extension behavior is desired for realism), set the filter to “Text Files” using `ChooseFilter(protocol, 1)` and type `new`, expecting the final selection to be normalized to `new.txt`.
- Clicks `Save` (via `PressSave`).
- Asserts the dialog result is confirmed:
	- Read `GuiSaveFileDialog::GetFileName()` from `dialogSave` on the main window and assert it is the expected full path (e.g. `FilePath(L"/new.txt").GetFullPath()`).

This intentionally avoids any message box interaction for this case:

- In the unit test, `Save PromptCreateFile` sets `dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile` (it does not enable `INativeDialogService::FileDialogFileMustExist`), so the “file not exist” error path should be open-dialog-only.
- In `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, `FileDialogViewModel::TryConfirm` currently only asks `dialogAskCreateFile` when `!selectToSave && promptCreateFile`, so a save dialog should not pop up a create prompt for a non-existing file.

This keeps the task focused on realistic save selection flow while not turning into full “prompt behavior matrix” coverage (which belongs to TASK No.7).

### 2) “Overwrite prompt (click existing file)” (PromptOverwriteFile button)

Replace `Save PromptOverwriteFile: Open and Close` with a test that:

- Opens the save dialog using the existing UI entry point button (`"Save PromptOverwriteFile"`).
- Navigates selection by clicking an existing file in the data grid (via `ClickFile(protocol, L"root.txt")`).
	- Using `root.txt` is deterministic because it is created by `CreateFileItemRoot()` at the root folder.
- Clicks `Save` (via `PressSave`).
- Handles the overwrite prompt minimally to allow the dialog to close:
	- Find the message dialog using `GetOpeningMessageDialog()`.
	- Click `OK` to accept overwrite.
- Asserts the dialog result:
	- `GuiSaveFileDialog::GetFileName()` should equal `FilePath(L"/root.txt").GetFullPath()`.

This test’s purpose is to validate the realistic interaction sequence “select existing file from list -> confirm -> prompt -> accept -> dialog closes”, not to validate all combinations of overwrite prompting results.

## Frame Sequencing Guidance (Determinism)

For both tests, keep each `UnitTestRemoteProtocol::OnNextIdleFrame(...)` step aligned with one visible UI change, so the harness reliably observes transitions:

- Open dialog (click the launcher button).
- Make selection (type or click).
- Attempt confirm (press `Save`).
- If a message dialog appears, resolve it with a real UI click (e.g. overwrite prompt: click `OK`).
- Assert results and close main window.

## Non-Goals (Deferred to TASK No.7)

- Validating prompt text contents (`FileDialogAskCreateFile`, `FileDialogAskOverrideFile`) and branching behavior (`OK` vs `Cancel`).
- Testing error message boxes (file must exist, multiple selection not enabled, etc).
- Comprehensive coverage of filter-driven default extension normalization for save dialogs beyond what is needed to keep these two tests realistic.

# !!!FINISHED!!!
