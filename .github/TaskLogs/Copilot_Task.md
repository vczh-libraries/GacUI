# !!!TASK!!!

# PROBLEM DESCRIPTION
`UnitTestRemoteProtocol_Rendering` potential bug:

`DocumentParagraphLineInfo::(start|end)Pos` supposed to be text positions but it actually maps to `DocumentParagraphState::characterLayouts` which treat an inline object with one character, therefore `(start|end)Pos` does not map to `caret` or `textPos`.

Improve the data structure to correct the mapping. Here are my thought but feel free to make your own decision:
- Change from characterLayouts from a list to a map.
- The key would be valid carets.
  - According to the unit test only mock implementation, an inline object covers multiple consecutive characters, only the first one is a valid caret.
  - Even though text.Length() is a valid caret but the item should not appear on the map, because there is no character to render after it anyway.
  - `BinarySearchLambda` could help locating a textpos even when it is not a valid caret, you can use this function to find the largest available caret that smaller or equal to the textpos. Make a helper function for it.
  - Therefore you can improve the implementation of `UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_IsValidCaret`.

Add a test case with to lines `[Image]x[Image]` and `y[Image]z`, pressing home/end/left/right to navigate all caret positions.
- Find the `InlineObject` test case.
- Make a new test case in the same test file.
- You can name it `InlineObjectWithCaret`.
- A new frame must be created to press one caret moving key. And you should also check if the caret is updated or not. Remember that the frame name describes what has been done in the previous frame.

Another bug could be in UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos:
- `GetNearestCaretFromTextPosRequest::frontSide` is not used.
- When it is true, it should prefer the caret <= this textpos over another one.
- When it is false, it should prefer the caret > this textpos over another one.

# UPDATES

## UPDATE

The document correctly diagnoses the bug as a coordinate-system mismatch: `DocumentParagraphLineInfo::{startPos,endPos}` are **text positions**, while the current `characterLayouts` is effectively indexed by **layout entries** (inline objects collapse multiple text positions into one entry), so using text positions to index layouts breaks caret and hit-testing logic.

Proceed with the proposed fix of separating “text positions” from “renderable layout entries” by storing layouts keyed by **valid caret positions** (textPos values where a drawable item begins), and using an ordered key list for range iteration + binary search.

Actionable feedback to incorporate:

- Clarify semantics around `text.Length()`:
  - It must remain a valid caret for navigation/editing.
  - It should not have a layout entry (no glyph after it), so geometry queries must handle it explicitly (e.g., position at end of the last drawable item).
  - Document the distinction: “valid caret” is not the same as “has drawable layout entry”.

- If keeping both `Dictionary<vint, Layout>` and `SortedList<vint>` (same keys), explicitly document and enforce the synchronization invariant; consider adding a small helper to update both atomically.

- Implement helpers using `collections::BinarySearchLambda` (guard empty buffers) to locate nearest caret keys:
  - largest key `<= textPos` and smallest key `> textPos`.
  - Use them for line-range iteration (find first key `>= line.startPos`, iterate while `< line.endPos`) and for Home/End behavior.

- Fix `Impl_DocumentParagraph_GetNearestCaretFromTextPos` to respect `frontSide`:
  - `frontSide==true`: prefer caret `<= textPos`.
  - `frontSide==false`: prefer caret `> textPos`.
  - If preferred side doesn’t exist, fall back to the other side; handle empty paragraphs.

- Define consistent behavior when geometry APIs are called with invalid carets (inside inline-object span): snap to nearest valid caret, return empty bounds, or assert—choose one consistent with existing mock error-handling.

- Testing additions should be explicit and observable per unit-test learnings:
  - Add `InlineObjectWithCaret` near the existing inline-object editor tests with two lines `[Image]x[Image]` and `y[Image]z`.
  - Drive Home/End/Left/Right across all valid caret positions (including no-op edges) with one movement per frame and assertions on caret begin/end.
  - Consider typing marker characters (where appropriate) to make caret placement observable in the document content.
  - Ensure frame titles describe the previous frame’s action.
  - Verify CRLF/newline behavior doesn’t allow `End` to land between `\r` and `\n`.

- If any new test files are added (vs. editing an existing one), update `UnitTest.vcxproj` and `.filters` accordingly.

# INSIGHTS AND REASONING

## Context: What this code is and why it matters

The suspected bug is in the *unit-test-only remote renderer mock* (`vl::presentation::unittest::UnitTestRemoteProtocol_Rendering` in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering_Document.cpp`). It provides paragraph layout and caret services for editor control unit tests via the remote protocol:

- `DocumentParagraphLineInfo::{startPos,endPos}` are explicitly described as **text positions** (`GuiUnitTestProtocol_Rendering.h`).
- The mock layout builder currently constructs `DocumentParagraphState::characterLayouts` as a **linear list indexed by something that is treated like a text index**.
- Inline objects are currently modeled as one `DocumentParagraphCharLayout` item with `length > 1`, i.e. the code *skips* text indices while still emitting only one layout item.

As a result:

- `lines[].{startPos,endPos}` are produced from `lineRanges` built on the loop variable `i` (text index).
- But later code often uses `startPos/endPos` to index `characterLayouts` (layout index), assuming `layoutIndex == textPos`.
- This assumption breaks when inline objects cover multiple consecutive characters in the text (only the first one has a rendered layout entry).

This mismatch can cause incorrect behavior for:

- caret-to-geometry (`GetCaretBounds`) because it enumerates `characterLayouts` and expands `length` into multiple caret boundaries, but its internal counter `i` is not a true text position when layout items represent multiple text positions.
- caret navigation (`GetCaret`, esp. up/down that scans between `line.startPos` and `line.endPos` and indexes into `characterLayouts`).
- point-to-inline-object (`GetInlineObjectFromPoint`) which iterates textPos ranges but reads `characterLayouts[i]`.

## Observed API semantics from existing documentation

From `manual\gacui\components\controls\editor\home.md`:

- Caret positions are in `TextPos(row, column)`.
- “For non-character objects, each of them count as one character.” (real control semantics)

However, the unit-test mock additionally states inline object run ranges are `[caretBegin, caretEnd)` and that only `caretBegin` is a valid caret (existing `Impl_DocumentParagraph_IsValidCaret` checks `caretBegin < caret < caretEnd` as invalid).

Therefore the mock needs *two concepts*:

- **Text positions**: the raw index into `state.text` (including positions inside an inline object placeholder range).
- **Valid carets**: a subset of text positions that the caret can land on.

The bug described in the request is essentially that the mock is mixing these two with a single index.

## Proposed design: separate textPos from layout entries (caret-indexed map)

### Key decision

Replace `DocumentParagraphState::characterLayouts : List<DocumentParagraphCharLayout>` with a structure keyed by **valid caret (textPos)**, so that all geometry queries are done against a stable textPos coordinate system.

A practical, minimal-to-reason-about structure for the mock is:

- `Dictionary<vint, DocumentParagraphCharLayout> caretLayouts;` where key is a **valid caret position** that also has a **drawable layout entry** following it.
  - For normal text, for each character at text index `i`, add an entry for caret `i`.
  - For inline objects spanning `[caretBegin, caretEnd)`, add only one entry at `caretBegin` (the first text position).
  - Do **not** add an entry at `text.Length()` (there is no glyph/object after the end).

Additionally keep a sorted list of keys to support “nearest caret” and “line scanning” efficiently:

- `SortedList<vint> caretLayoutKeys;` (same keys as the dictionary).

Important invariant: `caretLayouts.Keys()` and `caretLayoutKeys` must always contain exactly the same set of keys. To prevent drift, update them via a tiny helper (e.g. `AddCaretLayout(caret, layout)` / `ClearCaretLayouts()`) that mutates both atomically.

### Semantics: “valid caret” vs “has a drawable layout entry”

These are not the same thing in this mock:

- `0..text.Length()` are *potential* caret positions in text space.
- A **valid caret** is a text position that caret navigation can land on (per inline-object rule: only `caretBegin` is valid inside an inline object run).
- A **drawable layout entry** exists only for carets that have a glyph/object after it; therefore `text.Length()` is a valid caret for navigation/editing, but it has no layout entry.

Any geometry API that needs a drawable entry must handle `text.Length()` explicitly (e.g. use the end of the last drawable item, or `(0, lastLineHeight)` when the paragraph is empty).

This matches the request’s “map keyed by valid carets” and also ensures we can binary search.

### Line info remains in textPos

Keep `DocumentParagraphLineInfo::{startPos,endPos}` as text positions (as the header already claims). Then any code needing to iterate visual items in a line should not do `for (i=startPos; i<endPos; ++i) layout = characterLayouts[i]`, but instead iterate the subset of caret keys in that line.

### Helper: locate layout at/near a textPos

Add helpers that map an arbitrary `textPos` to nearby caret keys in `caretLayoutKeys`:

- `vint FindLayoutCaretLE(const DocumentParagraphState& state, vint textPos)`:
  - returns the largest caret key `<= textPos`.
  - if none, return `-1`.
- `vint FindLayoutCaretGT(const DocumentParagraphState& state, vint textPos)`:
  - returns the smallest caret key `> textPos`.
  - if none, return `-1`.

Implement both using `collections::BinarySearchLambda` over the contiguous buffer behind `caretLayoutKeys` and **guard empty** before taking `&caretLayoutKeys[0]` (see `KnowledgeBase\Learning.md`).

These helpers are used for `frontSide` selection and for iterating line ranges by caret keys (instead of by raw text positions).

## Fixing `IsValidCaret`

Currently `Impl_DocumentParagraph_IsValidCaret` validates by checking if the caret is inside any inline object run. With the new caret-keyed layout list, we can tighten this:

- Accept `0..text.Length()` range check as today.
- Still reject `caretBegin < caret < caretEnd` for inline objects (keeps the “only first is valid” rule).
- Optionally also reject carets that are past the last valid caret key when the caller expects a caret with geometry; but **do not change** `text.Length()` validity because it is a legitimate caret in editor semantics.

Design note: validity for editing/navigation and existence of a drawable layout entry are different; keep `IsValidCaret` aligned with navigation semantics, not renderability.

## Fixing `GetNearestCaretFromTextPos(frontSide)`

`GuiRemoteGraphicsParagraph` (real remote client) forwards `frontSide` (see `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`), but the mock implementation currently ignores it and returns `textPos`.

Implement the mock behavior in terms of **valid carets** (navigation semantics), not “has layout entry”:

- First clamp `textPos` to `[0, text.Length()]`.
- If `state->IsValidCaret(textPos)` (per inline-object rule), return `textPos`.
- Otherwise choose based on `frontSide`:
  - `frontSide == true`: prefer the nearest valid caret `<= textPos`.
  - `frontSide == false`: prefer the nearest valid caret `> textPos`.

Implementation detail:

- Use the helper searches on `caretLayoutKeys` as a fast approximation, but remember `text.Length()` is a valid caret without a key. So when `textPos == text.Length()` and `frontSide == true`, return `text.Length()` directly.
- If the preferred side does not exist, fall back to the other side; for empty paragraphs, return 0.

This satisfies the request’s stated preference rule and makes end-of-text behavior explicit.

## Updating other paragraph APIs to use caretLayouts

### `GetCaretBounds`

Today it enumerates `characterLayouts` and expands `layout.length` to add multiple boundaries. With caretLayouts keyed by caret positions, compute caret bounds in **textPos space** without ever treating a textPos as a list index:

- Ensure callers only request geometry for **valid carets** (the mock should already enforce this via `IsValidCaret` / nearest-caret snapping).
- For a given caret `c`:
  - If `c == text.Length()`: return a caret bound at the end of the last drawable item on the last line (or `(0,0)` on an empty paragraph).
  - Otherwise resolve `layoutCaret = FindLayoutCaretLE(state, c)` and use its layout entry for the line/x metrics.

This makes `text.Length()` explicit and avoids manufacturing bounds for invalid carets inside inline-object spans.

If any API currently needs to enumerate drawable items to build aggregate bounds, iterate `caretLayoutKeys` (layout entries) rather than `0..text.Length()`.

### `GetCaret` navigation

- Left/right: use `IsValidCaret` (inline-object rule) to skip invalid positions inside inline-object spans.
- Up/down: compute xOffset from the current caret’s geometry (resolve via `FindLayoutCaretLE` or special-case `text.Length()`), then choose the closest caret key in the target line.
- Home/end: treat `line.startPos/endPos` as **text positions** and snap to the nearest valid caret on that side:
  - Home: smallest valid caret `>= line.startPos` (or `line.startPos` if already valid).
  - End: prefer `line.endPos` if it is a valid caret; otherwise choose the largest valid caret `< line.endPos`.

This ensures Home/End never lands inside inline-object internal text positions.

### `GetInlineObjectFromPoint`

The iteration `for (i=line.startPos; i<line.endPos; ++i)` must be replaced by iterating caret keys in that line, so the code never indexes a missing layout.

## Unit test design addition (requested)

Add a new unit test case `InlineObjectWithCaret` next to the existing inline-object editor test in `Test\GacUISrc\UnitTest\TestControls_Editor_InlineObject.cpp` (do not add a new file).

Test goal: cover caret navigation across inline object boundaries where inline objects cover more than one text position internally and ensure the mock respects “only caretBegin is valid”.

- Content: two lines:
  - `[Image]x[Image]`
  - `y[Image]z`
- Test structure (per unit-test learnings):
  - Focus the control before sending keyboard input.
  - Use one `OnNextIdleFrame` per key press, and ensure each frame causes a UI update (navigation-only frames can be risky; if needed, combine key press + caret assertion + a small observable update in the same frame, consistent with existing editor tests).
  - Assert caret begin/end after each action; include no-op edges (LEFT at begin, RIGHT at end).
  - Consider typing a marker character or otherwise making caret position observable if pure navigation does not trigger a UI update reliably (see `Learning_Testing.md`: frames must change UI; caret-only may not update when time is hijacked).

Frame titles must describe what happened in the previous frame.

## Alternatives considered

1) Keep `characterLayouts` as list but also maintain a separate `textPos->layoutIndex` mapping.
- Pros: less refactor in layout builder.
- Cons: still easy to accidentally index the wrong coordinate system; more places to keep in sync.

2) Expand `characterLayouts` to have one entry per text position (duplicate the same geometry for inline-object internal positions).
- Pros: minimal downstream code changes.
- Cons: conflicts with the “only first caret is valid” rule and inflates data; still ambiguous semantics.

The caret-keyed dictionary + sorted key list is the clearest and aligns with the request.

# AFFECTED PROJECTS
- Build the solution in folder REPO-ROOT\Test\GacUISrc
  - Run Test Project UnitTest

# !!!FINISHED!!!
