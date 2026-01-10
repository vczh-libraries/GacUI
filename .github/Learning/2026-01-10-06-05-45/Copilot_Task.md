# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.2: Create GuiSinglelineTextBox key test scaffold

Create a shared test window/resource and minimal helper flow in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` so later categories can focus on keyboard behaviors without repeating boilerplate, and so the same test cases can be reused by other `GuiDocumentLabel` subclasses in the future.

### what to be done

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` to `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Implement `TEST_FILE` using the structure:
  - `const auto resourceSinglelineTextBox = LR"GacUISrc( ... )GacUISrc";`
  - An outer `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox")); }`.
- Add (or update) a shared `RunSinglelineTextBox` helper that:
  - Loads the UI from `resource` (XML) and starts the test using `GacUIUnitTest_StartFast_WithResourceAsText`.
  - Builds per-test log paths as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/CATEGORY_CASE")` (CATEGORY and CASE replaced by actual names).
  - Finds the target control by `FindObjectByName<GuiDocumentLabel>(window, L"textBox")`.
- Ensure each test case establishes a consistent initial state (focus + known text like `L"0123456789"`) before exercising key operations via `UnitTestRemoteProtocol` (`KeyPress`, `TypeString`).

### rationale

- All subsequent tasks add `TEST_CATEGORY` blocks in `RunSinglelineTextBoxTestCases`; a shared scaffold keeps those tasks small and consistent.
- Using an index-friendly initial text makes it easy to assert caret/selection changes precisely.

# UPDATES

# INSIGHTS AND REASONING

## Existing patterns to reuse

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` already follows the intended “shared test cases + outer control category” structure, and demonstrates the standard unit test flow used by GacUI GUI tests:
  - Each `TEST_CASE` configures a `GacUIUnitTest_SetGuiMainProxy` callback that drives the UI using `UnitTestRemoteProtocol` and `protocol->OnNextIdleFrame(...)` steps.
  - Each `TEST_CASE` then calls `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(appName, windowType, resourceText)` where `appName` is used as the test log/snapshot identity.
- The Vlpp unit test framework is intended to organize tests via `TEST_CATEGORY` and `TEST_CASE` (see `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md`). For the key tests, the outer category in `TEST_FILE` is `GuiSinglelineTextBox`, while inner categories should be short names like `Navigation`, `Clipboard`, etc (per the scrum update).

## Proposed scaffold design (for `TestControls_Editor_GuiSinglelineTextBox_Key.cpp`)

### 1) File entry point and resource

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` and keep all future keyboard-related categories inside it.
- In `TEST_FILE`, define `resourceSinglelineTextBox` as a minimal UI containing one window and one named textbox `ref.Name="textBox"` (mirroring the existing non-key tests).
- `TEST_FILE` contains exactly one outer category:
  - `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox")); }`
- This keeps the key test file consistent with the refactored non-key test file and enables future reuse by invoking `RunSinglelineTextBoxTestCases(...)` with a different resource/controlName pair.

### 2) Shared `RunSinglelineTextBox` helper

- Introduce a helper whose responsibility is only “standard boot + standard lookup + standard initial state”, so individual key tests can stay focused on their key sequences and assertions.
- Suggested responsibilities:
  - Construct the per-test `appName` as:
    - `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/") + categoryName + WString::Unmanaged(L"_") + caseName`
    - This directly implements the required `.../Key/CATEGORY_CASE` scheme without depending on any implicit unit test framework context.
  - Start the window using `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(appName, WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resource)`.
  - Always resolve the tested control as `FindObjectByName<GuiDocumentLabel>(window, L"textBox")` (not `GuiSinglelineTextBox`) to keep the scaffold compatible with other `GuiDocumentLabel` subclasses.
  - Establish a consistent baseline in the first idle frame:
    - `textBox->SetFocused()`.
    - `textBox->SetText(L"0123456789")` (or select-all + type) and clear any selection so caret/selection assertions start from a known state.
- The helper should accept a callback so each test case can append additional `OnNextIdleFrame` steps and drive `protocol->KeyPress(...)` / `protocol->TypeString(...)` without repeating window startup and control lookup.

### 3) Minimal “scaffold smoke” test (optional but recommended)

- Add one small `TEST_CATEGORY` + `TEST_CASE` (e.g. `TEST_CATEGORY(L"Scaffold")`) that only verifies:
  - The window loads.
  - The control can be found by name as `GuiDocumentLabel`.
  - Baseline text is applied.
  - The window can be closed.
- This ensures the new file is exercised by the test runner even before other keyboard categories are implemented in later tasks.

## Risks / open questions to validate during implementation (later execution tasks)

- Whether setting baseline text should use `textBox->SetText(...)` or `SelectAll()` + `protocol->TypeString(...)` depends on how later tasks want to validate undo/redo boundaries; the scaffold should keep baseline setup simple and let each test override it if needed.
- The key test resource should remain minimal and stable; any properties needed by later categories (e.g. tab acceptance) should be introduced only when a test category depends on them to avoid speculative configuration.

# !!!FINISHED!!!
