# !!!EXECUTION!!!

# UPDATES

## UPDATE

I believe passing invalid index to GetTextValue and GetBindingValue should trigger CHECK_ERROR. If the source code is not written in this way, you will need to fix it. Please confirm

You must use the ERROR_MESSAGE_PREFIX pattern with CHECK_ERROR and use TEST_ERROR in test cases for index out of bound situation. In ERROR_MESSAGE_PREFIX, the method name should also come with full namespace.

# IMPROVEMENT PLAN

## STEP 1: Fix GetTextValue and GetBindingValue to Use CHECK_ERROR [DONE]

**File**: `Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.cpp`

**Current code (lines 215-231)**:

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
	auto node = RequestNode(itemIndex);
	return root->GetTextValue(node.Obj());
#undef ERROR_MESSAGE_PREFIX
}

description::Value NodeItemProvider::GetBindingValue(vint itemIndex)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::tree::NodeItemProvider::GetBindingValue(vint)#"
	CHECK_ERROR(0 <= itemIndex && itemIndex < Count(), ERROR_MESSAGE_PREFIX L"Index out of range.");
	auto node = RequestNode(itemIndex);
	return root->GetBindingValue(node.Obj());
#undef ERROR_MESSAGE_PREFIX
}
```

## STEP 2: Add Test Category "EdgeCasesAndComplexScenarios" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: After the `ExpandCollapseDynamics` category (after line 1042)

**Code to add**:

```cpp
TEST_CATEGORY(L"EdgeCasesAndComplexScenarios")
{
```

## STEP 3: Implement Test Case "InvalidIndicesForDataRetrieval" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category

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

## STEP 4: Implement Test Case "CalculateIndexForForeignNode" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category, after `InvalidIndicesForDataRetrieval`

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

## STEP 5: Implement Test Case "DeeplyNestedTreeIndexMapping" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category, after `CalculateIndexForForeignNode`

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

## STEP 6: Implement Test Case "DynamicChildAdditionRemoval" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category, after `DeeplyNestedTreeIndexMapping`

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

## STEP 7: Implement Test Case "LargeTreeStressTest" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category, after `DynamicChildAdditionRemoval`

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

## STEP 8: Implement Test Case "FullyExpandedVsFullyCollapsed" [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: Inside the `EdgeCasesAndComplexScenarios` category, after `LargeTreeStressTest`

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

## STEP 9: Close the TEST_CATEGORY Block [DONE]

**File**: `Test\GacUISrc\UnitTest\TestControls_List_ItemProvider_NodeItemProvider.cpp`

**Location**: After all test cases in the `EdgeCasesAndComplexScenarios` category

**Code to add**:

```cpp
});
```

# TEST PLAN

All test cases will be added to a new `TEST_CATEGORY(L"EdgeCasesAndComplexScenarios")` block in `TestControls_List_ItemProvider_NodeItemProvider.cpp`. Each test case follows the established patterns from existing tests.

## Test Case 1: InvalidIndicesForDataRetrieval

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

## Test Case 2: CalculateIndexForForeignNode

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

## Test Case 3: DeeplyNestedTreeIndexMapping

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

## Test Case 4: DynamicChildAdditionRemoval

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

## Test Case 5: LargeTreeStressTest

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

## Test Case 6: FullyExpandedVsFullyCollapsed

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

# !!!FINISHED!!!

# FIXING ATTEMPTS

## Fixing attempt No.1

**Why the original change did not work**:
The test case `CalculateIndexForForeignNode` is failing because `CalculateNodeVisibilityIndexInternal` doesn't verify that a node belongs to this `NodeItemProvider`'s tree. When a node from tree B is passed to tree A's method, it recurses up the parent chain of tree B's nodes, which doesn't match tree A's root.

**What I need to do**:
Add a check in `CalculateNodeVisibilityIndexInternal` to verify that the node belongs to the correct tree by checking if it eventually reaches this `NodeItemProvider`'s root node. If the parent chain leads to a different root or if the node is the root itself, return -1 (not found).

**Why this would solve the test break**:
By checking that the node belongs to the correct tree (by verifying the root matches), foreign nodes will correctly return -1 instead of attempting to calculate an index in the wrong tree structure.

**Result**: The fix was incomplete. The recursive call returns -1 for a foreign root, but then the code continued to search for the child in the parent's children. Added an additional check to propagate the -1 immediately when the parent is not in our tree.

**Second Issue**: Propagating -1 broke the callback system because -1 is also used for our own root node. When adding a child to the root, `CalculateNodeVisibilityIndexInternal` is called on the new child, which recursively calls it on the root (returns -1), and then our check propagates -1 immediately instead of searching for the child. Changed to use -3 for foreign trees to distinguish from our own root (-1).

**Third Issue**: The test `DeeplyNestedTreeIndexMapping` was failing with count=4 instead of expected count=2. The test creates a 5-level deep tree (Root→A→B→C→D→E) where A is expanded, B is collapsed, and C/D are expanded (but invisible). The issue was in `MemoryNodeProvider::OnChildTotalVisibleNodesChanged` - it was unconditionally propagating visibility count changes to the parent even when the parent was collapsed. When nodeC->SetExpanding(true) was called, it propagated the count change through B (collapsed) up to Root, causing collapsed nodes to be counted as visible.

**Fix for Third Issue**:
Modified `OnChildTotalVisibleNodesChanged` in `DataSourceImpl_INodeProvider_MemoryNodeProvider.cpp` to only propagate to parent if parent is expanding:

```cpp
void MemoryNodeProvider::OnChildTotalVisibleNodesChanged(vint offset)
{
	totalVisibleNodeCount+=offset;
	if(parent && parent->GetExpanding())  // Added parent->GetExpanding() check
	{
		parent->OnChildTotalVisibleNodesChanged(offset);
	}
}
```

This ensures that visibility count changes in collapsed subtrees don't affect the total count.

# !!!VERIFIED!!!

# Comparing to User Edit

## Analysis of User Changes

After comparing the execution document with the actual source code, I identified the following key user edit:

### 1. Foreign Node Detection: Changed from Return -1 to CHECK_ERROR

**What was documented in Copilot_Execution.md**:
- Initially, `CalculateNodeVisibilityIndex` should return `-1` for foreign nodes (nodes from a different tree)
- The fixing attempt used `-3` internally to distinguish foreign trees from our own root node
- The test case expected: `TEST_ASSERT(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB1.Obj()) == -1);`

**What the user actually implemented**:
In `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` (lines 152-157), the user changed foreign node detection to trigger a `CHECK_ERROR`:

```cpp
if (index == -1)
{
	// Parent returned -1, which means parent has no parent (is a root node)
	// Check if this parent is OUR root - if not, this is a foreign node
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::tree::NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider*)#"
	CHECK_ERROR(parent.Obj() == root->GetRootNode().Obj(), ERROR_MESSAGE_PREFIX L"The node does not belong to the tree associated with this NodeItemProvider.");
#undef ERROR_MESSAGE_PREFIX
}
```

**Corresponding test case change**:
In `TestItemProviders_NodeItemProvider.cpp` (lines 1107-1108), the test was updated to use `TEST_ERROR`:

```cpp
// Verify nodes from tree B trigger CHECK_ERROR (foreign nodes) through INodeItemView interface
TEST_ERROR(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB1.Obj()));
TEST_ERROR(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB2.Obj()));
```

### 2. User's Philosophy and Preferences

From this edit, I learned the following about user's preferences:

#### Preference 1: Strict Error Checking Over Silent Failure
- **Original approach**: Return sentinel values (like `-1`) for invalid input, allowing the caller to continue
- **User's preference**: Use `CHECK_ERROR` to immediately detect and report programming errors
- **Rationale**: Passing a foreign node to `CalculateNodeVisibilityIndex` is a programming error, not a valid use case. It should be caught immediately during development rather than silently returning `-1`, which could mask bugs.

#### Preference 2: Clear Error Messages with Full Context
- The error message includes the full method signature: `vl::presentation::controls::tree::NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider*)#`
- The error message is descriptive: `"The node does not belong to the tree associated with this NodeItemProvider."`
- This follows the established pattern of using `ERROR_MESSAGE_PREFIX` with full namespace qualification

#### Preference 3: Defensive Programming at API Boundaries
- Invalid input should be caught as early as possible
- Foreign nodes are not just "not found" - they represent a fundamental misuse of the API
- By using `CHECK_ERROR`, the API contract is explicitly enforced

### 3. Impact on Future Tasks

For future tasks, I should:

1. **Distinguish between "not found" vs "invalid input"**:
   - "Not found" (like a collapsed node): Return sentinel values like `-1`
   - "Invalid input" (like foreign nodes, out-of-range indices): Use `CHECK_ERROR`

2. **When designing test cases for edge cases**:
   - Analyze whether an edge case is a valid use case (test with assertions) or a programming error (test with `TEST_ERROR`)
   - Foreign nodes, null pointers to required parameters, indices out of range are programming errors

3. **Follow the ERROR_MESSAGE_PREFIX pattern consistently**:
   - Always include full namespace path in method signature
   - Provide clear, descriptive error messages that help developers understand what went wrong

4. **Code comments should explain the "why"**:
   - The user added a comment explaining the logic: "Parent returned -1, which means parent has no parent (is a root node)"
   - This helps future maintainers understand the reasoning behind the check
