# !!!EXECUTION!!!

# UPDATES

## UPDATE

In order to avoid confusion, the function you want to edit has already been moved to #file:TestControls_Editor_Key_Paragraph.cpp . Do not update other files if possible. Update the document to mark it down

## UPDATE

I do not like the `FindCrlf` approach, as in test cases everything is deterministic, you should just know what exactly is the string, instead of just finding CrLf.

## User Update Spotted

- In `RunTextBoxKeyTestCases_MultiParagraph` (moved to `Test/GacUISrc/UnitTest/TestControls_Editor_Key_Paragraph.cpp`), the `UpDown_MovesAcrossLinesAndParagraphs` test case no longer locates `\r\n` dynamically (as described in earlier snippets here). It asserts the exact paragraph text and uses deterministic caret indices instead.

# EXECUTION PLAN

## STEP 1: Extend RunTextBoxKeyTestCases_MultiParagraph with a new Navigation category [DONE]

File to edit:
- `Test/GacUISrc/UnitTest/TestControls_Editor_Key_Paragraph.cpp`

Location:
- In `template<typename TTextBox> void RunTextBoxKeyTestCases_MultiParagraph(...)`.
- Insert a new `TEST_CATEGORY(L"Navigation (Paragraph)")` after the existing `TEST_CATEGORY(L"Clipboard (Paragraph)") { ... }` block, and before the end of `RunTextBoxKeyTestCases_MultiParagraph`.

Insert:

	TEST_CATEGORY(L"Navigation (Paragraph)")
	{
		// New test cases added in STEP 2..6
	}

## STEP 2: Add LEFT/RIGHT boundary navigation across paragraphs [DONE]

Add the following `TEST_CASE` into `TEST_CATEGORY(L"Navigation (Paragraph)")`:

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

## STEP 3: Add UP/DOWN movement across lines inside a paragraph and across paragraph boundaries [DONE]

Add the following `TEST_CASE` into `TEST_CATEGORY(L"Navigation (Paragraph)")`:

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

## STEP 4: Add HOME/END escalation semantics (line → paragraph → document) [DONE]

Add the following `TEST_CASE`s into `TEST_CATEGORY(L"Navigation (Paragraph)")`:

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

## STEP 5: Add CTRL+HOME / CTRL+END document boundary tests [DONE]

Add the following `TEST_CASE` into `TEST_CATEGORY(L"Navigation (Paragraph)")`:

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

## STEP 6: Add PAGE UP / PAGE DOWN tests using marker search (directionality only) [DONE]

Add the following `TEST_CASE` into `TEST_CATEGORY(L"Navigation (Paragraph)")`:

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

# FIXING ATTEMPTS

## Fixing attempt No.1

The build failed with `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp(1578,3): error C1075: '(': no matching token found`.

NOTE: The helper `RunTextBoxKeyTestCases_MultiParagraph` has since been moved to `Test/GacUISrc/UnitTest/TestControls_Editor_Key_Paragraph.cpp`. The same macro-closing fix applies to the moved code.

The root cause is that several newly added `TEST_CASE(...)` blocks under `TEST_CATEGORY(L"Navigation (Paragraph)")` were closed with `}` instead of `});`. In this unit test framework, `TEST_CASE` is a macro that requires `});` to close the macro call, otherwise the opening `(` from the macro expansion is left unmatched and triggers C1075.

I changed the closing braces after each `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` call in that category from `}` to `});`, restoring correct macro pairing. This should unblock compilation because all `TEST_CASE` invocations now have balanced parentheses/braces.

## Fixing attempt No.2

The build still failed, but with errors in `Test/GacUISrc/UnitTest/TestControls_Editor_Key_Paragraph.cpp` like:

- `error C2664: 'vl::vint vl::ObjectString<wchar_t>::IndexOf(T) const': cannot convert argument 1 from 'const wchar_t [3]' to 'T'`

The root cause is that `vl::ObjectString<wchar_t>::IndexOf` only searches for a single `wchar_t` code point, but the test code was calling `IndexOf(L"\r\n")` and `IndexOf(L"^")` using string literals. This makes overload resolution attempt `IndexOf(wchar_t)` and fails to convert a wide string literal array into a single `wchar_t`.

I changed the test to:

- Remove all `IndexOf(L"\r\n")` occurrences by asserting deterministic expected strings containing `\r\n`, and use fixed caret indices derived from these exact expected strings.
- Remove the extra `FindCrlf(...)` helper and avoid searching for CRLF in these test cases.
- Replace all `IndexOf(L"^")` occurrences with `IndexOf(L'^')` (or exact-string assertions where appropriate).

This should fix the build because all `IndexOf(...)` calls now match the available `IndexOf(wchar_t)` API and no longer attempt to pass string literals to a single-character search function.

## Fixing attempt No.3

Unit tests crashed in `GuiDocumentTextBox / Navigation (Paragraph) / End_Escalation_Line_Paragraph_Document` with:

- `Assertion failure: document->paragraphs[1]->GetTextForReading() == L"AB^\r\nCD"`

The root cause is in the unit-test remote rendering caret calculation: `CaretLineLast` removed only a trailing `\n` but not a preceding `\r`. For CRLF newlines this makes `KEY_END` land between `\r` and `\n`, so typing inserts the marker inside the CRLF sequence (e.g. `AB\r^\nCD`) instead of before it (`AB^\r\nCD`).

I fixed `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp` so `CaretLineLast` strips both `\r` and `\n` at line ends, ensuring END moves to the true end-of-line before CRLF. This should make the paragraph navigation tests deterministic and match the intended editor behavior.

# !!!FINISHED!!!

# !!!VERIFIED!!!
