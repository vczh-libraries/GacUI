# !!!TASK!!!

# PROBLEM DESCRIPTION
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
- Style inheritance is a property of the document model's style resolution, not of `EditStyleName` itself. Inheritance behavior is verified indirectly through `RenameStyle` parent reference tests (Task 8) and by inspecting the document model's `styles` dictionary.

# UPDATES

# INSIGHTS AND REASONING

## Scope

This design covers only TASK No.3: adding unit tests in `Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp` for `GuiDocumentCommonInterface::EditStyleName` behavior on a **single paragraph**.

Out of scope:
- Multi-paragraph behavior (TASK No.4).
- RemoveStyleName (TASK No.5/6).
- SummarizeStyleName edge cases beyond what is needed to validate Task 3 (TASK No.7).
- RenameStyle and undo/redo verification (TASK No.8-10).

## Evidence from implementation (why these cases matter)

- `vl::presentation::GuiDocumentCommonInterface::EditStyleName` delegates into the document element through `EditStyleInternal(...)` (see `Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp`, around `GuiDocumentCommonInterface::EditStyleName`).
- The underlying model operation is `vl::presentation::DocumentModel::EditStyleName`, which uses `EditContainer` and calls `document_editor::AddStyleName(...)` (see `Source\Resources\GuiDocument_Edit.cpp`).
- `document_editor::AddStyleName` wraps intersecting `DocumentTextRun` nodes with `DocumentStyleApplicationRun` and stores `containerRun->styleName = styleName` without consulting `DocumentModel::styles` (see `Source\Resources\GuiDocumentEditor_AddContainer.cpp`).
  - This is the key evidence for the “unregistered style name still summarizes to that name” expectation.
- `GuiDocumentCommonInterface::SummarizeStyleName` swaps `begin/end` when `begin > end` (see `Source\Controls\TextEditorPackage\GuiDocumentCommonInterface.cpp`), so tests can safely use normal forward ranges; reversed ranges will be covered by TASK No.7.

## Test harness / structure (match existing patterns)

- Follow the same unit-test structure as `TestControls_Editor_RichText.cpp`:
  - `TooltipTimer timer;`
  - `GacUIUnitTest_SetGuiMainProxy(...)` with two `protocol->OnNextIdleFrame(...)` callbacks: one for setup, one for verification.
  - End by `window->Hide();`.
- Keep everything under existing top-level `TEST_CATEGORY(L"Styles")` in `TestControls_Editor_Styles.cpp`, and add:
  - `TEST_CATEGORY(L"EditStyleName")`
    - `TEST_CATEGORY(L"SingleParagraph")`
      - One `TEST_CASE` per bullet in “what to be done”.
- Use `GuiDocumentViewer` (`ref.Name="textBox"`, `EditMode="Editable"`) as already scaffolded in `resource_DocumentViewer`.

### Stable log identities / frames / lookups (avoid ambiguity)

- Each `TEST_CASE` must use its own stable `GacUIUnitTest_StartFast_WithResourceAsText` log identity string (keep these paths stable during future refactors):
  - `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_RegisteredStyle`
  - `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_PartialRange`
  - `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_UnregisteredStyle`
  - `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_AdjacentRanges`
  - `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_Overwrite`
- Keep the current minimal two-frame structure and give frames stable titles (e.g. `L"Init"`, `L"Verify"`). Ensure each frame does an observable UI change (the last frame can do `window->Hide()`).
- Re-find the window and control inside each `protocol->OnNextIdleFrame(...)` body (do not capture and reuse `textBox` pointers across frames).
- Since the XML uses `<DocumentViewer ref.Name="textBox" .../>`, use `FindObjectByName<GuiDocumentViewer>(...)`, and pass it to helpers that accept `GuiDocumentCommonInterface*`.
- If these tests are added to the existing `TestControls_Editor_Styles.cpp`, no project wiring change is needed; if a new test file is created instead, update `Test\GacUISrc\UnitTest\UnitTest.vcxproj` and `UnitTest.vcxproj.filters` (and potentially `UnitTest.vcxproj.user` `/F:` filters).

## Per-test design (single paragraph)

All tests use:
- `textBox->SetFocused();`
- `textBox->LoadTextAndClearUndoRedo(L"0123456789");` (or equivalent text for the case).
- The helper `RegisterStyle(textBox, styleName, parent, properties)` when testing a registered style.
- The helper `SummarizeName(textBox, begin, end)` and `TEST_ASSERT` on `Nullable<WString>`:
  - `TEST_ASSERT(summary); TEST_ASSERT(summary.Value() == WString::Unmanaged(L"...") );`
  - `TEST_ASSERT(!summary);` for null.

Note about coverage: Test 1 is the minimal “registered style baseline”; Test 2 is the range-boundary isolation case (outside-range assertions). Keeping both is fine as long as their intent is explicit.

### 1) Apply a registered style to a range

- Register `MyBold` with `MakeStyleWithBold(true)` and parent `WString::Empty`.
- Call `textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"MyBold"));`
- Verify:
  - `SummarizeName(textBox, 2, 5)` returns `MyBold`.

### 2) Apply a style to a partial range

- Register `MyBold` in this `TEST_CASE` (registration from Test 1 does not carry across test cases).
- Apply `MyBold` to `[2,5)`.
- Verify:
  - `SummarizeName(textBox, 0, 2)` is null.
  - `SummarizeName(textBox, 2, 5)` is `MyBold`.
  - `SummarizeName(textBox, 5, 10)` is null.

### 3) Apply an unregistered style name

- Do NOT register the style name.
- Apply `EditStyleName` with `UnregisteredStyle` to `[2,5)`.
- Verify:
  - `SummarizeName(textBox, 2, 5)` returns `UnregisteredStyle`.

Rationale reminder:
- `AddStyleNameVisitor` stores the name on `DocumentStyleApplicationRun` directly (no lookup in `document->styles`).

### 4) Apply different styles to adjacent ranges

- Register `StyleA` and `StyleB` in this `TEST_CASE` (recommended: give them distinct properties, e.g. bold=true vs bold=false).
- Apply `StyleA` to `[0,5)` and `StyleB` to `[5,10)`.
- Verify:
  - `SummarizeName(textBox, 0, 5)` is `StyleA`.
  - `SummarizeName(textBox, 5, 10)` is `StyleB`.
  - `SummarizeName(textBox, 0, 10)` is null.

### 5) Overwrite existing style name

- Register `StyleA` and `StyleB` in this `TEST_CASE`.
- Apply `StyleA` to `[0,10)`.
- Apply `StyleB` to `[3,7)`.
- Verify:
  - `SummarizeName(textBox, 0, 3)` is `StyleA`.
  - `SummarizeName(textBox, 3, 7)` is `StyleB`.
  - `SummarizeName(textBox, 7, 10)` is `StyleA`.

## Notes / Guardrails

- `GuiDocumentCommonInterface::EditStyleName` early-returns when `config.pasteAsPlainText` is true. `<DocumentViewer EditMode="Editable">` is compatible for these tests, but call out this precondition to avoid future confusion.
- Avoid `begin == end` in summarize checks: `DocumentModel::SummarizeStyleName` returns empty for empty ranges (see `Source\Resources\GuiDocument_Edit.cpp`).
- These tests only validate the **stored style-name runs** via `SummarizeStyleName`, not the resolved visual properties (since name-to-properties resolution depends on registered styles and rendering).

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# !!!FINISHED!!!

