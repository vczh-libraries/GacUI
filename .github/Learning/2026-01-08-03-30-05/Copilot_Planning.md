# !!!PLANNING!!!

# UPDATES

## UPDATE

You don't need to extract `MakeSinglelineTextBoxAppName` and `RunSinglelineTextBox`, just repeat the code in every test case. And for [TestControls_Editor_GuiSinglelineTextBox.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp) only, keep path the same after the last "/", do not use category_case. This is for another file.

## UPDATE

You do not need to add the variable `appName`, just inline it to the function call

# EXECUTION PLAN

## STEP 1: Add a shared test-case entry point for reuse

Refactor `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` by introducing:

- `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName);`

Why this change is necessary:

- The `controlName` parameter is the boundary that makes the test cases reusable across future `GuiDocumentLabel` subclasses (each providing its own resource XML + name).
- It keeps `TEST_FILE` minimal (resource + one outer control category), while moving behavior into a reusable function.

## STEP 2: Move `Basic` and `Typing` test cases into `RunSinglelineTextBoxTestCases`

Move the existing `TEST_CASE(L"Basic")` and `TEST_CASE(L"Typing")` bodies into `RunSinglelineTextBoxTestCases(resource, controlName)` while keeping the remote-protocol sequencing and assertions unchanged.

For this file only, keep the log path segment after the last `/` exactly the same as the existing cases (do not use `CATEGORY_CASE`). That means:

- `Controls/Editor/<controlName>/Basic`
- `Controls/Editor/<controlName>/Typing`

Since no shared `MakeSinglelineTextBoxAppName` / `RunSinglelineTextBox` helper should be extracted, repeat the `GacUIUnitTest_SetGuiMainProxy(...)` + `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` in each test case, and inline the app name expression directly into the `GacUIUnitTest_StartFast_WithResourceAsText` call.

Proposed code shape inside each test case:

```cpp
GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
{
	// unchanged setup + protocol steps
});
GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
	WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Basic"),
	WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
	resource
);
```

Why this change is necessary:

- The outer `TEST_CATEGORY` should select the control variant; the shared function should define behaviors (test cases) so they can be invoked for multiple controls later.
- Not extracting helpers keeps this file aligned with the updated requirement while still achieving reuse via `RunSinglelineTextBoxTestCases(...)`.

## STEP 3: Relax the control lookup type to `GuiDocumentLabel`

In the shared test bodies, change:

```cpp
auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
```

to:

```cpp
auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
```

Why this change is necessary:

- It is a prerequisite for sharing the same test steps with other editor controls inheriting from `GuiDocumentLabel` while keeping a strongly typed lookup (still fails fast if the control is not the expected base type).
- All exercised APIs in the current tests (`GetText`, `SetText`, `SetFocused`, `SelectAll`, and `UnitTestRemoteProtocol` interactions via the control instance) remain applicable through `GuiDocumentLabel`.

## STEP 4: Update `TEST_FILE` to contain only resource + one outer control category

Restructure `TEST_FILE` so it only contains:

- The resource XML (unchanged).
- One outer `TEST_CATEGORY(L"GuiSinglelineTextBox")` that invokes the shared test cases.

Proposed code shape:

```cpp
TEST_FILE
{
	const auto resource = LR"GacUISrc(
		/* unchanged XML */
	)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases(resource, WString::Unmanaged(L"GuiSinglelineTextBox"));
	}
}
```

Why this change is necessary:

- This makes the file match the intended reuse pattern: the control-specific section is minimal (resource + category), while the behaviors live in a reusable shared function.

## STEP 5: Test plan (no new test cases)

After refactoring, verify:

- The `Basic` and `Typing` behaviors are unchanged (same assertions and end states).
- Each test case routes logs to `Controls/Editor/GuiSinglelineTextBox/<CaseName>` (e.g. `Basic`, `Typing`).
- The control lookup is `FindObjectByName<GuiDocumentLabel>(..., L"textBox")` and the test still passes.

Run the existing GUI unit test suite via the repository's normal Visual Studio / task workflow (no new tests are required for this refactor-only change).

# !!!FINISHED!!!
