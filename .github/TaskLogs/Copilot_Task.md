# !!!TASK!!!

# PROBLEM DESCRIPTION
vl::presentation::FakeDialogServiceBase::ShowFileDialog currently in filter we can only say:
All Files (*.*)|*.*|Text Files (*.txt)|*.txt
And in the file dialog you can see a combo box for filtering file type with two item:
- `All Files (*.*)`, matching file name against wildcard `*.*.
- Another one for text files.
We can clearly see that, if we count from zero, even ones are label and odd ones are wildcards.
And when one select text file, and type file name ABC, because ABC does not match the filter, it becomes ABC.txt and search again. This is already implemented. The rule is that, if the wildcard begins from "*" and "." and a series of text without "*" or "?", it generates a file extension to override the default file extension that passed to the `ShowFileDialog` function.

Here come to work item 1: this part is not properly implemented, no validation is performed on the wildcard, causing *.* to generate a default extension ".*" and it is clearly wrong. You need to create a test case to repro and fix it, the correct behavior is that, when *.* is selected, the default file extension takes place, and when one type an unmatching file name ABC, it does not become ABC.*. You can check out existing test cases to understand how this feature is tested before.

Another task is that, I would like to support ";" in wildcard. for example, `*.bmp;*.png;*.jpg` now becomes a filter that accept any of the following wildcard:
- *.bmp
- *.png
- *.jpg
And clearly such filter does not generate a default extension name to override the one passed to ShowFileDialog. You will need more test cases for this new feature.

Here come to work item 2: implement the ";" behavior. Not only test against default extension name, but also the filter actually accept 3 kinds of files.

Pay attention to the `normalized` lambda in `TryConfirm`, and other code in this function. The assumption before is that a filter maps to one extension name, and now this assumption does not exist.

Here come to work item 3: you need to scan existing test cases against file dialog, if anything is about filtering and appending default extension name, you need to consider about, is it necessary to perform a similar test with multiple-extension filter, add them if justified to be necessary.

You are going to add a new test category for the above work items in
Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp
Here are also where every other test cases about the file dialog stay.

To speed up testing, you can make UnitTest project only run TestApplication_Dialog_File.cpp

# UPDATES

# INSIGHTS AND REASONING

## Where the behavior lives (evidence)

### Filter parsing and wildcard->regex conversion

- Filter parsing is implemented in `Source\Utilities\FakeServices\GuiFakeDialogServiceBase_FileDialog.cpp` inside `vl::presentation::FakeDialogServiceBase::ShowFileDialog`.
- The filter string is split into `(label|wildcard)` pairs by scanning for `'|'`.
- For each wildcard, a `FileDialogFilter` is created with:
  - `filterItem->filter`: raw wildcard string (e.g. `*.txt`).
  - `filterItem->defaultExtension`: derived from wildcard when it matches `*.<something>`.
  - `filterItem->regexFilter`: a `vl::regex::Regex` built from the wildcard to filter file names.

Notable implementation details:

- Default-extension extraction currently uses:
  - `Regex regexFilterExt(L"/*.[^*?]+");`
  - and if the entire wildcard matches, it sets `defaultExtension = filter.Right(filter.Length() - 2)`.
- Wildcard-to-regex currently treats `';'` as an alternation marker when building the regex:
  - `Regex regexWildcard(L"[*?;]");`
  - on `';'` it writes `"|"` into the regex.

### Extension appending during confirmation (TryConfirm)

- Extension appending is implemented in `FileDialogViewModel::TryConfirm` (same `.cpp`).
- Logic summary:
  - `extensionFromFilter = selectedFilter->GetDefaultExtension()` (nullable).
  - If no extension from filter, fall back to `defaultExtension` passed to `ShowFileDialog`.
  - `normalized(path)` appends `"." + extension` when appropriate.

The key risk area for this task is that `TryConfirm` assumes `selectedFilter->GetDefaultExtension()` (when present) is a single meaningful extension that should override the dialog default; this assumption becomes invalid for `*.*` and for multi-wildcard filters like `*.bmp;*.png;*.jpg`.

## Root cause hypotheses

### Work item 1 (*.* produces " .*")

The problem statement describes an observed wrong behavior (`*.*` yields a filter-default extension `"*"`, producing appended `".*"`). Before treating this as a confirmed existing bug, verify the current behavior in this repo: with `vl::regex::Regex` and pattern `L"/*.[^*?]+"`, `*.*` likely **does not** match the “single extension” extractor today (the trailing `*` should make `MatchHead` fail). Even if it is not reproducible, switching away from regex-based extraction is still justified for clarity, correctness, and to support the new `';'` contract.

Design decision: replace “derive default extension” with explicit string validation instead of relying on regex matching.

### Work item 2 (support ';' as multi-wildcard)

The current wildcard-to-regex conversion already maps `';'` to `"|"`, which matches the intended “OR across patterns” semantics for listing/filtering.

However, default-extension extraction and `TryConfirm` extension-appending logic must treat multi-wildcard filters as **not providing** a filter-specific extension override.

## Proposed changes (high-level)

### 1) Make default-extension extraction validate the wildcard

In `FakeDialogServiceBase::ShowFileDialog`, replace the `regexFilterExt`-based logic with a small helper (local lambda is fine) that:

- Takes `filterItem->filter` (raw wildcard string).
- Returns `Nullable<WString>` default extension **only when** the wildcard is exactly a single `*.<ext>` pattern where:
  - It starts with `"*."`.
  - The suffix after `"*."` is non-empty.
  - The suffix contains **no** `'*'`, `'?'`, or `';'`.

Defensive-only note: explicitly rejecting the special case suffix `"*"` (i.e. wildcard `"*.*"`) is redundant if the “no `*`/`?`/`;` in suffix” rule is enforced, but keeping it as an explicit guard is acceptable.

Rationale:

- This implements the rule described in the problem statement (“series of text without `*` or `?`”), and extends it to reject `';'` because it indicates multiple patterns.
- It guarantees `*.*` will not produce a filter default extension, so `TryConfirm` will fall back to the dialog’s `defaultExtension` instead of ever appending `".*"`.

### 2) Define ';' semantics precisely for matching and for extension override

Matching contract:

- `"*.bmp;*.png;*.jpg"` means OR across the listed wildcards.
- Implementation can keep the current wildcard-to-regex conversion (since it already maps `';'` to `"|"`), but the intended conceptual model is:
  - Split by `';'` into patterns, convert each pattern, then join with `"|"`.

Extension-override contract:

- Any wildcard containing `';'` yields **no** filter-derived default extension override.
  - Therefore `selectedFilter->GetDefaultExtension()` must be null for multi-wildcard filters.
  - `TryConfirm` then uses the dialog-level `DefaultExtension` (if configured) exactly as it does today.

### 3) Adjust TryConfirm’s assumptions (no longer “one filter => one extension”)

Keep the current `normalized` behavior but ensure:

- `extensionFromFilter` is only true when the selected filter has a validated single-extension default.
- For multi-wildcard filters, `extensionFromFilter` stays false, so `normalized` does not try to enforce a specific filter suffix.

No new behavior is proposed beyond making the default-extension extractor explicit and ensuring multi-wildcard filters don’t pretend to have a single extension.

## Test design (new category in TestApplication_Dialog_File.cpp)

All file-dialog tests live in `Test\GacUISrc\UnitTest\TestApplication_Dialog_File.cpp`.

Critical fixture constraints (to avoid breaking existing tests):

- Do **not** modify any existing shared XML resource’s “All Files” filter. It currently uses `All Files (*.*)|*` (wildcard `*`, not `*.*`), and changing it to `*.*` would change listing behavior (e.g. hiding extensionless files like `README`) and break existing listing assertions.
- Create a **new, dedicated XML resource** for this new test category, and put `DefaultExtension="txt"` only there.
- Keep mock filesystem changes isolated: do not add `pic.bmp/pic.png/pic.jpg` to a shared `CreateFileItemRoot()` that other tests depend on. Use a new helper (e.g. `CreateFileItemRootWithImages()`), or a parameterized variant, so existing row-sequence assertions remain stable.

### New category: filter extension / multi-wildcard

Add a new `TEST_CATEGORY` (sibling to existing categories) focused on these work items.

#### Test A (work item 1): "*.*" must not yield a filter-derived default extension

- Use the **new dedicated resource XML** so that:
  - Filter includes `All Files (*.*)|*.*` (not `|*`).
  - Dialog `DefaultExtension` is set to `"txt"`.
- Scenario:
  1. Open dialog (FileMustExist).
  2. Select `All Files (*.*)` filter.
  3. Type `root2`.
  4. Confirm (Open).
- Expected:
  - Confirmation succeeds and selected file is `/root2.txt`.

Add a branch case in the same category:

- Type `root2.xyz` (already has a dot).
- Expected: confirmation must not append anything; with `FileMustExist`, it should fail cleanly if the file is absent.

#### Test B (work item 2): listing under multi-wildcard filter accepts all listed extensions

- Using the isolated mock filesystem helper with images.
- Add a filter entry:
  - `Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg`.
- Scenario:
  1. Open dialog.
  2. Choose the image filter.
  3. Assert the full, stable visible sequence using existing grid helpers (e.g. collect texts via `GuiBindableDataGrid::GetItemProvider()->GetTextValue(i)`), not partial membership checks.

#### Test C (work item 2): typed selection under multi-wildcard does not invent an extension (open dialog)

- Still using the image filter and `FileMustExist`.
- Scenario:
  1. Choose image filter.
  2. Type `pic` (no extension) and confirm -> expect error (file not exist).
  3. Dismiss the GUI-rendered message box via the fake-dialog OK path (click the `OK` button).
  4. Type `pic.png` and confirm -> expect success selecting `/pic.png`.
- Expected:
  - The first attempt must fail, proving the filter does not force a single default extension.

#### Test D (work item 2): typed selection under multi-wildcard does not invent an extension (save dialog)

- Add at least one save-dialog scenario with `DefaultExtension="txt"`.
- Under `*.bmp;*.png;*.jpg`, typing a name without extension must not “invent” a single image extension; it should use the dialog-level default extension if any defaulting occurs.

### Work item 3: extend existing “typed selection” patterns to multi-wildcard where justified

Existing tests already validate single-extension behaviors (listing + filter-derived extension appending).

Justified analogous coverage for multi-wildcard:

- The open/save “does not invent an extension” tests guard against regressions where the implementation accidentally picks the first extension and appends it.

## Verification strategy (high-level)

- Restrict UnitTest to only run `TestApplication_Dialog_File.cpp` during iteration (via existing `/F:<file>` mechanism used elsewhere in this repo’s workflow).
- Ensure UI protocol frames follow established rules (no nested `OnNextIdleFrame`, re-find controls each frame, and each frame causes an observable UI change).
- Run UnitTest after implementing the fix and new tests.

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Run UnitTest project `UnitTest` (Debug|x64), during iteration with command arguments `/F:TestApplication_Dialog_File.cpp`.

# !!!FINISHED!!!
