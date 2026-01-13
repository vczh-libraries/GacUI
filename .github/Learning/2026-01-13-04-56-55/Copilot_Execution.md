# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Target file [DONE]

Edit the following file:

- `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`

Goal:

- Add a new helper `RunTextBoxKeyTestCases_MultiParagraph` for Paragraph-mode ENTER/CRLF behaviors.
- Wire it into the 3 Paragraph-mode editor controls in this file:
	- `GuiDocumentTextBox`
	- `GuiDocumentLabel`
	- `GuiDocumentViewer`

Constraint:

- All new content assertions must use `GetDocument()` + `DocumentParagraphRun::GetTextForReading()` (not `GetText()`).

## STEP 2: Add `RunTextBoxKeyTestCases_MultiParagraph` helper [DONE]

In `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, add the following new helper template.

Recommended insertion location:

- Insert it after `RunTextBoxKeyTestCases_Multiline(...)` and before the `TEST_FILE` block (so all helper templates are grouped).

Code to insert:

```cpp
template<typename TTextBox>
void RunTextBoxKeyTestCases_MultiParagraph(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
	TEST_CATEGORY(L"Enter/CRLF (Paragraph)")
	{
		TEST_CASE(L"Enter_CreatesNewParagraph")
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
				+ WString::Unmanaged(L"/Key/EnterCRLF_Paragraph_Enter_CreatesNewParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlEnter_InsertsLineBreakInParagraph")
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
					TEST_ASSERT(document->paragraphs.Count() == 1);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AB\r\nCDE");
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
				+ WString::Unmanaged(L"/Key/EnterCRLF_Paragraph_CtrlEnter_InsertsLineBreakInParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

	TEST_CATEGORY(L"Clipboard (Paragraph)")
	{
		TEST_CASE(L"CtrlV_SingleCRLF_StaysInParagraph")
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
						writer->SetText(L"a\r\nb");
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
					TEST_ASSERT(document->paragraphs.Count() == 1);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a\r\nb");
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
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_SingleCRLF_StaysInParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_DoubleCRLF_CreatesNewParagraph")
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
						writer->SetText(L"a\r\n\r\nb");
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
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"b");
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
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_DoubleCRLF_CreatesNewParagraph"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlV_TripleCRLF_FollowsParserRule")
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
						writer->SetText(L"a\r\n\r\n\r\nb");
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
					TEST_ASSERT(document->paragraphs.Count() == 2);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"a");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"\r\nb");
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
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlV_TripleCRLF_FollowsParserRule"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlC_CtrlV_Roundtrip_PreservesParagraphAndLineBreakStructure")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AB");
					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					protocol->TypeString(L"CD");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"EF");
				});

				protocol->OnNextIdleFrame(L"Typed 2 paragraphs with a line break", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

					auto before = textBox->GetDocument();
					TEST_ASSERT(before->paragraphs.Count() == 2);
					TEST_ASSERT(before->paragraphs[0]->GetTextForReading() == L"AB\r\nCD");
					TEST_ASSERT(before->paragraphs[1]->GetTextForReading() == L"EF");

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

					auto after = textBox->GetDocument();
					TEST_ASSERT(after->paragraphs.Count() == 2);
					TEST_ASSERT(after->paragraphs[0]->GetTextForReading() == L"AB\r\nCD");
					TEST_ASSERT(after->paragraphs[1]->GetTextForReading() == L"EF");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Clipboard_Paragraph_CtrlC_CtrlV_Roundtrip_PreservesParagraphAndLineBreakStructure"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}
```

## STEP 3: Wire helper into the three Paragraph-mode control categories [DONE]

In `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, locate the bottom `TEST_FILE` categories for:

- `TEST_CATEGORY(L"GuiDocumentTextBox")`
- `TEST_CATEGORY(L"GuiDocumentLabel")`
- `TEST_CATEGORY(L"GuiDocumentViewer")`

Update them to call the new helper.

Also fix the incorrect template argument currently used in the `GuiDocumentTextBox` category.

Replace the entire three categories with the following code:

```cpp
	TEST_CATEGORY(L"GuiDocumentTextBox")
	{
		RunTextBoxKeyTestCases<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	}

	TEST_CATEGORY(L"GuiDocumentLabel")
	{
		RunTextBoxKeyTestCases<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
	}

	TEST_CATEGORY(L"GuiDocumentViewer")
	{
		RunTextBoxKeyTestCases<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
	}
```

## STEP 4: Validate

Use the existing VS Code tasks:

- Build Unit Tests
- Run Unit Tests

If a test fails, check frame logs to determine whether the failure is in:

- ENTER paragraph creation vs CTRL+ENTER in-paragraph line break
- Paste parsing for 1 / 2 / 3 consecutive CRLF
- Copy/paste roundtrip preserving paragraph and line-break structure

# FIXING ATTEMPTS

## Fixing attempt No.1

The change introduced new tests referencing `GuiDocumentTextBox`, but `GuiDocumentTextBox` is not a real C++ class type in this repo. In the XML compiler, `presentation::controls::GuiDocumentTextBox` is created as a *virtual type* based on `GuiDocumentLabel` (see `LoadDocumentControls` in `Source/Compiler/InstanceLoaders/GuiInstanceLoader_Document.cpp`).

To keep the execution plan’s naming (`GuiDocumentTextBox`) while still compiling and matching what `<DocumentTextBox>` actually instantiates, I added a local alias in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`:

- `using GuiDocumentTextBox = GuiDocumentLabel;`

This makes `FindObjectByName<GuiDocumentTextBox>(...)` behave the same as `FindObjectByName<GuiDocumentLabel>(...)`, which is consistent with the virtual-type behavior.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

The final checked-in implementation differs from this execution record in a few places that look like manual cleanups (not explained by the fixing attempt above):

- In `RunTextBoxKeyTestCases_MultiParagraph` clipboard test cases, the last “hide window” frame was merged into the verification frame.
	- The final code uses only `Ready` → `Pasted` (and calls `window->Hide();` at the end of `Pasted`) instead of `Ready` → `Pasted` → `Verified`.
	- This still satisfies the “each frame must introduce UI change” constraint, while keeping the log shorter.
- The extra caret-reset calls (e.g. `textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));`) in clipboard verification frames were removed.
	- This avoids introducing an extra, unneeded UI change right after assertions.

Learning: prefer the smallest number of frames that still (1) keeps names meaningful and (2) guarantees a UI change per frame; merging `window->Hide();` into the last verification frame is acceptable and often cleaner.
