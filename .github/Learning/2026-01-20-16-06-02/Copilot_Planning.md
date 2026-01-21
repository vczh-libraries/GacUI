# !!!PLANNING!!!

# UPDATES

## UPDATE

You can just merge eventsChanged and eventsExecuted together, this even grands you the opportunitiy to check there execution order.

# EXECUTION PLAN

## STEP 1: Add deterministic helpers for hyperlink interaction tests

Add two small local helpers in `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` (in the existing anonymous namespace near `Summarize(...)`) to keep tests concise and avoid fragile pixel coordinates / ad-hoc event assertions.

1) Caret-to-global-point conversion helper (per task requirement):

- Purpose: turn a `(row, column)` caret position into a `NativePoint` usable by `UnitTestRemoteProtocol::MouseMove` / `LClick`, so we hit-test hyperlinks deterministically.
- Required behavior:
	- Call `textBox->GetCaretBounds(caret, false)` to get the caret rectangle in container coordinates.
	- Convert container origin to global with `protocol->LocationOf(container, 0.0, 0.0, 0, 0)`.
	- Add the center point of `caretBounds`.
	- Add `(x=4, y=0)` to bias into the character cell.

Proposed code:

	static NativePoint CaretToGlobalPoint(UnitTestRemoteProtocol* protocol, GuiDocumentLabel* textBox, TextPos caret)
	{
		auto caretBounds = textBox->GetCaretBounds(caret, false);
		auto container = textBox->GetContainerComposition();
		auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);

		auto x = topLeft.x + caretBounds.x1 + caretBounds.Width() / 2 + 4;
		auto y = topLeft.y + caretBounds.y1 + caretBounds.Height() / 2;
		return { x, y };
	}

2) Event log assertion helper (per UPDATE guidance, reuse `AssertCallbacks` in `Test/GacUISrc/UnitTest/TestItemProviders.h`):

- Purpose: make it easy to verify a merged event log for `ActiveHyperlinkChanged` / `ActiveHyperlinkExecuted`, and then clear the log for the next frame.

Proposed code:

	template<vint Count>
	static void AssertEventLogAndClear(List<WString>& eventLog, const wchar_t* (&expected)[Count])
	{
		AssertCallbacks(eventLog, expected);
		eventLog.Clear();
	}

Why this step is necessary:

- Hyperlink hit-testing is sensitive to layout/theme; caret-derived points keep tests stable and aligned with real behavior.
- Reusing `AssertCallbacks` standardizes event verification and makes failures readable.

## STEP 2: Add a dedicated hyperlink test category (single paragraph model editing)

Add `TEST_CATEGORY(L"Hyperlink")` to `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` (parallel to `SingleParagraph` / `MultiParagraph`) and start with model-editing tests that do not require mouse input.

All cases in this category must use log paths under:

	Controls/Editor/Features/RichText/Hyperlink_<CaseName>

Add a single-paragraph editing-focused test case that covers create/edit/remove and partial removal behavior following current implementation:

Proposed test case: `Hyperlink_SingleParagraph_EditRemove_Model`

- Init:
	- `textBox->LoadTextAndClearUndoRedo(L"0123456789");`
	- Create a hyperlink covering `[2, 8)`:
		- `textBox->EditHyperlink(0, 2, 8, WString::Unmanaged(L"refA"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
- Verify creation:
	- `auto doc = textBox->GetDocument();`
	- Query a position inside the range using `doc->GetHyperlink(0, 3, 3)`:
		- `TEST_ASSERT(package->hyperlinks.Count() > 0);`
		- `TEST_ASSERT(package->hyperlinks[0]->reference == WString::Unmanaged(L"refA"));`
- Verify `EditHyperlink` updates an existing hyperlink in-place:
	- `textBox->EditHyperlink(0, 2, 8, WString::Unmanaged(L"refB"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
	- `TEST_ASSERT(doc->GetHyperlink(0, 4, 4)->hyperlinks[0]->reference == WString::Unmanaged(L"refB"));`
- Verify partial `RemoveHyperlink` behavior matches current implementation (per UPDATE: follow implementation if it removes the whole hyperlink):
	- `textBox->RemoveHyperlink(0, 4, 6);` (strictly inside `[2, 8)`)
	- After removal, the hyperlink should be gone everywhere in the original range:
		- `TEST_ASSERT(doc->GetHyperlink(0, 3, 3)->hyperlinks.Count() == 0);`
		- `TEST_ASSERT(doc->GetHyperlink(0, 7, 7)->hyperlinks.Count() == 0);`

Why this step is necessary:

- It directly validates the editing APIs (`EditHyperlink` / `RemoveHyperlink`) without conflating failures with hit-testing or event dispatch.
- It codifies the “partial remove removes whole hyperlink” behavior so future changes are intentional.

## STEP 3: Add single-paragraph hyperlink interaction tests (hover/leave/click)

Add a single-paragraph interaction test that validates:

- Hover activates a hyperlink (`ActiveHyperlinkChanged` fires and `GetActiveHyperlinkReference()` matches).
- Leaving clears the active hyperlink.
- Clicking executes the hyperlink (`ActiveHyperlinkExecuted` fires).

Proposed test case: `Hyperlink_SingleParagraph_ActivateExecute`

Key implementation details:

- In the outer `GacUIUnitTest_SetGuiMainProxy` lambda, create state shared across frames:

		List<WString> events;
		NativePoint pointInsideLink = { 0, 0 };

- In `Init` frame:
	- `textBox->LoadTextAndClearUndoRedo(L"0123456789");`
	- `textBox->EditHyperlink(0, 2, 5, WString::Unmanaged(L"refA"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
	- Set mode explicitly (interaction is implemented in `GuiDocumentEditMode::ViewOnly`):
		- `textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);`
	- Attach events:

		textBox->ActiveHyperlinkChanged.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
		{
			events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
		});

		textBox->ActiveHyperlinkExecuted.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
		{
			events.Add(L"Executed " + textBox->GetActiveHyperlinkReference());
		});

	- Precompute a point inside the hyperlink from caret `(row=0, column=3)`:

		pointInsideLink = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));

- In `Hover` frame:
	- `protocol->MouseMove(pointInsideLink);`
	- Assertions:
		- `TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));`
		- `const wchar_t* expected[] = { L"Activated refA" }; AssertEventLogAndClear(events, expected);`

- In `Click` frame:
	- `protocol->LClick(pointInsideLink);`
	- Assertions:
		- `const wchar_t* expected[] = { L"Executed refA" }; AssertEventLogAndClear(events, expected);`

- In `Leave` frame:
	- `protocol->MouseMove({ 0, 0 });`
	- Assertions:
		- `TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));`
		- `const wchar_t* expected[] = { L"Activated " }; AssertEventLogAndClear(events, expected);`

Why this step is necessary:

- It exercises the full interaction pipeline: hit-testing -> active state -> events -> `GetActiveHyperlinkReference`.
- It validates that activation state is cleared correctly (no stale references).

## STEP 4: Add multi-paragraph hyperlink editing + interaction coverage

Add a multi-paragraph test that validates activation/execution works independently per paragraph and that paragraph indices in editing APIs map correctly to interaction behavior.

Proposed test case: `Hyperlink_MultiParagraph_ActivateExecute`

- Init:
	- Load multiple paragraphs with short lines (avoid wrapping):
		- `textBox->LoadTextAndClearUndoRedo(L"012345\n6789AB\nCDEF");`
	- Add hyperlinks in different paragraphs:
	- `textBox->EditHyperlink(0, 1, 3, WString::Unmanaged(L"refP0"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
	- `textBox->EditHyperlink(1, 2, 5, WString::Unmanaged(L"refP1"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
	- `textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);`
	- Attach the same merged `events` log pattern from STEP 3.
	- Precompute two points inside hyperlinks:
		- `p0 = CaretToGlobalPoint(protocol, textBox, TextPos(0, 2));`
		- `p1 = CaretToGlobalPoint(protocol, textBox, TextPos(1, 3));`

- Hover/click for paragraph 0:
	- `protocol->MouseMove(p0);` => active `refP0`
	- `protocol->LClick(p0);` => executed `refP0`

- Hover/click for paragraph 1:
	- `protocol->MouseMove(p1);` => active `refP1`
	- `protocol->LClick(p1);` => executed `refP1`

- Leave:
	- `protocol->MouseMove({ 0, 0 });` => active reference becomes empty.

Why this step is necessary:

- It ensures row/paragraph indexing in `EditHyperlink` is consistent with caret layout and hit-testing across paragraphs.
- It catches regressions where active hyperlink state “sticks” across paragraphs.

## STEP 5: Add an overlap/robustness scenario and avoid fragile style assumptions

Add one compact robustness test focused on overlapping edit/remove and “no stale active reference” behavior, while following the requirement to avoid assuming `Nullable<>` style fields are always empty.

Proposed test case: `Hyperlink_Overlap_Remove_DoesNotLeaveStaleActive`

- Init:
	- `textBox->LoadTextAndClearUndoRedo(L"0123456789");`
	- Create a non-trivial hyperlink structure:
		- `textBox->EditHyperlink(0, 2, 7, WString::Unmanaged(L"refA"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
		- `textBox->EditHyperlink(0, 5, 9, WString::Unmanaged(L"refB"), WString::Unmanaged(L"#NormalLink"), WString::Unmanaged(L"#ActiveLink"));`
	- Capture a baseline style summary outside the edited range (do not assume nulls):
		- `auto baselineOutside = Summarize(textBox, 0, 0, 2);`
	- `textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);`
	- Attach the same merged `events` log pattern from STEP 3 and compute points for caret `(0, 3)` and `(0, 8)`:
		- `pA = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));`
		- `pB = CaretToGlobalPoint(protocol, textBox, TextPos(0, 8));`

- Interaction + removal:
	- Hover `pA` and verify active reference is `refA`.
	- Hover `pB` and verify active reference is `refB`.
	- Remove a range overlapping both (e.g. `textBox->RemoveHyperlink(0, 6, 7);`) and then:
		- `protocol->MouseMove(pA);` and `protocol->MouseMove(pB);`
		- Verify `GetActiveHyperlinkReference()` is either the new correct reference or empty, but never a removed/stale reference.
	- Leave (`{ 0, 0 }`) and verify it becomes empty.

- Outside-range invariant check (baseline compare):
	- Re-summarize the same outside range and compare against baseline field-by-field:

		auto afterOutside = Summarize(textBox, 0, 0, 2);
		TEST_ASSERT(baselineOutside->face == afterOutside->face);
		TEST_ASSERT(baselineOutside->size == afterOutside->size);
		TEST_ASSERT(baselineOutside->color == afterOutside->color);
		TEST_ASSERT(baselineOutside->backgroundColor == afterOutside->backgroundColor);
		TEST_ASSERT(baselineOutside->bold == afterOutside->bold);
		TEST_ASSERT(baselineOutside->italic == afterOutside->italic);
		TEST_ASSERT(baselineOutside->underline == afterOutside->underline);
		TEST_ASSERT(baselineOutside->strikeline == afterOutside->strikeline);
		TEST_ASSERT(baselineOutside->antialias == afterOutside->antialias);
		TEST_ASSERT(baselineOutside->verticalAntialias == afterOutside->verticalAntialias);

Why this step is necessary:

- Overlapping edit/remove is where stale active state bugs typically appear; this test makes regressions obvious.
- Baseline comparison avoids brittle assumptions about theme-default style resolution producing `Nullable<> == null`.

# TEST PLAN

- Run the new `TestControls_Editor_RichText.cpp` cases under:
	- `Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_EditRemove_Model`
	- `Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_ActivateExecute`
	- `Controls/Editor/Features/RichText/Hyperlink_MultiParagraph_ActivateExecute`
	- `Controls/Editor/Features/RichText/Hyperlink_Overlap_Remove_DoesNotLeaveStaleActive`
- Verify each case asserts both state (`GetDocument()->GetHyperlink(...)` or `GetActiveHyperlinkReference()`) and events (`ActiveHyperlinkChanged` / `ActiveHyperlinkExecuted`) using a merged `List<WString>` + `AssertCallbacks` pattern.
- If any case is flaky, adjust only caret positions (via `CaretToGlobalPoint`) and/or text to avoid wrapping; do not introduce fixed pixel coordinates.

# !!!FINISHED!!!
