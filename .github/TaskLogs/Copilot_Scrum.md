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

## UPDATE

I think the first 2 task can be merged, and that task should also update reflection of GuiDocumentElement

## UPDATE

InvokeOnElementStateChanged is not needed, please document this so that you don't forget in the next phrase
CLASS_MEMBER_PROPERTY_GET/SET does not exist. please read the knowledge base and find out what to do, and find out the file that implementing GuiDocumentElement reflection and checkout out all existing members

## UPDATE

For task 2, you need to take a step further, think about:

- How to define initial values of the range
- NotifyParagraphUpdated will be called multiple times between two Render, it indicates that document has been changed. Does it affect the range?
- How to actually release a IGuiGraphicsParagraph? I would like to keep all statistics data remain available, since they are important in locating between caret/paragraph/pixel position

## UPDATE

For the range expression I have an idea:
- previousRenderBegin, default -1
- previousRenderCount, default 0
if "begin" is -1 it means these two value is invalid.

documentUpdatedSinceLastRender is not a good idea, hasValidPreviousRange can just be removed because we can check if begin is -1.

When NotifyParagraphUpdated, it means (index, oldCount) is updated to (index, newCount).
When updated text is true, they will be deleted anyway. You will have to check the overlapped between (begin, count), (index, oldCount) and (index, newCount).
When updated text is false, paragraphs will be kept anyway, nothing needs to do.

The NotifyParagraphUpdated means part of consecutive paragraphs are changed. Paragraphs before index and paragraphs after index+oldCount-1 are not touched, but the middle part may be changed and the number of paragraphs may be changed too.
I would like you to track (begin, count) with (index, oldCount, newCount), so that the (begin, count) still mean those paragraphs that are previously rendered. Both begin and count may be changed.

For example, if begin is after the updated range, begin will be changed according to the difference of newCount and oldCount.
If (begin,count) is before index, leave it.
If they have overlapped...you need to now find the reasonable way for me

## UPDATE

Actually when updatedText is false, nothing needs to be done, and newCount and oldCOunt will be guaranteed equal.
We only need to care about when updatedText is true.

Since we don't know which paragraphs are replaced or what, when updatedText is true and the two range overlapps, we can extend (begin,count) to its union with (index, oldCount) first, and then extend the range according to newCount, by count+=newCount-oldCount.

## UPDATE

No test is needed as unit test does not cover this control yet.

# TASKS

- [x] TASK No.1: Add ParagraphRecycle Property to GuiDocumentElement and Connect to Configuration System
- [x] TASK No.2: Implement Paragraph Recycling Logic in GuiDocumentElementRenderer::Render

## TASK No.1: Add ParagraphRecycle Property to GuiDocumentElement and Connect to Configuration System

Add a ParagraphRecycle property to the GuiDocumentElement class, positioned right after the PasswordChar property, and establish the complete connection from GuiDocumentConfig through GuiDocumentCommonInterface to enable document controls to control paragraph recycling behavior. This task also includes updating the reflection system to expose the new property.

### what to be done

- Add a `bool paragraphRecycle` member variable to GuiDocumentElement in the protected section, positioned after `passwordChar`
- Initialize `paragraphRecycle` to `false` in the GuiDocumentElement constructor
- Add `GetParagraphRecycle()` public method that returns the current value
- Add `SetParagraphRecycle(bool value)` public method that updates the value
- Update the header file with appropriate documentation comments following the existing pattern
- NOTE: InvokeOnElementStateChanged is NOT needed when the property changes
- Locate where GuiDocumentCommonInterface initializes or updates the documentElement
- Add code to set the ParagraphRecycle property on documentElement using the value from config.paragraphRecycle
- Ensure this happens during both initial setup and when configuration changes
- Update GuiDocumentElement reflection in GuiReflectionElements.cpp (not GuiReflectionControls.cpp)
- Add `CLASS_MEMBER_PROPERTY_FAST(ParagraphRecycle)` entry after the PasswordChar property, following the existing pattern
- The reflection file contains properties like: Document, ParagraphPadding, WrapLine, PasswordChar, CaretBegin, CaretEnd, CaretVisible, CaretColor

### how to test it

No testing is needed as unit test does not cover this control yet.

### rationale

Merging these tasks makes sense because they are tightly coupled - the property is meaningless without the configuration connection, and the configuration connection requires the property to exist. This creates a complete, testable feature in a single task. Adding reflection updates ensures the property is properly exposed to the GacUI type system using the correct CLASS_MEMBER_PROPERTY_FAST macro, which is essential for XML binding and other advanced features. The implementation follows the established pattern in GuiReflectionElements.cpp where other GuiDocumentElement properties like Document, ParagraphPadding, WrapLine, and PasswordChar are registered. InvokeOnElementStateChanged is not needed for this property as it doesn't require immediate renderer notification when changed - the recycling logic is applied during the next render cycle.

## TASK No.2: Implement Paragraph Recycling Logic in GuiDocumentElementRenderer::Render

Implement the core paragraph recycling functionality in the GuiDocumentElementRenderer::Render method. This involves tracking which paragraphs are currently visible during rendering and releasing IGuiGraphicsParagraph instances that were rendered in the previous frame but are no longer visible, while preserving all statistics data for position calculations. The implementation must handle dynamic document updates between renders by tracking range adjustments through NotifyParagraphUpdated calls.

### what to be done

- Add member variables to GuiDocumentElementRenderer to track the range of consecutive visible paragraphs from the previous render:
  - `vint previousRenderBegin` initialized to -1 (indicating invalid/uninitialized range)
  - `vint previousRenderCount` initialized to 0 (when begin is -1, this is also invalid)
- In the Render method, determine the current range of consecutive visible paragraphs (currentBegin, currentCount)
- Handle NotifyParagraphUpdated calls between renders by implementing range adjustment logic:
  - When NotifyParagraphUpdated(index, oldCount, newCount, updatedText) is called, adjust previousRenderBegin and previousRenderCount if they are valid (begin != -1)
  - If updatedText is false, no adjustment is needed (newCount == oldCount is guaranteed)
  - If updatedText is true, handle the three cases:
    - Case 1: If previousRenderBegin + previousRenderCount <= index (range is before the updated area), no adjustment needed
    - Case 2: If previousRenderBegin >= index + oldCount (range is after the updated area), adjust previousRenderBegin by (newCount - oldCount)
    - Case 3: If there is overlap between [previousRenderBegin, previousRenderBegin + previousRenderCount) and [index, index + oldCount):
      - Extend the tracked range to the union of the two ranges: set new begin to min(previousRenderBegin, index) and calculate the union range
      - Then extend the count according to the paragraph count change: count += (newCount - oldCount)
- After rendering is complete, if element->GetParagraphRecycle() is true and previousRenderBegin != -1:
  - Compare the current visible range [currentBegin, currentBegin + currentCount) with the previous range [previousRenderBegin, previousRenderBegin + previousRenderCount)
  - For paragraphs that were in the previous range but not in the current range, release their IGuiGraphicsParagraph instances
  - IMPORTANT: Only release the graphicsParagraph object itself, preserve all statistics data (paragraph bounds, line information, etc.) that may be needed for caret/paragraph/pixel position calculations
  - Consider implementing a selective release mechanism that keeps essential measurement data while freeing rendering resources
- Update the tracking variables to store the current visible range for the next render cycle:
  - Set previousRenderBegin to currentBegin
  - Set previousRenderCount to currentCount
- Ensure the recycling logic only affects paragraphs that were actually rendered (have graphicsParagraph instances)
- Handle edge cases like the first render (begin == -1), empty documents, or when all paragraphs fit in the view

### how to test it

No testing is needed as unit test does not cover this control yet.

### rationale

This enhanced implementation addresses the sophisticated challenge of maintaining accurate paragraph range tracking across dynamic document updates. The use of (previousRenderBegin, previousRenderCount) with begin=-1 as the invalid indicator provides a clean and efficient way to track state without additional boolean flags. The simplified handling of NotifyParagraphUpdated calls focuses only on the updatedText=true case, since updatedText=false guarantees newCount==oldCount and requires no adjustment. The three-case analysis (before, after, overlapping) covers all possible scenarios for paragraph updates when text is actually changed. For overlapping cases, the union-then-adjust approach ensures that the tracked range encompasses all potentially affected paragraphs while maintaining accurate count tracking through the newCount-oldCount adjustment. This approach provides robust memory optimization while maintaining the integrity of position-dependent operations that rely on statistical data.

# Impact to the Knowledge Base

## GacUI

### New Content to Add

The knowledge base currently lacks specific guidance about memory optimization techniques for document rendering and reflection system integration. The following should be added:

#### Memory Optimization in Document Rendering

Document controls can consume significant memory when rendering large documents with many paragraphs:

- Use GuiDocumentConfig::paragraphRecycle to enable automatic cleanup of non-visible paragraph graphics objects
- The paragraph recycling feature tracks consecutive visible paragraphs and releases IGuiGraphicsParagraph instances outside the visible range
- Recycling is most beneficial for documents with many paragraphs where only a small portion is visible at once
- GuiDocumentElement::SetParagraphRecycle provides low-level control over recycling behavior for custom document implementations
- Recycled paragraphs are automatically recreated when they become visible again during scrolling

#### Document Element Property Management

GuiDocumentElement properties follow a consistent pattern for state management:

- Properties that affect rendering should trigger InvokeOnElementStateChanged() when changed to notify the renderer
- Boolean properties typically have Get/Set method pairs with appropriate documentation
- Property changes that affect layout or rendering need proper invalidation to ensure visual updates
- Configuration properties should flow from GuiDocumentConfig through GuiDocumentCommonInterface to GuiDocumentElement for consistent behavior

#### Reflection System Integration

When adding new properties to GUI elements, the reflection system must be updated to ensure proper integration:

- Update GuiReflectionElements.cpp (not GuiReflectionControls.cpp) to include new GuiDocumentElement properties
- Use CLASS_MEMBER_PROPERTY_FAST for simple getter/setter properties following the existing pattern
- Follow the existing pattern for similar element properties to maintain consistency  
- GuiDocumentElement reflection currently includes: Document, ParagraphPadding, WrapLine, PasswordChar, CaretBegin, CaretEnd, CaretVisible, CaretColor
- Ensure reflection updates are included when adding new element properties to support XML binding and type system integration
- Property reflection enables advanced features like data binding and XML-based control configuration

### Existing Content Updates

No existing knowledge base content requires updates for this task, as this adds new functionality rather than modifying existing APIs or design patterns. The paragraph recycling feature is designed to be backward compatible and optional.

# !!!FINISHED!!!

