# !!!TASK!!!
# PROBLEM DESCRIPTION
## TASK No.3: TEST_CATEGORY for filter visibility + extension behavior (in dialog)

Add dialog-driven tests validating that changing the filter updates the listing, and that the selected filter affects typed selection / extension normalization.

### what to be done

- In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` dedicated to filter behaviors.
- Add helper functions for selecting the correct opened dialog window when both file dialog and message dialog share the same title:
	- `GetOpeningFileDialog`: find `GuiWindow::GetText() == L"FileDialog"` with `w->GetNativeWindow()->GetParent() == nullptr`.
	- `GetOpeningMessageDialog`: find `GuiWindow::GetText() == L"FileDialog"` with `w->GetNativeWindow()->GetParent() != nullptr`.
	- Refactor existing helper functions (e.g. `PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter`) to use `GetOpeningFileDialog` instead of duplicating `From(GetApplication()->GetWindows())...`.
- Reuse the test dialog’s `Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt"` and drive the UI (avoid re-testing basic folder navigation and listing already covered by TASK No.2; only assert deltas caused by filter/extension logic):
	- Switch filters via `ChooseFilter(protocol, filterIndex)`.
	- Add a minimal visibility assertion (e.g. in `/A`, `a.txt` remains visible in `Text Files`, while a known non-`.txt` entry becomes invisible).
- Add typed-selection subcases that demonstrate the chosen filter’s expected extension behavior:
	- Navigate into `/A`, choose `Text Files (*.txt)`, type `L"a"` and click `Open`; assert the returned filename is `/A/a.txt` (extension auto-append).
	- Choose `All Files (*.*)`, type `L"README"` at root and click `Open`; assert the returned filename is `/README` (no auto-append under the all-files filter).
	- Regression: choose `Text Files (*.txt)`, type `L"README"` at root and click `Open`; expected behavior is the dialog stays open and shows the "File(s) not exist" message because the real target is `README.txt` (auto-append happens before existence checks). Use `GetOpeningMessageDialog` to locate the message dialog and close it (following patterns in `Test/GacUISrc/UnitTest/TestApplication_Dialog_Message.cpp`). If the test fails, fix `FileSystemViewModel::TryConfirm` in `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` so that extension auto-append happens before existence/prompt checks, while folder navigation still uses the raw, non-appended name when the selection resolves to a folder.

### rationale

- Filter correctness is core dialog behavior and is tightly coupled to selection parsing and extension normalization; validating it through the UI ensures `FileDialog.xml` bindings and localized text initialization are also exercised.
- Extension auto-append influences both the returned selection and validation logic (existence checks, prompts); the regression case ensures these stay consistent under a restrictive filter.

# UPDATES

# INSIGHTS AND REASONING

## Problem Restatement (Observed Risk)

`Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` currently locates a file dialog window by title only (`GuiWindow::GetText() == L"FileDialog"`). `FileSystemViewModel::TryConfirm` shows message boxes using `owner->GetText()` as the title, so the message dialog title also becomes `L"FileDialog"`. This creates ambiguous window selection in UI-driven tests once message boxes are involved.

In `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, `FileSystemViewModel::TryConfirm` appends the selected filter/default extension to `confirmedSelection` only after it has already performed existence checks and prompt logic. This order can cause a false-positive confirmation under restrictive filters (e.g. selecting `README` under `*.txt` should behave like selecting `README.txt`).

## Design Goals

- Keep dialog-driven tests stable when both the file dialog and a modal message dialog are present.
- Validate filter-driven visibility and typed selection behavior through the real dialog UI so that bindings/resources are also covered.
- Ensure confirmation logic validates the final (extension-normalized) selection, not the pre-normalized input, while keeping folder navigation semantics unchanged.

## Proposed Changes (High Level)

### 1) Disambiguate windows in tests

In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`:

- Add two centralized helpers:
	- `GetOpeningFileDialog()`: choose the top-level `GuiWindow` titled `L"FileDialog"` by checking `GetNativeWindow()->GetParent() == nullptr`.
	- `GetOpeningMessageDialog()`: choose the owned modal `GuiWindow` titled `L"FileDialog"` by checking `GetNativeWindow()->GetParent() != nullptr`.
- Refactor UI helpers (`PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter`) to call `GetOpeningFileDialog()` so any future message box does not hijack the “first window” selection.
- For the regression case, locate the message dialog via `GetOpeningMessageDialog()` and close it using the same interaction style as `Test/GacUISrc/UnitTest/TestApplication_Dialog_Message.cpp` (e.g. `protocol->KeyPress(VKEY::KEY_RETURN)` or clicking the localized `OK` button, depending on existing conventions in the file dialog test).

This isolates “which window” logic into one place and makes the new tests resilient to additional modal dialogs.

### 2) Add filter-focused dialog-driven tests

In `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`, add a `TEST_CATEGORY` focused on:

- Filter visibility deltas (minimal assertions):
	- Reuse the existing test resource filter string `All Files (*.*)|*|Text Files (*.txt)|*.txt`.
	- Navigate to `/A` and assert `a.txt` remains visible while `noext` becomes invisible under `Text Files`.
- Typed selection + extension behavior:
	- Under `Text Files`, typed `a` should confirm as `/A/a.txt`.
	- Under `All Files`, typed `README` should confirm as `/README` (no auto-append).
- Regression (extension normalization before validation):
	- Under `Text Files`, typed `README` at root should be treated as `README.txt`; if the dialog enforces "file must exist" (open dialog), it should show "File(s) not exist" and keep the dialog open.

These cases avoid duplicating the broader navigation/listing coverage already present in TASK No.2 and directly target filter/extension coupling.

### 3) Normalize selection before validation in `TryConfirm`

In `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`, adjust `FileSystemViewModel::TryConfirm` so that:

- Folder resolution happens on the raw typed name first:
	- If there is exactly one selection and `wd / raw` is an existing folder, navigate to that folder and return `false` (keep dialog open), without appending extensions.
- For file selections, compute “final file path candidates” before any existence checks:
	- Determine the active extension rule from `selectedFilter->GetDefaultExtension()` first, otherwise `defaultExtension` when non-empty.
	- Apply the same append rules currently used for `confirmedSelection`, but do it on the per-selection paths before classifying into `files`, `folders`, `unexistings` and before prompt checks.
- Run existence checks, `fileMustExist`/`folderMustExist` validations, and `promptCreateFile`/`promptOverriteFile` prompts against these normalized paths.
- Populate `confirmedSelection` from the normalized file paths directly, so the final returned selection and the validation logic are consistent.

This preserves the current behavior of folder navigation while making validation semantics match the user-visible selection rules implied by the filter.

## Evidence and Touch Points

- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`: repeated window selection via `From(GetApplication()->GetWindows()).Where(w->GetText()==L"FileDialog").First()` in helpers like `PressButton`, `TypeFile`, `ClickFileInternal`, `ChooseFilter`.
- `Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`:
	- `TryConfirm(...)` performs existence checks and prompts before appending extensions to `confirmedSelection`.
	- Message boxes are shown through `GetCurrentController()->DialogService()->ShowMessageBox(...)` with `owner->GetText()` as the title, making message dialog titles collide with the file dialog title.

## Acceptance Criteria

- New tests can reliably interact with either the file dialog or message dialog even when both are visible.
- Switching filter updates listing visibility as expected in `/A` (e.g. hides `noext` under `Text Files`).
- Typed selection under `Text Files` appends `.txt` when missing and returns the correct full path.
- Under `Text Files`, typing `README` triggers the "File(s) not exist" message (for open dialog's existence rule) and does not confirm a selection.

# !!!FINISHED!!!
