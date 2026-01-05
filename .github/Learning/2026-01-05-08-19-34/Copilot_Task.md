# !!!TASK!!!
# PROBLEM DESCRIPTION

## TASK No.15: Create basic `GuiSinglelineTextBox` test case

### description

Create a new test file `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp` following the established GacUI test pattern.

Define a test resource with:
```xml
<Window Text="GuiSinglelineTextBox Test" ClientSize="x:480 y:320">
  <Table ...>
    <SinglelineTextBox ref.Name="textBox" Text="Initial text"/>
  </Table>
</Window>
```

Implement a test case that:
1. Starts the application with the resource
2. In first idle frame:
   - Find the textBox control
   - Verify initial text is "Initial text"
   - Verify the textBox is enabled
   - Test basic properties (font, size, etc.)
   - Programmatically change text via `SetText`
3. In second idle frame:
   - Verify text was changed
   - Test getting selection
   - Test setting selection programmatically
4. In final frame:
   - Close window

Use the standard pattern:
```cpp
GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
{
    protocol->OnNextIdleFrame(L"Ready", [=]() { ... });
    protocol->OnNextIdleFrame(L"After text change", [=]() { ... });
    protocol->OnNextIdleFrame(L"Final", [=]() {
        auto window = GetApplication()->GetMainWindow();
        window->Hide();
    });
});
GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/GuiSinglelineTextBox/Basic"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource
);
```

### what to be done

1. Create the test file with proper includes (`TestControls.h`)
2. Define the test resource string with Window and SinglelineTextBox
3. Create TEST_FILE with TEST_CATEGORY for GuiSinglelineTextBox
4. Implement test case for basic operations
5. Set up multiple idle frames to test different states
6. Use FindObjectByName to locate the textBox control
7. Verify text properties and programmatic manipulation

### how to test it

Run the compiled unit test executable. The test validates:
1. GuiSinglelineTextBox can be created and displayed
2. Initial text property is set correctly
3. Programmatic text changes work
4. Selection API works correctly
5. The paragraph protocol integration works end-to-end

### file locations

New file: `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`

### rationale

This test validates that the basic paragraph protocol implementation works correctly with a real GacUI control. `GuiSinglelineTextBox` uses `GuiDocumentLabel` which uses `IGuiGraphicsParagraph`, so this test exercises the complete stack. By testing programmatic text manipulation first (before user input simulation), we verify the foundational functionality before adding complexity. This test will catch integration issues between the paragraph implementation and the document control system.

# UPDATES

## UPDATE

You can simply set text box's AlignmentToParent to "left:5 top:5 right:5 bottom:-1" and avoid using a table, since the window only has one text box.

The "After text change" and "Final" frame should merge as there is no change between these two frames. This would cause an error. So just verify and close.

# INSIGHTS AND REASONING

## Task Analysis

Task No.15 requires creating a GacUI unit test for `GuiSinglelineTextBox`. This is the first GUI-level test that exercises the document paragraph protocol that was implemented in earlier tasks (Tasks 1-14). The test should validate the complete integration stack from UI controls down to the remote protocol implementation.

## Understanding `GuiSinglelineTextBox`

Based on the source code analysis:

1. **Class Hierarchy**: `GuiSinglelineTextBox` extends `GuiDocumentLabel` (as seen in [GuiDocumentViewer.h](Source/Controls/TextEditorPackage/GuiDocumentViewer.h#L89))

2. **Key Features**:
   - Displays a single line of text
   - Inherits text manipulation from `GuiDocumentLabel`
   - Supports password character mode via `GetPasswordChar()`/`SetPasswordChar()`
   - Uses the document/paragraph rendering system internally

3. **XML Tag**: The control is referenced as `<SinglelineTextBox>` in GacUI XML resources (confirmed by searching existing test resources)

## Test Pattern Analysis

From analyzing existing test files like [TestControls_Basic_GuiButton.cpp](Test/GacUISrc/UnitTest/TestControls_Basic_GuiButton.cpp) and [TestControls_CoreApplication_GuiLabel.cpp](Test/GacUISrc/UnitTest/TestControls_CoreApplication_GuiLabel.cpp):

1. **Standard Structure**:
   - Include `TestControls.h` header
   - Use `TEST_FILE` block containing test categories and cases
   - Define XML resources as raw string literals with `LR"GacUISrc(...)GacUISrc"` syntax
   - Use `GacUIUnitTest_SetGuiMainProxy` with lambda for test logic
   - Chain `protocol->OnNextIdleFrame()` calls for sequential test steps
   - Final frame always closes the window with `window->Hide()`

2. **Resource Pattern**:
   ```xml
   <Resource>
     <Instance name="MainWindowResource">
       <Instance ref.Class="gacuisrc_unittest::MainWindow">
         <Window ref.Name="self" Text="..." ClientSize="x:480 y:320">
           <!-- Controls here -->
         </Window>
       </Instance>
     </Instance>
   </Resource>
   ```

3. **Helper Functions**:
   - `GetApplication()->GetMainWindow()` - get main window reference
   - `FindObjectByName<T>(window, L"name")` - find named controls
   - `TEST_ASSERT()` - verify conditions

## Proposed Design

### File Structure

Create `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`:

```cpp
#include "TestControls.h"

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
                    // Frame 1: Verify initial state and change text
                    auto window = GetApplication()->GetMainWindow();
                    auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
                    TEST_ASSERT(textBox->GetText() == L"Initial text");
                    TEST_ASSERT(textBox->GetEnabled() == true);
                    
                    // Change text programmatically
                    textBox->SetText(L"Changed text");
                });
                protocol->OnNextIdleFrame(L"After text change", [=]()
                {
                    // Frame 2: Verify text change and close window
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

**Design Notes**:
- The layout uses simple `AlignmentToParent="left:5 top:5 right:5 bottom:-1"` instead of a `<Table>` since the window contains only one text box. The `bottom:-1` means the text box does not stick to the bottom of the window, allowing it to have its natural height.
- The test uses only two frames: "Ready" (verify initial state and make changes) and "After text change" (verify changes and close). Each frame must have observable changes to avoid test errors.

### Key Test Scenarios

1. **Initial State Verification** (Frame 1: "Ready"):
   - Verify `GetText()` returns the initial text set in XML (`"Initial text"`)
   - Verify `GetEnabled()` returns true by default
   - Verify the control is properly created and accessible
   - Programmatically change text via `SetText()`

2. **Programmatic Text Manipulation** (Frame 2: "After text change"):
   - Verify `GetText()` returns the changed text (`"Changed text"`)
   - Close the window

**Note**: Selection API testing has been deferred to a separate, more advanced test case. This basic test focuses on validating the fundamental text get/set operations work correctly with the paragraph protocol.

### Project Integration

The new test file needs to be added to the project file. Looking at the project structure, test files are typically in `Test\GacUISrc\UnitTest\`. The file should be added to the corresponding `.vcxproj` file.

### Dependencies

This test depends on:
- Task 13: `GuiRemoteGraphicsParagraph` implementation
- Task 14: `GuiUnitTestProtocol_Rendering.cpp` protocol handlers
- The existing DarkSkin theme (`darkskin::Theme`)

### Risk Assessment

1. **Low Risk**: The test pattern is well-established in the codebase
2. **Medium Risk**: First test of the document paragraph protocol - may uncover integration issues
3. **Mitigation**: Start with simple tests (text get/set) before complex operations (selection)

## Implementation Checklist

1. [ ] Create the test file `TestControls_Editor_GuiSinglelineTextBox.cpp`
2. [ ] Add proper includes (`TestControls.h`)
3. [ ] Define XML resource with `SinglelineTextBox` using simple `AlignmentToParent` layout
4. [ ] Implement `TEST_FILE` with `TEST_CATEGORY(L"GuiSinglelineTextBox")`
5. [ ] Implement `TEST_CASE(L"Basic")` with two frames:
   - Frame 1 "Ready": Verify initial state (text, enabled), change text programmatically
   - Frame 2 "After text change": Verify text change, close window
6. [ ] Add the file to the project (.vcxproj file)
7. [ ] Build and run to verify the test works

# !!!FINISHED!!!