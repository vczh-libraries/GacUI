# !!!EXECUTION!!!

# UPDATES

(none)

# EXECUTION PLAN

## STEP 1: Add `Deletion` test category for Backspace/Delete/Enter [DONE]

Edit file:

- Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp

Insert location:

- Inside `RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)`
- Insert a new `TEST_CATEGORY(L"Deletion")` after the existing `TEST_CATEGORY(L"Navigation")` block and before the end of `RunSinglelineTextBoxTestCases`.

In the new `TEST_CATEGORY(L"Deletion")`, add the following `TEST_CASE`s (paste them in this order).

### Backspace_DeletesPreviousChar

```
		TEST_CASE(L"Backspace_DeletesPreviousChar")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"012356789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 4));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Backspace_DeletesPreviousChar"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### Delete_DeletesNextChar

```
		TEST_CASE(L"Delete_DeletesNextChar")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetText() == L"0123456789");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"012346789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Delete_DeletesNextChar"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### BackspaceAtStart_NoChange

```
		TEST_CASE(L"BackspaceAtStart_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 0), TextPos(0, 0));

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 0));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_BackspaceAtStart_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### DeleteAtEnd_NoChange

```
		TEST_CASE(L"DeleteAtEnd_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 10), TextPos(0, 10));

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetCaretBegin() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetCaretEnd() == TextPos(0, 10));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_DeleteAtEnd_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### Backspace_DeletesSelection

```
		TEST_CASE(L"Backspace_DeletesSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Backspace", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_BACK);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Backspace_DeletesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### Delete_DeletesSelection

```
		TEST_CASE(L"Delete_DeletesSelection")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_DELETE);
					TEST_ASSERT(textBox->GetText() == L"0156789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Delete_DeletesSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### Enter_Ignored_NoSelection_NoChange

```
		TEST_CASE(L"Enter_Ignored_NoSelection_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_NoSelection_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### Enter_Ignored_SelectionPreserved

```
		TEST_CASE(L"Enter_Ignored_SelectionPreserved")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_Enter_Ignored_SelectionPreserved"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### CtrlEnter_Ignored_NoSelection_NoChange

```
		TEST_CASE(L"CtrlEnter_Ignored_NoSelection_NoChange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 5), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_NoSelection_NoChange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

### CtrlEnter_Ignored_SelectionPreserved

```
		TEST_CASE(L"CtrlEnter_Ignored_SelectionPreserved")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Enter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetCaret(TextPos(0, 2), TextPos(0, 5));
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					TEST_ASSERT(textBox->GetSelectionText() == L"234");

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Deletion_CtrlEnter_Ignored_SelectionPreserved"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
```

## STEP 2: Ignore Enter / Ctrl+Enter in Singleline paragraph mode [DONE]

Edit file:

- Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp

Update location:

- Function: `vl::presentation::controls::GuiDocumentCommonInterface::ProcessKey`
- In the `switch (key)` statement, update `case VKEY::KEY_RETURN:`.

Replace the existing `case VKEY::KEY_RETURN:` with the following:

```
				case VKEY::KEY_RETURN:
					if (editMode == GuiDocumentEditMode::Editable)
					{
						if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
						{
							// Singleline text boxes should ignore Enter entirely.
							// In particular, Enter must not delete selection.
							return true;
						}
						if (ctrl)
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
						return true;
					}
					break;
```

# FIXING ATTEMPTS

(none)

# Comparing to User Edit

- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`, frame labels follow the KB rule that a frame title describes what happened in the previous frame (e.g. after `Init` which seeds text, the next frame is labeled `Typing`).
- For deletion cases, `window->Hide()` is moved into a final dedicated frame so the last frame is minimal; the action+assert frame no longer hides the window.
- When adding more key tests, label frames for the previous frame, and prefer `Init` (focus + `TypeString`) -> action+assert in one frame -> (if an input was issued) a final hide-only frame whose title describes the previous action frame.

# !!!FINISHED!!!
