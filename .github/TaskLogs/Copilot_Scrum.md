# !!!SCRUM!!!

# DESIGN REQUEST

Prepare unit test for `TextItemProvider` class.

- Task No.1, prepare empty test file
  - It should name `TestItemProviders_TextItemProvider.cpp`
  - It should be added to `UnitTest` project in `Source Files\ItemProviders` solution explorer folder

- Task No.2, implement test cases
  - The goal is to test, after changing items in TextItemProvider, or changing properties in each item, are callbacks executed as expected
    - `ITextItemProviderCallback`
    - `IItemProviderCallback`
    - You must find a way to record the callback execution orders, and in a test case, do something, and verify the record.

## UPDATE

TestItemProviders.h offers utility for testing item providers. Checkout the sample test case and refresh Task No.2

# TASKS

- [x] TASK No.1: Create Empty Test File and Project Integration
- [ ] TASK No.2: Implement Callback Testing Infrastructure and Test Cases

## TASK No.1: Create Empty Test File and Project Integration

Create the test file `TestItemProviders_TextItemProvider.cpp` with proper basic structure and integrate it into the UnitTest project in the correct solution explorer folder to establish the foundation for TextItemProvider testing.

### what to be done

- Create the file `TestItemProviders_TextItemProvider.cpp` in the directory `c:\Code\VczhLibraries\GacUI\Test\GacUISrc\UnitTest\`
- Add the basic test file structure with proper includes and namespace usage:
  - Include necessary headers: `TestControls.h` for the test framework infrastructure
  - Include `#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITextItemView.h"` for TextItemProvider classes
  - Use appropriate namespaces: `using namespace vl;`, `using namespace vl::unittest;`, `using namespace vl::presentation::controls::list;`
  - Add `TEST_FILE` block with empty structure ready for test categories
- Add the file to the UnitTest.vcxproj project:
  - Update `c:\Code\VczhLibraries\GacUI\Test\GacUISrc\UnitTest\UnitTest.vcxproj` to include `<ClCompile Include="TestItemProviders_TextItemProvider.cpp" />`
- Add the file to the UnitTest.vcxproj.filters project:
  - Update `c:\Code\VczhLibraries\GacUI\Test\GacUISrc\UnitTest\UnitTest.vcxproj.filters` to include the file in the `Source Files\ItemProviders` filter
  - Use the format: `<ClCompile Include="TestItemProviders_TextItemProvider.cpp"><Filter>Source Files\ItemProviders</Filter></ClCompile>`
- Verify the file can be compiled by building the UnitTest project

### how to test it

- Build the UnitTest project to ensure the file compiles without errors
- Verify the file appears in the correct Solution Explorer folder (`Source Files\ItemProviders`)
- Run the unit test executable to ensure no test failures are introduced by the empty test structure

### rationale

This task establishes the proper foundation for TextItemProvider testing by creating the test file with correct structure and project integration. Creating the file in the right location and properly integrating it into both the project file and filters ensures it will be built and appears in the correct location in Visual Studio's Solution Explorer. The basic structure with proper includes and namespaces follows the established pattern seen in other test files like TestControls_Basic_GuiScroll_Tracker.cpp. This foundation task is essential before implementing actual test cases, as it ensures the development environment is properly configured and the file can be compiled successfully. Since there are no existing ItemProvider tests, the ItemProviders filter already exists but is empty, making this the first test file for this category.

## TASK No.2: Implement Comprehensive TextItemProvider Test Cases

Implement comprehensive test cases for TextItemProvider callback functionality using the existing testing utilities in `TestItemProviders.h`. The existing infrastructure provides `MockItemProviderCallback`, `MockTextItemProviderCallback`, and `AssertCallbacks()` helper function for verifying callback execution sequences.

### what to be done

- Expand the existing `TestItemProviders_TextItemProvider.cpp` file with comprehensive test cases using the available testing utilities:
  - Include `#include "TestItemProviders.h"` to access `MockItemProviderCallback`, `MockTextItemProviderCallback`, `AssertCallbacks()`, and `PrintCallbacks()` utilities
  - Use the existing `gacui_unittest_template` namespace which contains the mock classes and helper functions
- Implement test cases following the established pattern from the sample test case:
  - **Item Insertion/Removal Tests**: Test that `OnItemModified` is called correctly when items are added to or removed from the provider using Add(), Insert(), RemoveAt(), Clear() methods
  - **Item Property Change Tests**: Test that both `OnItemModified` and `OnItemCheckedChanged` are called when `TextItem::SetChecked()` changes an item's check state
  - **Text Change Tests**: Test that `OnItemModified` is called when `TextItem::SetText()` changes an item's text content
  - **Multiple Item Operations**: Test batch operations and verify correct callback sequences with proper start/count/newCount parameters
  - **Provider Method Tests**: Test `SetChecked(itemIndex, value)` and `GetChecked(itemIndex)` methods work correctly and trigger appropriate callbacks
  - **Callback Attachment/Detachment**: Test `AttachCallback` and `DetachCallback` methods and verify `OnAttached` is called appropriately
  - **Edge Case Tests**: Test behavior with invalid indices, setting identical values (should not trigger callbacks), and operations on empty providers
- For each test case, follow the established pattern:
  - Create `List<WString> callbackLog` to record callback execution
  - Create `MockTextItemProviderCallback textCallback(callbackLog)` and `MockItemProviderCallback itemCallback(callbackLog)`
  - Create `TextItemProvider` and attach callbacks using constructor parameter and `AttachCallback()`
  - Perform operations on the provider or individual TextItem objects
  - Use `AssertCallbacks(callbackLog, expected)` with string array to verify the callback sequence matches expectations
  - Leverage `PrintCallbacks()` for diagnostic output when tests fail

### how to test it

- Build and run the UnitTest project to execute all TextItemProvider test cases
- Verify all test cases pass and properly validate callback behavior using the `AssertCallbacks()` helper function
- Test that the existing mock classes correctly capture all expected callback invocations with accurate string representations of parameters
- Ensure that callbacks are only triggered when values actually change (no spurious callbacks)
- Validate that multiple attached callbacks all receive proper notifications in the correct sequence
- Use the diagnostic capabilities of `PrintCallbacks()` to troubleshoot any test failures

### rationale

This task leverages the existing robust testing infrastructure in `TestItemProviders.h` rather than reinventing mock callback functionality. The existing `MockItemProviderCallback` and `MockTextItemProviderCallback` classes already provide comprehensive logging of callback invocations with formatted string representations of all parameters. The `AssertCallbacks()` helper function provides clear diagnostic output when tests fail, showing both expected and actual callback sequences. This approach is more efficient and consistent with the existing codebase patterns. The sample test case demonstrates the correct pattern for setting up providers, attaching callbacks, performing operations, and verifying results. By following this established pattern, we ensure consistency with existing test infrastructure and can focus on comprehensive test coverage of TextItemProvider functionality rather than building testing utilities. The string-based callback logging approach makes test assertions readable and debugging straightforward when callback sequences don't match expectations.

# Impact to the Knowledge Base

## GacUI

The current knowledge base lacks specific guidance about testing item providers and callback mechanisms in list controls. The following should be added:

### Testing Item Provider Callbacks

Item providers in GacUI use callback interfaces to notify list controls of data changes and maintain UI consistency:

- Use mock callback classes implementing `IItemProviderCallback` and specific provider callbacks (like `ITextItemProviderCallback`) to test callback execution in unit tests
- Record callback invocations with parameters, sequence information, and call history to verify proper notification behavior and execution order
- Test both successful operations and edge cases (invalid indices, unchanged values, empty providers) to ensure callbacks are only triggered when appropriate
- Item providers typically trigger `OnItemModified` for structural changes (insertions, removals, content updates) and specific callbacks (like `OnItemCheckedChanged`) for property-specific changes
- Multiple callbacks can be attached to a single provider using `AttachCallback()`, and all registered callbacks should receive notifications
- The `itemReferenceUpdated` parameter in `OnItemModified` distinguishes between item replacement (true) and content updates (false), which affects how list controls handle the change
- Mock-based callback testing should verify that list controls receive timely and accurate notifications of all data changes
- TextItemProvider serves as a good example of a complete item provider implementation that supports both general item operations and text-specific functionality

### Unit Testing Item Provider Infrastructure

When testing item provider classes, use the existing infrastructure in `TestItemProviders.h`:

- Use `MockItemProviderCallback` and `MockTextItemProviderCallback` classes that automatically log callback invocations to a `List<WString>`
- Leverage `AssertCallbacks(callbackLog, expected)` helper function to verify callback sequences with clear diagnostic output
- Use `PrintCallbacks()` for debugging when callback sequences don't match expectations
- Follow the established pattern: create callback log, attach mock callbacks, perform operations, verify with AssertCallbacks()
- String-based callback logging provides readable test assertions and clear parameter representation
- Use `TEST_CATEGORY` organization to group related item provider tests logically
- Test attachment and detachment of callbacks to ensure proper lifecycle management and `OnAttached` invocation

# !!!FINISHED!!!
