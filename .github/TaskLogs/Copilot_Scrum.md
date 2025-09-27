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

## TASK No.2: Implement Callback Testing Infrastructure and Test Cases

Implement comprehensive test cases for TextItemProvider callback functionality by creating mock callback classes to record callback invocations and testing various scenarios of item manipulation to verify that both ITextItemProviderCallback and IItemProviderCallback are executed in the correct order with proper parameters.

### what to be done

- Create mock callback classes to record callback execution:
  - `MockTextItemProviderCallback` implementing `ITextItemProviderCallback` with a record of `OnItemCheckedChanged` calls storing item index and call sequence
  - `MockItemProviderCallback` implementing `IItemProviderCallback` with records of `OnAttached` and `OnItemModified` calls storing all parameters (start, count, newCount, itemReferenceUpdated) and call sequence
  - Each mock should maintain a call history with sequence numbers to verify execution order
  - Provide methods to clear the history and access call records for verification
- Implement test cases in a `TEST_CATEGORY(L"TextItemProvider")`:
  - **Item Insertion/Removal Tests**: Test that `OnItemModified` is called correctly when items are added to or removed from the provider using Add(), Insert(), RemoveAt() methods
  - **Item Property Change Tests**: Test that both `OnItemModified` and `OnItemCheckedChanged` are called when `TextItem::SetChecked()` changes an item's check state
  - **Text Change Tests**: Test that `OnItemModified` is called when `TextItem::SetText()` changes an item's text content
  - **Multiple Callback Registration**: Test that multiple `IItemProviderCallback` objects can be registered via `AttachCallback()` and all receive notifications
  - **Callback Attachment/Detachment**: Test `AttachCallback` and `DetachCallback` methods work correctly and `OnAttached` is called appropriately
  - **Provider Method Tests**: Test `SetChecked(itemIndex, value)` and `GetChecked(itemIndex)` methods work correctly and trigger appropriate callbacks
  - **Edge Case Tests**: Test behavior with invalid indices, setting identical values (should not trigger callbacks), and operations on empty providers
- For each test case:
  - Create a `TextItemProvider` with the mock callbacks attached
  - Perform operations on the provider or individual TextItem objects
  - Verify the callback history matches the expected sequence and parameters using `TEST_ASSERT`
  - Validate that callback parameters are correct (proper indices, counts, and flags)
  - Ensure callbacks are invoked in the expected order when multiple callbacks should be triggered

### how to test it

- Build and run the UnitTest project to execute all TextItemProvider test cases
- Verify all test cases pass and properly validate callback behavior
- Test that mock classes correctly capture all expected callback invocations with accurate parameters
- Ensure that callbacks are only triggered when values actually change (no spurious callbacks)
- Validate that multiple attached callbacks all receive proper notifications

### rationale

This task addresses the core requirement of testing callback execution order and parameters for TextItemProvider operations. The TextItemProvider class inherits from ListProvider<Ptr<TextItem>> and implements ITextItemView, making it a central component in the item provider system that requires thorough callback testing. By creating dedicated mock callback classes, we can precisely track when callbacks are invoked and verify they happen at the right times with correct parameters. The test cases cover the main scenarios where callbacks should be triggered: item modifications through the provider interface, property changes on individual TextItem objects, and provider-level operations. Testing both ITextItemProviderCallback (specific to text items) and IItemProviderCallback (general item provider interface) ensures complete coverage of the callback system. The mock-based approach allows for detailed verification of callback sequences and parameters, which is essential for validating the contract between TextItemProvider and its consumers (like GuiVirtualTextList). This comprehensive testing approach follows the unit testing framework patterns shown in the knowledge base and ensures the reliability of the callback mechanism that list controls depend on for maintaining UI consistency.

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

When testing item provider classes, establish proper test infrastructure following these patterns:

- Create separate mock classes for each callback interface to isolate testing concerns
- Maintain call history with sequence numbers to verify callback execution order across multiple callback types
- Use `TEST_CATEGORY` organization to group related item provider tests logically
- Include tests for provider methods, individual item property changes, and multi-callback scenarios
- Verify that callbacks contain correct parameters matching the actual data changes performed
- Test attachment and detachment of callbacks to ensure proper lifecycle management

# !!!FINISHED!!!
