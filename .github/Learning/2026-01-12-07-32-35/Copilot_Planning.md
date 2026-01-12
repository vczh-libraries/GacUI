# !!!PLANNING!!!

# UPDATES

## UPDATE

One tips for organizing frames
```
				protocol->OnNextIdleFrame(L"Press Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					textBox->SetCaret(TextPos(0, 2), TextPos(0, 2));
					protocol->KeyPress(VKEY::KEY_RETURN);

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"CDE");

					window->Hide();
				});
```

Pressing ENTER definitely changes the UI, so we can have a last frame only for window->Hide();. The purpose is to log the UI state after pressing ENTER to the file, so I can check it in a very straight forward way later.

Check all proposed test cases, verify if any step changes or does not change the UI, adjust frame boundaries accordingly if necessary.

# EXECUTION PLAN

## STEP 1: Add multiline key test entry point

Change [Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp](Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp) so `GuiMultilineTextBox` test category calls a new helper `RunTextBoxKeyTestCases_Multiline`.

Why:

- The task requires real test cases so `RunTextBoxKeyTestCases_Multiline` is not empty.
- Keeping multiline-only cases in a dedicated helper matches the current structure (`RunTextBoxKeyTestCases_Singleline`) and avoids mixing semantics across paragraph modes.

Proposed code changes:

1) Add a new helper (template, matching the style of `RunTextBoxKeyTestCases_Singleline`):

	template<typename TTextBox>
	void RunTextBoxKeyTestCases_Multiline(const wchar_t* resource, const WString& controlName)
		requires(std::is_base_of_v<GuiControl, TTextBox> && std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
	{
		// Test cases will be added in STEP 2 and STEP 3.
	}

2) Update the category:

	TEST_CATEGORY(L"GuiMultilineTextBox")
	{
		RunTextBoxKeyTestCases<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
		RunTextBoxKeyTestCases_Multiline<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
	});

## STEP 2: Add ENTER / CTRL+ENTER semantics tests for multiline mode

Implement key tests in `RunTextBoxKeyTestCases_Multiline` for `GuiDocumentParagraphMode::Multiline` (used by `GuiMultilineTextBox`).

Why:

- The design request requires: `ENTER` and `CTRL+ENTER` do the same thing in multiline mode and both insert a CRLF (represented structurally as a new line).
- Current implementation differs between `ENTER` and `CTRL+ENTER`, so tests are needed to lock behavior and prevent regressions.

Test case requirements:

- For every new assertion about content:
  - Do not call `GuiDocumentCommonInterface::GetText()`.
  - Validate paragraph count via `GuiDocumentCommonInterface::GetDocument()->paragraphs.Count()`.
  - Validate each paragraph’s content via `DocumentParagraphRun::GetTextForReading()`.

Proposed test cases (all under `RunTextBoxKeyTestCases_Multiline`):

Frame-boundary requirements (apply to all cases in STEP 2 and STEP 3):

- The UI snapshot for each frame is captured **before** executing the frame callback.
- Each `protocol->OnNextIdleFrame` callback must introduce UI change, otherwise the unit test framework will fail.
- For key actions like `ENTER`, add an additional final frame that only calls `window->Hide();` when possible, so the “after action” UI state is logged to the frame file.

1) `Enter_SplitsIntoTwoLines`

	TEST_CATEGORY(L"Enter/CRLF (Multiline)")
	{
		TEST_CASE(L"Enter_SplitsIntoTwoLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"ABCDE");
				});

				protocol->OnNextIdleFrame(L"Typed ABCDE", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					textBox->SetCaret(TextPos(0, 2), TextPos(0, 2));
					protocol->KeyPress(VKEY::KEY_RETURN);

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"CDE");
				});

				protocol->OnNextIdleFrame(L"Pressed Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/EnterCRLF_Enter_SplitsIntoTwoLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	}

2) `CtrlEnter_BehavesSameAsEnter`

- Same setup as above, but press `protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);`.
- Assert the same document structure and paragraph texts.

Frame boundary adjustment:

- Keep the last frame only calling `window->Hide();` so the “after Ctrl+Enter” UI state is logged.

Notes:

- This intentionally validates behavior at the document model level (paragraph count + each paragraph text), which is the task requirement.

## STEP 3: Add CRLF preservation tests for multiline clipboard paste and roundtrip

Add tests to ensure multi-line input is preserved in multiline mode:

- Any pasted text containing CRLF remains multi-line (not flattened).
- Copy/paste roundtrip preserves multiple lines.

Why:

- Multiline mode is expected to treat lines as structural (CRLF-delimited) rather than flattening line breaks (singleline behavior).
- The task specifically asks to include multi-line copy/paste cases.

Proposed test cases:

1) `CtrlV_PreservesMultipleLines`

- Put text with mixed line breaks into clipboard:

	auto clipboard = GetCurrentController()->ClipboardService();
	{
		auto writer = clipboard->WriteClipboard();
		writer->SetText(L"12\r\n34\n\r\n56");
		TEST_ASSERT(writer->Submit());
	}

- Paste with `CTRL+V`.
- Assert document paragraphs:
  - `Count() == 4`
  - `paragraphs[0]->GetTextForReading() == L"12"`
  - `paragraphs[1]->GetTextForReading() == L"34"`
  - `paragraphs[2]->GetTextForReading() == L""` (empty line)
  - `paragraphs[3]->GetTextForReading() == L"56"`

Frame boundary adjustment:

- Ensure there is a final frame containing only `window->Hide();` so the UI state after paste is logged.

2) `CtrlC_CtrlV_Roundtrip_MultipleLines`

- Arrange: create a multi-line document via paste (reuse the previous clipboard input), then `CTRL+A` to select all, then `CTRL+C`.
- Clear content using `KEY_BACK` or `KEY_DELETE` after selection (or set selection then delete).
- Paste again with `CTRL+V`.
- Assert that the final document paragraphs match the expected lines (count + per-paragraph `GetTextForReading()`).

Frame boundary adjustment:

- Split actions into frames only when each frame’s callback introduces a UI change (paste / select / delete / paste).
- Prefer a last frame containing only `window->Hide();` to log the “after final paste” UI state.

## STEP 4: Fix Enter handling in GuiDocumentCommonInterface for multiline mode

If STEP 2 fails, fix the underlying implementation in [Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp) so multiline mode treats `ENTER` and `CTRL+ENTER` the same.

Why:

- In multiline mode, `GuiDocumentCommonInterface::UserInput_FormatText` removes line breaks inside a paragraph, so inserting a literal `L"\r\n"` inside a paragraph is not a stable representation of a new line.
- Multiline mode already uses per-line paragraphs (because `doubleLineBreaksBetweenParagraph == false` causes `StringReader::ReadLine()` to create one paragraph per line), so Enter should create a new paragraph, regardless of `CTRL`.

Proposed code change (precise):

In `GuiDocumentCommonInterface::ProcessKey`, `case VKEY::KEY_RETURN:`, replace the `if (ctrl) { ... } else { ... }` logic with:

	const auto paragraphMode = config.paragraphMode;
	if (paragraphMode == GuiDocumentParagraphMode::Paragraph && ctrl)
	{
		collections::Array<WString> text(1);
		text[0] = L"\r\n";
		EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
	}
	else
	{
		collections::Array<WString> text(2);
		EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
	}

This preserves existing behavior:

- `GuiDocumentParagraphMode::Singleline`: already returns early and does nothing.
- `GuiDocumentParagraphMode::Paragraph`: keeps the distinction (`CTRL+ENTER` inserts a line break, `ENTER` inserts a new paragraph).
- `GuiDocumentParagraphMode::Multiline`: `CTRL+ENTER` behaves the same as `ENTER` (new line).

# TEST PLAN

Run the existing unit test pipeline (no new scripts/files):

- Build: run the VS Code task `Build Unit Tests`.
- Execute: run the VS Code task `Run Unit Tests`.

Specific validations:

- `GuiMultilineTextBox` test category must include and execute `RunTextBoxKeyTestCases_Multiline` (ensures it is not empty).
- `Enter/CRLF (Multiline)` tests must pass:
  - `Enter_SplitsIntoTwoLines`
  - `CtrlEnter_BehavesSameAsEnter`
- Clipboard tests must pass:
  - `CtrlV_PreservesMultipleLines`
  - `CtrlC_CtrlV_Roundtrip_MultipleLines`

Regression checks (targeted):

- `GuiSinglelineTextBox` key tests must still pass (especially the existing Enter-ignored tests).
- Any existing paragraph-mode behaviors must remain unchanged (the change in STEP 4 is gated by `paragraphMode == Paragraph && ctrl`).

# !!!FINISHED!!!
