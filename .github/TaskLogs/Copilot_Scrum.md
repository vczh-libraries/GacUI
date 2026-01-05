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

#file:GuiUnitTestProtocol_Rendering.h has been splitted into header and cpp files, and sibling files are changed a little bit so that there is no more template in #file:GuiUnitTestProtocol.h

Please review unfinished tasks and fix source references if necessary. Keep finished tasks untouched.

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

## UPDATE

I would like you to add a new task right after Task 6, as the previous implemented RunDiff test coverage is insufficient.

You can make a new TEST_CATEGORY("DiffRuns (Complex)").

It is very common that existing runs are modified. So please prepare test cases to cover like:

text run -> object run

object run -> text run

text run -> text run

object run -> object run

and must include situation that:

caret range changed

prop changed

multiple runs are updated to multiple runs with caret ranges also changed

and whatever you think need to be covered.

## UPDATE

Just to mention that, the result of DiffRuns should contain complete caret range from the "new runs".

For example, we have 3 consecutive text runs: (1-2)prop1 (2-5)prop2 (5-10)prop1

and they are updated to (1-10)prop1

DiffRuns should report (1-10)prop1 instead of (2-5)prop1

## UPDATE

I need to add 2 more tasks before unfinished ones:

Make a copy of DocumentTextRunProperty to DocumentTextRunPropertyOverrides in GuiRemoteGraphics_Document.h, right after `CaretRange`, but change every fields to Nullable<T>. There is a fontProperties field in DocumentTextRunProperty, expand it to fontFamily, size, TextStyle in overrides, turning each font properties to nullable.

TextStyle is used as a set of style switches, not just each member individually.

DocumentTextRunPropertyMap should use overrides too. So MergeRuns are affected. Details below.

Null means keep the original value. AddTextRun will use it as the 3rd parameter but keep the map type unchanged. New rules applies:
- When a property is not defined for a range, a default value applied.
- When AddTextRun, because now null members means "not changing", it is possible that the same fontFamily applies to two text runs with different values. Resulting in still two different values but they are updated to use the new fontFamily. More complicated cases happen. And eventually you still have to merge consecutive identical text runs into a single one just like what has been implemented today.
- When MergeRuns, we requires fontFamily and size must be defined in every fragment of text run properties, others use black or false.

First task:
- Fix AddTextRun
- Fix MergeRuns
- Fix TestRemote_DocumentRunManagement.cpp to fix build breaks and ensure existing test cases work, you don't need to add new tests
- Since current AddTextRun and MergeRuns do not support nullable members, so all test cases define what should happen when there is no unspecified text run properties. Supporting nullable members should not break them.

Second task:
Add new test cases for AddTextRun and MergeRuns to cover new scenario.

## UPDATE

When merging runs at the end of AddTextRun, null is considered as a valid value. So two runs with same property values and same null values are considered the same, two runs with different defined properties are considered not the same.

Null remains "not changing" when applying text run changes or in MergeRuns.

## UPDATE

Split task 9 to two as they test two functions

## UPDATE

The last execution of Task 8 was a disaster. So I would like you to split Task 8 into 3 tasks:

First task:
Make the new override type, but do not apply nullable types yet. So that we can fix and make sure the current test cases work with the new signature

Second task:
We now use nullable properties, treat null like a normal value first, do not implement any overriding features. So that we can fix and make sure the current test cases work with the new signature

Third task:
Implement the overriding feature, but do not touch the test cases, make sure there is no regresssion. In Task 9 and 10 there will be new test cases that cover the new logic.

When you split task 8, you should do this in order:
- Adjust existing task numbers through the document first
- Rewrite task 8 into 3 tasks

## UPDATE

I would like to add more information to Task No.14. In `GuiUnitTestProtocol_Rendering.cpp` line 394 you can see `auto w = (c < 128 ? 0.6 : 1) * size;`. It means that for `GuiSolidLabelElement` remote protocol implementation, characters are categorized into groups. Characters whose UTF-16 code point is smaller than 128, its width is 0.6 times the font size, otherwise the width is the font size. So I want remote protocol implementation of `IGuiGraphicsParagraph` to be implemented in the same way.

And I would like you to add one more task at the end, so that both `GuiSolidLabelElement` and `IGuiGraphicsParagraph` remote protocol implementation in `GuiUnitTestProtocol_Rendering.cpp` should consider surrogate pairs. The current implementation does not take care of them. And keep Task No.14 not taking care of them.

## UPDATE

In Task.17 you don't have to implement surrogate pair detection by yourself, In Vlpp there are function to convert from `WString` to `U32String`. Just convert everything to `U32String` and everything will be just fine. The Regex tools used here also support U32String.

# TASKS

- [x] TASK No.1: Define `CaretRange` struct and run management functions
- [x] TASK No.2: Unit test for CaretRange comparison and AddTextRun
- [x] TASK No.3: Unit test for AddInlineObjectRun
- [x] TASK No.4: Unit test for ResetInlineObjectRun
- [x] TASK No.5: Unit test for MergeRuns
- [x] TASK No.6: Unit test for DiffRuns
- [x] TASK No.7: Unit test for DiffRuns (Complex run modification scenarios)
- [x] TASK No.8: Define DocumentTextRunPropertyOverrides type and update signatures without nullable semantics
- [x] TASK No.9: Implement nullable property storage and comparison (no overriding logic)
- [x] TASK No.10: Implement nullable property overriding logic in AddTextRun and default application in MergeRuns
- [x] TASK No.11: Add comprehensive test cases for nullable property scenarios in AddTextRun
- [x] TASK No.12: Add comprehensive test cases for nullable property scenarios in MergeRuns
- [x] TASK No.13: Implement `GuiRemoteGraphicsParagraph` class
- [x] TASK No.14: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.cpp`
- [x] TASK No.15: Create basic `GuiSinglelineTextBox` test case
- [ ] TASK No.16: Create typing simulation test case and complete typing helper functions
- [ ] TASK No.17: Add surrogate pair support to `GuiSolidLabelElement` and `IGuiGraphicsParagraph` protocol implementations

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

## TASK No.7: Unit test for DiffRuns (Complex run modification scenarios)

### description

Add a new test category `TEST_CATEGORY(L"DiffRuns (Complex)")` to `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` to thoroughly test complex run modification scenarios that occur commonly in real-world document editing.

Task 6 established basic DiffRuns testing with simple addition, removal, and change detection. This task focuses on the more complex scenarios where existing runs are modified in place, which is critical for realistic document editing workflows.

**Test Category 7: DiffRuns (Complex)**

The test scenarios should cover four primary transformation types:

**1. Text run → Inline object run**:
   - Same caret range, text property replaced with inline object
   - Verify `runsDiff` contains the new inline object
   - Verify `createdInlineObjects` contains the new callback ID
   - Different caret range (shifted/resized), text replaced with inline object
   - Multiple text runs replaced by one inline object
   - One text run replaced by multiple inline objects

**2. Inline object run → Text run**:
   - Same caret range, inline object replaced with text property
   - Verify `runsDiff` contains the new text run
   - Verify `removedInlineObjects` contains the old callback ID
   - Different caret range (shifted/resized), inline object replaced with text
   - Multiple inline objects replaced by one text run
   - One inline object replaced by multiple text runs

**3. Text run → Text run (property changes)**:
   - Same caret range, property changed (color, font, size, style)
   - Verify `runsDiff` contains the run with new property
   - Caret range changed, property unchanged (range splitting or merging)
   - Both caret range and property changed simultaneously
   - Multiple consecutive runs with different properties → single merged run with uniform property
   - Single run → multiple runs with different properties (range splitting with property changes)
   - **CRITICAL TEST**: Old runs (1-2)prop1, (2-5)prop2, (5-10)prop1 → New run (1-10)prop1
     - Must report complete new range (1-10) in `runsDiff`, NOT partial range like (2-5)

**4. Inline object run → Inline object run (property or position changes)**:
   - Same caret range, callback ID changed (replacing one inline object with another)
   - Verify old callback ID in `removedInlineObjects`
   - Verify new callback ID in `createdInlineObjects`
   - Same callback ID, caret range changed (inline object moved/resized)
   - Multiple inline objects repositioned (ranges changed but callback IDs same)
   - Inline object replaced at different position

**5. Complex multi-run modification scenarios**:
   - Mixed transformations in single diff (some text→object, some object→text, some text→text)
   - Range merging + property changes (e.g., [0,5] and [5,10] → [0,10] with new property)
   - Range splitting + type changes (e.g., [0,10] text → [0,3] object + [3,7] text + [7,10] object)
   - Overlapping modifications affecting adjacent runs
   - Multiple runs updated with cascading caret range adjustments

**6. Edge cases for run modifications**:
   - Modifying first run in paragraph
   - Modifying last run in paragraph
   - Modifying middle run surrounded by unchanged runs
   - All runs modified simultaneously
   - Partial paragraph updates (some runs unchanged, some modified)

### what to be done

1. Add a new `TEST_CATEGORY(L"DiffRuns (Complex)")` block in `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`
2. Implement test cases for all four primary transformation types (text→object, object→text, text→text, object→object)
3. For each transformation type, test:
   - Same caret range with property/type change
   - Different caret range with property/type change
   - Multiple runs involved in the transformation
4. Implement test cases for complex multi-run modification scenarios
5. Implement test cases for edge cases
6. Each test case should:
   - Set up initial `oldRuns` map with a realistic run configuration
   - Set up modified `newRuns` map representing the transformation
   - Call `DiffRuns` to compute the difference
   - Use `AssertDiffArray` to verify `runsDiff` contains expected entries with correct keys from `newRuns`
   - Use `AssertCallbackIdArray` to verify `createdInlineObjects` and `removedInlineObjects`
7. Use existing helper functions from Task 6
8. **CRITICAL**: Verify that result keys always come from `newRuns`, never from `oldRuns`
   - Specifically test: When oldRuns [(1-2)prop1, (2-5)prop2, (5-10)prop1] → newRuns [(1-10)prop1]
   - The `runsDiff` must contain (1-10), NOT (2-5) or any partial range from oldRuns
9. **IMPORTANT**: Use designated initializers for CaretRange: `{.caretBegin = x, .caretEnd = y}`

### how to test it

Run the compiled unit test executable. The test cases validate:
1. DiffRuns correctly handles in-place run modifications (common in document editing)
2. Type transformations (text↔object) are correctly tracked in all output arrays
3. Property changes on text runs are correctly detected
4. Inline object replacement and repositioning scenarios work correctly
5. Complex multi-run transformations preserve protocol correctness
6. **CRITICAL**: The result always uses complete caret ranges from `newRuns`, never partial or mixed ranges from `oldRuns`
   - Example: oldRuns [(1-2), (2-5), (5-10)] merging to newRuns [(1-10)] must report (1-10) in diff
7. All combinations of range changes + property changes are handled correctly

This comprehensive test coverage ensures DiffRuns works correctly for realistic document editing scenarios, not just basic additions and removals.

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Task 6 established basic DiffRuns functionality with simple scenarios, but real-world document editing primarily involves modifying existing runs rather than adding/removing them. Users change text formatting (bold, color, size), replace text with inline objects (inserting images), and move inline objects around. These modification scenarios have subtle corner cases:

- When a text run becomes an inline object at the same position, both `runsDiff` and `createdInlineObjects` must be updated
- When caret ranges change during type transformation, the protocol must use the NEW ranges consistently
- **CRITICAL**: When multiple runs merge or split with simultaneous property changes, the diff must represent the complete final state using NEW caret ranges
  - Example: When 3 old runs (1-2)prop1, (2-5)prop2, (5-10)prop1 merge into new run (1-10)prop1, the diff must report (1-10), not (2-5) or any partial range from the old state
  - This ensures the remote side receives complete, consistent range information matching the current state

Without comprehensive testing of these scenarios, bugs could cause:
- Inline object lifecycle mismatches (creation callbacks not sent, destruction callbacks double-sent)
- Incorrect text formatting in the remote renderer
- **Protocol synchronization failures** when the remote side receives partial or mixed range information from old and new states
- Rendering corruption due to incomplete caret range updates

This task ensures the DiffRuns implementation handles the full complexity of document editing, not just the simplified scenarios tested in Task 6. The separation into a dedicated task (rather than expanding Task 6) follows the successful pattern established by the Task 2→Task 3-6 split: start with basic functionality, then comprehensively test the complex scenarios once the foundation is validated.

**Learning from Task 6**: Start with 10-15 focused test cases covering the most critical transformation types before attempting exhaustive coverage. Prioritize the most common editing operations (text property changes, text↔object transformations) over rare edge cases. The goal is thorough coverage of realistic scenarios, not theoretical completeness.

## TASK No.8: Define DocumentTextRunPropertyOverrides type and update signatures without nullable semantics

### description

Create the infrastructure for nullable property support by defining the `DocumentTextRunPropertyOverrides` type and updating function signatures, but WITHOUT implementing nullable property semantics yet. This task focuses on making the type system changes while keeping all behavior identical to the current implementation.

**Step 1: Define DocumentTextRunPropertyOverrides**

Create the struct in `GuiRemoteGraphics_Document.h` right after `CaretRange`:

```cpp
struct DocumentTextRunPropertyOverrides
{
	Nullable<Color> textColor;
	Nullable<Color> backgroundColor;
	Nullable<WString> fontFamily;
	Nullable<vint> size;
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};
```

Note: Unlike `DocumentTextRunProperty` which has a `fontProperties` field of type `FontProperties`, the overrides struct expands font properties into individual nullable fields:
- `fontFamily` (from `FontProperties::fontFamily`)
- `size` (from `FontProperties::size`)
- `textStyle` (combines `bold`, `italic`, `underline`, `strikeline` as a single enum flag set)

**Step 2: Update DocumentTextRunPropertyMap type**

Change the map type alias to use overrides:
```cpp
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
```

**Step 3: Update AddTextRun signature**

Change signature:
```cpp
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides);
```

**Step 4: Update MergeRuns signature**

No signature change needed (it already takes `const DocumentTextRunPropertyMap&`).

**Step 5: Implementation approach for this task**

**CRITICAL**: Do NOT implement nullable semantics yet. Instead:

1. **In AddTextRun**: 
   - Keep the exact same splitting and merging logic as before
   - When comparing properties for merging, compare all fields including nullability exactly
   - When applying properties during splits, just copy the property values as-is (null or not)
   - The behavior should be identical to before, just with a different type

2. **In MergeRuns**:
   - Keep the exact same merge logic as before
   - Just treat overrides as a struct with nullable fields
   - Convert to DocumentRunProperty by extracting values from Nullable<> wrappers
   - No special handling for null values yet (that comes in Task No.10)

**Step 6: Update test helper functions**

In `TestRemote_DocumentRunManagement.cpp`:
- Update `CreateTextProp()` to return `DocumentTextRunPropertyOverrides` with all fields defined (non-null)
- Update `CompareRunProperty()` to handle the overrides type
- Update `FormatRunProperty()` to display overrides type
- No changes needed to test logic - all existing tests should pass unchanged

### what to be done

1. Define `DocumentTextRunPropertyOverrides` struct in `GuiRemoteGraphics_Document.h`
2. Update `DocumentTextRunPropertyMap` type alias
3. Update `AddTextRun()` signature to use overrides
4. Update AddTextRun implementation to work with the new type (but no nullable semantics yet)
5. Update MergeRuns implementation to work with the new type (but no nullable semantics yet)
6. Update test helper functions to use the new type
7. Ensure all existing test cases pass without modification

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- Existing AddTextRun tests verify splitting and merging still work correctly
- Existing MergeRuns tests verify inline object priority still works
- Build succeeds (signature changes compile correctly)
- All test categories pass: CaretRange, AddTextRun, AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, DiffRuns

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`
- `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

The previous attempt at Task 8 tried to do too much at once: change types, implement nullable storage, implement nullable semantics, and update tests. This led to confusion and failures.

By splitting into 3 incremental tasks:
1. **Task 8** (this task): Change the types and signatures, but keep behavior identical. This validates that the type system changes compile and don't break existing tests.
2. **Task 9**: Add nullable storage and comparison, treating null as a normal value. This validates the storage mechanism without complex overriding logic.
3. **Task 10**: Implement the actual overriding semantics (null = "keep existing" in AddTextRun, null = "use default" in MergeRuns).

This approach follows the principle of making one type of change at a time, making each step easier to debug and verify.

## TASK No.9: Implement nullable property storage and comparison (no overriding logic)

```cpp
struct DocumentTextRunPropertyOverrides
{
	Nullable<Color> textColor;
	Nullable<Color> backgroundColor;
	Nullable<WString> fontFamily;
	Nullable<vint> size;
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};
```

Note: Unlike `DocumentTextRunProperty` which has a `fontProperties` field of type `FontProperties` (containing `fontFamily`, `size`, `bold`, `italic`, `underline`, `strikeline`, `antialias`, `verticalAntialias`), the overrides struct expands font properties into individual nullable fields:
- `fontFamily` (from `FontProperties::fontFamily`)
- `size` (from `FontProperties::size`)
- `textStyle` (combines `bold`, `italic`, `underline`, `strikeline` as a single enum flag set)

The `textStyle` field uses `IGuiGraphicsParagraph::TextStyle` which is a flags enum combining Bold, Italic, Underline, Strikeline. This treats style as a unified set of switches, not individual nullable booleans.

**Step 2: Update DocumentTextRunPropertyMap type**

Change the map type alias to use overrides:
```cpp
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
```

**Step 3: Update AddTextRun signature and implementation**

Change signature:
```cpp
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides);
```

Update implementation logic:
1. When adding a new run with overrides, split/merge logic remains the same
2. **Nullable property comparison for merging**: When determining if two consecutive runs can merge, compare **all** properties including null values. Two runs are considered identical only if:
   - Properties that are defined (non-null) in both runs have the same value
   - Properties that are null in both runs are both null
   - **CRITICAL**: If property X is null in run A but defined in run B (or vice versa), the runs are NOT identical and cannot merge
3. **Property application during splits**: When an existing run needs to be split:
   - For fragments that will be overwritten by the new run: The new overrides are applied, null values mean "keep the existing value"
   - For fragments that remain unchanged: Keep their existing property values
4. **Example scenario**:
   - Existing map: `[(0,10) with fontFamily="Arial", size=12, color=Black]`
   - Add run: `[(5,15) with fontFamily="Times", color=null]`
   - Result:
     - `[(0,5) fontFamily="Arial", size=12, color=Black]` - unchanged part
     - `[(5,10) fontFamily="Times", size=12, color=Black]` - fontFamily updated, others kept from original
     - `[(10,15) fontFamily="Times", size=<null>, color=<null>]` - new range with only fontFamily defined
5. **Consecutive run merging examples**:
   - `[(0,5) font="Arial", color=null]` + `[(5,10) font="Arial", color=null]` → CAN merge (both have font="Arial" and color=null)
   - `[(0,5) font="Arial", color=null]` + `[(5,10) font="Arial", color=Red]` → CANNOT merge (color differs: null vs Red)
   - `[(0,5) font="Arial", color=Red]` + `[(5,10) font="Arial", color=null]` → CANNOT merge (color differs: Red vs null)
   - `[(0,5) font="Arial", color=Red]` + `[(5,10) font="Arial", color=Red]` → CAN merge (both properties identical)

**Step 4: Update MergeRuns requirements**

MergeRuns expects all text runs to have certain properties fully defined (non-null):
- `fontFamily` must be defined in all text runs
- `size` must be defined in all text runs
- Other properties (textColor, backgroundColor, textStyle) use default values if null:
  - `textColor`: Black `Color(0, 0, 0)`
  - `backgroundColor`: Black `Color(0, 0, 0)`
  - `textStyle`: `(TextStyle)0` (no styles)

Update MergeRuns implementation:
1. When processing text runs, verify `fontFamily` and `size` are non-null
2. Convert `DocumentTextRunPropertyOverrides` to full `DocumentTextRunProperty` by filling defaults for null properties
3. Rest of the merge logic remains unchanged (inline object priority, splitting, etc.)

**Step 5: Update test cases**

Update existing test cases in `TestRemote_DocumentRunManagement.cpp`:
- Since current test cases don't use nullable properties, they define all properties explicitly
- Supporting nullable members should not break them
- Update helper functions:
  - `CreateTextProp()` should create overrides with all properties defined (non-null)
  - `CompareRunProperty()` should handle nullable comparisons correctly
  - `FormatRunProperty()` should display null properties as `<null>`
- All existing TEST_CASE blocks in AddTextRun and MergeRuns categories should continue to pass without modification to their test logic

### what to be done

1. Define `DocumentTextRunPropertyOverrides` struct in `GuiRemoteGraphics_Document.h`
2. Update `DocumentTextRunPropertyMap` type alias to use overrides
3. Modify `AddTextRun()` signature and implementation:
   - Change parameter type to `DocumentTextRunPropertyOverrides`
   - Implement nullable property application during splits (null = keep existing value)
   - Update consecutive run merging to only compare defined (non-null) properties
4. Modify `MergeRuns()` implementation:
   - Add validation that `fontFamily` and `size` are defined in all text runs
   - Convert overrides to full properties by filling defaults for null values
   - Keep rest of merge logic unchanged
5. Update helper functions in test file:
   - `CreateTextProp()` returns overrides with all fields defined
   - `CompareRunProperty()` handles nullable comparisons
   - `FormatRunProperty()` displays null values
6. Verify all existing test cases still pass without modification

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- Existing AddTextRun tests verify splitting and merging still work correctly
- Existing MergeRuns tests verify inline object priority still works
- No new test cases needed in this task (covered in Task No.9)

The key validation points:
1. Non-nullable test cases (all properties defined) behave identically to before
2. Build succeeds (signature changes compile correctly)
3. All test categories pass: CaretRange, AddTextRun, AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, DiffRuns

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`
- `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Current implementation requires specifying all text properties even when only changing one (e.g., changing font family requires re-specifying color, size, style). This creates unnecessary coupling between property changes and forces redundant data specification.

The nullable property design enables:
1. **Partial updates**: Change only the properties you care about (e.g., "make this bold" without knowing/caring about current color)
2. **Composition of formatting**: Multiple operations can layer formatting changes (first set font, then set color, then set bold) without each operation needing complete property context
3. **Realistic API**: Matches how text editors work - format commands typically affect one property dimension at a time

The design choices:
- **TextStyle as unified flag set**: Bold/italic/underline/strikeline are conceptually a single "style" dimension, treated atomically (you set the entire style mask, not individual flags). This matches the `SetStyle()` method in `IGuiGraphicsParagraph`.
- **Mandatory font family and size in MergeRuns**: These are fundamental text properties required for layout calculation. The protocol layer (`ElementDesc_DocumentParagraph`) needs complete `FontProperties`, so MergeRuns enforces these are always defined before protocol conversion.
- **Null means "keep existing"**: During AddTextRun, null values preserve whatever the text already had. This is the most useful semantic for formatting operations.
- **Merging only considers defined properties**: Two runs merge if their defined properties match. This prevents excessive fragmentation from null properties.

This task focuses on implementation without breaking existing tests. Task No.9 will add comprehensive tests for nullable property scenarios.

## TASK No.9: Implement nullable property storage and comparison (no overriding logic)

### description

Now that the type system supports nullable properties (Task No.8), implement nullable property storage and comparison. Treat null as a valid value that can be stored and compared, but do NOT implement any special "overriding" semantics yet.

**Key principle**: In this task, null is just another value. Two runs with `color=null` are considered to have the same color. A run with `color=null` is different from a run with `color=Red`.

**Learning from Task No.8**: The incremental approach (type changes first, then semantics) worked well. Task 9 continues this pattern by adding nullable storage without overriding logic. This maintains the testability and verifiability that made Task 8 successful.

**Step 1: Update AddTextRun implementation**

1. **Property comparison for merging**: When determining if two consecutive runs can merge, compare all properties including null values as regular values:
   - `color=null` equals `color=null` ✓
   - `color=Red` equals `color=Red` ✓
   - `color=null` does NOT equal `color=Red` ✗
   - `color=Red` does NOT equal `color=null` ✗

2. **Property storage during splits**: When splitting runs, just copy property values directly (null or not). No special "apply overrides" logic yet.

**Important**: AddTextRun already handles nullable comparison correctly via the updated `AreEqual` function from Task 8. The `Nullable<T>` operator== compares both null state and values. No changes needed to AddTextRun implementation in this task.

**Step 2: Update MergeRuns implementation**

Update the existing `ConvertToFullProperty` helper function to handle null values:

1. For text runs with null properties, provide default values:
   - `textColor`: if null, use Black `Color(0, 0, 0)`
   - `backgroundColor`: if null, use Black `Color(0, 0, 0)`
   - `textStyle`: if null, use `(TextStyle)0` (no styles)
   - `fontFamily` and `size`: must be non-null (validate with CHECK_ERROR)

2. Implementation approach:
```cpp
remoteprotocol::DocumentTextRunProperty ConvertToFullProperty(const DocumentTextRunPropertyOverrides& propertyOverrides)
{
	remoteprotocol::DocumentTextRunProperty fullProp;
	
	// Required properties - validate non-null
	CHECK_ERROR(propertyOverrides.fontFamily, L"fontFamily must be defined");
	CHECK_ERROR(propertyOverrides.size, L"size must be defined");
	
	fullProp.fontProperties.fontFamily = propertyOverrides.fontFamily.Value();
	fullProp.fontProperties.size = propertyOverrides.size.Value();
	
	// Optional properties - use defaults if null
	fullProp.textColor = propertyOverrides.textColor ? propertyOverrides.textColor.Value() : Color(0, 0, 0);
	fullProp.backgroundColor = propertyOverrides.backgroundColor ? propertyOverrides.backgroundColor.Value() : Color(0, 0, 0);
	
	auto style = propertyOverrides.textStyle ? propertyOverrides.textStyle.Value() : (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
	// ... rest of style conversion unchanged ...
	
	return fullProp;
}
```

**Step 3: Update test helpers**

Update helper functions to handle nullable display:

1. `FormatRunProperty(const DocumentTextRunPropertyOverrides&)`: Display null values as `<null>`
```cpp
WString FormatRunProperty(const DocumentTextRunPropertyOverrides& prop)
{
	WString colorStr = prop.textColor ? 
		(itow(prop.textColor.Value().r) + L"," + itow(prop.textColor.Value().g) + L"," + itow(prop.textColor.Value().b)) :
		L"<null>";
	// ... similar for other fields ...
}
```

2. `CompareRunProperty` already handles nullable comparison correctly via operator==, no changes needed.

**Step 4: Verify existing tests**

All existing tests create runs with all properties defined (non-null via `CreateTextProp`), so they should continue to pass without modification.

### what to be done

1. Update `ConvertToFullProperty` helper function in `GuiRemoteGraphics_Document.cpp`:
   - Add validation for required fields (fontFamily, size)
   - Add default value application for optional fields (textColor, backgroundColor, textStyle)
2. Update `FormatRunProperty` in test file to display null values as `<null>`
3. No changes needed to AddTextRun (already handles nullable comparison)
4. No changes needed to `CompareRunProperty` (already handles nullable comparison)
5. Verify all existing test cases pass

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- AddTextRun tests verify merging considers null as a distinct value (via existing `AreEqual`)
- MergeRuns tests verify default value application and validation
- No new tests needed in this task (covered in Task No.11 and No.12)

**Key validation**: If existing tests pass, it proves:
1. Required field validation works
2. Default value application doesn't break existing behavior
3. The nullable system is ready for overriding logic (Task 10)

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp` (update `ConvertToFullProperty` only)
- `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` (update `FormatRunProperty` only)

### rationale

This task establishes the nullable value system without the complexity of overriding semantics. By treating null as "just another value" and focusing changes on a single helper function (`ConvertToFullProperty`), we:
1. Minimize risk - only one function changes behavior
2. Maintain testability - existing tests validate the changes
3. Validate the nullable storage mechanism works correctly
4. Set up proper validation for required fields (fontFamily, size)

**Learning from Task 8**: The helper function extraction (ConvertToFullProperty) makes this task much simpler. Instead of updating three duplicated code blocks, we update one function. This validates that the refactoring requested by the user improved maintainability.

This creates a stable foundation before adding the more complex overriding logic in Task No.10. If existing tests fail at this stage, it's easier to debug because we're only changing default value handling in one place.

## TASK No.10: Implement nullable property overriding logic in AddTextRun and default application in MergeRuns

### description

Now that nullable properties can be stored and compared (Task No.9), implement the actual overriding semantics: in AddTextRun, null means "keep existing value", and in MergeRuns, defaults are already applied.

**Critical note**: MergeRuns already applies defaults in Task No.9, so no changes needed there. This task only changes AddTextRun.

**Step 1: Update AddTextRun overriding logic**

When adding a new run that overlaps existing runs, the new run's properties should override:
- If a property in the new run is **defined** (non-null): Replace the existing run's property value
- If a property in the new run is **null**: Keep the existing run's property value (don't change it)

**Example scenario**:
- Existing map: `[(0,10) fontFamily="Arial", size=12, color=Black, bgColor=White]`
- Add run: `[(5,15) fontFamily="Times", color=null, bgColor=null, size=null]`
- Result:
  - `[(0,5) fontFamily="Arial", size=12, color=Black, bgColor=White]` - unchanged part
  - `[(5,10) fontFamily="Times", size=12, color=Black, bgColor=White]` - fontFamily updated, others kept
  - `[(10,15) fontFamily="Times", size=<null>, color=<null>, bgColor=<null>]` - new range beyond original

Note: The overriding only applies to existing ranges. For new ranges (like 10-15 above), just use the new run's properties as-is.

**Step 2: Keep merging logic unchanged**

The merging logic from Task No.9 remains:
- Two runs merge if all properties match exactly (including nullability)
- `color=null` can merge with `color=null`
- `color=null` cannot merge with `color=Red`

**Step 3: Implementation approach**

When splitting an existing run because of overlap with a new run:
1. Identify the overlapping fragment
2. For each property in the fragment:
   - If the new run's property is defined: Use the new value
   - If the new run's property is null: Use the existing fragment's value
3. Continue with merging logic as before

### what to be done

1. Update AddTextRun implementation to apply overriding semantics
2. When a new run overlaps existing runs, apply the "null = keep existing" rule
3. Keep merging logic unchanged (from Task No.9)
4. Do NOT modify test cases - they should continue to pass
5. Verify no regression in existing behavior

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- Tests with all properties defined (non-null) should behave identically to Task No.9
- The overriding logic only affects cases where properties are null, which existing tests don't use
- No new tests needed in this task (covered in Task No.11)

Success criteria:
- All existing test categories pass
- Build succeeds
- No regressions in observable behavior for non-null property cases

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

### rationale

This task completes the nullable property implementation by adding the overriding semantics. By deferring this to the third task in the sequence, we ensure:
1. The type system changes are validated (Task No.8)
2. The nullable storage and comparison is validated (Task No.9)
3. Only then do we add the complex overriding logic (this task)

Since existing tests use all-defined properties, the overriding logic won't affect them - they'll continue to pass. The new behavior will be thoroughly tested in Task No.11 and No.12, which add test cases specifically for nullable property scenarios.

This incremental approach makes each step small and verifiable, avoiding the "disaster" of trying to do everything at once.

## TASK No.11: Add comprehensive test cases for nullable property scenarios in AddTextRun

### description

Add new test cases to `TestRemote_DocumentRunManagement.cpp` within the existing `TEST_CATEGORY("AddTextRun")` to thoroughly test the nullable property functionality added in Task No.8, No.9, and No.10.

**Test approach**:
- Add new TEST_CASE blocks within existing TEST_CATEGORY("AddTextRun")
- Use helper function variants that create overrides with some properties null
- Verify nullable property semantics: null = keep existing value in AddTextRun

**New test cases for AddTextRun**:

1. **Partial property updates**:
   - Existing run with all properties defined, add overlapping run with only fontFamily defined → verify only fontFamily changes
   - Existing run, add run with only textColor defined → verify only color changes
   - Existing run, add run with only textStyle defined → verify only style changes
   - Test all property combinations (fontFamily, size, textColor, backgroundColor, textStyle)

2. **Null property merging**:
   - Adjacent runs with identical properties including identical nullability → should merge
   - Example: Run1 [(0,5) fontFamily="Arial", color=null] + Run2 [(5,10) fontFamily="Arial", color=null] → should merge to [(0,10) fontFamily="Arial", color=null]
   - Adjacent runs where a property is null in one but defined in the other → should NOT merge
   - Example: Run1 [(0,5) fontFamily="Arial", color=null] + Run2 [(5,10) fontFamily="Arial", color=Red] → remain separate (null vs Red)
   - Example: Run1 [(0,5) fontFamily="Arial", color=Red] + Run2 [(5,10) fontFamily="Arial", color=null] → remain separate (Red vs null)
   - Adjacent runs with different defined properties → should NOT merge
   - Example: Run1 [(0,5) fontFamily="Arial"] + Run2 [(5,10) fontFamily="Times"] → remain separate

3. **Layered property application**:
   - Start with empty map
   - Add run [(0,10) fontFamily="Arial"]
   - Add run [(0,10) size=12]  → should merge to single run with both properties
   - Add run [(5,10) textColor=Red] → should split: [(0,5) Arial+12], [(5,10) Arial+12+Red]
   - Verify final state has correct property combinations

4. **Split with partial updates**:
   - Existing run [(0,20) with all properties defined]
   - Add run [(5,15) with only fontFamily defined]
   - Verify three fragments: [(0,5) original], [(5,15) updated fontFamily only], [(15,20) original]

5. **Edge cases**:
   - Add run with all properties null → effectively a no-op or only affects range structure
   - Consecutive additions with different nullable combinations
   - Null properties in complex merge scenarios (3+ consecutive runs merging)

**Helper function additions**:

Create new helper functions:
```cpp
// Create overrides with only specified properties defined
DocumentTextRunPropertyOverrides CreateTextPropPartial(
	Nullable<WString> fontFamily,
	Nullable<vint> size,
	Nullable<Color> textColor,
	Nullable<Color> backgroundColor,
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle);

// Create overrides with single property defined
DocumentTextRunPropertyOverrides CreateTextPropWithFont(const WString& fontFamily);
DocumentTextRunPropertyOverrides CreateTextPropWithSize(vint size);
DocumentTextRunPropertyOverrides CreateTextPropWithColor(Color textColor);
DocumentTextRunPropertyOverrides CreateTextPropWithStyle(IGuiGraphicsParagraph::TextStyle style);
```

### what to be done

1. Create helper functions for partial property override creation
2. Add new test cases to TEST_CATEGORY("AddTextRun"):
   - Test partial property updates (5-10 test cases covering different property combinations)
   - Test null property merging rules (3-5 test cases)
   - Test layered property application (2-3 test cases building up complex states)
   - Test split with partial updates (2-3 test cases)
   - Test edge cases (2-3 test cases)
3. Each test case should:
   - Use the new partial helper functions
   - Set up initial state with specific null patterns
   - Perform AddTextRun operation
   - Use AssertMap to verify result matches expected state including nullability
4. Ensure test output clearly shows which properties are null vs defined

### how to test it

Run the compiled unit test executable. The new test cases validate:
1. AddTextRun correctly applies only non-null properties, preserving existing values for null properties
2. AddTextRun merging rules work correctly with nullable properties (merge when all properties including nullability match)
3. Complex combinations of nullable properties work correctly in realistic scenarios
4. The nullable property system enables the intended use case: partial/layered formatting operations

Success criteria:
- All new test cases pass
- Test coverage includes all property combinations
- Edge cases with multiple null properties are handled correctly

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Task No.8, No.9, and No.10 implement the nullable property mechanism for AddTextRun but maintain backward compatibility by only testing with fully-defined properties. This task validates that the nullable property system actually works as intended for AddTextRun's primary use cases:

1. **Partial formatting operations**: Real text editors need to change one property at a time (make bold, change color, etc.) without knowing or specifying all other properties. The nullable system enables this.

2. **Layered formatting**: Multiple formatting operations can compose naturally. First set font, then set color, then set bold - each operation only touches its property dimension.

3. **Merge semantics**: The merging rules need to work correctly with nullable properties. Two runs should merge if their properties match completely, including nullability. Null is treated as a valid value for comparison purposes.

Without comprehensive testing of nullable scenarios for AddTextRun, we risk:
- Subtle bugs in property application (null treated as value instead of "no change")
- Incorrect merging (runs that should merge staying separate due to nullability differences, or vice versa)
- Protocol corruption when layering multiple partial updates

This task focuses specifically on AddTextRun behavior, allowing for thorough testing of this critical function's nullable property handling.

## TASK No.12: Add comprehensive test cases for nullable property scenarios in MergeRuns

### description

Add new test cases to `TestRemote_DocumentRunManagement.cpp` in a new `TEST_CATEGORY("MergeRuns (partial application)")` to thoroughly test the nullable property functionality added in Task No.8, No.9, and No.10.

**Test approach**:
- Add new TEST_CASE blocks in a new TEST_CATEGORY("MergeRuns (partial application)") after the existing TEST_CATEGORY("MergeRuns")
- Use helper functions that create text run maps with nullable properties
- Verify nullable property semantics: null = use default in MergeRuns

**New test cases for MergeRuns**:

1. **Mandatory property validation**:
   - Text run with null fontFamily → should fail assertion or produce error
   - Text run with null size → should fail assertion or produce error
   - Text run with both fontFamily and size defined → should succeed

2. **Default value application**:
   - Text run with null textColor → verify result uses Black (0,0,0)
   - Text run with null backgroundColor → verify result uses Black (0,0,0)
   - Text run with null textStyle → verify result uses (TextStyle)0
   - Text run with all optional properties null → verify result has defaults

3. **Mixing defined and null properties**:
   - Text run [(0,10) fontFamily="Arial", size=12, color=null] → verify result has Black color
   - Inline object run + text run with nulls → verify defaults applied, inline object priority preserved

4. **Complex nullable scenarios**:
   - Multiple text runs with different null patterns merging with inline objects
   - Ensure gap filling and splitting still work correctly with nullable properties
   - Verify inline object priority is preserved regardless of null properties in text runs

### what to be done

1. Add a new TEST_CATEGORY("MergeRuns (partial application)") after the existing TEST_CATEGORY("MergeRuns")
2. Add new test cases to the new category:
   - Test mandatory property validation (2-3 test cases)
   - Test default value application (3-4 test cases)
   - Test mixing defined and null properties (3-5 test cases)
   - Test complex nullable scenarios (2-3 test cases)
3. Each test case should:
   - Create DocumentTextRunPropertyMap with nullable properties
   - Call MergeRuns to produce DocumentRunPropertyMap result
   - Use AssertMap to verify result has correct default values for null properties
   - Verify mandatory properties (fontFamily, size) are validated
4. Test cases should verify that inline object priority and text run splitting work correctly even with nullable properties

### how to test it

Run the compiled unit test executable. The new test cases validate:
1. MergeRuns correctly enforces mandatory properties (fontFamily, size)
2. MergeRuns correctly applies default values for optional null properties
3. Complex combinations of nullable properties work correctly in MergeRuns
4. The nullable property system integrates correctly with inline object priority rules

Success criteria:
- All new test cases pass
- Test coverage includes validation of mandatory properties
- Default value application is correct for all optional properties
- Inline object priority is preserved with nullable text properties

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Task No.8, No.9, and No.10 implement the nullable property mechanism for MergeRuns but maintain backward compatibility by only testing with fully-defined properties. This task validates that MergeRuns correctly handles nullable properties according to its specific requirements:

1. **Protocol requirements**: MergeRuns must ensure fontFamily and size are always defined because the protocol layer needs complete FontProperties. This validation must be tested.

2. **Default value semantics**: Unlike AddTextRun where null means "keep existing", in MergeRuns null means "use default". This different semantic must be thoroughly verified.

3. **Integration with inline objects**: The inline object priority system must work correctly even when text runs have nullable properties. This prevents regressions in the complex merge logic.

Without comprehensive testing of nullable scenarios for MergeRuns, we risk:
- Missing validation allowing incomplete properties to reach the protocol layer
- Incorrect default values being applied
- Breaking inline object priority when text runs have nulls

This task focuses specifically on MergeRuns behavior, complementing Task No.11's focus on AddTextRun. Splitting these into two tasks allows for thorough, focused testing of each function's unique nullable property semantics. The separate test category "MergeRuns (partial application)" matches the organizational pattern established in Task No.11, making it clear that these tests validate nullable property behavior specifically.

## TASK No.13: Implement `GuiRemoteGraphicsParagraph` class

### description

Implement all methods of `GuiRemoteGraphicsParagraph` class in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`.

**Learning from Task 6 and Task 7**: When working with complex protocol types, start with basic functionality first rather than trying to implement everything comprehensively. The protocol schema may use different types than initially expected (e.g., `Ptr<List<>>` instead of `Array<>`, structs instead of variants). Verify types by reading the protocol schema carefully before implementation.

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

### what to be done

1. Add protected members and helper conversion functions to the class
2. Categorize all IGuiGraphicsParagraph methods
3. Implement the lazy update strategy with `needUpdate` flag
4. Implement all Category A methods (property getters/setters)
5. Implement all Category B methods (run manipulation)
6. Implement all Category C methods (query operations)
7. Ensure proper lifecycle management (constructor/destructor)

### how to test it

Testing will be covered in subsequent tasks (Task 12-14) with GacUI test cases. This task focuses on implementation.

### file locations

Implementation: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`
Header updates (if needed): `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`

### rationale

This is the core implementation that bridges `IGuiGraphicsParagraph` interface with the remote protocol. The three-category approach cleanly separates concerns: properties configure the paragraph, run methods modify formatting, query methods interact with the remote side. The lazy update strategy (only send when needed) optimizes protocol traffic. Text position conversion functions, though currently identity functions, provide future extensibility for surrogate pair handling or other text encoding complexities.

## TASK No.14: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.cpp`

### description

Implement the document-related protocol handler methods in `UnitTestRemoteProtocol_Rendering` class in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`.

**Learning from Task 6**: Protocol implementation should start simple and incrementally add complexity. When protocol data structures are complex, verify the actual types used in the schema before writing extensive code. Test basic message flow first before implementing all edge cases.

**Character width calculation**: Follow the same pattern as `GuiSolidLabelElement` in `CalculateSolidLabelSizeIfNecessary` (line 394 of `GuiUnitTestProtocol_Rendering.cpp`). Characters are categorized by their UTF-16 code point:
- Characters with code point < 128: width = 0.6 × font size
- Characters with code point >= 128: width = 1.0 × font size

This simplified approach avoids complex font metrics while providing consistent character width calculations for testing purposes. Do NOT handle surrogate pairs in this task - that will be addressed in Task No.17.

The header file `GuiUnitTestProtocol_Rendering.h` declares the following methods that need implementation in the cpp file. Implement:

**Main rendering handler**:
- `Impl_RendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments)`: 
  - Store paragraph text, wrapLine, maxWidth, alignment
  - Process `runsDiff` array to build internal run representation
  - Calculate paragraph size based on font metrics using the character width formula above
  - Return calculated size

**Caret navigation handlers**:
- `Impl_DocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)`:
  - Implement basic caret movement logic for CaretFirst, CaretLast, CaretLineFirst, CaretLineLast
  - For CaretMoveLeft/Right/Up/Down, calculate new position based on text layout
  - Return `GetCaretResponse` with new caret and preferFrontSide

- `Impl_DocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)`:
  - Calculate bounds rectangle for caret at given position
  - Consider frontSide flag for RTL/complex scripts
  - Return Rect with width=0, positioned at caret

- `Impl_DocumentParagraph_GetCaretFromPoint(vint id, const Point& arguments)`:
  - Perform hit testing to find nearest caret to given point
  - Consider line layout and character positions

**Other query handlers**:
- `Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, const Point& arguments)`:
  - Check if point intersects any inline object
  - Return DocumentRun with inline object info, or null

- `Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)`:
  - Convert text position to nearest valid caret position
  - Consider frontSide preference

- `Impl_DocumentParagraph_IsValidCaret(vint id, const vint& arguments)`:
  - Validate if caret position is within valid range (0 to text.Length())

**Caret display handlers**:
- `Impl_DocumentParagraph_OpenCaret(const OpenCaretRequest& arguments)`:
  - Store caret state for rendering (position, color, frontSide)

- `Impl_DocumentParagraph_CloseCaret()`:
  - Clear stored caret state

**Implementation approach**:

Follow the pattern from `CalculateSolidLabelSizeIfNecessary`:
- Use cached font measurements
- Store paragraph state in `lastElementDescs` or similar structure keyed by `id`
- For complex layout calculations, implement simplified version suitable for testing (exact pixel-perfect layout not required, but consistent behavior is)

### what to be done

1. Implement `Impl_RendererUpdateElement_DocumentParagraph` with paragraph state storage and size calculation
2. Implement all caret navigation handlers (`Impl_DocumentParagraph_GetCaret`, `Impl_DocumentParagraph_GetCaretBounds`, `Impl_DocumentParagraph_GetCaretFromPoint`)
3. Implement query handlers (`Impl_DocumentParagraph_GetInlineObjectFromPoint`, `Impl_DocumentParagraph_GetNearestCaretFromTextPos`, `Impl_DocumentParagraph_IsValidCaret`)
4. Implement caret display handlers (`Impl_DocumentParagraph_OpenCaret`, `Impl_DocumentParagraph_CloseCaret`)
5. Follow existing pattern in `GuiUnitTestProtocol_Rendering.cpp` for state management and font metrics
6. Keep implementation simple but consistent for testing purposes

### how to test it

Testing will be covered in subsequent tasks (Task No.15-16) with GacUI test cases. This task provides the foundation for document control testing.

### file locations

Modified file: `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`
Header file (declarations already present): `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`

### rationale

These handlers are the server-side implementation of the paragraph protocol for the unit test environment. Without these implementations, any test using document controls will fail. The implementations can be simplified compared to production renderers (e.g., Windows/Linux) since they only need to support testing scenarios. However, they must be consistent and functional enough to validate the paragraph protocol and allow document control tests to pass.

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

## TASK No.16: Create typing simulation test case and complete typing helper functions

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

### what to be done

1. Add `TypeText` and `TypeString` methods to `GuiUnitTestProtocol_IOCommands`
2. Implement character-to-virtual-key mapping for common characters
3. Handle shift key state for uppercase letters and special characters
4. Add new test case to existing GuiSinglelineTextBox test file
5. Test basic typing functionality
6. Test special key handling (backspace, delete, etc.)
7. Test interaction between mouse positioning and typing

### how to test it

Run the compiled unit test executable. The test validates:
1. Typing helper functions correctly simulate keyboard input
2. Text is correctly inserted at cursor position
3. Special keys (backspace, delete, arrows) work correctly
4. Selection + typing correctly replaces selected text
5. The complete user interaction pipeline works end-to-end

### file locations

Protocol functions: `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h` and `.cpp`
Test case: `Test\GacUISrc\UnitTest\TestControls_Editor_GuiSinglelineTextBox.cpp` (add to existing file)

### rationale

Typing simulation is essential for testing interactive text editing scenarios. The current unit test framework has mouse and keyboard primitives but no high-level typing functions. Adding these functions enables realistic user interaction testing. This test validates the complete paragraph editing pipeline: user input → text change → rendering update → visual verification. It's the most comprehensive test of the paragraph implementation and will catch issues that only appear during interactive editing.

## TASK No.17: Add surrogate pair support to `GuiSolidLabelElement` and `IGuiGraphicsParagraph` protocol implementations

### description

Update the character width calculation logic in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp` to properly handle UTF-16 surrogate pairs for both `GuiSolidLabelElement` (in `CalculateSolidLabelSizeIfNecessary`) and `IGuiGraphicsParagraph` (in the document paragraph handlers).

**Background**: UTF-16 uses surrogate pairs to represent characters outside the Basic Multilingual Plane (BMP), such as emojis and rare CJK characters. The current implementation iterates character-by-character using `line[i]`, which treats each `wchar_t` as a separate character. This causes surrogate pairs to be counted as two characters with two widths instead of one.

**Simplified Approach using Vlpp**: Instead of manually detecting surrogate pairs, use the existing Vlpp functions `wtou32()` to convert `WString` to `U32String`. In `U32String`, each element is a complete Unicode code point (`char32_t`), so surrogate pairs are automatically handled. The Regex tools in Vlpp also support `U32String`.

**What needs to change**:

1. **Convert to U32String**: When iterating through characters, first convert the `WString` (or line) to `U32String` using `wtou32()`.

2. **Iterate over char32_t**: Instead of iterating over `wchar_t` elements, iterate over `char32_t` elements in the `U32String`. Each element is a complete Unicode code point.

3. **Width calculation**: Apply the same width rule but on `char32_t` values:
   - ASCII characters (code point < 128): width = 0.6 × font size
   - Non-ASCII characters (code point >= 128): width = 1.0 × font size

**Affected code locations in `GuiUnitTestProtocol_Rendering.cpp`**:

1. `CalculateSolidLabelSizeIfNecessary` - wrap line mode (around line 380-420):
```cpp
// Change from:
for (vint i = 0; i < line.Length(); i++)
{
    auto c = line[i];
    auto w = (c < 128 ? 0.6 : 1) * size;
    // ...
}

// To:
auto u32line = wtou32(line);
for (vint i = 0; i < u32line.Length(); i++)
{
    auto c = u32line[i];
    auto w = (c < 128 ? 0.6 : 1) * size;
    // ...
}
```

2. `CalculateSolidLabelSizeIfNecessary` - non-wrap line mode (around line 420-435):
```cpp
// Change from:
textWidth = (vint)(size * From(lines)
    .Select([](const WString& line)
    {
        double sum = 0;
        for (vint i = 0; i < line.Length(); i++)
        {
            auto c = line[i];
            sum += (c < 128 ? 0.6 : 1);
        }
        return sum;
    })
    .Max());

// To:
textWidth = (vint)(size * From(lines)
    .Select([](const WString& line)
    {
        auto u32line = wtou32(line);
        double sum = 0;
        for (vint i = 0; i < u32line.Length(); i++)
        {
            auto c = u32line[i];
            sum += (c < 128 ? 0.6 : 1);
        }
        return sum;
    })
    .Max());
```

3. Document paragraph handler implementations (Task No.14) - all places where character width is calculated should similarly convert to `U32String` first.

**Key advantage**: Using `wtou32()` is simpler and less error-prone than manual surrogate pair detection. The conversion handles all edge cases (incomplete pairs, lone surrogates, etc.) correctly and consistently with how Vlpp handles Unicode throughout the codebase.

### what to be done

1. Update `CalculateSolidLabelSizeIfNecessary` wrap-line mode loop to convert line to `U32String` using `wtou32()` before iterating
2. Update `CalculateSolidLabelSizeIfNecessary` non-wrap-line mode loop similarly
3. Update all document paragraph character iteration in the implementations from Task No.14 to use the same `U32String` conversion pattern
4. Ensure consistent usage of `wtou32()` across all affected code

### how to test it

Testing surrogate pair handling:

1. **Existing tests should continue to pass**: All existing tests use ASCII or BMP characters and should not be affected by the surrogate pair changes.

2. **New test case for GuiSolidLabelElement with emojis**: Create a test that sets `GuiSolidLabelElement` text containing emoji characters (which require surrogate pairs in UTF-16), and verify that:
   - The measured width treats each emoji as a single character with width = font size
   - Text containing "AB🎉CD" (4 ASCII + 1 emoji) should have width roughly = 4 × 0.6 × size + 1 × size

3. **New test case for document paragraph with emojis**: Similar test for `IGuiGraphicsParagraph` rendered via document controls.

4. **Edge cases to test**:
   - Text with only ASCII characters (no change in behavior)
   - Text with only non-ASCII BMP characters (no change in behavior)  
   - Text with surrogate pairs in different positions (start, middle, end)
   - Text with consecutive surrogate pairs (multiple emojis)

### file locations

Modified file: `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`

New test cases can be added to:
- Existing element test files for `GuiSolidLabelElement`
- Test file from Task No.15 for document paragraph testing

### rationale

Using Vlpp's existing `wtou32()` function provides a simple and robust solution for handling surrogate pairs:

1. **Simplicity**: Converting to `U32String` eliminates the need for manual surrogate pair detection logic
2. **Consistency**: Uses the same approach as other parts of Vlpp (e.g., regex tools support `U32String`)
3. **Correctness**: The conversion function handles all edge cases properly, including incomplete or invalid surrogate sequences
4. **Maintainability**: Less custom code means fewer bugs and easier maintenance

The original approach of manual surrogate pair detection would have required:
- Checking for high surrogate (U+D800 to U+DBFF)
- Checking for low surrogate (U+DC00 to U+DFFF)
- Handling edge cases (lone surrogates, incomplete pairs at end of string)
- Maintaining this logic in multiple places

By using `wtou32()`, all this complexity is delegated to a well-tested library function. The only overhead is the string conversion, which is acceptable for the unit test protocol implementation.

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
