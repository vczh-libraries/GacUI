# !!!PLANNING!!!

# IMPROVEMENT PLAN

## STEP 1: Add Test Case for Adding Children to Collapsed Nodes

### What to Change

Add a new test case `AddChildrenToCollapsedNode` in the `OperationsOnInvisibleNodes` test category in `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`.

This test will:
1. Create a 3-level tree: Root -> Parent (expanded) -> Child (collapsed)
2. Add 2 children to the collapsed Child node
3. Verify no callbacks are triggered and `Count()` remains unchanged
4. Expand the Child node and verify the children become visible

**Code to add:**

```cpp
TEST_CASE(L"AddChildrenToCollapsedNode")
{
	// Setup callback logging
	List<WString> callbackLog;
	MockItemProviderCallback itemCallback(callbackLog);
	
	// Create tree: Root -> Parent (expanded) -> Child (collapsed)
	// INodeRootProvider interface
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto parent = CreateTreeViewItem(L"Parent");
	auto child = CreateTreeViewItem(L"Child");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(parent);
	
	auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
	parentMemoryNode->Children().Add(child);
	
	// Expand parent so Child is visible
	parent->SetExpanding(true);
	
	// Create NodeItemProvider and attach callback
	// IItemProvider interface
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	nodeItemProvider->AttachCallback(&itemCallback);
	callbackLog.Clear();
	
	// Verify initial state - Parent and Child visible, Child collapsed
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	const wchar_t* expectedBefore[] = {
		L"Parent",
		L"Child"
	};
	AssertItems(nodeItemProvider, expectedBefore);
	
	// INodeItemView interface - verify Child is visible but has no visible children
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
	
	// Add children to collapsed Child node
	// INodeProvider interface - Children() returns collection of child nodes
	auto childMemoryNode = rootProvider->GetMemoryNode(child.Obj());
	auto grandchild1 = CreateTreeViewItem(L"Grandchild1");
	auto grandchild2 = CreateTreeViewItem(L"Grandchild2");
	childMemoryNode->Children().Add(grandchild1);
	childMemoryNode->Children().Add(grandchild2);
	
	// Verify no callbacks triggered and Count unchanged
	// IItemProvider interface
	TEST_ASSERT(callbackLog.Count() == 0);
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	AssertItems(nodeItemProvider, expectedBefore);
	
	// INodeItemView interface - verify grandchildren are invisible
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild1.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild2.Obj()) == -1);
	
	callbackLog.Clear();
	
	// Expand Child node
	// INodeProvider interface
	child->SetExpanding(true);
	
	// Verify grandchildren become visible and callback is triggered
	// IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 4);
	const wchar_t* expectedAfter[] = {
		L"Parent",
		L"Child",
		L"Grandchild1",
		L"Grandchild2"
	};
	AssertItems(nodeItemProvider, expectedAfter);
	
	// IItemProviderCallback interface - verify callback for expansion
	const wchar_t* expectedCallbacks[] = {
		L"OnItemModified(start=2, count=0, newCount=2, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expectedCallbacks);
	
	// INodeItemView interface - verify all nodes have correct indices
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild1.Obj()) == 2);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild2.Obj()) == 3);
});
```

### Why This Change is Necessary

This test verifies the most common real-world scenario: lazy loading children when a node is expanded. It ensures that:
- Adding children to invisible nodes does not trigger callbacks
- The `Count()` of visible items remains unchanged
- When the parent is later expanded, the added children become visible correctly
- This is critical for performance - UI should not update for changes to invisible nodes

## STEP 2: Add Test Case for Removing Children from Collapsed Nodes

### What to Change

Add a new test case `RemoveChildrenFromCollapsedNode` in the `OperationsOnInvisibleNodes` test category.

This test will:
1. Create a 3-level tree where a collapsed node already has children
2. Remove one child from the collapsed node
3. Verify no callbacks are triggered and `Count()` remains unchanged
4. Expand the parent node and verify only the remaining child is visible

**Code to add:**

```cpp
TEST_CASE(L"RemoveChildrenFromCollapsedNode")
{
	// Setup callback logging
	List<WString> callbackLog;
	MockItemProviderCallback itemCallback(callbackLog);
	
	// Create tree with collapsed node that has children
	// Root -> Parent (expanded) -> Child (collapsed with 2 grandchildren)
	// INodeRootProvider interface
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto parent = CreateTreeViewItem(L"Parent");
	auto child = CreateTreeViewItem(L"Child");
	auto grandchild1 = CreateTreeViewItem(L"Grandchild1");
	auto grandchild2 = CreateTreeViewItem(L"Grandchild2");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(parent);
	
	auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
	parentMemoryNode->Children().Add(child);
	
	auto childMemoryNode = rootProvider->GetMemoryNode(child.Obj());
	childMemoryNode->Children().Add(grandchild1);
	childMemoryNode->Children().Add(grandchild2);
	
	// Expand parent so Child is visible (but Child remains collapsed)
	parent->SetExpanding(true);
	
	// Create NodeItemProvider and attach callback
	// IItemProvider interface
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	nodeItemProvider->AttachCallback(&itemCallback);
	callbackLog.Clear();
	
	// Verify initial state - only Parent and Child visible
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	const wchar_t* expectedBefore[] = {
		L"Parent",
		L"Child"
	};
	AssertItems(nodeItemProvider, expectedBefore);
	
	// INodeItemView interface - verify grandchildren are invisible
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild1.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild2.Obj()) == -1);
	
	// Remove one grandchild from collapsed Child node
	// INodeProvider interface - Children().RemoveAt() modifies child collection
	childMemoryNode->Children().RemoveAt(0);
	
	// Verify no callbacks triggered and Count unchanged
	// IItemProvider interface
	TEST_ASSERT(callbackLog.Count() == 0);
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	AssertItems(nodeItemProvider, expectedBefore);
	
	callbackLog.Clear();
	
	// Expand Child node
	// INodeProvider interface
	child->SetExpanding(true);
	
	// Verify only remaining grandchild becomes visible
	// IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 3);
	const wchar_t* expectedAfter[] = {
		L"Parent",
		L"Child",
		L"Grandchild2"
	};
	AssertItems(nodeItemProvider, expectedAfter);
	
	// IItemProviderCallback interface - verify callback for expansion
	const wchar_t* expectedCallbacks[] = {
		L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expectedCallbacks);
	
	// INodeItemView interface - verify correct indices
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild2.Obj()) == 2);
	
	// Verify removed grandchild remains invisible
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild1.Obj()) == -1);
});
```

### Why This Change is Necessary

This test ensures that removal operations on invisible nodes are properly isolated. It verifies:
- Removing children from collapsed nodes does not affect the visible state
- The removal is correctly reflected when the parent is later expanded
- This is important for scenarios where tree data is updated while portions are collapsed

## STEP 3: Add Test Case for Multiple Modifications to Invisible Subtree

### What to Change

Add a new test case `MultipleModificationsToInvisibleSubtree` in the `OperationsOnInvisibleNodes` test category.

This test will:
1. Create a 4-level tree with a deeply nested structure
2. Perform multiple add/remove operations on invisible nodes at different levels
3. Verify no callbacks are triggered during modifications
4. Expand parent nodes sequentially and verify cumulative changes

**Code to add:**

```cpp
TEST_CASE(L"MultipleModificationsToInvisibleSubtree")
{
	// Setup callback logging
	List<WString> callbackLog;
	MockItemProviderCallback itemCallback(callbackLog);
	
	// Create 4-level tree: Root -> A (expanded) -> B (collapsed) -> C -> D
	// INodeRootProvider interface
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto nodeA = CreateTreeViewItem(L"A");
	auto nodeB = CreateTreeViewItem(L"B");
	auto nodeC = CreateTreeViewItem(L"C");
	auto nodeD = CreateTreeViewItem(L"D");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(nodeA);
	
	auto nodeAMemoryNode = rootProvider->GetMemoryNode(nodeA.Obj());
	nodeAMemoryNode->Children().Add(nodeB);
	
	auto nodeBMemoryNode = rootProvider->GetMemoryNode(nodeB.Obj());
	nodeBMemoryNode->Children().Add(nodeC);
	
	auto nodeCMemoryNode = rootProvider->GetMemoryNode(nodeC.Obj());
	nodeCMemoryNode->Children().Add(nodeD);
	
	// Expand only A, so B, C, D are invisible
	nodeA->SetExpanding(true);
	
	// Create NodeItemProvider and attach callback
	// IItemProvider interface
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	nodeItemProvider->AttachCallback(&itemCallback);
	callbackLog.Clear();
	
	// Verify initial state - only A and B visible
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	const wchar_t* expectedInitial[] = {
		L"A",
		L"B"
	};
	AssertItems(nodeItemProvider, expectedInitial);
	
	// INodeItemView interface - verify C and D are invisible
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeD.Obj()) == -1);
	
	// Perform multiple modifications on invisible nodes
	// Add children to B (invisible subtree root)
	auto newChildB1 = CreateTreeViewItem(L"B-Child1");
	auto newChildB2 = CreateTreeViewItem(L"B-Child2");
	nodeBMemoryNode->Children().Add(newChildB1);
	nodeBMemoryNode->Children().Add(newChildB2);
	
	// Add children to C (deeper invisible node)
	auto newChildC1 = CreateTreeViewItem(L"C-Child1");
	nodeCMemoryNode->Children().Add(newChildC1);
	
	// Remove D from C
	nodeCMemoryNode->Children().RemoveAt(0);
	
	// Verify no callbacks triggered and Count unchanged
	// IItemProvider interface
	TEST_ASSERT(callbackLog.Count() == 0);
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	AssertItems(nodeItemProvider, expectedInitial);
	
	callbackLog.Clear();
	
	// Expand B
	// INodeProvider interface
	nodeB->SetExpanding(true);
	
	// Verify B's children and C become visible
	// IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 5);
	const wchar_t* expectedAfterExpandB[] = {
		L"A",
		L"B",
		L"C",
		L"B-Child1",
		L"B-Child2"
	};
	AssertItems(nodeItemProvider, expectedAfterExpandB);
	
	// IItemProviderCallback interface - verify callback for B's expansion
	const wchar_t* expectedCallbacksB[] = {
		L"OnItemModified(start=2, count=0, newCount=3, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expectedCallbacksB);
	
	callbackLog.Clear();
	
	// Expand C
	// INodeProvider interface
	nodeC->SetExpanding(true);
	
	// Verify C's new child becomes visible (D was removed)
	// IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 6);
	const wchar_t* expectedAfterExpandC[] = {
		L"A",
		L"B",
		L"C",
		L"C-Child1",
		L"B-Child1",
		L"B-Child2"
	};
	AssertItems(nodeItemProvider, expectedAfterExpandC);
	
	// IItemProviderCallback interface - verify callback for C's expansion
	const wchar_t* expectedCallbacksC[] = {
		L"OnItemModified(start=3, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expectedCallbacksC);
	
	// INodeItemView interface - verify D remains invisible (it was removed)
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeD.Obj()) == -1);
});
```

### Why This Change is Necessary

This test verifies that multiple cumulative modifications to deeply nested invisible nodes work correctly. It ensures:
- Multiple operations on different levels of invisible nodes do not trigger callbacks
- The cumulative effect is correctly reflected when expanding parents sequentially
- This is important for complex tree updates where multiple invisible branches are modified
- It tests that the implementation correctly handles nested invisible subtrees

## STEP 4: Add Test Case for Visibility Index of Invisible Nodes

### What to Change

Add a new test case `VisibilityIndexForInvisibleNodes` in the `OperationsOnInvisibleNodes` test category.

This test will:
1. Create a tree with mixed expand states in an invisible subtree
2. Verify that `CalculateNodeVisibilityIndex()` returns -1 for all invisible nodes regardless of their own expand state
3. Expand the parent and verify visibility indices update correctly

**Code to add:**

```cpp
TEST_CASE(L"VisibilityIndexForInvisibleNodes")
{
	// Create tree with mixed expand states in invisible subtree
	// Root -> Parent (collapsed) -> Child1 (collapsed), Child2 (expanded) -> Grandchild
	// INodeRootProvider interface
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto parent = CreateTreeViewItem(L"Parent");
	auto child1 = CreateTreeViewItem(L"Child1");
	auto child2 = CreateTreeViewItem(L"Child2");
	auto grandchild = CreateTreeViewItem(L"Grandchild");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(parent);
	
	auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
	parentMemoryNode->Children().Add(child1);
	parentMemoryNode->Children().Add(child2);
	
	auto child2MemoryNode = rootProvider->GetMemoryNode(child2.Obj());
	child2MemoryNode->Children().Add(grandchild);
	
	// Set expand states: Child1 collapsed, Child2 expanded
	// INodeProvider interface
	child1->SetExpanding(false);
	child2->SetExpanding(true);
	
	// Keep Parent collapsed, making all children invisible
	parent->SetExpanding(false);
	
	// Create NodeItemProvider
	// IItemProvider interface
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	
	// Verify only Parent is visible
	TEST_ASSERT(nodeItemProvider->Count() == 1);
	const wchar_t* expectedCollapsed[] = {
		L"Parent"
	};
	AssertItems(nodeItemProvider, expectedCollapsed);
	
	// INodeItemView interface - verify all descendants return -1 regardless of expand state
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -1);
	
	// Verify Parent has valid index
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
	
	// Expand Parent
	// INodeProvider interface
	parent->SetExpanding(true);
	
	// Verify Child1 and Child2 become visible, Grandchild also visible (Child2 is expanded)
	// IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 4);
	const wchar_t* expectedExpanded[] = {
		L"Parent",
		L"Child1",
		L"Child2",
		L"Grandchild"
	};
	AssertItems(nodeItemProvider, expectedExpanded);
	
	// INodeItemView interface - verify correct visibility indices
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == 2);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == 3);
});
```

### Why This Change is Necessary

This test verifies that `CalculateNodeVisibilityIndex()` correctly identifies invisible nodes. It ensures:
- All nodes in a collapsed subtree return -1, regardless of their own expand state
- The expand state of invisible nodes is preserved (when parent expands, Child2's expanded state is honored and Grandchild becomes visible)
- This is critical for correctness - visibility depends on all ancestors being expanded, not just the node itself

## STEP 5: Create the Test Category Structure

### What to Change

Add the `TEST_CATEGORY` wrapper around all four test cases at the end of `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, before the closing brace of `TEST_FILE`.

**Code to add:**

```cpp
TEST_CATEGORY(L"OperationsOnInvisibleNodes")
{
	// TEST_CASE(L"AddChildrenToCollapsedNode") from STEP 1
	// TEST_CASE(L"RemoveChildrenFromCollapsedNode") from STEP 2
	// TEST_CASE(L"MultipleModificationsToInvisibleSubtree") from STEP 3
	// TEST_CASE(L"VisibilityIndexForInvisibleNodes") from STEP 4
});
```

### Why This Change is Necessary

The `TEST_CATEGORY` groups related test cases together for better organization and reporting. All four test cases address the same concern: operations on invisible nodes should not affect the visible state of `NodeItemProvider`.

# TEST PLAN

## Overview

The test plan covers four comprehensive test cases that verify the isolation between visible and invisible portions of the tree in `tree::NodeItemProvider`. These tests ensure that operations on invisible nodes (nodes in collapsed subtrees) do not affect the visible state or trigger callbacks.

## Test Case 1: AddChildrenToCollapsedNode

**Purpose**: Verify that adding children to a collapsed (invisible) node does not affect `NodeItemProvider`.

**Setup**:
- Create a 3-level tree: Root -> Parent (expanded) -> Child (collapsed)
- Attach callback monitor to `NodeItemProvider`
- Child node is visible but collapsed (has no visible children)

**Test Steps**:
1. Verify initial state: Parent and Child visible, Count() = 2
2. Add 2 grandchildren to the collapsed Child node
3. Verify no callbacks triggered (callbackLog.Count() == 0)
4. Verify Count() unchanged (still 2)
5. Verify visible structure unchanged (still Parent, Child)
6. Verify grandchildren return -1 from `CalculateNodeVisibilityIndex()`
7. Expand Child node
8. Verify Count() increases to 4
9. Verify grandchildren become visible with correct indices
10. Verify expansion callback triggered with correct parameters

**Expected Outcome**:
- No callbacks during invisible modifications
- Correct structure after expansion
- This is the most common scenario: lazy loading children

## Test Case 2: RemoveChildrenFromCollapsedNode

**Purpose**: Verify that removing children from a collapsed node does not affect `NodeItemProvider`.

**Setup**:
- Create a 3-level tree where Child already has 2 grandchildren
- Root -> Parent (expanded) -> Child (collapsed with Grandchild1, Grandchild2)
- Attach callback monitor

**Test Steps**:
1. Verify initial state: Parent and Child visible, Count() = 2
2. Verify grandchildren are invisible (return -1)
3. Remove Grandchild1 from collapsed Child node
4. Verify no callbacks triggered
5. Verify Count() unchanged (still 2)
6. Verify visible structure unchanged
7. Expand Child node
8. Verify only Grandchild2 becomes visible (Grandchild1 was removed)
9. Verify Count() = 3
10. Verify expansion callback triggered
11. Verify removed node remains invisible

**Expected Outcome**:
- Removal operations on invisible nodes are isolated
- Removal is correctly reflected after expansion

## Test Case 3: MultipleModificationsToInvisibleSubtree

**Purpose**: Verify that multiple operations on invisible nodes accumulate correctly.

**Setup**:
- Create a 4-level deep tree: Root -> A (expanded) -> B (collapsed) -> C -> D
- Attach callback monitor
- B, C, D are all invisible

**Test Steps**:
1. Verify initial state: A and B visible, Count() = 2
2. Verify C and D are invisible
3. Add B-Child1 and B-Child2 to B
4. Add C-Child1 to C
5. Remove D from C
6. Verify no callbacks triggered during all modifications
7. Verify Count() unchanged (still 2)
8. Expand B
9. Verify B's children and C become visible (Count() = 5)
10. Verify correct callback for B's expansion
11. Expand C
12. Verify C-Child1 becomes visible (D does not, it was removed)
13. Verify Count() = 6
14. Verify correct callback for C's expansion
15. Verify D remains invisible (return -1)

**Expected Outcome**:
- Multiple cumulative modifications work correctly
- Sequential expansion reveals correct structure
- Tests deeply nested invisible modifications

## Test Case 4: VisibilityIndexForInvisibleNodes

**Purpose**: Verify that invisible nodes consistently return -1 from `CalculateNodeVisibilityIndex()` regardless of their own expand state.

**Setup**:
- Create tree with mixed expand states: Root -> Parent (collapsed) -> Child1 (collapsed), Child2 (expanded with Grandchild)
- Child2 is expanded but invisible (because Parent is collapsed)
- Attach callback monitor

**Test Steps**:
1. Verify only Parent visible, Count() = 1
2. Verify Child1, Child2, and Grandchild all return -1 (all invisible)
3. Note: Child2 is expanded, but still invisible because Parent is collapsed
4. Verify Parent returns valid index (0)
5. Expand Parent
6. Verify Count() = 4 (Parent, Child1, Child2, Grandchild)
7. Verify Child2's expanded state was preserved (Grandchild is visible)
8. Verify all four nodes have correct visibility indices (0, 1, 2, 3)

**Expected Outcome**:
- Expand state of invisible nodes doesn't affect visibility calculation
- All nodes in collapsed subtree return -1
- Expand state is preserved for when parent is later expanded

## Corner Cases Covered

1. **Lazy Loading**: Test 1 covers the common pattern of loading children on demand
2. **Removal from Invisible Nodes**: Test 2 ensures removal operations are isolated
3. **Deep Modifications**: Test 3 tests multiple levels of nesting
4. **State Preservation**: Test 4 ensures expand state is preserved for invisible nodes
5. **Callback Isolation**: All tests verify no callbacks during invisible modifications
6. **Count() Stability**: All tests verify Count() doesn't change during invisible modifications

## Cases NOT Tested (By Design)

Based on learnings from Task 3 and warnings in the task description:

1. **Do NOT test `SetExpanding()` on invisible nodes** - the semantics are unclear and implementation-dependent
2. **Avoid ambiguous edge cases** - focus on clear, practical scenarios
3. **State preservation already tested** - Task 3's `CollapseNodeWithExpandedDescendants` already covers this

## Existing Test Coverage

From Tasks 1-4 in the same file:
- **Task 1**: Basic setup and construction
- **Task 2**: Basic index mapping with static trees
- **Task 3**: Expand/collapse dynamics (including state preservation)
- **Task 4**: Edge cases including dynamic child addition/removal on **expanded** nodes

Task 5 fills the gap by testing operations on **collapsed/invisible** nodes.

## Integration with Existing Tests

- All test cases follow the same patterns as Tasks 1-4
- Use the same helper functions: `CreateTreeViewItem()`, `MockItemProviderCallback`, `AssertCallbacks()`, `AssertItems()`
- Follow the same code style: direct method calls, interface comments, blank line separation
- Organized in a single `TEST_CATEGORY` like other test groups

## Expected Line Count

- Test Case 1: ~90 lines
- Test Case 2: ~95 lines
- Test Case 3: ~115 lines
- Test Case 4: ~85 lines
- Category wrapper: ~5 lines
- **Total: ~390 lines**

This exceeds the "a few hundred lines" mentioned in the task description but is necessary for comprehensive coverage with proper comments and clear structure.

# !!!FINISHED!!!
