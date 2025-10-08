# !!!PLANNING!!!

# UPDATES

## UPDATE

I believe passing invalid index to GetTextValue and GetBindingValue should trigger CHECK_ERROR. If the source code is not written in this way, you will need to fix it. Please confirm

You must use the ERROR_MESSAGE_PREFIX pattern with CHECK_ERROR and use TEST_ERROR in test cases for index out of bound situation. In ERROR_MESSAGE_PREFIX, the method name should also come with full namespace.

# IMPROVEMENT PLAN

## STEP 1: Fix GetTextValue and GetBindingValue to Use CHECK_ERROR

The current implementation of `NodeItemProvider::GetTextValue()` and `NodeItemProvider::GetBindingValue()` returns default values for invalid indices instead of raising errors. This is inconsistent with other `IItemProvider` implementations like `ListViewItemProvider`, which use `CHECK_ERROR` for index validation.

**Current code in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` (lines 215-231)**:

```cpp
WString NodeItemProvider::GetTextValue(vint itemIndex)
{
	if (auto node = RequestNode(itemIndex))
	{
		return root->GetTextValue(node.Obj());
	}
	return L"";
}

description::Value NodeItemProvider::GetBindingValue(vint itemIndex)
{
	if (auto node = RequestNode(itemIndex))
	{
		return root->GetBindingValue(node.Obj());
	}
	return Value();
}
```

**Fixed code**:

```cpp
WString NodeItemProvider::GetTextValue(vint itemIndex)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::tree::NodeItemProvider::GetTextValue(vint)#"
	CHECK_ERROR(0 <= itemIndex && itemIndex < Count(), ERROR_MESSAGE_PREFIX L"Index out of range.");
	return root->GetTextValue(node.Obj());
#undef ERROR_MESSAGE_PREFIX
}

description::Value NodeItemProvider::GetBindingValue(vint itemIndex)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::tree::NodeItemProvider::GetBindingValue(vint)#"
	CHECK_ERROR(0 <= itemIndex && itemIndex < Count(), ERROR_MESSAGE_PREFIX L"Index out of range.");
	return root->GetBindingValue(node.Obj());
#undef ERROR_MESSAGE_PREFIX
}
```

**Why**: 
- Consistency with other `IItemProvider` implementations (e.g., `ListViewItemProvider::GetText()` calls `Get(itemIndex)` which uses `CHECK_ERROR`)
- From `Import\Vlpp.h` line 2044: `ArrayBase::Get()` uses `CHECK_ERROR(index >= 0 && index < this->count, ...)` 
- Defensive programming: These methods should fail fast on programming errors (invalid indices) rather than silently returning default values
- The ERROR_MESSAGE_PREFIX pattern is standard practice in this codebase (see `ItemProvider_Binding.cpp` line 110 and `GuiListControls.cpp` line 93)
- Tests expect `TEST_ERROR` for invalid indices (as seen in `TestItemProviders_ListViewItemProvider.cpp` lines 725-728)

## STEP 2: Add Test Category "EdgeCasesAndComplexScenarios"

Add a new `TEST_CATEGORY(L"EdgeCasesAndComplexScenarios")` block at the end of the test file, after the `ExpandCollapseDynamics` category (after line 1042).

**Why**: This organizes all edge case tests into a single category, following the structure established by existing test categories.

## STEP 2: Implement Test Case "InvalidIndicesForDataRetrieval"

Add the first test case to verify that `GetTextValue()` and `GetBindingValue()` handle invalid indices gracefully.

**Code to add**:

```cpp
TEST_CASE(L"InvalidIndicesForDataRetrieval")
{
	// Setup: Simple flat tree with 3 nodes
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto node1 = CreateTreeViewItem(L"Node1");
	auto node2 = CreateTreeViewItem(L"Node2");
	auto node3 = CreateTreeViewItem(L"Node3");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(node1);
	rootMemoryNode->Children().Add(node2);
	rootMemoryNode->Children().Add(node3);
	
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	
	// Verify valid indices return correct data through IItemProvider interface
	TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Node1");
	TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Node2");
	TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Node3");
	
	// Verify GetTextValue() raises errors for invalid indices through IItemProvider interface
	TEST_ERROR(nodeItemProvider->GetTextValue(-1));
	TEST_ERROR(nodeItemProvider->GetTextValue(3));
	TEST_ERROR(nodeItemProvider->GetTextValue(nodeItemProvider->Count()));
	TEST_ERROR(nodeItemProvider->GetTextValue(999));
	
	// Verify GetBindingValue() raises errors for invalid indices through IItemProvider interface
	TEST_ERROR(nodeItemProvider->GetBindingValue(-1));
	TEST_ERROR(nodeItemProvider->GetBindingValue(3));
	TEST_ERROR(nodeItemProvider->GetBindingValue(nodeItemProvider->Count()));
	TEST_ERROR(nodeItemProvider->GetBindingValue(999));
});
```

**Why**: After fixing the implementation to use `CHECK_ERROR`, these methods must raise errors for invalid indices, consistent with other `IItemProvider` implementations like `ListViewItemProvider`. This follows the pattern established in `TestItemProviders_ListViewItemProvider.cpp` (lines 725-728) where `TEST_ERROR` is used to verify error handling. The test covers negative indices, indices at Count(), and large out-of-bounds indices to ensure comprehensive error detection.

**Lines**: ~30 lines

## STEP 4: Implement Test Case "CalculateIndexForForeignNode"

Add a test case to verify that `CalculateNodeVisibilityIndex()` returns -1 for nodes that don't belong to the tree.

**Code to add**:

```cpp
TEST_CASE(L"CalculateIndexForForeignNode")
{
	// Setup: Create two separate tree structures
	auto rootProviderA = Ptr(new TreeViewItemRootProvider);
	auto nodeA1 = CreateTreeViewItem(L"TreeA-Node1");
	auto nodeA2 = CreateTreeViewItem(L"TreeA-Node2");
	
	auto rootMemoryNodeA = rootProviderA->GetMemoryNode(rootProviderA->GetRootNode().Obj());
	rootMemoryNodeA->Children().Add(nodeA1);
	rootMemoryNodeA->Children().Add(nodeA2);
	
	auto rootProviderB = Ptr(new TreeViewItemRootProvider);
	auto nodeB1 = CreateTreeViewItem(L"TreeB-Node1");
	auto nodeB2 = CreateTreeViewItem(L"TreeB-Node2");
	
	auto rootMemoryNodeB = rootProviderB->GetMemoryNode(rootProviderB->GetRootNode().Obj());
	rootMemoryNodeB->Children().Add(nodeB1);
	rootMemoryNodeB->Children().Add(nodeB2);
	
	// Create NodeItemProvider for tree A
	auto nodeItemProviderA = Ptr(new NodeItemProvider(rootProviderA));
	
	// Verify nodes from tree A have valid indices through INodeItemView interface
	TEST_ASSERT(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeA1.Obj()) == 0);
	TEST_ASSERT(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeA2.Obj()) == 1);
	
	// Verify nodes from tree B return -1 (not found) through INodeItemView interface
	TEST_ASSERT(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB1.Obj()) == -1);
	TEST_ASSERT(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB2.Obj()) == -1);
});
```

**Why**: This tests a boundary condition where a valid node pointer is passed, but the node doesn't belong to the tree being queried. The implementation's `CalculateNodeVisibilityIndexInternal()` will traverse the parent chain and eventually return -1 when the node is not found in the expected parent's children list. This ensures the implementation doesn't crash or return incorrect indices for foreign nodes.

**Lines**: ~30 lines

## STEP 5: Implement Test Case "DeeplyNestedTreeIndexMapping"

Add a test case to verify index mapping works correctly in a 5-level deep tree with mixed expand states.

**Code to add**:

```cpp
TEST_CASE(L"DeeplyNestedTreeIndexMapping")
{
	// Setup: Create a 5-level deep tree
	// Root -> A -> B -> C -> D -> E
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto nodeA = CreateTreeViewItem(L"A");
	auto nodeB = CreateTreeViewItem(L"B");
	auto nodeC = CreateTreeViewItem(L"C");
	auto nodeD = CreateTreeViewItem(L"D");
	auto nodeE = CreateTreeViewItem(L"E");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(nodeA);
	
	auto nodeAMemoryNode = rootProvider->GetMemoryNode(nodeA.Obj());
	nodeAMemoryNode->Children().Add(nodeB);
	
	auto nodeBMemoryNode = rootProvider->GetMemoryNode(nodeB.Obj());
	nodeBMemoryNode->Children().Add(nodeC);
	
	auto nodeCMemoryNode = rootProvider->GetMemoryNode(nodeC.Obj());
	nodeCMemoryNode->Children().Add(nodeD);
	
	auto nodeDMemoryNode = rootProvider->GetMemoryNode(nodeD.Obj());
	nodeDMemoryNode->Children().Add(nodeE);
	
	// Set expansion states: A expanded, B collapsed, C and D expanded (but invisible due to B collapsed)
	nodeA->SetExpanding(true);
	nodeB->SetExpanding(false); // B is collapsed
	nodeC->SetExpanding(true);  // Expanded but invisible
	nodeD->SetExpanding(true);  // Expanded but invisible
	
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	
	// Verify only A and B are visible through IItemProvider interface
	const wchar_t* expected[] = {
		L"A",
		L"B"
	};
	AssertItems(nodeItemProvider, expected);
	
	// Verify visible nodes have correct indices through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == 1);
	
	// Verify nodes in collapsed subtree return -1 through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeD.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeE.Obj()) == -1);
	
	// Now expand all nodes to make the full 5-level hierarchy visible
	nodeB->SetExpanding(true);
	
	const wchar_t* expectedFullyExpanded[] = {
		L"A",
		L"B",
		L"C",
		L"D",
		L"E"
	};
	AssertItems(nodeItemProvider, expectedFullyExpanded);
	
	// Verify all nodes have correct sequential indices through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == 2);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeD.Obj()) == 3);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeE.Obj()) == 4);
	
	// Verify bidirectional consistency through INodeItemView interface
	for (vint i = 0; i < nodeItemProvider->Count(); i++)
	{
		auto node = nodeItemProvider->RequestNode(i);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
	}
});
```

**Why**: This tests scalability with deep nesting (5 levels). The mixed expand states ensure the implementation correctly handles invisible subtrees at various depths. The bidirectional consistency check verifies that `RequestNode()` and `CalculateNodeVisibilityIndex()` work correctly together even in deeply nested structures.

**Lines**: ~80 lines

## STEP 6: Implement Test Case "DynamicChildAdditionRemoval"

Add a test case to verify that dynamically adding and removing children to expanded nodes correctly updates indices and triggers callbacks.

**Code to add**:

```cpp
TEST_CASE(L"DynamicChildAdditionRemoval")
{
	// Setup callback logging
	List<WString> callbackLog;
	MockItemProviderCallback itemCallback(callbackLog);
	
	// Setup: Create tree with expanded parent
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto parent = CreateTreeViewItem(L"Parent");
	auto child1 = CreateTreeViewItem(L"Child1");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(parent);
	
	auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
	parentMemoryNode->Children().Add(child1);
	
	parent->SetExpanding(true);
	
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	nodeItemProvider->AttachCallback(&itemCallback);
	callbackLog.Clear(); // Clear the OnAttached callback
	
	// Verify initial state through IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	const wchar_t* expectedInitial[] = {
		L"Parent",
		L"Child1"
	};
	AssertItems(nodeItemProvider, expectedInitial);
	
	// Add a new child dynamically
	auto child2 = CreateTreeViewItem(L"Child2");
	parentMemoryNode->Children().Add(child2);
	
	// Verify Count() increased through IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 3);
	
	// Verify new child is visible at correct position through IItemProvider interface
	const wchar_t* expectedAfterAdd[] = {
		L"Parent",
		L"Child1",
		L"Child2"
	};
	AssertItems(nodeItemProvider, expectedAfterAdd);
	
	// Verify callback was triggered through IItemProviderCallback interface
	const wchar_t* addCallbacks[] = {
		L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, addCallbacks);
	callbackLog.Clear();
	
	// Verify new child has correct index through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == 2);
	
	// Remove the first child dynamically using INodeProvider interface
	parentMemoryNode->Children().RemoveAt(0);
	
	// Verify Count() decreased through IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	
	// Verify remaining children through IItemProvider interface
	const wchar_t* expectedAfterRemove[] = {
		L"Parent",
		L"Child2"
	};
	AssertItems(nodeItemProvider, expectedAfterRemove);
	
	// Verify callback was triggered through IItemProviderCallback interface
	const wchar_t* removeCallbacks[] = {
		L"OnItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, removeCallbacks);
	
	// Verify removed child now returns -1 through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == -1);
	
	// Verify child2 index updated through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == 1);
});
```

**Why**: This test verifies that the `OnBeforeItemModified()` and `OnAfterItemModified()` callbacks work correctly for dynamic tree modifications (as opposed to expand/collapse which uses different callbacks). This is a different code path from the expand/collapse operations tested in the `ExpandCollapseDynamics` category. The implementation tracks modifications through `offsetBeforeChildModifieds` dictionary and fires `InvokeOnItemModified()` with the correct parameters.

**Lines**: ~75 lines

## STEP 7: Implement Test Case "LargeTreeStressTest"

Add a test case to verify operations work correctly with many nodes (50+ nodes).

**Code to add**:

```cpp
TEST_CASE(L"LargeTreeStressTest")
{
	// Setup: Create tree with 10 top-level nodes, each with 10 children (110 nodes total when expanded)
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	
	List<Ptr<MemoryNodeProvider>> topLevelNodes;
	List<Ptr<MemoryNodeProvider>> allChildNodes;
	
	// Create 10 top-level nodes, each with 10 children
	for (vint i = 0; i < 10; i++)
	{
		auto topNode = CreateTreeViewItem(WString::Unmanaged(L"Top") + itow(i));
		rootMemoryNode->Children().Add(topNode);
		topLevelNodes.Add(topNode);
		
		auto topNodeMemory = rootProvider->GetMemoryNode(topNode.Obj());
		for (vint j = 0; j < 10; j++)
		{
			auto childNode = CreateTreeViewItem(WString::Unmanaged(L"Top") + itow(i) + L"-Child" + itow(j));
			topNodeMemory->Children().Add(childNode);
			allChildNodes.Add(childNode);
		}
		
		topNode->SetExpanding(true);
	}
	
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	
	// Verify Count() returns correct total through IItemProvider interface
	TEST_ASSERT(nodeItemProvider->Count() == 110); // 10 top-level + 100 children
	
	// Test RequestNode() at various positions through INodeItemView interface
	// First node
	auto firstNode = nodeItemProvider->RequestNode(0);
	TEST_ASSERT(firstNode != nullptr);
	TEST_ASSERT(rootProvider->GetTextValue(firstNode.Obj()) == L"Top0");
	
	// Middle node
	auto middleNode = nodeItemProvider->RequestNode(55);
	TEST_ASSERT(middleNode != nullptr);
	
	// Last node
	auto lastNode = nodeItemProvider->RequestNode(109);
	TEST_ASSERT(lastNode != nullptr);
	TEST_ASSERT(rootProvider->GetTextValue(lastNode.Obj()) == L"Top9-Child9");
	
	// Test CalculateNodeVisibilityIndex() for various nodes through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(topLevelNodes[0].Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(topLevelNodes[9].Obj()) == 99); // After 9 parents and 90 children
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(allChildNodes[0].Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(allChildNodes[99].Obj()) == 109);
	
	// Verify bidirectional consistency for sample nodes through INodeItemView interface
	List<vint> sampleIndices;
	sampleIndices.Add(0);
	sampleIndices.Add(1);
	sampleIndices.Add(11);
	sampleIndices.Add(55);
	sampleIndices.Add(99);
	sampleIndices.Add(109);
	
	for (vint i = 0; i < sampleIndices.Count(); i++)
	{
		auto node = nodeItemProvider->RequestNode(sampleIndices[i]);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == sampleIndices[i]);
	}
});
```

**Why**: This stress test ensures the implementation scales to real-world tree sizes (110 nodes). It verifies that `Count()`, `RequestNode()`, and `CalculateNodeVisibilityIndex()` all work correctly with many nodes and that there are no performance issues, integer overflows, or other scalability problems.

**Lines**: ~65 lines

## STEP 8: Implement Test Case "FullyExpandedVsFullyCollapsed"

Add a test case to verify correct behavior in extreme expand states.

**Code to add**:

```cpp
TEST_CASE(L"FullyExpandedVsFullyCollapsed")
{
	// Setup: Create 3-level tree
	// Root -> Parent1, Parent2
	//   Parent1 -> Child1-1, Child1-2
	//   Parent2 -> Child2-1, Child2-2
	auto rootProvider = Ptr(new TreeViewItemRootProvider);
	auto parent1 = CreateTreeViewItem(L"Parent1");
	auto parent2 = CreateTreeViewItem(L"Parent2");
	auto child11 = CreateTreeViewItem(L"Child1-1");
	auto child12 = CreateTreeViewItem(L"Child1-2");
	auto child21 = CreateTreeViewItem(L"Child2-1");
	auto child22 = CreateTreeViewItem(L"Child2-2");
	
	auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
	rootMemoryNode->Children().Add(parent1);
	rootMemoryNode->Children().Add(parent2);
	
	auto parent1MemoryNode = rootProvider->GetMemoryNode(parent1.Obj());
	parent1MemoryNode->Children().Add(child11);
	parent1MemoryNode->Children().Add(child12);
	
	auto parent2MemoryNode = rootProvider->GetMemoryNode(parent2.Obj());
	parent2MemoryNode->Children().Add(child21);
	parent2MemoryNode->Children().Add(child22);
	
	auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
	
	// Test fully collapsed state (all nodes collapsed)
	TEST_ASSERT(nodeItemProvider->Count() == 2);
	const wchar_t* expectedCollapsed[] = {
		L"Parent1",
		L"Parent2"
	};
	AssertItems(nodeItemProvider, expectedCollapsed);
	
	// Verify only top-level nodes have valid indices through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent1.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent2.Obj()) == 1);
	
	// Verify grandchildren return -1 through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child11.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child12.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child21.Obj()) == -1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child22.Obj()) == -1);
	
	// Test fully expanded state
	parent1->SetExpanding(true);
	parent2->SetExpanding(true);
	
	TEST_ASSERT(nodeItemProvider->Count() == 6);
	const wchar_t* expectedExpanded[] = {
		L"Parent1",
		L"Child1-1",
		L"Child1-2",
		L"Parent2",
		L"Child2-1",
		L"Child2-2"
	};
	AssertItems(nodeItemProvider, expectedExpanded);
	
	// Verify all nodes have valid indices through INodeItemView interface
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent1.Obj()) == 0);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child11.Obj()) == 1);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child12.Obj()) == 2);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent2.Obj()) == 3);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child21.Obj()) == 4);
	TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child22.Obj()) == 5);
	
	// Verify bidirectional consistency through INodeItemView interface
	for (vint i = 0; i < nodeItemProvider->Count(); i++)
	{
		auto node = nodeItemProvider->RequestNode(i);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
	}
});
```

**Why**: This tests the extreme boundary conditions of expand state. The fully collapsed state ensures that only direct children of the root are visible. The fully expanded state ensures all nodes in the tree are visible with correct sequential indexing. These are important edge cases that exercise the expand state logic comprehensively.

**Lines**: ~75 lines

# TEST PLAN

## Overview

All test cases will be added to a new `TEST_CATEGORY(L"EdgeCasesAndComplexScenarios")` block in `TestItemProviders_NodeItemProvider.cpp`. Each test case follows the established patterns from existing tests:

- Use `TreeViewItemRootProvider` and `NodeItemProvider`
- Use `MockItemProviderCallback` for event tracking (where needed)
- Use `AssertItems()` helper to verify visible node sequences
- Use `AssertCallbacks()` helper to verify event sequences
- Test through both `IItemProvider` and `INodeItemView` interfaces with clarifying comments
- Use `TEST_ERROR` to verify methods raise errors for invalid inputs

## Implementation Fix Validation

**Purpose**: Verify the implementation fix in STEP 1 works correctly.

**Approach**: Build the project after fixing `GetTextValue()` and `GetBindingValue()` to ensure no compilation errors. The test case in Test Case 1 will validate runtime behavior.

## Test Case 1: InvalidIndicesForDataRetrieval

**Purpose**: Verify `GetTextValue()` and `GetBindingValue()` raise errors for invalid indices using `CHECK_ERROR`.

**Setup**:
- Create simple flat tree with 3 nodes
- Create `NodeItemProvider`

**Test Steps**:
1. Verify valid indices return correct data
2. Test negative index (-1) with `TEST_ERROR`
3. Test index at Count() with `TEST_ERROR`
4. Test large index (999) with `TEST_ERROR`
5. Verify both `GetTextValue()` and `GetBindingValue()` raise errors for all invalid indices

**Expected Results**:
- Valid indices return correct node text
- Invalid indices trigger `CHECK_ERROR` in both methods
- Consistent behavior with other `IItemProvider` implementations like `ListViewItemProvider`

**Coverage**: Tests error handling in data retrieval methods after implementing `CHECK_ERROR` for index validation. Ensures consistency with established patterns (see `TestItemProviders_ListViewItemProvider.cpp` lines 725-728).

## Test Case 2: CalculateIndexForForeignNode

**Purpose**: Verify `CalculateNodeVisibilityIndex()` returns -1 for nodes from a different tree.

**Setup**:
- Create two separate `TreeViewItemRootProvider` instances (Tree A and Tree B)
- Create `NodeItemProvider` for Tree A
- Each tree has 2 nodes

**Test Steps**:
1. Verify nodes from Tree A have valid indices (0, 1)
2. Pass nodes from Tree B to Tree A's `CalculateNodeVisibilityIndex()`
3. Verify both return -1

**Expected Results**:
- Own tree's nodes return valid indices
- Foreign tree's nodes return -1

**Coverage**: Tests boundary condition where valid node pointer doesn't belong to the tree. Ensures implementation traverses parent chain correctly and detects foreign nodes.

## Test Case 3: DeeplyNestedTreeIndexMapping

**Purpose**: Verify index mapping works correctly in 5-level deep tree with mixed expand states.

**Setup**:
- Create 5-level linear tree: Root -> A -> B -> C -> D -> E
- Set expansion: A expanded, B collapsed, C and D expanded (but invisible)
- Create `NodeItemProvider`

**Test Steps**:
1. Verify only A and B are visible (collapsed B hides its subtree)
2. Verify A has index 0, B has index 1
3. Verify C, D, E return -1 (in collapsed subtree)
4. Expand B to make full hierarchy visible
5. Verify all 5 nodes have sequential indices (0-4)
6. Verify bidirectional consistency for all nodes

**Expected Results**:
- Before B expansion: Count=2, only A and B visible
- After B expansion: Count=5, all nodes visible sequentially
- Bidirectional consistency maintained

**Coverage**: Tests scalability with deep nesting (5 levels), mixed expand states, and correct handling of invisible subtrees at various depths.

## Test Case 4: DynamicChildAdditionRemoval

**Purpose**: Verify dynamically adding/removing children to expanded nodes correctly updates indices and triggers callbacks.

**Setup**:
- Create tree with expanded parent and 1 child
- Attach `MockItemProviderCallback`
- Create `NodeItemProvider`

**Test Steps**:
1. Verify initial state: Count=2, Parent and Child1 visible
2. Dynamically add Child2 using `Children().Add()`
3. Verify Count increased to 3
4. Verify Child2 is visible at correct position (index 2)
5. Verify callback fired: `OnItemModified(start=2, count=0, newCount=1)`
6. Remove Child1 using `Children().RemoveAt(0)`
7. Verify Count decreased to 2
8. Verify only Parent and Child2 remain
9. Verify callback fired: `OnItemModified(start=1, count=1, newCount=0)`
10. Verify Child1 now returns -1, Child2 has index 1

**Expected Results**:
- Adding child: Count increases, new child visible, correct callback
- Removing child: Count decreases, correct items remain, correct callback
- Indices update correctly after modifications

**Coverage**: Tests `OnBeforeItemModified()` and `OnAfterItemModified()` callbacks, which handle dynamic tree modifications (different code path from expand/collapse). Verifies `offsetBeforeChildModifieds` tracking works correctly.

## Test Case 5: LargeTreeStressTest

**Purpose**: Verify operations work correctly with many nodes (110 total).

**Setup**:
- Create 10 top-level nodes, each with 10 children (110 nodes total)
- Expand all top-level nodes
- Create `NodeItemProvider`

**Test Steps**:
1. Verify Count() returns 110
2. Test `RequestNode()` at first position (index 0)
3. Test `RequestNode()` at middle position (index 55)
4. Test `RequestNode()` at last position (index 109)
5. Verify text values for first and last nodes
6. Test `CalculateNodeVisibilityIndex()` for first top-level node (should be 0)
7. Test `CalculateNodeVisibilityIndex()` for last top-level node (should be 99)
8. Test `CalculateNodeVisibilityIndex()` for first child (should be 1)
9. Test `CalculateNodeVisibilityIndex()` for last child (should be 109)
10. Verify bidirectional consistency for sample indices: 0, 1, 11, 55, 99, 109

**Expected Results**:
- All operations work correctly with 110 nodes
- No performance issues or integer overflows
- Bidirectional consistency maintained

**Coverage**: Tests scalability to real-world tree sizes. Ensures no issues with larger trees that might not appear in smaller tests.

## Test Case 6: FullyExpandedVsFullyCollapsed

**Purpose**: Verify correct behavior in extreme expand states.

**Setup**:
- Create 3-level tree: 2 parents, each with 2 children (6 nodes total)
- Initially all collapsed
- Create `NodeItemProvider`

**Test Steps**:
1. **Fully Collapsed State:**
   - Verify Count=2 (only parents visible)
   - Verify parents have indices 0 and 1
   - Verify all children return -1
2. **Fully Expanded State:**
   - Expand both parents
   - Verify Count=6 (all nodes visible)
   - Verify all nodes have sequential indices (0-5)
   - Verify bidirectional consistency

**Expected Results**:
- Fully collapsed: Only top-level nodes visible
- Fully expanded: All nodes visible with sequential indexing
- Transitions between states work correctly

**Coverage**: Tests extreme boundary conditions of expand state. Ensures both minimal visibility (only top level) and maximal visibility (all nodes) work correctly.

## Testing Strategy

1. **Build the project** using the "Build Unit Tests" task
2. **Run all tests** using the "Run Unit Tests" task
3. **Verify all tests pass** without errors
4. **Check test coverage** to ensure all new code paths are exercised

## Corner Cases Covered

- **Invalid input handling**: Negative indices, out-of-bounds indices, large indices
- **Foreign node detection**: Nodes from different trees
- **Deep nesting**: 5-level hierarchy
- **Mixed expand states**: Some levels expanded, some collapsed
- **Dynamic modifications**: Adding/removing children at runtime
- **Scalability**: 110 nodes in a single tree
- **Extreme states**: Fully collapsed (minimal visibility) vs fully expanded (maximal visibility)

## What's NOT Tested (and Why)

- **`CalculateNodeVisibilityIndex(nullptr)`**: Would crash due to no null check in implementation. Not a practical scenario.
- **Operations on invisible nodes**: Reserved for Task 5 as specified in task description.
- **Expand/collapse on invisible nodes**: Learned from Task 3 that this has uncertain semantics.

## Integration with Existing Tests

These tests complement existing test categories:
- **BasicIndexMapping**: Tests basic structure and index correctness
- **ExpandCollapseDynamics**: Tests expand/collapse operations
- **EdgeCasesAndComplexScenarios** (NEW): Tests edge cases, extreme conditions, and scalability

No duplication of test coverage. Each category has distinct focus.

# !!!FINISHED!!!
