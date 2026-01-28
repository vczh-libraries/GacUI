# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.7: TEST_CATEGORY for dialog message box interactions (open/save options)

Add dialog-driven tests for error/prompt message boxes triggered by `TryConfirm`, by causing invalid selections and answering prompts in the UI.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` for message box interactions.
- When adding proxy frames (`UnitTestRemoteProtocol::OnNextIdleFrame`), name each frame by what the previous frame has done, and ensure each frame makes a visible UI change (split “type”, “click confirm”, “close message box”, “assert output” into separate frames when needed).
- Keep assertions deterministic (avoid `A || B`); when asserting selection display strings, control the selection order and assert the exact expected string.
- When reading the file name text box (`filePickerControl.textBox`), expect `FileDialogViewModel::GetDisplayString(...)` to always quote file names; multi-selection display strings are formatted like `"a";"b"` (and single selection like `"a"`).
- When finding buttons to click, use string literals directly (e.g. `Open`, `Save`, `OK`, `Cancel`); no need to involve `DialogStrings` for button texts.
- Locate and close message dialogs by native-parent relationships (`GetOpeningMessageDialog`) instead of relying on titles or sending `Enter`; closing should be a real UI click (so the test harness observes a UI update).
- Cover at least these scenarios (reusing the existing open/save buttons from `resourceFileDialogs` to open dialogs with the intended options):
	- **File must exist (open dialog)**: click the open button that enables `FileDialogFileMustExist`, type a non-existing file name and press `Open`; assert a message box appears and that clicking `OK` dismisses it while keeping the file dialog open.
	- **Prompt create file (open dialog)**: open a dialog that enables `FileDialogPromptCreateFile` (add a launcher button in `resourceFileDialogs` if needed), type a non-existing file name and press `Open`; assert the create prompt appears and validate both `OK` (confirm selection) and `Cancel` (stay in dialog) flows.
	- **Prompt overwrite file (save dialog)**: click the save button that enables `FileDialogPromptOverwriteFile`, type an existing file name and press `Save`; assert the overwrite prompt and validate both `OK`/`Cancel` flows.
	- **Multi-selection not enabled**: open a dialog without `FileDialogAllowMultipleSelection`, attempt to confirm multiple selections and assert the “Multiple selection is not enabled” message box and dismissal behavior.

### rationale

- These behaviors are user-visible and driven by localized dialog resources; asserting them through the UI is the most reliable way to ensure the wiring between `FileDialog.xml`, `Resource.xml`, and `TryConfirm` stays correct.

# UPDATES

## UPDATE

`Centralize dialog string access for tests` is not necessary, you can just use Open/Save/OK/Cancel, and I think Yes and No is not used, you can checkout the TryConfirm function to make sure of it.

`Case D: Multi-selection not enabled shows error and stays open` you can only type to the text box with multiple files separated by ";", as the dataGrid will disable multiple selection if there is no such option.

In order to trigger `dialogErrorFileExpected`, you still need to type to the text box mixing files and folders. A Case E can be added.

`Frame naming and UI-step granularity` is pretty good. But I still want to emphasize: ensure that the name of the frame describes what the previous frame has been done, and each frame should make actual UI change.

For Case B/C, you can press Cancel first and then OK, so that the file dialog could stay across the whole test case.

# INSIGHTS AND REASONING

## Context and existing infrastructure

- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` already drives file dialogs using `GacUIUnitTest_SetGuiMainProxy(...)` + `UnitTestRemoteProtocol::OnNextIdleFrame(...)` and opens dialogs from the embedded `resourceFileDialogs` Workflow resource.
- The file dialog and message box windows can be located reliably via native parent relationships:
	- `GetOpeningFileDialog()` filters windows by `GuiWindow::GetText() == L"FileDialog"` and checks the native parent is null or the main window.
	- `GetOpeningMessageDialog()` finds another `GuiWindow` whose native parent is the file dialog’s native window.
- The confirm logic that triggers message boxes lives in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` (`FileDialogViewModel::TryConfirm`):
	- Error paths use `dialogService->ShowMessageBox(..., DisplayOK, IconError, ...)` and return `false`.
	- Prompt paths (create/overwrite) use `DisplayOKCancel, IconQuestion` and return `false` only when the user selects `Cancel`.
	- Multiple-selection display formatting comes from `FileDialogViewModel::GetDisplayString(...)`, which always quotes file names and joins them with `;`.
- The dialog resources in `Source/Utilities/FakeServices/Dialogs/Resource.xml` define the actual captions. Tests can click buttons using simple English captions (`Open`, `Save`, `OK`, `Cancel`) without accessing `DialogStrings`.

## Design goals

- Add a new `TEST_CATEGORY` dedicated to dialog message box interactions, without weakening existing navigation/selection tests.
- Keep UI-driving steps and assertions deterministic:
	- One visible UI action per `OnNextIdleFrame` (type, press, close message box, assert).
	- Frame names describe what the previous frame has done.
	- Avoid ambiguous assertions and selection-order uncertainty.
- For button texts, hard-coded English captions are acceptable (e.g. `Open`, `Save`, `OK`, `Cancel`); `DialogStrings` is not required.

## Proposed changes (high-level)

### 1) Use string literals for button texts

- Use string literals directly wherever button text is needed (e.g. `Open`, `Save`, `OK`, `Cancel`).
- `FileDialogViewModel::TryConfirm` only produces `INativeDialogService::DisplayOK` or `INativeDialogService::DisplayOKCancel` message boxes, so `Yes()` / `No()` are not required for these scenarios.

Rationale: button captions used by these dialogs are stable in the test environment (`Open`, `Save`, `OK`, `Cancel`), so using string literals keeps the tests simple and avoids wiring in `DialogStrings` just for button lookup.

### 2) Add missing launcher button(s) in the test UI resource

- Extend `resourceFileDialogs` in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to include an “Open PromptCreateFile” button (only if no existing open-launcher already enables `FileDialogPromptCreateFile`):
	- In the Workflow click handler, set `dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;`
	- Reuse the same `ShowDialog()` -> write result to `self.Text` pattern as existing open tests.

Rationale: Task No.7 requires prompt-create flows in an open dialog; existing launchers cover `FileDialogFileMustExist` and save-side options but not open-side `PromptCreateFile`.

### 3) Add `TEST_CATEGORY` for message box interactions

Add `TEST_CATEGORY(L"File Dialog Message Box Interactions")` to `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, containing at least the following cases.

#### Case A: File must exist (open dialog) shows error and stays open

- Open the dialog using the launcher that enables `INativeDialogService::FileDialogOptions::FileDialogFileMustExist` (currently “Open DefaultOptions”).
- Type a non-existing file name and press `Open`.
- Assert a message dialog appears (`GetOpeningMessageDialog()` returns a window).
- (Optional but recommended) Read the message label text (from `MessageBox.xml`, the `Label` bound to `ViewModel.Text`) and assert it is built from `strings->FileDialogErrorFileNotExist()` and includes the relative path of the typed file.
- Click `OK` and assert:
	- The message dialog closes.
	- The file dialog remains open (e.g. `GetOpeningFileDialog()` still succeeds).
- Close the file dialog via `Cancel` and assert no selection is committed (`dialogOpen->GetFileNames().Count() == 0`).

#### Case B: Prompt create file (open dialog) OK confirms; Cancel keeps dialog open

Keep the file dialog open across both branches:

- Open the dialog via the new “Open PromptCreateFile” launcher.
- Type a non-existing file and press `Open`.
- Assert a prompt message dialog appears; (optional) assert message starts with `strings->FileDialogAskCreateFile()`.
- Click `Cancel` and assert the file dialog stays open and no selection is committed.
- Press `Open` again (keeping the same input) and assert the prompt appears again.
- Click `OK` and assert the dialog closes with selection committed (read `dialogOpen->GetFileNames()` or the main window text set by the Workflow handler).

#### Case C: Prompt overwrite file (save dialog) OK confirms; Cancel keeps dialog open

Keep the file dialog open across both branches:

- Open the save dialog via existing “Save PromptOverwriteFile” launcher (sets `INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile`).
- Type an existing file name (not click-select) and press `Save`.
- Assert the overwrite prompt appears; (optional) assert message starts with `strings->FileDialogAskOverrideFile()`.
- Click `Cancel` and assert the save dialog stays open and no selection is committed.
- Press `Save` again (keeping the same input) and assert the prompt appears again.
- Click `OK` and assert `dialogSave->GetFileName()` is the expected full path and the dialog closes.

Rationale: this aligns with `TryConfirm` branching (`selectToSave && promptOverriteFile` uses `DisplayOKCancel` and checks only for `SelectCancel`), and it matches the required interaction sequence (typed existing name).

#### Case D: Multi-selection not enabled shows error and stays open

- Open an open dialog that does not enable `FileDialogAllowMultipleSelection` (currently “Open DefaultOptions”).
- Since the data grid disables multiple selection in this mode, type multiple selections directly into the text box (e.g. `README;root.txt`) and press `Open`.
- Assert the message dialog appears; (optional) assert message equals `strings->FileDialogErrorMultipleSelectionNotEnabled()` (this path uses a plain string, not a generated list).
- Click `OK` and assert the file dialog remains open.
- Close the file dialog via `Cancel` and assert no selection is committed.

#### Case E: File(s) expected but folder(s) selected shows error and stays open

- Open an open dialog that enables `FileDialogAllowMultipleSelection` (currently “Open MultipleSelection”).
- Type a mixed list of an existing file and an existing folder into the text box (e.g. `README;A`) and press `Open`.
- Assert the message dialog appears; (optional) assert the message starts with `strings->FileDialogErrorFileExpected()` and includes the folder name.
- Click `OK` and assert the file dialog remains open.
- Close the file dialog via `Cancel` and assert no selection is committed.

### 4) Frame naming and UI-step granularity

For each test case, structure frames like:

- Every frame performs a UI action that produces a visible UI change, and the frame name describes what the previous frame has done.
- `Ready` -> click launcher button
- `Clicked: ...` -> perform one action (type/select) only
- `Typed: ...` / `Selected: ...` -> press confirm button
- `Pressed: Open/Save` -> assert message dialog appears
- `Popped up message dialog` -> close message dialog via localized button click
- `Closed message dialog` -> assert still-open or committed result

This matches the task requirement (“name each frame by what the previous frame has done”) and keeps proxy frames deterministic and easy to debug.

## Risks and mitigations

- Message text formatting could be newline-sensitive. To keep tests stable:
	- Prefer asserting the presence of the correct message dialog and correct commit/stay-open behavior.
	- If asserting message text, build expected strings from `IDialogStringsStrings` + `filesystem::FilePath` computations (rather than hard-coded `/` paths) and normalize line endings before comparing.
- Avoid relying on dialog titles to find message dialogs; always use `GetOpeningMessageDialog()` (native-parent based) as already implemented.

# !!!FINISHED!!!
