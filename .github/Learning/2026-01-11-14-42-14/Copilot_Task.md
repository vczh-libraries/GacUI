# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.8: Add TEST_CATEGORY for typing and char-input filtering

Add test cases for the `OnCharInput` path: normal typing, tab input, control-modified typing being ignored, and newline characters being flattened/ignored as appropriate for single-line text.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): structure each interaction as an input frame followed by a verification frame.
  - Input frame: issue one or more inputs (`TypeString`, `KeyPress`, `_KeyDown`, `_KeyUp`, etc) and do not assert.
  - Verification frame: perform all assertions for the result *and* ensure the callback triggers a UI update (e.g. `window->SetText(textBox->GetText());`) so the harness accepts the frame.
  - Schedule another frame (except the last one) only when the current frame issues an input that will change UI in the next frame.
  - Keep frame titles short, and let titles describe what happened in the previous frame.
  - Keep the last frame minimal and hide-only (`window->Hide();`).
  - Build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Typing_<CASE>")`.
- When asserting that a `WString` contains no line breaks, use `WString::IndexOf(L'\r')` and `WString::IndexOf(L'\n')` (character literals) instead of `L"\r"` / `L"\n"` to match the correct overload.
- Add `TEST_CATEGORY(L"Typing")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Keep adding cases inside the existing template-based shared test function (i.e. continue using `FindObjectByName<TTextBox>` in test code, and do not hardcode `GuiDocumentLabel`).
- Add multiple `TEST_CASE`s:
  - `TypeString_InsertsPlainText`: use `protocol->TypeString(L\"...\")` and assert text updates.
  - `TypeString_InsertsTab_WhenAcceptTabInput`: type `L\"\\t\"` and assert a tab character appears in text.
  - `TypeString_IgnoresWhenCtrlPressed`: use `protocol->_KeyDown(KEY_CONTROL)` + `TypeString(...)` + `_KeyUp(KEY_CONTROL)` and assert no text change.
  - `TypeString_NewlineCharsFlattened`: type a string containing `\\n` and assert the resulting text contains no line breaks (flattened by formatting); also verify `\\r` (KEY_RETURN) is ignored by char input.
  - `TypeString_ReplacesSelection`: create a selection (e.g. Shift+Arrow or `SelectAll()`), then `TypeString(...)` and assert replacement.

### rationale

- `GuiSinglelineTextBox` is configured to accept only plain text; char-input filtering + flattening is the core guarantee.
- Explicitly testing tab and ctrl-modified typing prevents regressions in `OnCharInput` gating logic.

# UPDATES

# INSIGHTS AND REASONING

## What “typing” means in the codebase

Keyboard input reaches editor controls via the hosted/remote input pipeline:

- `vl::presentation::GuiGraphicsHost::OnCharInput(...)` raises `previewCharInput` / `charInput` events on compositions.
- `vl::presentation::controls::GuiDocumentCommonInterface` attaches to the focusable composition and handles character input in `GuiDocumentCommonInterface::OnCharInput`.

For this task, the key behavior is implemented by `GuiDocumentCommonInterface::OnCharInput` (see Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp). It only edits text when all of these are true:

- The control is visually enabled.
- `editMode == GuiDocumentEditMode::Editable`.
- The incoming character is not treated as a “special key” for this path:
  - `KEY_ESCAPE`, `KEY_BACK`, `KEY_RETURN` are rejected.
  - `KEY_TAB` is rejected unless `GetAcceptTabInput()` is true.
- `arguments.ctrl` is false.

When accepted, it edits by calling:

- `EditText(caretBegin, caretEnd, caretPreferFrontSide, [single-character string])`

This directly implies two important test axes for the Typing category:

1. Positive path: accepted characters are inserted/replacing the selection.
2. Negative path: characters are ignored when they should be filtered (Ctrl held, Tab not accepted, Return/Back/Esc).

## Why newline handling is part of “typing” for singleline

`GuiSinglelineTextBox` is a `GuiDocumentLabel` configured with `GuiDocumentConfig::GetSinglelineTextBoxDefaultConfig()` (see Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp):

- `paragraphMode = GuiDocumentParagraphMode::Singleline`
- `pasteAsPlainText = true`
- `spaceForFlattenedLineBreak = false`

The contract of formatting in single-line mode is documented in `GuiDocumentCommonInterface::UserInput_FormatText` comments (see Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h):

- In Singleline mode, it “removes line breaks in each paragraph and concatenates all paragraphs to one paragraph”, while considering `spaceForFlattenedLineBreak`.

Therefore, even if a `\n` character makes it through the `OnCharInput` filter (it is not `KEY_RETURN`), the single-line formatting path is expected to remove line breaks and concatenate without adding spaces (because `spaceForFlattenedLineBreak=false`). The observable post-condition for a `GuiSinglelineTextBox` is:

- The resulting `GetText()` contains neither `L'\r'` nor `L'\n'`.

Additionally, `KEY_RETURN` is explicitly rejected in `OnCharInput`, so a `\r`-like char input should never produce a newline in the text in the first place.

## AcceptTabInput and why “Tab typing” must be tested

`OnCharInput` has an explicit Tab gate:

- `(arguments.code != KEY_TAB || documentControl->GetAcceptTabInput())`

`GuiDocumentLabel` enables it by default (`SetAcceptTabInput(true)` in its constructor; see Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp), so `GuiSinglelineTextBox` inherits “Tab inserts a tab character” behavior.

Testing this is valuable because Tab has a competing meaning in the UI framework (focus navigation), and the framework routes Tab based on `GetAcceptTabInput()` (see Source/Application/GraphicsHost/GuiGraphicsHost_Tab.cpp).

## Frame-based unit test constraints (why the 2-frame pattern is needed)

GUI unit tests run in “remote mode” and are driven frame-by-frame through `UnitTestRemoteProtocol::OnNextIdleFrame(...)` (see Source/UnitTestUtilities/GuiUnitTestProtocol.h). The driver enforces that rendering must occur between frames:

- `ProcessRemoteEvents()` checks that the logged rendering frame index has advanced since the last idle-frame callback.

This is why the task requires splitting each interaction into:

- Input frame: issue input that causes a UI change and therefore a render.
- Verification frame: assert state after the render; if more frames follow, ensure an observable UI update occurs so there will be another render.

This requirement is also consistent with the knowledge base description of “Frame-Based Testing” for `OnNextIdleFrame` (see .github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md).

## Proposed test design (Typing category)

All test cases should follow the existing reusable template-based test harness in Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp:

- Add a new `TEST_CATEGORY(L"Typing")` inside `RunSinglelineTextBoxTestCases<TTextBox>(...)`.
- Keep using `FindObjectByName<TTextBox>(window, L"textBox")` so the same test cases can later run against other `GuiDocumentCommonInterface`-derived controls.
- Keep creating a local `TooltipTimer` at the beginning of each `TEST_CASE` to keep time deterministic.

Test case matrix (minimal but representative):

1. `TypeString_InsertsPlainText`
	- Input: focus + seed (e.g. `L""`), then `TypeString(L"abc")`.
	- Verify: `GetText() == L"abc"`.

2. `TypeString_InsertsTab_WhenAcceptTabInput`
	- Input: ensure `GetAcceptTabInput()` is true (default) and `TypeString(L"\t")`.
	- Verify: `GetText()` contains `L'\t'`.

3. `TypeString_IgnoresWhenCtrlPressed`
	- Input: `_KeyDown(KEY_CONTROL)`, `TypeString(L"abc")`, `_KeyUp(KEY_CONTROL)`.
	- Verify: text unchanged.

4. `TypeString_NewlineCharsFlattened`
	- Input: `TypeString(L"A\nB\rC")` (or equivalent).
	- Verify: `IndexOf(L'\n') == -1` and `IndexOf(L'\r') == -1`, and text equals `L"ABC"` (no spaces expected because `spaceForFlattenedLineBreak=false`).

5. `TypeString_ReplacesSelection`
	- Input: seed `L"012345"`, create selection (via `SelectAll()` or Shift+Arrow), then `TypeString(L"X")`.
	- Verify: selection is replaced (e.g. `L"X"` after SelectAll, or a smaller replacement if selecting a range).

## Risks / things to validate during implementation

- What `UnitTestRemoteProtocol::TypeString(...)` emits for line breaks (whether it emits `\r`, `\n`, or both) impacts the exact expected string; the assertion should primarily enforce “no line breaks remain” plus an exact normalized value if the emitted sequence is known.
- Some state updates can be synchronous (text changes immediately) even before a render; however, the unit test driver still requires a render to happen between frames, so the “verification frame forces an update when needed” rule must be followed to avoid `No rendering occured after the last idle frame.` failures.

# !!!FINISHED!!!
