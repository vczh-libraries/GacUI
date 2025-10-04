#include "TestItemProviders.h"
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		// Create TreeViewItemRootProvider and attach callbacks
		auto provider = Ptr(new TreeViewItemRootProvider);
		provider->AttachCallback(&nodeCallback);
		
		// Action: Create MemoryNodeProvider with TreeViewItem data and add it to root provider
		auto item = Ptr(new TreeViewItem(nullptr, L"Test Item"));
		auto node = Ptr(new MemoryNodeProvider(item));
		provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});

	TEST_CASE(L"AttachCallback")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemRootProvider);
		provider->AttachCallback(&nodeCallback);
		
		// Don't clear - testing AttachCallback itself
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)"
		};
		AssertCallbacks(callbackLog, expected);
	});

	TEST_CATEGORY(L"BasicNodeOperations")
	{
		TEST_CASE(L"AddSingleNodeToRoot")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();  // Clear after attach
			
			// Add one node to root
			auto node = CreateTreeViewItem(L"Node1");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			
			// Verify Before/After callbacks with itemReferenceUpdated=true (structural change)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"AddMultipleNodesToRoot")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add three nodes sequentially
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node2 = CreateTreeViewItem(L"Node2");
			auto node3 = CreateTreeViewItem(L"Node3");
			
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			rootMemoryNode->Children().Add(node3);
			
			// Verify three pairs of Before/After callbacks
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"RemoveSingleNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node first
			auto node = CreateTreeViewItem(L"Node1");
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on removal
			
			// Remove the node
			rootMemoryNode->Children().RemoveAt(0);
			
			// Verify Before/After callbacks for removal
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"RemoveRangeOfNodes")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add multiple nodes
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			for (vint i = 0; i < 5; i++)
			{
				auto node = CreateTreeViewItem(L"Node" + itow(i + 1));
				rootMemoryNode->Children().Add(node);
			}
			callbackLog.Clear();  // Clear to focus on removal
			
			// Remove range of 3 nodes (indices 1, 2, 3)
			rootMemoryNode->Children().RemoveRange(1, 3);
			
			// Verify Before callbacks for each individual item, then single After callback
			// Note: Base class calls BeforeRemove once per item with original indices (1,2,3),
			// then AfterRemove once for the entire range
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=2, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=3, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"HierarchicalStructure")
	{
		TEST_CASE(L"MultiLevelTreeConstruction")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Build tree: Root -> Parent1 -> Child1, Child2
			auto parent1 = CreateTreeViewItem(L"Parent1");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent1);
			
			auto child1 = CreateTreeViewItem(L"Child1");
			auto child2 = CreateTreeViewItem(L"Child2");
			parent1->Children().Add(child1);
			parent1->Children().Add(child2);
			
			// Verify three pairs of callbacks (one for parent, two for children)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"AddChildToExistingNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add parent node
			auto parent = CreateTreeViewItem(L"Parent");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent);
			callbackLog.Clear();  // Clear to focus on child addition
			
			// Add child to the parent
			auto child = CreateTreeViewItem(L"Child");
			parent->Children().Add(child);
			
			// Verify callbacks fire for child addition
			const wchar_t* expected[] = {
				L"Parent->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"RemoveNodeWithChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Build tree with children
			auto parent = CreateTreeViewItem(L"Parent");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent);
			
			auto child1 = CreateTreeViewItem(L"Child1");
			auto child2 = CreateTreeViewItem(L"Child2");
			parent->Children().Add(child1);
			parent->Children().Add(child2);
			callbackLog.Clear();  // Clear to focus on removal
			
			// Remove parent node (should implicitly remove children)
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().RemoveAt(0);
			
			// Verify single Before/After pair for parent removal
			// Children are implicitly removed, not separately notified
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"DeepTreeConstruction")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Build 4-level deep tree
			auto level1 = CreateTreeViewItem(L"Level1");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(level1);
			
			auto level2 = CreateTreeViewItem(L"Level2");
			level1->Children().Add(level2);
			
			auto level3 = CreateTreeViewItem(L"Level3");
			level2->Children().Add(level3);
			
			auto level4 = CreateTreeViewItem(L"Level4");
			level3->Children().Add(level4);
			
			// Verify four pairs of callbacks (one per level)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level2->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level2->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level3->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Level3->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"ExpansionCollapseOperations")
	{
		TEST_CASE(L"ExpandNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add a node
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on expansion
			
			// Expand the node
			node->SetExpanding(true);
			
			// Verify OnItemExpanded callback fires
			const wchar_t* expected[] = {
				L"Node->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"CollapseNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add and expand a node
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			node->SetExpanding(true);
			callbackLog.Clear();  // Clear to focus on collapse
			
			// Collapse the node
			node->SetExpanding(false);
			
			// Verify OnItemCollapsed callback fires
			const wchar_t* expected[] = {
				L"Node->OnItemCollapsed()"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ExpandAlreadyExpandedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add and expand a node
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			node->SetExpanding(true);
			callbackLog.Clear();  // Clear to focus on second expansion
			
			// Try expanding again
			node->SetExpanding(true);
			
			// Verify no callback fires (already expanded)
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"CollapseAlreadyCollapsedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add a node (collapsed by default)
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on collapse attempt
			
			// Try collapsing (already collapsed)
			node->SetExpanding(false);
			
			// Verify no callback fires (already collapsed)
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"ExpandLeafNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add a leaf node (no children)
			auto leaf = CreateTreeViewItem(L"Leaf");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(leaf);
			callbackLog.Clear();  // Clear to focus on expansion
			
			// Expand the leaf node
			leaf->SetExpanding(true);
			
			// Verify callback fires even though there are no children
			const wchar_t* expected[] = {
				L"Leaf->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ModifyChildrenWhileExpanded")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add and expand parent node
			auto parent = CreateTreeViewItem(L"Parent");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent);
			parent->SetExpanding(true);
			callbackLog.Clear();  // Clear to focus on child operations
			
			// Add child while parent is expanded
			auto child = CreateTreeViewItem(L"Child");
			parent->Children().Add(child);
			
			// Verify Before/After callbacks fire normally
			const wchar_t* expected[] = {
				L"Parent->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify parent remains expanded
			TEST_ASSERT(parent->GetExpanding() == true);
		});

		TEST_CASE(L"RapidExpansionCollapse")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on expansion/collapse
			
			// Rapidly expand and collapse
			node->SetExpanding(true);
			node->SetExpanding(false);
			node->SetExpanding(true);
			node->SetExpanding(false);
			
			// Verify all callbacks fire in correct order
			const wchar_t* expected[] = {
				L"Node->OnItemExpanded()",
				L"Node->OnItemCollapsed()",
				L"Node->OnItemExpanded()",
				L"Node->OnItemCollapsed()"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"DataModification")
	{
		TEST_CASE(L"UpdateTreeViewDataTriggersCallbacks")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node with TreeViewItem data
			auto node = CreateTreeViewItem(L"Original");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on data update
			
			// Modify TreeViewItem data
			auto item = node->GetData().Cast<TreeViewItem>();
			item->text = L"Modified";
			
			// Call UpdateTreeViewData to notify observers
			provider->UpdateTreeViewData(node.Obj());
			
			// Verify Before/After callbacks with itemReferenceUpdated=false (data change only)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"NotifyDataModifiedTriggersCallbacks")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on notification
			
			// Call NotifyDataModified directly on the MemoryNodeProvider
			node->NotifyDataModified();
			
			// Verify Before/After callbacks with itemReferenceUpdated=false
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ModifyPropertiesWithoutUpdate")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node with TreeViewItem
			auto node = CreateTreeViewItem(L"Original");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on property modification
			
			// Modify TreeViewItem properties WITHOUT calling UpdateTreeViewData
			auto item = node->GetData().Cast<TreeViewItem>();
			item->text = L"Modified";
			item->tag = BoxValue<vint>(42);
			
			// Verify NO callbacks fire (manual notification required)
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"ModifyPropertiesThenUpdate")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node with TreeViewItem
			auto node = CreateTreeViewItem(L"Original");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on update sequence
			
			// Modify TreeViewItem properties
			auto item = node->GetData().Cast<TreeViewItem>();
			item->text = L"Modified";
			item->tag = BoxValue<vint>(100);
			
			// Now call UpdateTreeViewData
			provider->UpdateTreeViewData(node.Obj());
			
			// Verify callbacks fire after manual notification
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"SetTreeViewDataAndUpdate")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node
			auto node = CreateTreeViewItem(L"Original");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			callbackLog.Clear();  // Clear to focus on data replacement
			
			// Replace TreeViewItem data entirely
			auto newItem = Ptr(new TreeViewItem(nullptr, L"Replaced"));
			node->SetData(newItem);  // SetData automatically calls NotifyDataModified
			
			// Call UpdateTreeViewData to notify again
			provider->UpdateTreeViewData(node.Obj());
			
			// Verify callbacks fire twice (once from SetData, once from UpdateTreeViewData)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"InterfaceMethodsAndNavigation")
	{
		TEST_CASE(L"RequestViewITreeViewItemView")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Test RequestView with ITreeViewItemView identifier
			auto treeViewItemView = provider->RequestView(ITreeViewItemView::Identifier);
			TEST_ASSERT(treeViewItemView != nullptr);
			
			// Merge with dynamic_cast test
			auto castedView = dynamic_cast<ITreeViewItemView*>(provider.Obj());
			TEST_ASSERT(castedView != nullptr);
			TEST_ASSERT(treeViewItemView == castedView);
		});

		TEST_CASE(L"RequestViewINodeRootProvider")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Test dynamic_cast to INodeRootProvider (no RequestView identifier available)
			auto castedProvider = dynamic_cast<INodeRootProvider*>(provider.Obj());
			TEST_ASSERT(castedProvider != nullptr);
		});

		TEST_CASE(L"GetMemoryNodeReturnsCorrectNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add nodes to build tree structure
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node2 = CreateTreeViewItem(L"Node2");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node1);
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node2);
			
			// GetMemoryNode should return the MemoryNodeProvider from INodeProvider interface
			auto memoryNode1 = provider->GetMemoryNode(node1.Obj());
			auto memoryNode2 = provider->GetMemoryNode(node2.Obj());
			
			TEST_ASSERT(memoryNode1 == node1.Obj());
			TEST_ASSERT(memoryNode2 == node2.Obj());
		});

		TEST_CASE(L"GetRootNodeReturnsRoot")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Get root node
			auto rootNode = provider->GetRootNode();
			TEST_ASSERT(rootNode != nullptr);
			
			// Root node should have no parent
			auto rootMemoryNode = provider->GetMemoryNode(rootNode.Obj());
			TEST_ASSERT(rootMemoryNode->GetParent() == nullptr);
		});

		TEST_CASE(L"GetTextValueRetrievesText")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node with specific text
			auto node = CreateTreeViewItem(L"Test Text");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			
			// Get text value through INodeRootProvider interface
			auto nodeRootProvider = dynamic_cast<INodeRootProvider*>(provider.Obj());
			auto text = nodeRootProvider->GetTextValue(node.Obj());
			TEST_ASSERT(text == L"Test Text");
		});

		TEST_CASE(L"GetBindingValueRetrievesTag")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node with tag data
			auto node = CreateTreeViewItem(L"Node");
			auto item = node->GetData().Cast<TreeViewItem>();
			item->tag = BoxValue<vint>(12345);
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			
			// Get binding value (tag) through INodeRootProvider interface
			auto nodeRootProvider = dynamic_cast<INodeRootProvider*>(provider.Obj());
			auto item2 = UnboxValue<Ptr<TreeViewItem>>(nodeRootProvider->GetBindingValue(node.Obj()));
			TEST_ASSERT(UnboxValue<vint>(item2->tag) == 12345);
		});
	});

	TEST_CATEGORY(L"CallbackCoordination")
	{
		TEST_CASE(L"BeforeAfterCallbackPairsAlwaysTogether")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Perform multiple operations
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			auto node1 = CreateTreeViewItem(L"Node1");
			auto node2 = CreateTreeViewItem(L"Node2");
			
			rootMemoryNode->Children().Add(node1);
			rootMemoryNode->Children().Add(node2);
			rootMemoryNode->Children().RemoveAt(0);
			
			// Verify each operation has Before followed immediately by After
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"CallbackOrderingMultipleOperations")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Complex sequence: add, expand, add child, modify data, collapse, remove
			auto parent = CreateTreeViewItem(L"Parent");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent);
			
			parent->SetExpanding(true);
			
			auto child = CreateTreeViewItem(L"Child");
			parent->Children().Add(child);
			
			provider->UpdateTreeViewData(parent.Obj());
			
			parent->SetExpanding(false);
			
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().RemoveAt(0);
			
			// Verify exact callback sequence
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent->OnItemExpanded()",
				L"Parent->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Parent->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Parent->OnItemCollapsed()",
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"DetachCallbackFiresOnAttachedNull")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Detach callback
			provider->DetachCallback(&nodeCallback);
			
			// Verify OnAttached fires with provider=null
			const wchar_t* expected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"NoCallbacksAfterDetach")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			provider->DetachCallback(&nodeCallback);
			callbackLog.Clear();  // Clear after detach
			
			// Perform operations after detach
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			node->SetExpanding(true);
			provider->UpdateTreeViewData(node.Obj());
			
			// Verify NO callbacks fire
			TEST_ASSERT(callbackLog.Count() == 0);
		});
	});

	TEST_CATEGORY(L"EdgeCasesAndErrorHandling")
	{
		TEST_CASE(L"AddDuplicateNodeThrowsException")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add node to root
			auto node = CreateTreeViewItem(L"Node");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node);
			
			// Try adding same node again - should throw ArgumentException
			TEST_EXCEPTION(
				provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(node),
				ArgumentException,
				[](const ArgumentException& e)
				{
					// Verify exception message mentions duplicate/parent
					WString msg = e.Message();
					return msg.Length() > 0;
				}
			);
		});

		TEST_CASE(L"AddNodeToMultipleParentsThrowsException")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add two parent nodes
			auto parent1 = CreateTreeViewItem(L"Parent1");
			auto parent2 = CreateTreeViewItem(L"Parent2");
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent1);
			provider->GetMemoryNode(provider->GetRootNode().Obj())->Children().Add(parent2);
			
			// Add child to parent1
			auto child = CreateTreeViewItem(L"Child");
			parent1->Children().Add(child);
			
			// Try adding same child to parent2 - should throw
			TEST_EXCEPTION(
				parent2->Children().Add(child),
				ArgumentException,
				[](const ArgumentException& e){ return true; }
			);
		});

		TEST_CASE(L"AddNodeAfterRemoving")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add then remove a node
			auto node = CreateTreeViewItem(L"Node");
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			rootMemoryNode->Children().Add(node);
			rootMemoryNode->Children().RemoveAt(0);
			callbackLog.Clear();  // Clear to focus on re-addition
			
			// Add the same node again - should succeed (parent was cleared on removal)
			rootMemoryNode->Children().Add(node);
			
			// Verify callbacks fire normally
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"GetChildWithInvalidIndex")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Add one node
			auto node = CreateTreeViewItem(L"Node");
			auto rootNode = provider->GetRootNode();
			provider->GetMemoryNode(rootNode.Obj())->Children().Add(node);
			
			// Try accessing child with out-of-bounds index
			TEST_ERROR(rootNode->GetChild(999));
			TEST_ERROR(rootNode->GetChild(-1));
		});

		TEST_CASE(L"RemoveNonexistentNodeHasNoEffect")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Create node but don't add it
			auto node = CreateTreeViewItem(L"Node");
			
			// Try removing at empty index
			auto rootMemoryNode = provider->GetMemoryNode(provider->GetRootNode().Obj());
			
			// RemoveAt on empty collection should return false (no effect)
			TEST_ASSERT(rootMemoryNode->Children().RemoveAt(0) == false);
			
			// Verify no callbacks fired
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"EmptyTreeOperations")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Operations on empty tree
			auto rootNode = provider->GetRootNode();
			TEST_ASSERT(rootNode != nullptr);
			TEST_ASSERT(rootNode->GetChildCount() == 0);
			
			// GetChild on empty should error
			TEST_ERROR(rootNode->GetChild(0));
			
			// Verify no callbacks fired
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"DeepTreeNavigation")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemRootProvider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Build 5-level deep tree
			auto current = provider->GetMemoryNode(provider->GetRootNode().Obj());
			Ptr<MemoryNodeProvider> deepestNode;
			
			for (vint i = 0; i < 5; i++)
			{
				auto node = CreateTreeViewItem(L"Level" + itow(i + 1));
				current->Children().Add(node);
				deepestNode = node;
				current = node.Obj();
			}
			
			// Navigate back up through parents
			auto node = deepestNode;
			vint levelCount = 0;
			while (node->GetParent() != nullptr)
			{
				node = Ptr(provider->GetMemoryNode(node->GetParent().Obj()));
				levelCount++;
			}
			
			// Should have navigated 5 levels (excluding root)
			TEST_ASSERT(levelCount == 5);
		});
	});
}