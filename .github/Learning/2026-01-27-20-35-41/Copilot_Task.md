# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

# INSIGHTS AND REASONING

## Context / Current State

- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` defines a unit-test UI resource string `resourceFileDialogs` that currently hosts:
	- `<OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>`
	- Two buttons that set `dialogOpen.Options`, call `dialogOpen.ShowDialog()`, and join `dialogOpen.FileNames` into `self.Text`.
- The unit-test driving helpers (`PressOpen`, `PressSave`, `PressCancel`, `TypeFile`, `ChooseFilter`, `GetOpeningFileDialog`) assume:
	- The file dialog window title is `"FileDialog"`.
	- Dialog internals are located via `ref.Name="filePickerControl"` children (e.g. `dataGrid`, `textBox`, `comboBox`).
- `Source/Utilities/FakeServices/Dialogs/FileDialog.xml` explicitly supports both modes: `MakeOpenFileDialog()` and `MakeSaveFileDialog()` (e.g. `buttonOK.Text` switches to "Open" / "Save"), so a `GuiSaveFileDialog` should reuse the same visual structure and internal control names.

## Proposed Design

### Resource wiring (UI)

- Extend `resourceFileDialogs` by adding a `GuiSaveFileDialog` resource entry near `dialogOpen`:
	- `<SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>`
	- Keep `Title`, `Filter`, and `Directory` identical to `dialogOpen` so:
		- `GetOpeningFileDialog()` continues to locate the dialog window via its title.
		- Existing filter/navigation tests remain stable and comparable between open/save dialogs.
- Add two new buttons in the same table (prefer extending the existing row/column definition rather than restructuring):
	- Button A: "Save PromptCreateFile"
		- In `Clicked` handler: set `dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;`
		- Call `dialogSave.ShowDialog()` and, if true, join `dialogSave.FileNames` into `self.Text` (same pattern as `dialogOpen`).
	- Button B: "Save PromptOverwriteFile"
		- In `Clicked` handler: set `dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile;`
		- Same `ShowDialog()` + join output behavior.

### Compatibility / Non-goals

- Keep existing open buttons unchanged (text and behavior). This prevents churn in already-landed tests (Tasks 1-4) which locate buttons by text (e.g. `FindControlByText<GuiButton>(..., L"Open DefaultOptions")`).
- Do not add any message-box related assertions in this task; this task is only introducing a stable entry point for later tests to open the save dialog with save-only options.

## Evidence / Rationale

- The open-dialog test UI already uses the correct, stable interaction pattern: set dialog options -> `ShowDialog()` -> read `FileNames` from the dialog instance (`Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`).
- Save/open are already differentiated by the dialog's OK button label in the dialog resource (`FileDialog.xml` contains `MakeSaveFileDialog()`), so adding a save dialog should not require changes to test-driving helpers beyond being able to click a new "open save dialog" button.
- Reusing the same `Title` ("FileDialog") keeps the existing window-finding logic working without branching for open vs save dialogs.

# !!!FINISHED!!!
