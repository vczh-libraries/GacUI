# !!!SCRUM!!!

# DESIGN REQUEST

You ultimate goal is to make a plan about finishing TestControls_Editor_Styles.cpp, which is still an empty test file. In order to speed up you could let UnitTest project to only run this file.

You are going to test against some methods in `GuiDocumentCommonInterface` and it is fine only to test against `GuiDocumentViewer` with `Editable` view. Put all test cases under path Controls/Editor/Features/Styles:
- EditStyleName, RemoveStyleName for editing text
- RenameStyle for editing text as well as the registered style
- SummarizeStyleName, which returns non-null string if the target text is completely assigned with the same style name
You need to also test redo/undo feature along with these functions. Make sure everything work.
You can categorize all your test cases properly with test categories.

A little bit introduction to styles. A style is a name associated with multiple properties (bold, italic, etc). There are functions to change properties of text directly, but they are not related to this work.

To make styles available in an editor, you need to assign the document (already with desired style registered) to the editor control. A style control all properties for text, and styles can inherit with each other, styles could also only contain partial propertie values. Unassigned properties inherit values from the parent style prior than properties assigned to actual text.

When assigning a style name to a piece of text without the style registered, no error happens instead it fallback to default.
Carefully design your test to cover everything about style, work out your plan.
After everything is finished, commit your Copilot_Scrum.md and push to the current branch.

# UPDATES

## UPDATE

Using `SummarizeStyle` does not work with style name. Although task 2 is completed but I manually deleted `SummarizeStyle` helper functions from the code.

`textBox->SummarizeStyle` only summarize styles that assigned by `EditStyle`, `SummarizeStyle` will not calculate styles that applied style names. Please reconsider the remaining tasks.

## UPDATE

I would like you to merge 5,6,7,8 into one single task and 9,10 into one single task. But keep the existing TEST_CATEGORY design, by saying merging task I don't mean merging test cases into one TEST_CATEGORY.

# TASKS

- [x] TASK No.1: Configure UnitTest to Only Run TestControls_Editor_Styles.cpp
- [x] TASK No.2: Set Up Resource XML and Helper Functions
- [x] TASK No.3: Test EditStyleName (SingleParagraph)
- [x] TASK No.4: Test EditStyleName (MultiParagraph)
- [x] TASK No.5: Test RemoveStyleName, SummarizeStyleName, and RenameStyle
- [ ] TASK No.6: Test Undo/Redo for Style Operations
- [ ] TASK No.7: Restore UnitTest to Run All Tests

## TASK No.1: Configure UnitTest to Only Run TestControls_Editor_Styles.cpp

During development of these test cases, restrict the UnitTest project to only run the test file `TestControls_Editor_Styles.cpp` to speed up iteration.

### what to be done

- Edit `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` to set `<LocalDebuggerCommandArguments>` to `/F:TestControls_Editor_Styles.cpp` in the `Debug|x64` configuration. The `copilotExecute.ps1` script reads this value and passes it as command-line arguments to the `UnitTest.exe`.

### rationale

- The Execute.log shows the unit test framework already supports `/F:filename` filtering (e.g., `/F:TestControls_Editor_Styles.cpp`).
- The `copilotExecute.ps1` script reads `LocalDebuggerCommandArguments` from the `.vcxproj.user` file and appends them after the `/C` flag when running in UnitTest mode.
- This avoids running the entire test suite every time while developing the style tests, significantly speeding up the feedback loop.

## TASK No.2: Set Up Resource XML and Helper Functions

Define the resource XML for a `GuiDocumentViewer` with `Editable` mode and helper functions in `TestControls_Editor_Styles.cpp`, following the patterns established in `TestControls_Editor_RichText.cpp` and `TestControls_Editor.cpp`.

### what to be done

- In `TestControls_Editor_Styles.cpp`, define a resource XML constant `resource_DocumentViewer` that creates a `<Window>` containing a `<DocumentViewer ref.Name="textBox" EditMode="Editable">`.
- Create helper functions to create `DocumentStyleProperties` objects (`MakeStyleWithBold`, `MakeStyleWithItalic`, `MakeStyleWithColor`).
- Create a `RegisterStyle` helper function that registers a named style into the document model with a parent style name and properties.
- Create a `SummarizeName` helper function that wraps `textBox->SummarizeStyleName(begin, end)` for convenience.
- Restructure the outer `TEST_CATEGORY` to `L"Styles"` and plan inner categories for grouping.

### rationale

- Following the convention in `TestControls_Editor_RichText.cpp` and `TestControls_Editor.cpp` for defining resource XML and helpers.
- The `GuiDocumentViewer` is used with `EditMode="Editable"` because `SummarizeStyleName` and style edit operations require `Editable` mode.
- Helpers reduce code duplication across the many test cases.
- `SummarizeStyle` helpers were intentionally excluded because `SummarizeStyle` only reports properties applied via `EditStyle` (direct property editing), NOT properties applied via named styles (`EditStyleName`). Verification of named style effects will be done through `SummarizeStyleName` and direct inspection of the document model's `styles` dictionary.

## TASK No.3: Test EditStyleName (SingleParagraph)

Write test cases for `EditStyleName` on a single paragraph of text, under `TEST_CATEGORY(L"EditStyleName")` / `TEST_CATEGORY(L"SingleParagraph")`.

### what to be done

- **Test: Apply a registered style to a range** — Load text, register a style (e.g., `"MyBold"` with `bold=true`), call `EditStyleName(begin, end, "MyBold")`, then verify via `SummarizeStyleName` that the range returns `"MyBold"`.
- **Test: Apply a style to a partial range** — Load text `"0123456789"`, apply style to range `[2,5)`, verify that `SummarizeStyleName(0,2)` does NOT return the style name (returns null or different), `SummarizeStyleName(2,5)` returns the style name, and `SummarizeStyleName(5,10)` does NOT.
- **Test: Apply an unregistered style name** — Apply a style name that does not exist in the document's styles dictionary. Verify that `SummarizeStyleName` still returns the style name (the style name is stored in the run even if not registered, it just falls back to default visually).
- **Test: Apply different styles to adjacent ranges** — Apply `"StyleA"` to `[0,5)` and `"StyleB"` to `[5,10)`. Verify each range has its own style and that `SummarizeStyleName(0,10)` returns null (mixed styles).
- **Test: Overwrite existing style name** — Apply `"StyleA"` to `[0,10)` then `"StyleB"` to `[3,7)`. Verify the overlapping range has `"StyleB"`, and non-overlapping ranges still have `"StyleA"`.

### rationale

- `EditStyleName` is the primary function to apply named styles to text. Testing it on a single paragraph covers the most fundamental use case.
- The `AddStyleName` visitor in `GuiDocumentEditor_AddContainer.cpp` wraps text in `DocumentStyleApplicationRun` containers, so overlapping and adjacent range tests are critical to verify the run tree structure.
- Testing unregistered style names verifies the fallback behavior described in the problem statement: "When assigning a style name to a piece of text without the style registered, no error happens instead it fallback to default."
- Style inheritance is a property of the document model's style resolution, not of `EditStyleName` itself. Inheritance behavior is verified indirectly through `RenameStyle` parent reference tests (Task 5) and by inspecting the document model's `styles` dictionary.

## TASK No.4: Test EditStyleName (MultiParagraph)

Write test cases for `EditStyleName` spanning multiple paragraphs, under `TEST_CATEGORY(L"EditStyleName")` / `TEST_CATEGORY(L"MultiParagraph")`.

### what to be done

- **Test: Apply a style across paragraphs** — Load multi-paragraph text (e.g., `"0123456789\r\n\r\nabcdefghij"`), register a style, apply it across paragraph boundaries (e.g., `TextPos(0,5)` to `TextPos(2,5)`), verify via `SummarizeStyleName` on each paragraph.
- **Test: Apply a style to entire multi-paragraph document** — Apply a single style to the full range spanning all paragraphs and verify every paragraph has the style.

### rationale

- Multi-paragraph is important because the document model operates per-paragraph (`DocumentParagraphRun`), and `EditStyleName` on `GuiDocumentCommonInterface` delegates through `documentElement->EditStyleName` which iterates over paragraphs.
- The `TestControls_Editor_RichText.cpp` follows the same SingleParagraph/MultiParagraph split pattern.

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

- **Test: Remove style across paragraphs** — Apply a style across multiple paragraphs, then `RemoveStyleName` across the same range. Verify all paragraphs no longer have the style.
- **Test: Partial removal across paragraphs** — Apply a style across 3 paragraphs, then remove it from a range that spans the middle of paragraph 1 to the middle of paragraph 3. Verify the edges still retain the style while the middle is removed.

**Under `TEST_CATEGORY(L"SummarizeStyleName")`:**

- **Test: Returns non-null when entire range has the same style** — Apply `"MyStyle"` to `[0,10)`, verify `SummarizeStyleName(0,10)` returns `"MyStyle"`.
- **Test: Returns non-null for a subrange within a styled range** — Apply `"MyStyle"` to `[0,10)`, verify `SummarizeStyleName(2,5)` returns `"MyStyle"`.
- **Test: Returns null for mixed styles** — Apply `"StyleA"` to `[0,5)` and `"StyleB"` to `[5,10)`, verify `SummarizeStyleName(0,10)` returns null (Nullable without value).
- **Test: Returns null for partially styled text in the range** — Apply `"MyStyle"` to `[0,5)` only, verify `SummarizeStyleName(0,10)` returns null because `[5,10)` has no style name.
- **Test: Returns null for text with no style at all** — Load text with no style applied, verify `SummarizeStyleName(0,10)` returns null.
- **Test: Reversed begin/end** — Verify that calling `SummarizeStyleName` with `begin > end` still works correctly (the implementation swaps them, confirmed in `GuiDocumentCommonInterface.cpp`).
- **Test: Multi-paragraph with consistent style** — Apply the same style to all paragraphs, verify `SummarizeStyleName` across paragraphs returns the style name.

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

## TASK No.6: Test Undo/Redo for Style Operations

Write test cases for undo/redo behavior of `EditStyleName`, `RemoveStyleName`, and `RenameStyle`, each under their own `TEST_CATEGORY` hierarchy. This task covers the following TEST_CATEGORYs:
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"EditStyleName")`
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RemoveStyleName")`
- `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RenameStyle")`

### what to be done

**Under `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"EditStyleName")` and `TEST_CATEGORY(L"RemoveStyleName")`:**

- **Test: Undo EditStyleName** — Apply a style name, verify `CanUndo()` is true, call `Undo()`, verify `SummarizeStyleName` returns null (original state).
- **Test: Redo EditStyleName** — After undoing, verify `CanRedo()` is true, call `Redo()`, verify `SummarizeStyleName` returns the style name again.
- **Test: Undo RemoveStyleName** — Apply a style name, then remove it, then undo the removal. Verify the style name is restored.
- **Test: Redo RemoveStyleName** — After undoing a removal, redo it. Verify the style name is removed again.
- **Test: Multiple edits then undo all** — Apply style, then remove style, then undo both in sequence. Verify we return to the original unstyled state.
- **Test: Undo/Redo with CanUndo/CanRedo checks** — Verify `CanUndo()` and `CanRedo()` return correct values at each step of the undo/redo chain.

**Under `TEST_CATEGORY(L"UndoRedo")` / `TEST_CATEGORY(L"RenameStyle")`:**

- **Test: Undo RenameStyle** — Register a style, apply it, rename it, then undo. Verify:
  - The style name in text runs reverts to the old name.
  - The styles dictionary reverts to the old name.
- **Test: Redo RenameStyle** — After undoing, redo. Verify everything is renamed again.
- **Test: Undo RenameStyle with parent references** — Register parent and child styles, rename parent, undo. Verify the child's `parentStyleName` reverts to the original parent name.
- **Test: Multiple renames then undo all** — Rename `"A"` to `"B"`, then `"B"` to `"C"`, then undo both. Verify we return to `"A"`.

### rationale

- `EditStyleName` and `RemoveStyleName` both use `EditStyleInternal` which takes a before/after snapshot of the document model and submits to `undoRedoProcessor->OnReplaceModel()`. This is the same mechanism used by `EditStyle` and is confirmed in `GuiDocumentCommonInterface.cpp`.
- `RenameStyle` uses a separate undo/redo path: `undoRedoProcessor->OnRenameStyle(arguments)` with `RenameStyleStruct`, instead of the `ReplaceModelStruct` used by `EditStyleName`/`RemoveStyleName`. The `RenameStyleStep::Undo()` and `Redo()` in `GuiTextUndoRedo.cpp` (lines 136, 145) swap old/new names.
- The undo/redo pattern from `TestControls_Editor_Key_Shared.cpp` shows checking `CanUndo/CanRedo` and calling `Undo/Redo` in loops — this same pattern should be used for style operations.
- All undo/redo tests are grouped into one task while keeping separate TEST_CATEGORYs for each operation, since undo/redo is a cross-cutting concern and can be implemented together.

## TASK No.7: Restore UnitTest to Run All Tests

Remove the test file filter to restore running all tests.

### what to be done

- Edit `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` to clear `<LocalDebuggerCommandArguments>` in the `Debug|x64` configuration (remove the `/F:TestControls_Editor_Styles.cpp` filter).
- Run the full test suite to verify nothing is broken.

### rationale

- After all style tests are implemented and verified, the filter should be removed so that the full test suite is run during normal development to guard against regressions.
- This is the cleanup step for the configuration change made in TASK No.1.

# Impact to the Knowledge Base

## GacUI

- Consider adding a note under the document editor API section clarifying the distinction between `SummarizeStyle` (returns only properties applied via `EditStyle`) and `SummarizeStyleName` (returns the named style applied via `EditStyleName`). This is a subtle but important difference: `SummarizeStyle` does NOT resolve properties from named styles.

# !!!FINISHED!!!