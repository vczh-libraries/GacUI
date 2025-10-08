# !!!PLANNING!!!

# UPDATES

## UPDATE

If callback is not verified, you can just skip callbackLog and MockItemProviderCallback.

By the way, instead of asserting Count and RequestNode in this way, you can just use Count and RequestNode to make a List<WString> of texts, and compare it using the AssertCallbacks function. It has two obvious benifits:
1. Make the code compact. If you assigned different text/name to each node, it makes the code looks clean
2. Fail with verbose information. In the current code, when asserting against Count failed, you have no information. But with the AssertCallback function you actually get a list of names.

CalculateNodeVisibilityIndex looks good, keep the current way.

## UPDATE

The idea of AssertItems is actually good! But it would be making the code easier to call it in this way:
- If the list is expected to be empty, assert Count against 0
- Otherwise, define a const wchar_t* array in the code and pass it to the function. Just like how other test cases using AssertCallback.

## UPDATE

In AssertItems function, from `// Compare with expected items` all the way to the end is redundant, as you can just call AssertCallbacks to complete it. AssertCallbacks accepts a list of string and a array of string literals, exactly the work you need to do.

# IMPROVEMENT PLAN

## STEP 1: Add AssertItems Helper Function

**What to Change**: Add a helper function `AssertItems` to verify the visible items in a `NodeItemProvider` matches expected node texts.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders.h`, after the `AssertCallbacks` function template.

**Code to Add**:

```cpp
template<vint Count>
void AssertItems(Ptr<NodeItemProvider> provider, const wchar_t* (&expected)[Count])
{
	// Build actual items list from the provider
	List<WString> actualItems;
	for (vint i = 0; i < provider->Count(); i++)
	{
		actualItems.Add(provider->GetTextValue(i));
	}

	// Compare with expected items using AssertCallbacks
	AssertCallbacks(actualItems, expected);
}
```

**Why This is Necessary**: 
- Follows the established pattern of `AssertCallbacks` which takes a `const wchar_t*` array.
- Provides compact and readable test code by eliminating repetitive Count() and GetTextValue() assertions.
- Provides verbose failure messages showing actual vs expected node lists with indices.
- Makes test failures immediately actionable by showing exactly what differs.

## STEP 2: Add Test Case - EmptyTreeAndSingleNode

**What to Change**: Add a new test case `L"EmptyTreeAndSingleNode"` in the `BasicIndexMapping` category to test edge cases with minimal tree structures.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, after the `BasicSetupAndConstruction` test case.

**Code to Add**:

```cpp
TEST_CATEGORY(L"BasicIndexMapping")
{
	TEST_CASE(L"EmptyTreeAndSingleNode")
	{
		// Test 1: Empty tree (root with no children)
		auto rootProvider1 = Ptr(new TreeViewItemRootProvider);
		auto nodeItemProvider1 = Ptr(new NodeItemProvider(rootProvider1));
		
		// Verify empty tree through IItemProvider interface
		TEST_ASSERT(nodeItemProvider1->Count() == 0);
		
		// Verify RequestNode() through INodeItemView interface - should return nullptr for invalid index
		TEST_ASSERT(nodeItemProvider1->RequestNode(0) == nullptr);

		// Test 2: Single node tree (root with one child)
		auto rootProvider2 = Ptr(new TreeViewItemRootProvider);
		auto singleNode = CreateTreeViewItem(L"OnlyChild");
		auto rootMemoryNode2 = rootProvider2->GetMemoryNode(rootProvider2->GetRootNode().Obj());
		rootMemoryNode2->Children().Add(singleNode);
		
		auto nodeItemProvider2 = Ptr(new NodeItemProvider(rootProvider2));
		
		// Verify single node tree through IItemProvider interface
		const wchar_t* expected[] = {
			L"OnlyChild"
		};
		AssertItems(nodeItemProvider2, expected);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface - should return 0
		TEST_ASSERT(nodeItemProvider2->CalculateNodeVisibilityIndex(singleNode.Obj()) == 0);
	});
}
```

**Why This is Necessary**: 
- Edge cases with minimal data structures often reveal off-by-one errors in index calculations.
- Testing empty trees ensures proper handling of the degenerate case where root has no children.
- For empty lists, directly asserting Count() == 0 is clearest.
- Testing single-node trees validates the simplest non-empty scenario using `AssertItems` with a const wchar_t* array.
- This establishes the baseline before testing more complex hierarchies.

## STEP 3: Add Test Case - FlatTreeBasicMapping

**What to Change**: Add a test case `L"FlatTreeBasicMapping"` to verify the most basic scenario - a flat list with no hierarchy expansion.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `EmptyTreeAndSingleNode` test.

**Code to Add**:

```cpp
	TEST_CASE(L"FlatTreeBasicMapping")
	{
		// Setup: Create root provider with flat structure (all collapsed)
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto node1 = CreateTreeViewItem(L"Node1");
		auto node2 = CreateTreeViewItem(L"Node2");
		auto node3 = CreateTreeViewItem(L"Node3");
		
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(node1);
		rootMemoryNode->Children().Add(node2);
		rootMemoryNode->Children().Add(node3);
		
		// Create NodeItemProvider
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Verify flat tree structure through IItemProvider interface
		const wchar_t* expected[] = {
			L"Node1",
			L"Node2",
			L"Node3"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node1.Obj()) == 0);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node2.Obj()) == 1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node3.Obj()) == 2);
	});
```

**Why This is Necessary**:
- This establishes the baseline behavior without any hierarchy complexity.
- Verifies basic index-to-node and node-to-index bidirectional mapping works correctly.
- Tests all three core methods (`Count()`, `RequestNode()`, `CalculateNodeVisibilityIndex()`) in the simplest meaningful scenario.
- Validates that when all nodes are collapsed, only root's direct children are visible.
- Using `AssertItems` with a const wchar_t* array provides compact code and verbose failure messages.

## STEP 4: Add Test Case - SingleLevelExpansion

**What to Change**: Add a test case `L"SingleLevelExpansion"` to verify depth-first traversal when one parent node is expanded.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `FlatTreeBasicMapping` test.

**Code to Add**:

```cpp
	TEST_CASE(L"SingleLevelExpansion")
	{
		// Setup: Create root with 2 parent nodes, expand only first one
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto parent1 = CreateTreeViewItem(L"Parent1");
		auto parent2 = CreateTreeViewItem(L"Parent2");
		
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(parent1);
		rootMemoryNode->Children().Add(parent2);
		
		// Add children to both parents
		auto memoryParent1 = rootProvider->GetMemoryNode(parent1.Obj());
		auto child11 = CreateTreeViewItem(L"Child1-1");
		auto child12 = CreateTreeViewItem(L"Child1-2");
		memoryParent1->Children().Add(child11);
		memoryParent1->Children().Add(child12);
		
		auto memoryParent2 = rootProvider->GetMemoryNode(parent2.Obj());
		auto child21 = CreateTreeViewItem(L"Child2-1");
		auto child22 = CreateTreeViewItem(L"Child2-2");
		memoryParent2->Children().Add(child21);
		memoryParent2->Children().Add(child22);
		
		// Expand only Parent1
		parent1->SetExpanding(true);
		
		// Create NodeItemProvider
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Verify depth-first traversal through IItemProvider interface
		// Should be: Parent1, Child1-1, Child1-2, Parent2
		const wchar_t* expected[] = {
			L"Parent1",
			L"Child1-1",
			L"Child1-2",
			L"Parent2"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface - visible nodes
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent1.Obj()) == 0);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child11.Obj()) == 1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child12.Obj()) == 2);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(parent2.Obj()) == 3);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface - invisible nodes in collapsed subtree
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child21.Obj()) == -1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child22.Obj()) == -1);
	});
```

**Why This is Necessary**:
- Verifies the depth-first traversal algorithm works correctly: parent first, then its children in order.
- Tests that collapsed subtrees are completely invisible (children of Parent2 return -1).
- Validates `Count()` correctly includes expanded children but excludes collapsed ones.
- This is the first test of hierarchical structure, establishing foundation for more complex tests.
- Using `AssertItems` with a const wchar_t* array makes the expected depth-first order immediately visible in the code.

## STEP 5: Add Test Case - MultiLevelExpansion

**What to Change**: Add a test case `L"MultiLevelExpansion"` to verify correct indexing with 3-level hierarchy and partial expansion.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `SingleLevelExpansion` test.

**Code to Add**:

```cpp
	TEST_CASE(L"MultiLevelExpansion")
	{
		// Setup: Build 3-level tree structure
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto level1A = CreateTreeViewItem(L"Level1-A");
		auto level1B = CreateTreeViewItem(L"Level1-B");
		
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(level1A);
		rootMemoryNode->Children().Add(level1B);
		
		// Build Level1-A subtree: expanded, has 2 children
		auto memoryLevel1A = rootProvider->GetMemoryNode(level1A.Obj());
		auto level2A1 = CreateTreeViewItem(L"Level2-A1");
		auto level2A2 = CreateTreeViewItem(L"Level2-A2");
		memoryLevel1A->Children().Add(level2A1);
		memoryLevel1A->Children().Add(level2A2);
		level1A->SetExpanding(true);
		
		// Build Level2-A1 subtree: expanded, has 1 child
		auto memoryLevel2A1 = rootProvider->GetMemoryNode(level2A1.Obj());
		auto level3A1a = CreateTreeViewItem(L"Level3-A1a");
		memoryLevel2A1->Children().Add(level3A1a);
		level2A1->SetExpanding(true);
		
		// Build Level2-A2 subtree: collapsed, has 1 child (invisible)
		auto memoryLevel2A2 = rootProvider->GetMemoryNode(level2A2.Obj());
		auto level3A2a = CreateTreeViewItem(L"Level3-A2a");
		memoryLevel2A2->Children().Add(level3A2a);
		
		// Build Level1-B subtree: collapsed, has 1 child (invisible)
		auto memoryLevel1B = rootProvider->GetMemoryNode(level1B.Obj());
		auto level2B1 = CreateTreeViewItem(L"Level2-B1");
		memoryLevel1B->Children().Add(level2B1);
		
		// Create NodeItemProvider
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Verify 3-level depth-first traversal through IItemProvider interface
		// Visible: Level1-A, Level2-A1, Level3-A1a, Level2-A2, Level1-B = 5 nodes
		const wchar_t* expected[] = {
			L"Level1-A",
			L"Level2-A1",
			L"Level3-A1a",
			L"Level2-A2",
			L"Level1-B"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface - visible nodes
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level1A.Obj()) == 0);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level2A1.Obj()) == 1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level3A1a.Obj()) == 2);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level2A2.Obj()) == 3);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level1B.Obj()) == 4);
		
		// Verify CalculateNodeVisibilityIndex() through INodeItemView interface - invisible nodes
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level3A2a.Obj()) == -1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(level2B1.Obj()) == -1);
	});
```

**Why This is Necessary**:
- Tests more complex hierarchy traversal with 3 levels of nesting.
- Verifies that depth-first traversal works correctly even when some intermediate nodes are expanded and others are collapsed.
- Ensures the implementation correctly handles multiple levels of expansion state.
- This tests the recursive nature of `GetNodeByOffset()` which must skip entire collapsed subtrees.
- Using `AssertItems` with a const wchar_t* array clearly shows the expected 3-level depth-first traversal order.

## STEP 6: Add Test Case - RootNodeExclusionAndBidirectionalConsistency

**What to Change**: Add a test case `L"RootNodeExclusionAndBidirectionalConsistency"` to explicitly verify root node is excluded and test bidirectional mapping consistency.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `MultiLevelExpansion` test.

**Code to Add**:

```cpp
	TEST_CASE(L"RootNodeExclusionAndBidirectionalConsistency")
	{
		// Setup: Reuse SingleLevelExpansion tree structure
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto parent1 = CreateTreeViewItem(L"Parent1");
		auto parent2 = CreateTreeViewItem(L"Parent2");
		
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(parent1);
		rootMemoryNode->Children().Add(parent2);
		
		auto memoryParent1 = rootProvider->GetMemoryNode(parent1.Obj());
		auto child11 = CreateTreeViewItem(L"Child1-1");
		auto child12 = CreateTreeViewItem(L"Child1-2");
		memoryParent1->Children().Add(child11);
		memoryParent1->Children().Add(child12);
		
		parent1->SetExpanding(true);
		
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Verify structure through IItemProvider interface
		const wchar_t* expected[] = {
			L"Parent1",
			L"Child1-1",
			L"Child1-2",
			L"Parent2"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Get root node through INodeRootProvider interface
		auto rootNode = rootProvider->GetRootNode();
		
		// Verify root node exclusion through INodeItemView interface
		// CalculateNodeVisibilityIndex(root) should return -1
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(rootNode.Obj()) == -1);
		
		// RequestNode(0) should return first child of root, NOT root itself
		TEST_ASSERT(nodeItemProvider->RequestNode(0) == parent1.Obj());
		TEST_ASSERT(nodeItemProvider->RequestNode(0) != rootNode.Obj());
		
		// Verify bidirectional consistency: for each index, RequestNode -> CalculateNodeVisibilityIndex should return same index
		for (vint i = 0; i < nodeItemProvider->Count(); i++)
		{
			auto node = nodeItemProvider->RequestNode(i);
			TEST_ASSERT(node != nullptr);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node.Obj()) == i);
		}
		
		// Verify reverse bidirectional consistency: for each visible node, CalculateNodeVisibilityIndex -> RequestNode should return same node
		List<Ptr<INodeProvider>> visibleNodes;
		visibleNodes.Add(parent1);
		visibleNodes.Add(child11);
		visibleNodes.Add(child12);
		visibleNodes.Add(parent2);
		
		for (vint i = 0; i < visibleNodes.Count(); i++)
		{
			auto index = nodeItemProvider->CalculateNodeVisibilityIndex(visibleNodes[i].Obj());
			TEST_ASSERT(index >= 0);
			auto retrievedNode = nodeItemProvider->RequestNode(index);
			TEST_ASSERT(retrievedNode == visibleNodes[i].Obj());
		}
	});
```

**Why This is Necessary**:
- Explicitly tests the critical design decision that root node is excluded from visible indices.
- Verifies bidirectional consistency: `RequestNode(i) -> node` and `CalculateNodeVisibilityIndex(node) -> i` should be inverse operations.
- This consistency is fundamental to the adapter pattern - the list view expects indices to consistently map to the same items.
- Catches potential bugs where the two mapping directions might drift out of sync.
- Using `AssertItems` with a const wchar_t* array at the beginning provides clear baseline verification before testing consistency.

## STEP 7: Add Test Case - BoundaryConditionsForRequestNode

**What to Change**: Add a test case `L"BoundaryConditionsForRequestNode"` to verify boundary handling for invalid indices.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `RootNodeExclusionAndBidirectionalConsistency` test.

**Code to Add**:

```cpp
	TEST_CASE(L"BoundaryConditionsForRequestNode")
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
		
		// Verify structure through IItemProvider interface
		const wchar_t* expected[] = {
			L"Node1",
			L"Node2",
			L"Node3"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Verify valid indices through INodeItemView interface - should return non-null
		for (vint i = 0; i < nodeItemProvider->Count(); i++)
		{
			TEST_ASSERT(nodeItemProvider->RequestNode(i) != nullptr);
		}
		
		// Verify invalid indices through INodeItemView interface - should return nullptr
		TEST_ASSERT(nodeItemProvider->RequestNode(-1) == nullptr);
		TEST_ASSERT(nodeItemProvider->RequestNode(nodeItemProvider->Count()) == nullptr);
		TEST_ASSERT(nodeItemProvider->RequestNode(nodeItemProvider->Count() + 1) == nullptr);
		TEST_ASSERT(nodeItemProvider->RequestNode(999) == nullptr);
	});
```

**Why This is Necessary**:
- Robust error handling prevents crashes when invalid indices are used.
- List controls may pass invalid indices due to bugs or race conditions.
- Returning `nullptr` for out-of-bounds indices is the expected defensive behavior.
- Tests both negative indices and indices beyond the valid range.
- Using `AssertItems` with a const wchar_t* array establishes baseline before testing boundary conditions.

## STEP 8: Add Test Case - ComplexMixedExpansionWithCount

**What to Change**: Add a test case `L"ComplexMixedExpansionWithCount"` to verify Count() accurately reflects visible nodes in complex scenarios.

**Location**: `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`, within the `BasicIndexMapping` category after the `BoundaryConditionsForRequestNode` test. This should be the last test case in the category.

**Code to Add**:

```cpp
	TEST_CASE(L"ComplexMixedExpansionWithCount")
	{
		// Setup: Build complex tree with mixed expansion states
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto nodeA = CreateTreeViewItem(L"A");
		auto nodeB = CreateTreeViewItem(L"B");
		auto nodeC = CreateTreeViewItem(L"C");
		
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(nodeA);
		rootMemoryNode->Children().Add(nodeB);
		rootMemoryNode->Children().Add(nodeC);
		
		// Build A subtree: expanded, has 2 children
		auto memoryA = rootProvider->GetMemoryNode(nodeA.Obj());
		auto nodeA1 = CreateTreeViewItem(L"A1");
		auto nodeA2 = CreateTreeViewItem(L"A2");
		memoryA->Children().Add(nodeA1);
		memoryA->Children().Add(nodeA2);
		nodeA->SetExpanding(true);
		
		// Build A1 subtree: collapsed, has 1 child (invisible)
		auto memoryA1 = rootProvider->GetMemoryNode(nodeA1.Obj());
		auto nodeA1a = CreateTreeViewItem(L"A1a");
		memoryA1->Children().Add(nodeA1a);
		
		// Build A2 subtree: expanded, has 2 children
		auto memoryA2 = rootProvider->GetMemoryNode(nodeA2.Obj());
		auto nodeA2a = CreateTreeViewItem(L"A2a");
		auto nodeA2b = CreateTreeViewItem(L"A2b");
		memoryA2->Children().Add(nodeA2a);
		memoryA2->Children().Add(nodeA2b);
		nodeA2->SetExpanding(true);
		
		// Build B subtree: collapsed, has 1 child (invisible)
		auto memoryB = rootProvider->GetMemoryNode(nodeB.Obj());
		auto nodeB1 = CreateTreeViewItem(L"B1");
		memoryB->Children().Add(nodeB1);
		
		// C has no children
		
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Verify complex mixed expansion through IItemProvider interface
		// Visible: A, A1, A2, A2a, A2b, B, C = 7 nodes
		const wchar_t* expected[] = {
			L"A",
			L"A1",
			L"A2",
			L"A2a",
			L"A2b",
			L"B",
			L"C"
		};
		AssertItems(nodeItemProvider, expected);
		
		// Verify each visible node has correct index via CalculateNodeVisibilityIndex() through INodeItemView interface
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA.Obj()) == 0);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA1.Obj()) == 1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA2.Obj()) == 2);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA2a.Obj()) == 3);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA2b.Obj()) == 4);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == 5);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == 6);
		
		// Verify invisible nodes return -1 via CalculateNodeVisibilityIndex() through INodeItemView interface
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeA1a.Obj()) == -1);
		TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB1.Obj()) == -1);
	});
}
```

**Why This is Necessary**:
- Comprehensive test ensures `Count()` calculation is correct even with complex expansion patterns.
- Tests the full depth-first traversal algorithm with a mix of expanded and collapsed nodes at various levels.
- Verifies that all three methods (`Count()`, `RequestNode()`, `CalculateNodeVisibilityIndex()`) work correctly together in a realistic complex scenario.
- This is the final validation that all pieces work together before declaring the basic mapping feature complete.
- Using `AssertItems` with a const wchar_t* array makes the complex expected structure immediately visible and provides verbose failure information.

# TEST PLAN

## Overview

All test cases are implemented in `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp` within a new `TEST_CATEGORY(L"BasicIndexMapping")` block. The tests cover static tree structures only (no expand/collapse operations during testing).

## Test Case 1: EmptyTreeAndSingleNode

**Purpose**: Test edge cases with minimal tree structures to catch off-by-one errors.

**Scenarios**:
- Empty tree (root with no children): verifies `Count()` returns 0 and `RequestNode(0)` returns nullptr
- Single node tree (root with one child): verifies `Count()` returns 1, node can be retrieved, and its index is 0

**Expected Results**:
- Empty tree: `Count() == 0`, `RequestNode(0) == nullptr`
- Single node: `Count() == 1`, `RequestNode(0)` returns the child, `CalculateNodeVisibilityIndex(child) == 0`

**Corner Cases Covered**:
- Minimum viable tree structures
- Degenerate case validation

## Test Case 2: FlatTreeBasicMapping

**Purpose**: Verify basic bidirectional mapping in a flat list with no hierarchy.

**Scenarios**:
- Create 3 nodes at root level (all collapsed)
- Verify `Count()`, `RequestNode()`, `CalculateNodeVisibilityIndex()`, and `GetTextValue()`

**Expected Results**:
- `Count() == 3`
- `RequestNode(0, 1, 2)` returns Node1, Node2, Node3 in order
- `CalculateNodeVisibilityIndex()` returns 0, 1, 2 for the three nodes
- `GetTextValue()` matches node names

**Corner Cases Covered**:
- Basic sequential indexing
- Bidirectional mapping consistency in simplest scenario

## Test Case 3: SingleLevelExpansion

**Purpose**: Verify depth-first traversal with one level of expansion.

**Scenarios**:
- Create 2 parent nodes, each with 2 children
- Expand only the first parent
- Verify visible nodes follow depth-first order: Parent1, Child1-1, Child1-2, Parent2

**Expected Results**:
- `Count() == 4`
- `RequestNode()` returns nodes in depth-first order
- Children of expanded parent have visible indices
- Children of collapsed parent return -1

**Corner Cases Covered**:
- Depth-first traversal algorithm
- Collapsed subtree invisibility
- Mixed expansion states

## Test Case 4: MultiLevelExpansion

**Purpose**: Verify correct indexing with 3-level hierarchy and partial expansion.

**Scenarios**:
- Build 3-level tree: Level1-A (expanded) -> Level2-A1 (expanded) -> Level3-A1a
- Also: Level1-A -> Level2-A2 (collapsed) -> Level3-A2a (invisible)
- Also: Level1-B (collapsed) -> Level2-B1 (invisible)

**Expected Results**:
- `Count() == 5` (Level1-A, Level2-A1, Level3-A1a, Level2-A2, Level1-B)
- Nodes accessible in depth-first order
- Invisible nodes (Level3-A2a, Level2-B1) return -1

**Corner Cases Covered**:
- Multi-level hierarchy traversal
- Recursive depth-first algorithm
- Partial expansion across multiple levels

## Test Case 5: RootNodeExclusionAndBidirectionalConsistency

**Purpose**: Explicitly verify root node exclusion and bidirectional mapping consistency.

**Scenarios**:
- Build tree with expansion
- Verify root node returns -1 from `CalculateNodeVisibilityIndex()`
- Verify `RequestNode(0)` returns first child, not root
- For each index i: verify `CalculateNodeVisibilityIndex(RequestNode(i)) == i`
- For each visible node: verify `RequestNode(CalculateNodeVisibilityIndex(node)) == node`

**Expected Results**:
- Root node is excluded (returns -1, never appears at index 0)
- Bidirectional consistency holds for all visible nodes

**Corner Cases Covered**:
- Root node special handling
- Index-to-node and node-to-index inverse operations
- Consistency validation

## Test Case 6: BoundaryConditionsForRequestNode

**Purpose**: Verify robust handling of invalid indices.

**Scenarios**:
- Create flat tree with 3 nodes
- Test valid indices (0..Count-1) return non-null
- Test invalid indices: -1, Count, Count+1, 999

**Expected Results**:
- Valid indices return non-null nodes
- All invalid indices return nullptr

**Corner Cases Covered**:
- Negative indices
- Indices at and beyond boundary
- Large out-of-range indices
- Defensive error handling

## Test Case 7: ComplexMixedExpansionWithCount

**Purpose**: Comprehensive test with complex expansion patterns.

**Scenarios**:
- Build tree with 7 visible nodes and 2 invisible nodes
- Mix of expanded and collapsed nodes at various levels
- Verify Count(), all RequestNode() calls, all CalculateNodeVisibilityIndex() calls

**Expected Results**:
- `Count() == 7`
- All 7 visible nodes accessible in correct depth-first order
- All 2 invisible nodes return -1

**Corner Cases Covered**:
- Complex real-world expansion patterns
- Count() accuracy with mixed states
- Full system integration test

## Test Execution Strategy

1. **Run existing tests first**: Ensure `BasicSetupAndConstruction` still passes to confirm no regressions
2. **Run new tests in order**: Execute test cases 1-7 sequentially
3. **Verify no callbacks**: Since these are static tests, callback logs should remain empty after initial clear
4. **Check compilation**: All tests should compile without warnings
5. **Performance**: All tests should execute quickly (< 100ms total) as they only test in-memory operations

## Expected Test Coverage

After completing these 7 test cases, the following aspects of `NodeItemProvider` basic index mapping will be covered:

**Methods Tested**:
- ✓ `Count()` - IItemProvider interface
- ✓ `RequestNode(vint index)` - INodeItemView interface
- ✓ `CalculateNodeVisibilityIndex(INodeProvider* node)` - INodeItemView interface
- ✓ `GetTextValue(vint index)` - IItemProvider interface (basic validation)

**Scenarios Tested**:
- ✓ Empty trees
- ✓ Single node trees
- ✓ Flat trees (no hierarchy)
- ✓ Single-level expansion
- ✓ Multi-level expansion (3 levels)
- ✓ Partial expansion (some expanded, some collapsed)
- ✓ Complex mixed expansion patterns
- ✓ Root node exclusion
- ✓ Bidirectional mapping consistency
- ✓ Boundary conditions (invalid indices)
- ✓ Depth-first traversal order

**Not Tested** (intentionally out of scope for this task):
- ✗ Dynamic expand/collapse operations (Task 3)
- ✗ Node modification during iteration (Task 3)
- ✗ Callback verification during expand/collapse (Task 3)
- ✗ Stress testing with large trees (Task 4)
- ✗ Performance benchmarking (Task 4)

## Existing Tests Review

The existing `BasicSetupAndConstruction` test case already covers:
- Basic setup and construction
- Callback attachment and clearing
- Simple tree structure with 2 nodes
- Basic `Count()` and `GetTextValue()` verification
- Node retrieval through `RequestNode()`
- Callback propagation when nodes are added

**No changes needed to existing tests** because:
- The existing test covers construction and callback basics
- New tests focus on index mapping, which is orthogonal to construction
- Both can coexist without interference

# !!!FINISHED!!!
