# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.3: Fix caret navigation semantics for multiline mode (GuiMultilineTextBox)

Add multiline-mode caret-navigation coverage into `RunTextBoxKeyTestCases_Multiline` and fix implementation when necessary so that keyboard navigation works consistently for `GuiDocumentParagraphMode::Multiline` in `GuiMultilineTextBox`.

Per the latest UPDATE, this task must include real test cases so `RunTextBoxKeyTestCases_Multiline` is not empty beyond ENTER/CRLF.

### what to be done


- Extend `RunTextBoxKeyTestCases_Multiline` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp with keyboard caret-navigation test cases that cover:
	- `LEFT` / `RIGHT` boundary movement across line boundaries in multiline mode.
	- `UP` / `DOWN` and `PAGE UP` / `PAGE DOWN` vertical movement.
	- `HOME` / `END` behavior per the UPDATE (line  paragraph  document), and `CTRL+HOME` / `CTRL+END` (document).
	- Any other navigation operations that are mode-sensitive for `GuiDocumentParagraphMode::Multiline` as discovered from existing tests and implementation.
- In order to make caret movement visible in logs and to ensure each frame produces UI change:
	- After each navigation key (or small sequence), type a character (or insert a marker) so the caret position becomes visually reflected and the frame is valid.
	- When a step already causes a visible UI change (e.g. editing text), it is fine to put `window->Hide();` in a final dedicated frame to log the post-action UI state cleanly.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to avoid wrapping in the unit-test renderer, so the tests primarily validate caret navigation rules rather than the stub wrapping algorithm.

- Review keyboard navigation handling in #file:GuiTextCommonInterface.cpp (especially `KEY_HOME`, `KEY_END`, and boundary behaviors for `KEY_LEFT`/`KEY_RIGHT`).
- Ensure the requested HOME/END semantics:
	- `HOME` / `END` first targets the beginning/ending of the current line.
	- If that would not move the caret (already at that position), it should target the beginning/ending of the current paragraph.
	- If that still would not move the caret, it should target the beginning/ending of the whole document.
	- `CTRL+HOME` / `CTRL+END` always targets the beginning/ending of the whole document.
	- Ensure `LEFT`/`RIGHT` boundary movement matches expectations for multiline mode (line-level boundaries).
- If the new tests reveal mismatches, fix the implementation in #file:GuiTextCommonInterface.cpp until they pass.
- If failures are caused by the unit-test rendering stub (caret bounds / caret movement), adjust #file:GuiUnitTestProtocol_Rendering.cpp minimally so caret navigation in tests reflects the intended behavior, while keeping line-wrapping out of scope.

### rationale

- Scoping this task to multiline mode matches the clarified design: Task 2/3 focus on `GuiMultilineTextBox` and `RunTextBoxKeyTestCases_Multiline`.
- Adding navigation test cases here ensures `RunTextBoxKeyTestCases_Multiline` contains real multiline-specific coverage beyond basic editing semantics.
- The unit-test rendering layer participates in caret queries; when failures are caused by stub behavior, it should be fixed in isolation and as minimally as possible.

# UPDATES

# INSIGHTS AND REASONING

## Scope clarification

- This design only targets TASK No.3: multiline mode (`GuiMultilineTextBox`, `GuiDocumentParagraphMode::Multiline`) navigation behaviors, and the corresponding navigation test coverage in `RunTextBoxKeyTestCases_Multiline`.
- Multiline text boxes use `GuiDocumentConfig::GetMultilineTextBoxDefaultConfig()` which sets `wrapLine = false` and `paragraphMode = GuiDocumentParagraphMode::Multiline` (see `Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp`).
- The document-formatting contract in `GuiDocumentCommonInterface` states that when `paragraphMode` is Multiline it “removes line breaks in each paragraph” after breaking text into paragraphs (see the doc comment in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h`). Combined with `doubleLineBreaksBetweenParagraph = false` for multiline text boxes, this implies each line break in user text becomes a paragraph boundary, and each paragraph is conceptually a “line” in this mode (with no in-paragraph line breaks).

## Current behavior and gaps (evidence)

- `RunTextBoxKeyTestCases_Multiline` currently has ENTER/CRLF and clipboard coverage only (see `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`); it contains no caret-navigation test coverage.
- `GuiDocumentCommonInterface::ProcessKey` handles `UP/DOWN/LEFT/RIGHT/HOME/END` by calling `documentElement->CalculateCaret(...)` and then `Move(...)` (see `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).
  - `KEY_HOME` / `KEY_END` currently do “line first/last”, then fall back to `CaretFirst` / `CaretLast` only. This matches “line -> paragraph”, but does not implement the final “paragraph -> document” escalation.
  - `CTRL+HOME` / `CTRL+END` are not implemented (the `ctrl` flag is ignored for HOME/END in this function, and there is no corresponding shortcut registration).
  - `KEY_PRIOR` / `KEY_NEXT` (PageUp/PageDown) are currently empty blocks.
- Cross-paragraph navigation behavior for arrow keys is implemented in the document renderer: if `IGuiGraphicsParagraph::GetCaret(..., CaretMoveLeft/Right/Up/Down, ...)` does not move the caret, `GuiDocumentElementRenderer::CalculateCaret` escalates to the previous/next paragraph using `GetCaret(...)` and `GetCaretFromPoint(...)` (see `Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp` around `GuiDocumentElementRenderer::CalculateCaret`). In multiline mode, where each paragraph corresponds to a “line”, this is the primary mechanism for moving across line boundaries.

## Proposed design (implementation changes)

### 1) HOME/END: implement the requested escalation chain

Implement the full “line -> paragraph -> document” escalation chain in `GuiDocumentCommonInterface::ProcessKey` for `KEY_HOME` and `KEY_END`, in a way that works for all paragraph modes (but is validated by multiline-mode tests in this task):

- For `HOME` (no CTRL):
  1. `CaretLineFirst` (beginning of current visual line within current paragraph).
  2. If no movement: `CaretFirst` (beginning of current paragraph).
  3. If still no movement: move to beginning of document (paragraph 0, caret 0).
- For `END` (no CTRL):
  1. `CaretLineLast`.
  2. If no movement: `CaretLast`.
  3. If still no movement: move to end of document (last paragraph, paragraph-local `CaretLast`).

Rationale:
- In multiline mode with `wrapLine = false`, each paragraph is a single line, so step (1) and step (2) are frequently equivalent; step (3) is therefore essential to observe the requested behavior when HOME/END are pressed repeatedly at edges.

### 2) CTRL+HOME / CTRL+END: add explicit document-edge movement

Add explicit handling for CTRL-modified HOME/END in `GuiDocumentCommonInterface::ProcessKey`:

- `CTRL+HOME`: move to beginning of document directly.
- `CTRL+END`: move to end of document directly.

Rationale:
- The current implementation ignores the `ctrl` modifier for HOME/END, so CTRL+HOME/END cannot behave differently from HOME/END.

### 3) PAGE UP / PAGE DOWN: implement viewport-relative vertical movement

Implement `KEY_PRIOR` / `KEY_NEXT` in `GuiDocumentCommonInterface::ProcessKey` as “move caret vertically by roughly one page while preserving horizontal intent”, using the existing caret-bounds APIs (which already integrate with scrolling via `EnsureDocumentRectVisible`):

- Determine the caret’s current x coordinate using `documentElement->GetCaretBounds(currentCaret, frontSide)`.
- Determine a target y coordinate by subtracting/adding approximately one viewport height.
  - Use `GetDocumentViewPosition()` and the available container bounds (e.g. via the control/bounds composition) to derive the viewport height in document coordinates.
- Compute the destination caret by calling `documentElement->CalculateCaretFromPoint(Point(targetX, targetY))`.
- Call `Move(newCaret, shift, frontSide)`.

Rationale:
- Unlike the deprecated text-box interface (which can move by “rows”), the document-based controls already have point-based caret calculations and caret bounds; using point-based movement aligns with how `GuiDocumentElementRenderer::CalculateCaret` escalates `UP/DOWN` across paragraphs using `GetCaretFromPoint(...)`.

### 4) Unit-test renderer adjustments only when strictly necessary

If new navigation tests fail due to unit-test-only rendering behavior (rather than document navigation logic), apply minimal changes in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp` so:

- `GetCaretBounds` and caret movement calculations are consistent enough for `GuiRemoteGraphicsParagraph::GetCaretFromPoint` (which in the remote provider scans carets using `GetCaretBounds`) to approximate real caret behavior.
- Wrapping behavior remains out of scope; fixes should not aim to make wrapping “more correct”, only to ensure caret navigation is stable for non-wrapping test inputs.

## Proposed design (test additions in RunTextBoxKeyTestCases_Multiline)

Add a new `TEST_CATEGORY(L"Navigation (Multiline)")` under `RunTextBoxKeyTestCases_Multiline` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`.

General test strategy:
- Build a multiline document with multiple short lines (paragraphs) using typing + Enter and/or clipboard paste so that each paragraph stays well under half window width (avoid wrapping sensitivity).
- For each navigation operation:
  - Press the navigation key (or a small sequence).
  - Immediately type a single marker character (e.g. `A`, `B`, `C`, …) so the caret position is visible in rendered logs and each frame produces UI change.
  - Assert the underlying document model by inspecting:
    - `textBox->GetDocument()->paragraphs.Count()`
    - Each paragraph’s content via `DocumentParagraphRun::GetTextForReading()`

Recommended test cases (minimal but representative):
- `LeftRight_Boundary_JumpsAcrossLines`:
  - Prepare 3 lines with distinct content (e.g. `L0`, `L1`, `L2`).
  - Place caret at (row 1, col 0), press LEFT, type marker, assert marker ended at end of row 0.
  - Place caret at end of row 1, press RIGHT, type marker, assert marker inserted at beginning of row 2.
- `UpDown_MovesAcrossLines`:
  - Use 3+ lines of equal-ish lengths.
  - Place caret in middle line at a non-zero column, press UP, type marker, verify marker inserted in the expected line at the closest column.
  - Repeat for DOWN.
- `HomeEnd_Escalation_Line_Paragraph_Document`:
  - Place caret in a middle line at a middle column.
  - HOME then marker: marker appears at start of the current line.
  - HOME then marker again: marker appears at the start of the whole document (since in multiline mode “line == paragraph”, the intermediate step would not move).
  - Similarly for END twice (line end, then document end).
- `CtrlHomeEnd_JumpsToDocumentEdges`:
  - Place caret in a middle line, press CTRL+HOME, type marker, assert marker appears at the beginning of paragraph 0.
  - Place caret in a middle line, press CTRL+END, type marker, assert marker appears at the end of the last paragraph.
- `PageUpPageDown_MovesVerticallyByViewport`:
  - Create enough lines to exceed one viewport (e.g. 30 short lines).
  - Place caret near bottom, press PAGE UP, type marker, assert marker appears several lines earlier.
  - Place caret near top, press PAGE DOWN, type marker, assert marker appears several lines later.
  - Assertions should avoid depending on an exact number of lines moved; instead verify that the target paragraph index changes in the expected direction and remains within bounds, and that the inserted marker lands within a plausible column range for the prepared content.

## Why this design fits the repo

- Navigation is centralized in `GuiDocumentCommonInterface::ProcessKey`, and paragraph-crossing semantics are already designed into `GuiDocumentElementRenderer::CalculateCaret`. Implementing HOME/END escalation, CTRL+HOME/END, and PAGE UP/DOWN here keeps behavior consistent across all document-based text controls.
- Multiline mode is already modeled as paragraph-per-line in the user-input formatting contract; validating boundary movement and HOME/END escalation in multiline mode directly exercises the intended “paragraph as line” design without involving wrapping.
- Using `GetDocument()` + `GetTextForReading()` for assertions keeps tests stable against any future `GetText()` normalization rules and aligns with the overall testing direction in this scrum.

# !!!FINISHED!!!
