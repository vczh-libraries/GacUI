# !!!EXECUTION!!!

# UPDATES

## UPDATE

In `RunTextBoxKeyTestCases_Multiline`, test category `Navigation (Multiline)` does not follow the frame organization rule mentioned in #file:copilot-instructions.md . Learn the rule again and twist frames a little bit. The most serious problem is frame naming.

## UPDATE

I have manually reorganized `LeftRight_Boundary_JumpsAcrossLines` test case. You can compare it with the execution document, find out why my change is better following the `## Organizing Frames` instructions. Apply the idea to other test cases in the current task.

## User Update Spotted

- In `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, `LeftRight_Boundary_JumpsAcrossLines` has been reorganized to better follow `## Organizing Frames`.
- Other test cases under `Navigation (Multiline)` have now been adjusted to follow the same idea: action + verification in the same frame, and every frame name describes what the previous frame did.

# EXECUTION PLAN

## STEP 1: Add multiline navigation test coverage [DONE]

Update `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` in `RunTextBoxKeyTestCases_Multiline`:

- Insert a new `TEST_CATEGORY(L"Navigation (Multiline)")` after `TEST_CATEGORY(L"Clipboard (Multiline)")` and before the end of `RunTextBoxKeyTestCases_Multiline`.
- Add the following test cases under that new category.

NOTE: The code snippet below is the original insertion draft for STEP 1. After user feedback, frames in `Navigation (Multiline)` were reorganized to follow `## Organizing Frames` (action + verification in the same frame; every frame name describes what the previous frame did). The source code in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` is the current truth.

	Code to insert:

	TEST_CATEGORY(L"Navigation (Multiline)")
	{
		TEST_CASE(L"LeftRight_Boundary_JumpsAcrossLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCC");
				});

				protocol->OnNextIdleFrame(L"LEFT boundary", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"Verify LEFT boundary", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
				});

				protocol->OnNextIdleFrame(L"RIGHT boundary", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 3), TextPos(1, 3));
					protocol->KeyPress(VKEY::KEY_RIGHT);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"Verify RIGHT boundary", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"|CCC");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_LeftRight_Boundary_JumpsAcrossLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"UpDown_MovesAcrossLines")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAAAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBBBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCCCC");
				});

				protocol->OnNextIdleFrame(L"UP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"Verify UP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AA|AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBBBB");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCCCC");
				});

				protocol->OnNextIdleFrame(L"DOWN", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"Verify DOWN", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CC|CCC");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_UpDown_MovesAcrossLines"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"HomeEnd_Escalation_Line_Paragraph_Document")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCC");
				});

				protocol->OnNextIdleFrame(L"HOME (line)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"HOME (escalate to document)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
					protocol->KeyPress(VKEY::KEY_HOME);
					protocol->TypeString(L"!");
				});

				protocol->OnNextIdleFrame(L"END (line)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 2), TextPos(1, 2));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"^");
				});

				protocol->OnNextIdleFrame(L"END (escalate to document)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 5), TextPos(1, 5));
					protocol->KeyPress(VKEY::KEY_END);
					protocol->TypeString(L"$");
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"!AAA");
					TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"|BBB^");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC$");
					window->Hide();
				});
			});


		## STEP 2: Fix multiline navigation frame naming [DONE]

		- Update all `protocol->OnNextIdleFrame(...)` names in `TEST_CATEGORY(L"Navigation (Multiline)")` to follow the rule: the first frame is always "Ready", and every subsequent frame is named for what the previous frame does.
		- Keep the last frame as `window->Hide();` only when possible.
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_HomeEnd_Escalation_Line_Paragraph_Document"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"CtrlHomeEnd_JumpsToDocumentEdges")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"AAA");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"BBB");
					protocol->KeyPress(VKEY::KEY_RETURN);
					protocol->TypeString(L"CCC");
				});

				protocol->OnNextIdleFrame(L"CTRL+HOME", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
					protocol->TypeString(L"|");
				});

				protocol->OnNextIdleFrame(L"CTRL+END", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
					protocol->KeyPress(VKEY::KEY_END, true, false, false);
					protocol->TypeString(L"#");
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 3);
					TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"|AAA");
					TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC#");
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_CtrlHomeEnd_JumpsToDocumentEdges"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"PageUpPageDown_MovesVerticallyByViewport")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetFocused();
					for (vint i = 0; i < 40; i++)
					{
						protocol->TypeString(L"L" + itow(i));
						if (i != 39)
						{
							protocol->KeyPress(VKEY::KEY_RETURN);
						}
					}
				});

				protocol->OnNextIdleFrame(L"PAGE UP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(30, 1), TextPos(30, 1));
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->TypeString(L"^");
				});

				protocol->OnNextIdleFrame(L"Verify PAGE UP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 40);

					vint index = -1;
					for (vint i = 0; i < document->paragraphs.Count(); i++)
					{
						if (document->paragraphs[i]->GetTextForReading().IndexOf(L'^') != -1)
						{
							index = i;
							break;
						}
					}
					TEST_ASSERT(index != -1);
					TEST_ASSERT(index < 30);
				});

				protocol->OnNextIdleFrame(L"PAGE DOWN", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					textBox->SetCaret(TextPos(5, 1), TextPos(5, 1));
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->TypeString(L"v");
				});

				protocol->OnNextIdleFrame(L"Verify PAGE DOWN", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
					auto document = textBox->GetDocument();
					TEST_ASSERT(document->paragraphs.Count() == 40);

					vint index = -1;
					for (vint i = 0; i < document->paragraphs.Count(); i++)
					{
						if (document->paragraphs[i]->GetTextForReading().IndexOf(L'v') != -1)
						{
							index = i;
							break;
						}
					}
					TEST_ASSERT(index != -1);
					TEST_ASSERT(index > 5);

					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/NavigationMultiline_PageUpPageDown_MovesVerticallyByViewport"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});

## STEP 2: Fix HOME/END semantics in document key handling [DONE]

Update `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` in `GuiDocumentCommonInterface::ProcessKey`:

- Replace the whole `case VKEY::KEY_HOME:` block with this code.

	Code to write:

				case VKEY::KEY_HOME:
					{
						if (ctrl)
						{
							Move(TextPos(0, 0), shift, true);
							break;
						}

						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineFirst, frontSide);
						if (newCaret == currentCaret)
						{
							newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretFirst, frontSide);
						}
						if (newCaret == currentCaret)
						{
							newCaret = TextPos(0, 0);
						}
						Move(newCaret, shift, frontSide);
					}
					break;

- Replace the whole `case VKEY::KEY_END:` block with this code.

	Code to write:

				case VKEY::KEY_END:
					{
						if (ctrl)
						{
							vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
							auto lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];
							Move(TextPos(lastIndex, lastParagraph->GetTextForCaret().Length()), shift, false);
							break;
						}

						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineLast, frontSide);
						if (newCaret == currentCaret)
						{
							newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLast, frontSide);
						}
						if (newCaret == currentCaret)
						{
							vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
							auto lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];
							newCaret = TextPos(lastIndex, lastParagraph->GetTextForCaret().Length());
						}
						Move(newCaret, shift, frontSide);
					}
					break;

## STEP 3: Implement PAGE UP / PAGE DOWN caret movement [DONE]

Update `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` in `GuiDocumentCommonInterface::ProcessKey`:

- Replace the whole `case VKEY::KEY_PRIOR:` and `case VKEY::KEY_NEXT:` empty blocks with a combined implementation.
- Keep `KEY_PRIOR` / `KEY_NEXT` as no-op for `GuiDocumentParagraphMode::Singleline` (to preserve existing `Navigation_PageKeys_NoEffectInSingleline` coverage).

	Code to write:

				case VKEY::KEY_PRIOR:
				case VKEY::KEY_NEXT:
					{
						if (config.paragraphMode == GuiDocumentParagraphMode::Singleline) break;
						if (!documentMouseArea) break;
						vint page = documentMouseArea->GetCachedBounds().Height();
						if (page <= 0) break;

						Rect caretBounds = documentElement->GetCaretBounds(currentCaret, frontSide);
						vint x = caretBounds.x1;
						vint y = (caretBounds.y1 + caretBounds.y2) / 2;
						y += (code == VKEY::KEY_PRIOR ? -page : page);

						TextPos newCaret = documentElement->CalculateCaretFromPoint(Point(x, y));
						Move(newCaret, shift, frontSide);
					}
					break;

## STEP 4: Fix unit-test renderer only if required by failures

Only if STEP 1 tests fail due to unit-test-only caret geometry / caret-from-point behavior (rather than key handling):

- Apply minimal adjustments in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp` so `GetCaretBounds` and `GetCaretFromPoint`-related behavior is stable for non-wrapping inputs.
- Keep scope limited to making `PAGE UP` / `PAGE DOWN` and `UP` / `DOWN` stable for short (non-wrapping) lines.

# FIXING ATTEMPTS

## Fixing attempt No.1

- Ran VS Code task `Build Unit Tests`.
- Source of truth: `.github/TaskLogs/Build.log`.
- Result: **Build succeeded** with `0 Warning(s)` and `0 Error(s)`.
- No compiler errors/warnings related to `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, so no code changes were necessary.

## ATTEMPT 1

- Apply STEP 1–3 changes.
- Run unit tests via the existing VS/Task pipeline for the `TaskLogs` workflow.
- If failures point to caret geometry issues only in unit-test rendering, apply STEP 4 minimally.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

- In `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`, `LeftRight_Boundary_JumpsAcrossLines` was reorganized to better follow `## Organizing Frames` in `.github/copilot-instructions.md`.
	- Each non-Ready frame name describes what the previous frame did (e.g. `Ready` -> `Typed 3 lines` -> `Caret at (1,0) and [LEFT]`).
	- Action + verification were merged into the same frame, leaving a final hide-only frame when possible.
	- This makes logs easier to read because each screenshot corresponds to the UI state right before the next action, and avoids extra frames that only re-check state without changing UI.
