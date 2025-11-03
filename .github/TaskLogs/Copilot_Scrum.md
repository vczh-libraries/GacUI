# !!!SCRUM!!!

# DESIGN REQUEST

## Goal
You are going to implement `GuiRemoteGraphicsParagraph` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h` and unimplemented document related functions in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`, and necessary unit test

## Background
Remote protocol already implemented renderers of all elements except `GuiDocumentElement` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.h`. But `GuiDocumentElement` will not be implemented in this way, instead it creates and maintains multiple `IGuiGraphicsParagraph` in its dedicated renderer.

`Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_Document.txt` has all comments that you can use while implementing `GuiRemoteGraphicsParagraph`. Although `IGuiGraphicsParagraph` is not an element and not a renderer, but in the protocol definition it pretends to be one of a renderer. So you can learn from other renderer implementation about how to send messages and receive responses and events.

`Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h` will be the implementation processing these messages when headless GacUI aplications are hosted in the unit test project. You can learn from there about the actual function definition of those messages, and also about how other renderers are implemented in the unit test project.

But as a UI framework, there will be multiple implementation of these messages, do not make too many assumptions.

Protocol definitions are code generated to C++ in `Source\PlatformProviders\Remote\Protocol\Generated\GuiRemoteProtocolSchema.h`.

## Proposed Tasks

### Task 1: Define CaretRange and DocumentRunPropertyMap
Define `CaretRange` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h` and `using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;`, with the following functions:

`AddRun`, add a run to the map. `CaretRange` as keys cannot overlap with each other, you will have to do items splitting or remerging accordingly. Meanwhile DocumentInlineObjectRunProperty cannot be splitted, any DocumentTextRunProperty trying to overlap a DocumentInlineObjectRunProperty will results in applying unoverlapped part of the run. The argument of this function are a map and a run
`ResetInlineObjectRun`, remove an inline object run from the map. The argument of this function are a map and a run.
`ClearTextRuns`, remove all DocumentTextRunProperty from the map

### Task 2: Unit Test for Run Management
Unit test for the previous task. You need to create a dedicated test file.

### Task 3: Implement GuiRemoteGraphicsParagraph
Implement `GuiRemoteGraphicsParagraph`. Before doing anything you need to create two protected function:
`NativeTextPosToRemoteTextPos`
`RemoteTextPosToNativeTextPos`
In the current implementation just return the only `vint textPos` parameter directly. But you must call them.

TextPos means caret or textpos or anything representing the unicode code point position in a paragraph. The native one means anything in `GuiRemoteGraphicsParagraph`'s signature. The remote one means anything in protocol data structures. You must call them for converting although they are they same currently, but this is just temporarily.

In this class, some functions are updating runs, some functions are filling members in `ElementDesc_DocumentParagraph`, some functions are calling messages. Identify and categorize them, and implement them by sending messages accordingly.

The `RendererUpdateElement_DocumentParagraph` message is for sending the current paragraph to the remote side and retrieve its size. This message must be send if anything in the paragraph is changed and only before sending other messages. Multiple changes will be made on the paragrpah, but when you don't need information from the remote side, `RendererUpdateElement_DocumentParagraph` is not necessary. This also means sending consecutive of multiple `RendererUpdateElement_DocumentParagraph` is incorrect.

### Task 4: Implement Unit Test Protocol Functions
Implement related functions in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`
This is the foundation of the unit test, so it will not be tested in this task.

### Task 5: Basic GacUI Test Case
Create a GacUI test case, with a window and a GuiSinglelineTextBox. Set some text in the text box. It requires a dedicated test file.

### Task 6: Typing Simulation Test Case
Create another GacUI test case that simulating typing text to the GuiSinglelineTextBox. In the current unit test framework, mouse and key helper functions are completed but typing are not. You will need to also complete them.

# TASKS

- [ ] TASK No.1: Define `CaretRange` struct and run management functions
- [ ] TASK No.2: Create unit test for run management (CaretRange and DocumentRunPropertyMap)
- [ ] TASK No.3: Implement `GuiRemoteGraphicsParagraph` class
- [ ] TASK No.4: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.h`
- [ ] TASK No.5: Create basic `GuiSinglelineTextBox` test case
- [ ] TASK No.6: Create typing simulation test case and complete typing helper functions

## TASK No.1: Define `CaretRange` struct and run management functions

### description

Define a `CaretRange` struct to represent a range of caret positions `(caretBegin, caretEnd)` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`. Implement comparison operators to enable its use as a dictionary key.

Create type alias `using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;` where `DocumentRunProperty` comes from the protocol schema.

Implement three global functions for managing the run map:

1. **`AddRun(DocumentRunPropertyMap& map, const DocumentRun& run)`**: Adds a run to the map, handling overlapping ranges. Key requirements:
   - `CaretRange` keys cannot overlap
   - When overlap occurs, existing ranges must be split or merged
   - `DocumentInlineObjectRunProperty` cannot be split - if a `DocumentTextRunProperty` overlaps it, only apply the non-overlapping parts
   - `DocumentTextRunProperty` can be split when overlapped by another run

2. **`ResetInlineObjectRun(DocumentRunPropertyMap& map, const DocumentRun& run)`**: Removes an inline object run from the specified range. Must handle partial overlaps by splitting ranges as needed.

3. **`ClearTextRuns(DocumentRunPropertyMap& map)`**: Removes all entries containing `DocumentTextRunProperty` from the map, keeping only inline object runs.

These functions form the foundation for managing text formatting and inline objects in paragraphs. The implementation must correctly handle all edge cases including adjacent ranges, partial overlaps, and complete overlaps.

### file locations

New definitions in: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
Implementation in: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

### rationale

This is the foundational data structure for managing text runs in paragraphs. It must be implemented first because `GuiRemoteGraphicsParagraph` depends on it. The run management logic is complex and deserves isolated implementation and testing. By creating these utility functions, we separate the run merging/splitting logic from the paragraph class implementation, making both easier to understand and test.

## TASK No.2: Create unit test for run management (CaretRange and DocumentRunPropertyMap)

### description

Create a dedicated test file `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` using the standard GacUI test framework.

Test cases should cover:

1. **Basic AddRun operations**:
   - Adding non-overlapping runs
   - Adding adjacent runs
   - Complete overlap (same range)
   - Partial overlap with text runs
   - Overlapping inline object runs (should reject overlapping portions)

2. **Split and merge scenarios**:
   - Text run split by another text run
   - Multiple splits creating fragmented ranges
   - Merging identical adjacent properties

3. **Inline object protection**:
   - Attempting to add text run over inline object (should only apply non-overlapping parts)
   - Adding inline object over existing inline object
   - Edge cases where inline objects are adjacent

4. **ResetInlineObjectRun**:
   - Removing entire inline object
   - Partial removal causing split
   - Removing from empty map
   - Removing non-existent range

5. **ClearTextRuns**:
   - Clearing map with only text runs
   - Clearing map with mixed runs (inline objects should remain)
   - Clearing empty map

Use `TEST_FILE`, `TEST_CATEGORY`, and `TEST_CASE` macros. Use `TEST_ASSERT` for verification. Each test should verify the map state after operations using `map.Count()` and checking specific entries.

### file locations

New file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

The run management logic is complex with many edge cases. Testing it in isolation before using it in `GuiRemoteGraphicsParagraph` will catch bugs early. This follows the test-driven development approach and ensures the foundation is solid. The dedicated test file keeps related tests organized and makes it easy to add more test cases as edge cases are discovered.

## TASK No.3: Implement `GuiRemoteGraphicsParagraph` class

### description

Implement all methods of `GuiRemoteGraphicsParagraph` class in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`.

**Step 1: Add protected members and helper functions**

Add protected members:
- `DocumentRunPropertyMap runs` - stores text formatting and inline objects
- `bool wrapLine = false`
- `vint maxWidth = -1`
- `Alignment paragraphAlignment = Alignment::Left`
- `Size cachedSize = Size(0, 0)`
- `bool needUpdate = true` - tracks if paragraph needs resending to remote
- `vint id = -1` - paragraph ID for protocol communication

Add protected conversion functions:
- `vint NativeTextPosToRemoteTextPos(vint textPos)` - currently returns `textPos` directly
- `vint RemoteTextPosToNativeTextPos(vint textPos)` - currently returns `textPos` directly

**Step 2: Categorize IGuiGraphicsParagraph methods**

Group methods into three categories:

*Category A: Property getters/setters (affect ElementDesc_DocumentParagraph)*:
- `GetWrapLine`, `SetWrapLine`
- `GetMaxWidth`, `SetMaxWidth`
- `GetParagraphAlignment`, `SetParagraphAlignment`

*Category B: Run manipulation (modify runs map)*:
- `SetFont`, `SetSize`, `SetStyle`, `SetColor`, `SetBackgroundColor`
- `SetInlineObject`, `ResetInlineObject`

*Category C: Query operations (send messages)*:
- `GetSize` - sends `RendererUpdateElement_DocumentParagraph`, caches result
- `GetCaret` - sends `DocumentParagraph_GetCaret`
- `GetCaretBounds` - sends `DocumentParagraph_GetCaretBounds`
- `GetCaretFromPoint` - sends `DocumentParagraph_GetCaretBounds` with point
- `GetInlineObjectFromPoint` - sends `DocumentParagraph_GetInlineObjectFromPoint`
- `GetNearestCaretFromTextPos` - sends `DocumentParagraph_GetNearestCaretFromTextPos`
- `IsValidCaret` - sends `DocumentParagraph_IsValidCaret`
- `OpenCaret` - sends `DocumentParagraph_OpenCaret`
- `CloseCaret` - sends `DocumentParagraph_CloseCaret`

**Step 3: Implement update strategy**

- Category A and B methods set `needUpdate = true`
- Before any Category C message, if `needUpdate == true`, send `RendererUpdateElement_DocumentParagraph` first with runs converted to `runsDiff` array, then set `needUpdate = false`
- `GetSize()` caches the size returned from `RendererUpdateElement_DocumentParagraph`
- Constructor allocates paragraph ID via `remote->AllocateParagraphId()`
- Destructor sends paragraph destruction message

**Step 4: Implement each method**

- `GetProvider`, `GetRenderTarget`, `IsValidTextPos` - simple implementations
- Property methods - update members and set `needUpdate = true`
- Run manipulation methods - use `AddRun`, `ResetInlineObjectRun` with text position conversion
- Query methods - ensure update is sent, then send corresponding protocol message with proper text position conversion
- `Render` - triggers rendering via render target

### file locations

Implementation: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`
Header updates (if needed): `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`

### rationale

This is the core implementation that bridges `IGuiGraphicsParagraph` interface with the remote protocol. The three-category approach cleanly separates concerns: properties configure the paragraph, run methods modify formatting, query methods interact with the remote side. The lazy update strategy (only send when needed) optimizes protocol traffic. Text position conversion functions, though currently identity functions, provide future extensibility for surrogate pair handling or other text encoding complexities.

## TASK No.4: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.h`

### description

Implement the document-related protocol handler methods in `UnitTestRemoteProtocol_Rendering` template class in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`.

The current file has placeholder implementations (mostly empty or throwing errors). Implement:

**Main rendering handler**:
- `RequestRendererUpdateElement_DocumentParagraph(const ElementDesc_DocumentParagraph& arguments)`: 
  - Store paragraph text, wrapLine, maxWidth, alignment
  - Process `runsDiff` array to build internal run representation
  - Calculate paragraph size based on font metrics (similar to `CalculateSolidLabelSizeIfNecessary`)
  - Return calculated size

**Caret navigation handlers**:
- `RequestDocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)`:
  - Implement basic caret movement logic for CaretFirst, CaretLast, CaretLineFirst, CaretLineLast
  - For CaretMoveLeft/Right/Up/Down, calculate new position based on text layout
  - Return `GetCaretResponse` with new caret and preferFrontSide

- `RequestDocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)`:
  - Calculate bounds rectangle for caret at given position
  - Consider frontSide flag for RTL/complex scripts
  - Return Rect with width=0, positioned at caret

- `RequestDocumentParagraph_GetCaretFromPoint(vint id, const Point& arguments)`:
  - Perform hit testing to find nearest caret to given point
  - Consider line layout and character positions

**Other query handlers**:
- `RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const Point& arguments)`:
  - Check if point intersects any inline object
  - Return DocumentRun with inline object info, or null

- `RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)`:
  - Convert text position to nearest valid caret position
  - Consider frontSide preference

- `RequestDocumentParagraph_IsValidCaret(vint id, const int& arguments)`:
  - Validate if caret position is within valid range (0 to text.Length())

**Caret display handlers**:
- `RequestDocumentParagraph_OpenCaret(vint id, const OpenCaretRequest& arguments)`:
  - Store caret state for rendering (position, color, frontSide)

- `RequestDocumentParagraph_CloseCaret(vint id)`:
  - Clear stored caret state

**Implementation approach**:

Follow the pattern from `CalculateSolidLabelSizeIfNecessary`:
- Use cached font measurements
- Store paragraph state in `lastElementDescs` or similar structure keyed by `id`
- For complex layout calculations, implement simplified version suitable for testing (exact pixel-perfect layout not required, but consistent behavior is)

### file locations

Modified file: `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`

### rationale

These handlers are the server-side implementation of the paragraph protocol for the unit test environment. Without these implementations, any test using document controls will fail. The implementations can be simplified compared to production renderers (e.g., Windows/Linux) since they only need to support testing scenarios. However, they must be consistent and functional enough to validate the paragraph protocol and allow document control tests to pass.

## TASK No.5: Create basic `GuiSinglelineTextBox` test case

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

### file locations

New file: `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`

### rationale

This test validates that the basic paragraph protocol implementation works correctly with a real GacUI control. `GuiSinglelineTextBox` uses `GuiDocumentLabel` which uses `IGuiGraphicsParagraph`, so this test exercises the complete stack. By testing programmatic text manipulation first (before user input simulation), we verify the foundational functionality before adding complexity. This test will catch integration issues between the paragraph implementation and the document control system.

## TASK No.6: Create typing simulation test case and complete typing helper functions

### description

**Part 1: Complete typing helper functions**

In `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h` and `.cpp`, add typing support:

- `TypeText(const WString& text)`: Types each character by:
  - Sending KeyDown for the character
  - Sending Char event for the character
  - Sending KeyUp for the character
  - Use appropriate virtual key codes for common characters

- `TypeString(const WString& text)`: Types entire string by calling `TypeText` for each character

Handle special characters:
- Space, Enter, Tab
- Punctuation
- Numbers and letters (uppercase/lowercase - consider shift key state)

Study existing `KeyPress`, `_KeyDown`, `_KeyUp` methods for pattern.

**Part 2: Create typing test case**

Add a new test case in `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp`:

```cpp
TEST_CASE(L"GuiSinglelineTextBox Typing")
{
    GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
    {
        protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            
            // Clear existing text
            textBox->SelectAll();
            protocol->TypeText(L"Hello");
        });
        protocol->OnNextIdleFrame(L"Typed Hello", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello");
            
            protocol->TypeText(L" World");
        });
        protocol->OnNextIdleFrame(L"Typed World", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello World");
            
            // Test backspace
            protocol->KeyPress(VKEY::KEY_BACK);
        });
        protocol->OnNextIdleFrame(L"Backspace pressed", [&, protocol]()
        {
            auto window = GetApplication()->GetMainWindow();
            auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
            TEST_ASSERT(textBox->GetText() == L"Hello Worl");
            
            window->Hide();
        });
    });
    GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...);
}
```

Test cases should verify:
- Basic character typing
- String typing
- Special keys (backspace, delete, arrows)
- Text selection + typing (replaces selection)
- Cursor positioning with mouse then typing

### file locations

Protocol functions: `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h` and `.cpp`
Test case: `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp` (add to existing file)

### rationale

Typing simulation is essential for testing interactive text editing scenarios. The current unit test framework has mouse and keyboard primitives but no high-level typing functions. Adding these functions enables realistic user interaction testing. This test validates the complete paragraph editing pipeline: user input → text change → rendering update → visual verification. It's the most comprehensive test of the paragraph implementation and will catch issues that only appear during interactive editing.

# Impact to the Knowledge Base

## GacUI Project

### Changes to Existing Content

No existing knowledge base content needs to be updated. The paragraph protocol implementation is an internal detail of the remote rendering system and doesn't change any public APIs or design patterns documented in the knowledge base.

### New Content to Add

**New Topic under GacUI/Design Explanation**:

#### Topic: IGuiGraphicsParagraph Protocol and Remote Implementation

Description: The paragraph protocol enables remote rendering of rich text paragraphs with complex formatting. Unlike regular elements that have renderers, paragraphs are created and managed by document elements through a factory pattern. The protocol treats paragraphs as pseudo-renderers with a unique lifecycle.

Key architectural points:
- Paragraphs don't inherit from `IGuiGraphicsElement` but implement `IGuiGraphicsParagraph`
- Run-based text formatting with support for inline objects (elements embedded in text)
- Lazy protocol updates - changes batched and sent only when remote data is needed
- Text position abstraction layer for future Unicode handling improvements
- Separate protocol handler implementations per platform (unit test, Windows, Linux, etc.)

Bullet points:
- Run management requires careful handling of overlapping ranges and inline object protection
- The `RendererUpdateElement_DocumentParagraph` message serves as both update and query
- Caret position calculations must account for text layout, wrapping, and inline objects
- Protocol message ordering matters: always send paragraph update before caret queries
- Unit test implementations can be simplified but must be consistent

[Design Explanation](./KB_GacUI_Design_IGuiGraphicsParagraphProtocol.md)

**Rationale**: This is a complex subsystem with unique design characteristics. While not needed for typical GacUI application development, it's important for developers working on:
- Remote rendering implementations
- Unit test infrastructure
- Document control debugging
- Platform porting

The design differs significantly from regular element rendering and deserves documentation to guide future maintainers.

# !!!FINISHED!!!
