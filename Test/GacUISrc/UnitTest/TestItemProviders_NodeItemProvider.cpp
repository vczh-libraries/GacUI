#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"BasicSetupAndConstruction")
	{
		// Setup callback logging
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		
		// Setup: Create root provider with simple tree structure
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto node1 = CreateTreeViewItem(L"Node1");
		auto node2 = CreateTreeViewItem(L"Node2");
		
		// Add nodes to root
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(node1);
		rootMemoryNode->Children().Add(node2);
		
		// Create NodeItemProvider wrapping the root provider
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Attach callback to NodeItemProvider
		nodeItemProvider->AttachCallback(&itemCallback);
		callbackLog.Clear(); // Clear the OnAttached callback
		
		// Verify basic operations
		TEST_ASSERT(nodeItemProvider->Count() == 2);
		TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Node1");
		TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Node2");
		
		// Verify node retrieval through INodeItemView interface
		auto retrievedNode = nodeItemProvider->RequestNode(0);
		TEST_ASSERT(retrievedNode != nullptr);
		
		// Verify GetTextValue on root provider through INodeRootProvider interface
		TEST_ASSERT(rootProvider->GetTextValue(retrievedNode.Obj()) == L"Node1");
		
		// Verify root provider retrieval
		TEST_ASSERT(nodeItemProvider->GetRoot() == rootProvider);
		
		// Add another node to trigger callback propagation
		auto node3 = CreateTreeViewItem(L"Node3");
		rootMemoryNode->Children().Add(node3);
		
		// Verify callback was triggered with correct parameters
		// NodeItemProvider should propagate tree modifications as item modifications
		const wchar_t* expected[] = {
			L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});

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
});

	TEST_CATEGORY(L"ExpandCollapseDynamics")
	{
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
			// Children are no longer visible, so they should return -1 (invisible)
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child1.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child2.Obj()) == -1);
		});

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
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -1); // Still invisible
			
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
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -1); // Invisible again
			
			// Step 4: Collapse parent - both child and grandchild disappear
			parent->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			AssertItems(nodeItemProvider, expectedOnlyParent);
			
			const wchar_t* collapseParentCallbacks[] = {
				L"OnItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, collapseParentCallbacks);
			
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -1);
		});

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
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB1.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB2.Obj()) == -1);
			
			nodeA->SetExpanding(false);
			TEST_ASSERT(nodeItemProvider->Count() == 1); // Only A
			const wchar_t* expected5[] = { L"A" };
			AssertItems(nodeItemProvider, expected5);
			
			// Verify all descendants are invisible through INodeItemView interface
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeB.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC1.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(nodeC2.Obj()) == -1);
			
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
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(child.Obj()) == -1);
			TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(grandchild.Obj()) == -1);
			
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
	});

	TEST_CATEGORY(L"EdgeCasesAndComplexScenarios")
	{
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
			
			// Verify nodes from tree B trigger CHECK_ERROR (foreign nodes) through INodeItemView interface
			TEST_ERROR(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB1.Obj()));
			TEST_ERROR(nodeItemProviderA->CalculateNodeVisibilityIndex(nodeB2.Obj()));
		});

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
	});

	TEST_CATEGORY(L"OperationsOnInvisibleNodes")
	{
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
	});

	TEST_CATEGORY(L"BasicDataRetrieval")
	{
		TEST_CASE(L"GetBindingValueValidIndices")
		{
			// Setup: Create root provider with simple tree structure
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node2 = CreateTreeViewItem(L"Node2");
			auto node3 = CreateTreeViewItem(L"Node3");
			auto node4 = CreateTreeViewItem(L"Node4");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			rootMemoryNode->Children().Add(node3);
			
			// Add a child to node2 and expand it to have more visible nodes
			auto node2MemoryNode = rootProvider->GetMemoryNode(node2.Obj());
			node2MemoryNode->Children().Add(node4);
			node2->SetExpanding(true); // Expand node2 to show node4
			
			// Create NodeItemProvider
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Verify Count() through IItemProvider interface - should have 4 visible nodes
			TEST_ASSERT(nodeItemProvider->Count() == 4);
			
			// Verify GetBindingValue() through IItemProvider interface for first position
			auto binding0 = nodeItemProvider->GetBindingValue(0);
			auto node0 = nodeItemProvider->RequestNode(0);
			auto expectedBinding0 = rootProvider->GetBindingValue(node0.Obj());
			// Both should return the same binding value
			TEST_ASSERT(binding0.GetRawPtr() == expectedBinding0.GetRawPtr());
			
			// Verify GetBindingValue() for middle position (index 1)
			auto binding1 = nodeItemProvider->GetBindingValue(1);
			auto node1Retrieved = nodeItemProvider->RequestNode(1);
			auto expectedBinding1 = rootProvider->GetBindingValue(node1Retrieved.Obj());
			TEST_ASSERT(binding1.GetRawPtr() == expectedBinding1.GetRawPtr());
			
			// Verify GetBindingValue() for another middle position (index 2)
			auto binding2 = nodeItemProvider->GetBindingValue(2);
			auto node2Retrieved = nodeItemProvider->RequestNode(2);
			auto expectedBinding2 = rootProvider->GetBindingValue(node2Retrieved.Obj());
			TEST_ASSERT(binding2.GetRawPtr() == expectedBinding2.GetRawPtr());
			
			// Verify GetBindingValue() for last position
			auto binding3 = nodeItemProvider->GetBindingValue(3);
			auto node3Retrieved = nodeItemProvider->RequestNode(3);
			auto expectedBinding3 = rootProvider->GetBindingValue(node3Retrieved.Obj());
			TEST_ASSERT(binding3.GetRawPtr() == expectedBinding3.GetRawPtr());
		});

		TEST_CASE(L"GetBindingValueAfterExpandCollapse")
		{
			// Setup: Create hierarchical tree structure
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
			
			auto child1MemoryNode = rootProvider->GetMemoryNode(child1.Obj());
			child1MemoryNode->Children().Add(grandchild);
			
			// Initially all collapsed - only parent visible
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Verify initial state through IItemProvider interface
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			auto initialBinding = nodeItemProvider->GetBindingValue(0);
			auto initialNode = nodeItemProvider->RequestNode(0);
			auto expectedInitial = rootProvider->GetBindingValue(initialNode.Obj());
			TEST_ASSERT(initialBinding.GetRawPtr() == expectedInitial.GetRawPtr());
			
			// Expand parent to show children
			parent->SetExpanding(true);
			
			// Verify expanded state through IItemProvider interface - now 3 visible nodes
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			
			// Verify GetBindingValue() for all visible nodes after expansion
			for (vint i = 0; i < 3; i++)
			{
				auto binding = nodeItemProvider->GetBindingValue(i);
				auto node = nodeItemProvider->RequestNode(i);
				auto expected = rootProvider->GetBindingValue(node.Obj());
				TEST_ASSERT(binding.GetRawPtr() == expected.GetRawPtr());
			}
			
			// Expand child1 to show grandchild
			child1->SetExpanding(true);
			
			// Verify further expanded state through IItemProvider interface - now 4 visible nodes
			TEST_ASSERT(nodeItemProvider->Count() == 4);
			
			// Verify GetBindingValue() for all visible nodes including grandchild
			for (vint i = 0; i < 4; i++)
			{
				auto binding = nodeItemProvider->GetBindingValue(i);
				auto node = nodeItemProvider->RequestNode(i);
				auto expected = rootProvider->GetBindingValue(node.Obj());
				TEST_ASSERT(binding.GetRawPtr() == expected.GetRawPtr());
			}
			
			// Collapse parent back
			parent->SetExpanding(false);
			
			// Verify collapsed state through IItemProvider interface - back to 1 visible node
			TEST_ASSERT(nodeItemProvider->Count() == 1);
			auto finalBinding = nodeItemProvider->GetBindingValue(0);
			auto finalNode = nodeItemProvider->RequestNode(0);
			auto expectedFinal = rootProvider->GetBindingValue(finalNode.Obj());
			TEST_ASSERT(finalBinding.GetRawPtr() == expectedFinal.GetRawPtr());
		});

		TEST_CASE(L"RequestViewWithNodeItemView")
		{
			// Setup: Create simple tree
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Node1");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Request INodeItemView through IItemProvider interface
			auto view = nodeItemProvider->RequestView(WString::Unmanaged(INodeItemView::Identifier));
			
			// Verify the view is returned (not nullptr)
			TEST_ASSERT(view != nullptr);
			
			// Verify it can be cast to INodeItemView
			auto nodeView = dynamic_cast<INodeItemView*>(view);
			TEST_ASSERT(nodeView != nullptr);
			
			// Verify the view actually works by calling a method on it
			auto retrievedNode = nodeView->RequestNode(0);
			TEST_ASSERT(retrievedNode != nullptr);
			TEST_ASSERT(rootProvider->GetTextValue(retrievedNode.Obj()) == L"Node1");
		});

		TEST_CASE(L"RequestViewWithUnknownIdentifier")
		{
			// Setup: Create simple tree
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Node1");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Request unknown view type through IItemProvider interface
			auto view = nodeItemProvider->RequestView(L"UnknownViewIdentifier");
			
			// Verify it returns nullptr (TreeViewItemRootProvider doesn't support arbitrary identifiers)
			TEST_ASSERT(view == nullptr);
		});
	});

	TEST_CATEGORY(L"IntegrationAndCallbacks")
	{
		TEST_CASE(L"DataMatchesDelegatedCalls")
		{
			// Setup: Create tree with multiple nodes
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node2 = CreateTreeViewItem(L"Node2");
			auto child1 = CreateTreeViewItem(L"Child1");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			
			auto node1MemoryNode = rootProvider->GetMemoryNode(node1.Obj());
			node1MemoryNode->Children().Add(child1);
			
			node1->SetExpanding(true); // Expand to show child1
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Verify 3 visible nodes
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			
			// For each visible node, verify data matches between NodeItemProvider and direct root provider calls
			for (vint i = 0; i < 3; i++)
			{
				// Get data through NodeItemProvider (IItemProvider interface)
				auto textFromProvider = nodeItemProvider->GetTextValue(i);
				auto bindingFromProvider = nodeItemProvider->GetBindingValue(i);
				
				// Get the node through NodeItemProvider (INodeItemView interface)
				auto node = nodeItemProvider->RequestNode(i);
				TEST_ASSERT(node != nullptr);
				
				// Get data directly from root provider (INodeRootProvider interface)
				auto textDirect = rootProvider->GetTextValue(node.Obj());
				auto bindingDirect = rootProvider->GetBindingValue(node.Obj());
				
				// Verify both approaches return identical data
				TEST_ASSERT(textFromProvider == textDirect);
				TEST_ASSERT(bindingFromProvider.GetRawPtr() == bindingDirect.GetRawPtr());
			}
		});

		TEST_CASE(L"DynamicDataUpdatesReflected")
		{
			// Setup: Create tree with nodes
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto node1 = CreateTreeViewItem(L"Original1");
			auto node2 = CreateTreeViewItem(L"Original2");
			auto node3 = CreateTreeViewItem(L"Original3");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			rootMemoryNode->Children().Add(node3);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Verify initial text values through IItemProvider interface
			TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Original1");
			TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Original2");
			TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Original3");
			
			// Modify the underlying tree items directly
			rootProvider->GetTreeViewData(node1.Obj())->text = L"Modified1";
			rootProvider->GetTreeViewData(node2.Obj())->text = L"Modified2";
			rootProvider->GetTreeViewData(node3.Obj())->text = L"Modified3";
			
			// Verify changes are immediately reflected through NodeItemProvider (no caching)
			TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Modified1");
			TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Modified2");
			TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Modified3");
			
			// Modify again to be thorough
			rootProvider->GetTreeViewData(node2.Obj())->text = L"SecondModification";
			TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"SecondModification");
		});

		TEST_CASE(L"MultipleCallbacksReceiveEvents")
		{
			// Setup: Create tree structure
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child1 = CreateTreeViewItem(L"Child1");
			auto child2 = CreateTreeViewItem(L"Child2");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child1);
			parentMemoryNode->Children().Add(child2);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Attach multiple callbacks
			List<WString> callbackLog1;
			List<WString> callbackLog2;
			List<WString> callbackLog3;
			MockItemProviderCallback callback1(callbackLog1);
			MockItemProviderCallback callback2(callbackLog2);
			MockItemProviderCallback callback3(callbackLog3);
			
			nodeItemProvider->AttachCallback(&callback1);
			nodeItemProvider->AttachCallback(&callback2);
			nodeItemProvider->AttachCallback(&callback3);
			
			// Clear the OnAttached callbacks
			callbackLog1.Clear();
			callbackLog2.Clear();
			callbackLog3.Clear();
			
			// Perform an operation that triggers callbacks - expand parent
			parent->SetExpanding(true);
			
			// Verify all three callbacks received the event
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog1, expected);
			AssertCallbacks(callbackLog2, expected);
			AssertCallbacks(callbackLog3, expected);
			
			// Clear logs and perform another operation
			callbackLog1.Clear();
			callbackLog2.Clear();
			callbackLog3.Clear();
			
			// Add a new child
			auto child3 = CreateTreeViewItem(L"Child3");
			parentMemoryNode->Children().Add(child3);
			
			// Verify all three callbacks received the new event
			const wchar_t* expected2[] = {
				L"OnItemModified(start=3, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog1, expected2);
			AssertCallbacks(callbackLog2, expected2);
			AssertCallbacks(callbackLog3, expected2);
		});

		TEST_CASE(L"DetachCallbackStopsEvents")
		{
			// Setup: Create tree structure
			auto rootProvider = Ptr(new TreeViewItemRootProvider);
			auto parent = CreateTreeViewItem(L"Parent");
			auto child1 = CreateTreeViewItem(L"Child1");
			auto child2 = CreateTreeViewItem(L"Child2");
			
			auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(parent);
			
			auto parentMemoryNode = rootProvider->GetMemoryNode(parent.Obj());
			parentMemoryNode->Children().Add(child1);
			parentMemoryNode->Children().Add(child2);
			
			auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
			
			// Attach two callbacks
			List<WString> callbackLog1;
			List<WString> callbackLog2;
			MockItemProviderCallback callback1(callbackLog1);
			MockItemProviderCallback callback2(callbackLog2);
			
			nodeItemProvider->AttachCallback(&callback1);
			nodeItemProvider->AttachCallback(&callback2);
			
			// Clear the OnAttached callbacks
			callbackLog1.Clear();
			callbackLog2.Clear();
			
			// Perform an operation - both should receive events
			parent->SetExpanding(true);
			
			// Verify both callbacks received the event
			const wchar_t* expected1[] = {
				L"OnItemModified(start=1, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog1, expected1);
			AssertCallbacks(callbackLog2, expected1);
			
			// Detach the first callback and clear logs
			nodeItemProvider->DetachCallback(&callback1);
			callbackLog1.Clear();
			callbackLog2.Clear();
			
			// Perform another operation
			auto child3 = CreateTreeViewItem(L"Child3");
			parentMemoryNode->Children().Add(child3);
			
			// Verify only callback2 received the event
			TEST_ASSERT(callbackLog1.Count() == 0); // callback1 should not receive events after detach
			
			const wchar_t* expected2[] = {
				L"OnItemModified(start=3, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog2, expected2);
			
			// Detach the second callback and verify no callbacks fire
			nodeItemProvider->DetachCallback(&callback2);
			callbackLog2.Clear();
			
			// Perform another operation
			auto child4 = CreateTreeViewItem(L"Child4");
			parentMemoryNode->Children().Add(child4);
			
			// Verify neither callback received events
			TEST_ASSERT(callbackLog1.Count() == 0);
			TEST_ASSERT(callbackLog2.Count() == 0);
		});
	});

	TEST_CATEGORY(L"TreeViewItemBindableRootProviderIntegration")
	{
		auto CreateMultiLevelTree = [](const WString& rootName, vint level1Count, vint level2Count) -> Ptr<BindableItem>
		{
			auto root = Ptr(new BindableItem());
			root->name = rootName;
			for (vint i = 0; i < level1Count; i++)
			{
				auto child = Ptr(new BindableItem());
				child->name = rootName + L".Child" + itow(i + 1);
				root->children.Add(child);
				for (vint j = 0; j < level2Count; j++)
				{
					auto grandchild = Ptr(new BindableItem());
					grandchild->name = child->name + L".GrandChild" + itow(j + 1);
					child->children.Add(grandchild);
				}
			}
			return root;
		};

		TEST_CASE(L"BasicBindableProviderIntegration")
		{
			// Setup: Create bindable provider with property bindings
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			provider->textProperty = BindableItem::Prop_name();
			provider->childrenProperty = BindableItem::Prop_children();
			
			// Create a 2-level tree: Root -> 3 children -> 2 grandchildren each
			auto rootItem = CreateMultiLevelTree(L"Root", 3, 2);
			provider->SetItemSource(BoxValue(rootItem));
			
			// Wrap with NodeItemProvider (adapter pattern)
			auto nodeItemProvider = Ptr(new NodeItemProvider(provider));
			
			// Phase 1: Verify initial state (all nodes collapsed)
			// Should only see the 3 top-level children (Child1, Child2, Child3)
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			
			// Through IItemProvider interface - GetTextValue
			TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Root.Child1");
			TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Root.Child2");
			TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Root.Child3");
			
			// Through IItemProvider interface - GetBindingValue
			auto binding0 = nodeItemProvider->GetBindingValue(0);
			auto binding1 = nodeItemProvider->GetBindingValue(1);
			auto binding2 = nodeItemProvider->GetBindingValue(2);
			
			// Unbox and verify bindable item data
			auto item0 = UnboxValue<Ptr<BindableItem>>(binding0);
			auto item1 = UnboxValue<Ptr<BindableItem>>(binding1);
			auto item2 = UnboxValue<Ptr<BindableItem>>(binding2);
			
			TEST_ASSERT(item0->name == L"Root.Child1");
			TEST_ASSERT(item1->name == L"Root.Child2");
			TEST_ASSERT(item2->name == L"Root.Child3");
			
			// Verify children property is accessible from bindable items
			TEST_ASSERT(item0->children.Count() == 2); // Child1 has 2 grandchildren
			TEST_ASSERT(item0->children[0]->name == L"Root.Child1.GrandChild1");
			TEST_ASSERT(item0->children[1]->name == L"Root.Child1.GrandChild2");
			
			// Phase 2: Expand the first child node
			// Through INodeProvider interface
			auto child1Node = nodeItemProvider->RequestNode(0);
			TEST_ASSERT(child1Node != nullptr);
			
			child1Node->SetExpanding(true);
			
			// Verify count increased (now showing 5 items: Child1, GrandChild1.1, GrandChild1.2, Child2, Child3)
			TEST_ASSERT(nodeItemProvider->Count() == 5);
			
			// Verify the visible sequence using AssertItems helper
			const wchar_t* expectedAfterExpand[] = {
				L"Root.Child1",
				L"Root.Child1.GrandChild1",
				L"Root.Child1.GrandChild2",
				L"Root.Child2",
				L"Root.Child3"
			};
			AssertItems(nodeItemProvider, expectedAfterExpand);
			
			// Verify data retrieval still works after expansion
			auto bindingGrandChild1 = nodeItemProvider->GetBindingValue(1);
			auto itemGrandChild1 = UnboxValue<Ptr<BindableItem>>(bindingGrandChild1);
			TEST_ASSERT(itemGrandChild1->name == L"Root.Child1.GrandChild1");
			TEST_ASSERT(itemGrandChild1->children.Count() == 0); // Grandchildren have no children
			
			auto bindingGrandChild2 = nodeItemProvider->GetBindingValue(2);
			auto itemGrandChild2 = UnboxValue<Ptr<BindableItem>>(bindingGrandChild2);
			TEST_ASSERT(itemGrandChild2->name == L"Root.Child1.GrandChild2");
			
			// Phase 3: Collapse the first child node
			child1Node->SetExpanding(false);
			
			// Verify count decreased back to original (3 items: Child1, Child2, Child3)
			TEST_ASSERT(nodeItemProvider->Count() == 3);
			
			// Verify the visible sequence is back to original
			const wchar_t* expectedAfterCollapse[] = {
				L"Root.Child1",
				L"Root.Child2",
				L"Root.Child3"
			};
			AssertItems(nodeItemProvider, expectedAfterCollapse);
			
			// Verify data retrieval still works correctly after collapse
			auto bindingAfterCollapse = nodeItemProvider->GetBindingValue(0);
			auto itemAfterCollapse = UnboxValue<Ptr<BindableItem>>(bindingAfterCollapse);
			TEST_ASSERT(itemAfterCollapse->name == L"Root.Child1");
			TEST_ASSERT(itemAfterCollapse->children.Count() == 2); // Data unchanged
		});
	});
}