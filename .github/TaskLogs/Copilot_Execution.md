# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Run UnitTest project `UnitTest` (Debug|x64).
  - During iteration: run UnitTest with command arguments `/F:TestApplication_Dialog_File.cpp` to only execute `Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp`.

# EXECUTION PLAN

## STEP 1: Fix default-extension derivation for file dialog filters [DONE]

### Edit file

- `Source\Utilities\FakeServices\GuiFakeDialogServiceBase_FileDialog.cpp`
  - In `vl::presentation::FakeDialogServiceBase::ShowFileDialog`.

### Replace default-extension extraction

Replace the current regex-based default-extension extraction:

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

Minimal, local helper (lambda) to add near the start of filter parsing:

	auto GetFilterDefaultExtension = [](const WString& wildcard) -> Nullable<WString>
	{
		// Accept only "*.<ext>" where <ext> is non-empty and contains no '*', '?', or ';'.
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
		return Nullable<WString>(ext);
	};

Then when creating `filterItem`:

	filterItem->defaultExtension = GetFilterDefaultExtension(filterItem->filter);

Also remove `regexFilterExt` since it becomes unused.
Do NOT remove `regexWildcard`, it is still used for wildcard-to-regex conversion.

## STEP 2: Confirm multi-wildcard matching behavior (";" in wildcard) [DONE]

Verification-only (no code change expected if the current wildcard-to-regex conversion already treats `;` as alternation):

- Treat `;` as an OR separator when building `filterItem->regexFilter`.
- Ensure `filterItem->defaultExtension` stays empty for any wildcard containing `;` (guaranteed by STEP 1 validation).

If any subtle bug is found during testing (e.g. missing grouping), the safe conceptual model is:

- Split `filterItem->filter` by `;` into N wildcard patterns.
- Convert each wildcard pattern into a regex fragment.
- Join fragments with `|` and wrap in `^( ... )$`.

## STEP 3: Add new unit tests for "*.*" and multi-wildcard filters [DONE]

### Edit file

- `Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp`

### Add new test category and isolated helpers/resources

Add a new test category (keep existing categories unchanged):

- `TEST_CATEGORY(L"File Dialog Filter Extensions")`

Add two local helpers dedicated to this category to avoid breaking existing expected sequences:

	Ptr<FileItemMock> CreateFileItemRoot_WithImages()
	{
		auto root = CreateFileItemRoot();
		AddFile(root, L"pic.bmp");
		AddFile(root, L"pic.png");
		AddFile(root, L"pic.jpg");
		return root;
	}

	GacUIUnitTest_Installer CreateInstallerWithImages(Ptr<FileSystemMock>& fsMock)
	{
		return {
			.initialize = [&]() { fsMock = Ptr(new FileSystemMock(CreateFileItemRoot_WithImages())); },
			.finalize = [&]() { fsMock = {}; }
		};
	}

Add a dedicated resource string for this category so it does not change existing tests’ filter definitions.

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

#### Test B (work item 2): multi-wildcard filter lists all supported extensions

- Use `CreateInstallerWithImages(fsMock)`.
- Open with default options.
- Choose filter index 2 (`Image Files`).
- Assert the data grid items are exactly (in order) by collecting via the established item-provider pattern (e.g. `GetItemProvider()->GetTextValue(i)`) and comparing to an expected `const wchar_t* expected[]` sequence:

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

#### Test D (work item 3): analogous coverage for save dialog under multi-wildcard

- Use `CreateInstallerWithImages(fsMock)`.
- Save dialog with `FileDialogPromptCreateFile` (keep it to ensure the option does not change save behavior).
- Set `dialogSave.DefaultExtension = "txt"` in the click handler.
- Choose filter index 2 (`Image Files`).
- Type `newfile` and press Save.
  - Expected: it should normalize to `/newfile.txt` (dialog default extension applies; filter does not override).
  - Expected: assert using `GetFileName()` (not `GetFileNames()`).
  - Expected: no create-file prompt appears for save dialogs (the create-file prompt is guarded by `!selectToSave && promptCreateFile`); dialog returns `/newfile.txt` directly.

### Cross-cutting rules (all dialog UI tests)

- Do not nest `protocol->OnNextIdleFrame(...)` calls.
- Re-find windows/controls by name in each frame.
- Ensure every frame produces an observable UI change.

### Embedded XML / Workflow script hygiene

- Use 2-space indentation inside embedded XML literals.
- In embedded Workflow scripts, ensure `var name : type` uses spaces around `:`.
- Ensure `TEST_CATEGORY` / `TEST_CASE` blocks close with `});`.

# FIXING ATTEMPTS

## Fixing attempt No.1

- Why the original change did not work:
  - The "*.* uses dialog default extension" test chose filter index 0 in the first dialog frame, which is already the default, so no UI change occurred and the test framework raised a "no UI update" error.
- What I need to do:
  - Insert a frame that changes the filter selection (switch to Text Files first, then back to All Files) before typing and confirming the file name.
- Why this should solve the build break:
  - Each `OnNextIdleFrame` will now trigger a visible UI change, satisfying the framework requirement and allowing the test to proceed.

# !!!FINISHED!!!


# !!!VERIFIED!!!
