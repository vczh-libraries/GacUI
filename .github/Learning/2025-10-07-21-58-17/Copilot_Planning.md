# !!!PLANNING!!!

# UPDATES

(No updates yet - starting fresh from task document)

# IMPROVEMENT PLAN

## STEP 1: Add Test Category for ExpandCollapseDynamics

Add a new `TEST_CATEGORY(L"ExpandCollapseDynamics")` to `TestItemProviders_NodeItemProvider.cpp` after the existing `BasicIndexMapping` test category. This category will contain all test cases for expand/collapse dynamics.

**Code Change**: Insert at the end of the file (after line 403, before the closing `}` of `TEST_FILE`):

```cpp
	TEST_CATEGORY(L"ExpandCollapseDynamics")
	{
		// Test cases will be added here
	});
```

**Why**: This organizes all dynamic behavior tests under a single category, making it clear what aspect of `NodeItemProvider` is being tested and aligning with the existing test structure.

## STEP 2: Implement BasicSingleNodeExpandCollapse Test Case

Add a test case that verifies the most fundamental expand/collapse behavior: expanding a collapsed parent node to reveal its children, then collapsing it to hide them again.

**Code Change**: Add inside the `ExpandCollapseDynamics` category:

```cpp
		TEST_CASE(L"BasicSingleNodeExpandCollapse")
		{
			// Setup callback logging
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			// Setup: Create root provider with a parent node that has 2 children
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child1 = CreateTreeViewItem(L"Child1");
			auto child2 = CreateTreeViewItem(L"Child2");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child1);
			parentMemoryNode->Children().Add(child2);
			
			// Create NodeItemProvider - parent starts collapsed by default
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			nodeItemProvider->AttachCallback(&itemCallback);
			callbackLog.Clear(); // Clear attachment callback
			
			// Verify initial state - only parent visible through IItemProvider interface
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			const wchar_t* expectedCollapsed[] = { L"Parent" };
			AssertItems(nodeItemProvider, expectedCollapsed);
			
			// Test expanding the parent
			parent->SetExpanding(true);
			
			// Verify expanded state through IItemProvider interface
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			const wchar_t* expectedExpanded[] = { L"Parent", L"Child1", L"Child2" };
			AssertItems(nodeItemProvider, expectedExpanded);
			
			// Verify callback was triggered with correct parameters
			// base = 0 (parent's index), so children inserted at base + 1 = 1
			// count = 0 (no items were there), newCount = 2 (two children added)
			const wchar_t* expandCallbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandCallbacks);
			callbackLog.Clear();
			
			// Verify indices through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == 2);
			
			// Test collapsing the parent
			parent->SetExpanding(false);
			
			// Verify collapsed state through IItemProvider interface
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			AssertItems(nodeItemProvider, expectedCollapsed);
			
			// Verify callback was triggered with correct parameters
			// base = 0, so children removed from base + 1 = 1
			// count = 2 (two children were there), newCount = 0 (removed)
			const wchar_t* collapseCallbacks[] = {
				L"OnItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseCallbacks);
			
			// Verify child indices after collapse through INodeItemView interface
			// Children are no longer visible, so they should return -2 (invisible)
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == -2);
		});
```

**Why**: This is the foundational test that validates the core expand/collapse mechanism. It verifies:
- `Count()` changes correctly (1 → 3 → 1)
- Visible node sequence updates correctly using `AssertItems()`
- Callback parameters are correct for both expand and collapse operations
- Index calculations work for both visible and invisible nodes

## STEP 3: Implement ExpandCollapseAtDifferentPositions Test Case

Add a test case that verifies expand/collapse operations work correctly when the target node is at the beginning, middle, or end of the visible list, ensuring that subsequent nodes' indices adjust properly.

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"ExpandCollapseAtDifferentPositions")
		{
			// Setup callback logging
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			// Setup: Create 3 sibling nodes, each with 2 children, all collapsed
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node1Child1 = CreateTreeViewItem(L"Node1Child1");
			auto node1Child2 = CreateTreeViewItem(L"Node1Child2");
			
			auto node2 = CreateTreeViewItem(L"Node2");
			auto node2Child1 = CreateTreeViewItem(L"Node2Child1");
			auto node2Child2 = CreateTreeViewItem(L"Node2Child2");
			
			auto node3 = CreateTreeViewItem(L"Node3");
			auto node3Child1 = CreateTreeViewItem(L"Node3Child1");
			auto node3Child2 = CreateTreeViewItem(L"Node3Child2");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			rootMemoryNode->Children().Add(node3);
			
			auto node1MemoryNode = rootProvider->GetMemoryNode(node1.Obj());
			node1MemoryNode->Children().Add(node1Child1);
			node1MemoryNode->Children().Add(node1Child2);
			
			auto node2MemoryNode = rootProvider->GetMemoryNode(node2.Obj());
			node2MemoryNode->Children().Add(node2Child1);
			node2MemoryNode->Children().Add(node2Child2);
			
			auto node3MemoryNode = rootProvider->GetMemoryNode(node3.Obj());
			node3MemoryNode->Children().Add(node3Child1);
			node3MemoryNode->Children().Add(node3Child2);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			nodeItemProvider->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Initial state: 3 collapsed nodes
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			const wchar_t* expectedInitial[] = { L"Node1", L"Node2", L"Node3" };
			AssertItems(nodeItemProvider, expectedInitial);
			
			// Test 1: Expand at beginning (Node1 at index 0)
			node1->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 5);
			const wchar_t* expectedAfterNode1[] = { 
				L"Node1", L"Node1Child1", L"Node1Child2", L"Node2", L"Node3" 
			};
			AssertItems(nodeItemProvider, expectedAfterNode1);
			
			// Verify callback: children inserted at position 1
			const wchar_t* expandNode1Callbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandNode1Callbacks);
			callbackLog.Clear();
			
			// Verify subsequent nodes shifted correctly
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node2.Obj()) == 3);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node3.Obj()) == 4);
			
			// Test 2: Expand in middle (Node2 at index 3)
			node2->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 7);
			const wchar_t* expectedAfterNode2[] = { 
				L"Node1", L"Node1Child1", L"Node1Child2", 
				L"Node2", L"Node2Child1", L"Node2Child2", 
				L"Node3" 
			};
			AssertItems(nodeItemProvider, expectedAfterNode2);
			
			// Verify callback: children inserted at position 4 (3 + 1)
			const wchar_t* expandNode2Callbacks[] = {
				L"OnItemModified(start=4, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandNode2Callbacks);
			callbackLog.Clear();
			
			// Verify subsequent node shifted correctly
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node3.Obj()) == 6);
			
			// Test 3: Expand at end (Node3 at index 6)
			node3->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 9);
			const wchar_t* expectedAfterNode3[] = { 
				L"Node1", L"Node1Child1", L"Node1Child2", 
				L"Node2", L"Node2Child1", L"Node2Child2", 
				L"Node3", L"Node3Child1", L"Node3Child2" 
			};
			AssertItems(nodeItemProvider, expectedAfterNode3);
			
			// Verify callback: children inserted at position 7 (6 + 1)
			const wchar_t* expandNode3Callbacks[] = {
				L"OnItemModified(start=7, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandNode3Callbacks);
			callbackLog.Clear();
			
			// Test 4: Collapse in middle (Node2 at index 3)
			node2->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 7);
			const wchar_t* expectedAfterCollapseNode2[] = { 
				L"Node1", L"Node1Child1", L"Node1Child2", 
				L"Node2", 
				L"Node3", L"Node3Child1", L"Node3Child2" 
			};
			AssertItems(nodeItemProvider, expectedAfterCollapseNode2);
			
			// Verify callback: children removed from position 4
			const wchar_t* collapseNode2Callbacks[] = {
				L"OnItemModified(start=4, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseNode2Callbacks);
			callbackLog.Clear();
			
			// Test 5: Collapse at beginning (Node1 at index 0)
			node1->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 5);
			const wchar_t* expectedAfterCollapseNode1[] = { 
				L"Node1", L"Node2", L"Node3", L"Node3Child1", L"Node3Child2" 
			};
			AssertItems(nodeItemProvider, expectedAfterCollapseNode1);
			
			// Verify callback: children removed from position 1
			const wchar_t* collapseNode1Callbacks[] = {
				L"OnItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseNode1Callbacks);
			callbackLog.Clear();
			
			// Test 6: Collapse at end (Node3 at index 2)
			node3->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			AssertItems(nodeItemProvider, expectedInitial);
			
			// Verify callback: children removed from position 3 (2 + 1)
			const wchar_t* collapseNode3Callbacks[] = {
				L"OnItemModified(start=3, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseNode3Callbacks);
		});
```

**Why**: This test ensures that expand/collapse operations are position-independent. The index calculations in `OnItemExpanded` and `OnItemCollapsed` use `CalculateNodeVisibilityIndexInternal` which recursively computes the position. This test verifies that:
- Expanding a node at any position correctly inserts children at `nodeIndex + 1`
- Collapsing a node at any position correctly removes children from `nodeIndex + 1`
- Subsequent nodes' indices shift correctly after each operation
- Callback parameters reflect the correct start position based on node location

## STEP 4: Implement NestedExpandCollapse Test Case

Add a test case that verifies expand/collapse operations on nested (multi-level) tree structures, ensuring that the entire visible index range updates correctly when operating on parent-child-grandchild relationships.

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"NestedExpandCollapse")
		{
			// Setup callback logging
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			// Setup: Create a 3-level tree structure
			// Parent -> Child -> Grandchild
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child = CreateTreeViewItem(L"Child");
			auto grandchild = CreateTreeViewItem(L"Grandchild");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child);
			
			auto childMemoryNode = rootProvider->GetMemoryNode(child.Obj());
			childMemoryNode->Children().Add(grandchild);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			nodeItemProvider->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Initial state: all collapsed, only parent visible
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			const wchar_t* expectedOnlyParent[] = { L"Parent" };
			AssertItems(nodeItemProvider, expectedOnlyParent);
			
			// Step 1: Expand parent - child becomes visible
			parent->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 2);
			const wchar_t* expectedParentChild[] = { L"Parent", L"Child" };
			AssertItems(nodeItemProvider, expectedParentChild);
			
			const wchar_t* expandParentCallbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandParentCallbacks);
			callbackLog.Clear();
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -2); // Still invisible
			
			// Step 2: Expand child - grandchild becomes visible
			child->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			const wchar_t* expectedAll[] = { L"Parent", L"Child", L"Grandchild" };
			AssertItems(nodeItemProvider, expectedAll);
			
			const wchar_t* expandChildCallbacks[] = {
				L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandChildCallbacks);
			callbackLog.Clear();
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == 2);
			
			// Step 3: Collapse child - grandchild disappears
			child->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 2);
			AssertItems(nodeItemProvider, expectedParentChild);
			
			const wchar_t* collapseChildCallbacks[] = {
				L"OnItemModified(start=2, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseChildCallbacks);
			callbackLog.Clear();
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -2); // Invisible again
			
			// Step 4: Collapse parent - both child and grandchild disappear
			parent->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			AssertItems(nodeItemProvider, expectedOnlyParent);
			
			const wchar_t* collapseParentCallbacks[] = {
				L"OnItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseParentCallbacks);
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -2);
		});
```

**Why**: This test verifies the recursive nature of visible index calculations in nested structures. The implementation uses `CalculateNodeVisibilityIndexInternal` which:
- Returns -2 if any ancestor is collapsed
- Recursively computes position by summing visible nodes in previous siblings

This test ensures that:
- Expanding nodes at different levels correctly updates the visible range
- Collapsing a parent hides all its descendants regardless of their expand state
- Index calculations correctly handle multi-level recursion
- Callbacks accurately report the number of added/removed nodes at each level

## STEP 5: Implement MultipleSequentialOperations Test Case

Add a test case that performs multiple expand/collapse operations in sequence and verifies that the state remains consistent throughout, ensuring that the visible index mapping is maintained correctly across complex operation sequences.

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"MultipleSequentialOperations")
		{
			// Setup: Create a more complex tree
			//   A
			//   ├── B
			//   │   ├── B1
			//   │   └── B2
			//   └── C
			//       ├── C1
			//       └── C2
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto nodeA = CreateTreeViewItem(L"A");
			auto nodeB = CreateTreeViewItem(L"B");
			auto nodeB1 = CreateTreeViewItem(L"B1");
			auto nodeB2 = CreateTreeViewItem(L"B2");
			auto nodeC = CreateTreeViewItem(L"C");
			auto nodeC1 = CreateTreeViewItem(L"C1");
			auto nodeC2 = CreateTreeViewItem(L"C2");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(nodeA);
			
			auto nodeAMemoryNode = rootProvider->GetMemoryNode(nodeA.Obj());
			nodeAMemoryNode->Children().Add(nodeB);
			nodeAMemoryNode->Children().Add(nodeC);
			
			auto nodeBMemoryNode = rootProvider->GetMemoryNode(nodeB.Obj());
			nodeBMemoryNode->Children().Add(nodeB1);
			nodeBMemoryNode->Children().Add(nodeB2);
			
			auto nodeCMemoryNode = rootProvider->GetMemoryNode(nodeC.Obj());
			nodeCMemoryNode->Children().Add(nodeC1);
			nodeCMemoryNode->Children().Add(nodeC2);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Operation sequence 1: Expand A -> Expand B -> Expand C
			nodeA->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 3); // A, B, C
			const wchar_t* expected1[] = { L"A", L"B", L"C" };
			AssertItems(nodeItemProvider, expected1);
			
			nodeB->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 5); // A, B, B1, B2, C
			const wchar_t* expected2[] = { L"A", L"B", L"B1", L"B2", L"C" };
			AssertItems(nodeItemProvider, expected2);
			
			nodeC->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 7); // A, B, B1, B2, C, C1, C2
			const wchar_t* expected3[] = { L"A", L"B", L"B1", L"B2", L"C", L"C1", L"C2" };
			AssertItems(nodeItemProvider, expected3);
			
			// Verify all indices through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB1.Obj()) == 2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB2.Obj()) == 3);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == 4);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC1.Obj()) == 5);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC2.Obj()) == 6);
			
			// Verify bidirectional consistency through INodeItemView interface
			for (vint i = 0; i < nodeItemProvider->Count(); i++)
			{
				auto node = nodeItemProvider->RequestNode(i);
				TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
			}
			
			// Operation sequence 2: Collapse B -> Collapse A -> Expand A
			nodeB->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 5); // A, B, C, C1, C2
			const wchar_t* expected4[] = { L"A", L"B", L"C", L"C1", L"C2" };
			AssertItems(nodeItemProvider, expected4);
			
			// Verify B's children are invisible through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB1.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB2.Obj()) == -2);
			
			nodeA->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 1); // Only A
			const wchar_t* expected5[] = { L"A" };
			AssertItems(nodeItemProvider, expected5);
			
			// Verify all descendants are invisible through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC1.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC2.Obj()) == -2);
			
			nodeA->SetExpanding(true);
			// B is collapsed, C is still expanded from earlier
			TEST_ASSERT(nodeItemProvider->Count() == 5); // A, B, C, C1, C2
			AssertItems(nodeItemProvider, expected4);
			
			// Verify bidirectional consistency after all operations
			for (vint i = 0; i < nodeItemProvider->Count(); i++)
			{
				auto node = nodeItemProvider->RequestNode(i);
				TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
			}
			
			// Operation sequence 3: Expand B again
			nodeB->SetExpanding(true);
			TEST_ASSERT(nodeItemProvider->Count() == 7); // Back to fully expanded
			AssertItems(nodeItemProvider, expected3);
			
			// Final verification: all indices correct
			for (vint i = 0; i < nodeItemProvider->Count(); i++)
			{
				auto node = nodeItemProvider->RequestNode(i);
				TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
			}
		});
```

**Why**: This test validates that the implementation maintains consistency across complex operation sequences. It verifies:
- Multiple sequential expand operations accumulate correctly
- Collapsing a parent hides all descendants, but their expand states are preserved
- Re-expanding a parent restores the previously set expand states of children
- Bidirectional consistency (index ↔ node) is maintained after every operation
- The implementation correctly handles the tree structure's invariants across many state changes

This is critical because real applications perform many expand/collapse operations during a user session, and any inconsistency would lead to UI bugs.

## STEP 6: Implement ExpandNodeWithExpandedChildren Test Case

Add a test case that verifies expanding a collapsed parent node correctly reveals all descendants when some of those descendants are already in an expanded state (though invisible).

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"ExpandNodeWithExpandedChildren")
		{
			// Setup: Create 3-level tree: Parent -> Child -> Grandchild
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child = CreateTreeViewItem(L"Child");
			auto grandchild = CreateTreeViewItem(L"Grandchild");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child);
			
			auto childMemoryNode = rootProvider->GetMemoryNode(child.Obj());
			childMemoryNode->Children().Add(grandchild);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Initial state: parent collapsed, child and grandchild invisible
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			const wchar_t* expectedOnlyParent[] = { L"Parent" };
			AssertItems(nodeItemProvider, expectedOnlyParent);
			
			// Expand child while parent is still collapsed
			// This sets child's expanding state, but child remains invisible
			child->SetExpanding(true);
			
			// Verify child is still invisible (no callback triggered, count unchanged)
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			AssertItems(nodeItemProvider, expectedOnlyParent);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -2);
			
			// Now expand parent - both child and grandchild should become visible
			// because child's expanding state was already set to true
			parent->SetExpanding(true);
			
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			const wchar_t* expectedAll[] = { L"Parent", L"Child", L"Grandchild" };
			AssertItems(nodeItemProvider, expectedAll);
			
			// Verify all nodes are visible with correct indices through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == 2);
		});
```

**Why**: This test verifies that expand states are preserved for invisible nodes. The implementation's `OnItemExpanded` method uses `CalculateTotalVisibleNodes()` which recursively counts visible nodes including those in already-expanded subtrees. This means:
- Setting a node's expand state while it's invisible doesn't trigger callbacks (because `CalculateNodeVisibilityIndexInternal` returns -2 when the parent is collapsed, and `OnItemExpanded` checks `if (base != -2)`)
- When the parent is expanded, `CalculateTotalVisibleNodes()` correctly counts all descendants that are already expanded
- The callback's `newCount` parameter reflects the total number of newly visible nodes, including those in expanded subtrees

This is important for maintaining consistency when users expand/collapse nodes in arbitrary order.

## STEP 7: Implement CollapseNodeWithExpandedDescendants Test Case

Add a test case that verifies collapsing a node correctly hides all of its descendants, regardless of whether those descendants are themselves expanded, and that re-expanding restores the previous expand states.

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"CollapseNodeWithExpandedDescendants")
		{
			// Setup callback logging
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			// Setup: Create 3-level tree all fully expanded
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child = CreateTreeViewItem(L"Child");
			auto grandchild = CreateTreeViewItem(L"Grandchild");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child);
			
			auto childMemoryNode = rootProvider->GetMemoryNode(child.Obj());
			childMemoryNode->Children().Add(grandchild);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			nodeItemProvider->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Expand all nodes - full tree visible
			parent->SetExpanding(true);
			child->SetExpanding(true);
			callbackLog.Clear(); // Clear expansion callbacks
			
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			const wchar_t* expectedAll[] = { L"Parent", L"Child", L"Grandchild" };
			AssertItems(nodeItemProvider, expectedAll);
			
			// Collapse parent - all descendants should disappear
			parent->SetExpanding(false);
			
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			const wchar_t* expectedOnlyParent[] = { L"Parent" };
			AssertItems(nodeItemProvider, expectedOnlyParent);
			
			// Verify callback indicates 2 items were removed (child and grandchild)
			const wchar_t* collapseCallbacks[] = {
				L"OnItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseCallbacks);
			callbackLog.Clear();
			
			// Verify all descendants are invisible through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == -2);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -2);
			
			// Re-expand parent - both child and grandchild should reappear
			// because their expand states were preserved
			parent->SetExpanding(true);
			
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			AssertItems(nodeItemProvider, expectedAll);
			
			// Verify callback indicates 2 items were added back
			const wchar_t* expandCallbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expandCallbacks);
			
			// Verify all nodes are visible again through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent.Obj()) == 0);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == 1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == 2);
		});
```

**Why**: This test verifies critical behavior for collapsing expanded subtrees. The implementation's `OnItemCollapsed` method iterates through all children and sums their `CalculateTotalVisibleNodes()`, which recursively counts all visible descendants. This means:
- Collapsing a node removes ALL visible descendants, not just direct children
- The callback's `count` parameter correctly reflects the total number of removed nodes
- Expand states of descendants are preserved (they're stored in each node's state, not in the provider)
- Re-expanding reveals descendants in their previous expand states

This is essential for user experience - when users collapse and re-expand a section, they expect the internal expand states to be preserved.

## STEP 8: Implement CallbackParametersInComplexScenarios Test Case

Add a comprehensive test case that specifically focuses on verifying callback parameters in various complex scenarios, ensuring that controls receive accurate information for UI updates.

**Code Change**: Add after the previous test case:

```cpp
		TEST_CASE(L"CallbackParametersInComplexScenarios")
		{
			// Setup callback logging
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			// Scenario 1: Expand node with varying numbers of children
			auto rootProvider1 = Ptr(new TreeViewItemRootProvider);
			auto parent1 = CreateTreeViewItem(L"Parent1");
			auto rootMemoryNode1 = rootProvider1->GetMemoryNode(rootProvider1->GetRootNode().Obj());
			rootMemoryNode1->Children().Add(parent1);
			
			auto parent1MemoryNode = rootProvider1->GetMemoryNode(parent1.Obj());
			// Add 5 children
			for (vint i = 0; i < 5; i++)
			{
				auto child = CreateTreeViewItem(WString::Unmanaged(L"Child") + itow(i));
				parent1MemoryNode->Children().Add(child);
			}
			
			auto nodeItemProvider1 = Ptr(new NodeItemProvider(rootProvider1));
			nodeItemProvider1->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			parent1->SetExpanding(true);
			
			// Verify callback for 5 children insertion at position 1
			const wchar_t* scenario1Callbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=5, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, scenario1Callbacks);
			callbackLog.Clear();
			
			// Scenario 2: Expand node in middle of list with siblings before and after
			auto rootProvider2 = Ptr(new TreeViewItemRootProvider);
			auto nodeA = CreateTreeViewItem(L"A");
			auto nodeB = CreateTreeViewItem(L"B");
			auto nodeC = CreateTreeViewItem(L"C");
			auto nodeBChild = CreateTreeViewItem(L"BChild");
			
			auto rootMemoryNode2 = rootProvider2->GetMemoryNode(rootProvider2->GetRootNode().Obj());
			rootMemoryNode2->Children().Add(nodeA);
			rootMemoryNode2->Children().Add(nodeB);
			rootMemoryNode2->Children().Add(nodeC);
			
			auto nodeBMemoryNode = rootProvider2->GetMemoryNode(nodeB.Obj());
			nodeBMemoryNode->Children().Add(nodeBChild);
			
			auto nodeItemProvider2 = Ptr(new NodeItemProvider(rootProvider2));
			nodeItemProvider2->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// nodeB is at index 1, so child inserted at index 2
			nodeB->SetExpanding(true);
			
			const wchar_t* scenario2Callbacks[] = {
				L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, scenario2Callbacks);
			callbackLog.Clear();
			
			// Scenario 3: Collapse node with deeply nested expanded children
			auto rootProvider3 = Ptr(new TreeViewItemRootProvider);
			auto level0 = CreateTreeViewItem(L"Level0");
			auto level1 = CreateTreeViewItem(L"Level1");
			auto level2 = CreateTreeViewItem(L"Level2");
			auto level3 = CreateTreeViewItem(L"Level3");
			
			auto rootMemoryNode3 = rootProvider3->GetMemoryNode(rootProvider3->GetRootNode().Obj());
			rootMemoryNode3->Children().Add(level0);
			
			auto level0MemoryNode = rootProvider3->GetMemoryNode(level0.Obj());
			level0MemoryNode->Children().Add(level1);
			
			auto level1MemoryNode = rootProvider3->GetMemoryNode(level1.Obj());
			level1MemoryNode->Children().Add(level2);
			
			auto level2MemoryNode = rootProvider3->GetMemoryNode(level2.Obj());
			level2MemoryNode->Children().Add(level3);
			
			auto nodeItemProvider3 = Ptr(new NodeItemProvider(rootProvider3));
			nodeItemProvider3->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Expand all levels
			level0->SetExpanding(true);
			level1->SetExpanding(true);
			level2->SetExpanding(true);
			callbackLog.Clear(); // Clear expansion callbacks
			
			TEST_ASSERT(nodeItemProvider3->Count() == 4);
			
			// Collapse level0 - should remove 3 descendants
			level0->SetExpanding(false);
			
			const wchar_t* scenario3Callbacks[] = {
				L"OnItemModified(start=1, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, scenario3Callbacks);
			callbackLog.Clear();
			
			TEST_ASSERT(nodeItemProvider3->Count() == 1);
			
			// Scenario 4: Multiple siblings with children
			auto rootProvider4 = Ptr(new TreeViewItemRootProvider);
			auto sibling1 = CreateTreeViewItem(L"Sibling1");
			auto sibling2 = CreateTreeViewItem(L"Sibling2");
			auto sibling1Child1 = CreateTreeViewItem(L"S1C1");
			auto sibling1Child2 = CreateTreeViewItem(L"S1C2");
			auto sibling2Child1 = CreateTreeViewItem(L"S2C1");
			
			auto rootMemoryNode4 = rootProvider4->GetMemoryNode(rootProvider4->GetRootNode().Obj());
			rootMemoryNode4->Children().Add(sibling1);
			rootMemoryNode4->Children().Add(sibling2);
			
			auto sibling1MemoryNode = rootProvider4->GetMemoryNode(sibling1.Obj());
			sibling1MemoryNode->Children().Add(sibling1Child1);
			sibling1MemoryNode->Children().Add(sibling1Child2);
			
			auto sibling2MemoryNode = rootProvider4->GetMemoryNode(sibling2.Obj());
			sibling2MemoryNode->Children().Add(sibling2Child1);
			
			auto nodeItemProvider4 = Ptr(new NodeItemProvider(rootProvider4));
			nodeItemProvider4->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Expand sibling1 - 2 children at position 1
			sibling1->SetExpanding(true);
			
			const wchar_t* scenario4aCallbacks[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, scenario4aCallbacks);
			callbackLog.Clear();
			
			// Now sibling2 is at index 3, expand it - 1 child at position 4
			sibling2->SetExpanding(true);
			
			const wchar_t* scenario4bCallbacks[] = {
				L"OnItemModified(start=4, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, scenario4bCallbacks);
			
			TEST_ASSERT(nodeItemProvider4->Count() == 5);
		});
```

**Why**: This test provides comprehensive verification of callback parameters across diverse scenarios:
1. **Varying child counts**: Ensures `newCount` accurately reflects the number of children
2. **Position in middle of list**: Verifies `start` parameter accounts for preceding nodes
3. **Deeply nested structures**: Confirms `count` parameter includes all removed descendants
4. **Multiple siblings**: Tests that expanding one sibling correctly shifts subsequent siblings' positions

The callback parameters are critical because UI controls use them to efficiently update their display:
- `start`: Tells where to begin updating the UI
- `count`: How many old items to remove/invalidate
- `newCount`: How many new items to insert/create
- `itemReferenceUpdated`: Whether item references need refreshing (always `true` for expand/collapse)

Incorrect parameters would cause visual glitches, crashes, or incorrect selection/scrolling behavior.

# TEST PLAN

## Overview

All test cases added in the improvement plan are themselves the test plan. The 8 test cases in the `ExpandCollapseDynamics` category comprehensively cover the dynamic behavior of expand/collapse operations.

## Test Coverage Summary

### 1. BasicSingleNodeExpandCollapse
- **Covers**: Fundamental expand/collapse mechanism
- **Tests**: Count changes, visible node sequence, callback parameters, index calculations for visible and invisible nodes
- **Edge Cases**: Single parent with multiple children, toggle operations

### 2. ExpandCollapseAtDifferentPositions
- **Covers**: Position independence of expand/collapse operations
- **Tests**: Operations at beginning, middle, and end of visible list; index shifting of subsequent nodes
- **Edge Cases**: Boundary positions (first, last), interleaved expand/collapse operations

### 3. NestedExpandCollapse
- **Covers**: Multi-level tree operations
- **Tests**: Recursive index calculations, visibility propagation through levels, parent-child relationships
- **Edge Cases**: 3-level nesting, sequential operations at different levels

### 4. MultipleSequentialOperations
- **Covers**: State consistency across operation sequences
- **Tests**: Complex operation sequences, expand state preservation, bidirectional index consistency
- **Edge Cases**: Collapse parent then re-expand (state restoration), mixed expand/collapse sequences

### 5. ExpandNodeWithExpandedChildren
- **Covers**: Expand state preservation for invisible nodes
- **Tests**: Setting expand state while invisible, revealing pre-expanded subtrees
- **Edge Cases**: Child expanded before parent, multi-level pre-expansion

### 6. CollapseNodeWithExpandedDescendants
- **Covers**: Complete subtree hiding
- **Tests**: Collapsing removes all visible descendants, re-expanding restores expand states
- **Edge Cases**: Fully expanded multi-level tree collapse, state restoration after re-expansion

### 7. CallbackParametersInComplexScenarios
- **Covers**: Callback parameter accuracy in diverse scenarios
- **Tests**: Various child counts, different positions, deeply nested structures, multiple siblings
- **Edge Cases**: Large child counts (5 children), 4-level nesting, sibling position updates

## Verification Methods

Each test case uses multiple verification approaches:

1. **Count Verification**: `TEST_ASSERT(nodeItemProvider->Count() == expectedCount)`
   - Ensures the visible node count is correct after each operation

2. **Sequence Verification**: `AssertItems(nodeItemProvider, expectedArray)`
   - Validates the complete visible node sequence matches expectations
   - Catches ordering bugs

3. **Index Calculation Verification**: `TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == expectedIndex)`
   - Tests both visible nodes (returns index) and invisible nodes (returns -2)
   - Verifies the index calculation algorithm

4. **Bidirectional Consistency**: Loop checking `RequestNode(i)` and `CalculateNodeVisibilityIndex(node)` match
   - Ensures the mapping is consistent in both directions
   - Critical for control functionality

5. **Callback Verification**: `AssertCallbacks(callbackLog, expectedCallbacks)`
   - Validates that callbacks are triggered with correct parameters
   - Tests the event notification system

## Corner Cases Addressed

1. **Empty Children**: Not explicitly tested as existing tests cover this (empty tree tests in BasicIndexMapping)
2. **Single Child**: Covered in multiple tests (NestedExpandCollapse, CallbackParametersInComplexScenarios)
3. **Many Children**: Covered in CallbackParametersInComplexScenarios (5 children)
4. **Deep Nesting**: Covered in CallbackParametersInComplexScenarios (4 levels)
5. **Sibling Interactions**: Covered in ExpandCollapseAtDifferentPositions and CallbackParametersInComplexScenarios
6. **State Preservation**: Covered in MultipleSequentialOperations, ExpandNodeWithExpandedChildren, CollapseNodeWithExpandedDescendants
7. **Invisible Node Operations**: Covered in ExpandNodeWithExpandedChildren (expand while invisible)
8. **Boundary Positions**: Covered in ExpandCollapseAtDifferentPositions (first, middle, last)

## Integration with Existing Tests

The new test category complements the existing `BasicIndexMapping` category:
- **BasicIndexMapping**: Tests static tree structures with different expand states
- **ExpandCollapseDynamics**: Tests dynamic state changes

Together, they provide complete coverage of `NodeItemProvider`'s functionality:
- Static behavior: Index mapping for pre-configured trees
- Dynamic behavior: Index mapping updates during runtime state changes

## Expected Test Results

All test cases should pass without errors. The implementation in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` is already complete, so these tests verify that the existing implementation is correct.

If any test fails, it would indicate either:
1. A bug in the implementation (unlikely, as it's production code)
2. A misunderstanding of the implementation's behavior in the test design
3. Edge cases not properly handled by the implementation

## Maintenance Considerations

These tests are designed to be maintainable:
- **Clear Structure**: Each test case has a single, well-defined purpose
- **Readable Assertions**: `AssertItems()` and `AssertCallbacks()` helpers make expectations clear
- **Comments**: Interface annotations explain which API is being tested
- **Consistent Pattern**: All tests follow the same setup-action-verify pattern
- **Self-Documenting**: Test names clearly describe what is being tested

If the implementation changes (e.g., callback parameter format changes), the tests can be easily updated by modifying the expected callback strings in one place.

# !!!FINISHED!!!
