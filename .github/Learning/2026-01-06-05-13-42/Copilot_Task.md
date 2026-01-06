# !!!TASK!!!
# PROBLEM DESCRIPTION
## TASK No.16: Create typing simulation test case and complete typing helper functions

### description

**Part 1: Complete typing helper functions**

In `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h` and `.cpp`, add typing support:

- `TypeText(const WString& text)`: Types each character by:
  - Sending KeyDown for the character
  - Sending Char event for the character
  - Sending KeyUp for the character
  - Use appropriate virtual key codes for common characters

- `TypeString(const WString& text)`: Types entire string by calling `TypeText` for each character

Handle special characters:
- Space, Enter, Tab
- Punctuation
- Numbers and letters (uppercase/lowercase - consider shift key state)

Study existing `KeyPress`, `_KeyDown`, `_KeyUp` methods for pattern.

**Part 2: Create typing test case**

Add a new test case in `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`:

Reuse the simple window layout finalized in Task No.15: keep the lone `SinglelineTextBox` anchored with `<att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>` and avoid wrapping it in extra tables or compositions. Ensure each `protocol->OnNextIdleFrame` introduces new observable work—merge frames that would otherwise repeat the same state, because redundant frames cause the unit test harness to error.

```cpp
TEST_CASE(L"GuiSinglelineTextBox Typing")
{
    GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    {
        protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            
            // Clear existing text
            textBox->SelectAll();
            protocol->TypeText(L"Hello");
        });
        protocol->OnNextIdleFrame(L"Typed Hello", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello");
            
            protocol->TypeText(L" World");
        });
        protocol->OnNextIdleFrame(L"Typed World", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello World");
            
            // Test backspace
            protocol->KeyPress(VKEY::KEY_BACK);
        });
        protocol->OnNextIdleFrame(L"Backspace pressed", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello Worl");
            
            window->Hide();
        });
    });
    GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...);
}
```

Follow the existing unit test style by only including `TestControls.h` in this file—do not add a `using namespace gacui_unittest_template;` line since the header already exposes the necessary symbols.

Test cases should verify:
- Basic character typing
- String typing
- Special keys (backspace, delete, arrows)
- Text selection + typing (replaces selection)
- Cursor positioning with mouse then typing

### what to be done

1. Add `TypeText` and `TypeString` methods to `GuiUnitTestProtocol_IOCommands`
2. Implement character-to-virtual-key mapping for common characters
3. Handle shift key state for uppercase letters and special characters
4. Add new test case to existing GuiSinglelineTextBox test file
5. Test basic typing functionality
6. Test special key handling (backspace, delete, etc.)
7. Test interaction between mouse positioning and typing

### how to test it

Run the compiled unit test executable. The test validates:
1. Typing helper functions correctly simulate keyboard input
2. Text is correctly inserted at cursor position
3. Special keys (backspace, delete, arrows) work correctly
4. Selection + typing correctly replaces selected text
5. The complete user interaction pipeline works end-to-end

### file locations

Protocol functions: `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h` and `.cpp`
Test case: `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp` (add to existing file)

### rationale

Typing simulation is essential for testing interactive text editing scenarios. The current unit test framework has mouse and keyboard primitives but no high-level typing functions. Adding these functions enables realistic user interaction testing. This test validates the complete paragraph editing pipeline: user input → text change → rendering update → visual verification. It's the most comprehensive test of the paragraph implementation and will catch issues that only appear during interactive editing.

# UPDATES

# INSIGHTS AND REASONING
## Repository signals and gaps
- `UnitTestRemoteProtocol_IOCommands` already tracks pressed keys, mouse buttons, and caps-lock state (see `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h:25-88`) but has no helper that emits paired key + character events, so text boxes only see low-level `KeyPress` calls.
- The GuiSinglelineTextBox test file currently contains only the basic lifecycle check from Task 15 (`Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp:1-44`), so there is no coverage for user-like typing, selection replacement, or caret movement.

## Typing helper design options
### Option A – Table-driven keystroke synthesis (recommended)
- Extend `UnitTestRemoteProtocol_IOCommands` with a private `NativeWindowCharInfo MakeCharInfo(wchar_t code)` helper so the new API can reuse the same modifier detection logic as `_KeyDown`/`MakeKeyInfo`.
- Maintain a constexpr lookup table that maps ASCII characters (letters, digits, common punctuation, whitespace) to `{VKEY key, bool requiresShift}` pairs, and fall back to `KEY_UNKNOWN` for characters that cannot be produced with the built-in table. This keeps the implementation deterministic and independent from platform keyboard layouts, which fits the deterministic unit-test runtime.
- Implement `TypeText(const WString&)` as the high-level entry point: iterate each Unicode code unit, derive its keystroke, press the necessary modifier (only if it is not already pressed), send `_KeyDown(key)`, fire `UseEvents().OnIOChar(MakeCharInfo(ch))`, then `_KeyUp(key)`, and finally release the temporary modifier. Non-ASCII characters can skip the key events but must still dispatch the char event so document controls get the actual text.
- Provide `TypeString(const WString&)` as the compatibility wrapper requested in the task by forwarding each code point to `TypeText` (or to a shared `TypeSingleCharacter` helper) so both APIs stay in sync and tests can call whichever reads better.

### Option B – Input-service-driven translation
- Instead of maintaining our own table, route every character through `GetCurrentController()->InputService()` (available via `GuiApplicationInterface`) to obtain a virtual key and modifier set based on the current layout, then emit the same event sequence as above.
- This keeps the mapping aligned with whatever layout the host controller exposes but introduces a dependency on the GUI runtime being fully bootstrapped before tests begin, increases mocking complexity inside `UnitTestRemoteProtocol_IOCommands`, and makes deterministic testing harder because layout differences (e.g., AZERTY) would change the produced key stream.

### Comparison
- Option A is self-contained, easy to reason about, and mirrors how other IO helpers in this class hard-code wheel deltas or button enums. Option B could better reflect real layouts but adds a moving part to an area that is supposed to be deterministic; failures would be harder to reproduce across machines. Given the unit-test focus and the fact the sample pseudo-code already expects well-known ASCII input, Option A is the safer choice.

## Selected plan for Option A
- Add `MakeCharInfo` plus a helper `struct KeyStrokePattern { VKEY key; bool needsShift; }` inside the class, along with functions to detect if we temporarily pressed Shift so we can release it (important because `_KeyDown` and `_KeyUp` validate the internal `pressingKeys` list and would throw if we double-release).
- Handle special characters explicitly: map `\n` to `KEY_RETURN`, `\t` to `KEY_TAB`, `' '` to `KEY_SPACE`, digits to `KEY_0`..`KEY_9`, and punctuation that requires Shift (e.g., `!`, `@`, `?`) to the correct `VKEY` plus automatic `_KeyDown(VKEY::KEY_SHIFT)` around the keystroke. Unsupported characters will still call `OnIOChar` so the text box receives the glyph even if no virtual key is available.
- Keep `capslockToggled` in sync by reusing `_KeyDown`/`_KeyUp`; when we temporarily press Shift we must respect the existing modifier state so we do not release a key the test intentionally held down before invoking `TypeText`.
- `TypeText` will accept multi-character strings (as in the provided pseudo-test) and internally loop through a new `TypeOneCharacter` helper; `TypeString` will simply forward to that helper per character, satisfying the instruction while avoiding duplicated logic.

## Typing test plan
- Reuse the existing resource so both test cases operate on the same XML snippet, keeping start-up cost low and avoiding project-file edits.
- Add `TEST_CASE(L"Typing")` under the same `TEST_CATEGORY`. The frames will: (1) focus the text box (mouse click if needed), clear text, and call `protocol->TypeText(L"Hello")`; (2) assert the intermediate string, append `" World"` plus a punctuation typed through `TypeString`, and verify caret movement via `protocol->KeyPress(VKEY::KEY_LEFT)` followed by `protocol->TypeText(L"!")`; (3) test selection replacement by calling `textBox->SelectAll()` and then `protocol->TypeText(L"Replaced")`; (4) exercise special keys such as Backspace/Delete and arrow navigation; (5) close the window. Each frame performs unique work to satisfy the “no redundant frames” rule from the problem statement.
- Assertions will rely on `GuiSinglelineTextBox::GetText()` and `GetSelectionStart()/GetSelectionLength()` to prove that typing and modifier interactions happened through the new helpers rather than via direct `SetText`.

## Verification considerations
- The new helpers emit the same `NativeWindowKeyInfo`/`NativeWindowCharInfo` pattern as the platform controller (`OnIOKeyDown` → `OnIOChar` → `OnIOKeyUp`), so existing document input code paths work unchanged.
- The typing test doubles as a regression suite for caret replacement, modifier handling, and the new helpers. Additional manual testing is minimal because the remote unit-test runner already covers Windows and Linux behavior identically through the protocol.

# !!!FINISHED!!!
