#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		MockColumnItemViewCallback columnCallback(callbackLog);
		
		// Create ListViewItemProvider and attach callbacks
		auto provider = Ptr(new ListViewItemProvider);
		static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
		provider->AttachCallback(&columnCallback);
		
		// Action: Add one ListViewItem
		auto item = Ptr(new ListViewItem);
		item->SetText(L"Test Item");
		provider->Add(item);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}