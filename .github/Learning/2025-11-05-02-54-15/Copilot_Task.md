# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# UPDATES

# INSIGHTS AND REASONING

## Analysis of MergeRuns Implementation

After examining the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp` (lines 206-290), I've identified the following key characteristics:

### Algorithm Overview

The `MergeRuns` function implements a **two-pointer merge algorithm** with a state machine that processes both input maps (text runs and inline object runs) in sorted order:

1. **State Machine Variables**:
   - `textIdx` and `inlineIdx`: Pointers for iterating through both input maps
   - `hasCurrentText`: Flag indicating whether we're building a partial text run
   - `currentTextRange` and `currentTextProperty`: The current text run being processed (may be partial)

2. **Core Invariant**: Inline objects **always have priority** over text runs. When they overlap, text runs are split and only non-overlapping portions are preserved.

3. **Processing Logic**:
   - The algorithm continuously loops while there are unprocessed items in either map OR there's a partial text run pending
   - Text runs can be split across multiple iterations when they overlap inline objects
   - Inline objects are always added complete (atomic)
   - The result map maintains sorted order by leveraging Dictionary's auto-sorting

### Critical Implementation Details

**Issue Found in Original Implementation (line 235-239)**:
```cpp
if (inlineIdx < inlineKeys.Count())
{
    auto&& inlineKey = inlineKeys[inlineIdx];
    remoteprotocol::DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
    result.Add(inlineKey, runProp);
    inlineIdx++;
    continue;
}
```

This block appears **inside the main loop but before proper overlap detection**. This is problematic because:
- It processes inline objects without checking if there's a pending text run (`hasCurrentText`) 
- The logic seems to add inline objects unconditionally when they exist

**Corrected Understanding**: Looking more carefully at the full algorithm (lines 223-289), the implementation actually has proper logic:
- Lines 225-231: Load next text run if needed
- Lines 233-240: Handle case when no more inline objects (just output text)
- Lines 242-248: Handle case when text exists but no more inline objects
- Lines 250-288: Handle the overlap detection and splitting logic

The algorithm correctly handles all cases through careful conditionals.

### Test Strategy Design

Based on the implementation analysis, tests must cover:

## Test Category 1: Basic Merging (No Overlap)

**Purpose**: Verify the basic two-pointer merge works correctly when there's no conflict between text and inline objects.

**Test Cases**:
1. **Both maps empty** → Result is empty
   - Validates base case, loop termination
   
2. **Only text runs** → Result contains all text runs
   - Validates text-only path, ensures `DocumentRunProperty` variant correctly holds text
   
3. **Only inline objects** → Result contains all inline objects  
   - Validates inline-only path, ensures variant correctly holds inline objects
   
4. **Both types, no overlap** → Result contains all runs in sorted order
   - Validates interleaving works correctly
   - Example: Text [0,10], Inline [20,30], Text [40,50] → All three in result

## Test Category 2: Inline Object Priority (Overlap Scenarios)

**Purpose**: Verify that when text and inline objects overlap, inline objects win and text is correctly split.

**Critical Test Cases**:

1. **Text run completely overlaps inline object**
   - Text [0,30], Inline [10,20] → Result: Inline [10,20] only (text removed)
   - Validates that inline objects completely override text in their range

2. **Text run partially overlaps from left**
   - Text [0,25], Inline [20,30] → Result: Text [0,20], Inline [20,30]
   - Validates left split logic (`beforeRange` in line 268-272)

3. **Text run partially overlaps from right**
   - Text [20,40], Inline [10,25] → Result: Inline [10,25], Text [25,40]
   - Validates the state machine's ability to update `currentTextRange.caretBegin` (line 281)

4. **One text run overlaps multiple inline objects** (CRITICAL)
   - Text [0,50], Inline [10,15], Inline [20,25], Inline [40,45]
   - Expected: Text [0,10], Inline [10,15], Text [15,20], Inline [20,25], Text [25,40], Inline [40,45], Text [45,50]
   - This tests the state machine's ability to process one text run across multiple iterations
   - Validates `hasCurrentText` flag and `currentTextRange` mutation

5. **Multiple text runs overlap one inline object**
   - Text [0,15], Text [25,40], Inline [10,30]
   - Expected: Text [0,10], Inline [10,30], Text [30,40]
   - Validates that the algorithm correctly handles multiple text runs being cut by the same inline object

## Test Category 3: Complex Scenarios

**Purpose**: Test real-world combinations and edge cases.

**Test Cases**:

1. **Interleaved runs with gaps**
   - Text [0,10], Inline [15,20], Text [25,35], Inline [40,45]
   - Result: All four runs preserved as-is
   - Validates gap handling

2. **Text runs that would merge but separated by inline object**
   - Text [0,10] with prop1, Inline [10,20], Text [20,30] with prop1
   - Expected: Text [0,10] prop1, Inline [10,20], Text [20,30] prop1 (NOT merged)
   - **Rationale**: MergeRuns doesn't perform merging—that's done by AddTextRun. This validates separation is preserved.

3. **Multiple overlaps in single merge**
   - Complex combination of cases 1-5 above in a single merge operation
   - Validates overall algorithm robustness

## Required Helper Functions

Since `DocumentRunProperty` is a **union type** (variant), we need special handling:

### 1. `FormatRunProperty(const DocumentRunProperty& prop) -> WString`

```cpp
WString FormatRunProperty(const DocumentRunProperty& prop)
{
    if (auto textProp = prop.TryGet<DocumentTextRunProperty>())
    {
        return FormatRunProperty(*textProp);  // Reuse existing function
    }
    else
    {
        auto inlineProp = prop.Get<DocumentInlineObjectRunProperty>();
        return FormatRunProperty(inlineProp);  // Reuse existing function
    }
}
```

### 2. `CompareRunProperty(const DocumentRunProperty& a, const DocumentRunProperty& b) -> bool`

```cpp
bool CompareRunProperty(const DocumentRunProperty& a, const DocumentRunProperty& b)
{
    if (a.Index() != b.Index())
        return false;  // Different variant types
    
    if (auto textA = a.TryGet<DocumentTextRunProperty>())
    {
        auto textB = b.Get<DocumentTextRunProperty>();
        return CompareRunProperty(*textA, textB);  // Reuse existing
    }
    else
    {
        auto inlineA = a.Get<DocumentInlineObjectRunProperty>();
        auto inlineB = b.Get<DocumentInlineObjectRunProperty>();
        return CompareRunProperty(inlineA, inlineB);  // Reuse existing
    }
}
```

### 3. `AssertMap` overload for `DocumentRunPropertyMap`

The existing `AssertMap` template should work, but we need to ensure the comparison uses the `DocumentRunProperty` overload. The template approach should automatically select the right `CompareRunProperty` overload based on the map value type.

## Test Organization Structure

```cpp
TEST_CATEGORY(L"MergeRuns")
{
    // === BASIC MERGING ===
    TEST_CASE(L"Empty maps") { ... }
    TEST_CASE(L"Only text runs") { ... }
    TEST_CASE(L"Only inline objects") { ... }
    TEST_CASE(L"Both types, no overlap") { ... }
    
    // === INLINE OBJECT PRIORITY ===
    TEST_CASE(L"Text completely overlaps inline - text removed") { ... }
    TEST_CASE(L"Text partial overlap from left - text split") { ... }
    TEST_CASE(L"Text partial overlap from right - text split") { ... }
    TEST_CASE(L"One text run overlaps multiple inline objects - fragmentation") { ... }
    TEST_CASE(L"Multiple text runs overlap one inline object") { ... }
    
    // === COMPLEX SCENARIOS ===
    TEST_CASE(L"Interleaved runs with gaps") { ... }
    TEST_CASE(L"Adjacent text runs separated by inline - no merge") { ... }
    TEST_CASE(L"Multiple simultaneous overlaps") { ... }
}
```

## Critical Validation Points

Each test case must verify:

1. **Correct count**: Result map has expected number of entries
2. **Correct keys**: All expected `CaretRange` keys are present
3. **Correct values**: Each run has the correct property (text or inline)
4. **Correct order**: Runs are in sorted order (guaranteed by Dictionary)
5. **No extra entries**: No unexpected runs in the result
6. **Variant type**: Each `DocumentRunProperty` holds the correct variant (text vs inline)

## Alignment with Task Requirements

The task description specifies:

✓ **Basic merging** - Covered by Test Category 1
✓ **Inline object priority** - Covered by Test Category 2 with comprehensive overlap scenarios
✓ **Complex scenarios** - Covered by Test Category 3
✓ **Helper functions** - Designed and specified above
✓ **Designated initializers** - Will use `{.caretBegin = x, .caretEnd = y}` throughout

## Rationale for Test Prioritization

The most critical tests are:
1. **One text run overlaps multiple inline objects** - This stresses the state machine's ability to maintain `hasCurrentText` and mutate `currentTextRange` across iterations
2. **Text partial overlap from right** - This tests the `currentTextRange.caretBegin` mutation (line 281 in implementation)
3. **Multiple text runs overlap one inline object** - This validates that the two-pointer algorithm correctly advances through multiple text runs

These scenarios exercise the most complex code paths and are most likely to reveal bugs in the state machine logic.

## Design Quality Assessment

**Strengths of this test design**:
- Comprehensive coverage of all overlap scenarios
- Tests both simple paths (basic merging) and complex paths (state machine)
- Helper functions reuse existing code where possible
- Clear organization by functional category
- Each test has a specific purpose documented

**Potential Issues to Watch**:
- The variant type handling in helper functions must correctly use `TryGet` and `Get` from the protocol schema
- Need to verify the exact API for `DocumentRunProperty` variant operations (might be `Index()`, `TryGet<T>()`, `Get<T>()`)
- Must ensure `AssertMap` template instantiation works correctly with the union type

# !!!FINISHED!!!