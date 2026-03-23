# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Add EditStyleName(SingleParagraph) unit tests

### What to change

Add unit tests to:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Under the existing `TEST_CATEGORY(L"Styles")`, add:
- `TEST_CATEGORY(L"EditStyleName")`
  - `TEST_CATEGORY(L"SingleParagraph")`
    - one `TEST_CASE` per scenario in TASK No.3.

Insert this inside the existing `TEST_CATEGORY(L"Styles")` scope (between its opening `{` and its closing `});`).

Use the existing helpers already defined at the top of `TestControls_Editor_Styles.cpp`:
- `MakeStyleWithBold(...)` (and other `MakeStyleWith...` helpers if needed)
- `RegisterStyle(textBox, styleName, parentStyleName, properties)`
- `SummarizeName(textBox, begin, end)`

Follow the harness pattern from `TestControls_Editor_RichText.cpp`:
- `TooltipTimer timer;`
- `GacUIUnitTest_SetGuiMainProxy(...)`
- two frames via `protocol->OnNextIdleFrame(L"Init", ...)` and `protocol->OnNextIdleFrame(L"Verify", ...)`
- `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)`

### Why

`GuiDocumentCommonInterface::EditStyleName` is the primary API to assign style *names* to text ranges.
On the model side, the range editing wraps runs with `DocumentStyleApplicationRun` and stores `styleName` directly, so we must validate:
- correct application for a range
- range boundary isolation
- adjacent-range separation
- overwrite behavior with overlaps
- unregistered style name handling (should still be stored and summarized)

### Planned code (structure + key assertions)

Add categories:

```cpp
TEST_CATEGORY(L"Styles")
{
	TEST_CATEGORY(L"EditStyleName")
	{
		TEST_CATEGORY(L"SingleParagraph")
		{
			// ... TEST_CASEs ...
		});
	});
});
```

Each test case uses a stable log identity (keep these paths stable for snapshots/logs):
- `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_RegisteredStyle`
- `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_PartialRange`
- `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_UnregisteredStyle`
- `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_AdjacentRanges`
- `Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_Overwrite`

Common init pattern (per `TEST_CASE`):

```cpp
TooltipTimer timer;
GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
{
	protocol->OnNextIdleFrame(L"Init", [=]()
	{
		auto window = GetApplication()->GetMainWindow();
		auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
		textBox->SetFocused();
		textBox->LoadTextAndClearUndoRedo(L"0123456789");

		// per-test setup + EditStyleName calls
	});

	protocol->OnNextIdleFrame(L"Verify", [=]()
	{
		auto window = GetApplication()->GetMainWindow();
		auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

		// per-test SummarizeName assertions
		window->Hide();
	});
});
GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
	WString::Unmanaged(L"..."),
	WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
	resource_DocumentViewer
);
```

Per-test specifics:

1) Apply a registered style to a range (whole-range baseline)

```cpp
// Init:
RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));

// Verify:
auto summary = SummarizeName(textBox, 0, 10);
TEST_ASSERT(summary);
TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
```

2) Apply a style to a partial range (range boundary isolation)

```cpp
// Init:
RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"MyBold"));

// Verify:
TEST_ASSERT(!SummarizeName(textBox, 0, 2));
auto mid = SummarizeName(textBox, 2, 5);
TEST_ASSERT(mid);
TEST_ASSERT(mid.Value() == WString::Unmanaged(L"MyBold"));
TEST_ASSERT(!SummarizeName(textBox, 5, 10));
```

3) Apply an unregistered style name

```cpp
// Init:
textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"UnregisteredStyle"));

// Verify:
auto summary = SummarizeName(textBox, 2, 5);
TEST_ASSERT(summary);
TEST_ASSERT(summary.Value() == WString::Unmanaged(L"UnregisteredStyle"));
```

4) Apply different styles to adjacent ranges

```cpp
// Init:
RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
textBox->EditStyleName(TextPos(0, 0), TextPos(0, 5), WString::Unmanaged(L"StyleA"));
textBox->EditStyleName(TextPos(0, 5), TextPos(0, 10), WString::Unmanaged(L"StyleB"));

// Verify:
auto a = SummarizeName(textBox, 0, 5);
TEST_ASSERT(a);
TEST_ASSERT(a.Value() == WString::Unmanaged(L"StyleA"));
auto b = SummarizeName(textBox, 5, 10);
TEST_ASSERT(b);
TEST_ASSERT(b.Value() == WString::Unmanaged(L"StyleB"));
TEST_ASSERT(!SummarizeName(textBox, 0, 10));
```

5) Overwrite existing style name

```cpp
// Init:
RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
textBox->EditStyleName(TextPos(0, 3), TextPos(0, 7), WString::Unmanaged(L"StyleB"));

// Verify:
auto s0 = SummarizeName(textBox, 0, 3);
TEST_ASSERT(s0);
TEST_ASSERT(s0.Value() == WString::Unmanaged(L"StyleA"));
auto s1 = SummarizeName(textBox, 3, 7);
TEST_ASSERT(s1);
TEST_ASSERT(s1.Value() == WString::Unmanaged(L"StyleB"));
auto s2 = SummarizeName(textBox, 7, 10);
TEST_ASSERT(s2);
TEST_ASSERT(s2.Value() == WString::Unmanaged(L"StyleA"));
```

Guardrails:
- Keep ranges non-empty (`begin != end`) when calling `SummarizeStyleName`.
- Re-acquire `window` and `textBox` inside each frame callback (don’t persist pointers across frames).

## STEP 2: Verify by building and running UnitTest

### What to do

Build the solution (Debug|x64):

```powershell
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
& REPO-ROOT\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
```

Run unit tests (Debug|x64):

```powershell
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64
```

### Why

Any `*.cpp` change under `Test\GacUISrc\UnitTest` requires running the `UnitTest` project to validate behavior and avoid regressions.

# !!!FINISHED!!!
