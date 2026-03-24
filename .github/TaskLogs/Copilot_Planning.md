# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Run UnitTest project `UnitTest` (Debug|x64).
  - During iteration: run UnitTest with command arguments `/F:TestApplication_Dialog_File.cpp` to only execute `Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp`.

# EXECUTION PLAN

## STEP 1: Fix default-extension derivation for file dialog filters

### What to change

In `Source\Utilities\FakeServices\GuiFakeDialogServiceBase_FileDialog.cpp`, inside `vl::presentation::FakeDialogServiceBase::ShowFileDialog`, replace the current regex-based default-extension extraction:

	Regex regexFilterExt(L"/*.[^*?]+");
	...
	if (auto match = regexFilterExt.MatchHead(filterItem->filter))
	{
		if (match->Result().Length() == filterItem->filter.Length())
		{
			filterItem->defaultExtension = filterItem->filter.Right(filterItem->filter.Length() - 2);
		}
	}

with explicit validation against the rule described in the task:

- Only derive a filter-default extension when the wildcard is exactly `*.<ext>`.
- `<ext>` must be non-empty.
- `<ext>` must contain no `*`, `?`, or `;`.
  - This guarantees `*.*` never produces `"*"` (which would lead to appending `".*"`).
  - This guarantees `*.bmp;*.png;*.jpg` never produces a single extension override.

Minimal, local helper (lambda) to add near the start of filter parsing:

	auto GetFilterDefaultExtension = [](const WString& wildcard) -> Nullable<WString>
	{
		if (wildcard.Length() < 3) return {};
		if (wildcard[0] != L'*' || wildcard[1] != L'.') return {};
		auto ext = wildcard.Right(wildcard.Length() - 2);
		for (vint i = 0; i < ext.Length(); i++)
		{
			switch (ext[i])
			{
			case L'*':
			case L'?':
			case L';':
				return {};
			}
		}
		return ext;
	};

Then when creating `filterItem`:

	filterItem->defaultExtension = GetFilterDefaultExtension(filterItem->filter);

Also remove `regexFilterExt` since it becomes unused.

### Why this is necessary

- The appending logic in `FileDialogViewModel::TryConfirm` treats `selectedFilter->GetDefaultExtension()` as a strong override (`extensionFromFilter == true`), and will append `"." + extension` even if the user typed a name without a dot.
- While the existing regex + full-length check likely already rejects many invalid patterns (e.g. `*.*` won’t match `[^*?]+`), the contract is implicit; this change makes the `*.<ext>` rule and `;` exclusion explicit and future-proof.
- Supporting `;` means one filter can represent multiple patterns; such a filter must not pretend to provide a single overriding extension.

## STEP 2: Confirm multi-wildcard matching behavior (";" in wildcard)

### What to change

This step is verification-only: no code change is expected if the current wildcard-to-regex conversion already treats `;` as alternation.

- Treat `;` as an OR separator when building `filterItem->regexFilter`.
- Ensure `filterItem->defaultExtension` stays empty for any wildcard containing `;` (guaranteed by STEP 1 validation).

If any subtle bug is found during testing (e.g. missing grouping), the safe conceptual model is:

- Split `filterItem->filter` by `;` into N wildcard patterns.
- Convert each wildcard pattern into a regex fragment.
- Join fragments with `|` and wrap in `^( ... )$`.

### Why this is necessary

- Work item 2 requires `*.bmp;*.png;*.jpg` to actually filter and accept all three extensions.
- Even if the implementation already treats `;` as `|`, this step ensures the behavior is explicitly tested and protected.

## STEP 3: Add new unit tests for "*.*" and multi-wildcard filters

### What to change

Add a new test category in `Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp` (keep existing categories unchanged):

- `TEST_CATEGORY(L"File Dialog Filter Extensions")`

Add two local helpers dedicated to this category to avoid breaking existing expected sequences:

1) A new mock filesystem root with image files:

	Ptr<FileItemMock> CreateFileItemRoot_WithImages()
	{
		auto root = CreateFileItemRoot();
		AddFile(root, L"pic.bmp");
		AddFile(root, L"pic.png");
		AddFile(root, L"pic.jpg");
		return root;
	}

2) A new installer that uses this root (only for tests needing images):

	GacUIUnitTest_Installer CreateInstallerWithImages(Ptr<FileSystemMock>& fsMock)
	{
		return {
			.initialize = [&]() { fsMock = Ptr(new FileSystemMock(CreateFileItemRoot_WithImages())); },
			.finalize = [&]() { fsMock = {}; }
		};
	}

Add a dedicated resource string for this category so it does not change existing tests’ filter definitions:

- Open dialog filter includes an explicit `*.*` and an image multi-wildcard entry.
- The click handler sets `DefaultExtension` explicitly (since in tests it is currently not configured in XML).
- Use 2-space indentation in any embedded XML resource string literals.
- Put new helpers in the same namespace/style as existing helpers in this file (e.g. `gacui_unittest_template`).
- Choose stable `TEST_CATEGORY` / `TEST_CASE` names to keep log/snapshot paths stable.
- For any error prompt in UI-driven tests, close the GUI message box by clicking `OK` (e.g. `PressMessageOK(protocol)`).

Example resource fragment to use in a new `const auto resourceFileDialogs_FilterExtensions = LR"GacUISrc( ... )GacUISrc";`:

	<OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*.*|Text Files (*.txt)|*.txt|Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg" Directory=""/>
	<SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*.*|Text Files (*.txt)|*.txt|Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg" Directory=""/>

In each button’s click script, set default extension before calling `ShowDialog()`:

	dialogOpen.DefaultExtension = "txt";
	dialogSave.DefaultExtension = "txt";

### Test cases to add

#### Test A (work item 1): "*.*" must not force ".*" and should fall back to dialog default extension

- Use `CreateInstaller(fsMock)` (no images needed).
- Open with `FileDialogFileMustExist`.
- Choose filter index 0 (`All Files (*.*)` with wildcard `*.*`).
- Type `root2` and press Open.
- Expect confirmation success and selected path is `/root2.txt`.

This test guards against a regression where `*.*` (or any invalid wildcard) accidentally produces a filter-derived default extension (e.g. `"*"`), leading to a wrong normalized path like `root2.*` and a spurious error dialog.

#### Test B (work item 2): multi-wildcard filter lists all supported extensions

- Use `CreateInstallerWithImages(fsMock)`.
- Open with default options.
- Choose filter index 2 (`Image Files`).
- Assert the data grid items are exactly:

	A
	B
	pic.bmp
	pic.jpg
	pic.png

(Directories must remain visible regardless of filter; only files should be filtered.)

#### Test C (work item 2): multi-wildcard filter must not invent an extension for open dialog

- Use `CreateInstallerWithImages(fsMock)`.
- Open with `FileDialogFileMustExist`.
- Set `dialogOpen.DefaultExtension = "txt"` in the click handler.
- Choose filter index 2 (`Image Files`).
- Type `pic` and press Open.
  - Expected: fail with message dialog (because it should normalize to `/pic.txt`, which does not exist).
- Dismiss message OK.
- Type `pic.png` and press Open.
  - Expected: success selecting `/pic.png`.

This protects against an incorrect implementation that appends one of the image extensions (e.g. `.bmp`) for a multi-wildcard filter.

#### Test D (work item 3): analogous coverage for save dialog under multi-wildcard

- Use `CreateInstallerWithImages(fsMock)`.
- Save dialog with `FileDialogPromptCreateFile` (keep it to ensure the option does not change save behavior).
- Set `dialogSave.DefaultExtension = "txt"` in the click handler.
- Choose filter index 2 (`Image Files`).
- Type `newfile` and press Save.
  - Expected: it should normalize to `/newfile.txt` (dialog default extension applies; filter does not override).
  - Expected: no create-file prompt appears for save dialogs (the create-file prompt is guarded by `!selectToSave && promptCreateFile`); dialog returns `/newfile.txt` directly.

### Why this is necessary

- These tests directly cover:
  - Work item 1: invalid `*.*` must not produce a filter-derived extension override.
  - Work item 2: `;` works as multiple patterns for filtering, without producing a single extension override.
  - Work item 3: existing “typed selection” behaviors are mirrored for multi-wildcard filters to prevent regression (e.g. accidentally choosing the first extension).

# !!!FINISHED!!!
