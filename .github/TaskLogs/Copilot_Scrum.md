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

# TASKS

- [ ] TASK No.1: Add ParagraphRecycle Property to GuiDocumentElement and Connect to Configuration System
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

- Verify the property can be set and retrieved correctly
- Ensure the property maintains its value across multiple get/set operations
- Test that the default value is false when the element is newly created
- Create unit tests that verify the paragraphRecycle value is correctly passed from config to the document element
- Test that changing the configuration properly updates the document element property
- Verify that different document controls (viewer, label, etc.) correctly propagate the setting
- Ensure the property flows correctly from GuiDocumentConfig through GuiDocumentConfigEvaluated to the element
- Test reflection functionality to ensure the property is properly exposed and accessible using CLASS_MEMBER_PROPERTY_FAST
- Verify that existing functionality is not affected by the addition of this property
- NOTE: No testing needed for element state change notifications since InvokeOnElementStateChanged is not required

### rationale

Merging these tasks makes sense because they are tightly coupled - the property is meaningless without the configuration connection, and the configuration connection requires the property to exist. This creates a complete, testable feature in a single task. Adding reflection updates ensures the property is properly exposed to the GacUI type system using the correct CLASS_MEMBER_PROPERTY_FAST macro, which is essential for XML binding and other advanced features. The implementation follows the established pattern in GuiReflectionElements.cpp where other GuiDocumentElement properties like Document, ParagraphPadding, WrapLine, and PasswordChar are registered. InvokeOnElementStateChanged is not needed for this property as it doesn't require immediate renderer notification when changed - the recycling logic is applied during the next render cycle.

## TASK No.2: Implement Paragraph Recycling Logic in GuiDocumentElementRenderer::Render

Implement the core paragraph recycling functionality in the GuiDocumentElementRenderer::Render method. This involves tracking which paragraphs are currently visible during rendering and releasing IGuiGraphicsParagraph instances that were rendered in the previous frame but are no longer visible.

### what to be done

- Add member variables to GuiDocumentElementRenderer to track the range of consecutive visible paragraphs from the previous render
- In the Render method, determine the current range of consecutive visible paragraphs (startParagraph to endParagraph)
- After rendering is complete, if element->GetParagraphRecycle() is true, compare the current visible range with the previous visible range
- For paragraphs that were visible in the previous render but are not visible in the current render, release their IGuiGraphicsParagraph instances
- Update the tracking variables to store the current visible range for the next render cycle
- Ensure the recycling logic only affects paragraphs that were actually rendered (have graphicsParagraph instances)
- Handle edge cases like the first render, empty documents, or when all paragraphs fit in the view

### how to test it

- Unit tests that verify paragraphs outside the visible range are properly recycled when paragraphRecycle is true
- Test that paragraphs are not recycled when paragraphRecycle is false
- Verify that visible paragraphs are never recycled, even when recycling is enabled
- Test scrolling scenarios where different paragraphs become visible/invisible
- Ensure recycling works correctly with various document sizes and viewport configurations
- Test edge cases like single paragraph documents, empty documents, and documents smaller than the viewport
- Verify that recycled paragraphs are properly recreated when they become visible again

### rationale

This is the core implementation that provides the memory optimization benefit of paragraph recycling. By tracking the consecutive visible paragraph range between renders, the system can identify which paragraph graphics objects are no longer needed and can be safely released. This reduces memory usage for large documents where only a small portion is visible at any time. The implementation respects the recycling setting and only affects non-visible paragraphs, ensuring rendering quality is maintained.

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

