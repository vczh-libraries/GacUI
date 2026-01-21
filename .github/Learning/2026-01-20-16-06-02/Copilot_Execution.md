# !!!EXECUTION!!!

# UPDATES

## UPDATE (2026-01-20)

- Unit tests crashed at `Hyperlink_SingleParagraph_ActivateExecute` with: "The last frame didn't trigger UI updating".
- Fix attempt: in `TestControls_Editor_RichText.cpp`, make the click frame also move the mouse outside (deactivating the hyperlink) so the frame produces a visible UI update; adjust expected event log accordingly.
- Unit tests crashed at `Hyperlink_MultiParagraph_ActivateExecute` with: "The last frame didn't trigger UI updating".
- Fix attempt: in `TestControls_Editor_RichText.cpp`, after clicking a hyperlink, also move the mouse outside to force `ActiveHyperlinkChanged` and a real UI update; update expected event log, and make the final frame only hide the window.

## UPDATE

You don't have to always do the Init/Verify frame organization, you should make a new frame whenever a visible update to the text box is made, so the logging makes sense

## UPDATE

1. The name of the frame summarize what the previous frame did
2. Frame like "Verify Created" should not exist alone as it doesn't make any UI visible changing, which will crash the test case framework

# EXECUTION PLAN

## STEP 1: Add deterministic helpers for hyperlink interaction tests [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Add `AssertCallbacks` support:

```cpp
#include "TestControls.h"
#include "TestItemProviders.h"
```

In the existing `namespace { ... }` helper area (near `Summarize(...)`), add:

```cpp
static NativePoint CaretToGlobalPoint(UnitTestRemoteProtocol* protocol, GuiDocumentLabel* textBox, TextPos caret)
{
	auto caretBounds = textBox->GetCaretBounds(caret, false);
	auto container = textBox->GetContainerComposition();
	auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);

	auto x = topLeft.x + caretBounds.x1 + caretBounds.Width() / 2 + 4;
	auto y = topLeft.y + caretBounds.y1 + caretBounds.Height() / 2;
	return { x, y };
}

template<vint Count>
static void AssertEventLogAndClear(List<WString>& eventLog, const wchar_t* (&expected)[Count])
{
	AssertCallbacks(eventLog, expected);
	eventLog.Clear();
}
```

## STEP 2: Add a dedicated hyperlink test category (single paragraph model editing) [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Insert a new `TEST_CATEGORY(L"Hyperlink")` at the same level as existing `SingleParagraph` / `MultiParagraph` categories (e.g. after `TEST_CATEGORY(L"SingleParagraph")` and before `TEST_CATEGORY(L"MultiParagraph")`).

Add the following test case under `TEST_CATEGORY(L"Hyperlink")`:

```cpp
TEST_CASE(L"SingleParagraph_EditRemove_Model")
{
	TooltipTimer timer;
	GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
	{
		protocol->OnNextIdleFrame(L"Load", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			textBox->SetFocused();
			textBox->LoadTextAndClearUndoRedo(L"0123456789");
		});

		protocol->OnNextIdleFrame(L"Loaded", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				0, 2, 8,
				WString::Unmanaged(L"refA"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));
		});

		protocol->OnNextIdleFrame(L"Created Hyperlink", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			auto doc = textBox->GetDocument();

			auto package = doc->GetHyperlink(0, 3, 3);
			TEST_ASSERT(package->hyperlinks.Count() > 0);
			TEST_ASSERT(package->hyperlinks[0]->reference == WString::Unmanaged(L"refA"));


			TEST_ASSERT(textBox->EditHyperlink(
				0, 2, 8,
				WString::Unmanaged(L"refB"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));
		});

		protocol->OnNextIdleFrame(L"Edited Hyperlink", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			auto doc = textBox->GetDocument();

			auto package = doc->GetHyperlink(0, 4, 4);
			TEST_ASSERT(package->hyperlinks.Count() > 0);
			TEST_ASSERT(package->hyperlinks[0]->reference == WString::Unmanaged(L"refB"));

			TEST_ASSERT(textBox->RemoveHyperlink(0, 4, 6));
		});

		protocol->OnNextIdleFrame(L"Removed Hyperlink", [=]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			auto doc = textBox->GetDocument();

			TEST_ASSERT(doc->GetHyperlink(0, 3, 3)->hyperlinks.Count() == 0);
			TEST_ASSERT(doc->GetHyperlink(0, 7, 7)->hyperlinks.Count() == 0);

			window->Hide();
		});
	});
	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_EditRemove_Model"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resource_DocumentLabel
	);
});
```

## STEP 3: Add single-paragraph hyperlink interaction tests (hover/leave/click) [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Add the following test case under `TEST_CATEGORY(L"Hyperlink")`:

```cpp
TEST_CASE(L"SingleParagraph_ActivateExecute")
{
	List<WString> events;
	NativePoint inside = { 0,0 };
	NativePoint outside = { 0,0 };

	TooltipTimer timer;
	GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
	{
		protocol->OnNextIdleFrame(L"Load", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			textBox->SetFocused();
			textBox->LoadTextAndClearUndoRedo(L"0123456789");
		});

		protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				0, 2, 5,
				WString::Unmanaged(L"refA"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));

			textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);

			textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
			{
				events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
			});
			textBox->ActiveHyperlinkExecuted.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
			{
				events.Add(L"Executed " + textBox->GetActiveHyperlinkReference());
			});

			inside = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));
			{
				auto container = textBox->GetContainerComposition();
				auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
				outside = { topLeft.x - 10, topLeft.y - 10 };
			}
		});

		protocol->OnNextIdleFrame(L"Created Hyperlink", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(inside);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
			const wchar_t* expected[] = { L"Activated refA" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered Hyperlink", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->LClick(inside);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
			const wchar_t* expected[] = { L"Executed refA" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Clicked Hyperlink", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(outside);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
			const wchar_t* expected[] = { L"Activated " };
			AssertEventLogAndClear(events, expected);

			window->Hide();
		});
	});
	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_ActivateExecute"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resource_DocumentLabel
	);
});
```

## STEP 4: Add multi-paragraph hyperlink editing + interaction coverage [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Add the following test case under `TEST_CATEGORY(L"Hyperlink")`:

```cpp
TEST_CASE(L"MultiParagraph_ActivateExecute")
{
	List<WString> events;
	NativePoint insideP0 = { 0,0 };
	NativePoint insideP1 = { 0,0 };
	NativePoint outside = { 0,0 };

	TooltipTimer timer;
	GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
	{
		protocol->OnNextIdleFrame(L"Load", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			textBox->SetFocused();
			textBox->LoadTextAndClearUndoRedo(L"012345\r\n\r\n6789AB\r\n\r\nCDEF");
		});

		protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				0, 1, 3,
				WString::Unmanaged(L"refP0"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));
		});

		protocol->OnNextIdleFrame(L"Created P0", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				1, 2, 5,
				WString::Unmanaged(L"refP1"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));

			textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);

			textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
			{
				events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
			});
			textBox->ActiveHyperlinkExecuted.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
			{
				events.Add(L"Executed " + textBox->GetActiveHyperlinkReference());
			});

			insideP0 = CaretToGlobalPoint(protocol, textBox, TextPos(0, 2));
			insideP1 = CaretToGlobalPoint(protocol, textBox, TextPos(1, 3));
			{
				auto container = textBox->GetContainerComposition();
				auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
				outside = { topLeft.x - 10, topLeft.y - 10 };
			}
		});

		protocol->OnNextIdleFrame(L"Created P1", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(insideP0);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP0"));
			const wchar_t* expected[] = { L"Activated refP0" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered P0", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->LClick(insideP0);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP0"));
			const wchar_t* expected[] = { L"Executed refP0" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Clicked P0", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(insideP1);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP1"));
			const wchar_t* expected[] = { L"Activated refP1" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered P1", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->LClick(insideP1);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP1"));
			const wchar_t* expected[] = { L"Executed refP1" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Clicked P1", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(outside);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
			const wchar_t* expected[] = { L"Activated " };
			AssertEventLogAndClear(events, expected);

			window->Hide();
		});
	});
	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_MultiParagraph_ActivateExecute"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resource_DocumentLabel
	);
});
```

## STEP 5: Add an overlap/robustness scenario and avoid fragile style assumptions [DONE]

File: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

Add the following test case under `TEST_CATEGORY(L"Hyperlink")`:

```cpp
TEST_CASE(L"Overlap_Remove_DoesNotLeaveStaleActive")
{
	List<WString> events;
	NativePoint insideA = { 0,0 };
	NativePoint insideB = { 0,0 };
	NativePoint outside = { 0,0 };
	Ptr<DocumentStyleProperties> baselineOutside;

	TooltipTimer timer;
	GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
	{
		protocol->OnNextIdleFrame(L"Load", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			textBox->SetFocused();
			textBox->LoadTextAndClearUndoRedo(L"0123456789");
		});

		protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				0, 2, 7,
				WString::Unmanaged(L"refA"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));
		});

		protocol->OnNextIdleFrame(L"Created A", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->EditHyperlink(
				0, 5, 9,
				WString::Unmanaged(L"refB"),
				WString::Unmanaged(L"#NormalLink"),
				WString::Unmanaged(L"#ActiveLink")
			));

			baselineOutside = Summarize(textBox, 0, 0, 2);

			textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);
			textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
			{
				events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
			});

			insideA = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));
			insideB = CaretToGlobalPoint(protocol, textBox, TextPos(0, 8));
			{
				auto container = textBox->GetContainerComposition();
				auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
				outside = { topLeft.x - 10, topLeft.y - 10 };
			}
		});

		protocol->OnNextIdleFrame(L"Created B", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(insideA);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
			const wchar_t* expected[] = { L"Activated refA" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered A", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(insideB);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refB"));
			const wchar_t* expected[] = { L"Activated refB" };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered B", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			TEST_ASSERT(textBox->RemoveHyperlink(0, 6, 7));
			events.Clear();
		});

		protocol->OnNextIdleFrame(L"Removed Overlap", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			auto doc = textBox->GetDocument();

			protocol->MouseMove(insideA);
			auto package = doc->GetHyperlink(0, 3, 3);
			auto expectedRef = package->hyperlinks.Count() == 0 ? WString::Unmanaged(L"") : package->hyperlinks[0]->reference;
			WString expectedEvent = L"Activated " + expectedRef;
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == expectedRef);
			const wchar_t* expected[] = { expectedEvent.Buffer() };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered A After Remove", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
			auto doc = textBox->GetDocument();

			protocol->MouseMove(insideB);
			auto package = doc->GetHyperlink(0, 8, 8);
			auto expectedRef = package->hyperlinks.Count() == 0 ? WString::Unmanaged(L"") : package->hyperlinks[0]->reference;
			WString expectedEvent = L"Activated " + expectedRef;
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == expectedRef);
			const wchar_t* expected[] = { expectedEvent.Buffer() };
			AssertEventLogAndClear(events, expected);
		});

		protocol->OnNextIdleFrame(L"Hovered B After Remove", [&, protocol]()
		{
			auto window = GetApplication()->GetMainWindow();
			auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

			protocol->MouseMove(outside);
			TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
			const wchar_t* expected[] = { L"Activated " };
			AssertEventLogAndClear(events, expected);

			{
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
			}

			window->Hide();
		});
	});
	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_Overlap_Remove_DoesNotLeaveStaleActive"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resource_DocumentLabel
	);
});
```

# FIXING ATTEMPTS

## Fixing attempt No.1

### Why the original change did not work

New hyperlink tests were written assuming `GuiDocumentLabel::EditHyperlink` / `RemoveHyperlink` return `bool`. In current code, these APIs are `void` (they apply the edit and refresh the view), so wrapping them in `TEST_ASSERT(...)` expands to `if(!(void_expr)) ...` and fails to compile.

### What I changed

In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`, I removed `TEST_ASSERT(...)` wrappers around `textBox->EditHyperlink(...)` and `textBox->RemoveHyperlink(...)`, keeping the calls and the subsequent document-state assertions.

### Why it should solve the build break

This eliminates `operator !` and boolean-conversion on `void` expressions, fixing the C2171/C2451 compile errors while preserving the test flow (document verification remains).

## Fixing attempt No.2

### What failed

`Hyperlink::SingleParagraph_ActivateExecute` failed at the click frame: the event log contained both `Activated refA` and `Executed refA`, but the test expected only `Executed refA`.

### Root cause

`GuiDocumentCommonInterface::SetActiveHyperlink` always fires `ActiveHyperlinkChanged`, even when the active hyperlink stays the same. Clicking a hyperlink in `ViewOnly` mode triggers `OnMouseDown`, which calls `SetActiveHyperlink(...)` again for the already-active hyperlink, causing a redundant `ActiveHyperlinkChanged` ("Activated") right before `ActiveHyperlinkExecuted` ("Executed").

### What I changed

In `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`, I normalized the incoming hyperlink package and returned early when the new hyperlink is identical to the current one (using `CompareEnumerable` on the hyperlink run lists), so `ActiveHyperlinkChanged` only fires when the active hyperlink actually changes.

### Why it should fix the test

Hover still activates the hyperlink (one `ActiveHyperlinkChanged`), but clicking it no longer re-triggers activation if the active hyperlink didn't change; the click only produces `ActiveHyperlinkExecuted`.

## Fixing attempt No.3

### What failed

After the logic fix above, the unit test process crashed with:

> The last frame didn't trigger UI updating...

In `Hyperlink::SingleParagraph_ActivateExecute`, the frame after clicking was only doing verification work, so the framework considered it an invalid frame.

### Root cause

GacUI UI unit tests require each frame to introduce a UI-visible change, otherwise the snapshot/logging pipeline crashes.

### What I changed

In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`:

- Moved `protocol->MouseMove(outside);` into the click frame so the click frame always introduces a visible state change (active hyperlink deactivates).
- Updated expected events for that frame to include the deactivation event (`Activated `) caused by moving the mouse out.
- Kept the last frame as verification + `window->Hide();` only.

### Why it should fix the crash

Each frame now has a meaningful UI update: hover activates, click executes then moves out (deactivates), last frame hides the window.

## Fixing attempt No.4

### What failed

The same "frame didn't trigger UI updating" crash happened in `Hyperlink::MultiParagraph_ActivateExecute` (and related hyperlink interaction steps).

### Root cause

Two issues combined:

1. Frames that only asserted state without a UI-changing input.
2. The "outside" mouse location was sometimes outside the window (`topLeft - 10`), so the mouse move could be ignored and fail to cause a UI update.

### What I changed

In `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`:

- Adjusted the "outside" point from `topLeft - 10` to `topLeft - 3` so the cursor stays inside the window but outside the document label, ensuring the move is processed.
- For each click frame, moved the cursor to the outside point in the same frame and updated expected events accordingly.
- Ensured the last frame only hides the window.

### Why it should fix the crash

All frames now reliably trigger a visible hyperlink state change and the outside move is guaranteed to be within the window.

# !!!FINISHED!!!

# !!!VERIFIED!!!
