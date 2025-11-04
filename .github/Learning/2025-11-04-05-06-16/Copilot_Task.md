# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

## UPDATE

In order to TEST_ASSERT if the result is correct, you want to:
- Make a map by code
- Run a function to compare two maps
- First detect if they equal, if yes, nothing happens
- Otherwise, print both maps using TEST_PRINT and TEST_ASSERT(false)

Refer to `PrintCallbacks` and `AssertCallbacks` in `Test\GacUISrc\UnitTest\TestItemProviders.h`.

## UPDATE

Task 2 has been split into 5 separate tasks (Tasks 2-6) according to `Copilot_Scrum.md`. Task 2 now only covers CaretRange comparison and AddTextRun testing. The following test categories have been moved to separate tasks:
- Task 3: AddInlineObjectRun testing
- Task 4: ResetInlineObjectRun testing
- Task 5: MergeRuns testing
- Task 6: DiffRuns testing

This update removes content for Tasks 3-6 from Task 2 while keeping all relevant content for CaretRange and AddTextRun testing.

# INSIGHTS AND REASONING

## Understanding the Task

This task requires creating unit tests for the first two foundational run management components:

1. **CaretRange** - A struct representing text position ranges with comparison operators
2. **AddTextRun** - Adds text formatting runs with splitting and merging logic

Additional tests for AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, and DiffRuns have been moved to separate tasks (Tasks 3-6) as per the scrum document.

## Analysis of Existing Implementation

From examining `GuiRemoteGraphics_Document.cpp`, I can see:

### CaretRange Structure
- Simple struct with `caretBegin` and `caretEnd` members
- Uses C++20 spaceship operator (`operator<=>`) for automatic comparison operators
- Will order by `caretBegin` first, then by `caretEnd`

### AddTextRun Implementation
- Uses binary search to find overlapping ranges efficiently
- Splits existing runs when new runs overlap
- Merges consecutive runs with identical properties after insertion
- The merging logic checks both left and right neighbors iteratively

### AddInlineObjectRun Implementation
- Uses binary search to detect any overlap
- Returns `false` if any overlap is detected
- Returns `true` on successful addition

### ResetInlineObjectRun Implementation
- Simply calls `map.Remove(range)`
- Returns the boolean result (true if key existed, false otherwise)

### MergeRuns Implementation
- Processes two maps (text and inline objects) in sorted order
- Inline objects have absolute priority - they're added directly to result
- Text runs are split when they overlap inline objects
- Uses a state machine with `hasCurrentText` flag to track partial text runs

### DiffRuns Implementation
- Two-pointer algorithm to traverse both old and new maps
- Tracks inline object callback IDs separately to populate `createdInlineObjects` and `removedInlineObjects`
- When keys match, checks if values changed
- When keys don't match, the earlier key is processed
- Only changed runs appear in `runsDiff`

## Test Design Strategy

### Test Category 1: CaretRange
Need to verify the comparison operators work correctly for Dictionary ordering:
- Equal ranges (same begin and end)
- Ranges differing by `caretBegin` only
- Ranges differing by `caretEnd` only (when `caretBegin` is equal)
- Adjacent ranges (non-overlapping)
- Overlapping ranges

### Test Category 2: AddTextRun
This is the most complex function. Need to test:

**Basic Operations:**
- Empty map insertion
- Non-overlapping insertions
- Adjacent runs with different properties (should NOT merge)
- Adjacent runs with identical properties (SHOULD merge)

**Splitting Scenarios:**
- New run completely replaces old run (same range)
- New run partially overlaps from left
- New run partially overlaps from right
- New run completely contains old run
- New run is contained within old run (splits into 3 parts)
- New run overlaps multiple consecutive runs

**Merging Scenarios:**
- Insertion creates adjacent identical runs (should merge)
- Multiple consecutive identical runs after insertion
- Non-identical adjacent runs (should NOT merge)

**Implementation Approach:**
- Create helper function to verify map contents: checks count and iterates keys
- For each test, set up initial map state, perform operation, verify result
- Need to create `DocumentTextRunProperty` instances with different values for testing

## Test File Structure

```cpp
#include "TestRemote.h"

TEST_FILE
{
    TEST_CATEGORY(L"CaretRange")
    {
        TEST_CASE(L"Comparison operators")
        {
            // Test all comparison scenarios
        }
    }
    
    TEST_CATEGORY(L"AddTextRun")
    {
        TEST_CASE(L"Empty map insertion") { ... }
        TEST_CASE(L"Non-overlapping runs") { ... }
        TEST_CASE(L"Adjacent runs with different properties") { ... }
        TEST_CASE(L"Adjacent runs with identical properties - merge") { ... }
        TEST_CASE(L"Complete overlap - replacement") { ... }
        TEST_CASE(L"Partial overlap from left") { ... }
        TEST_CASE(L"Partial overlap from right") { ... }
        TEST_CASE(L"New run contains old run") { ... }
        TEST_CASE(L"New run contained within old run - splits into 3") { ... }
        TEST_CASE(L"Overlaps multiple runs") { ... }
        TEST_CASE(L"Merging after insertion") { ... }
    }
}
```

Note: Test categories for AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, and DiffRuns will be added in subsequent tasks (Tasks 3-6).

## Helper Functions Needed

1. **CreateTextProp**: Creates a `DocumentTextRunProperty` with specified colors
2. **CreateInlineProp**: Creates a `DocumentInlineObjectRunProperty` with specified parameters (for future tasks)
3. **FormatRunProperty**: Converts properties to readable strings for printing
4. **CompareRunProperty**: Compares property equality
5. **PrintMap**: Prints actual and expected maps for debugging
6. **AssertMap**: Compares maps and fails with detailed output if mismatched

## Property Creation for Testing

Need to create distinguishable properties:

```cpp
remoteprotocol::DocumentTextRunProperty CreateTextProp(vint colorValue)
{
    remoteprotocol::DocumentTextRunProperty prop;
    prop.textColor = Color(colorValue, colorValue, colorValue);
    prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
    prop.fontProperties.fontFamily = L"Test";
    prop.fontProperties.size = 12;
    return prop;
}

remoteprotocol::DocumentInlineObjectRunProperty CreateInlineProp(vint callbackId, vint width = 10)
{
    remoteprotocol::DocumentInlineObjectRunProperty prop;
    prop.size = Size(width, 10);
    prop.baseline = 8;
    prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Follow;
    prop.backgroundElementId = -1;
    prop.callbackId = callbackId;
    return prop;
}
```

## Key Testing Insights for This Task

1. **CaretRange ordering is crucial** - Dictionary depends on it for binary search and proper iteration order
2. **AddTextRun merging logic** - Must verify both directions (left and right neighbor merging)
3. **AddTextRun splitting logic** - Must handle all overlap scenarios correctly
4. **Helper functions are foundational** - They will be reused by all subsequent test tasks (Tasks 3-6)

## Expected Challenges

1. **Creating varied test data** - Need many different property combinations for distinguishable test runs
2. **Verifying complex map states** - Need robust helper functions that follow the pattern from `TestItemProviders.h`
3. **Splitting edge cases** - New run contained within old run creates 3 parts, must verify all parts correctly

## Design Conclusion

This task focuses on the foundational components:
- CaretRange comparison for Dictionary key ordering
- AddTextRun with its complex splitting and merging logic
- Helper functions that establish a consistent testing pattern

The helper functions created here (CreateTextProp, FormatRunProperty, CompareRunProperty, PrintMap, AssertMap) will be reused by all subsequent test tasks (3-6), ensuring consistency across all run management tests.

## Update: Helper Functions for Map Assertion

Based on the pattern in `TestItemProviders.h`, need to create similar helper functions for asserting map contents:

### Pattern Analysis from TestItemProviders.h

The existing code shows a clear pattern:

1. **PrintCallbacks(List<WString>&, const wchar_t* (&)[Count])**
   - Prints "Actual:" followed by the actual list contents
   - Prints "Expected:" followed by the expected array contents
   - Each item is indented with "  " for readability

2. **AssertCallbacks(List<WString>&, const wchar_t* (&)[Count])**
   - First checks if counts match
   - If counts match, checks if each item matches exactly
   - If everything matches, does nothing (assertion passes silently)
   - If there's any mismatch, calls `PrintCallbacks` to show both actual and expected
   - Then calls `TEST_ASSERT(false)` to fail the test

3. **AssertItems(Ptr<NodeItemProvider>, const wchar_t* (&)[Count])**
   - Builds actual items list from the provider
   - Delegates to `AssertCallbacks` for comparison

### Required Helper Functions for Map Testing

Need to create similar functions for `Dictionary<CaretRange, T>` where T is either `DocumentTextRunProperty` or `DocumentInlineObjectRunProperty`:

#### 1. PrintMap Template Function

```cpp
template<typename T>
void PrintMap(
    const Dictionary<CaretRange, T>& actualMap,
    const Dictionary<CaretRange, T>& expectedMap)
{
    TEST_PRINT(L"Actual: (" + itow(actualMap.Count()) + L")");
    for (vint i = 0; i < actualMap.Count(); i++)
    {
        auto key = actualMap.Keys()[i];
        auto value = actualMap.Values()[i];
        TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
    }
    
    TEST_PRINT(L"Expected: (" + itow(expectedMap.Count()) + L")");
    for (vint i = 0; i < expectedMap.Count(); i++)
    {
        auto key = expectedMap.Keys()[i];
        auto value = expectedMap.Values()[i];
        TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
    }
}
```

#### 2. FormatRunProperty Helper Functions

Need to convert property values to readable strings for printing:

```cpp
WString FormatRunProperty(const remoteprotocol::DocumentTextRunProperty& prop)
{
    return L"TextRun(color:" + 
           itow(prop.textColor.r) + L"," + 
           itow(prop.textColor.g) + L"," + 
           itow(prop.textColor.b) + L")";
}

WString FormatRunProperty(const remoteprotocol::DocumentInlineObjectRunProperty& prop)
{
    return L"InlineObject(callback:" + 
           itow(prop.callbackId) + 
           L", size:" + itow(prop.size.x) + L"x" + itow(prop.size.y) + L")";
}
```

#### 3. AssertMap Template Function

```cpp
template<typename T>
void AssertMap(
    const Dictionary<CaretRange, T>& actualMap,
    const Dictionary<CaretRange, T>& expectedMap)
{
    bool matches = (actualMap.Count() == expectedMap.Count());
    
    if (matches)
    {
        for (vint i = 0; i < expectedMap.Count(); i++)
        {
            auto expectedKey = expectedMap.Keys()[i];
            auto expectedValue = expectedMap.Values()[i];
            
            // Check if key exists in actual map
            vint actualIndex = actualMap.Keys().IndexOf(expectedKey);
            if (actualIndex == -1)
            {
                matches = false;
                break;
            }
            
            // Check if value matches
            auto actualValue = actualMap.Values()[actualIndex];
            if (!CompareRunProperty(actualValue, expectedValue))
            {
                matches = false;
                break;
            }
        }
    }
    
    if (!matches)
    {
        PrintMap(actualMap, expectedMap);
    }
    TEST_ASSERT(matches);
}
```

#### 4. CompareRunProperty Helper Functions

Need to compare property values for equality:

```cpp
bool CompareRunProperty(
    const remoteprotocol::DocumentTextRunProperty& a,
    const remoteprotocol::DocumentTextRunProperty& b)
{
    return a.textColor == b.textColor &&
           a.backgroundColor == b.backgroundColor &&
           a.fontProperties.fontFamily == b.fontProperties.fontFamily &&
           a.fontProperties.size == b.fontProperties.size &&
           a.fontProperties.bold == b.fontProperties.bold &&
           a.fontProperties.italic == b.fontProperties.italic &&
           a.fontProperties.underline == b.fontProperties.underline &&
           a.fontProperties.strikeline == b.fontProperties.strikeline &&
           a.fontProperties.antialias == b.fontProperties.antialias &&
           a.fontProperties.verticalAntialias == b.fontProperties.verticalAntialias;
}

bool CompareRunProperty(
    const remoteprotocol::DocumentInlineObjectRunProperty& a,
    const remoteprotocol::DocumentInlineObjectRunProperty& b)
{
    return a.size == b.size &&
           a.baseline == b.baseline &&
           a.breakCondition == b.breakCondition &&
           a.backgroundElementId == b.backgroundElementId &&
           a.callbackId == b.callbackId;
}
```

### Usage in Test Cases

With these helper functions, test cases become much cleaner:

```cpp
TEST_CASE(L"Adjacent runs with identical properties - merge")
{
    Dictionary<CaretRange, remoteprotocol::DocumentTextRunProperty> textMap;
    auto prop1 = CreateTextProp(100);
    
    remoteprotocol::AddTextRun(textMap, {0, 5}, prop1);
    remoteprotocol::AddTextRun(textMap, {5, 10}, prop1);
    
    // Create expected map
    Dictionary<CaretRange, remoteprotocol::DocumentTextRunProperty> expectedMap;
    expectedMap.Add({0, 10}, prop1);
    
    // Assert
    AssertMap(textMap, expectedMap);
}
```

### Benefits of This Approach

1. **Clean Separation**: Test logic is separate from assertion/printing logic
2. **Consistency**: All tests use the same assertion pattern
3. **Debugging**: When tests fail, both actual and expected maps are printed automatically
4. **Readability**: Test cases focus on setup and expected result, not comparison details
5. **Maintainability**: If map printing format needs to change, only one place to update

### Implementation Location

All these helper functions should be placed at the top of `TestRemote_DocumentRunManagement.cpp`, before the `TEST_FILE` block, making them available to all test cases in this file and subsequent test tasks (Tasks 3-6) that will be added to the same file.

# !!!FINISHED!!!
