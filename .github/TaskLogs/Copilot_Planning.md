# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS
- Build the solution in folder REPO-ROOT\Test\GacUISrc
  - Run Test Project UnitTest

# EXECUTION PLAN

## STEP 1: Confirm current coordinate mismatch
- Read the mock paragraph implementation in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering_Document.cpp` and identify all places that assume `textPos == layoutIndex` (typically indexing `characterLayouts` by `DocumentParagraphLineInfo::{startPos,endPos}` or by `caret/textPos`).
- Confirm the inline-object modeling rule in this mock: an inline object occupies a text range `[caretBegin, caretEnd)` but only `caretBegin` is a valid caret.

Why:
- This bounds the change to only the unit-test mock and clarifies which APIs need to be converted to the new data structure.

## STEP 2: Replace `characterLayouts` with caret-keyed layouts
- In `DocumentParagraphState`, replace the linear `characterLayouts : List<DocumentParagraphCharLayout>` with caret-keyed layouts:
  - `collections::Dictionary<vint, DocumentParagraphCharLayout> caretLayouts;` (key = **textPos** of a **valid caret** that also has a drawable layout entry)
  - `collections::SortedList<vint> caretLayoutKeys;` (sorted keys; MUST exactly match the dictionary keys)
- Enforce the single invariant in code structure (avoid drift):
  - Invariant: `caretLayoutKeys` contains all keys in `caretLayouts`, sorted, with no duplicates.
  - All mutations must go through small helpers that update both containers atomically, e.g.:
    - `void ClearCaretLayouts();`
    - `void AddCaretLayout(vint caret, const DocumentParagraphCharLayout& layout);`

Semantics to document in code (contracts):
- `0..text.Length()` are text positions.
- A **valid caret** is a text position where caret navigation may land.
- A **drawable layout entry** exists only when there is a glyph/object after that caret; therefore `text.Length()` is a valid caret but must not appear in `caretLayouts`/`caretLayoutKeys`.
- Empty paragraph behavior must be consistent across geometry/hit-testing APIs: return a stable well-defined result (e.g. `(0,0)` / “end of last drawable entry”, which is also `(0,0)` when there are none).

Why:
- This separates “text coordinate system” from “renderable layout entries”, fixing the root cause where inline objects collapse multiple text positions into one renderable entry.

## STEP 3: Add binary-search helpers for nearest caret keys
- Implement helpers using `collections::BinarySearchLambda` over the contiguous buffer behind `caretLayoutKeys`.
  - Guard the empty-container case before taking any contiguous-buffer address; return `-1` when there is no match.
  - Standardize the return values: helpers return the **caret value** (not an index).
  - `vint FindLayoutCaretLE(vint textPos);`  // largest key `<= textPos`, or `-1`
  - `vint FindLayoutCaretGT(vint textPos);`  // smallest key `> textPos`, or `-1`
- Use these helpers anywhere the mock needs to:
  - iterate layout entries within a text range (line rendering / hit-testing)
  - resolve geometry for a textPos that might not be a valid caret (inside inline-object span)

Why:
- The mock needs to “snap” from arbitrary text positions to the nearest available valid caret key without assuming 1:1 indexing.

## STEP 4: Update paragraph APIs to use caret-keyed layouts
Update each affected implementation in `UnitTestRemoteProtocol_Rendering` to avoid indexing a list by `textPos`:

- `Impl_DocumentParagraph_IsValidCaret`
  - Keep `0 <= caret <= text.Length()`.
  - Continue enforcing inline-object rule: reject `caretBegin < caret < caretEnd`.
  - Do NOT reject `caret == text.Length()`.

- `Impl_DocumentParagraph_GetNearestCaretFromTextPos`
  - Clamp `textPos` to `[0, text.Length()]`.
  - If `IsValidCaret(textPos)` return it.
  - Respect `GetNearestCaretFromTextPosRequest::frontSide` (fully specify fallback rules):
    - `frontSide == true`: prefer nearest valid caret `<= textPos`.
    - `frontSide == false`: prefer nearest valid caret `> textPos`.
    - If the preferred side doesn’t exist, fall back to the other side.
    - If both sides don’t exist (empty paragraph / only invalid spans), return `0`.
  - Special-case `textPos == text.Length()` when `frontSide == true` to return `text.Length()` even though there is no layout entry.

- Geometry / hit-testing APIs (`GetCaretBounds`, `GetInlineObjectFromPoint`, and any other API that walks characters in a line)
  - Iterate `caretLayoutKeys` restricted to `[line.startPos, line.endPos)` instead of looping `for (i=startPos; i<endPos; ++i)`.
  - When geometry is requested for `caret == text.Length()`, return a caret at the end of the last drawable entry on the last line (or `(0,0)` for an empty paragraph).

- Caret navigation (`GetCaret` for Home/End/Left/Right/Up/Down)
  - Home/End/Left/Right must never land inside inline-object spans (`caretBegin < caret < caretEnd`).
  - End/Home behavior must respect CRLF boundaries: End must not land between `\r` and `\n`.
  - Ensure Home/End produce valid carets in text space:
    - Home: smallest valid caret `>= line.startPos`.
    - End: prefer `line.endPos` if valid; otherwise largest valid caret `< line.endPos`.
  - For left/right stepping, ensure stepping never lands in invalid positions inside inline-object spans (either by using `IsValidCaret` checks or by snapping via nearest-caret helper).

Why:
- This removes every remaining path where a textPos is misused as a layout index, and it makes `frontSide` behavior deterministic.

## STEP 5: Add unit test `InlineObjectWithCaret`
- Edit existing test file `Test\GacUISrc\UnitTest\TestControls_Editor_InlineObject.cpp`.
- Create a new test case `InlineObjectWithCaret` next to the existing `InlineObject` test.
- Setup editor content with two lines:
  - `[Image]x[Image]`
  - `y[Image]z`
- Drive a full caret-navigation sequence using one `protocol->OnNextIdleFrame` per key press:
  - Use HOME / END to jump within a line.
  - Use LEFT / RIGHT to traverse across all valid carets.
  - Include edge no-op checks (LEFT at begin, RIGHT at end) and assert caret did/didn’t move as expected.
- Test stability/observability:
  - Navigation-only frames can be flaky if UI updates are idle/time driven; make caret movement observable per frame (e.g. insert a marker character after navigation or otherwise force a visible state change) so each `OnNextIdleFrame` is meaningful.
  - In addition to caret assertions, consider verifying editor text/model state (readback) to ensure navigation + insertion behaves as expected around inline objects.

Why:
- This reproduces the inline-object multi-textpos issue and verifies both (1) valid caret snapping and (2) correct line boundary behavior.

Optional extension (if needed):
- Add focused coverage for `GetNearestCaretFromTextPos(frontSide=...)` and edge cases like “only inline objects” and “empty paragraph”.

# !!!FINISHED!!!
