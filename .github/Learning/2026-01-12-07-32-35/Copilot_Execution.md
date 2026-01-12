# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Add multiline key test entry point [DONE]

Edit: Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp

### 1.1 Insert `RunTextBoxKeyTestCases_Multiline` [DONE]

Insert the following template helper after `RunTextBoxKeyTestCases_Singleline` (and before `TEST_FILE`).

```
template<typename TTextBox>
void RunTextBoxKeyTestCases_Multiline(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
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

		TEST_CASE(L"CtrlEnter_BehavesSameAsEnter")
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
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);

					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"CDE");
				});

				protocol->OnNextIdleFrame(L"Pressed Ctrl+Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/EnterCRLF_CtrlEnter_BehavesSameAsEnter"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Clipboard (Multiline)")
	{
		TEST_CASE(L"CtrlV_PreservesMultipleLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto clipboard = GetCurrentController()->ClipboardService();
					textBox->SetFocused();
					{
						auto writer = clipboard->WriteClipboard();
						writer->SetText(L"12\r\n34\n\r\n56");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Pasted", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 4);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"12");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"34");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"");
					TEST_ASSERT(document->paragraphs[3]->GetTextForReading() == L"56");

					// Move caret to ensure this frame introduces a UI change.
					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
				});

				protocol->OnNextIdleFrame(L"Verified", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlV_PreservesMultipleLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlC_CtrlV_Roundtrip_MultipleLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto clipboard = GetCurrentController()->ClipboardService();
					{
						auto writer = clipboard->WriteClipboard();
						writer->SetText(L"12\r\n34\n\r\n56");
						TEST_ASSERT(writer->Submit());
					}

					textBox->SetFocused();
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Pasted", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->KeyPress(VKEY::KEY_A, true, false, false);
					protocol->KeyPress(VKEY::KEY_C, true, false, false);
					protocol->KeyPress(VKEY::KEY_DELETE);
				});

				protocol->OnNextIdleFrame(L"Copied and Cleared", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->KeyPress(VKEY::KEY_V, true, false, false);
				});

				protocol->OnNextIdleFrame(L"Pasted Again", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 4);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"12");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"34");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"");
					TEST_ASSERT(document->paragraphs[3]->GetTextForReading() == L"56");

					// Move caret to ensure this frame introduces a UI change.
					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));
				});

				protocol->OnNextIdleFrame(L"Verified", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_CtrlC_CtrlV_Roundtrip_MultipleLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	}
}
```

### 1.2 Update `GuiMultilineTextBox` test category to call it [DONE]

Find the existing category:

```
TEST_CATEGORY(L"GuiMultilineTextBox")
{
	RunTextBoxKeyTestCases<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
});
```

Replace it with:

```
TEST_CATEGORY(L"GuiMultilineTextBox")
{
	RunTextBoxKeyTestCases<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
	RunTextBoxKeyTestCases_Multiline<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
});
```

## STEP 2: Fix Enter handling in `GuiDocumentCommonInterface` for multiline mode (only if tests fail)

Edit: Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp

In `GuiDocumentCommonInterface::ProcessKey`, locate `case VKEY::KEY_RETURN:`.

Inside the `if (editMode == GuiDocumentEditMode::Editable)` block and after the `Singleline` early-return, replace the existing `if (ctrl) { ... } else { ... }` logic with the following:

```
const auto paragraphMode = config.paragraphMode;
if (paragraphMode == GuiDocumentParagraphMode::Paragraph && ctrl && !config.pasteAsPlainText)
{
	Array<WString> text(1);
	text[0] = L"\r\n";
	EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
}
else
{
	Array<WString> text(2);
	EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
}
```

## STEP 3: Run unit tests

- Build: run the VS Code task `Build Unit Tests`.
- Execute: run the VS Code task `Run Unit Tests`.

# FIXING ATTEMPTS

## Fixing attempt No.1

Why it failed:

- `CtrlEnter_BehavesSameAsEnter` in multiline mode expected Ctrl+Enter to split into 2 paragraphs, but Ctrl+Enter inserted an in-paragraph `\r\n`, keeping `document->paragraphs.Count()` at 1.

What I changed:

- Updated `GuiDocumentCommonInterface::ProcessKey` (`case VKEY::KEY_RETURN`) to only treat Ctrl+Enter as an in-paragraph line break when `paragraphMode == Paragraph` **and** `pasteAsPlainText == false`.

Why this should fix it:

- Plain-text text boxes (`pasteAsPlainText == true`, including `GuiMultilineTextBox`) should treat Ctrl+Enter the same as Enter.
- Rich-text controls (`pasteAsPlainText == false`) can still use Ctrl+Enter to insert a line break inside a paragraph when in `Paragraph` mode.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

- In `GuiDocumentCommonInterface::ProcessKey` for `VKEY::KEY_RETURN`, the final code gates the in-paragraph `"\r\n"` insertion on `ctrl && paragraphMode == GuiDocumentParagraphMode::Paragraph` only (no `pasteAsPlainText` condition).
	- This differs from the documented fixing attempt (which additionally required `pasteAsPlainText == false`).
	- This makes sense for the plain-text paragraph-mode controls (e.g. `GuiDocumentTextBox`, `GuiDocumentLabel`, `GuiDocumentViewer`), where `CTRL+ENTER` should still insert an in-paragraph line break even though `pasteAsPlainText == true`.
