# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.1: Refactor existing GuiSinglelineTextBox tests for reuse

Restructure `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` to follow the same "shared test cases + outer control category" pattern as the key test file, so the same test cases can be reused for other `GuiDocumentLabel` subclasses in the future.

### what to be done

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`.
- Move existing `TEST_CASE(L"Basic")` and `TEST_CASE(L"Typing")` into `RunSinglelineTextBoxTestCases` (keeping their behavior the same).
- Introduce (or update) a shared `RunSinglelineTextBox` helper so each test case logs to `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/CATEGORY_CASE")` (CATEGORY and CASE replaced by actual names, consistent with the key test plan but without the `/Key/` segment).
- In `RunSinglelineTextBox`, change `FindObjectByName<GuiSinglelineTextBox>(window, L"textBox")` to `FindObjectByName<GuiDocumentLabel>(window, L"textBox")`.
- Update `TEST_FILE` to contain only the resource XML + one outer `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resource, WString::Unmanaged(L"GuiSinglelineTextBox")); }`.

### rationale

- This keeps non-key and key tests consistent and makes it straightforward to reuse the same test cases for other editor controls inheriting from `GuiDocumentLabel` later.
- Switching lookup to `GuiDocumentLabel` is a prerequisite for sharing without rewriting all test steps.

# UPDATES

# INSIGHTS AND REASONING

## Context and evidence from code / knowledge base

- `GuiSinglelineTextBox` inherits from `GuiDocumentLabel` (see `Source/Controls/TextEditorPackage/GuiDocumentViewer.h`), so looking up the control as `GuiDocumentLabel` keeps the test steps applicable to future `GuiDocumentLabel` subclasses while still allowing text/selection/edit operations provided by `GuiDocumentCommonInterface`.
- `vl::presentation::controls::FindObjectByName<T>` performs a type-checked `SafeAggregationCast<T>()` and fails fast when the type does not match (see `Source/Application/Controls/GuiBasicControls.h`), so changing the lookup type is the correct way to relax the test coupling without weakening runtime guarantees.
- GUI unit tests start with `GacUIUnitTest_StartFast_WithResourceAsText(...)` and use the `appName` string to label/route logs (see `Source/UnitTestUtilities/GuiUnitTestUtilities.h`), therefore centralizing `appName` construction in one helper prevents path drift between test cases.
- The unit test framework encourages a hierarchical `TEST_FILE` → `TEST_CATEGORY` → `TEST_CASE` organization (see `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md`), which matches the requested “outer control category + shared test cases” pattern.

## Current state (baseline)

`Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` currently:

- Defines one resource XML and one `TEST_CATEGORY(L"GuiSinglelineTextBox")`.
- Implements `TEST_CASE(L"Basic")` and `TEST_CASE(L"Typing")` directly in the file.
- Repeats per-test boilerplate:
  - `GacUIUnitTest_SetGuiMainProxy(...)` setup.
  - `FindObjectByName<GuiSinglelineTextBox>(..., L"textBox")` (overly specific for reuse).
  - Hard-coded `appName` strings (`Controls/Editor/GuiSinglelineTextBox/<CaseName>`).

This structure is fine for a single control, but it prevents reusing the exact same test cases for other `GuiDocumentLabel`-based editor controls without either (1) duplicating all test code, or (2) weakening the tests by avoiding typed lookups.

## Target structure (refactor for reuse)

### 1) Shared test-case entry point

Add a new function in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`:

- `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName);`

Responsibilities:

- Contain the existing behaviors currently in `TEST_CASE(L"Basic")` and `TEST_CASE(L"Typing")`.
- Keep test steps and assertions identical (same remote protocol sequencing, same `TEST_ASSERT` checks), while allowing the same code to be invoked under any outer control `TEST_CATEGORY` with a different `resource` and `controlName`.

### 2) Outer control category only in TEST_FILE

Update `TEST_FILE` to only declare:

- `const auto resourceSinglelineTextBox = LR"GacUISrc(... )GacUISrc";`
- `TEST_CATEGORY(L"GuiSinglelineTextBox") { RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox")); }`

This makes the file match the planned structure for `TestControls_Editor_GuiSinglelineTextBox_Key.cpp` (outer category decides the control “flavor”; inner shared function decides the behaviors to test).

### 3) Central helper for consistent logging and shared lookup type

Introduce a helper (either newly added or extracted from existing per-test code) that all test cases call, conceptually:

- Inputs:
  - `resource` and `controlName` (forwarded from `RunSinglelineTextBoxTestCases`).
  - `categoryName` and `caseName` strings used for log path formatting.
  - A callback to install `GacUIUnitTest_SetGuiMainProxy(...)` logic for the test case.
- Behavior:
  - Construct `appName` as `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/") + categoryName + WString::Unmanaged(L"_") + caseName`.
  - Start the test with `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(appName, WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resource);`.
  - In test steps, locate the subject control via `FindObjectByName<GuiDocumentLabel>(window, L"textBox")`.

Notes:

- This explicitly enforces the requested naming scheme `.../CATEGORY_CASE` while keeping category/case naming under the test author’s control.
- For this non-key file, a small “grouping category” can be introduced inside `RunSinglelineTextBoxTestCases` (e.g. `TEST_CATEGORY(L"General")`) so `CATEGORY_CASE` is always meaningful even when the file only has a few basic cases; this keeps the log-path scheme consistent with the key test file where inner categories are the primary organization.

## Why this design is the best fit

- Minimal behavioral risk: no change to how `UnitTestRemoteProtocol` steps are scheduled; only organization and parameterization change.
- Better reuse boundary: `RunSinglelineTextBoxTestCases(resource, controlName)` becomes the reusable unit, while `TEST_FILE` only supplies control-specific XML and label/path.
- Future-proof: the `GuiDocumentLabel` lookup and `controlName` parameter directly match the planned direction of sharing tests among multiple `GuiDocumentLabel` subclasses without rewriting “how to drive the editor” test logic.
- Log-path stability and consistency: centralizing `appName` construction ensures old and new cases follow the exact same pattern, which is important for diagnosing failures across control variants.

## Completion criteria

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` contains the shared `RunSinglelineTextBoxTestCases(...)` function and (if needed) a `RunSinglelineTextBox` helper.
- The `Basic` and `Typing` behaviors are preserved (same end state assertions).
- The subject lookup is via `FindObjectByName<GuiDocumentLabel>(..., L"textBox")`.
- `TEST_FILE` contains only the resource definition + one outer `TEST_CATEGORY(L"GuiSinglelineTextBox")` which invokes the shared test-case function.
- Each test case’s `appName` follows `Controls/Editor/<controlName>/<CATEGORY>_<CASE>` (no `/Key/` segment for this file).

# !!!FINISHED!!!
