# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.5: Add keyboard caret navigation tests (Paragraph mode controls)

Add keyboard caret navigation coverage into `RunTextBoxKeyTestCases_MultiParagraph` to validate Paragraph-mode boundary behaviors and HOME/END semantics for the three Paragraph-mode controls.

This task focuses on navigation only (ENTER/CRLF behaviors should already be covered by TASK No.4).

As clarified in the latest update, only the default paragraph mode for each Paragraph-mode control is covered here (no mode switching within a control instance).

Note: earlier request text used `RunTextBoxKeyTestCases_MultiParagraphs`; this document follows the latest update and uses `RunTextBoxKeyTestCases_MultiParagraph` as the helper name.

### what to be done

- Add new test cases in `RunTextBoxKeyTestCases_MultiParagraph` that cover:
	- `LEFT` / `RIGHT` boundary movement:
		- Multi-paragraph mode: beginning/ending of paragraph jumps to previous/next paragraph when appropriate.
	- `UP` / `DOWN` vertical caret movement.
	- `PAGE UP` / `PAGE DOWN` vertical caret movement by page.
	- `HOME` / `END` behavior per the UPDATE (line → paragraph → document), and `CTRL+HOME` / `CTRL+END` (document).
	- Any other navigation operations that are mode-sensitive for `GuiDocumentParagraphMode::Paragraph` as discovered from existing tests and implementation.
- In order to make caret movement visible in logs and to ensure each frame produces UI change:
	- After each navigation key (or small sequence), type a character (or insert a marker) so the caret position becomes visually reflected and the frame is valid.
	- Keep frame count minimal while still keeping logs readable.
		- It is acceptable to merge `window->Hide();` into the last verification frame (instead of a dedicated hide-only frame) as long as each frame still introduces a UI change.
- Organize `protocol->OnNextIdleFrame(...)` frames per `## Organizing Frames` in `.github/copilot-instructions.md`:
	- First frame is always `Ready`, and each subsequent frame name describes what the previous frame did.
	- Prefer action + verification in the same frame, and keep a final hide-only frame (`window->Hide();`) when possible.
	- For `PAGE UP` / `PAGE DOWN`, avoid asserting an exact moved line count; prefer inserting a marker and locating it in `GetDocument()` to assert the caret moved in the expected direction.
- Avoid triggering line wrapping in these new cases:
	- Keep each line/paragraph short enough to avoid wrapping in the unit-test renderer, so the tests primarily validate caret navigation rules rather than the stub wrapping algorithm.

### rationale

- Caret navigation is mode-sensitive and easy to regress; dedicated tests protect these behaviors when caret/line calculations change.
- Splitting navigation tests from ENTER/CRLF tests keeps failures localized and helps diagnose whether a regression is in editing semantics or navigation semantics.
- Typing after moving keeps frame logs readable and prevents “no UI change” frame crashes.

# UPDATES

## UPDATE

You must follow the frame organization of similar test cases in `RunTextBoxKeyTestCases_Multiline`, but I am not asking for following test case design, since there is behavior differences. Stick to you own test case design, but follow the frame organization.

# INSIGHTS AND REASONING

## Where navigation behavior comes from (evidence)

- Keyboard navigation is implemented in `GuiDocumentCommonInterface::ProcessKey` (see `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`). It handles:
	- `KEY_LEFT`/`KEY_RIGHT`/`KEY_UP`/`KEY_DOWN` via `documentElement->CalculateCaret(..., CaretMove*)`.
	- `KEY_HOME`/`KEY_END` and their fallback chain (line → paragraph → document) via `CaretLineFirst/Last`, then `CaretFirst/Last`, then document boundary.
	- `CTRL+HOME`/`CTRL+END` as direct document boundary jumps.
	- `KEY_PRIOR`/`KEY_NEXT` (PageUp/PageDown) using caret bounds + viewport height and `CalculateCaretFromPoint`.

- Cross-paragraph boundary behavior for arrow keys is implemented in the document renderer `GuiDocumentElementRenderer::CalculateCaret` (see `Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp`): when a per-paragraph caret move does not change the caret, it switches to the previous/next paragraph appropriately.

- In unit tests, the paragraph-level caret operations are backed by the stub renderer in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp` (e.g. `Impl_DocumentParagraph_GetCaret`). New tests must be written to avoid line wrapping so they validate navigation semantics rather than the stub’s wrapping algorithm.

## Targeted test surface

The helper `RunTextBoxKeyTestCases_MultiParagraph` already covers Paragraph-mode ENTER/CRLF + clipboard semantics in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`.

This task extends that helper with a new navigation-focused category (e.g. `TEST_CATEGORY(L"Navigation (Paragraph)")`) and adds a small set of stable navigation test cases that apply to all three Paragraph-mode editor controls that call this helper.

## Test design principles

- Prefer model-based assertions over visual/caret-pixel assertions:
	- Use `GetCaretEnd()` (and sometimes `GetCaretBegin()`) to assert caret movement directly in terms of `TextPos(row, column)`.
	- After asserting the caret, type a marker character so the frame introduces a UI change and the caret position becomes visible in rendering logs.
	- After typing a marker, use `GetDocument()` + `DocumentParagraphRun::GetTextForReading()` to verify that the marker landed in the expected paragraph/line.

- Avoid line wrapping:
	- Keep each logical line short (well under half the window width) so the renderer stub doesn’t introduce extra wrapped lines.
	- For long documents (needed for PageUp/PageDown), use many short lines/paragraphs rather than long lines.

- Keep frames readable and compliant with unit test conventions, and align frame organization with existing key tests in `RunTextBoxKeyTestCases_Multiline` (in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`):
	- Start with `Ready` for setup + initial typing/input.
	- Name each subsequent frame for what the previous frame did (e.g. `Typed ...`, `Caret at (...) and [LEFT]`, `Pressed ...`).
	- Put the *next* action + verification in the frame whose name describes the *previous* action, matching the existing multiline tests’ log readability pattern.
	- Keep a final hide-only frame (`window->Hide();`) when possible; merge hide into the last verification frame only when a dedicated hide frame would violate the “frame must introduce UI change” rule.

## Proposed navigation test cases (high-level)

### 1) Left/Right across paragraph boundaries

Goal: verify Paragraph mode jumps between paragraphs at boundaries.

Arrange a small document with 3 paragraphs (created by `ENTER`), each containing a short single line.

Frame organization (mirrors `RunTextBoxKeyTestCases_Multiline`):
- `Ready`: focus + type initial content (create paragraphs / lines).
- `Typed ...`: perform the first boundary navigation + insert a marker + assert using `GetDocument()`.
- `Caret at (...) and [KEY]`: perform the next boundary navigation + insert a marker + assert.
- Final frame named for the previous step: hide the window.

- Place caret at `TextPos(1, 0)` (start of paragraph 1), press `LEFT`.
	- Assert `GetCaretEnd().row == 0` and caret is at end of paragraph 0.
	- Type marker `L` and verify paragraph 0 ends with `L`.

- Place caret at end of paragraph 0, press `RIGHT`.
	- Assert `GetCaretEnd().row == 1` and caret is at start of paragraph 1.
	- Type marker `R` and verify paragraph 1 begins with `R`.

This test intentionally focuses on paragraph boundaries (not line boundaries) to stay robust across renderer differences.

### 2) Up/Down across lines and paragraph boundaries

Goal: verify vertical navigation moves between explicit lines and across paragraphs when leaving the first/last line.

Arrange:

- Paragraph 0 with two explicit lines using `CTRL+ENTER` (line break inside paragraph).
- Paragraph 1 with two explicit lines.

Cases:

- Put caret on the second line of paragraph 0, press `UP`.
	- Assert caret stays in paragraph 0 but moves to the first line (row unchanged, column adjusted reasonably).
	- Type marker and verify it appears on the first line.

- Put caret on the last line of paragraph 0, press `DOWN`.
	- Assert caret moves to paragraph 1 (row increases).
	- Type marker and verify it appears in paragraph 1.

To keep this stable, choose line strings of equal (or similar) lengths so horizontal column mapping is predictable.

Frame organization follows the multiline pattern:
- `Ready`: create the multi-line/multi-paragraph content.
- `Typed ...`: do `UP` case (action + verification + marker).
- `Caret at ... and [UP]`: do `DOWN` case (action + verification + marker).
- Final frame named for the previous step: hide.

### 3) HOME / END fallback chain (line → paragraph → document)

Goal: validate the multi-stage behavior already implemented in `GuiDocumentCommonInterface::ProcessKey`.

For HOME (repeat for END):

- Arrange a paragraph that has 2 lines (created via `CTRL+ENTER`) and multiple paragraphs in the document.
- Set caret somewhere in the second line.
	- Press `HOME`, assert caret moves to beginning of that line.
	- Type marker `h1` (single character) and verify it appears at that line start.
- Reset caret (by `SetCaret` to the same original location).
	- Press `HOME` twice:
		- First HOME takes caret to line start.
		- Second HOME (no longer moving at line scope) takes caret to paragraph start.
	- Type marker and verify it appears at paragraph start.
- Reset caret again.
	- Press `HOME` three times to reach document start.
	- Type marker and verify it appears at the very beginning of paragraph 0.

For `CTRL+HOME`/`CTRL+END`, a single key press should always jump to the beginning/ending of the whole document; verify using caret position and marker insertion.

Frame organization follows the multiline pattern:
- `Ready`: create content and capture any shared baselines needed for repeated HOME/END sequences.
- Subsequent frames: each performs one HOME/END scenario (or a small repeat sequence), inserts a marker, and asserts via `GetDocument()`.
- A final hide frame ends the test.

### 4) PageUp / PageDown moves caret by viewport direction

Goal: assert directionality without relying on an exact line delta.

Arrange a document with enough content to allow paging (many short lines; no wrapping).

- Record the starting caret row/paragraph and/or caret bounds `y` via `GetCaretBounds(GetCaretEnd(), ...)`.
- Press `PAGE DOWN`, assert that either caret bounds `y` increased or caret row increased (moved downward in document).
- Type a marker and locate it via `GetDocument()` to confirm it landed “later” in the document than a marker inserted before paging.

Repeat symmetrically for `PAGE UP`.

Frame organization follows the multiline pattern:
- `Ready`: create enough content to allow paging.
- `Typed ...`: capture baseline, perform `PAGE DOWN`, insert a marker, assert directionality.
- `Paged Down`: perform `PAGE UP`, insert a marker, assert directionality.
- Final hide frame.

## If tests fail: likely fix locations

- If HOME/END semantics differ from the required fallback chain, fix `GuiDocumentCommonInterface::ProcessKey` handling of `KEY_HOME`/`KEY_END` (in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).

- If arrow-key boundary movement between paragraphs is incorrect, check `GuiDocumentElementRenderer::CalculateCaret` behavior for `CaretMoveLeft`/`CaretMoveRight` (in `Source/GraphicsElement/GuiGraphicsDocumentRenderer.cpp`) and how the paragraph renderer reports “no movement” at boundaries.

- If failures only reproduce in unit tests (but logic is correct), the likely cause is the unit-test renderer stub’s caret/line model (`Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp`). Any changes there should be minimal and should not expand the scope into wrapping behavior.

# !!!FINISHED!!!
