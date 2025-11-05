# !!!TASK!!!

# PROBLEM DESCRIPTION

**TASK No.6: Unit test for DiffRuns**

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

**What to be done**:

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

**How to test it**:

Run the compiled unit test executable. The test cases validate:
1. DiffRuns correctly identifies added, removed, and changed runs
2. Inline object callback IDs are correctly tracked in separate arrays
3. **CRITICAL**: Result always uses complete keys from `newRuns`, never partial or mixed keys
4. The two-pointer algorithm efficiently handles all key transformation scenarios
5. Unchanged runs (same key, same value) are correctly omitted from diff

**File locations**:

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Rationale**:

DiffRuns is the most critical function for protocol correctness. It computes the difference between two paragraph states for sending incremental updates over the network. The key-handling requirement is crucial: when old runs are split or merged in new state, the diff must represent the NEW state with NEW keys. Any bug here causes synchronization failures between client and server, corrupting the remote paragraph state. The separate inline object tracking enables proper lifecycle management (creation/destruction callbacks). This function must be tested exhaustively with focus on key transformation scenarios.

# UPDATES

# INSIGHTS AND REASONING

## Understanding DiffRuns Function Contract

The `DiffRuns` function is the most complex and critical piece of the run management system. Its purpose is to compute the difference between two `DocumentRunPropertyMap` instances to enable incremental protocol updates. Let me analyze the key aspects:

### Core Responsibilities

1. **Compute run differences**: Identify which runs were added, removed, or changed between old and new states
2. **Track inline objects separately**: Maintain separate lists for created and removed inline objects (for lifecycle management callbacks)
3. **Handle key transformations**: When runs are split or merged, the result must use NEW keys exclusively

### Critical Design Constraint: New Keys Only

The most important requirement is that **the result must always use complete keys from `newRuns`**. This is different from typical diff algorithms that might preserve old keys. Consider this scenario:

- Old state: `[0,10] → TextProperty(Red)`
- New state: `[0,5] → TextProperty(Blue)`, `[5,10] → TextProperty(Green)`

A naive diff might output:
- Changed: `[0,10] → null` (removal)
- Added: `[0,5] → TextProperty(Blue)`
- Added: `[5,10] → TextProperty(Green)`

But this violates the protocol contract. The correct output must be:
- Changed: `[0,5] → TextProperty(Blue)` (complete new key)
- Changed: `[5,10] → TextProperty(Green)` (complete new key)

The rationale is that the remote side needs to know the FINAL state structure, not the transformation path. It doesn't care that `[0,10]` used to exist - it only cares about the current structure `[0,5]` and `[5,10]`.

### Algorithm Analysis

The function must implement a two-pointer scan over both sorted maps simultaneously:

1. **Start with pointers** at the beginning of both oldRuns and newRuns
2. **Compare current keys**:
   - If keys are equal → compare values (emit if different, skip if same)
   - If old key < new key → old run was removed (emit removal)
   - If new key < old key → new run was added (emit addition)
3. **Handle overlaps**: Keys might overlap without being equal (e.g., `[0,10]` vs `[0,5]`)
   - Must detect partial overlaps and process accordingly
   - Always emit complete new keys when there's any change in the overlapped region

### Inline Object Tracking

Inline objects need special handling:
- When an inline object is added, it appears in BOTH `runsDiff` (as a run change) AND `createdInlineObjects` (callback ID)
- When an inline object is removed, it appears in BOTH `runsDiff` (as a run removal) AND `removedInlineObjects` (callback ID)
- Unchanged inline objects (same key, same callbackId) don't appear in any list
- "Moved" inline objects (different key) are treated as remove + add

### Testing Strategy

The test suite must cover:

1. **Empty cases**: Establish baseline behavior
2. **Simple changes**: Single-key additions/removals/modifications
3. **Inline object tracking**: Verify separate callback ID lists
4. **Key transformations** (most critical):
   - Split: one old key → multiple new keys
   - Merge: multiple old keys → one new key
   - Partial updates: mix of changed and unchanged regions
5. **Complex scenarios**: Multiple transformations in one diff

### Test Structure Design

Following the successful pattern from previous tasks:

```cpp
TEST_CATEGORY(L"DiffRuns")
{
    // Helper functions for this category
    auto CreateEmptyDesc = []() { 
        ElementDesc_DocumentParagraph desc;
        // Initialize arrays
        return desc;
    };
    
    TEST_CASE(L"Empty maps")
    {
        // Test empty to empty, empty to filled, filled to empty
    });
    
    TEST_CASE(L"Simple changes - additions and removals")
    {
        // Test single key added, removed
    });
    
    TEST_CASE(L"Simple changes - modifications")
    {
        // Test same key, different value
    });
    
    TEST_CASE(L"Inline object tracking - additions")
    {
        // Verify both runsDiff and createdInlineObjects
    });
    
    TEST_CASE(L"Inline object tracking - removals")
    {
        // Verify both runsDiff and removedInlineObjects
    });
    
    TEST_CASE(L"Key transformations - split runs")
    {
        // One old key becomes multiple new keys
        // Critical: result uses ALL new keys
    });
    
    TEST_CASE(L"Key transformations - merge runs")
    {
        // Multiple old keys become one new key
        // Critical: result uses new merged key
    });
    
    TEST_CASE(L"Key transformations - partial overlaps")
    {
        // Complex overlapping scenarios
    });
    
    TEST_CASE(L"Complex scenarios")
    {
        // Mix of text and inline objects changing simultaneously
    });
}
```

### Helper Functions Required

Based on the function signature and output structure, we need:

1. **`FormatDocumentRun(const DocumentRun& run) -> WString`**:
   - Handle both text runs and inline object runs
   - Use variant/discriminated union pattern
   - Format as readable string like "Text(color=#FF0000)" or "InlineObj(callback=5, width=100)"

2. **`PrintDiffArray(const Array<Pair<CaretRange, DocumentRun>>& diff) -> WString`**:
   - Format each pair as "[begin,end) → Run"
   - Collect into multi-line string
   - Similar to PrintMap but for Pair arrays

3. **`AssertDiffArray(UnitTestScreenConfig*, const Array<Pair<CaretRange, DocumentRun>>& actual, const Array<Pair<CaretRange, DocumentRun>>& expected)`**:
   - Compare lengths first
   - Compare each element (both range and run)
   - Use CompareRunProperty for run comparison
   - On mismatch, print both arrays for debugging

4. **`PrintCallbackIdArray(const Array<vint>& ids) -> WString`**:
   - Simple comma-separated list: "1, 2, 5, 7"

5. **`AssertCallbackIdArray(UnitTestScreenConfig*, const Array<vint>& actual, const Array<vint>& expected)`**:
   - Compare as sorted arrays (order might not matter, need to check protocol)
   - Print both on mismatch

### Implementation Considerations

The `DiffRuns` implementation likely uses a state machine or two-pointer algorithm. Key implementation points:

1. **Range overlap detection**: Need helper to determine if two CaretRanges overlap
2. **Efficient iteration**: Leverage sorted Dictionary keys - don't repeatedly search
3. **Value comparison**: Need to compare DocumentRunProperty (which is a variant of text/inline)
4. **Output arrays**: Populate three separate arrays in ElementDesc_DocumentParagraph

### Edge Cases to Test

1. **Identical maps**: No diff should be generated
2. **Non-overlapping changes**: Clean additions/removals
3. **Complete overlap**: Old run completely replaced by new run(s)
4. **Partial overlap**: Old run partially replaced
5. **Multiple overlaps**: One old run overlaps multiple new runs (or vice versa)
6. **Inline object priority**: When inline objects are involved in transformations
7. **Mixed changes**: Some runs unchanged, some changed, in same operation

### Expected Test Count

Based on the comprehensive coverage needed:
- Empty and simple diffs: ~3 test cases
- Changes detection: ~4 test cases
- Inline object tracking: ~4 test cases
- Key transformations: ~5-7 test cases (this is the critical area)
- Complex scenarios: ~2-3 test cases

Total: approximately 18-21 test cases

### Comparison with Previous Tasks

- **Task 2 (AddTextRun)**: Validated splitting and merging of individual runs
- **Task 3 (AddInlineObjectRun)**: Validated overlap detection for additions
- **Task 4 (ResetInlineObjectRun)**: Validated exact key matching for removal
- **Task 5 (MergeRuns)**: Validated combining two maps with inline object priority

**Task 6 (DiffRuns)** builds on all previous tasks - it must handle:
- All the splitting/merging logic from Task 2
- Inline object detection from Tasks 3-4
- Priority rules from Task 5
- PLUS the additional complexity of comparing TWO states and producing incremental changes

This makes it the most comprehensive and critical test suite in the run management system.

### Protocol Context

Understanding why this matters helps guide the tests. In the remote protocol:

1. Client maintains paragraph state (runs, text, formatting)
2. When paragraph changes, client must send update to server
3. Server maintains its own copy of paragraph state
4. **Problem**: Sending entire paragraph state every time is inefficient
5. **Solution**: Send only the difference (runsDiff)

The server applies runsDiff to its local state to stay synchronized. If runsDiff is incorrect (e.g., uses old keys instead of new keys), the server's state becomes corrupted. Future operations (caret positioning, hit testing) will fail because they rely on accurate run information.

This is why the "new keys only" requirement is critical - the server needs to know the final structure, not the transformation history.

### Relationship to ElementDesc_DocumentParagraph

Looking at the protocol schema, `ElementDesc_DocumentParagraph` likely has these members:
- `Array<Pair<CaretRange, DocumentRun>> runsDiff`
- `Array<vint> createdInlineObjects` (callback IDs)
- `Array<vint> removedInlineObjects` (callback IDs)

The `DiffRuns` function populates these three arrays. The protocol message `RendererUpdateElement_DocumentParagraph` contains an ElementDesc_DocumentParagraph and is sent from client to server.

### Design Decision: Why Separate Inline Object Arrays?

Inline objects need lifecycle management:
- When created, server must allocate resources (callback handlers, layout space)
- When removed, server must deallocate resources
- The callback IDs enable the server to track which objects need creation/destruction

Text runs don't have this requirement - they're pure data, no lifecycle. Separating inline object tracking makes the protocol more explicit about lifecycle events.

## Test Implementation Plan

### Phase 1: Create Helper Functions

Implement all helper functions at the top of the test file, following patterns from Task 2-5:

1. `FormatDocumentRun` - must handle variant nature of DocumentRun
2. `PrintDiffArray` - format Pair arrays
3. `AssertDiffArray` - compare with detailed error messages
4. `PrintCallbackIdArray` - simple integer array formatting
5. `AssertCallbackIdArray` - compare integer arrays

### Phase 2: Implement Test Cases by Category

**Category 1: Empty and Simple Diffs**
- Establish baseline behavior
- Quick to implement, provides immediate validation

**Category 2: Changes Detection**
- Test basic add/remove/modify scenarios
- Foundation for understanding diff semantics

**Category 3: Inline Object Tracking**
- Validate separate callback ID arrays
- Ensure protocol lifecycle requirements are met

**Category 4: Key Transformations** (CRITICAL - allocate most time here)
- Test all split/merge scenarios
- Verify "new keys only" requirement
- Most likely place for bugs to hide

**Category 5: Complex Scenarios**
- Integration tests combining multiple changes
- Catch edge cases that don't appear in isolation

### Phase 3: Validation

Run all tests and verify:
1. All test cases pass
2. Test coverage is comprehensive
3. Error messages are clear and helpful
4. Test execution time is reasonable

## Potential Implementation Challenges

### Challenge 1: DocumentRun Variant Handling

`DocumentRun` is likely a discriminated union or variant of `DocumentTextRunProperty` and `DocumentInlineObjectRunProperty`. Comparing and formatting requires handling both cases.

**Solution**: Create overloaded helpers for each type, then use type inspection or visitor pattern for DocumentRun.

### Challenge 2: Range Overlap Logic

Detecting and categorizing range overlaps is non-trivial:
- Complete overlap: `[0,10]` and `[0,10]`
- Partial overlap: `[0,10]` and `[5,15]`
- Containment: `[0,10]` and `[3,7]`
- No overlap: `[0,5]` and `[6,10]`

**Solution**: Use helper function to classify relationship between two ranges. May need to look at DiffRuns implementation to understand the exact algorithm.

### Challenge 3: Expected Result Construction

For each test case, must manually construct the expected runsDiff, createdInlineObjects, and removedInlineObjects arrays. This is error-prone.

**Solution**: 
1. Start with simple cases to validate helper functions
2. Use clear naming and comments
3. Consider building expected results programmatically where possible
4. Double-check expected results by hand-tracing the algorithm

### Challenge 4: Test Data Setup

Each test needs:
1. Old DocumentRunPropertyMap
2. New DocumentRunPropertyMap
3. Expected ElementDesc_DocumentParagraph

This is a lot of boilerplate.

**Solution**:
- Use helper functions to build maps concisely
- Consider lambda factories for common patterns
- Keep test cases focused - don't test everything in one case

## Success Criteria

The implementation will be complete and correct when:

1. ✅ All helper functions compile and produce correct output
2. ✅ All test cases in category 1-2 pass (foundation)
3. ✅ All test cases in category 3 pass (inline object tracking)
4. ✅ All test cases in category 4 pass (KEY MILESTONE - critical functionality)
5. ✅ All test cases in category 5 pass (integration)
6. ✅ No test cases are marked as skipped or pending
7. ✅ Error messages are clear and actionable
8. ✅ Test code follows established patterns from Tasks 2-5
9. ✅ Code compiles without warnings
10. ✅ Test execution completes in reasonable time (< 5 seconds for this category)

## Next Steps After This Task

With Task 6 complete, all run management functions will be fully tested. This provides a solid foundation for:

- **Task 7**: Implementing GuiRemoteGraphicsParagraph (can confidently use these functions)
- **Task 8**: Implementing protocol handlers (can rely on correct diff generation)
- **Tasks 9-10**: End-to-end testing (any failures are likely in new code, not run management)

The comprehensive test suite for run management acts as a safety net for the entire paragraph protocol implementation.

# !!!FINISHED!!!