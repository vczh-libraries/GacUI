# !!!EXECUTION!!!

# UPDATES

## User Update Spotted

- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, the `CtrlV_FlattensLineBreaks` assertions use `WString::IndexOf(L'\r')` / `WString::IndexOf(L'\n')` (character literals) instead of the string-literal form shown in the `STEP 5` snippet (`L"\r"` / `L"\n"`).
- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, `CtrlV_EmptyClipboard_NoChange` closes the window in an additional idle frame (`"No Change"`) instead of calling `window->Hide();` inside the `"Typing"` frame shown in the `STEP 5` snippet.

# EXECUTION PLAN

## STEP 1: Confirm the existing test scaffold and APIs [DONE]

Target file:

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`

Execution notes:

- Reuse the existing interaction pattern already used in this file:
	- First frame: focus the control and seed initial text via `protocol->TypeString(...)`.
	- Next frame: perform keyboard input and do all assertions in the same callback.
	- Only schedule an extra frame when the current callback issues an input that changes UI.
	- If an input is issued, add one more final frame whose only job is `window->Hide();`.

## STEP 2: Fix FakeClipboardService so tests can set/clear clipboard deterministically [DONE]

Target file:

- `Source/Utilities/FakeServices/GuiFakeClipboardService.cpp`

Change:

- In `FakeClipboardWriter`, update the constructor to initialize `reader` with a fresh `FakeClipboardReader`.

Implementation detail (replace the existing constructor body / initializer list with the following):

```cpp
		FakeClipboardWriter(FakeClipboardService* _service)
				: service(_service)
		{
				reader = Ptr(new FakeClipboardReader);
		}
```

## STEP 3: Add TEST_CATEGORY(L"Clipboard") under the existing singleline textbox shared test function [DONE]

Target file:

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`

Insertion point:

- Inside `RunSinglelineTextBoxTestCases<TTextBox>(...)`, insert the new category between the end of `TEST_CATEGORY(L"Navigation")` and before `TEST_CATEGORY(L"Deletion")`.

Add the category block:

```cpp
	TEST_CATEGORY(L"Clipboard")
	{
				// TEST_CASE(...) blocks go here.
	});
```

Log identity rule (use exactly this pattern for all new cases):

- `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`

## STEP 4: Implement Ctrl+C and Ctrl+X test cases (copy/cut) [DONE]

Target file:

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`

Add the following `TEST_CASE`s inside `TEST_CATEGORY(L"Clipboard")`:

```cpp
		TEST_CASE(L"CtrlC_CopiesSelection_TextUnchanged")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
								TEST_ASSERT(textBox->GetSelectionText() == L"234");
								TEST_ASSERT(textBox->GetText() == L"0123456789");

								protocol->KeyPress(VKEY::KEY_C, true, false, false);

								auto reader = clipboard->ReadClipboard();
								window->SetText(reader->GetText());
								TEST_ASSERT(reader->ContainsText());
								TEST_ASSERT(reader->GetText() == L"234");
								TEST_ASSERT(textBox->GetText() == L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Copied", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlC_CopiesSelection_TextUnchanged"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});

		TEST_CASE(L"CtrlX_CutsSelection_TextUpdated")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
								TEST_ASSERT(textBox->GetSelectionText() == L"234");
								protocol->KeyPress(VKEY::KEY_X, true, false, false);

								auto reader = clipboard->ReadClipboard();
								window->SetText(reader->GetText());
								TEST_ASSERT(reader->ContainsText());
								TEST_ASSERT(reader->GetText() == L"234");
								TEST_ASSERT(textBox->GetText() == L"0156789");
						});

						protocol->OnNextIdleFrame(L"Cut", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlX_CutsSelection_TextUpdated"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});
```

## STEP 5: Implement Ctrl+V test cases (paste), including single-line flattening behavior [DONE]

Target file:

- `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`

Add the following `TEST_CASE`s inside `TEST_CATEGORY(L"Clipboard")`:

- `CtrlV_PastesAtCaret`
- `CtrlV_ReplacesSelection`
- `CtrlV_FlattensLineBreaks`
- `CtrlV_EmptyClipboard_NoChange`

Key API usage (clipboard setup):

```cpp
		auto clipboard = GetCurrentController()->ClipboardService();
		{
				auto writer = clipboard->WriteClipboard();
				writer->SetText(L"XX");
				TEST_ASSERT(writer->Submit());
		}
		textBox->SetCaret(TextPos(0, 3), TextPos(0, 3));
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"012XX3456789");
```

```cpp
		textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"01XX56789");
```

```cpp
		{
				auto writer = clipboard->WriteClipboard();
				writer->SetText(L"12\r\n34\n\r\n56");
				TEST_ASSERT(writer->Submit());
		}
		textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"1234560123456789");
		TEST_ASSERT(textBox->GetText().IndexOf(L"\r") == -1);
		TEST_ASSERT(textBox->GetText().IndexOf(L"\n") == -1);
```

```cpp
		{
				auto writer = clipboard->WriteClipboard();
				TEST_ASSERT(writer->Submit());
		}
		TEST_ASSERT(!clipboard->ReadClipboard()->ContainsText());
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"0123456789");
```

Concrete `TEST_CASE` blocks to add under `TEST_CATEGORY(L"Clipboard")`:

```cpp
		TEST_CASE(L"CtrlV_PastesAtCaret")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								{
									auto writer = clipboard->WriteClipboard();
									writer->SetText(L"XX");
									TEST_ASSERT(writer->Submit());
								}

								textBox->SetCaret(TextPos(0, 3), TextPos(0, 3));
								protocol->KeyPress(VKEY::KEY_V, true, false, false);
								TEST_ASSERT(textBox->GetText() == L"012XX3456789");
						});

						protocol->OnNextIdleFrame(L"Pasted", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_PastesAtCaret"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});

		TEST_CASE(L"CtrlV_ReplacesSelection")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								{
									auto writer = clipboard->WriteClipboard();
									writer->SetText(L"XX");
									TEST_ASSERT(writer->Submit());
								}

								textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
								TEST_ASSERT(textBox->GetSelectionText() == L"234");
								protocol->KeyPress(VKEY::KEY_V, true, false, false);
								TEST_ASSERT(textBox->GetText() == L"01XX56789");
						});

						protocol->OnNextIdleFrame(L"Pasted", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_ReplacesSelection"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});

		TEST_CASE(L"CtrlV_FlattensLineBreaks")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								{
									auto writer = clipboard->WriteClipboard();
									writer->SetText(L"12\r\n34\n\r\n56");
									TEST_ASSERT(writer->Submit());
								}

								textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
								protocol->KeyPress(VKEY::KEY_V, true, false, false);
								TEST_ASSERT(textBox->GetText() == L"1234560123456789");
								TEST_ASSERT(textBox->GetText().IndexOf(L"\r") == -1);
								TEST_ASSERT(textBox->GetText().IndexOf(L"\n") == -1);
						});

						protocol->OnNextIdleFrame(L"Pasted", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_FlattensLineBreaks"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});

		TEST_CASE(L"CtrlV_EmptyClipboard_NoChange")
		{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								textBox->SetFocused();
								protocol->TypeString(L"0123456789");
						});

						protocol->OnNextIdleFrame(L"Typing", [=]()
						{
								auto window = GetApplication()->GetMainWindow();
								auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
								auto clipboard = GetCurrentController()->ClipboardService();

								{
									auto writer = clipboard->WriteClipboard();
									TEST_ASSERT(writer->Submit());
								}
								TEST_ASSERT(!clipboard->ReadClipboard()->ContainsText());

								textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
								protocol->KeyPress(VKEY::KEY_V, true, false, false);
								TEST_ASSERT(textBox->GetText() == L"0123456789");

								window->Hide();
						});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/")
						+ controlName
						+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_EmptyClipboard_NoChange"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource
				);
		});
```

## STEP 6: Test plan (build only) [DONE]

What to run:

- Use the existing VS Code task: `Build Unit Tests`.

What to validate:

- The solution compiles with the new clipboard test cases.

# FIXING ATTEMPTS

## Fixing attempt No.1

The build failed because the new clipboard test used `WString::IndexOf(...)` with wide string literals (`L"\r"`, `L"\n"`).
`IndexOf(T)` expects a single `wchar_t`, so passing a `const wchar_t[2]` caused `error C2664`.

I changed the assertions to pass character literals (`L'\r'` and `L'\n'`), which matches the expected overload and should fix the compilation errors.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

No user edit found
