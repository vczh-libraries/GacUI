# !!!TASK!!!

# PROBLEM DESCRIPTION
## TASK No.5: Test RemoveStyleName, SummarizeStyleName, and RenameStyle

Write test cases for `RemoveStyleName`, `SummarizeStyleName`, and `RenameStyle`, each under their own `TEST_CATEGORY` hierarchy. This task covers the following TEST_CATEGORYs:
- `TEST_CATEGORY(L"RemoveStyleName")` / `TEST_CATEGORY(L"SingleParagraph")`
- `TEST_CATEGORY(L"RemoveStyleName")` / `TEST_CATEGORY(L"MultiParagraph")`
- `TEST_CATEGORY(L"SummarizeStyleName")`
- `TEST_CATEGORY(L"RenameStyle")`

### what to be done

**Under `TEST_CATEGORY(L"RemoveStyleName")` / `TEST_CATEGORY(L"SingleParagraph")`:**

- **Test: Remove style from a fully styled range** — Apply a style to `[0,10)`, then `RemoveStyleName(0,10)`. Verify `SummarizeStyleName(0,10)` returns null.
- **Test: Remove style from a partial range** — Apply a style to `[0,10)`, then `RemoveStyleName(3,7)`. Verify `SummarizeStyleName(0,3)` still returns the style name, `SummarizeStyleName(3,7)` returns null, `SummarizeStyleName(7,10)` still returns the style name.
- **Test: Remove style when no style is applied** — Call `RemoveStyleName` on text without any style name applied; verify no crash and `SummarizeStyleName` still returns null.
- **Test: Remove style with overlapping styles** — Apply `"StyleA"` to `[0,5)` and `"StyleB"` to `[5,10)`, then `RemoveStyleName(3,7)`. Verify style names are removed in the overlapping region.

**Under `TEST_CATEGORY(L"RemoveStyleName")` / `TEST_CATEGORY(L"MultiParagraph")`:**

- **Test: Remove style across paragraphs** — Apply a style across multiple paragraphs, then `RemoveStyleName` across the same range. Verify all paragraphs no longer have the style, and verify `SummarizeStyleName(TextPos(...), TextPos(...))` is null over the removed multi-paragraph range.
- **Test: Partial removal across paragraphs** — Apply a style across 3 paragraphs, then remove it from a range that spans the middle of paragraph 1 to the middle of paragraph 3. Verify the edges still retain the style while the middle is removed, and verify `SummarizeStyleName(TextPos(...), TextPos(...))` is null over the removed multi-paragraph range.

**Under `TEST_CATEGORY(L"SummarizeStyleName")`:**

- **Test: Returns null for mixed styles** — Apply `"StyleA"` to `[0,5)` and `"StyleB"` to `[5,10)`, verify `SummarizeStyleName(0,10)` returns null (Nullable without value).
- **Test: Returns null for partially styled text in the range** — Apply `"MyStyle"` to `[0,5)` only, verify `SummarizeStyleName(0,10)` returns null because `[5,10)` has no style name.
- **Test: Returns null for text with no style at all** — Load text with no style applied, verify `SummarizeStyleName(0,10)` returns null.
- **Test: Reversed begin/end** — Verify that calling `SummarizeStyleName` with `begin > end` still works correctly (the implementation swaps them, confirmed in `GuiDocumentCommonInterface.cpp`).

**Under `TEST_CATEGORY(L"RenameStyle")`:**

- **Test: Rename a registered style** — Register `"OldName"`, apply it to text, then `RenameStyle("OldName", "NewName")`. Verify:
  - `SummarizeStyleName` returns `"NewName"` for the affected range.
  - The document's styles dictionary no longer contains `"OldName"` and now contains `"NewName"`.
- **Test: Rename updates parent references** — Register `"Parent"` and `"Child"` with `parentStyleName="Parent"`. Apply `"Child"` to text. Rename `"Parent"` to `"NewParent"`. Verify `"Child"` now has `parentStyleName="NewParent"` (confirmed by `DocumentModel::RenameStyle` implementation in `GuiDocument_Edit.cpp` lines 535-540).
- **Test: Rename a style applied to multiple ranges** — Apply the same style name to discontiguous ranges, then rename. Verify all ranges now summarize as the new name.
- **Test: Rename to an existing style name fails** — Register both `"StyleA"` and `"StyleB"`, try to rename `"StyleA"` to `"StyleB"`. The `DocumentModel::RenameStyle` returns `false` when the new name already exists. Verify nothing changes.
- **Test: Rename a non-existent style** — Try to rename a style name not in the styles dictionary. `DocumentModel::RenameStyle` returns `false` when the old name is not found. Verify nothing changes.
- **Test: Rename across multiple paragraphs** — Apply a style to text spanning multiple paragraphs, rename, verify all paragraphs are updated.

### rationale

- These four TEST_CATEGORYs (`RemoveStyleName/SingleParagraph`, `RemoveStyleName/MultiParagraph`, `SummarizeStyleName`, `RenameStyle`) are grouped into a single task because they are all non-undo/redo functional tests for the remaining style operations. Each TEST_CATEGORY retains its own identity and scope.
- `RemoveStyleName` uses `RemoveStyleNameVisitor` which unwraps `DocumentStyleApplicationRun` containers from the run tree (see `GuiDocumentEditor_RemoveContainer.cpp`). Testing partial removal is critical as it involves splitting run containers.
- `SummarizeStyleName` is the primary query function for style name information. The implementation in `GuiDocumentEditor_SummerizeStyle.cpp` uses a visitor pattern with `assignedStyleName` and `currentStyleName` tracking to determine consistency. The begin/end swap behavior is confirmed in `GuiDocumentCommonInterface.cpp`.
- `RenameStyle` has unique behavior: it modifies both the styles dictionary (`styles.Remove + styles.Add`) AND the text runs (`ReplaceStyleName` on all paragraphs), AND updates parent references of other styles. The `DocumentModel::RenameStyle` implementation (in `GuiDocument_Edit.cpp` line 523-544) shows explicit failure conditions.

# UPDATES

# INSIGHTS AND REASONING

## Goal / scope

This design covers **TASK No.5** only: add new unit tests to `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp` for:
- `vl::presentation::controls::GuiDocumentCommonInterface::RemoveStyleName`
- `vl::presentation::controls::GuiDocumentCommonInterface::SummarizeStyleName`
- `vl::presentation::controls::GuiDocumentCommonInterface::RenameStyle`

Out of scope:
- Undo/redo verification (TASK No.6).
- Restoring full UnitTest run (TASK No.7).
- Any production-code change.

## Evidence / behavior constraints from source code

### RemoveStyleName removes `DocumentStyleApplicationRun` containers

- Model-side edit is `vl::presentation::DocumentModel::RemoveStyleName(TextPos, TextPos)` which calls `EditContainer(...)` and then `document_editor::RemoveStyleName(...)` (`Source\Resources\GuiDocument_Edit.cpp`, around `DocumentModel::RemoveStyleName`).
- `document_editor::RemoveStyleName(...)` uses `RemoveStyleNameVisitor`, which overrides `Visit(DocumentStyleApplicationRun*)` to `RemoveContainer(run)`; i.e. it unwraps the container and splices its child runs into the parent (`Source\Resources\GuiDocumentEditor_RemoveContainer.cpp`).

Implication for tests:
- The externally observable outcome should be: affected ranges no longer summarize to a style name, while unaffected ranges still do.
- We don’t need to inspect the run tree directly; `SummarizeStyleName` provides a stable, user-facing query for correctness.

### SummarizeStyleName semantics and normalization

- UI-facing `GuiDocumentCommonInterface::SummarizeStyleName` normalizes reversed ranges by swapping when `begin > end` (`Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp:1276-1285`).
- Model-side `DocumentModel::SummarizeStyleName` returns empty on `begin == end` and on invalid ranges (`CheckEditRange` failure) (`Source\Resources\GuiDocument_Edit.cpp:638+`).
- Paragraph-level aggregation uses `document_editor::SummarizeStyleName` backed by `SummarizeStyleNameVisitor` which tracks `currentStyleName` in `DocumentStyleApplicationRun` and compares content runs (`Source\Resources\GuiDocumentEditor_SummerizeStyle.cpp`).

Implication for tests:
- Assertions for "null" should be `TEST_ASSERT(!summary)` (Nullable without value), not string equality.
- Include a single explicit reversed-begin/end test in the `SummarizeStyleName` category to lock in the UI behavior.

### RenameStyle: dictionary + parent references + run replacement

- `DocumentModel::RenameStyle` returns `false` when `oldStyleName` doesn’t exist or when `newStyleName` already exists; on success it:
  - Removes+adds the style key in `document->styles`.
  - Updates `parentStyleName` in all styles that referenced the old name.
  - Applies `ReplaceStyleName(...)` to all paragraphs to update `DocumentStyleApplicationRun` (and hyperlink style names) (`Source\Resources\GuiDocument_Edit.cpp:527-549`, `Source\Resources\GuiDocumentEditor_ReplaceStyleName.cpp`).
- Note: `GuiDocumentCommonInterface::RenameStyle` is `void` and does not expose the boolean success value; tests must verify success/failure by observing document state and `SummarizeStyleName` output (`Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.h/.cpp`).

Implication for tests:
- For success: verify both the *run-level* style name (via `SummarizeStyleName`) and the *model-level* style registry (`document->styles`).
- For failure: verify "nothing changes" by checking both registry keys and existing summarized style name remain the same.

## Proposed changes (test-only, high-level)

### Where to add tests

Update existing file:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Add these new categories under `TEST_CATEGORY(L"Styles")`, as siblings to `EditStyleName`:
- `TEST_CATEGORY(L"RemoveStyleName")`
  - `TEST_CATEGORY(L"SingleParagraph")`
  - `TEST_CATEGORY(L"MultiParagraph")`
- `TEST_CATEGORY(L"SummarizeStyleName")`
- `TEST_CATEGORY(L"RenameStyle")`

### Harness / helpers

Reuse the already-established harness in this file:
- `resource_DocumentViewer` (DocumentViewer, `EditMode="Editable"`).
- Two idle frames (`Init`, `Verify`) via `GacUIUnitTest_SetGuiMainProxy`.
- Existing helper functions:
  - `RegisterStyle(textBox, ...)` (adds to `textBox->GetDocument()->styles`).
  - `SummarizeName(...)` wrappers (single-row only; cross-paragraph checks must call `textBox->SummarizeStyleName(TextPos(...), TextPos(...))` directly).

Do not use `SummarizeStyle` (per Scrum update: it only summarizes direct property edits, not style names).

### Test naming / log paths

Follow the current convention used in this file:
- `Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_*`
- `Controls/Editor/Features/Styles/RemoveStyleName/MultiParagraph_*`
- `Controls/Editor/Features/Styles/SummarizeStyleName/*`
- `Controls/Editor/Features/Styles/RenameStyle/*`

Concrete `TEST_CASE` names (suffixes) to keep the file navigable:
- `RemoveStyleName/SingleParagraph`: `FullRange`, `PartialRange`, `NoStyle`, `OverlappingStyles`
- `RemoveStyleName/MultiParagraph`: `SameRange`, `EdgeRetention`
- `SummarizeStyleName`: `MixedStyles`, `PartiallyStyled`, `NoStyle`, `ReversedRange`
- `RenameStyle`: `Success`, `ParentReference`, `MultipleRanges`, `ExistingNameFails`, `NonExistentFails`, `MultiParagraph`

## Test case matrix (what to assert)

### RemoveStyleName / SingleParagraph

Start from `LoadTextAndClearUndoRedo(L"0123456789")`.

- Fully styled then removed:
  - Apply `EditStyleName([0,10), StyleA)` then `RemoveStyleName([0,10))`.
  - Verify `!SummarizeName(textBox, 0, 10)`.

- Partial removal:
  - Apply `StyleA` to `[0,10)` then remove `[3,7)`.
  - Verify `[0,3)` summarizes `StyleA`, `[3,7)` summarizes null, `[7,10)` summarizes `StyleA`.

- Remove when nothing is styled:
  - Call `RemoveStyleName([0,10))` on clean text.
  - Verify still null and no crash.

- Overlapping styles removal:
  - Apply `StyleA` to `[0,5)`, `StyleB` to `[5,10)`; remove `[3,7)`.
  - Verify `[0,3)` is `StyleA`, `[3,7)` is null, `[7,10)` is `StyleB`.

### RemoveStyleName / MultiParagraph

Start from 3 paragraphs (10 chars each):
`L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST"`.

- Remove same range that was styled across paragraphs:
  - Apply `EditStyleName(TextPos(0,5), TextPos(2,5), MyStyle)`.
  - Remove the same range.
  - Verify per-paragraph segments are all unstyled.
  - Also verify `!textBox->SummarizeStyleName(TextPos(0,5), TextPos(2,5))`.

- Partial removal across paragraphs (edge retention):
  - Apply `EditStyleName(TextPos(0,0), TextPos(2,10), MyStyle)`.
  - Remove `TextPos(0,5)` to `TextPos(2,5)`.
  - Verify paragraph 0 head `[0,5)` is styled, mid portions are unstyled, paragraph 2 tail `[5,10)` is styled.
  - Also verify `!textBox->SummarizeStyleName(TextPos(0,5), TextPos(2,5))`.

### SummarizeStyleName

These tests are query-focused; they can reuse edit helpers but should primarily assert returned Nullable values.

- Mixed styles returns null.
- Partially styled (styled + unstyled) returns null.
- Entirely unstyled returns null.
- Reversed begin/end normalization: call `textBox->SummarizeStyleName(TextPos(0,5), TextPos(0,2))` and verify it matches the forward call.

### RenameStyle

All rename tests should validate both:
- `SummarizeStyleName` outputs on affected text ranges.
- `textBox->GetDocument()->styles` keys/values and child `parentStyleName` where relevant.

- Success path:
  - Register `OldName`, apply to text, rename to `NewName`.
  - Verify summarized name becomes `NewName` and registry key moved.

- Parent reference update:
  - Register `Parent` and `Child(parent=Parent)`, rename `Parent->NewParent`.
  - Verify `textBox->GetDocument()->styles[L"Child"]->parentStyleName == L"NewParent"`.

- Multiple ranges:
  - Apply the same style name to two discontiguous ranges; rename; verify both ranges updated.

- Rename-to-existing fails (observable, not via return):
  - Register both names; apply the old name; rename old->existing.
  - Verify summary stays old and registry keys unchanged.

- Rename non-existent fails:
  - Rename missing style; verify no new key added and existing summaries unchanged.

- Multi-paragraph rename:
  - Apply style name across paragraphs; rename; verify each paragraph summarizes the new name.

## Risks / guardrails

- `GuiDocumentCommonInterface::RenameStyle` is `void` and unconditionally enqueues an undo/redo rename step, including when the rename fails (old name missing or new name already exists); TASK No.5 should avoid invoking undo/redo and should verify success/failure via document state only.
  - Note for future planning (TASK No.6): the undo stack can be polluted by failed renames even if no document state changes.
- Use `TEST_ASSERT(!nullable)` / `TEST_ASSERT(nullable)` consistently; do not compare null summaries to empty strings.
- Avoid empty-range summaries (`begin == end`) because the model explicitly returns empty in that case.
- Caveat for cross-paragraph summarization expectations: there is a known quirk in the production multi-paragraph summarize path (middle-paragraph branch uses `end.column`); keep paragraphs uniform length when asserting cross-paragraph behavior (fixing production code is out of scope for TASK No.5).

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# !!!FINISHED!!!
