# !!!TASK!!!

# PROBLEM DESCRIPTION

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
   - Use `AssertDiffList` to verify `runsDiff` contains expected entries with correct keys from `newRuns`
   - Use `AssertCallbackIdList` to verify `createdInlineObjects` and `removedInlineObjects`
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

# UPDATES

# INSIGHTS AND REASONING

## Analysis of the Current State

### Code Review of Existing Implementation

After reviewing the test file `TestRemote_DocumentRunManagement.cpp` and the implementation in `GuiRemoteGraphics_Document.cpp`, I have identified the following:

1. **Task 6 Implementation Status**: The DiffRuns basic tests are already implemented with 6 test cases covering:
   - Empty maps
   - Old map empty with new text/inline runs
   - New map empty with old inline runs
   - Same key comparisons (same/different values)

2. **Helper Functions Available**: The test file has comprehensive helper functions:
   - `CreateTextProp()` and `CreateInlineProp()` for creating test properties
   - `FormatRunProperty()` overloads for both text and inline properties
   - `CompareRunProperty()` overloads for equality checking
   - `AssertMap()` template for map comparison
   - **DocumentRun-specific helpers** already exist:
     - `FormatDocumentRun()` - formats a DocumentRun for debugging
     - `PrintDiffList()` - prints List of DocumentRun
     - `CompareDocumentRun()` - compares two DocumentRun objects
     - `AssertDiffList()` - asserts diff list equality with detailed error messages
     - `PrintCallbackIdList()` - prints callback ID lists
     - `AssertCallbackIdList()` - asserts callback ID list equality

3. **DiffRuns Implementation Analysis**: The `DiffRuns` function in `GuiRemoteGraphics_Document.cpp` uses a two-pointer algorithm:
   - Iterates through both old and new run maps simultaneously
   - Tracks inline object callback IDs in `SortedList<vint>`
   - When keys are equal, checks if values differ - if so, adds to `runsDiff`
   - When keys differ, the run in the map with the smaller key is processed
   - **CRITICAL BEHAVIOR**: The function adds entries to `runsDiff` using the **NEW** key's caret range when there's a change
   - Properly computes `createdInlineObjects` and `removedInlineObjects` by set difference

### Understanding the Critical Requirement

The task description emphasizes a **CRITICAL** requirement:

> **CRITICAL TEST**: Old runs (1-2)prop1, (2-5)prop2, (5-10)prop1 → New run (1-10)prop1
> - Must report complete new range (1-10) in `runsDiff`, NOT partial range like (2-5)

This requirement tests whether `DiffRuns` correctly handles the case where multiple old runs with different keys are replaced by a single new run. The expected behavior is:
- The diff should contain **only** the new key (1-10) in the result
- The diff should **not** contain partial ranges from the old state like (2-5) or mixtures

Looking at the implementation, the two-pointer algorithm processes runs in order:
- When `newKey < oldKey`: New run is added to diff (this is an addition)
- When `oldKey < newKey`: Old run is skipped (this is a removal, tracked via callback IDs if inline object)
- When `oldKey == newKey`: Values are compared, diff entry added if different

**This means the current implementation DOES use new keys exclusively** - it only adds to `runsDiff` when processing new runs (either `newKey < oldKey` or `oldKey == newKey` with different values). The implementation is correct for this requirement.

However, the edge case being tested is more subtle: When old runs [(1-2), (2-5), (5-10)] are merged into new run [(1-10)], the algorithm will process this as:
1. `newKey (1-10) > oldKey (1-2)`: Old (1-2) processed, no diff entry
2. `newKey (1-10) > oldKey (2-5)`: Old (2-5) processed, no diff entry  
3. `newKey (1-10) == oldKey (5-10)`: Keys equal, but values likely differ (merged property), so (1-10) added to diff with new property

**Wait, this reveals a bug!** When `newKey (1-10)` is compared with `oldKey (5-10)`, they are NOT equal. The algorithm will:
1. Process old (1-2): `oldIdx++`
2. Process old (2-5): `oldIdx++`
3. Process old (5-10): `oldIdx++`
4. Process new (1-10): `newIdx++`, adds (1-10) to diff

So the implementation is actually correct - it will add the complete new range (1-10) to the diff.

## Test Design Strategy

### Organizing Test Cases

Following the task description and learning from Task 6 (start with focused tests), I will organize test cases into these categories:

**Category 1: Text→Inline Object Transformations** (4-5 test cases)
- Same range, type change
- Different range (shifted), type change
- Multiple text → one inline
- One text → multiple inline

**Category 2: Inline Object→Text Transformations** (4-5 test cases)
- Same range, type change
- Different range (shifted), type change
- Multiple inline → one text
- One inline → multiple text

**Category 3: Text→Text Property Changes** (5-6 test cases)
- Same range, property changed
- Range changed, property same
- Both range and property changed
- **CRITICAL**: Multiple old runs merge into single new run (the (1-2,2-5,5-10)→(1-10) case)
- Single run splits into multiple with different properties
- Consecutive runs with different props → merged run

**Category 4: Inline Object→Inline Object Changes** (3-4 test cases)
- Same range, callback ID changed
- Range changed, callback ID same
- Multiple inline objects repositioned

**Category 5: Complex Mixed Scenarios** (2-3 test cases)
- Mixed transformations in one diff
- Range merging + type changes
- Range splitting + type changes

**Category 6: Edge Cases** (3-4 test cases)
- Modify first run
- Modify last run
- Modify middle run with unchanged neighbors
- All runs modified

**Total: Approximately 21-27 test cases**

This follows the Task 6 learning: "Start with 10-15 focused test cases" - I'll implement the core transformation types first (15-18 cases from categories 1-4), then add complex scenarios and edge cases if needed.

### Key Testing Insights

1. **Callback ID Tracking**: When testing type transformations (text↔inline), must verify:
   - `runsDiff` contains the changed entry
   - `createdInlineObjects` updated correctly for inline→text or text→inline
   - `removedInlineObjects` updated correctly

2. **Property Comparison**: Use different color values to distinguish properties easily:
   - `CreateTextProp(100)`, `CreateTextProp(150)`, `CreateTextProp(200)` for different text properties
   - `CreateInlineProp(1)`, `CreateInlineProp(2)`, etc. for different inline objects

3. **Range Merging Test**: The critical test case:
   ```cpp
   TEST_CASE(L"Multiple text runs merge to single run with same property")
   {
       DocumentRunPropertyMap oldRuns, newRuns;
       auto prop1 = CreateTextProp(100);
       auto prop2 = CreateTextProp(150);
       
       // Old: three separate runs with different properties
       oldRuns.Add({.caretBegin = 1, .caretEnd = 2}, DocumentRunProperty(prop1));
       oldRuns.Add({.caretBegin = 2, .caretEnd = 5}, DocumentRunProperty(prop2));
       oldRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(prop1));
       
       // New: single merged run with uniform property
       newRuns.Add({.caretBegin = 1, .caretEnd = 10}, DocumentRunProperty(prop1));
       
       remoteprotocol::ElementDesc_DocumentParagraph desc;
       DiffRuns(oldRuns, newRuns, desc);
       
       // CRITICAL: Must report complete new range (1,10), NOT partial like (2,5)
       auto expected = MakeExpectedList();
       AddExpectedRun(expected, 1, 10, DocumentRunProperty(prop1));
       
       AssertDiffList(desc.runsDiff, expected, L"Merged range must use new key (1,10)");
   }
   ```

## Implementation Plan

### Step 1: Add TEST_CATEGORY Block

Add `TEST_CATEGORY(L"DiffRuns (Complex)")` after the existing `TEST_CATEGORY(L"DiffRuns")` block in the test file.

### Step 2: Implement Core Transformation Tests (Categories 1-4)

Implement 15-18 focused test cases covering the four primary transformation types. Each test:
1. Sets up `oldRuns` and `newRuns` maps
2. Calls `DiffRuns(oldRuns, newRuns, desc)`
3. Uses `AssertDiffList()` to verify `desc.runsDiff`
4. Uses `AssertCallbackIdList()` to verify `desc.createdInlineObjects` and `desc.removedInlineObjects`

### Step 3: Add Complex Scenarios (Categories 5-6)

If time permits and basic tests pass, add 6-9 more test cases for complex scenarios and edge cases.

### Step 4: Verify Critical Test Case

Ensure the "three runs merge to one" test case is included and properly validates the new key requirement.

## Design Decisions

### Decision 1: Test Case Count

**Decision**: Start with 15-18 core transformation tests, add complex scenarios incrementally.

**Rationale**: Following Task 6 learning, avoid attempting comprehensive coverage immediately. Validate the basic transformation types first, then expand if the foundation is solid.

### Decision 2: Test Organization

**Decision**: Group tests by transformation type (text→inline, inline→text, etc.) rather than by feature.

**Rationale**: This organization:
- Makes it clear which scenarios are being tested
- Helps identify coverage gaps
- Follows the task description's structure
- Makes debugging easier when a specific transformation type fails

### Decision 3: Helper Function Reuse

**Decision**: Reuse all existing helper functions from Task 6, do not create new ones.

**Rationale**: The existing helpers (`AssertDiffList`, `AssertCallbackIdList`, `MakeExpectedList`, `AddExpectedRun`) are comprehensive and well-tested. Creating new helpers would be redundant and increase maintenance burden.

### Decision 4: Property Distinctiveness

**Decision**: Use easily distinguishable property values:
- Text colors: 0x00, 0x50, 0xA0, 0xFF (well-separated values)
- Inline callback IDs: 1, 2, 3, 4, ... (sequential)
- Inline widths: 10, 20, 30, ... (for visual distinction in debug output)

**Rationale**: Makes test failures easier to diagnose - you can immediately see which property is which in the error output.

## Expected Challenges

### Challenge 1: Type Transformation Tracking

When a text run becomes an inline object or vice versa, three things must be verified:
1. The run appears in `runsDiff` with the new type
2. The callback ID appears in `createdInlineObjects` (if text→inline) or `removedInlineObjects` (if inline→text)
3. Adjacent runs are not affected

**Mitigation**: Create a dedicated helper pattern for type transformation tests that explicitly checks all three aspects.

### Challenge 2: Range Merging Validation

The critical test case (multiple runs → single run) must verify that the diff contains ONLY the new range, not any partial ranges from the old state.

**Mitigation**: The `AssertDiffList` helper already provides detailed error messages showing both expected and actual diffs. If the implementation is incorrect, it will clearly show the wrong ranges in the output.

### Challenge 3: Inline Object vs Text Run Comparison

DocumentRunProperty is a variant type that can hold either DocumentTextRunProperty or DocumentInlineObjectRunProperty. Tests must handle both types correctly.

**Mitigation**: Use the existing `CompareRunProperty()` function which already handles both types via `Index()` check and type-safe access.

## Summary

This design document proposes implementing a comprehensive test category `DiffRuns (Complex)` with 15-27 test cases organized by transformation type. The approach follows the successful pattern from Task 6: start with focused core tests, validate the foundation, then expand to complex scenarios.

The critical requirement (new keys only in diff results) is correctly handled by the current implementation and will be explicitly validated by the "three runs merge to one" test case.

All existing helper functions will be reused, no new infrastructure is needed. The tests will use distinctive property values to make debugging easy.

The implementation is straightforward: each test case sets up old/new run maps, calls DiffRuns, and verifies the three output arrays (runsDiff, createdInlineObjects, removedInlineObjects) using the existing assertion helpers.

# !!!FINISHED!!!
