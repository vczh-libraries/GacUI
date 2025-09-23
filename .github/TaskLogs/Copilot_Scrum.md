# !!!SCRUM!!!

# DESIGN REQUEST

There is GuiDocumentConfig::paragraphRecycle. When it is true, IGuiGraphicsParagraph scrolled out of the control are recommended to be removed. It is OK not to completely clean but at least most of them must be clean.

In order to do this, I believe a ParagraphRecycle property should be added to GuiDocumentElement, just put it right after PasswordChar. It has a default value of false. By doing this, document controls can pass this value via its base class GuiDocumentCommonInterface to GuiDocumentElementRenderer

GuiDocumentElementRenderer::Render is a place that actually do the rendering. You can easily find out how to calculate the range of consecurive visible IGuiGraphicsParagraph after rendering. This is the key place to do the work.

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
- [ ] TASK No.2: Implement Paragraph Recycling Logic in GuiDocumentElementRenderer::Render

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

