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

## UPDATE

Break Task 2 into 5 separate tasks:
1. First task for CaretRange and AddTextRun testing
2. One task each for AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, and DiffRuns testing

# TASKS

- [x] TASK No.1: Define `CaretRange` struct and run management functions
- [x] TASK No.2: Unit test for CaretRange comparison and AddTextRun
- [x] TASK No.3: Unit test for AddInlineObjectRun
- [x] TASK No.4: Unit test for ResetInlineObjectRun
- [x] TASK No.5: Unit test for MergeRuns
- [x] TASK No.6: Unit test for DiffRuns
- [ ] TASK No.7: Implement `GuiRemoteGraphicsParagraph` class
- [ ] TASK No.8: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.h`
- [ ] TASK No.9: Create basic `GuiSinglelineTextBox` test case
- [ ] TASK No.10: Create typing simulation test case and complete typing helper functions

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

## TASK No.2: Unit test for CaretRange comparison and AddTextRun

### description

Create a dedicated test file `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` using the standard GacUI test framework.

This task covers the first two test categories: **CaretRange** and **AddTextRun**.

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

**Helper Functions** (to be created for use across all test tasks):
- `CreateTextProp(vint colorValue)`: Creates a `DocumentTextRunProperty` with distinguishable colors
- `CreateInlineProp(vint callbackId, vint width)`: Creates a `DocumentInlineObjectRunProperty`
- `FormatRunProperty`: Overloaded functions to convert properties to readable strings
- `CompareRunProperty`: Overloaded functions to compare property equality
- `PrintMap`: Template function to print actual and expected maps
- `AssertMap`: Template function to compare maps and fail with detailed output if mismatched

These helper functions follow the pattern from `TestItemProviders.h` (`PrintCallbacks` and `AssertCallbacks`).

### what to be done

1. Create the test file with proper includes (`TestRemote.h`)
2. Implement all helper functions at the top of the file
3. Create `TEST_FILE` block with two `TEST_CATEGORY` blocks
4. For CaretRange: Create test cases for all comparison operators and ordering rules
5. For AddTextRun: Create test cases for basic operations, splitting scenarios, and merging scenarios
6. Each test case should:
   - Set up initial state (create maps, add runs)
   - Perform the operation being tested
   - Use `AssertMap` to verify the result

### how to test it

Run the compiled unit test executable. The test cases validate:
1. CaretRange can be used as Dictionary key with correct ordering
2. AddTextRun correctly handles splitting when runs overlap
3. AddTextRun correctly merges consecutive runs with identical properties
4. Map state is always consistent after operations

### file locations

New file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

CaretRange and AddTextRun are foundational - all other functions depend on them. CaretRange must work correctly for Dictionary operations. AddTextRun is the most complex function with splitting and merging logic that needs thorough validation. Testing these first ensures a solid foundation. The helper functions created here will be reused by all subsequent test tasks, establishing a consistent testing pattern.

## TASK No.3: Unit test for AddInlineObjectRun

### description

Add test category for `AddInlineObjectRun` to `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`.

**Test Category 3: AddInlineObjectRun**

1. **Success cases** (return true):
   - Adding to empty map
   - Adding non-overlapping inline objects (multiple objects with gaps)
   - Adjacent inline objects (touching at boundaries but not overlapping)

2. **Failure cases** (return false):
   - Adding inline object that completely overlaps existing inline object
   - Adding inline object that partially overlaps existing inline object from left
   - Adding inline object that partially overlaps existing inline object from right
   - Adding inline object that contains existing inline object
   - Adding inline object that is contained within existing inline object

3. **Edge cases**:
   - Multiple inline objects in map, new one fits in gap (should succeed)
   - Attempting to add inline object in a position that would touch but not overlap (should succeed)

### what to be done

1. Add a new `TEST_CATEGORY(L"AddInlineObjectRun")` block to the existing test file
2. Create test cases for all success scenarios, verifying:
   - Return value is `true`
   - Map count increases correctly
   - Map contains the expected entries (use `AssertMap`)
3. Create test cases for all failure scenarios, verifying:
   - Return value is `false`
   - Map state is unchanged (objects that failed to add are not in map)
4. Use existing helper functions (`CreateInlineProp`, `AssertMap`)
5. **IMPORTANT**: Use designated initializers for CaretRange: `{.caretBegin = x, .caretEnd = y}`

### how to test it

Run the compiled unit test executable. The test cases validate:
1. AddInlineObjectRun succeeds when there's no overlap
2. AddInlineObjectRun fails and returns false for any overlapping scenario
3. Map state remains consistent - no partial updates on failure
4. Adjacent inline objects (non-overlapping) are correctly handled

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

AddInlineObjectRun has critical no-overlap validation logic. Inline objects represent atomic UI elements (like images or controls) that cannot be split. The overlap detection must be bulletproof - any bug here could cause protocol corruption or crashes. Testing in isolation ensures the binary search-based overlap detection works correctly for all edge cases. This function's boolean return value (instead of throwing) requires careful verification of both return value and map state.

**Note from Task 2 learning**: The implementation uses the same binary search pattern as AddTextRun. However, unlike AddTextRun which must handle the "any match" behavior of binary search, AddInlineObjectRun only needs to detect *any* overlap (returns -1 for no overlap, non-negative for found overlap). So it doesn't need the backward scan that AddTextRun required.

## TASK No.4: Unit test for ResetInlineObjectRun (✓ COMPLETED)

### description

Add test category for `ResetInlineObjectRun` to `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`.

**Test Category 4: ResetInlineObjectRun**

1. **Success cases** (returns true):
   - Remove inline object with exact range match from map with single object
   - Remove inline object with exact range match from map with multiple objects
   - Verify the specific inline object is removed and others remain

2. **Failure cases** (returns false):
   - Remove from empty map (key not found)
   - Remove with non-existent range (key not found)
   - Remove with partially overlapping range (key doesn't match exactly)
   - Remove with range that contains an inline object but doesn't match exactly
   - Remove with range that is contained within an inline object but doesn't match exactly
   - Remove with adjacent range (should fail - no exact match)

3. **Edge cases**:
   - After successful removal, verify map count decreased by exactly 1
   - After failed removal, verify map state is completely unchanged

### what to be done

1. Add a new `TEST_CATEGORY(L"ResetInlineObjectRun")` block to the existing test file
2. Create test cases for success scenarios:
   - Set up initial map with inline objects
   - Call `ResetInlineObjectRun` with exact matching key
   - Verify return value is `true`
   - Verify map count decreased
   - Verify specific object removed (use `AssertMap` with expected state)
3. Create test cases for failure scenarios:
   - Set up initial map state
   - Call `ResetInlineObjectRun` with various non-matching keys
   - Verify return value is `false`
   - Verify map state unchanged (use `AssertMap` comparing before and after)
4. Use existing helper functions
5. **IMPORTANT**: Use designated initializers for CaretRange: `{.caretBegin = x, .caretEnd = y}`

### how to test it

Run the compiled unit test executable. The test cases validate:
1. ResetInlineObjectRun requires exact key match (no partial matching)
2. Function returns true only when key is found and removed
3. Function returns false when key doesn't exist or doesn't match exactly
4. Map state is correctly updated on success, unchanged on failure

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

ResetInlineObjectRun has strict exact-match semantics that differ from typical removal operations. The function only removes if the key matches exactly - this prevents accidental removal of inline objects when text formatting changes nearby. The boolean return value lets callers detect whether removal succeeded, enabling proper error handling in the protocol layer. Testing all failure cases ensures the exact-match requirement is properly enforced.

**Learnings from Task 4 execution**: This task completed successfully with no user edits required. Key success factors:
1. **Simple implementation leveraging library functions**: The function is a thin wrapper around `Dictionary::Remove()`, demonstrating that when library functionality matches requirements perfectly, simple implementations are best.
2. **Comprehensive test coverage**: 17 test cases (4 success, 10 failure, 3 edge cases) thoroughly validated the exact-match contract.
3. **Clear test organization**: Grouping tests by success/failure/edge cases with descriptive comments made the test intent immediately clear.
4. **Focus on contract validation over implementation**: Tests focused on the "exact match" requirement rather than implementation details, which is appropriate for a function that delegates to well-tested library code.

This validates that the established testing pattern (comprehensive coverage, clear organization, contract-focused validation) is effective for this type of function.

## TASK No.5: Unit test for MergeRuns

### description

Add test category for `MergeRuns` to `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`.

**Test Category 5: MergeRuns**

1. **Basic merging**:
   - Merge empty maps (result is empty)
   - Merge with only text runs (result contains all text runs)
   - Merge with only inline objects (result contains all inline objects)
   - Merge with both types, no overlap (result contains all runs)

2. **Inline object priority** (inline objects always win):
   - Text run completely overlaps inline object → inline object kept, text run removed
   - Text run partially overlaps inline object from left → text run split, non-overlapping part kept, overlapping part removed
   - Text run partially overlaps inline object from right → text run split, non-overlapping part kept, overlapping part removed
   - One text run overlaps multiple inline objects → text run fragmented into gaps between inline objects
   - Multiple text runs overlap one inline object → all text runs cut, inline object kept

3. **Complex scenarios**:
   - Text runs with gaps, inline objects fill some gaps → interleaved result
   - Text runs that would merge (identical properties) but are separated by inline object → remain separate
   - Multiple overlaps in single merge operation

### what to be done

1. Add a new `TEST_CATEGORY(L"MergeRuns")` block to the existing test file
2. Create test cases for basic merging:
   - Create separate `DocumentTextRunPropertyMap` and `DocumentInlineObjectRunPropertyMap`
   - Call `MergeRuns` to produce `DocumentRunPropertyMap` result
   - Use `AssertMap` to verify result contains expected keys and values
3. Create test cases for inline object priority:
   - Set up text and inline maps with various overlapping scenarios
   - Verify inline objects appear complete in result
   - Verify text runs are properly split/removed where they overlap inline objects
4. Create test cases for complex scenarios
5. Need to extend helper functions:
   - Create `AssertMap` overload for `DocumentRunPropertyMap` (which can contain either text or inline objects)
   - Create `FormatRunProperty` overload for `DocumentRunProperty`
   - Create `CompareRunProperty` overload for `DocumentRunProperty`
6. **IMPORTANT**: Use designated initializers for CaretRange: `{.caretBegin = x, .caretEnd = y}`

### how to test it

Run the compiled unit test executable. The test cases validate:
1. MergeRuns correctly combines two separate maps
2. Inline object priority is always enforced (they cannot be split or removed)
3. Text runs are correctly split when overlapping inline objects
4. The result map is correctly typed as `DocumentRunPropertyMap`
5. Complex multi-overlap scenarios are handled correctly

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

MergeRuns implements the critical priority system where inline objects (atomic UI elements) take precedence over text formatting. This function is used when building the final paragraph state from separate text and inline object tracking. The splitting logic must be precise - any error could result in text runs overlapping inline objects in the protocol, causing rendering corruption. Testing ensures the state machine correctly handles gaps, overlaps, and boundary conditions.

**Note from Task 2 learning**: The MergeRuns implementation uses a state machine with `hasCurrentText` flag to track partial text runs being built. Be careful not to hold references (`auto&&`) to keys when iterating and potentially modifying collections - always use value copies to avoid reference invalidation issues similar to those found in AddTextRun.

**Note from Task 4 learning**: Follow the comprehensive testing pattern validated in Task 4: organize tests by success/failure/edge cases with clear comments, focus on contract validation (inline object priority), and ensure thorough coverage of boundary conditions. This pattern proved effective for testing functions with clear semantic contracts.

## TASK No.6: Unit test for DiffRuns

### description

Add test category for `DiffRuns` to `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`.

**Test Category 6: DiffRuns**

1. **Empty and simple diffs**:
   - Both maps empty (no diff)
   - Old map empty, new map has runs (all in `runsDiff`, inline objects in `createdInlineObjects`)
   - New map empty, old map has runs (all in `runsDiff` as removals, inline objects in `removedInlineObjects`)

2. **Changes detection**:
   - Same key, same value → no entry in diff
   - Same key, different value → entry in `runsDiff`
   - Key only in new map → entry in `runsDiff` (addition)
   - Key only in old map → entry in `runsDiff` (removal)

3. **Inline object tracking**:
   - Inline object added → in both `runsDiff` and `createdInlineObjects`
   - Inline object removed → in both `runsDiff` and `removedInlineObjects`
   - Inline object unchanged (same key, same callbackId) → not in any diff list
   - Inline object moved (different key, same or different callbackId) → old in `removedInlineObjects`, new in `createdInlineObjects`

4. **Key difference handling** (CRITICAL - most important test scenarios):
   - Old run [0,10] → New runs [0,5] and [5,10] with different properties
     - Result should have [0,5] and [5,10] as complete entries in `runsDiff`
   - Old runs [0,5] and [5,10] → New run [0,10] with changed property
     - Result should have [0,10] as complete entry in `runsDiff`
   - Old run [0,15] → New runs [0,5], [5,10], [10,15] where only middle changed
     - Result should have all three new ranges in `runsDiff` if properties differ
   - Mixed: some keys unchanged, some split, some merged in same diff
     - Verify result uses only new keys for changed ranges

5. **Complex scenarios**:
   - Multiple simultaneous changes with key transformations
   - Mix of text runs and inline objects changing
   - Inline objects added/removed while text runs split/merge

### what to be done

1. Add a new `TEST_CATEGORY(L"DiffRuns")` block to the existing test file
2. Create additional helper functions for diff verification:
   - `FormatDocumentRun`: Convert `DocumentRun` to readable string
   - `PrintDiffArray`: Print array of `Pair<CaretRange, DocumentRun>`
   - `AssertDiffArray`: Compare actual and expected diff arrays
   - `PrintCallbackIdArray`: Print arrays of callback IDs
   - `AssertCallbackIdArray`: Compare callback ID arrays
3. Create test cases for all scenarios:
   - Set up old and new `DocumentRunPropertyMap`
   - Create `ElementDesc_DocumentParagraph` with empty arrays
   - Call `DiffRuns` to populate the descriptor
   - Verify `runsDiff`, `createdInlineObjects`, and `removedInlineObjects` using assertion helpers
4. Focus heavily on key difference scenarios - this is critical for protocol correctness
5. **IMPORTANT**: Use designated initializers for CaretRange: `{.caretBegin = x, .caretEnd = y}`

### how to test it

Run the compiled unit test executable. The test cases validate:
1. DiffRuns correctly identifies added, removed, and changed runs
2. Inline object callback IDs are correctly tracked in separate arrays
3. **CRITICAL**: Result always uses complete keys from `newRuns`, never partial or mixed keys
4. The two-pointer algorithm efficiently handles all key transformation scenarios
5. Unchanged runs (same key, same value) are correctly omitted from diff

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

DiffRuns is the most critical function for protocol correctness. It computes the difference between two paragraph states for sending incremental updates over the network. The key-handling requirement is crucial: when old runs are split or merged in new state, the diff must represent the NEW state with NEW keys. Any bug here causes synchronization failures between client and server, corrupting the remote paragraph state. The separate inline object tracking enables proper lifecycle management (creation/destruction callbacks). This function must be tested exhaustively with focus on key transformation scenarios.

**Note from Task 2 learning**: Be careful when iterating through both old and new maps simultaneously in the two-pointer algorithm. Avoid holding references to container keys/values when they might be accessed after potential container modifications. The DiffRuns implementation already uses value copies correctly, which is good practice.

**Note from Task 4 learning**: Apply the comprehensive testing pattern: organize by functional categories (empty diffs, change detection, inline object tracking, key transformations, complex scenarios), use clear comments, and thoroughly test boundary conditions. The success of Task 4 validates this approach for functions with well-defined contracts.

**Learning from Task 6 execution**: When planning comprehensive test suites for protocol code:
1. **Start with basic tests first**: Implement 5-10 basic test cases covering fundamental scenarios (empty inputs, simple additions/removals, basic changes) before attempting comprehensive edge case coverage.
2. **Verify protocol types early**: Read the actual protocol schema header file to confirm data structure types. Protocol code often uses wrapper types (`Ptr<List<>>` instead of `Array<>`) or different representations than initially assumed.
3. **Test incrementally**: Get basic tests compiling and passing before expanding to comprehensive coverage. This validates understanding of the protocol API and catches type mismatches early.
4. **Adjust scope based on complexity**: If protocol types are more complex than expected, reduce initial test scope rather than struggling with incorrect assumptions. Comprehensive coverage can be added in follow-up tasks once the basic patterns are established.
5. **Focus on critical contracts**: For protocol code, the most important tests are those that validate the protocol contract (message format, field population, lifecycle tracking), not implementation details.

This approach proved successful in Task 6, where the original 20-test-case plan was appropriately reduced to 6 basic tests that successfully verified the core protocol behavior.

## TASK No.7: Implement `GuiRemoteGraphicsParagraph` class

### description

Implement all methods of `GuiRemoteGraphicsParagraph` class in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`.

**Learning from Task 6**: When working with complex protocol types, start with basic functionality first rather than trying to implement everything comprehensively. The protocol schema may use different types than initially expected (e.g., `Ptr<List<>>` instead of `Array<>`, structs instead of variants). Verify types by reading the protocol schema carefully before implementation.

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

## TASK No.8: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.h`

### description

Implement the document-related protocol handler methods in `UnitTestRemoteProtocol_Rendering` template class in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`.

**Learning from Task 6**: Protocol implementation should start simple and incrementally add complexity. When protocol data structures are complex, verify the actual types used in the schema before writing extensive code. Test basic message flow first before implementing all edge cases.

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

## TASK No.9: Create basic `GuiSinglelineTextBox` test case

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

## TASK No.10: Create typing simulation test case and complete typing helper functions

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

**Additional Topic from Task 6 Learning**:

#### Topic: Testing Strategy for Protocol Code

Description: Protocol code requires a different testing approach than regular application code due to type complexity and message-based communication patterns.

Key principles learned from Task 6:
- Start with 5-10 basic tests covering fundamental scenarios before expanding to comprehensive edge cases
- Always verify actual protocol types in the schema header before writing tests (wrapper types, pointer semantics, struct vs variant)
- Test incrementally: get basic tests compiling and passing first to validate API understanding
- Adjust scope based on complexity rather than forcing through incorrect assumptions
- Focus on protocol contracts (message format, field population, lifecycle) over implementation details
- Comprehensive test coverage can be added incrementally in follow-up work once basic patterns are validated

Bullet points:
- Protocol types often differ from intuitive expectations (e.g., `Ptr<List<>>` vs `Array<>`)
- Basic test validation is more valuable than extensive tests based on wrong assumptions
- Incremental approach catches type mismatches early and cheaply
- Protocol contracts are the critical test focus, not exhaustive edge cases
- Scope reduction is a valid engineering decision when facing unexpected complexity

This testing philosophy prevents wasted effort on incorrect implementations and establishes a solid foundation for future expansion.

# !!!FINISHED!!!
