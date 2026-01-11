# !!!PLANNING!!!

# UPDATES

## UPDATE

In `CtrlC_CopiesSelection_TextUnchanged` you can set the window's title to the text from the clipboard reader, so that it is also logged in the frame. By doing this UI is changed so that it is legal to leave the last frame simple (aka calling Hide())

# EXECUTION PLAN

## STEP 1: Confirm the existing test scaffold and APIs

What to change

- Inspect the existing template-based test entry point:
	- `RunSinglelineTextBoxTestCases<TTextBox>(resource, controlName)` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`.
- Reuse the existing interaction pattern:
	- First frame: focus the control and seed initial text via `protocol->TypeString(...)`.
	- Next frame: perform keyboard input and do all assertions in the same callback.
	- Only schedule an extra frame when the current callback issues an input that changes UI.
	- If an input is issued, add one more final frame whose only job is `window->Hide();`.

Why

- TASK No.6 requires the same frame discipline as TASK No.2 to keep logs deterministic and easy to inspect.
- Existing tests in this file already establish the exact `UnitTestRemoteProtocol::KeyPress` signature used for modifier keys:
	- `protocol->KeyPress(VKEY::KEY_LEFT, /*ctrl*/false, /*shift*/true, /*alt*/false);`

## STEP 2: Fix FakeClipboardService so tests can set/clear clipboard deterministically

What to change

- In `Source/Utilities/FakeServices/GuiFakeClipboardService.cpp`, ensure `FakeClipboardService::WriteClipboard()` returns a writer that can actually stage clipboard content and submit it.
- Initialize `FakeClipboardWriter::reader` to a fresh `FakeClipboardReader` in the constructor, so `SetText(...)` and `Submit()` work.

Concrete code change

In `FakeClipboardWriter`:

		FakeClipboardWriter(FakeClipboardService* _service)
				: service(_service)
		{
				reader = Ptr(new FakeClipboardReader);
		}

Why

- The TASK requires tests to use `GetCurrentController()->ClipboardService()->WriteClipboard()` / `ReadClipboard()`.
- Without a working fake writer, tests cannot:
	- Pre-fill clipboard text for Ctrl+V.
	- Clear clipboard content for the empty-clipboard case.
- This fix keeps tests using the public clipboard API (no downcasts, no test-only backdoors).

## STEP 3: Add TEST_CATEGORY(L"Clipboard") under the existing singleline textbox shared test function

What to change

- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, inside `RunSinglelineTextBoxTestCases<TTextBox>`, add:
	- `TEST_CATEGORY(L"Clipboard")`.
- Follow the log identity rule exactly:
	- `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/Clipboard_<CASE>")`.

Concrete scaffold

		TEST_CATEGORY(L"Clipboard")
		{
				// TEST_CASE(...) blocks go here.
		});

Why

- Keeps the new keyboard shortcut tests grouped and consistent with the file’s existing category layout.
- Ensures the log folder structure remains stable and searchable.

## STEP 4: Implement Ctrl+C and Ctrl+X test cases (copy/cut)

What to change

- Add the following `TEST_CASE`s under `TEST_CATEGORY(L"Clipboard")` in the shared template function:
	- `CtrlC_CopiesSelection_TextUnchanged`
	- `CtrlX_CutsSelection_TextUpdated`
- Use deterministic selection setup by calling `textBox->SetCaret(begin, end)` directly (avoid shift-selection inputs).
- Use clipboard service read-back assertions:
	- `GetCurrentController()->ClipboardService()->ReadClipboard()->ContainsText()`
	- `GetText()`

Concrete code skeleton

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

Why

- Validates the core contract of Ctrl+C/Ctrl+X: selection text is copied to clipboard.
- Validates the core contract of Ctrl+C/Ctrl+X: selection text is copied to clipboard.
- Ensures editing state for cut is correct (selection removed), while copy keeps the text unchanged.
- For Ctrl+C, `window->SetText(reader->GetText())` is intentionally used to:
	- Log the clipboard content in the frame.
	- Ensure UI changes after the key press, so scheduling the final hide-only frame remains consistent with the frame rule.

## STEP 5: Implement Ctrl+V test cases (paste), including single-line flattening behavior

What to change

- Add the following `TEST_CASE`s under `TEST_CATEGORY(L"Clipboard")`:
	- `CtrlV_PastesAtCaret`
	- `CtrlV_ReplacesSelection`
	- `CtrlV_FlattensLineBreaks`
	- `CtrlV_EmptyClipboard_NoChange`
- Use clipboard writer to set up paste content:
	- `auto writer = clipboard->WriteClipboard(); writer->SetText(...); writer->Submit();`
- For the flattening case, ensure clipboard contains `\r\n` and/or `\n`, paste, then assert:
	- No line breaks exist in the textbox result.
	- Segments are concatenated with no extra spaces (because `spaceForFlattenedLineBreak = false`).

Concrete code skeleton (key parts)

Pastes at caret:

		auto clipboard = GetCurrentController()->ClipboardService();
		{
				auto writer = clipboard->WriteClipboard();
				writer->SetText(L"XX");
				TEST_ASSERT(writer->Submit());
		}
		textBox->SetCaret(TextPos(0, 3), TextPos(0, 3));
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"012XX3456789");

Replaces selection:

		textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"01XX56789");

Flattens line breaks:

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

Empty clipboard (requires the fake clipboard writer to support “submit without any Set*” as clear):

		{
				auto writer = clipboard->WriteClipboard();
				TEST_ASSERT(writer->Submit());
		}
		TEST_ASSERT(!clipboard->ReadClipboard()->ContainsText());
		protocol->KeyPress(VKEY::KEY_V, true, false, false);
		TEST_ASSERT(textBox->GetText() == L"0123456789");

Why

- Paste-at-caret and replace-selection cover the two primary edit modes used by Ctrl+V.
- Flattening is the singleline-specific behavior that is easy to regress (CR/LF normalization + no inserted spaces).
- Empty clipboard validates `CanPaste()` gating behavior (no paste when no text).

## STEP 6: Test plan (run and validate)

What to run

- Build and run unit tests using the existing VS Code task:
	- `Build and Run Unit Tests`

What to validate

- All new test logs appear under:
	- `Controls/Editor/<controlName>/Key/Clipboard_*`
- Each test contains:
	- A setup frame (`Init`) that only focuses and types initial content.
		- One action+assertion frame (usually `Typing`) that performs Ctrl+X/C/V and asserts immediately.
			- For Ctrl+C, the action frame also updates the window title from clipboard text, so the clipboard value is visible in logs.
	- A minimal final frame (`Copied` / `Cut` / `Pasted` / `Ready`) whose only job is `window->Hide();`.

Failure triage guidance

- If clipboard writer `Submit()` fails in remote tests, confirm STEP 2 is applied and that `FakeClipboardWriter` stages a new `FakeClipboardReader` per write session.
- If the empty-clipboard case reports `ContainsText()==true`, confirm the “clear” path uses a fresh reader with no `text` set (submit without calling `SetText`).

# !!!FINISHED!!!
