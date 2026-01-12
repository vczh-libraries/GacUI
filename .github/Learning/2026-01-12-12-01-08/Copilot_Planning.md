# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add multiline navigation test coverage

Change `RunTextBoxKeyTestCases_Multiline` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` to include a new `TEST_CATEGORY(L"Navigation (Multiline)")` and add representative caret-navigation test cases for `GuiMultilineTextBox` in `GuiDocumentParagraphMode::Multiline`.

Why:
- The task explicitly requires adding multiline-mode caret-navigation coverage beyond ENTER/CRLF.
- New tests are needed to reproduce and pin down the expected semantics before changing implementation.

Planned test strategy (applies to all new cases):
- Prepare multiple short lines (paragraphs) to avoid wrapping in the unit-test renderer.
- For each navigation key (or short sequence), immediately type a visible marker character so that:
  - the UI changes every frame (stable rendering / logs),
  - the caret position is reflected in the underlying document text.
- Assert behavior by inspecting `textBox->GetDocument()->paragraphs[*]->GetTextForReading()` rather than relying on rendered coordinates.

Planned helper pattern inside tests:
	// After a navigation operation, type a marker to make the caret visible in the model.
	protocol->KeyPress(VKEY::KEY_XXX, shift, ctrl, false);
	protocol->TypeString(L"|");

Concrete test cases to add (minimal set that covers required semantics):

1) `LeftRight_Boundary_JumpsAcrossLines`
- Arrange: create 3 paragraphs with short text, e.g. `AAA`, `BBB`, `CCC`.
- Act/Assert:
  - Set caret to `TextPos(1, 0)`, press `KEY_LEFT`, type `|`.
    - Expect `|` is appended to paragraph 0 (end of previous line).
  - Set caret to end of paragraph 1, press `KEY_RIGHT`, type `|`.
    - Expect `|` is inserted at the beginning of paragraph 2.

Planned code sketch:
	protocol->OnNextIdleFrame(L"Ready", [=]()
	{
		auto window = GetApplication()->GetMainWindow();
		auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
		textBox->SetFocused();
		protocol->TypeString(L"AAA");
		protocol->KeyPress(VKEY::KEY_RETURN);
		protocol->TypeString(L"BBB");
		protocol->KeyPress(VKEY::KEY_RETURN);
		protocol->TypeString(L"CCC");
	});

	protocol->OnNextIdleFrame(L"LEFT boundary", [=]()
	{
		auto window = GetApplication()->GetMainWindow();
		auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
		textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
		protocol->KeyPress(VKEY::KEY_LEFT);
		protocol->TypeString(L"|");
	});

	protocol->OnNextIdleFrame(L"Verify LEFT boundary", [=]()
	{
		auto window = GetApplication()->GetMainWindow();
		auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
		auto document = textBox->GetDocument();
		TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
	});

2) `UpDown_MovesAcrossLines`
- Arrange: multiple lines with similar lengths (to avoid ambiguous clamping).
- Act/Assert:
  - Place caret at a mid-column in a middle paragraph, press `KEY_UP`, type marker, verify marker lands in the previous paragraph at the same (or clamped) column.
  - Repeat for `KEY_DOWN`.

3) `HomeEnd_Escalation_Line_Paragraph_Document`
- Arrange: multiple paragraphs, place caret in the middle of a paragraph.
- Act/Assert (per task semantics):
  - `HOME` then `|`: marker inserted at the beginning of the current line.
  - `HOME` again then `|`: marker inserted at the beginning of the current paragraph (if different from line semantics in other modes).
  - `HOME` again then `|`: marker inserted at the beginning of the document.
  - Similarly for `END` (line end -> paragraph end -> document end).

Note for multiline mode:
- In `GuiDocumentParagraphMode::Multiline`, each line break becomes a paragraph boundary, so “line == paragraph” for most inputs.
- The tests should still explicitly validate the “no movement => escalate” rule by pressing HOME/END repeatedly and verifying that the final escalation reaches document edges.

4) `CtrlHomeEnd_JumpsToDocumentEdges`
- Arrange: multiple paragraphs.
- Act/Assert:
  - `CTRL+HOME` then `|` inserts at `TextPos(0, 0)`.
  - `CTRL+END` then `|` inserts at the end of the last paragraph.

5) `PageUpPageDown_MovesVerticallyByViewport`
- Arrange: create enough short lines to require vertical scrolling (e.g. 30+ paragraphs like `L00`, `L01`, ...).
- Act/Assert:
  - Place caret somewhere in the middle, press `KEY_PRIOR`, type marker, verify marker lands in an earlier paragraph.
  - Press `KEY_NEXT`, type marker, verify marker lands in a later paragraph.
- Assertions avoid depending on an exact number of lines moved; only verify direction and that the resulting paragraph index changes and remains in range.

## STEP 2: Fix HOME/END semantics in document key handling

Change `GuiDocumentCommonInterface::ProcessKey` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` to match required semantics:
- `HOME` / `END`:
  1) target beginning/ending of current line,
  2) if that does not move, target beginning/ending of current paragraph,
  3) if that still does not move, target beginning/ending of whole document.
- `CTRL+HOME` / `CTRL+END`:
  - always target beginning/ending of whole document.

Why:
- Current implementation only escalates `HOME` / `END` from line to paragraph (via `CaretFirst` / `CaretLast`) and does not implement document-level escalation.
- New multiline navigation tests will require consistent behavior in multiline mode, especially repeated HOME/END and CTRL+HOME/END.

Planned code change (sketch, matching existing types and helpers in this file):
	case VKEY::KEY_HOME:
		{
			if (ctrl)
			{
				Move(TextPos(0, 0), shift, true);
				break;
			}

			TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineFirst, frontSide);
			if (newCaret == currentCaret)
			{
				newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretFirst, frontSide);
			}
			if (newCaret == currentCaret)
			{
				newCaret = TextPos(0, 0);
			}
			Move(newCaret, shift, frontSide);
		}
		break;

	case VKEY::KEY_END:
		{
			if (ctrl)
			{
				vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
				auto lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];
				Move(TextPos(lastIndex, lastParagraph->GetTextForCaret().Length()), shift, false);
				break;
			}

			TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineLast, frontSide);
			if (newCaret == currentCaret)
			{
				newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLast, frontSide);
			}
			if (newCaret == currentCaret)
			{
				vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
				auto lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];
				newCaret = TextPos(lastIndex, lastParagraph->GetTextForCaret().Length());
			}
			Move(newCaret, shift, frontSide);
		}
		break;

Notes:
- `IGuiGraphicsParagraph::CaretFirst` / `CaretLast` are paragraph-scoped; document-scoped movement is implemented by explicitly constructing `TextPos` at document edges (consistent with existing `SelectAll()` and `LoadTextAndClearUndoRedo()` patterns in the same file).
- The `frontSide` argument for `Move` continues to follow existing behavior; if new tests reveal “preferFrontSide” issues at boundaries, adjust minimal logic around `frontSide` in the relevant branches.

## STEP 3: Implement PAGE UP / PAGE DOWN caret movement

Change `GuiDocumentCommonInterface::ProcessKey` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` to implement `VKEY::KEY_PRIOR` (Page Up) and `VKEY::KEY_NEXT` (Page Down) for document-based text controls.

Why:
- Multiline navigation coverage explicitly requires PAGE UP / PAGE DOWN behavior.
- Current implementation has empty cases for these keys, so tests will fail until behavior is defined.

Planned approach:
- Use point-based caret calculation to align with existing mouse caret logic:
  - Determine the current caret bounds via `documentElement->GetCaretBounds(currentCaret, frontSide)`.
  - Use the mouse-area viewport height as the page size.
  - Compute a target point with same x as current caret and y shifted by ±viewport height.
  - Convert the target point to a caret using `documentElement->CalculateCaretFromPoint(Point(x, y))`.
  - `Move(newCaret, shift, frontSide)`.

Planned code change (sketch):
	case VKEY::KEY_PRIOR:
	case VKEY::KEY_NEXT:
		{
			if (!documentMouseArea) break;
			vint page = documentMouseArea->GetCachedBounds().Height();
			if (page <= 0) break;

			Rect caretBounds = documentElement->GetCaretBounds(currentCaret, frontSide);
			vint x = caretBounds.x1;
			vint y = (caretBounds.y1 + caretBounds.y2) / 2;
			y += (code == VKEY::KEY_PRIOR ? -page : page);

			TextPos newCaret = documentElement->CalculateCaretFromPoint(Point(x, y));
			Move(newCaret, shift, frontSide);
		}
		break;

Quality constraints:
- Keep behavior stable for non-wrapping inputs (the test inputs explicitly avoid wrapping).
- Do not attempt to implement “exact number of lines per page”; use the viewport height, matching typical page movement semantics.

## STEP 4: Fix unit-test renderer only if required by failures

If newly added tests fail due to unit-test-only rendering/caret calculations (rather than document navigation logic), apply minimal fixes in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp`:
- Ensure caret bounds and caret-from-point computations are consistent enough for stable navigation in non-wrapping scenarios.
- Do not expand scope to improve wrapping correctness.

Why:
- The unit-test remote renderer participates in caret geometry queries; mismatches can manifest as incorrect “vertical” movements in tests even if document logic is correct.

# TEST PLAN

All tests are added under `RunTextBoxKeyTestCases_Multiline` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` and executed via the existing unit-test runner.

Test cases (expected pass criteria):
- `LeftRight_Boundary_JumpsAcrossLines`: markers appear in the expected previous/next paragraph when crossing boundaries.
- `UpDown_MovesAcrossLines`: markers appear in the expected adjacent paragraph, with column preserved or clamped.
- `HomeEnd_Escalation_Line_Paragraph_Document`: repeated HOME/END escalates until document edges are reached.
- `CtrlHomeEnd_JumpsToDocumentEdges`: markers land at document begin/end regardless of caret location.
- `PageUpPageDown_MovesVerticallyByViewport`: markers land in an earlier/later paragraph without leaving document bounds.

# !!!FINISHED!!!
