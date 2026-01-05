# !!!PLANNING!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Create the Test File

Create a new test file at `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`.

**File Content:**

```cpp
#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox" Text="Initial text">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		TEST_CASE(L"Basic")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Initial text");
					TEST_ASSERT(textBox->GetEnabled() == true);
					textBox->SetText(L"Changed text");
				});
				protocol->OnNextIdleFrame(L"After text change", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Changed text");
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/GuiSinglelineTextBox/Basic"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}
```

**Why this is necessary:**
- This is the primary deliverable of Task No.15: creating a basic test case for `GuiSinglelineTextBox`.
- The test validates the document paragraph protocol implementation that was completed in Tasks 1-14.
- The layout uses `AlignmentToParent="left:5 top:5 right:5 bottom:-1"` instead of a `<Table>` since the window only contains one text box. The `bottom:-1` allows natural height.
- Two frames are used: "Ready" (verify initial state and make changes) and "After text change" (verify changes and close). Each frame must have observable changes to avoid test errors.

## STEP 2: Add the Test File to the Project

Add the new file to `Test\GacUISrc\UnitTest\UnitTest.vcxproj` in the `<ItemGroup>` containing `<ClCompile>` elements.

**Code to add (after the existing test file entries, maintaining alphabetical order within the Editor category):**

```xml
    <ClCompile Include="TestControls_Editor_GuiSinglelineTextBox.cpp" />
```

This line should be added after other `TestControls_*` entries to maintain project organization.

**Why this is necessary:**
- The test file must be included in the Visual Studio project for it to be compiled with the unit test executable.

## STEP 3: Create the Solution Explorer Filter and Add the File

First, add a new filter definition for the "Editor" category to `Test\GacUISrc\UnitTest\UnitTest.vcxproj.filters`. This filter needs to be added in the `<ItemGroup>` containing `<Filter>` elements.

**Add the filter definition (after `Source Files\Controls\Ribbon` filter entry):**

```xml
    <Filter Include="Source Files\Controls\Editor">
      <UniqueIdentifier>{NEW-GUID-FOR-EDITOR}</UniqueIdentifier>
    </Filter>
```

Note: A new GUID should be generated for `{NEW-GUID-FOR-EDITOR}`. Use a format like `{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}`.

Then, add the file mapping in the `<ItemGroup>` containing `<ClCompile>` elements:

```xml
    <ClCompile Include="TestControls_Editor_GuiSinglelineTextBox.cpp">
      <Filter>Source Files\Controls\Editor</Filter>
    </ClCompile>
```

**Why this is necessary:**
- The filter file organizes how files appear in Visual Studio's Solution Explorer.
- Since this is the first "Editor" category test file, a new filter must be created.
- This follows the existing pattern where tests are organized by category (Basic, CoreApplication, List, Toolstrip, Ribbon, etc.).

# TEST PLAN

## Test Scenarios Covered by the New Test Case

The `TEST_CASE(L"Basic")` covers the following scenarios:

1. **Control Instantiation**: Verify that `GuiSinglelineTextBox` can be created via GacUI XML and properly displays in the window.

2. **Initial Text Property**: Verify that `GetText()` returns the text set in the XML resource (`"Initial text"`).

3. **Default Enabled State**: Verify that `GetEnabled()` returns `true` by default.

4. **Programmatic Text Manipulation**: Verify that calling `SetText()` successfully changes the text and `GetText()` reflects the change in the next frame.

5. **End-to-End Protocol Integration**: The test implicitly validates that the paragraph protocol implementation (from Tasks 1-14) works correctly through the complete stack: `GuiSinglelineTextBox` → `GuiDocumentLabel` → `IGuiGraphicsParagraph` → remote protocol.

## What This Test Does NOT Cover (Deferred to Future Tests)

- Selection API (`GetSelection()`, `SetSelection()`)
- User input simulation (typing, mouse selection)
- Password character mode (`GetPasswordChar()`, `SetPasswordChar()`)
- Text events and callbacks
- Focus behavior
- Read-only mode

These are intentionally deferred to separate, more advanced test cases per the task description.

## Existing Tests - No Changes Required

No existing test cases need to be modified. This is a new test file that adds coverage for `GuiSinglelineTextBox`.

## Build Verification

After implementing the changes:
1. Build the unit test project using the "Build Unit Tests" task.
2. Run the unit tests using the "Run Unit Tests" task.
3. Verify that the new test `Controls/Editor/GuiSinglelineTextBox/Basic` passes.

# !!!FINISHED!!!
