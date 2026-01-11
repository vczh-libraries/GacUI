# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.5: Add TEST_CATEGORY for deletion and Enter keys

Add test cases for editing keys handled in `ProcessKey`: Backspace, Delete, Enter, and Ctrl+Enter, including behavior with and without a selection.

In `GuiSinglelineTextBox`, Enter is processed but line breaks are flattened by configuration (`paragraphMode=Singleline`, `spaceForFlattenedLineBreak=false`), so no newline should remain in the resulting text.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones; keep the final callback minimal so the last logged frame is easy to inspect; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Deletion_<CASE>")`.
- Add `TEST_CATEGORY(L"Deletion")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Add multiple `TEST_CASE`s:
	- `Backspace_DeletesPreviousChar`: set caret after a known character; press `KEY_BACK`; assert one character removed and caret moved appropriately.
	- `Delete_DeletesNextChar`: set caret before a known character; press `KEY_DELETE`; assert one character removed.
	- `Backspace_DeletesSelection` and `Delete_DeletesSelection`: create a selection via `Shift+Arrow` or `SelectAll()`, then press Backspace/Delete; assert selection removed.
	- `BackspaceAtStart_NoChange` and `DeleteAtEnd_NoChange`: boundary behavior should not change text.
	- `Enter_DoesNotInsertNewline_ButCanDeleteSelection`: with no selection, press `KEY_RETURN` and assert text unchanged; with a selection, press `KEY_RETURN` and assert selection removed.
	- `CtrlEnter_DoesNotInsertNewline_ButCanDeleteSelection`: same as above but using `KeyPress(KEY_RETURN, ctrl=true, ...)`.

### rationale

- Backspace/Delete/Enter are the primary editing keys beyond typing; they have subtle selection-dependent behavior.
- Single-line newline-flattening is a key "plain text" guarantee of `GuiSinglelineTextBox` and needs coverage.

# UPDATES

(none)

# INSIGHTS AND REASONING

## Context and existing scaffold

- The target file already contains a reuse-oriented scaffold: `RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` and a `TEST_FILE` wrapper in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Every existing `TEST_CASE` constructs a local `TooltipTimer` and uses `GacUIUnitTest_SetGuiMainProxy` with `UnitTestRemoteProtocol::OnNextIdleFrame` to execute frame-based UI interactions.
- Remote mode and `OnNextIdleFrame` are designed for frame-by-frame logging and verification; each idle frame corresponds to a rendering frame with a descriptive label (see `.github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md`).

## What we are actually testing (source evidence)

### Deletion keys are implemented in the shared core

- The behavior under test is implemented in `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`.
- `VKEY::KEY_BACK` and `VKEY::KEY_DELETE` share the same overall algorithm:
	- If there is no selection (`begin == end`), they first extend a one-character selection by calling `ProcessKey(VKEY::KEY_LEFT, true, false)` or `ProcessKey(VKEY::KEY_RIGHT, true, false)`.
	- They then call `EditText(caretBegin, caretEnd, ..., text)` with an empty `collections::Array<WString>` to delete the selected range.
	- Boundary behavior (caret at start for Backspace, caret at end for Delete) naturally results in “no selection created => no text removed”, so the expected outcome is “no change”.

### Enter/Ctrl+Enter perform “replace selection with line break intent”, which Singleline config flattens

- In the same `ProcessKey` implementation, `VKEY::KEY_RETURN` triggers `EditText(...)` if editable:
	- For Ctrl+Enter, it passes a one-item array containing `L"\r\n"`.
	- For Enter, it passes a two-item array (used as a “paragraph break intent”).
- In `vl::presentation::controls::GuiDocumentCommonInterface::EditText` (same file), when `config.paragraphMode != GuiDocumentParagraphMode::Paragraph`, it formats the input with `UserInput_FormatText(paragraphTexts, config)` before editing.
- For single-line text boxes, the evaluated config comes from `vl::presentation::controls::GuiDocumentConfig::GetSinglelineTextBoxDefaultConfig` in `Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp`, which sets:
	- `paragraphMode = GuiDocumentParagraphMode::Singleline`
	- `spaceForFlattenedLineBreak = false`
	- `pasteAsPlainText = true`
- In `vl::presentation::controls::GuiDocumentCommonInterface::NotifyParagraphUpdated` (same cpp), singleline mode enforces the invariant that the document must have at most one paragraph, and line breaks inside paragraphs are joined when not in Paragraph mode.

Design implication:
- Enter/Ctrl+Enter should not leave any newline in the resulting text for a single-line textbox.
- Even when newline insertion is flattened to “insert nothing”, the operation still behaves like “replace selection with nothing”, so Enter/Ctrl+Enter should delete an active selection.

## Proposed high-level test design

### Add a new category

- Add `TEST_CATEGORY(L"Deletion")` inside `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Each `TEST_CASE` follows the established pattern:
	- Construct a local `TooltipTimer` to keep time-dependent behavior deterministic.
	- Frame 1 (`Init`): focus the `GuiDocumentLabel`-typed object and seed a known baseline text via `protocol->TypeString(L"0123456789")`.
	- Frame 2 (case-specific label): set caret/selection to the target scenario, send the key(s) via `protocol->KeyPress(...)`, and assert the resulting state in the same callback.
	- End by hiding the window (keep the last callback minimal).

### State and assertions

- Prefer asserting only observable, stable outcomes:
	- `GuiDocumentLabel::GetText()` for text mutations.
	- `GuiDocumentLabel::GetSelectionText()` to verify whether the selection was removed.
	- `GuiDocumentLabel::GetCaretBegin()` / `GetCaretEnd()` where caret movement is deterministic (especially for the “no selection => delete one char” cases).
- Selection setup:
	- Because selection movement itself is already covered by the Navigation category, Deletion tests can set a selection directly using `SetCaret(TextPos, TextPos)` for stability.
	- For full-range selection coverage, use `vl::presentation::controls::GuiDocumentCommonInterface::SelectAll` (available through the `GuiDocumentLabel` API surface) as an explicit selection creation path.

### Concrete coverage mapping to test cases

- Backspace, no selection:
	- Place caret after a known character (e.g. column 5), press `VKEY::KEY_BACK`, assert text becomes `L"012346789"` (removes `5`’s predecessor) or the equivalent expected removal based on chosen caret placement; assert selection becomes empty.
- Delete, no selection:
	- Place caret before a known character (e.g. column 5), press `VKEY::KEY_DELETE`, assert the next character is removed; assert selection becomes empty.
- Backspace/Delete with selection:
	- Create a selection spanning a known substring (e.g. select `L"234"`), press Backspace/Delete, assert that substring is removed; assert selection becomes empty.
- Boundaries:
	- Backspace at start (caret at column 0) produces no change.
	- Delete at end (caret at `Length()`) produces no change.
- Enter and Ctrl+Enter:
	- With no selection: press `VKEY::KEY_RETURN` (and ctrl variant) and assert text unchanged and selection remains empty.
	- With a selection: press Enter / Ctrl+Enter and assert selection removed, and the resulting text contains no newline characters.

### Logging identity

- Keep per-case log paths consistent with the existing file’s scheme:
	- `Controls/Editor/` + controlName + `/Key/Deletion_<CASE>`
- This preserves log discoverability and matches the established category naming conventions already used in the Navigation category.

## Risks and non-goals

- These tests should not depend on clipboard services, IME composition, or rendering differences; they only validate text/selection/caret semantics exposed by `GuiDocumentLabel`.
- If any caret-position assertion turns out to be unstable across renderers/hosts, the design allows downgrading that assertion to only text + selection correctness (the primary contract for deletion and singleline newline flattening).

# !!!FINISHED!!!
