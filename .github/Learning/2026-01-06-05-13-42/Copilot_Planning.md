# !!!PLANNING!!!
# UPDATES
## UPDATE
In the first version you don't need to make TypeString or TypeText handle keydown or keyup, just sending multiple IOChar. And therefore only one TypeString function is enough.
# IMPROVEMENT PLAN
## STEP 1: Expose minimal typing helper
- Update `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h` to declare a single `void TypeString(const WString& text);` next to the existing keyboard helpers, and add a private `NativeWindowCharInfo MakeCharInfo(wchar_t ch) const;` prototype so all char-generation paths share the same metadata construction.
- Extend the class comment to clarify that the initial `TypeString` implementation only emits sequential IOChar events (no KeyDown/KeyUp synthesis) to set expectations for test authors and future contributors.
- Why: publishing a narrow API keeps the protocol surface consistent with the updated requirement while still enabling tests to request realistic text entry.

## STEP 2: Implement IOChar-only typing pipeline
- In `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.cpp`, implement `TypeString` to iterate every character in the provided `WString`, build the corresponding `NativeWindowCharInfo` via `MakeCharInfo`, and immediately dispatch it through `controller->InputService()->OnIOChar(info);`, skipping any keydown/keyup logic per the new directive.
- Ensure `MakeCharInfo` populates `code`, `codePoint`, and modifier flags (caps lock, existing pressing keys) the same way `_Char` helpers do today so text controls see consistent data, and guard the loop so empty strings exit quickly.
- Document inline that this is the first-stage implementation and that additional key-event simulation can be layered later without breaking callers.
- Why: firing IOChar events directly satisfies the updated scope quickly while keeping the code structured for future enhancements.

## STEP 3: Add GuiSinglelineTextBox typing regression test
- Extend `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` with a new `TEST_CASE(L"GuiSinglelineTextBox Typing")` that uses the existing window resource containing the lone textbox with `<att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>`.
- Inside the proxy, create sequential `protocol->OnNextIdleFrame` handlers: (1) focus the textbox, clear text, and call `protocol->TypeString(L"Hello");`; (2) assert the intermediate text, append `" World"` via `TypeString`, and use arrow keys/punctuation to prove caret movement still works; (3) select all text, replace it with `TypeString(L"Replaced")`, then press Backspace/Delete to check special keys; (4) reposition the caret with the mouse and type again before hiding the window. Ensure each frame label describes new work to satisfy the no-redundant-frame rule.
- Add `TEST_ASSERT` statements after each interaction to verify text content (and selection length when applicable) so failures immediately pinpoint broken behaviors.
- Why: this test covers the entire IOChar-only typing flow plus selection replacement and navigation, preventing regressions in both the helper and `GuiSinglelineTextBox`.

# TEST PLAN
- `GuiSinglelineTextBox Typing` test: execute through the existing unit-test harness to prove that `TypeString` inserts `"Hello"`, appends `" World"`, and coexists with caret movement triggered by arrow keys/mouse clicks.
- Selection replacement scenario: within the same test, call `SelectAll()` followed by `protocol->TypeString(L"Replaced")` and assert the textbox text equals `"Replaced"` to confirm IOChar sequences override highlighted ranges.
- Special key coverage: after typing punctuation, invoke `protocol->KeyPress(VKEY::KEY_BACK)` and `protocol->KeyPress(VKEY::KEY_DELETE)` to ensure deletions still behave when text comes from IOChar-only input.
- Non-letter characters: type `protocol->TypeString(L" !?")` (or similar) and assert the resulting text to validate whitespace/punctuation delivery without dedicated keydown logic.
- Modifier neutrality: manually press Shift via existing helpers before calling `TypeString` in one frame to confirm the helper does not interfere with caller-managed modifier states (it never toggles them), documenting the expected behavior.
# !!!FINISHED!!!
