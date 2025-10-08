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
}
