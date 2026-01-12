# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.1: Split singleline-specific cases into RunTextBoxKeyTestCases_Singleline

Refactor the shared key-test scaffold in the editor unit tests so that singleline-only behaviors are isolated in a dedicated helper, while keeping log paths stable.

This task removes the `singleline` flag from `RunTextBoxKeyTestCases` and moves all currently-conditional tests into a new `RunTextBoxKeyTestCases_Singleline` helper.

### what to be done

- Update `RunTextBoxKeyTestCases` in Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp to remove the `singleline` parameter.
- Introduce `RunTextBoxKeyTestCases_Singleline` and move all tests that were guarded by `if (singleline)` into that function.
- Update `TEST_FILE` to:
	- Call `RunTextBoxKeyTestCases_Singleline` only for `GuiSinglelineTextBox`.
	- Append " (Singleline)" to the outer `TEST_CATEGORY` name for `GuiSinglelineTextBox`, while keeping `controlName` used in log paths unchanged so the log path stays stable.
- Ensure the moved test cases keep the same per-test log identity string (the `Controls/Editor/<controlName>/Key/...` path) as before.
- Fix any call-site or template-parameter mismatches exposed by the refactor so the file still compiles and the tests still find the correct `<... ref.Name="textBox"/>` control instance.

### rationale

- The `singleline` flag currently couples unrelated behaviors into one monolithic helper and makes it hard to add multiline/paragraph-specific coverage cleanly.
- Preserving the log path while changing category names is important for existing log consumers and for comparing historical test runs.
- Making singleline-only behaviors explicit creates a stable baseline for the next two tasks to build multiline and multi-paragraph semantics without adding more conditionals.

# UPDATES

## UPDATE

By saying appending (Singleline) after the category, I mean call two function in the outer most GuiSinglelineTextBox test category, but changing all test categories in the new function to have the postfix, e.g. Clipboard->Clipboard (Singleline)

# INSIGHTS AND REASONING

## Current State (Evidence)

- The editor key tests are centralized in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` in the template helper `RunTextBoxKeyTestCases(const wchar_t* resource, const WString& controlName, bool singleline)`.
- The helper defines a shared set of `TEST_CATEGORY` blocks (e.g. Scaffold/Navigation/Typing/Clipboard/UndoRedo/Deletion), and inside three of those categories there are `if (singleline)` branches that register extra test cases.
- The test log identity (output folder) is not derived from the test category name; it is explicitly built via:
	- `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/<CaseName>")`
	- This string is passed to `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)`.

This matches the requirement that we can change unit test category names without changing the log path, as long as `controlName` and the `"/Key/..."` suffix remain unchanged.

## Design Goal

- Remove the `singleline` parameter from the shared helper so the baseline test suite becomes unconditional and reusable.
- Isolate singleline-only behaviors into a dedicated helper that is invoked only for `GuiSinglelineTextBox`.
- Keep every existing log path stable by preserving:
	- the `controlName` argument at call sites (e.g. `L"GuiSinglelineTextBox"`), and
	- the exact `"/Key/..."` suffix strings for moved test cases.

## Proposed Refactor Shape

### 1) Keep the shared baseline helper (remove the flag)

- Change the signature to:
	- `template<typename TTextBox> void RunTextBoxKeyTestCases(const wchar_t* resource, const WString& controlName)`
	- Keep the existing `requires(std::is_base_of_v<GuiControl, TTextBox> && std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)` constraint.
- Remove the three `if (singleline)` blocks entirely from this helper.

This makes `RunTextBoxKeyTestCases` the stable “common denominator” suite shared by all editor-like controls.

### 2) Add a dedicated helper for singleline-only cases

- Introduce a second template helper with the same argument list:
	- `template<typename TTextBox> void RunTextBoxKeyTestCases_Singleline(const wchar_t* resource, const WString& controlName)`
	- Apply the same `requires(...)` constraint for consistency.
- Move all test cases that were guarded by `if (singleline)` into this helper.

Practically, this corresponds to extracting (and only extracting) the conditional cases currently located under:

- `TEST_CATEGORY(L"Clipboard")`: `CtrlV_FlattensLineBreaks`
- `TEST_CATEGORY(L"UndoRedo")`: `CtrlZ_UndoesPaste_FlattenedText`
- `TEST_CATEGORY(L"Deletion")`: the `Enter_Ignored_*` cases

### 3) Preserve category structure without reintroducing branching

To avoid duplicated category names under `GuiSinglelineTextBox` (while still keeping the singleline-only cases isolated), the singleline helper should use the same category concepts but with a postfix:

- In `RunTextBoxKeyTestCases_Singleline`, name categories like:
	- `TEST_CATEGORY(L"Clipboard (Singleline)")`
	- `TEST_CATEGORY(L"UndoRedo (Singleline)")`
	- `TEST_CATEGORY(L"Deletion (Singleline)")`

This matches the clarified requirement that “(Singleline)” applies to the categories introduced by the new helper (e.g. `Clipboard -> Clipboard (Singleline)`), while keeping the baseline helper’s category names unchanged.

### 4) Adjust `TEST_FILE` call sites and category name

- Keep the outer category as `TEST_CATEGORY(L"GuiSinglelineTextBox")`.
- Inside that outer category, call both helpers:
	- `RunTextBoxKeyTestCases<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));`
	- `RunTextBoxKeyTestCases_Singleline<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));`

Other controls keep calling only the baseline `RunTextBoxKeyTestCases` (with the updated signature), and keep their category names unchanged.

This satisfies the clarified meaning of “append " (Singleline)" after the category” (apply it to the categories contributed by the new helper), while keeping `controlName` (and therefore log paths) stable.

## Why This Preserves Logs

- Log paths are explicitly assembled from `controlName` + a hardcoded suffix inside each test case.
- The outer `TEST_CATEGORY` name (e.g. `GuiSinglelineTextBox (Singleline)`) is not used in the log path construction.
- Therefore, the only requirement for stable logs is:
	- keep `controlName` passed to the helpers unchanged (`L"GuiSinglelineTextBox"`), and
	- do not change the per-case suffix strings like `"/Key/Clipboard_CtrlV_FlattensLineBreaks"`.

## Testing Considerations

- The refactor must not alter the frame sequence (`OnNextIdleFrame`) inside each moved test case.
	- The knowledge base notes that each `OnNextIdleFrame` is a frame boundary and logs are generated frame-by-frame (see `.github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md`, “Frame-Based Testing”).
- By moving the test case blocks verbatim into the new helper (no frame renaming, no additional frames), the recorded frame logs remain directly comparable.

## Risks / Edge Cases

- If any tooling previously assumed category names like `Clipboard` exist under `GuiSinglelineTextBox`, those category names will now be split into `Clipboard` (baseline) and `Clipboard (Singleline)` (singleline-only). This is a deliberate behavior change per the update and should not affect log folders.
- Template parameter mismatches must remain unchanged at call sites (e.g. `<DocumentTextBox>` tests currently instantiate `RunTextBoxKeyTestCases<GuiDocumentLabel>(...)` while using `controlName` `GuiDocumentTextBox`). The refactor must preserve this mapping; it only changes the helper signature, not the chosen `TTextBox`.

# !!!FINISHED!!!
