# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.8: Test edge cases for TreeViewItemBindableRootProvider

This task focuses on testing edge cases, error handling, boundary conditions, and cleanup scenarios for TreeViewItemBindableRootProvider to ensure robustness.

### what to be done

- Complete `TestItemProviders_TreeViewItemBindableRootProvider.cpp` with edge case and error handling test cases
- Each test case must create its own callbackLog and callback objects for isolation and robustness
- Clear callback logs after SetItemSource calls (except when testing SetItemSource behavior itself)
- Test childrenProperty with null or empty child collections - empty should only trigger OnAttached
- Use `TEST_ERROR` for property binding with missing or invalid childrenProperty (index out of range)
- Test SetItemSource transitions between different hierarchical structures
- Test expansion state when childrenProperty changes dynamically
- Test callback detachment scenarios during tree structure modifications - verify DetachCallback fires `OnAttached(provider=nullptr)`
- Use `TEST_ERROR` for out-of-bounds operations in hierarchical context (index out of range)
- **CRITICAL**: Access children via `item->children[i]` directly - do NOT use `UnboxValue` for direct ObservableList subscript access
- **CRITICAL**: When replacing ObservableList contents, use Clear() then Add() in a loop - assignment operator is deleted

### how to test it

**Test Infrastructure:**
- Each test case must create its own callbackLog and MockNodeProviderCallback objects at the beginning
- Clear callback logs after SetItemSource to isolate verification (except when testing SetItemSource itself)
- If creating helper lambdas, ensure proper capture of other lambdas (C++ doesn't implicitly capture lambdas)

**Edge Cases:**
- Test SetItemSource with null childrenProperty and verify safe handling
  - **Important**: With null children, expect `newCount=0` due to no children to prepare eagerly
- Test SetItemSource with empty childrenProperty ObservableList - verify only OnAttached fires, no OnItemModified
  - **Important**: Empty ObservableList results in `newCount=0` in callbacks (nothing to prepare)
- Use `TEST_ERROR` for accessing nodes when childrenProperty is missing or invalid (index out of range)
- Test SetItemSource transition from null to non-null hierarchical structure
  - **Important**: Callbacks show `count=0, newCount=<actual child count>` due to eager preparation of new structure
- Test SetItemSource transition from complex hierarchy to simple flat structure
  - **Important**: Callbacks show `count=<old child count>, newCount=<new child count>` reflecting eager preparation
- Test SetItemSource transition from simple to complex hierarchy
  - **Important**: Both old and new child counts reflected in callbacks due to eager preparation
- Test SetItemSource with circular reference detection (if applicable)
- Test expansion state behavior when childrenProperty becomes empty dynamically
  - **Important**: When children are cleared via ObservableList.Clear(), expect ONE callback pair: `count=<old count>, newCount=0`
  - **Important**: ObservableList operations fire INDIVIDUAL callbacks, not batched - Clear() = 1 pair, each Add() = 1 pair
- Test expansion operation on node with null children
- Test DetachCallback during active tree operations - verify `OnAttached(provider=nullptr)` fires correctly
- Test DetachCallback after expansion state changes - verify proper cleanup
- Use `TEST_ERROR` for node operations with invalid indices (out of range)
- Use `TEST_ERROR` for expansion/collapse on invalid node references (out of range)
- Test rapid SetItemSource changes and verify proper cleanup between transitions
  - **Important**: Each SetItemSource eagerly prepares children, so rapid changes show actual counts at each step
- Test property binding with items that have inconsistent childrenProperty types
- Test dynamic childrenProperty modification (if applicable)
  - **Important**: If using ObservableList operations, each operation fires individual callbacks - not atomic
  - **Important**: Cannot assign ObservableList with `=` operator - must use Clear() then Add() loop

### rationale

Edge case testing completes the comprehensive test coverage for TreeViewItemBindableRootProvider by verifying robustness and error handling:
- Null and empty childrenProperty handling is critical for safe operation
- SetItemSource transitions cover common usage patterns and ensure proper cleanup
- Callback detachment scenarios verify proper resource management
- Following established testing philosophy including all learnings from Task No.7:
  - Use TEST_ERROR for out-of-bounds operations (index out of range)
  - Use TEST_ERROR for invalid property bindings
  - Empty ItemSource should only trigger OnAttached, not OnItemModified (lesson from Task No.2)
  - Test DetachCallback fires `OnAttached(provider=nullptr)` for proper cleanup
  - Focus on practical edge cases that could occur in real applications
  - Use minimal test infrastructure with existing mock callbacks
  - Each test case creates its own callback objects for isolation (critical lesson from Task No.5)
  - Clear callback logs after SetItemSource for clean verification
  - **All callback expectations must account for eager child preparation** (critical lesson from Task No.6)
  - Empty childrenProperty results in `newCount=0`, non-empty results in actual prepared counts
  - SetItemSource transitions show accurate before/after child counts due to eager preparation
  - The eager preparation behavior is crucial for understanding edge case callback sequences
  - **Type system: `item->children[i]` returns element directly, NOT a Value object** (critical lesson from Task No.7)
  - **Do NOT use UnboxValue for direct ObservableList subscript access** (critical lesson from Task No.7)
  - **ObservableList operations fire individual callbacks, never batched** (critical lesson from Task No.7)
  - **ObservableList cannot be assigned with `=`, must use Clear()+Add() loop** (critical lesson from Task No.7)
  - **Lambda capture: explicitly capture other lambdas, not automatic** (critical lesson from Task No.7)
- This task completes the provider testing pattern progression from simple (TextItemProvider) to complex hierarchical data binding
- With this task complete, all six provider types have comprehensive test coverage
- The learnings about eager preparation, type system, and ObservableList behavior ensure edge case tests have accurate expectations and correct implementation

# INSIGHTS AND REASONING

## Understanding the Current State

Based on the existing test file `TestItemProviders_TreeViewItemBindableRootProvider.cpp`, I can see that:

1. **Task No.6 and No.7 are complete** with comprehensive coverage of:
   - Property binding setup and verification
   - SetItemSource operations with various hierarchical structures
   - ObservableList integration and callback propagation
   - Expansion/collapse operations
   - Node lifecycle and relationships
   - UpdateBindingProperties coordination
   - Interface method testing
   - Multi-level hierarchical binding scenarios
   - Callback coordination across tree levels

2. **Existing test categories include**:
   - `AttachCallback` (basic test case)
   - `SimpleItemAddition` (basic test case)
   - `PropertyBindingSetup`
   - `SetItemSourceOperations`
   - `ObservableListIntegration`
   - `ExpansionCollapseOperations`
   - `NodeLifecycleAndRelationships`
   - `UpdateBindingProperties`
   - `InterfaceMethods`
   - `HierarchicalBindingScenarios`

3. **What remains for Task No.8**: Edge cases, error handling, boundary conditions, and cleanup scenarios

## Design Strategy for Edge Case Testing

### Category 1: Null and Empty ChildrenProperty Handling

This category focuses on boundary conditions where the `childrenProperty` is not properly set or is empty.

**Test Cases Needed:**

1. **`SetItemSourceWithNullChildrenProperty`**
   - **Purpose**: Verify safe handling when an item's `childrenProperty` returns null
   - **Setup**: Create a `BindableItem` where the children property is not initialized or explicitly set to null
   - **Expected Behavior**: 
     - SetItemSource should complete without crashing
     - Root node callbacks should show `newCount=0` (no children to prepare)
     - Accessing `GetChildCount()` should return 0
   - **Why Important**: Real-world scenarios may have items without children collections

2. **`EmptyChildrenPropertyScenarios`**
   - **Purpose**: Verify that empty ObservableList triggers only OnAttached, not OnItemModified
   - **Setup**: Create a `BindableItem` with an empty `children` ObservableList
   - **Expected Behavior**:
     - SetItemSource with callback attached should show callbacks with `newCount=0`
     - No OnItemModified callbacks should fire for the empty state itself
   - **Why Important**: Aligns with testing philosophy from Task No.2 about empty ItemSource behavior

3. **`AccessNodeWithNullChildren`**
   - **Purpose**: Verify error handling when accessing children of a node without childrenProperty
   - **Setup**: Create a tree structure where some nodes don't have children property set
   - **Expected Behavior**: 
     - `GetChildCount()` should return 0
     - `GetChild(0)` should return nullptr or handle gracefully
   - **Why Important**: Prevents crashes in malformed data scenarios

### Category 2: SetItemSource Transitions

This category tests various state transitions when changing the ItemSource.

**Test Cases Needed:**

1. **`SetItemSourceNullToNonNull`**
   - **Purpose**: Verify transition from no data to hierarchical data
   - **Setup**: Start with provider initialized but no ItemSource set, then set a multi-level tree
   - **Expected Behavior**:
     - Initial state: root node has no children
     - After SetItemSource: callbacks show `count=0, newCount=<actual child count>`
   - **Why Important**: Common initialization pattern in real applications

2. **`SetItemSourceNonNullToNull`**
   - **Purpose**: Verify cleanup when clearing ItemSource
   - **Setup**: Start with populated tree, then set ItemSource to null/empty Value
   - **Expected Behavior**:
     - Callbacks show `count=<old child count>, newCount=0`
     - All children should be properly released
   - **Why Important**: Memory cleanup verification

3. **`SetItemSourceComplexToSimple`**
   - **Purpose**: Verify transition from multi-level hierarchy to flat structure
   - **Setup**: Start with 3-level tree, replace with 1-level (root + children only)
   - **Expected Behavior**:
     - Callbacks reflect old count from multi-level and new count from flat structure
     - Old hierarchy should be properly cleaned up
   - **Why Important**: Tests structural changes in data

4. **`SetItemSourceSimpleToComplex`**
   - **Purpose**: Verify transition from flat structure to multi-level hierarchy
   - **Setup**: Start with flat structure, replace with 3-level tree
   - **Expected Behavior**:
     - Callbacks reflect both old and new child counts
     - New hierarchy should be fully accessible
   - **Why Important**: Tests expansion of data structure

5. **`RapidSetItemSourceChanges`**
   - **Purpose**: Verify robustness with rapid ItemSource changes
   - **Setup**: Quickly change ItemSource multiple times with different structures
   - **Expected Behavior**:
     - Each SetItemSource properly cleans up previous state
     - Callbacks show accurate counts at each step
     - No memory leaks or dangling references
   - **Why Important**: Stress test for state management

### Category 3: Expansion State Edge Cases

This category tests expansion state behavior under boundary conditions.

**Test Cases Needed:**

1. **`ExpandNodeWithNullChildren`**
   - **Purpose**: Verify expansion behavior when node has no children
   - **Setup**: Create node with null or uninitialized children property
   - **Expected Behavior**:
     - SetExpanding(true) should complete safely
     - OnItemExpanded callback should fire (or not, depending on implementation)
     - GetExpanding() should return expected state
   - **Why Important**: Prevents crashes on empty nodes

2. **`ExpansionStatePersistsWhenChildrenClearedDynamically`**
   - **Purpose**: Verify expansion state maintained when children are dynamically removed
   - **Setup**: Expand a node with children, then clear all children via ObservableList.Clear()
   - **Expected Behavior**:
     - Expansion state should remain true even with 0 children
     - Clear() fires ONE callback pair: `count=<old count>, newCount=0`
     - Node should remain expanded (just showing no children)
   - **Why Important**: UI should maintain expansion state regardless of data changes

3. **`CollapseAndReexpandAfterChildrenCleared`**
   - **Purpose**: Verify expansion operations still work after children are cleared
   - **Setup**: Expand node, clear children, collapse, then expand again
   - **Expected Behavior**:
     - All expansion/collapse operations should work normally
     - Callbacks should fire for each expansion state change
   - **Why Important**: Tests state machine robustness

4. **`ExpandNodeThenModifyChildren`**
   - **Purpose**: Verify children modifications work correctly on expanded nodes
   - **Setup**: Expand a node, then add/remove children dynamically
   - **Expected Behavior**:
     - Each ObservableList operation fires individual callbacks
     - Expansion state is maintained throughout
     - Children are immediately visible (eager preparation)
   - **Why Important**: Common user interaction pattern

### Category 4: Callback Detachment Scenarios

This category tests proper cleanup when detaching callbacks.

**Test Cases Needed:**

1. **`DetachCallbackDuringTreeOperation`**
   - **Purpose**: Verify DetachCallback works correctly during active tree modifications
   - **Setup**: Start modifying tree structure, then detach callback mid-operation
   - **Expected Behavior**:
     - DetachCallback fires `OnAttached(provider=nullptr)`
     - Subsequent tree modifications don't fire callbacks
     - No crashes or dangling callback references
   - **Why Important**: Tests callback lifecycle management

2. **`DetachCallbackAfterExpansionChanges`**
   - **Purpose**: Verify cleanup after expansion state changes
   - **Setup**: Expand/collapse some nodes, then detach callback
   - **Expected Behavior**:
     - DetachCallback fires `OnAttached(provider=nullptr)`
     - Expansion state is preserved in nodes
     - No memory leaks
   - **Why Important**: Ensures proper resource cleanup

3. **`ReattachCallbackAfterDetach`**
   - **Purpose**: Verify callback can be reattached after detachment
   - **Setup**: Attach callback, detach it, then reattach
   - **Expected Behavior**:
     - First attach fires `OnAttached(provider=valid)`
     - Detach fires `OnAttached(provider=nullptr)`
     - Reattach fires `OnAttached(provider=valid)` again
     - All operations after reattach fire callbacks correctly
   - **Why Important**: Tests callback reusability

4. **`DetachCallbackWithMultipleLevelsPrepared`**
   - **Purpose**: Verify detachment works with deeply nested prepared nodes
   - **Setup**: Prepare a 4-level tree with all nodes prepared, then detach
   - **Expected Behavior**:
     - DetachCallback fires `OnAttached(provider=nullptr)`
     - All prepared nodes remain accessible (no corruption)
     - No memory leaks from deep hierarchy
   - **Why Important**: Tests cleanup of complex state

### Category 5: Out-of-Bounds and Invalid Operations

This category tests error handling for invalid operations.

**Test Cases Needed:**

1. **`GetChildWithInvalidIndex`**
   - **Purpose**: Verify TEST_ERROR for out-of-bounds child access
   - **Setup**: Create node with 3 children, access child at index -1, 3, 100
   - **Expected Behavior**:
     - Implementation should use CHECK_ERROR for invalid indices
     - TEST_ERROR should catch these errors
   - **Why Important**: Tests defensive programming

2. **`GetChildBeforeChildrenPrepared`**
   - **Purpose**: Verify lazy/eager preparation behavior
   - **Setup**: Access GetChild() without calling GetChildCount() first
   - **Expected Behavior**:
     - Should work correctly (children prepared on demand)
     - Or return nullptr if not prepared (depending on implementation)
   - **Why Important**: Tests initialization order independence

3. **`SetExpandingOnInvalidNode`**
   - **Purpose**: Verify error handling when setting expansion on invalid/destroyed node
   - **Setup**: This is tricky - may need to test with nullptr or destroyed node reference
   - **Expected Behavior**:
     - Should handle gracefully (no crash)
     - TEST_ERROR if implementation uses CHECK_ERROR
   - **Why Important**: Tests robustness

4. **`AccessAfterParentRemoved`**
   - **Purpose**: Verify node behavior after parent is removed from tree
   - **Setup**: Get reference to grandchild node, then remove its parent from tree
   - **Expected Behavior**:
     - Node object still exists (referenced)
     - But it's detached from tree structure
     - GetParent() might return nullptr or old parent
   - **Why Important**: Tests object lifecycle vs tree structure

### Category 6: Property Binding Edge Cases

This category tests boundary conditions in property binding.

**Test Cases Needed:**

1. **`InconsistentChildrenPropertyTypes`**
   - **Purpose**: Verify handling when childrenProperty returns inconsistent types
   - **Setup**: Create items where some have ObservableList, others have regular List, others have null
   - **Expected Behavior**:
     - Should handle gracefully (convert or fail safely)
     - May show different callback behavior
   - **Why Important**: Real-world data may have inconsistencies

2. **`TextPropertyReturnsNull`**
   - **Purpose**: Verify handling when textProperty returns null
   - **Setup**: Create item where name property is null
   - **Expected Behavior**:
     - GetTextValue should return empty string or handle null safely
     - No crashes
   - **Why Important**: Defensive programming for missing data

3. **`UpdateBindingPropertiesWithNullChildren`**
   - **Purpose**: Verify UpdateBindingProperties works when children are null
   - **Setup**: Create tree where some nodes have null children, call UpdateBindingProperties
   - **Expected Behavior**:
     - Should complete without crashing
     - Callbacks show appropriate counts (0 for null children)
   - **Why Important**: Tests robustness of update mechanism

4. **`DynamicChildrenPropertyModification`**
   - **Purpose**: Verify behavior when childrenProperty is replaced with new ObservableList
   - **Setup**: This is complex - BindableItem's children is a field, not easily replaceable
   - **Implementation Note**: May not be testable as children is a concrete field
   - **Expected Behavior**: If testable, should unprepare old children and prepare new ones
   - **Why Important**: Understanding limitations of current design

### Category 7: ObservableList Operation Patterns

This category tests specific ObservableList operation sequences.

**Test Cases Needed:**

1. **`ObservableListClearThenMultipleAdds`**
   - **Purpose**: Verify callback sequence for Clear() followed by multiple Add()
   - **Setup**: Node with children, clear them, then add 3 new children
   - **Expected Behavior**:
     - Clear() fires ONE callback pair: `count=<old>, newCount=0`
     - Each Add() fires ONE callback pair at appropriate position
     - Total: 1 + 3 = 4 callback pairs
   - **Why Important**: Validates individual callback firing (not batched)

2. **`ObservableListRemoveRangeAtBoundaries`**
   - **Purpose**: Verify RemoveRange at start, middle, and end
   - **Setup**: Node with 10 children, test RemoveRange(0, 3), RemoveRange(3, 4), RemoveRange(3, 3)
   - **Expected Behavior**:
     - Each RemoveRange fires ONE callback pair
     - Parameters correctly reflect start, count, newCount=0
   - **Why Important**: Tests boundary conditions of range operations

3. **`ObservableListMixedOperations`**
   - **Purpose**: Verify sequence of mixed Add, Remove, RemoveRange operations
   - **Setup**: Start with children, do: Add, Remove, Add, RemoveRange, Add
   - **Expected Behavior**:
     - Each operation fires exactly ONE callback pair
     - Sequence is correct
     - Final tree state is correct
   - **Why Important**: Tests complex operation sequences

4. **`ObservableListReplaceViaAssignmentNotAllowed`**
   - **Purpose**: Document that assignment operator is not available
   - **Setup**: This is a documentation test - comment explaining the limitation
   - **Implementation**: Not a runtime test, but a code comment or compile-time check
   - **Expected Behavior**: Must use Clear() + Add() loop instead
   - **Why Important**: Critical learning from Task No.7

### Category 8: Memory and Lifecycle Edge Cases

This category tests memory management and object lifecycle.

**Test Cases Needed:**

1. **`NodeReferencePersistsAfterTreeModification`**
   - **Purpose**: Verify node references remain valid after tree changes
   - **Setup**: Get reference to child node, modify tree, verify reference still works
   - **Expected Behavior**:
     - Node object remains valid (reference counted)
     - But position in tree may have changed
   - **Why Important**: Tests reference counting

2. **`GetBindingValueReturnsCorrectItemAfterModifications`**
   - **Purpose**: Verify GetBindingValue returns correct item after tree changes
   - **Setup**: Modify tree, then verify GetBindingValue for various nodes
   - **Expected Behavior**:
     - Each node returns its original BindableItem
     - Items are not corrupted by tree modifications
   - **Why Important**: Tests data integrity

3. **`MultipleProvidersShareSameBindableItems`**
   - **Purpose**: Verify multiple providers can use same items (or test for issues)
   - **Setup**: Create two providers with same BindableItem tree
   - **Expected Behavior**:
     - Each provider maintains its own node structure
     - Modifications in one don't affect the other (unless sharing data)
   - **Why Important**: Tests provider independence

## Test Organization Strategy

Given the large number of edge cases, I'll organize them into logical TEST_CATEGORY groups:

1. **`EmptyAndNullScenarios`** - Tests for null and empty children handling
2. **`SetItemSourceTransitions`** - Tests for various ItemSource change patterns
3. **`ExpansionStateEdgeCases`** - Tests for expansion behavior under edge conditions
4. **`CallbackDetachment`** - Tests for callback lifecycle management
5. **`InvalidOperations`** - Tests for out-of-bounds and error handling
6. **`PropertyBindingEdgeCases`** - Tests for property binding boundary conditions
7. **`ObservableListOperationPatterns`** - Tests for specific ObservableList sequences

## Implementation Considerations

### Critical Findings from Task No.7 That Apply Here:

1. **Type System**: `item->children[i]` returns element directly, not a Value object
2. **No UnboxValue for subscript**: Direct ObservableList subscript access doesn't need UnboxValue
3. **Individual Callbacks**: ObservableList operations fire individual callbacks, never batched
4. **No Assignment Operator**: Cannot use `=` to assign ObservableList, must use Clear() + loop
5. **Lambda Capture**: Must explicitly capture other lambdas in lambda capture lists

### Testing Philosophy Reminders:

1. **Focus on Behavior, Not State**: Test callbacks and observable effects, not internal fields
2. **Minimal Infrastructure**: Use existing MockNodeProviderCallback, don't create complex assertion frameworks
3. **Practical Edge Cases**: Focus on scenarios that could occur in real applications
4. **Test Isolation**: Each test case creates its own callback objects
5. **Eager Preparation**: All callback expectations must account for eager child preparation

### Expected Callback Patterns for Edge Cases:

1. **Empty Children**: `count=0, newCount=0` (or no callbacks for text-only changes)
2. **Null Children**: `count=0, newCount=0` (treated as empty)
3. **Clear Operation**: ONE pair `count=<old>, newCount=0`
4. **Multiple Adds**: Multiple pairs, one per Add
5. **SetItemSource Transitions**: ONE pair showing old count and new count

## Design Decisions

### Decision 1: How to Test Null ChildrenProperty

**Challenge**: BindableItem has `children` as a concrete ObservableList field, always initialized.

**Solutions**:
- **Option A**: Create a different test class with nullable children property
  - Pros: Can test null scenario properly
  - Cons: Requires new test infrastructure
- **Option B**: Test with "functionally null" (empty ObservableList)
  - Pros: Uses existing infrastructure
  - Cons: Not truly testing null, just empty
- **Option C**: Test both empty and a custom class with null children
  - Pros: Comprehensive coverage
  - Cons: More complex setup

**Decision**: Use **Option C** - Test empty with existing BindableItem, and create a minimal custom class for true null testing where critical.

### Decision 2: How to Test Invalid Operations (TEST_ERROR)

**Challenge**: Need to verify CHECK_ERROR is hit for invalid operations.

**Implementation**:
```cpp
TEST_CASE(L"GetChildWithInvalidIndex")
{
    // Setup provider with 3 children
    TEST_ERROR(provider->GetRootNode()->GetChild(-1));
    TEST_ERROR(provider->GetRootNode()->GetChild(100));
}
```

This uses the TEST_ERROR macro which expects CHECK_ERROR or CHECK_FAIL to be triggered.

### Decision 3: How to Test DetachCallback

**Implementation Pattern**:
```cpp
TEST_CASE(L"DetachCallbackDuringTreeOperation")
{
    // Setup
    provider->AttachCallback(&nodeCallback);
    callbackLog.Clear();
    
    // Perform operations
    // ... tree modifications ...
    
    // Detach
    provider->DetachCallback(&nodeCallback);
    
    // Verify OnAttached(provider=nullptr) in log
    // ... check for detach callback ...
    
    callbackLog.Clear();
    // Further modifications should not fire callbacks
    // ... more tree changes ...
    TEST_ASSERT(callbackLog.Count() == 0);
}
```

### Decision 4: Test Coverage vs Test Count

**Challenge**: Many edge cases could lead to 30+ test cases, making the file very long.

**Decision**: 
- Group related edge cases into single test cases where appropriate
- Focus on most important edge cases that represent real scenarios
- Prioritize: Null/empty handling, SetItemSource transitions, callback detachment, invalid operations
- Defer: Stress tests, performance tests, exotic scenarios

**Target**: Approximately 15-20 test cases in this task, covering all critical edge cases.

## Test Case Priority

### High Priority (Must Have):
1. EmptyChildrenPropertyScenarios
2. SetItemSourceNullToNonNull
3. SetItemSourceNonNullToNull
4. DetachCallbackDuringTreeOperation
5. GetChildWithInvalidIndex
6. ExpansionStatePersistsWhenChildrenClearedDynamically
7. ObservableListClearThenMultipleAdds

### Medium Priority (Should Have):
8. SetItemSourceComplexToSimple
9. SetItemSourceSimpleToComplex
10. ExpandNodeWithNullChildren
11. DetachCallbackAfterExpansionChanges
12. TextPropertyReturnsNull
13. NodeReferencePersistsAfterTreeModification

### Lower Priority (Nice to Have):
14. RapidSetItemSourceChanges
15. ReattachCallbackAfterDetach
16. InconsistentChildrenPropertyTypes
17. GetBindingValueReturnsCorrectItemAfterModifications

## Proposed Test Structure

```cpp
TEST_CATEGORY(L"EmptyAndNullScenarios")
{
    TEST_CASE(L"SetItemSourceWithEmptyChildren") { ... }
    TEST_CASE(L"ExpandNodeWithNoChildren") { ... }
    TEST_CASE(L"AccessChildrenOfEmptyNode") { ... }
}

TEST_CATEGORY(L"SetItemSourceTransitions")
{
    TEST_CASE(L"TransitionFromNullToHierarchy") { ... }
    TEST_CASE(L"TransitionFromHierarchyToNull") { ... }
    TEST_CASE(L"TransitionComplexToSimple") { ... }
    TEST_CASE(L"TransitionSimpleToComplex") { ... }
    TEST_CASE(L"RapidConsecutiveTransitions") { ... }
}

TEST_CATEGORY(L"ExpansionStateEdgeCases")
{
    TEST_CASE(L"ExpansionStateWithDynamicClear") { ... }
    TEST_CASE(L"CollapseAndReexpandEmptyNode") { ... }
}

TEST_CATEGORY(L"CallbackDetachment")
{
    TEST_CASE(L"DetachDuringTreeModification") { ... }
    TEST_CASE(L"DetachAfterExpansionChanges") { ... }
    TEST_CASE(L"ReattachAfterDetachment") { ... }
}

TEST_CATEGORY(L"InvalidOperations")
{
    TEST_CASE(L"InvalidChildIndex") { ... }
    TEST_CASE(L"InvalidNodeOperations") { ... }
}

TEST_CATEGORY(L"PropertyBindingEdgeCases")
{
    TEST_CASE(L"NullTextProperty") { ... }
    TEST_CASE(L"UpdateBindingPropertiesEdgeCases") { ... }
}

TEST_CATEGORY(L"ObservableListOperationPatterns")
{
    TEST_CASE(L"ClearFollowedByAdds") { ... }
    TEST_CASE(L"RemoveRangeBoundaries") { ... }
}
```

## Expected Callback Behavior Summary

Based on existing tests and understanding of eager preparation:

| Operation | Expected Callbacks | Notes |
|-----------|-------------------|-------|
| SetItemSource with empty children | `count=X, newCount=0` | X = old count |
| SetItemSource with null children | `count=X, newCount=0` | Treated as empty |
| SetItemSource null → data | `count=0, newCount=Y` | Y = new child count (eager) |
| SetItemSource data → null | `count=X, newCount=0` | X = old child count |
| ObservableList.Clear() | ONE pair: `count=X, newCount=0` | Not batched |
| ObservableList.Add() | ONE pair per Add | Individual callbacks |
| DetachCallback | `OnAttached(provider=nullptr)` | In callback log |
| Expansion state change | `OnItemExpanded()` or `OnItemCollapsed()` | No count parameters |

## Validation Checklist

Before marking task complete, verify:

- [ ] All test cases compile successfully
- [ ] All test cases pass
- [ ] Each test case creates its own callback objects (no sharing)
- [ ] Callback logs are cleared appropriately (except when testing the operation itself)
- [ ] TEST_ERROR is used for out-of-bounds operations
- [ ] TEST_EXCEPTION is not used (no duplicate detection in bindable providers)
- [ ] All expected callback sequences account for eager preparation
- [ ] No UnboxValue used for direct ObservableList subscript access
- [ ] ObservableList modifications use Clear() + Add() loop, not assignment
- [ ] Lambda captures are explicit for other lambdas
- [ ] Tests cover: null/empty, transitions, expansion, detachment, invalid ops, property bindings
- [ ] Test names are descriptive and follow existing patterns
- [ ] Code follows C++ 20 style with tabs for indentation
- [ ] Tests verify behavior, not internal state

# !!!FINISHED!!!
