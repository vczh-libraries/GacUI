# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.8: Add single-paragraph rich text style editing tests (GuiDocumentLabel)

Add a new rich-text test category that verifies font, font style, and color editing behaviors in a single paragraph for `GuiDocumentLabel`.

Per the UPDATE, this task does not include hyperlinks.

All test cases in this task must be grouped under a single test category, and the log path must start with `RichText` (instead of `Key` as used by key-navigation tests).

### what to be done

- In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, create a `TEST_FILE` that hosts a `GuiDocumentLabel` in editable mode (similar resource XML layout used in `TestControls_Editor_Key.cpp`).
- Add exactly one `TEST_CATEGORY` for all single-paragraph, non-hyperlink rich-text tests for `GuiDocumentLabel`.
	- Ensure each test case log path follows `Controls/Editor/Features/RichText/...` (or the equivalent existing `Controls/Editor/...` pattern) so it starts with `RichText` where `Key` is used in other files.
- Use rich-text editing APIs from `GuiDocumentCommonInterface` to modify style:
	- `EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)` for font (e.g. `face`, `size`), font style (e.g. `bold`/`italic`/`underline`/`strikeline`), and color (e.g. `color`).
	- Prefer verifying applied styles by calling `SummarizeStyle(begin, end)` on the edited range; if summary semantics are insufficient for overlap tests, inspect the underlying `DocumentModel` runs to ensure style segmentation is correct.
- Add test cases (all in this single category):
	- One test per property (font / font style / color), each using only one editing call.
	- One test that edits all properties on the same piece of text, using only one editing call per property.
	- A small set of overlap tests where multiple editing calls (different or same property) overlap and the final style state is verified.

### rationale

- Single-paragraph tests isolate style-run editing behavior without introducing paragraph-boundary concerns, making failures easier to interpret.
- Keeping all cases in one category matches the UPDATE and keeps logs predictable while cases are iteratively added later.
- Using `SummarizeStyle` provides a stable and direct way to assert computed style at a range, which is especially useful when edits cause run splitting/merging.

# UPDATES

# INSIGHTS AND REASONING

## Scope

This design only covers editor rich-text style editing tests for `vl::presentation::controls::GuiDocumentLabel` (single paragraph) using `vl::presentation::controls::GuiDocumentCommonInterface::EditStyle` / `SummarizeStyle`, and explicitly excludes hyperlink-related APIs.

## Why `GuiDocumentLabel`

- `EditStyle` is a no-op when `GuiDocumentCommonInterface` is configured with `pasteAsPlainText` (`GuiDocumentCommonInterface::EditStyle` returns early in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`), so these tests must run on a rich-text-capable control configuration.
- The knowledge base documents that `GuiDocumentLabel` and `GuiDocumentTextBox` have no functionality difference (see `.github/KnowledgeBase/manual/gacui/components/controls/editor/documentlabel.md`), so using `GuiDocumentLabel` is representative for rich document editing behavior while matching the task requirement.

## Test File Shape

Update `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` to follow the established editor test harness patterns (see `Test/GacUISrc/UnitTest/TestControls_Editor.cpp` and `Test/GacUISrc/UnitTest/TestControls_Editor_Key_Shared.cpp`):

- Define a resource XML string that hosts a `<DocumentLabel ref.Name="textBox" EditMode="Editable">` with a bounds composition (reuse the same layout pattern as the `resource_DocumentLabel` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`).
- Create exactly one `TEST_CATEGORY` containing all test cases in this task.
- Each `TEST_CASE` starts a GUI test run using `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` and drives operations inside `GacUIUnitTest_SetGuiMainProxy(...)`.
- Log path format: `Controls/Editor/Features/RichText/GuiDocumentLabel/<CategoryAndCaseIdentity>` (to ensure the path starts with `RichText`, unlike the key-navigation tests that use `/Key/...`).

## Document Setup Per Test

Each `TEST_CASE` should:

- Initialize the control content to a known single-paragraph string (no CRLF) using `GuiDocumentCommonInterface::LoadTextAndClearUndoRedo`, to remove any dependence on default content and to guarantee `TextPos(row=0, ...)` covers the entire test range.
- Select a stable character range using `vl::presentation::TextPos` (row is paragraph index, column is caret position; see `.github/KnowledgeBase/manual/gacui/components/controls/editor/home.md`).

## Style Editing API Usage

Use `vl::presentation::controls::GuiDocumentCommonInterface::EditStyle(TextPos begin, TextPos end, Ptr<vl::presentation::DocumentStyleProperties> style)` (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h`, and `vl::presentation::DocumentStyleProperties` is defined in `Source/Resources/GuiDocument.h`):

- Font: edit `DocumentStyleProperties::face` and/or `DocumentStyleProperties::size` in a single call.
- Font style: edit one or more of `bold` / `italic` / `underline` / `strikeline` in a single call.
- Color: edit `DocumentStyleProperties::color` in a single call.

## Verification Strategy

Primary assertions use `vl::presentation::controls::GuiDocumentCommonInterface::SummarizeStyle(begin, end)` (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h`):

- The knowledge base describes `SummarizeStyle` as returning a “common” style across a range, using `null` for fields that are not uniform across the range (see `.github/KnowledgeBase/manual/gacui/components/controls/editor/home.md`).
- Tests should assert only the fields that are explicitly edited in the case, and avoid depending on unrelated fields that may vary due to theme/baseline styles.

Fallback for overlap tests (only when needed):

- Inspect the underlying `vl::presentation::DocumentModel` (available via `GuiDocumentCommonInterface::GetDocument`, declared in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h`) and validate that runs are segmented at the expected boundaries.
- The run tree and paragraph/run types are defined in `Source/Resources/GuiDocument.h` (e.g., `DocumentModel::paragraphs` and run classes under `vl::presentation`).

## Planned Test Cases (All in One Category)

### One-call-per-property tests

- Font: apply a face/size change to a subrange in paragraph 0; verify `SummarizeStyle` on that subrange reports the new face/size.
- Font style: apply a bold/italic/underline/strikeline change to a subrange; verify `SummarizeStyle` reports the toggled field(s).
- Color: apply a color change to a subrange; verify `SummarizeStyle` reports the new color.

### One-range, multiple-properties test (one call per property)

On the same subrange:

- Call `EditStyle` once for font, once for font style, once for color.
- Verify `SummarizeStyle` reflects all edited fields on the range.

### Overlap tests (small set)

Use multiple `EditStyle` calls with overlapping ranges in the single paragraph and verify:

- Subranges inside each overlap region have the expected final field values (via `SummarizeStyle`).
- A superset range spanning multiple differently-styled subranges yields `null` for fields that are intentionally not uniform (also via `SummarizeStyle`), with optional run-tree inspection when this is insufficient to demonstrate correct splitting/merging.

# !!!FINISHED!!!
