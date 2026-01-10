# !!!EXECUTION!!!

# UPDATES

## User Update Spotted

The user edited `TestControls_Editor_GuiSinglelineTextBox_Key.cpp` with the following differences from the execution plan:

1. **Frame "Init"**: Removed `TEST_ASSERT(textBox->GetText() == L"");` - the user did not want this assertion.

2. **Frame "Typing"**: Added `TEST_ASSERT(textBox->GetText() == L"012345678X9");` at the end of this frame (moved from "Moved Cursor and Edited" frame).

3. **Frame "Moved Cursor and Edited"**: Removed all assertions and simplified to just `window->Hide();` - the assertion was moved to the previous frame.

These changes are respected and will not be reverted.

# EXECUTION PLAN

## STEP 1: Move `TooltipTimer` to `TestControls.h` [DONE]

Update `Test/GacUISrc/UnitTest/TestControls.h` by appending the following class definition after the existing `namespace gacui_unittest_template { }` block (near the end of the file):

```cpp
	class TooltipTimer : protected vl::feature_injection::FeatureImpl<vl::IDateTimeImpl>
	{
	private:
		vl::DateTime			currentTime;

	public:
		TooltipTimer()
		{
			currentTime = vl::DateTime::FromDateTime(2000, 1, 1, 0, 0, 0, 0);
			vl::InjectDateTimeImpl(this);
		}

		~TooltipTimer()
		{
			try
			{
				vl::EjectDateTimeImpl(nullptr);
			}
			catch (...)
			{
			}
		}

		void Tooltip()
		{
			currentTime = currentTime.Forward(600);
		}

		void WaitForClosing()
		{
			currentTime = currentTime.Forward(5100);
		}

	protected:
		vl::DateTime FromDateTime(vint _year, vint _month, vint _day, vint _hour, vint _minute, vint _second, vint _milliseconds) override
		{
			return Previous()->FromDateTime(_year, _month, _day, _hour, _minute, _second, _milliseconds);
		}

		vl::DateTime FromOSInternal(vuint64_t osInternal) override
		{
			return Previous()->FromOSInternal(osInternal);
		}

		vuint64_t LocalTime() override
		{
			return currentTime.osMilliseconds;
		}

		vuint64_t UtcTime() override
		{
			return Previous()->LocalToUtcTime(currentTime.osInternal);
		}

		vuint64_t LocalToUtcTime(vuint64_t osInternal) override
		{
			return Previous()->LocalToUtcTime(osInternal);
		}

		vuint64_t UtcToLocalTime(vuint64_t osInternal) override
		{
			return Previous()->UtcToLocalTime(osInternal);
		}

		vuint64_t Forward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Forward(osInternal, milliseconds);
		}

		vuint64_t Backward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Backward(osInternal, milliseconds);
		}
	};
```

## STEP 2: Update tooltip tests to use the shared helper [DONE]

Update `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp`:

1) Remove the now-unnecessary declaration:

```cpp
namespace vl
{
	extern IDateTimeImpl* GetOSDateTimeImpl();
}
```

2) Remove the local `TooltipTimer` class definition entirely (it is now provided by `TestControls.h`).

After this change, the top of the file should start like this:

```cpp
#include "TestControls.h"

using namespace gacui_unittest_template;
```

All existing `TooltipTimer timer;` usages inside `TEST_CASE` bodies remain unchanged.

## STEP 3: Hijack time in all single-line textbox editor unit tests [DONE]

Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` by inserting `TooltipTimer timer;` as the first statement inside every `TEST_CASE` body in this file.

1) In `TEST_CASE(L"Basic")`:

```cpp
	TEST_CASE(L"Basic")
	{
		TooltipTimer timer;
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
```

2) In `TEST_CASE(L"Typing")`:

```cpp
	TEST_CASE(L"Typing")
	{
		TooltipTimer timer;
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
```

Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` by inserting `TooltipTimer timer;` as the first statement inside every `TEST_CASE` body in this file.

In `TEST_CASE(L"SmokingTest")`:

```cpp
		TEST_CASE(L"SmokingTest")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"X");
				});

				protocol->OnNextIdleFrame(L"Moved Cursor and Edited", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"012345678X9");
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
```

## STEP 4: Align `OnNextIdleFrame` scaffold in the key scaffold test (keep existing frame titles unchanged) [DONE]

Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` in `TEST_CASE(L"SmokingTest")` to follow the frame rule, without changing any existing frame name strings.

Replace the three `protocol->OnNextIdleFrame(...)` blocks with the following:

```cpp
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"X");
				});

				protocol->OnNextIdleFrame(L"Moved Cursor and Edited", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"012345678X9");
					window->Hide();
				});
```

# FIXING ATTEMPTS

## Fixing attempt No.1

**Problem**: Build failed with 9 errors. The main error was `C2011: 'gacui_unittest_template::TooltipTimer': 'class' type redefinition` along with syntax errors in test files.

**Root Cause**: The `TestControls.h` header file was missing include guards (neither `#pragma once` nor `#ifndef/#define/#endif` macros). When multiple .cpp files include this header during parallel compilation, the `TooltipTimer` class gets defined multiple times in the same translation unit.

**Fix Applied**: Added include guards to `TestControls.h`:
- Added `#ifndef GACUI_UNITTEST_TESTCONTROLS` and `#define GACUI_UNITTEST_TESTCONTROLS` at the top of the file
- Added `#endif` at the end of the file

This follows the project's coding convention of using macros for header guards instead of `#pragma once`.

## Fixing attempt No.2

**Problem**: Build failed with 8 errors. Syntax errors in test files:
- `TestControls_Editor_GuiSinglelineTextBox.cpp(7,16): error C3878: syntax error: unexpected token 'timer' following 'expression'`
- `TestControls_Editor_GuiSinglelineTextBox_Key.cpp(9,17): error C3878: syntax error: unexpected token 'timer' following 'expression'`
- Multiple `error C2059: syntax error: ')'` errors in both files.

**Root Cause**: The `TooltipTimer` class is defined inside the `gacui_unittest_template` namespace in `TestControls.h`. Both test files were missing the `using namespace gacui_unittest_template;` directive, so the compiler did not recognize `TooltipTimer` as a type name.

**Fix Applied**: Added `using namespace gacui_unittest_template;` after the `#include "TestControls.h"` line in both files:
- `TestControls_Editor_GuiSinglelineTextBox.cpp`
- `TestControls_Editor_GuiSinglelineTextBox_Key.cpp`

# Comparing to User Edit

The user edited `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` after execution, and the current source code differs from the execution plan in these ways:

1. **Frame "Init"**: removed `TEST_ASSERT(textBox->GetText() == L"");`.
2. **Frame "Typing"**: added `TEST_ASSERT(textBox->GetText() == L"012345678X9");` after typing `L"X"`.
3. **Frame "Moved Cursor and Edited"**: removed assertions and kept only `window->Hide();`.

# !!!FINISHED!!!

# !!!VERIFIED!!!
