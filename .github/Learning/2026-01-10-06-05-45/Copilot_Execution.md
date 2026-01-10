# !!!EXECUTION!!!

# UPDATES

## UPDATE

I would like to split `Run` into two frames, as KEY_LEFT followed by X definitely updated the UI. Put `window->Hide();` in a third frame

# EXECUTION PLAN

## STEP 1: Create minimal UI resource and outer category [DONE]

Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` to have the same file-level structure as `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`:

- Add a minimal UI resource containing a `gacuisrc_unittest::MainWindow` and a `SinglelineTextBox` named `textBox`.
- Add an outer `TEST_CATEGORY(L"GuiSinglelineTextBox")` in `TEST_FILE` and call `RunSinglelineTextBoxTestCases(...)` so future categories can be added inside that function.

## STEP 2: Inline all TEST_CASE in RunSinglelineTextBoxTestCases [DONE]

In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`:

- Add `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)`.
- Place all `TEST_CATEGORY` / `TEST_CASE` blocks directly inside `RunSinglelineTextBoxTestCases(...)` (no `RunSinglelineTextBox` helper).
- Build the per-test log identity inline (no `appName` variable) using:
  - `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/<CATEGORY>_<CASE>")`

## STEP 3: Add a scaffold smoke test category [DONE]

Replace the entire content of `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` with:

```cpp
#include "TestControls.h"

static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
{
	TEST_CATEGORY(L"Scaffold")
	{
		TEST_CASE(L"SmokingTest")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_LEFT);
				});

				protocol->OnNextIdleFrame(L"Moved Cursor", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					protocol->TypeString(L"X");
					TEST_ASSERT(textBox->GetText() == L"012345678X9");
				});

				protocol->OnNextIdleFrame(L"Edited", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Scaffold_SmokingTest"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}

TEST_FILE
{
	const auto resourceSinglelineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Key Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});
}
```

**Applied UPDATE:** Split the original two-frame structure into four frames:
1. `Init` - Set focus and type initial text
2. `Typing` - Assert text and press KEY_LEFT  
3. `Moved Cursor` - Type "X" and assert final text
4. `Edited` - Hide window

Kept `FindObjectByName<GuiDocumentLabel>(..., L\"textBox\")` (per scrum update, for future reuse by other `GuiDocumentLabel` subclasses).

## STEP 4: Verification checklist [DONE]

- Ensure the test appears under `GuiSinglelineTextBox/Scaffold/SmokingTest`.
- Ensure the test log path matches `Controls/Editor/GuiSinglelineTextBox/Key/Scaffold_SmokingTest`.

# FIXING ATTEMPTS

## None

# Comparing to User Edit

No user edit found.

# !!!FINISHED!!!
