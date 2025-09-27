#include "TestItemProviders.h"

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
		provider->GetRootNode().Cast<MemoryNodeProvider>()->Children().Add(node);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
			L"OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}