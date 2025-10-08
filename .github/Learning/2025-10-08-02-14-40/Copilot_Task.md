# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.5: Implement Test Cases for Operations on Invisible Nodes

This task implements test cases to verify that operations on invisible nodes (in collapsed subtrees) do not affect `tree::NodeItemProvider` in any way, ensuring correct isolation between visible and invisible portions of the tree.

### what to be done

Implement a test category "OperationsOnInvisibleNodes" covering these scenarios:

- Test that adding/removing children to/from an invisible node (in a collapsed subtree) does not affect `Count()` of `tree::NodeItemProvider`
- Test that the expand state of invisible nodes is preserved (when parent is later expanded, the previously expanded invisible node's children become visible)
- Test that multiple structural changes to collapsed subtrees do not cause any `OnItemModified` events on `tree::NodeItemProvider`
- Verify that when a collapsed parent containing modified invisible nodes is expanded, the correct structure is reflected in visible indices
- Test that `CalculateNodeVisibilityIndex()` returns -1 for nodes in invisible subtrees regardless of their own expand state
- **NOTE**: Avoid testing expand/collapse operations on invisible nodes themselves, as this behavior has uncertain semantics (see learnings from Task 3)

This task should add approximately 100-150 lines of test code.

### how to test it

**BEFORE WRITING TESTS:**
1. **Do NOT test** `SetExpanding()` on invisible nodes unless you first verify the expected behavior in the implementation
2. **Focus on clear, practical scenarios** where the expected behavior is well-defined
3. **Avoid ambiguous edge cases** like "what happens when you expand an already-collapsed invisible node"

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build tree structures (3-4 levels) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events
- Collapse parent nodes to create invisible subtrees
- Perform **well-defined operations** on invisible nodes:
  - Add/remove children to invisible nodes (clear expected behavior: no callbacks)
  - Modify properties of invisible nodes (if applicable)
  - **Consider carefully** whether to test expand/collapse of invisible nodes (learned from Task 3: this has uncertain semantics)
- Verify that no callbacks are triggered and `Count()` doesn't change
- Clear callback logs to ensure isolation
- Later expand the parent and verify the invisible operations had the expected effect on the tree structure
- **Code Style (from completed tasks):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering
  - Use `AssertItems` helper with `const wchar_t*` arrays for verifying visible node sequences
  - **Focus on practical scenarios** that matter in real applications
  - **Remove any test** that has unclear expected behavior during implementation

Organize under a `TEST_CATEGORY` block named "OperationsOnInvisibleNodes".

### rationale

Testing operations on invisible nodes is critical because `tree::NodeItemProvider` should be completely unaware of and unaffected by changes in collapsed subtrees:

1. **Performance**: If `NodeItemProvider` reacts to every change in invisible nodes, it would waste CPU cycles recalculating indices for nodes that aren't displayed

2. **Correctness**: The `Count()` and visible indices should only reflect what's actually visible, not the state of hidden subtrees

3. **Event Discipline**: `OnItemModified` callbacks should only fire when the visible list actually changes, not when invisible nodes are modified

4. **State Preservation**: The tree structure maintains expand/collapse states even when nodes are invisible, so when a parent is later expanded, the child's previous state should be honored

5. **Learning from Task 3**: Avoid testing operations on invisible nodes where the expected behavior is not well-documented. The practical test case is: "collapse a parent with expanded children, then re-expand the parent - children should still be expanded" (already covered in `CollapseNodeWithExpandedDescendants` test from Task 3)

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` checking `CalculateNodeVisibilityIndex()` before invoking callbacks
- The visibility check returns -1 for nodes in collapsed subtrees, preventing callback propagation
- The underlying `INodeRootProvider` managing the full tree structure independently of what's visible
- **Key learning**: The semantics of calling `SetExpanding()` on invisible nodes is implementation-dependent and not clearly documented

This task should come after Tasks 2, 3, and 4 because it tests a specific isolation concern that builds on understanding of basic mapping, dynamics, and edge cases.

# INSIGHTS AND REASONING

## Understanding the Problem

Task 5 focuses on verifying that `tree::NodeItemProvider` correctly isolates visible and invisible portions of the tree. The key requirement is that operations on invisible nodes (nodes in collapsed subtrees) should not affect the visible state of the `NodeItemProvider` in any way.

## Analysis of Implementation Behavior

### How NodeItemProvider Detects Invisible Nodes

From `DataSourceImpl_IItemProvider_NodeItemProvider.cpp`, I found the critical mechanism:

```cpp
vint NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider* node)
{
    auto parent = node->GetParent();
    if (!parent) return -1;
    if (!parent->GetExpanding()) return -2;  // Parent is collapsed!
    
    vint index = CalculateNodeVisibilityIndexInternal(parent.Obj());
    if (index == -2) return -2;  // Ancestor is collapsed!
    // ... rest of calculation
}
```

The return value semantics:
- **-2**: Node is invisible (in a collapsed subtree)
- **-1**: Node's parent is the root node
- **>= 0**: Valid visible index

The public method `CalculateNodeVisibilityIndex()` converts all negative values to -1, which is why tests should expect -1 for invisible nodes.

### How Callbacks Are Prevented for Invisible Nodes

In `OnAfterItemModified()`:

```cpp
void NodeItemProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
{
    vint base = CalculateNodeVisibilityIndexInternal(parentNode);
    if (base != -2 && parentNode->GetExpanding())  // Only proceed if visible!
    {
        // ... calculate offsets and invoke callbacks
        InvokeOnItemModified(firstChildStart, offsetBeforeChildModified, offset, itemReferenceUpdated);
    }
}
```

The condition `base != -2 && parentNode->GetExpanding()` means:
1. The parent node must be visible (base != -2)
2. The parent node must be expanded (parentNode->GetExpanding())

This ensures that adding/removing children to an invisible or collapsed node will NOT trigger `OnItemModified` callbacks.

Similarly, in `OnItemExpanded()` and `OnItemCollapsed()`:

```cpp
void NodeItemProvider::OnItemExpanded(INodeProvider* node)
{
    vint base = CalculateNodeVisibilityIndexInternal(node);
    if (base != -2)  // Only if node is visible!
    {
        // ... trigger callbacks
    }
}
```

This means expanding/collapsing an invisible node will NOT trigger callbacks.

## Current Test Coverage Analysis

### What Has Been Tested (Tasks 1-4)

From examining `TestItemProviders_NodeItemProvider.cpp`:

1. **Task 1**: Basic setup and construction
2. **Task 2**: Basic index mapping with static trees
3. **Task 3**: Expand/collapse dynamics including:
   - `CollapseNodeWithExpandedDescendants`: Tests that expanded children remain expanded when parent is re-expanded (state preservation)
4. **Task 4**: Edge cases including:
   - `DynamicChildAdditionRemoval`: Tests adding/removing children to **expanded** nodes
   - Invalid indices
   - Foreign nodes
   - Deep hierarchies
   - Large trees

### What Has NOT Been Tested (Gap for Task 5)

No tests exist for:
1. **Adding children to collapsed nodes**: Does it trigger callbacks? (It shouldn't)
2. **Removing children from collapsed nodes**: Does it trigger callbacks? (It shouldn't)
3. **Multiple modifications to invisible subtrees**: Cumulative effect when parent is later expanded
4. **Verifying Count() doesn't change**: When invisible nodes are modified
5. **Verifying structure correctness**: After expanding parent that contains modified invisible nodes

## Design Approach

### Test Cases to Implement

Based on the implementation analysis and coverage gaps, I recommend **3-4 test cases**:

#### Test Case 1: Add Children to Collapsed Node
**Purpose**: Verify that adding children to a collapsed (invisible) node does not affect NodeItemProvider

**Steps**:
1. Create tree: Root -> Parent (expanded) -> Child (collapsed) -> [empty]
2. Attach callback monitor
3. Add 1-2 children to the collapsed Child node
4. Verify:
   - `Count()` remains unchanged
   - No `OnItemModified` callbacks triggered
   - Visible structure unchanged (Parent and Child still visible, new grandchildren not visible)
5. Expand Child node
6. Verify:
   - New children become visible
   - Correct callback triggered
   - Count increases appropriately

**Why this matters**: This is the most common real-world scenario - lazy loading children when a node is expanded.

#### Test Case 2: Remove Children from Collapsed Node
**Purpose**: Verify that removing children from a collapsed node does not affect NodeItemProvider

**Steps**:
1. Create tree: Root -> Parent (expanded) -> Child (collapsed with 2 children)
2. Attach callback monitor
3. Remove 1 child from the collapsed Child node
4. Verify:
   - `Count()` remains unchanged
   - No `OnItemModified` callbacks triggered
   - Visible structure unchanged
5. Expand Child node
6. Verify:
   - Only remaining child is visible
   - Correct structure reflected

**Why this matters**: Ensures removal operations on invisible nodes are properly isolated.

#### Test Case 3: Multiple Modifications to Invisible Subtree
**Purpose**: Verify that multiple operations on invisible nodes accumulate correctly

**Steps**:
1. Create tree with deeper hierarchy (4 levels): Root -> A (expanded) -> B (collapsed) -> C (expanded) -> D
2. Initially, only Root and A visible; B, C, D are invisible
3. Perform multiple modifications on invisible nodes:
   - Add children to B
   - Add children to C
   - Remove a child from B
4. Verify no callbacks triggered and Count() unchanged
5. Expand A and B sequentially
6. Verify cumulative modifications are correctly reflected

**Why this matters**: Tests that modifications to deeply nested invisible nodes work correctly.

#### Test Case 4: CalculateNodeVisibilityIndex on Invisible Nodes
**Purpose**: Verify that invisible nodes consistently return -1

**Steps**:
1. Create tree: Root -> Parent (collapsed) -> Child1 (collapsed) -> Child2 (expanded) -> Child3
2. All descendants of Parent are invisible
3. Verify `CalculateNodeVisibilityIndex()` returns -1 for:
   - Child1 (collapsed and invisible)
   - Child2 (expanded but invisible)
   - Child3 (in invisible subtree)
4. Expand Parent
5. Verify Child1 and Child2 now have valid indices, Child3 still returns -1

**Why this matters**: Ensures that expand state of invisible nodes doesn't affect their visibility calculation.

### What NOT to Test

Based on warnings in the task description and learnings from Task 3:

1. **Do NOT test `SetExpanding()` on invisible nodes directly** - the semantics are unclear and implementation-dependent
2. **Do NOT test ambiguous edge cases** like "expand an already-expanded invisible node"
3. **State preservation is already tested** in Task 3's `CollapseNodeWithExpandedDescendants`

### Code Organization

Following the established patterns from Tasks 1-4:

```cpp
TEST_CATEGORY(L"OperationsOnInvisibleNodes")
{
    TEST_CASE(L"AddChildrenToCollapsedNode")
    {
        // Setup with callback monitoring
        // Test modifications to invisible nodes
        // Verify no callbacks and Count() unchanged
        // Expand parent and verify structure
    });
    
    TEST_CASE(L"RemoveChildrenFromCollapsedNode")
    {
        // Similar structure
    });
    
    TEST_CASE(L"MultipleModificationsToInvisibleSubtree")
    {
        // Test cumulative modifications
    });
    
    TEST_CASE(L"VisibilityIndexForInvisibleNodes")
    {
        // Test CalculateNodeVisibilityIndex behavior
    });
});
```

### Expected Line Count

- Each test case: ~25-40 lines (including setup, operations, verifications)
- Total: ~100-160 lines
- Well within the guideline of "a few hundred lines of code"

## Implementation Evidence

### Key Source Files

1. **Implementation**: `Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.cpp`
   - Lines 42-104: `OnBeforeItemModified` and `OnAfterItemModified` with visibility checks
   - Lines 106-130: `OnItemExpanded` and `OnItemCollapsed` with visibility checks
   - Lines 132-178: `CalculateNodeVisibilityIndexInternal` returning -2 for invisible nodes

2. **Header**: `Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.h`
   - Defines the interface contracts

3. **Existing Tests**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`
   - Tasks 1-4 completed (lines 1-1416)
   - Patterns to follow established in previous test cases

### Key Helper Functions Available

From examining the test file:
- `CreateTreeViewItem(const wchar_t* text)`: Creates a node with text
- `MockItemProviderCallback`: Monitors callbacks with string logging
- `AssertCallbacks(List<WString>& log, const wchar_t** expected)`: Verifies callback sequences
- `AssertItems(Ptr<NodeItemProvider>, const wchar_t** expected)`: Verifies visible item texts
- `rootProvider->GetMemoryNode(node)->Children()`: Access to manipulate child collections

## Rationale Summary

This task is critical for ensuring that `NodeItemProvider` correctly implements the separation of concerns between:
- **Visible state**: What's currently shown in the UI (should trigger callbacks)
- **Invisible state**: What's hidden in collapsed subtrees (should NOT trigger callbacks)

This separation is essential for:
1. **Performance**: Avoid unnecessary UI updates for invisible changes
2. **Correctness**: Ensure Count() and indices reflect only visible items
3. **State preservation**: Invisible nodes maintain their structure for when they become visible

The implementation already has the correct behavior (checking `base != -2`), but this task adds comprehensive test coverage to ensure it stays correct and to document the expected behavior.

# !!!FINISHED!!!
