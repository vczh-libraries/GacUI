# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Extend RunTextBoxKeyTestCases_MultiParagraph with a new Navigation category

Add a new `TEST_CATEGORY(L"Navigation (Paragraph)")` to `RunTextBoxKeyTestCases_MultiParagraph` in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`.

Why:
- The helper currently covers Enter/CRLF and clipboard semantics for Paragraph mode, but it has no explicit keyboard navigation coverage.
- Adding navigation cases at the helper level automatically applies to all three Paragraph-mode controls that call it (`GuiDocumentTextBox`, `GuiDocumentLabel`, `GuiDocumentViewer`).

Code shape (insert after `TEST_CATEGORY(L"Clipboard (Paragraph)")`):

	TEST_CATEGORY(L"Navigation (Paragraph)")
	{
		// New test cases added in STEP 2..6
	}

All new test cases must follow the frame naming / organization used by `RunTextBoxKeyTestCases_Multiline`:
- First frame name must be `Ready`.
- Each subsequent frame name describes what the previous frame did.
- Each frame must introduce UI change; therefore every navigation key (or small sequence) will be followed by typing a marker character.

To avoid line wrapping in the unit-test renderer, all constructed text lines must remain short (use 1..6 visible characters per line, and prefer many paragraphs over long paragraphs).

## STEP 2: Add LEFT/RIGHT boundary navigation across paragraphs

Add a test case that validates Paragraph-mode boundary behavior:
- At the beginning of a paragraph, `LEFT` should move the caret to the end of the previous paragraph (when one exists).
- At the end of a paragraph, `RIGHT` should move the caret to the beginning of the next paragraph (when one exists).

Why:
- Paragraph-mode boundary rules differ from singleline/multiline behaviors and are easy to regress when caret calculation logic changes.

Proposed test case:

	TEST_CASE(L"LeftRight_Boundary_JumpsAcrossParagraphs")
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

			protocol->OnNextIdleFrame(L"Typed 3 paragraphs", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->TypeString(L"|");

				auto document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 3);
				TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"BBB");
				TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
			});

			protocol->OnNextIdleFrame(L"Caret at (1,0) and [LEFT]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p0 = document->paragraphs[0]->GetTextForReading();
				textBox->SetCaret(TextPos(0, p0.Length()), TextPos(0, p0.Length()));

				protocol->KeyPress(VKEY::KEY_RIGHT);
				protocol->TypeString(L"^");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 3);
				TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"AAA|");
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"^BBB");
				TEST_ASSERT(document->paragraphs[2]->GetTextForReading() == L"CCC");
			});

			protocol->OnNextIdleFrame(L"Caret at end of (0) and [RIGHT]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_LeftRight_Boundary_JumpsAcrossParagraphs"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

Notes:
- This deliberately asserts via `GetDocument()` contents so the caret move is reflected in logs and the test does not rely on pixel-based caret bounds.

## STEP 3: Add UP/DOWN movement across lines inside a paragraph and across paragraph boundaries

Add a test case to validate vertical movement in Paragraph mode:
- `UP` from the second line of a paragraph moves into the first line of the same paragraph.
- `DOWN` from the last line of a paragraph moves into the next paragraph (when one exists).

Why:
- Paragraph mode has two layers of vertical structure: explicit line breaks within a paragraph (Ctrl+Enter) and paragraph boundaries (Enter).
- This is a common regression point when caret-column preservation or “preferred X” changes.

Proposed test case (use symmetric short lines to keep column mapping predictable):

	TEST_CASE(L"UpDown_MovesAcrossLinesAndParagraphs")
	{
		TooltipTimer timer;
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetFocused();

				// Paragraph 0: "AAA\r\nBBB"
				protocol->TypeString(L"AAA");
				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
				protocol->TypeString(L"BBB");
				protocol->KeyPress(VKEY::KEY_RETURN);
				// Paragraph 1: "CCC\r\nDDD"
				protocol->TypeString(L"CCC");
				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
				protocol->TypeString(L"DDD");
			});

			protocol->OnNextIdleFrame(L"Typed 2 paragraphs with line breaks", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p0 = document->paragraphs[0]->GetTextForReading();
				auto crlf0 = p0.IndexOf(L"\r\n");
				TEST_ASSERT(crlf0 != -1);

				// Place caret on the second line of paragraph 0.
				textBox->SetCaret(TextPos(0, crlf0 + 2 + 1), TextPos(0, crlf0 + 2 + 1));
				protocol->KeyPress(VKEY::KEY_UP);
				protocol->TypeString(L"^");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 2);
				// Marker should be inserted into the first line (before the CRLF).
				auto updated0 = document->paragraphs[0]->GetTextForReading();
				TEST_ASSERT(updated0.IndexOf(L"^") != -1);
				TEST_ASSERT(updated0.IndexOf(L"^") < updated0.IndexOf(L"\r\n"));
			});

			protocol->OnNextIdleFrame(L"Caret in p0 line2 and [UP]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p0 = document->paragraphs[0]->GetTextForReading();
				auto crlf0 = p0.IndexOf(L"\r\n");
				TEST_ASSERT(crlf0 != -1);

				// Place caret on the last line of paragraph 0.
				textBox->SetCaret(TextPos(0, crlf0 + 2 + 1), TextPos(0, crlf0 + 2 + 1));
				protocol->KeyPress(VKEY::KEY_DOWN);
				protocol->TypeString(L"v");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 2);
				// Marker should be inserted in paragraph 1 (moved across paragraph boundary).
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading().IndexOf(L'v') != -1);
			});

			protocol->OnNextIdleFrame(L"Caret in p0 line2 and [DOWN]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_UpDown_MovesAcrossLinesAndParagraphs"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

Notes:
- This test asserts only what must be true for Paragraph mode (same paragraph for UP within a multi-line paragraph; next paragraph for DOWN at the bottom line).
- It avoids exact caret column assertions to keep the test stable across caret-column heuristics.

## STEP 4: Add HOME/END escalation semantics (line → paragraph → document)

Add a test case that verifies the multi-stage behavior described in the task:
- HOME/END first moves within the current visual line.
- If already at the line edge, HOME/END escalates to paragraph edge.
- If already at the paragraph edge, HOME/END escalates to document edge.

Why:
- HOME/END fallback chain is implemented explicitly and must remain consistent across refactors.
- Paragraph mode needs to validate both line scope (within paragraph via `\r\n`) and paragraph scope (across paragraphs).

Proposed test case:

To keep each test small and easy to diagnose, implement HOME and END escalation as two test cases.

Proposed HOME test case:

	TEST_CASE(L"Home_Escalation_Line_Paragraph_Document")
	{
		TooltipTimer timer;
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetFocused();

				// Paragraph 0: "P0"
				protocol->TypeString(L"P0");
				protocol->KeyPress(VKEY::KEY_RETURN);

				// Paragraph 1: "AB\r\nCD"
				protocol->TypeString(L"AB");
				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
				protocol->TypeString(L"CD");
				protocol->KeyPress(VKEY::KEY_RETURN);

				// Paragraph 2: "P2"
				protocol->TypeString(L"P2");
			});

			protocol->OnNextIdleFrame(L"Typed 3 paragraphs (middle has 2 lines)", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p1 = document->paragraphs[1]->GetTextForReading();
				TEST_ASSERT(p1.IndexOf(L"\r\n") != -1);

				// HOME (1): end of paragraph -> start of last line.
				textBox->SetCaret(TextPos(1, p1.Length()), TextPos(1, p1.Length()));
				protocol->KeyPress(VKEY::KEY_HOME);
				protocol->TypeString(L"|");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading() == L"AB\r\n|CD");
			});

			protocol->OnNextIdleFrame(L"Caret at end of p1 and [HOME]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p1 = document->paragraphs[1]->GetTextForReading();
				auto crlf = p1.IndexOf(L"\r\n");
				TEST_ASSERT(crlf != -1);

				// HOME (2): at start of line -> escalate to paragraph start.
				textBox->SetCaret(TextPos(1, crlf + 2), TextPos(1, crlf + 2));
				protocol->KeyPress(VKEY::KEY_HOME);
				protocol->TypeString(L"!");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading().Left(1) == L"!");
			});

			protocol->OnNextIdleFrame(L"Caret at start of p1 and [HOME]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				// HOME (3): at paragraph start -> escalate to document start.
				textBox->SetCaret(TextPos(1, 0), TextPos(1, 0));
				protocol->KeyPress(VKEY::KEY_HOME);
				protocol->TypeString(L"$");

				auto document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs[0]->GetTextForReading().Left(1) == L"$");
			});

			protocol->OnNextIdleFrame(L"Pressed [HOME] at paragraph start", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_Home_Escalation_Line_Paragraph_Document"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

Proposed END test case:

	TEST_CASE(L"End_Escalation_Line_Paragraph_Document")
	{
		TooltipTimer timer;
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetFocused();

				// Paragraph 0: "P0"
				protocol->TypeString(L"P0");
				protocol->KeyPress(VKEY::KEY_RETURN);

				// Paragraph 1: "AB\r\nCD"
				protocol->TypeString(L"AB");
				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);
				protocol->TypeString(L"CD");
				protocol->KeyPress(VKEY::KEY_RETURN);

				// Paragraph 2: "P2"
				protocol->TypeString(L"P2");
			});

			protocol->OnNextIdleFrame(L"Typed 3 paragraphs (middle has 2 lines)", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				// END (1): middle of first line -> end of that line.
				textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
				protocol->KeyPress(VKEY::KEY_END);
				protocol->TypeString(L"^");

				auto document = textBox->GetDocument();
				auto p1 = document->paragraphs[1]->GetTextForReading();
				auto crlf = p1.IndexOf(L"\r\n");
				TEST_ASSERT(crlf != -1);
				TEST_ASSERT(p1.IndexOf(L'^') != -1);
				TEST_ASSERT(p1.IndexOf(L'^') < crlf);
			});

			protocol->OnNextIdleFrame(L"Caret in p1 line1 and [END]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p1 = document->paragraphs[1]->GetTextForReading();
				auto crlf = p1.IndexOf(L"\r\n");
				TEST_ASSERT(crlf != -1);

				// END (2): at end of line -> escalate to paragraph end.
				textBox->SetCaret(TextPos(1, crlf), TextPos(1, crlf));
				protocol->KeyPress(VKEY::KEY_END);
				protocol->TypeString(L"$");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs[1]->GetTextForReading().Right(1) == L"$");
			});

			protocol->OnNextIdleFrame(L"Caret at end of p1 and [END]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");

				auto document = textBox->GetDocument();
				auto p1 = document->paragraphs[1]->GetTextForReading();

				// END (3): at paragraph end -> escalate to document end (paragraph 2 end).
				textBox->SetCaret(TextPos(1, p1.Length()), TextPos(1, p1.Length()));
				protocol->KeyPress(VKEY::KEY_END);
				protocol->TypeString(L"#");

				document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs[2]->GetTextForReading().Right(1) == L"#");
			});

			protocol->OnNextIdleFrame(L"Pressed [END] at paragraph end", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_End_Escalation_Line_Paragraph_Document"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

Notes:
- Both tests use marker insertion + `GetDocument()` assertions so each frame has a UI change and the caret position is visible in frame logs.
- Indexes around CRLF are computed from paragraph text to avoid relying on any special casing of `\r\n` in caret column counting.

## STEP 5: Add CTRL+HOME / CTRL+END document boundary tests

Add a test case that validates `CTRL+HOME` and `CTRL+END` always jump to document start/end.

Why:
- These are explicit document-level commands; they must remain stable across all editor implementations.

Proposed test case (same structure as multiline version, adapted to paragraphs):

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

			protocol->OnNextIdleFrame(L"Typed 3 paragraphs", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
				protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
				protocol->TypeString(L"|");

				auto document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 3);
				TEST_ASSERT(document->paragraphs[0]->GetTextForReading() == L"|AAA");
			});

			protocol->OnNextIdleFrame(L"Caret at (1,1) and [CTRL+HOME]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetCaret(TextPos(1, 1), TextPos(1, 1));
				protocol->KeyPress(VKEY::KEY_END, true, false, false);
				protocol->TypeString(L"#");

				auto document = textBox->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 3);
				TEST_ASSERT(document->paragraphs[2]->GetTextForReading().Right(1) == L"#");
			});

			protocol->OnNextIdleFrame(L"Caret at (1,1) and [CTRL+END]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_CtrlHomeEnd_JumpsToDocumentEdges"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

## STEP 6: Add PAGE UP / PAGE DOWN tests using marker search (directionality only)

Add a test case that validates PageUp/PageDown caret movement direction without asserting an exact moved count.

Why:
- Page operations depend on viewport height and can change when renderer metrics change.
- Directionality can be asserted robustly by inserting a marker and locating it by paragraph index, matching the approach in multiline tests.

Proposed test case:

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
					protocol->TypeString(WString::Unmanaged(L"P") + itow(i));
					if (i != 39)
					{
						protocol->KeyPress(VKEY::KEY_RETURN);
					}
				}
			});

			protocol->OnNextIdleFrame(L"Typed 40 paragraphs", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetCaret(TextPos(30, 1), TextPos(30, 1));
				protocol->KeyPress(VKEY::KEY_PRIOR);
				protocol->TypeString(L"^");

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

			protocol->OnNextIdleFrame(L"Caret at (30,1) and [PAGE UP]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetCaret(TextPos(5, 1), TextPos(5, 1));
				protocol->KeyPress(VKEY::KEY_NEXT);
				protocol->TypeString(L"v");

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
			});

			protocol->OnNextIdleFrame(L"Caret at (5,1) and [PAGE DOWN]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/")
			+ controlName
			+ WString::Unmanaged(L"/Key/NavigationParagraph_PageUpPageDown_MovesVerticallyByViewport"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	}

# TEST PLAN

- Build: run the existing VS Code task `Build Unit Tests`.
- Execute: run the existing VS Code task `Run Unit Tests`.
- Coverage expectation: the new Paragraph navigation cases execute three times (once per control) because `TEST_FILE` calls `RunTextBoxKeyTestCases_MultiParagraph` for `GuiDocumentTextBox`, `GuiDocumentLabel`, and `GuiDocumentViewer`.
- Failure diagnosis:
  - If failures are caret-semantics related (HOME/END escalation, paragraph boundary), check `GuiDocumentCommonInterface::ProcessKey` and document renderer caret calculation.
  - If failures only reproduce in unit tests and seem renderer-stub related (unexpected wrapping / line calculation), reduce line lengths further and keep assertions based on paragraph/marker placement.

# !!!FINISHED!!!
