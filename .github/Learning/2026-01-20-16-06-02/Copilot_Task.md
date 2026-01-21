# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.10: Add hyperlink editing + interaction tests (GuiDocumentLabel)

Add a hyperlink-focused rich-text test category for `GuiDocumentLabel`, covering hyperlink editing (`EditHyperlink` / `RemoveHyperlink`), hyperlink activation by mouse hover/leave, hyperlink execution by click, and correct reference reporting via `GetActiveHyperlinkReference`.

Per the UPDATE, this task creates a new test category, and must cover both single-paragraph and multi-paragraph cases, including overlapping edit/remove scenarios.

### what to be done

- In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, add a `TEST_CATEGORY` dedicated to hyperlink tests for `GuiDocumentLabel`.
	- Use a concise category name (e.g. `Hyperlink`) since `RichText` and `GuiDocumentLabel` are already implied by the file name and log path.
	- Ensure each test case log path starts with `Controls/Editor/Features/RichText/...` (not `Key`), and keep it consistent with TASK No.8, e.g. `Controls/Editor/Features/RichText/Hyperlink_<CaseName>`.
- Use `GuiDocumentCommonInterface` hyperlink editing APIs:
	- `EditHyperlink(paragraphIndex, begin, end, reference, normalStyleName, activeStyleName)`
	- `RemoveHyperlink(paragraphIndex, begin, end)`
- Extract a small static helper in the test file to compute a global mouse point for a caret position inside the document:
	- Call `GetCaretBounds({row, column}, false)` to get the caret rectangle in the control's container coordinate space.
	- Convert it to global space by getting the container composition (`GetContainerComposition()`), calling `LocationOf(composition, 0.0, 0.0, 0, 0)` for the top-left, and then adding the center point of the caret bounds rectangle.
	- Add `(x=4, y=0)` to the final result to compensate that the caret bounds center is the caret itself, not the character cell.
- Add test coverage (keep count small but representative):
	- Single-paragraph hyperlink creation/removal and reference visibility through `GetActiveHyperlinkReference`.
	- Mouse hover/leave triggers `ActiveHyperlinkChanged`, and clicking triggers `ActiveHyperlinkExecuted`.
	- Multi-paragraph scenarios (hyperlinks in different paragraphs) validating activation/execution in each paragraph.
	- Overlap / partial removal behavior:
		- Removing a middle part of a hyperlink breaks it into two hyperlinks with the same reference (as described in UPDATE).
		- Editing/removing overlapping ranges behaves correctly and does not leave stale active references.
	- When validating "outside edited range" invariants (e.g. unaffected style fields), compare against a baseline captured before edits/hover rather than assuming `Nullable<>` fields are always `null` (theme/default styles may produce concrete values).

### rationale

- Hyperlink behavior spans document editing, hit-testing, and event dispatch; putting it in its own category isolates failures from style-only tests.
- The caret-to-global-point helper keeps hyperlink interaction tests deterministic and avoids fragile pixel coordinates.
- Covering overlap and partial removal prevents regressions in hyperlink range splitting/merging logic.

# UPDATES

## UPDATE

If RemoveHyperlink just removes the whole hyperlink even when we select partial of the text, then we follow the implementation instead of my statement before.

There is an easy way to see if ActiveHyperlinkChanged or ActiveHyperlinkExecuted fires. We can define a List<WString> for each case, and when events happened, it added "Activated REFERENCE" or "Executed REFERENCE" to the list. And we can reuse the AssertCallbacks function in [TestItemProviders.h](Test/GacUISrc/UnitTest/TestItemProviders.h) to check the content of the list. You cal call Clear() to delete everything after calling AssertCallbacks (or just make a function AssertEvents locally but calls AssertCallbacks and Clear), so everytime you just need to check newly happened events.

# INSIGHTS AND REASONING

## Goal

Add a dedicated hyperlink test category to `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` that validates:

- Hyperlink model editing (`GuiDocumentCommonInterface::EditHyperlink` / `RemoveHyperlink`) for both single-paragraph and multi-paragraph text.
- Hyperlink interaction (`ActiveHyperlinkChanged`, `ActiveHyperlinkExecuted`, `GetActiveHyperlinkReference`) using deterministic mouse coordinates derived from caret positions.

## Key Existing Behaviors (Evidence)

- Hyperlink hover/click interaction is only implemented for `GuiDocumentEditMode::ViewOnly`:
	- Hover: `GuiDocumentCommonInterface::OnMouseMove` calls `documentElement->GetHyperlinkFromPoint(...)` and then `SetActiveHyperlink(package)` only in the `ViewOnly` switch case (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).
	- Click execution: `GuiDocumentCommonInterface::OnMouseUp` triggers `ActiveHyperlinkExecuted` only in the `ViewOnly` switch case and only when `activeHyperlinks` matches the hyperlink package from the click point (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).
- `GuiDocumentCommonInterface::GetActiveHyperlinkReference()` returns `activeHyperlinks->hyperlinks[0]->reference` when a hyperlink is active, otherwise `L""` (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).
- Hit-testing uses `GuiDocumentElementRenderer::GetHyperlinkFromPoint` which converts a point to a caret via `graphicsParagraph->GetCaretFromPoint(...)`, then queries `DocumentModel::GetHyperlink(row, caret, caret)` (`Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp`).
- `DocumentModel::RemoveHyperlink(paragraphIndex, begin, end)` removes the whole located hyperlink package, not the requested `[begin, end)`:
	- It calls `LocateHyperlink(...)` and then removes `[package->start, package->end)` (`Source/Resources/GuiDocument_Edit.cpp`).
	- Therefore, removing a sub-range inside a hyperlink removes the entire hyperlink package that overlaps the selection.
- Hyperlink editing APIs are implemented in the model as:
	- `DocumentModel::EditHyperlink(...)` mutates hyperlink runs if the target range is already inside a hyperlink package; otherwise it calls `RemoveHyperlink(...)` to clear any existing hyperlink(s) and then inserts a new hyperlink run for `[begin, end)` (`Source/Resources/GuiDocument_Edit.cpp`).
	- Default style names are `L"#NormalLink"` and `L"#ActiveLink"` (`Source/Resources/GuiDocument.cpp`).
- Unit-test input utilities already provide a reliable coordinate helper `UnitTestRemoteProtocol_IOCommands::LocationOf(...)` (`Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h`), which the task's caret-to-global conversion can build on.

## Proposed Test Category Shape

- Add a new top-level `TEST_CATEGORY(L"Hyperlink")` in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, parallel to the existing `SingleParagraph` / `MultiParagraph` categories.
- Each test case should use log paths under `Controls/Editor/Features/RichText/Hyperlink_<CaseName>` to match the existing log-path convention used by the rich-text tests in the file.

## Deterministic "Caret To Global Point" Helper

Create one small `static` helper local to `TestControls_Editor_RichText.cpp` to avoid fragile hard-coded pixel positions:

- Inputs: `(UnitTestRemoteProtocol* protocol, GuiDocumentLabel* textBox, TextPos caret)`.
- Compute caret rectangle: `auto caretBounds = textBox->GetCaretBounds(caret, false);`.
- Convert to global:
	- `auto container = textBox->GetContainerComposition();`
	- `auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);`
	- Add caret-bounds center to `topLeft`.
	- Add `(x=4, y=0)` to bias into the character cell (per task requirement).

This makes hyperlink activation/execution tests stable across theme/layout differences while still going through the real hit-testing path.

## Interaction Tests (Hover / Leave / Click)

Because interaction is only implemented in `ViewOnly` mode, these test cases should explicitly set the control to `GuiDocumentEditMode::ViewOnly` (even if the test window resource uses `EditMode="Editable"` today).

Common pattern (per test case):

- Init frame:
	- Load a short text where no line wrapping occurs.
	- Apply one or more hyperlinks via `EditHyperlink`.
	- Set edit mode to `ViewOnly`.
	- Attach `ActiveHyperlinkChanged` and `ActiveHyperlinkExecuted` handlers that append to `List<WString>` logs:
		- `ActiveHyperlinkChanged`: append `L"Activated " + textBox->GetActiveHyperlinkReference()`.
		- `ActiveHyperlinkExecuted`: append `L"Executed " + textBox->GetActiveHyperlinkReference()`.
		- Reuse `gacui_unittest_template::AssertCallbacks` from `Test/GacUISrc/UnitTest/TestItemProviders.h` to validate the content of the log list, and then call `Clear()` on the list (or create a tiny local `AssertEvents(...)` helper that calls `AssertCallbacks` and `Clear()`).
- Hover frame(s):
	- `protocol->MouseMove(pointInsideLink)` and verify that:
		- `GetActiveHyperlinkReference()` equals the expected reference.
		- `ActiveHyperlinkChanged` log shows the newly fired `Activated <REFERENCE>` entry (and is then cleared).
- Click frame(s):
	- `protocol->LClick(pointInsideLink)` and verify that:
		- `ActiveHyperlinkExecuted` log shows the newly fired `Executed <REFERENCE>` entry (and is then cleared).
		- The active reference stays correct.
- Leave frame(s):
	- Move the mouse outside the document container (e.g. `protocol->MouseMove({0,0})`) and verify:
		- `GetActiveHyperlinkReference()` becomes `L""`.
		- `ActiveHyperlinkChanged` log shows the newly fired `Activated ` entry (empty reference) (and is then cleared).

Multi-paragraph interaction should repeat this pattern for hyperlinks in different paragraphs (different `row` in `TextPos` and different caret-derived points), ensuring activation/execution is paragraph-local and does not "stick" after moving away.

## Editing Tests (Create / Remove / Overlap)

Editing-focused tests do not require mouse input and should validate document/model outcomes, preferably by querying the model rather than relying on rendering output:

- Create + remove (single paragraph):
	- Create a hyperlink `[begin, end)` and validate `GetHyperlink(paragraph, mid, mid)` returns a non-empty package with the expected reference.
	- Remove the hyperlink and validate `GetHyperlink(...)` becomes empty at positions that used to be inside the hyperlink.
- Create + remove (multi paragraph):
	- Repeat for hyperlinks in different paragraphs (different paragraph indices).
- Partial selection removal behavior (per current implementation):
	- Create one hyperlink covering a larger range.
	- Call `RemoveHyperlink(paragraphIndex, removeBegin, removeEnd)` where `[removeBegin, removeEnd)` is strictly inside the original hyperlink.
	- Verify that the entire hyperlink package is removed:
		- Positions that were on the left side and the right side of the removed sub-range should both report no hyperlink now.
	- Prefer validating via `DocumentModel::GetHyperlink(...)` at representative caret positions inside the original hyperlink range.
- Overlapping edit/remove:
	- Combine `EditHyperlink` + `RemoveHyperlink` on overlapping ranges and assert:
		- `GetActiveHyperlinkReference()` reflects the currently hovered hyperlink.
		- Hovering different areas after edits results in correct activation and no stale active state.
		- "Outside edited range" style summaries match a baseline captured before the operation (avoid assuming `Nullable<>` fields are always empty).

# !!!FINISHED!!!
