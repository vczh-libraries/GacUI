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
		
		// Verify node retrieval
		auto retrievedNode = nodeItemProvider->RequestNode(0);
		TEST_ASSERT(retrievedNode != nullptr);
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
}
