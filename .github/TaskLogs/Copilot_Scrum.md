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

# UPDATES

## UPDATE

Proposing a change to Task 1 and Task 2. The goal is to come out with a way to produce `runDiff` in `ElementDesc_DocumentParagraph`. `runDiff` means the difference between two `RendererUpdateElement_DocumentParagraph`.

To make the protocol complete, `createdInlineObjects` and `removedInlineObjects` is added. And due to the fact that inline objects are always high proprity than text, changes should be applied:

- Two new map types need to be added: `DocumentTextRunPropertyMap` and `DocumentInlineObjectRunProperty`. Just like the originally proposed `DocumentRunPropertyMap`.
- Discard all originally proposed functions and replace with the following:
  - AddTextRun, receiving `DocumentTextRunPropertyMap` and `DocumentTextRunProperty`. Make sure keys in the map does not overlap, when it happens, splitting should be applied. And I also don't want consecutive text runs with same property value, so after adding any run, you must check consecutive runs around it and merge them to one if property values are identical.
  - AddInlineObjectRun and ResetInlineObjectRun. It is different from text run as an inline object should always be complete, overlapping leading to failure. ResetInlineObjectRun's second parameter will be a `CaretRange` instead of a run.
  - MergeRuns, take two maps and produce a `DocumentRunPropertyMap` map. When any text run and inline object run overlapped, keep the inline object run, cut the text run if only part of it overlapps.
  - DiffRuns, take two DocumentRunPropertyMap to fill `runDiff`, `createInlineObjects` and `removeInlineObjects` in a given `ElementDesc_DocumentParagraph&` argument. 
- Be awared that Dictionary items are ordered by key, optimize your way leveraging to this fact.

Since the design has changed, rework Task 2 completely.

## UPDATE

Some correction to Task 1, update Task 2 accordingly.

- `AddInlineObjectRun` returns `false` for failure.
- `ResetInlineObjectRun` does not allow overlapping, it removes the value binding to the exact key. Returns `false` if the key cannot be found.
- `DiffRuns` pay attention to that, keys between `oldRuns` and `newRuns` could be different, e.g.  splitted or merged. When a part of an old run is updated, the function should always keep complete keys of `newRuns` in the result.

# TASKS

- [x] TASK No.1: Define `CaretRange` struct and run management functions
- [ ] TASK No.2: Create unit test for run management (CaretRange and DocumentRunPropertyMap)
- [ ] TASK No.3: Implement `GuiRemoteGraphicsParagraph` class
- [ ] TASK No.4: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.h`
- [ ] TASK No.5: Create basic `GuiSinglelineTextBox` test case
- [ ] TASK No.6: Create typing simulation test case and complete typing helper functions

## TASK No.1: Define `CaretRange` struct and run management functions

### description

Define a `CaretRange` struct to represent a range of caret positions `(caretBegin, caretEnd)` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`. Implement comparison operators (`<`, `==`, `!=`) to enable its use as a dictionary key. The comparison should order by `caretBegin` first, then by `caretEnd`.

Create three type aliases for managing runs:
- `using DocumentTextRunPropertyMap = Dictionary<CaretRange, DocumentTextRunProperty>;`
- `using DocumentInlineObjectRunPropertyMap = Dictionary<CaretRange, DocumentInlineObjectRunProperty>;`
- `using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;`

All types come from the protocol schema in `GuiRemoteProtocolSchema.h`.

Implement four global functions for managing runs:

1. **`AddTextRun(DocumentTextRunPropertyMap& map, CaretRange range, const DocumentTextRunProperty& property)`**: 
   - Adds a text run to the map with the specified range and property
   - Keys in the map cannot overlap - when overlap occurs, split existing runs
   - After adding, merge consecutive runs with identical property values into a single run
   - Algorithm: Insert the new run, then iterate through potentially affected ranges to split them, then scan for consecutive identical runs to merge

2. **`AddInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range, const DocumentInlineObjectRunProperty& property) -> bool`**:
   - Adds an inline object run to the map
   - Inline objects cannot be split and cannot overlap with other inline objects
   - If the new range overlaps any existing inline object, the operation fails and returns `false`
   - Returns `true` on success

3. **`ResetInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range) -> bool`**:
   - Removes the inline object run with the exact matching key (range)
   - Does NOT allow overlapping - must match the exact key
   - Returns `false` if the key cannot be found in the map
   - Returns `true` if the key is found and removed

4. **`MergeRuns(const DocumentTextRunPropertyMap& textRuns, const DocumentInlineObjectRunPropertyMap& inlineObjectRuns, DocumentRunPropertyMap& result)`**:
   - Combines two separate maps into a unified DocumentRunPropertyMap
   - When text runs and inline object runs overlap, inline objects have priority
   - Text runs that overlap inline objects must be cut/split to avoid the inline object ranges
   - Leverage the fact that Dictionary items are ordered by key for efficient iteration

5. **`DiffRuns(const DocumentRunPropertyMap& oldRuns, const DocumentRunPropertyMap& newRuns, ElementDesc_DocumentParagraph& result)`**:
   - Compares two run maps and fills the difference into the result
   - Populates `result.runsDiff` with runs that changed (added, removed, or modified)
   - Populates `result.createdInlineObjects` with inline objects that were added
   - Populates `result.removedInlineObjects` with inline objects that were removed
   - **IMPORTANT**: Keys between `oldRuns` and `newRuns` could be different (e.g., splitted or merged)
   - When a part of an old run is updated, the function must always keep complete keys of `newRuns` in the result
   - The result should represent the final state with `newRuns` keys, not a mixture of old and new keys
   - Efficiently process both maps in a single pass by leveraging their sorted key order

### what to be done

- Define `CaretRange` struct with `caretBegin` and `caretEnd` members, and comparison operators
- Define three type aliases for the different run map types
- Implement `AddTextRun` with splitting logic and consecutive run merging
- Implement `AddInlineObjectRun` with overlap detection, return `false` for failure (overlapping inline objects)
- Implement `ResetInlineObjectRun` to remove exact key match only, return `false` if key not found
- Implement `MergeRuns` with inline object priority over text runs
- Implement `DiffRuns` with efficient two-map comparison, ensuring the result uses complete keys from `newRuns` (not a mixture of old and new keys)
- All implementations should leverage Dictionary's sorted key property for optimal performance

### how to test it

Testing will be covered comprehensively in Task 2. The functions are designed to be pure logic operations on dictionaries, making them highly testable in isolation.

### file locations

New definitions in: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
Implementation in: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

### rationale

This redesign separates text runs from inline object runs at the storage level, reflecting their fundamentally different behaviors:
- Text runs can be split, merged, and overlap freely
- Inline objects are atomic and cannot be split or overlap

This separation makes the implementation cleaner and more maintainable. The `MergeRuns` function brings them together only when needed, enforcing the priority rule (inline objects > text runs). The `DiffRuns` function is crucial for the protocol - it enables sending only changes between updates, minimizing network traffic. By leveraging Dictionary's ordered keys, all operations can be implemented efficiently with linear complexity.

## TASK No.2: Create unit test for run management (CaretRange and DocumentRunPropertyMap)

### description

Create a dedicated test file `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` using the standard GacUI test framework.

Use `TEST_FILE`, `TEST_CATEGORY`, and `TEST_CASE` macros. Use `TEST_ASSERT` for verification. Each test should verify the map state after operations using `map.Count()` and checking specific entries with map iteration or direct key lookup.

**Test Category 1: CaretRange**
- Test comparison operators (`<`, `==`, `!=`)
- Verify ordering: ranges with smaller `caretBegin` come first, then by `caretEnd`
- Test edge cases: equal ranges, overlapping ranges, adjacent ranges

**Test Category 2: AddTextRun**

1. **Basic operations**:
   - Adding to empty map
   - Adding non-overlapping runs (verify map contains both)
   - Adding adjacent runs with different properties (should remain separate)
   - Adding adjacent runs with identical properties (should merge into one)

2. **Splitting scenarios**:
   - New run completely overlaps existing run (old run replaced)
   - New run partially overlaps existing run (split into non-overlapping parts)
   - New run overlaps multiple existing runs (all affected runs split/replaced)
   - New run is contained within existing run (existing run split into three parts: before, overlap, after)

3. **Merging scenarios**:
   - After adding a run that creates adjacent identical runs, verify they merge
   - Multiple consecutive identical runs merge into one
   - Different properties prevent merging even if adjacent

**Test Category 3: AddInlineObjectRun**

1. **Success cases**:
   - Adding to empty map
   - Adding non-overlapping inline objects

2. **Failure cases** (should return false or throw):
   - Adding inline object that overlaps existing inline object (complete overlap)
   - Adding inline object that partially overlaps existing inline object
   - Adding inline object that contains existing inline object
   - Adding inline object that is contained by existing inline object

3. **Edge cases**:
   - Adjacent inline objects (should succeed - no overlap)
   - Multiple inline objects in map, new one fits in gap (should succeed)

**Test Category 4: ResetInlineObjectRun**

1. **Success cases** (returns true):
   - Remove inline object with exact range match (key found)
   - Verify the specific inline object is removed from map

2. **Failure cases** (returns false):
   - Remove from empty map (key not found)
   - Remove non-existent range (key not found)
   - Remove with partially overlapping range (key doesn't match exactly)
   - Remove with range that contains an inline object but doesn't match exactly
   - Remove with range that is contained by an inline object but doesn't match exactly

3. **Edge cases**:
   - Remove with adjacent range (should fail - no exact match)
   - Multiple inline objects in map, remove one exact match (only that one removed)
   - After successful removal, verify map state is correct

**Test Category 5: MergeRuns**

1. **Basic merging**:
   - Merge empty maps (result is empty)
   - Merge with only text runs (result contains all text runs)
   - Merge with only inline objects (result contains all inline objects)
   - Merge with both types, no overlap (result contains all runs)

2. **Inline object priority**:
   - Text run completely overlaps inline object (text run cut out)
   - Text run partially overlaps inline object (text run split, non-overlapping part kept)
   - Multiple text runs overlap one inline object (all cut appropriately)
   - One text run overlaps multiple inline objects (text run fragmented)

3. **Complex scenarios**:
   - Text runs with gaps, inline objects fill gaps (interleaved result)
   - Consecutive text runs merged, then cut by inline object

**Test Category 6: DiffRuns**

1. **Empty and simple diffs**:
   - Both maps empty (no diff)
   - Old map empty, new map has runs (all runs in `runsDiff`, inline objects in `createdInlineObjects`)
   - New map empty, old map has runs (all runs in `runsDiff` with remove indication, inline objects in `removedInlineObjects`)

2. **Changes detection**:
   - Same text run in both maps with same key (no diff)
   - Text run property changed (appears in `runsDiff`)
   - Text run range changed (old removed, new added in `runsDiff`)
   - Text run added (appears in `runsDiff`)
   - Text run removed (appears in `runsDiff`)

3. **Inline object tracking**:
   - Inline object added (in both `runsDiff` and `createdInlineObjects`)
   - Inline object removed (in both `runsDiff` and `removedInlineObjects`)
   - Inline object unchanged (not in any diff list)
   - Inline object moved (old removed, new created)

4. **Key difference handling** (CRITICAL):
   - Old run split into multiple new runs (verify result uses new run keys completely)
   - Multiple old runs merged into one new run (verify result uses new run key)
   - One old run partially updated: part becomes new run with different key (verify result contains complete new run key, not partial)
   - Example: old run [0,10], new runs [0,5] and [5,10] with different properties → result should have [0,5] and [5,10] as complete entries
   - Example: old runs [0,5] and [5,10], new run [0,10] → result should have [0,10] as complete entry if property changed
   - Verify that `runsDiff` always contains complete keys from `newRuns`, never partial keys or mixed old/new keys

5. **Complex scenarios**:
   - Multiple changes in one diff operation with key splitting/merging
   - Mixed text runs and inline objects changing with different keys
   - Verify efficient single-pass algorithm by checking performance on large maps

### what to be done

Create the test file with comprehensive test cases organized into six test categories. Each test case should:
- Set up initial state (create maps, add runs)
- Perform the operation being tested
- Verify the result state (check map contents, counts, specific entries)
- Test both success and failure paths where applicable

For `AddInlineObjectRun` and `ResetInlineObjectRun`, verify return values (`true` for success, `false` for failure).

For diff testing, create helper functions to verify the contents of `runsDiff`, `createdInlineObjects`, and `removedInlineObjects` arrays. Pay special attention to verifying that keys in the diff result come from `newRuns`, not a mixture of old and new keys.

### how to test it

This task IS the testing task. Verification is done by running the compiled unit test executable. The test cases themselves serve as the specification and validation of the run management functions.

### file locations

New file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

The redesigned run management functions have more complex behavior than the original design:
- Text runs support splitting and merging
- Inline objects have strict no-overlap rules and exact-match removal semantics
- `AddInlineObjectRun` returns `false` on overlap failure
- `ResetInlineObjectRun` requires exact key match and returns `false` if not found
- MergeRuns implements priority-based combining
- DiffRuns requires efficient two-map comparison and must preserve `newRuns` keys completely

Each function has numerous edge cases that must be tested thoroughly. Comprehensive unit tests ensure:
1. Correctness of complex splitting/merging logic
2. Proper handling of inline object priority and failure cases
3. Correct difference detection for protocol updates with proper key handling
4. Prevention of invalid states (overlapping inline objects)
5. Verification that `DiffRuns` always uses complete keys from `newRuns` in the result

Testing in isolation catches bugs early and provides confidence before integrating with `GuiRemoteGraphicsParagraph`. The test cases also serve as executable documentation of the expected behavior, especially for the critical key-handling requirements in `DiffRuns`.

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
