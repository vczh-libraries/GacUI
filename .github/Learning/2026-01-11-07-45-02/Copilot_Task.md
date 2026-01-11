# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.6: Add TEST_CATEGORY for clipboard shortcuts

Add test cases for Ctrl+C/Ctrl+X/Ctrl+V, using the fake clipboard service in unit tests and validating single-line plain-text normalization on paste (CR/LF removed, styles ignored).

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones; keep the final callback minimal so the last logged frame is easy to inspect; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`.
- Follow the established scaffold from TASK No.2 (frame rule, clarified + learned): use a setup frame to focus the control and seed initial text via `protocol->TypeString(...)`; in the next `UnitTestRemoteProtocol::OnNextIdleFrame`, perform the key input(s) and the assertions for the resulting state in the same callback; schedule a next callback (except the last one) only when the current callback issues an input that will change UI; keep frame titles as short titles and do not rename existing ones (frame titles describe what happened in the previous frame); if an input is issued, add one more final frame whose only job is `window->Hide();` so the last logged frame stays minimal; build the test log identity as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`.
- Add `TEST_CATEGORY(L"Clipboard")` in `RunSinglelineTextBoxTestCases` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Keep adding cases inside the existing template-based shared test function (i.e. continue using `FindObjectByName<TTextBox>` in test code, and do not hardcode `GuiDocumentLabel`).
- Use `GetCurrentController()->ClipboardService()->WriteClipboard()` / `ReadClipboard()` in test code to set up and verify clipboard content.
- Add multiple `TEST_CASE`s:
  - `CtrlC_CopiesSelection_TextUnchanged`: create a selection, press `Ctrl+C`, assert textbox text unchanged and clipboard text equals the selection.
  - `CtrlX_CutsSelection_TextUpdated`: create a selection, press `Ctrl+X`, assert textbox text removed selection and clipboard text equals the removed substring.
  - `CtrlV_PastesAtCaret`: pre-fill clipboard text, place caret, press `Ctrl+V`, assert insertion at caret.
  - `CtrlV_ReplacesSelection`: pre-fill clipboard, create selection, press `Ctrl+V`, assert selection replaced.
  - `CtrlV_FlattensLineBreaks`: set clipboard text containing `\r\n` and/or `\n` (and optional double line breaks); paste and assert resulting textbox text contains no line breaks and joins segments without adding spaces (since `spaceForFlattenedLineBreak=false`).
  - `CtrlV_EmptyClipboard_NoChange`: ensure clipboard has no text, press `Ctrl+V`, assert text unchanged.

### rationale

- Clipboard shortcuts are core keyboard operations and are explicitly registered as shortcuts in `GuiDocumentCommonInterface`.
- Singleline + paste-as-plain-text behavior is easy to regress when document/clipboard handling changes; these tests pin expected normalization.

# UPDATES

# INSIGHTS AND REASONING

## Scope and intent

This design covers adding a new `TEST_CATEGORY(L"Clipboard")` under the existing template-based `RunSinglelineTextBoxTestCases<TTextBox>` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, focusing on keyboard clipboard shortcuts (Ctrl+C/Ctrl+X/Ctrl+V) and the single-line plain-text normalization rules on paste.

The tests are expected to be reusable for other `GuiDocumentCommonInterface`-derived editor controls by reusing the existing `TTextBox` template scaffold.

## Relevant implementation facts (evidence)

### Clipboard shortcuts are implemented as Ctrl+X/C/V commands

`vl::presentation::controls::GuiDocumentCommonInterface` wires up clipboard shortcuts in its constructor in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`:

- Ctrl+X calls `GuiDocumentCommonInterface::Cut`.
- Ctrl+C calls `GuiDocumentCommonInterface::Copy`.
- Ctrl+V calls `GuiDocumentCommonInterface::Paste`.

This means keyboard-driven clipboard behavior in tests should be validated by sending Ctrl-modified `VKEY` keystrokes via `UnitTestRemoteProtocol::KeyPress` and then asserting state through the `TTextBox` public surface (`GetText`, selection/caret APIs, and clipboard service APIs).

### Singleline textbox enforces plain-text paste and singleline layout

`vl::presentation::controls::GuiDocumentConfig::GetSinglelineTextBoxDefaultConfig` in `Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp` sets:

- `pasteAsPlainText = true` (clipboard document/image formats are ignored for paste, only text is relevant).
- `paragraphMode = GuiDocumentParagraphMode::Singleline`.
- `wrapLine = false`.
- `spaceForFlattenedLineBreak = false`.

Additionally, `GuiDocumentCommonInterface::SetSelectionText` uses `UserInput_FormatText(value, ..., config)` before editing the document (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`), meaning pasted text goes through the same formatting/normalization pipeline as typed text.

### Line breaks are flattened when paragraphMode != Paragraph

`GuiDocumentCommonInterface::UserInput_FormatDocument` joins lines/paragraphs when `config.paragraphMode != GuiDocumentParagraphMode::Paragraph`, and joins all paragraphs when `config.paragraphMode == GuiDocumentParagraphMode::Singleline` (`Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`).

This is the concrete reason why the `CtrlV_FlattensLineBreaks` test must expect `\r\n` and `\n` to be removed/flattened, and why it must also verify the no-space behavior when `spaceForFlattenedLineBreak = false`.

### Remote mode + unit tests rely on substituted services and frame-based assertions

The knowledge base describes remote-mode frame testing via `UnitTestRemoteProtocol::OnNextIdleFrame` and explains that each idle callback corresponds to a rendered frame with a descriptive label (`.github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md`). This aligns with the existing editor tests’ structure.

Remote mode uses service substitution for missing native services: `GuiRemoteController::ClipboardService()` returns `nullptr` (with an explicit comment “Use FakeClipboardService”) in `Source/PlatformProviders/Remote/GuiRemoteController.cpp`. `GuiInitializeUtilities()` substitutes `FakeClipboardService` as an optional service in `Source/Utilities/GuiUtilitiesRegistration.cpp`, and the substitution behavior is defined in `Source/NativeWindow/GuiNativeWindow.cpp` (optional substitutions are used only when the native controller provides `nullptr`).

Therefore the clipboard tests should interact with `GetCurrentController()->ClipboardService()` and expect an in-process clipboard implementation, not the OS clipboard.

## Proposed test design

### Test category layout

Add `TEST_CATEGORY(L"Clipboard")` under the existing `RunSinglelineTextBoxTestCases<TTextBox>` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.

- Keep log paths consistent with existing categories in that file: `Controls/Editor/<controlName>/Key/Clipboard_<CASE>`.
- Reuse the established scaffold already present in that file: focus the textbox, seed deterministic text (e.g. `L"0123456789"`), then do an action frame containing both the input and the assertions, ending with a minimal final frame that hides the window.

### Selection and caret setup

For Ctrl+C / Ctrl+X validation, the design relies on an explicit, deterministic selection. The most stable approach is:

- Use `TTextBox::SetCaret(TextPos(...), TextPos(...))` to create a selection range directly (this is already used in existing navigation tests in the same file).

This avoids variability in shift-selection behavior and keeps the clipboard-focused tests isolated from the navigation tests.

### Clipboard verification strategy

All cases should validate clipboard content using the controller clipboard APIs:

- Before Ctrl+V: set clipboard text via `GetCurrentController()->ClipboardService()->WriteClipboard()`, `SetText(...)`, `Submit()`.
- After Ctrl+C / Ctrl+X: read clipboard via `ReadClipboard()` and validate `ContainsText()`/`GetText()`.

Because singleline config has `pasteAsPlainText = true`, it is sufficient (and preferred) to use text-only clipboard operations in the tests.

### Proposed TEST_CASE coverage

1. `CtrlC_CopiesSelection_TextUnchanged`
	- Setup: seed text, set a known selection.
	- Action: Ctrl+C.
	- Assert: `TTextBox::GetText()` unchanged; `ReadClipboard()->GetText()` equals the selected substring.

2. `CtrlX_CutsSelection_TextUpdated`
	- Setup: seed text, set a known selection.
	- Action: Ctrl+X.
	- Assert: clipboard equals the removed substring; textbox text equals original with that span removed.

3. `CtrlV_PastesAtCaret`
	- Setup: seed text, set caret to a known position, pre-fill clipboard with a marker string.
	- Action: Ctrl+V.
	- Assert: textbox text contains inserted string at that caret position.

4. `CtrlV_ReplacesSelection`
	- Setup: seed text, set selection, pre-fill clipboard.
	- Action: Ctrl+V.
	- Assert: selection is replaced by clipboard text (equivalent to `SetSelectionText(clipboard)` behavior).

5. `CtrlV_FlattensLineBreaks`
	- Setup: pre-fill clipboard text containing mixed `\r\n` and `\n` (optionally with double line breaks).
	- Action: Ctrl+V.
	- Assert: resulting textbox text contains no line breaks and the segments are concatenated without inserted spaces, consistent with `spaceForFlattenedLineBreak = false`.

6. `CtrlV_EmptyClipboard_NoChange`
	- Setup: ensure clipboard has no text.
	- Action: Ctrl+V.
	- Assert: textbox text unchanged.

## Key risk / prerequisite: FakeClipboardWriter must support test-driven clipboard setup

The clipboard tests require deterministic read/write behavior in remote mode, including the ability to clear the clipboard to “no text” to validate `CtrlV_EmptyClipboard_NoChange`.

The current fake clipboard implementation (`Source/Utilities/FakeServices/GuiFakeClipboardService.cpp`) uses `FakeClipboardService` as the substituted clipboard service in remote mode. For the proposed tests to be implementable and reliable, `INativeClipboardWriter` returned by `FakeClipboardService::WriteClipboard()` must:

- Allow `SetText(...)` + `Submit()` to reliably populate clipboard text.
- Allow `Submit()` to represent “clear clipboard” (so the clipboard can be empty with `ContainsText() == false`).

If this behavior is missing, clipboard keyboard tests will either be flaky (dependent on prior test state) or impossible to write without adding new testing-only backdoors.

## Alternatives considered

### A) Avoid direct clipboard service calls and test only UI text changes

Pros:
- No dependency on clipboard service correctness.

Cons:
- Cannot validate the primary contract of Ctrl+C/Ctrl+X (that clipboard receives selection text).
- Cannot validate the `CanPaste()` gating in `GuiDocumentCommonInterface::CanPaste()` which depends on `INativeClipboardReader::ContainsText()`.

### B) Use `FakeClipboardService` directly (downcast) to control internal state

Pros:
- Would allow explicit clearing.

Cons:
- Breaks encapsulation; couples tests to a specific fake implementation.
- Conflicts with the repo’s goal of sharing tests across multiple controls and potentially different platform providers.

Recommendation: stick to the public clipboard service APIs (preferred by Task No.6), and ensure the fake clipboard service behaves as a compliant in-process implementation.

# !!!FINISHED!!!
