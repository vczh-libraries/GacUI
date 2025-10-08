# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.3: Implement Test Cases for Expand/Collapse Dynamics

This task implements test cases for the dynamic behavior of `tree::NodeItemProvider`: how the visible index mapping changes when nodes are expanded or collapsed.

### what to be done

Implement a test category "ExpandCollapseDynamics" covering these scenarios:

- Test that expanding a collapsed node correctly increases `Count()` and adjusts visible indices of subsequent nodes
- Test that collapsing an expanded node correctly decreases `Count()` and adjusts visible indices
- Test visible index calculations with partially expanded multi-level trees (some branches expanded, others collapsed)
- Test that expanding/collapsing nested nodes correctly updates the entire visible index range
- Verify that `OnItemModified` callbacks from the `IItemProvider` interface are triggered with correct parameters during expand/collapse
- Test expanding a node at the beginning, middle, and end of the visible list
- Test collapsing a node at the beginning, middle, and end of the visible list
- Test multiple expand/collapse operations in sequence and verify indices remain consistent

This task should add approximately 150-200 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build tree structures (2-3 levels) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events from the `NodeItemProvider`
- Record initial `Count()` and node positions
- Use `SetExpanding()` on nodes to test expand/collapse scenarios
- Verify `Count()`, `RequestNode()`, and `CalculateNodeVisibilityIndex()` return updated values after each operation
- Clear callback logs between test phases to isolate specific behaviors
- Use `AssertCallbacks()` to verify correct callback sequences with correct parameters (start, count, newCount)
- **Code Style (from Task 1 & 2 learnings):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering
  - Use `AssertItems` helper with `const wchar_t*` arrays for compact and readable verification of visible node sequences
  - For empty lists, directly assert `Count() == 0` rather than using helper functions

Organize under a `TEST_CATEGORY` block named "ExpandCollapseDynamics".

### rationale

This task verifies the dynamic behavior that makes `NodeItemProvider` useful in real applications:

1. **Real-World Usage**: Tree controls constantly expand and collapse nodes in response to user actions, so this dynamic behavior is critical

2. **Complex State Management**: The implementation in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` shows sophisticated logic in:
   - `OnItemExpanded/OnItemCollapsed()` - updating visible ranges when state changes
   - `InvokeOnItemModified()` - notifying listeners about visible range changes

3. **Event-Driven Architecture**: Controls depend on `OnItemModified` callbacks to know when to refresh, so correct callback parameters are essential

4. **Builds on Task 2**: Task 2 verified basic mapping works; this task verifies it updates correctly with state changes

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` calls `InvokeOnItemModified()` to notify listeners about visible range changes
- Parameters to callbacks are carefully calculated based on the position and size of the expanded/collapsed subtree

This task should come after Task 2 because it builds on the foundation of static index mapping.

# INSIGHTS AND REASONING

## Step 1: Understanding the Context

I examined the existing test file `TestItemProviders_NodeItemProvider.cpp` to understand:

1. **Current State**: Tasks No.1 and No.2 have been completed
   - The test file exists and is properly structured
   - Basic index mapping tests are implemented covering static tree structures
   - The test infrastructure includes `MockItemProviderCallback`, `AssertItems()`, and `AssertCallbacks()` helpers

2. **What's Already Tested**:
   - Empty trees and single node trees
   - Flat tree structures (all collapsed)
   - Single-level expansion
   - Multi-level expansion with mixed states
   - Root node exclusion
   - Bidirectional consistency between indices and nodes
   - Boundary conditions for `RequestNode()`
   - Complex mixed expansion states with `Count()` verification

3. **Code Style Established**:
   - Comments indicate which interface provides each method (e.g., "through IItemProvider interface")
   - Direct method calls without explicit casting
   - Use of `AssertItems()` helper with `const wchar_t*` arrays
   - Clear test structure with setup, action, and verification phases

## Step 2: Analyzing the Implementation

I examined `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` to understand the expand/collapse dynamics:

### Key Methods for Expand/Collapse

1. **`OnItemExpanded(INodeProvider* node)`** (lines 108-115):
   - Called when a node is expanded
   - Calculates the base index of the expanded node using `CalculateNodeVisibilityIndexInternal()`
   - If the node is visible (base != -2), it calculates the new visible count and triggers `InvokeOnItemModified(base + 1, 0, visibility - 1, true)`
   - This means: at position `base + 1`, insert `visibility - 1` items (the node's children)

2. **`OnItemCollapsed(INodeProvider* node)`** (lines 117-129):
   - Called when a node is collapsed
   - Calculates the base index of the collapsed node
   - If the node is visible, it counts all visible children and triggers `InvokeOnItemModified(base + 1, visibility, 0, true)`
   - This means: at position `base + 1`, remove `visibility` items (the node's children)

3. **`CalculateNodeVisibilityIndexInternal(INodeProvider* node)`** (lines 131-169):
   - Returns -1 if node is the root or not found
   - Returns -2 if the node's parent is collapsed (invisible subtree)
   - Otherwise recursively calculates the visible index

### Callback Parameters

The `InvokeOnItemModified(start, count, newCount, itemReferenceUpdated)` method is inherited from `ItemProviderBase`:
- `start`: The starting index where the change occurs
- `count`: The number of items that were at that position before the change
- `newCount`: The number of items that are now at that position
- `itemReferenceUpdated`: Whether item references need to be updated (always `true` for expand/collapse)

### Key Insights

1. When expanding a node at index `i`, children are inserted at `i + 1`
2. When collapsing a node at index `i`, all its visible descendants are removed starting at `i + 1`
3. The node itself remains visible; only its children's visibility changes
4. Operations on invisible nodes (in collapsed subtrees) should NOT trigger callbacks (base == -2 check)

## Step 3: Designing Test Cases

Based on the requirements and implementation analysis, I'll design test cases that progressively cover more complex scenarios:

### Test Case 1: Basic Single Node Expand/Collapse
**Purpose**: Verify the simplest case - expanding and collapsing a single node with children

**Scenario**:
- Create a parent with 2 children, parent initially collapsed
- Expand the parent and verify:
  - `Count()` increases from 1 to 3
  - Children are now visible at indices 1 and 2
  - `OnItemModified(1, 0, 2, true)` is triggered
- Collapse the parent and verify:
  - `Count()` decreases from 3 to 1
  - Children are no longer visible
  - `OnItemModified(1, 2, 0, true)` is triggered

**Rationale**: This is the foundational case that all other dynamic tests build upon.

### Test Case 2: Expand/Collapse at Different Positions
**Purpose**: Verify that expand/collapse works correctly regardless of where the node is in the visible list

**Scenario**:
- Create 3 sibling nodes at root level, each with 2 children
- Test expanding first node (beginning), second node (middle), third node (end)
- Verify that:
  - Indices of subsequent visible nodes shift correctly
  - Callback parameters reflect correct positions
- Test collapsing in the same positions

**Rationale**: Ensures that the index calculation works correctly regardless of the node's position.

### Test Case 3: Nested Expand/Collapse
**Purpose**: Verify that expanding/collapsing nested nodes correctly updates the entire visible range

**Scenario**:
- Create a 3-level tree: parent with child, child with grandchild
- Start all collapsed, expand parent, then expand child
- Verify visible count and indices after each operation
- Collapse in reverse order and verify

**Rationale**: Tests the recursive nature of visible index calculation when dealing with nested structures.

### Test Case 4: Multiple Sequential Operations
**Purpose**: Verify that multiple expand/collapse operations maintain consistency

**Scenario**:
- Create a complex tree with multiple branches
- Perform a sequence of expand/collapse operations
- After each operation, verify:
  - `Count()` is correct
  - All visible nodes have correct indices
  - Bidirectional consistency is maintained

**Rationale**: Ensures that the state remains consistent through multiple operations, which is critical for real-world usage.

### Test Case 5: Partially Expanded Tree Dynamics
**Purpose**: Test expand/collapse in a tree where some branches are already expanded

**Scenario**:
- Create a tree with mixed initial states (some nodes expanded, some collapsed)
- Expand a collapsed node in a partially expanded tree
- Collapse an expanded node in a partially expanded tree
- Verify that only the affected subtree's visible nodes change

**Rationale**: Real applications have complex mixed states, so this tests realistic scenarios.

### Test Case 6: Expand Node with Already-Expanded Children
**Purpose**: Verify that expanding a node correctly shows all descendants, including those in already-expanded subtrees

**Scenario**:
- Create a parent with a child, and the child has a grandchild
- Expand the child while parent is collapsed
- Then expand the parent
- Verify that both child and grandchild become visible (3 total nodes)

**Rationale**: Tests that the expand state of invisible nodes is preserved and correctly reflected when their parent is expanded.

### Test Case 7: Collapse Node with Expanded Descendants
**Purpose**: Verify that collapsing a node hides all descendants, even if they are expanded

**Scenario**:
- Create a 3-level tree all fully expanded
- Collapse the middle level node
- Verify that all descendants disappear from the visible list
- Re-expand and verify they reappear with their original expand state

**Rationale**: Ensures that collapsing removes the entire subtree from visibility, regardless of internal states.

### Test Case 8: Callback Verification During Complex Scenarios
**Purpose**: Verify that callbacks have correct parameters in complex scenarios

**Scenario**:
- Create various tree structures
- Monitor callbacks during expand/collapse
- Verify that `start`, `count`, and `newCount` parameters are always correct

**Rationale**: Controls rely on accurate callback parameters to update their UI efficiently.

## Step 4: Design Considerations

### Code Organization

The test category should be structured as:
```cpp
TEST_CATEGORY(L"ExpandCollapseDynamics")
{
    TEST_CASE(L"BasicSingleNodeExpandCollapse") { ... }
    TEST_CASE(L"ExpandCollapseAtDifferentPositions") { ... }
    TEST_CASE(L"NestedExpandCollapse") { ... }
    TEST_CASE(L"MultipleSequentialOperations") { ... }
    TEST_CASE(L"PartiallyExpandedTreeDynamics") { ... }
    TEST_CASE(L"ExpandNodeWithExpandedChildren") { ... }
    TEST_CASE(L"CollapseNodeWithExpandedDescendants") { ... }
    TEST_CASE(L"CallbackVerificationInComplexScenarios") { ... }
}
```

### Testing Pattern

Each test case should follow this pattern:
1. **Setup**: Create tree structure with `TreeViewItemRootProvider` and `NodeItemProvider`
2. **Attach Callback**: Use `MockItemProviderCallback` to monitor events
3. **Clear Initial Callbacks**: Clear the log after attachment
4. **Perform Action**: Call `SetExpanding()` on target nodes
5. **Verify State**: Check `Count()`, `AssertItems()`, and `CalculateNodeVisibilityIndex()`
6. **Verify Callbacks**: Use `AssertCallbacks()` to check event parameters
7. **Clear Callbacks**: Clear log between different test phases

### Helper Usage

- **`AssertItems()`**: Verify the complete visible node sequence after operations
- **`AssertCallbacks()`**: Verify that callbacks were triggered with correct parameters
- **`CreateTreeViewItem()`**: Build tree structures
- **`MockItemProviderCallback`**: Monitor `OnItemModified` events

### Expected Callback Format

Based on existing tests, callbacks are logged as:
```
"OnItemModified(start=X, count=Y, newCount=Z, itemReferenceUpdated=true/false)"
```

For expand operations: `newCount > count`
For collapse operations: `newCount < count`

## Step 5: Implementation Strategy

### Phase 1: Simple Cases (Test Cases 1-2)
- Implement basic single-node expand/collapse
- Test at different positions (beginning, middle, end)
- Approximately 40-50 lines of code

### Phase 2: Nested Cases (Test Cases 3-4)
- Implement nested expand/collapse
- Test multiple sequential operations
- Approximately 50-60 lines of code

### Phase 3: Complex Cases (Test Cases 5-7)
- Implement partially expanded tree dynamics
- Test preserve/restore of expand states
- Approximately 50-60 lines of code

### Phase 4: Verification (Test Case 8)
- Comprehensive callback verification
- Edge cases for callback parameters
- Approximately 20-30 lines of code

**Total estimated: 160-200 lines**, which aligns with the requirement of approximately 150-200 lines.

## Step 6: Key Implementation Details

### Expand Operation Testing

When testing expansion:
1. Record initial `Count()` - call it `countBefore`
2. Call `node->SetExpanding(true)`
3. Clear callback log if needed
4. Verify new `Count()` - call it `countAfter`
5. Verify `countAfter - countBefore` equals the number of newly visible descendants
6. Verify callback: `OnItemModified(start=nodeIndex+1, count=0, newCount=visibleDescendants, itemReferenceUpdated=true)`
7. Use `AssertItems()` to verify the complete sequence

### Collapse Operation Testing

When testing collapse:
1. Record initial `Count()` - call it `countBefore`
2. Record visible children count before collapse
3. Call `node->SetExpanding(false)`
4. Verify new `Count()` - call it `countAfter`
5. Verify `countBefore - countAfter` equals the number of hidden descendants
6. Verify callback: `OnItemModified(start=nodeIndex+1, count=hiddenDescendants, newCount=0, itemReferenceUpdated=true)`
7. Use `AssertItems()` to verify the complete sequence

### Index Verification Pattern

After each expand/collapse operation:
```cpp
// Verify visible nodes using AssertItems
const wchar_t* expected[] = { L"Node1", L"Child1", L"Node2", ... };
AssertItems(nodeItemProvider, expected);

// Verify individual indices
TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node1.Obj()) == 0);
TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == 1);
// etc.

// Verify bidirectional consistency if needed
for (vint i = 0; i < nodeItemProvider->Count(); i++)
{
    auto node = nodeItemProvider->RequestNode(i);
    TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
}
```

### Callback Verification Pattern

```cpp
// Setup callback logging
List<WString> callbackLog;
MockItemProviderCallback itemCallback(callbackLog);
nodeItemProvider->AttachCallback(&itemCallback);
callbackLog.Clear(); // Clear attachment callback

// Perform operation
node->SetExpanding(true);

// Verify callback
const wchar_t* expectedCallbacks[] = {
    L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
};
AssertCallbacks(callbackLog, expectedCallbacks);
```

## Step 7: Alignment with Requirements

Reviewing the original task requirements:

✅ **Test that expanding a collapsed node correctly increases Count() and adjusts visible indices** - Covered in Test Cases 1, 2, 5
✅ **Test that collapsing an expanded node correctly decreases Count() and adjusts visible indices** - Covered in Test Cases 1, 2, 7
✅ **Test visible index calculations with partially expanded multi-level trees** - Covered in Test Case 5
✅ **Test that expanding/collapsing nested nodes correctly updates the entire visible index range** - Covered in Test Cases 3, 6, 7
✅ **Verify that OnItemModified callbacks are triggered with correct parameters** - Covered in all test cases, especially Test Case 8
✅ **Test expanding a node at the beginning, middle, and end** - Covered in Test Case 2
✅ **Test collapsing a node at the beginning, middle, and end** - Covered in Test Case 2
✅ **Test multiple expand/collapse operations in sequence** - Covered in Test Case 4

All requirements are addressed by the proposed test cases.

## Step 8: Final Design Summary

The design proposes implementing 8 test cases organized under the "ExpandCollapseDynamics" category:

1. **BasicSingleNodeExpandCollapse**: Foundation test for single node operations
2. **ExpandCollapseAtDifferentPositions**: Tests positional independence
3. **NestedExpandCollapse**: Tests multi-level operations
4. **MultipleSequentialOperations**: Tests state consistency over time
5. **PartiallyExpandedTreeDynamics**: Tests realistic mixed-state scenarios
6. **ExpandNodeWithExpandedChildren**: Tests state preservation
7. **CollapseNodeWithExpandedDescendants**: Tests complete subtree hiding
8. **CallbackVerificationInComplexScenarios**: Tests event system accuracy

Each test case follows the established pattern from Task 2:
- Clear setup with `TreeViewItemRootProvider` and `NodeItemProvider`
- Use of `MockItemProviderCallback` for event monitoring
- Verification using `AssertItems()`, `AssertCallbacks()`, and direct assertions
- Comments indicating which interface methods are being tested
- Clean separation between test phases

The implementation leverages the existing helper functions and follows the coding style established in earlier tasks. The total code should be approximately 160-200 lines, meeting the requirement of 150-200 lines.

# !!!FINISHED!!!
