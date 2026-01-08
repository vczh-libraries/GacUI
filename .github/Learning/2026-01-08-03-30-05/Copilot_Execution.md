# !!!EXECUTION!!!
# UPDATES

# EXECUTION PLAN

## STEP 1: Add RunSinglelineTextBoxTestCases function [DONE]

**File:** `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`

**Action:** Insert after `#include "TestControls.h"` and before `TEST_FILE`:

```cpp
static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
{
	TEST_CASE(L"Basic")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Initial text");
				TEST_ASSERT(textBox->GetEnabled() == true);
				textBox->SetText(L"Changed text");
			});
			protocol->OnNextIdleFrame(L"After text change", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Changed text");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Basic"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});

	TEST_CASE(L"Typing")
	{
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				textBox->SetFocused();
				textBox->SelectAll();
				protocol->TypeString(L"Hello");
			});
			protocol->OnNextIdleFrame(L"Typed Hello", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello");
				protocol->TypeString(L" World");
			});
			protocol->OnNextIdleFrame(L"Typed World", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello World");
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->TypeString(L"!");
			});
			protocol->OnNextIdleFrame(L"Inserted punctuation", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello Worl!d");
				textBox->SelectAll();
				protocol->TypeString(L"Replaced");
			});
			protocol->OnNextIdleFrame(L"Selection replaced", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Replaced");
				protocol->KeyPress(VKEY::KEY_BACK);
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->KeyPress(VKEY::KEY_DELETE);
			});
			protocol->OnNextIdleFrame(L"Backspace/Delete applied", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Replac");
				auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.5, 2, 0);
				protocol->LClick(caretPoint);
				protocol->TypeString(L"Start ");
			});
			protocol->OnNextIdleFrame(L"Mouse typed at start", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Start Replac");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Typing"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});
}
```

## STEP 2: Update TEST_FILE to use the shared function [DONE]

**File:** `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`

**Action:** Replace the entire `TEST_FILE` block with:

```cpp
TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox" Text="Initial text">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases(resource, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});
}
```

## STEP 3: Verification [DONE]

Run the existing GUI unit test suite to verify:
- The `Basic` and `Typing` behaviors are unchanged (same assertions and end states).
- Each test case routes logs to `Controls/Editor/GuiSinglelineTextBox/Basic` and `Controls/Editor/GuiSinglelineTextBox/Typing`.
- The control lookup is `FindObjectByName<GuiSinglelineTextBox>(..., L"textBox")` and the test still passes.

# !!!FINISHED!!!