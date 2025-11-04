# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

# INSIGHTS AND REASONING

## Overview

Task 3 requires implementing comprehensive unit tests for the `AddInlineObjectRun` function. This function is critical for managing inline objects (like embedded images or controls) within document paragraphs. The key characteristic is that inline objects are **atomic** - they cannot be split and cannot overlap with other inline objects.

## Implementation Analysis

### Function Signature and Behavior

```cpp
bool AddInlineObjectRun(
    DocumentInlineObjectRunPropertyMap& map,
    CaretRange range,
    const remoteprotocol::DocumentInlineObjectRunProperty& property)
```

The function returns:
- `true`: Successfully added the inline object (no overlap detected)
- `false`: Failed to add (overlap detected), map remains unchanged

### Implementation Details

From `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

1. **Empty Map Fast Path**: If map is empty, directly adds the new inline object and returns true.

2. **Overlap Detection**: Uses binary search with a custom comparer:
   ```cpp
   auto comparer = [&](const CaretRange& key, const CaretRange& searchRange) -> std::strong_ordering
   {
       if (key.caretEnd <= searchRange.caretBegin)
           return std::strong_ordering::less;
       else if (key.caretBegin >= searchRange.caretEnd)
           return std::strong_ordering::greater;
       else
           return std::strong_ordering::equal;  // ANY overlap
   };
   ```

3. **Critical Logic**: The comparer returns `equal` for ANY overlap scenario:
   - Complete overlap: `[10,20]` overlaps `[10,20]`
   - Partial left: `[5,15]` overlaps `[10,20]`
   - Partial right: `[15,25]` overlaps `[10,20]`
   - Contains: `[5,25]` overlaps `[10,20]`
   - Contained: `[12,18]` overlaps `[10,20]`
   - But NOT adjacent: `[0,10]` does not overlap `[10,20]` (caretEnd == caretBegin is OK)

4. **Simpler than AddTextRun**: Unlike `AddTextRun`, this function doesn't need:
   - Backward scan to find first overlap (any overlap is rejection)
   - Splitting or merging logic (either add or reject)
   - Fragment management

## Test Strategy

### Test Categories

Following the task description, tests are organized into three categories:

#### 1. Success Cases (return true)
- **Add to empty map**: Simplest success scenario
- **Add non-overlapping objects**: Multiple objects with clear gaps
- **Adjacent objects**: Test boundary condition where ranges touch but don't overlap

#### 2. Failure Cases (return false)
- **Complete overlap**: Exact same range
- **Partial left overlap**: New range starts before, ends during existing
- **Partial right overlap**: New range starts during, ends after existing
- **Contains existing**: New range completely encloses existing
- **Contained within existing**: New range completely inside existing

#### 3. Edge Cases
- **Gap fitting**: Multiple objects exist, new one fits perfectly in a gap
- **Multiple adjacent objects**: Chain of touching objects

### Test Structure Pattern

Each test case should follow this structure:

```cpp
TEST_CASE(L"Descriptive Name")
{
    DocumentInlineObjectRunPropertyMap inlineMap;
    
    // Setup: Create initial state
    auto prop1 = CreateInlineProp(callbackId1);
    AddInlineObjectRun(inlineMap, {.caretBegin = x, .caretEnd = y}, prop1);
    
    // Action: Perform the operation being tested
    auto prop2 = CreateInlineProp(callbackId2);
    bool result = AddInlineObjectRun(inlineMap, {.caretBegin = a, .caretEnd = b}, prop2);
    
    // Verification:
    // 1. Check return value
    TEST_ASSERT(result == expectedBoolean);
    
    // 2. Check map state using AssertMap
    DocumentInlineObjectRunPropertyMap expectedMap;
    // ... build expected map ...
    AssertMap(inlineMap, expectedMap);
});
```

### Critical Verification Points

For **success cases**:
1. Return value is `true`
2. Map count increased by 1
3. New entry exists in map with correct range and property
4. All existing entries remain unchanged

For **failure cases**:
1. Return value is `false`
2. Map count unchanged
3. New entry does NOT exist in map
4. All existing entries remain unchanged

## Detailed Test Case Specifications

### Success Cases

**Test 1: Add to empty map**
- Initial: Empty map
- Action: Add `[0, 10]` with callback ID 100
- Expected: Success, map contains one entry `[0, 10]`

**Test 2: Add non-overlapping objects with gap**
- Initial: `[10, 20]` with callback ID 100
- Action: Add `[30, 40]` with callback ID 200
- Expected: Success, map contains two entries

**Test 3: Adjacent objects (boundary test)**
- Initial: `[0, 10]` with callback ID 100
- Action: Add `[10, 20]` with callback ID 200
- Expected: Success (caretEnd == caretBegin is allowed)
- Rationale: Adjacent ranges don't overlap, this is a valid configuration

**Test 4: Multiple non-overlapping objects**
- Initial: `[0, 10]` and `[30, 40]` with callback IDs 100, 200
- Action: Add `[50, 60]` with callback ID 300
- Expected: Success, map contains three entries

### Failure Cases

**Test 5: Complete overlap (exact match)**
- Initial: `[10, 20]` with callback ID 100
- Action: Add `[10, 20]` with callback ID 200
- Expected: Failure, map still contains only original entry

**Test 6: Partial overlap from left**
- Initial: `[20, 30]` with callback ID 100
- Action: Add `[15, 25]` with callback ID 200
- Expected: Failure, new range `[15, 25]` overlaps `[20, 30]`

**Test 7: Partial overlap from right**
- Initial: `[10, 20]` with callback ID 100
- Action: Add `[15, 25]` with callback ID 200
- Expected: Failure, new range `[15, 25]` overlaps `[10, 20]`

**Test 8: New contains existing**
- Initial: `[15, 25]` with callback ID 100
- Action: Add `[10, 30]` with callback ID 200
- Expected: Failure, new range `[10, 30]` completely contains `[15, 25]`

**Test 9: New contained within existing**
- Initial: `[10, 30]` with callback ID 100
- Action: Add `[15, 25]` with callback ID 200
- Expected: Failure, new range `[15, 25]` is completely inside `[10, 30]`

### Edge Cases

**Test 10: Gap fitting with multiple objects**
- Initial: `[0, 10]`, `[20, 30]`, `[40, 50]` with callback IDs 100, 200, 300
- Action: Add `[10, 20]` with callback ID 400
- Expected: Success, new object fits perfectly in gap between first two

**Test 11: Adjacent chain**
- Initial: `[0, 10]` with callback ID 100
- Action: Add `[10, 20]` with callback ID 200
- Action: Add `[20, 30]` with callback ID 300
- Expected: All succeed, creating chain of adjacent objects
- Verification: Map contains three entries covering `[0, 30]` continuously

## Leveraging Existing Infrastructure

The test file `TestRemote_DocumentRunManagement.cpp` already provides:

1. **Helper Functions**:
   - `CreateInlineProp(vint callbackId, vint width = 10)`: Creates test inline objects
   - `FormatRunProperty(const DocumentInlineObjectRunProperty&)`: For debug output
   - `CompareRunProperty(const DocumentInlineObjectRunProperty&, const DocumentInlineObjectRunProperty&)`: For comparison
   - `AssertMap<T>()`: Template function that works with any map type

2. **Test Pattern**: Established pattern from AddTextRun tests provides guidance

3. **Namespace**: Tests go in `remote_document_paragrpah_tests` namespace

## Implementation Approach

### File Modification

Add new `TEST_CATEGORY` block to existing file after the `AddTextRun` category:

```cpp
TEST_CATEGORY(L"AddInlineObjectRun")
{
    // Success cases
    TEST_CASE(L"Add to empty map") { ... }
    TEST_CASE(L"Add non-overlapping objects with gap") { ... }
    TEST_CASE(L"Adjacent objects") { ... }
    TEST_CASE(L"Multiple non-overlapping objects") { ... }
    
    // Failure cases
    TEST_CASE(L"Complete overlap - exact match") { ... }
    TEST_CASE(L"Partial overlap from left") { ... }
    TEST_CASE(L"Partial overlap from right") { ... }
    TEST_CASE(L"New contains existing") { ... }
    TEST_CASE(L"New contained within existing") { ... }
    
    // Edge cases
    TEST_CASE(L"Gap fitting with multiple objects") { ... }
    TEST_CASE(L"Adjacent chain") { ... }
});
```

### Consistent Style

- Use designated initializers: `{.caretBegin = x, .caretEnd = y}`
- Use meaningful callback IDs: 100, 200, 300, etc. (multiples of 100 for clarity)
- Clear test case names describing the scenario
- Comments explaining expected behavior when non-obvious

## Quality Assurance

### Coverage Verification

The test suite should achieve:
- **Branch coverage**: Both return paths (true/false) tested
- **Boundary coverage**: Adjacent ranges tested
- **Overlap coverage**: All overlap scenarios tested
- **State coverage**: Empty, single, multiple entries tested

### Potential Issues to Watch For

1. **Off-by-one errors**: Adjacent ranges should NOT overlap
   - `[0,10]` and `[10,20]` are adjacent, not overlapping
   - Binary search comparer uses `<=` for caretEnd check

2. **Map state consistency**: Failed operations must not modify map
   - Test by capturing initial state and comparing after failure

3. **Property preservation**: When adding succeeds, property must match
   - Use `CompareRunProperty` to verify deep equality

## Expected Test Output

When run, tests should:
- All pass (green)
- Provide clear failure messages if implementation has bugs
- Use `PrintMap` helper to show actual vs expected on failure
- Execute quickly (pure in-memory operations)

## Rationale for Test Design

This test design ensures:

1. **Atomic Behavior Validation**: Inline objects must remain complete and non-overlapping
2. **Protocol Correctness**: The remote protocol depends on this guarantee
3. **Binary Search Verification**: The custom comparer handles all overlap scenarios correctly
4. **Edge Case Coverage**: Adjacent boundaries are correctly handled
5. **Failure Semantics**: Failed operations leave map unchanged (transaction-like behavior)

The comprehensive test coverage prevents bugs that could cause:
- Protocol corruption (overlapping inline objects)
- UI rendering crashes (inline object conflicts)
- Memory corruption (invalid range assumptions)
- Data loss (unintended overwrites)

# !!!FINISHED!!!
