#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockTextItemProviderCallback textCallback(callbackLog);
		MockItemProviderCallback itemCallback(callbackLog);
		
		// Create TextItemProvider and attach callbacks
		TextItemProvider provider(&textCallback);
		provider.AttachCallback(&itemCallback);
		
		// Action: Add one TextItem
		auto item = Ptr(new TextItem(L"Test Item", false));
		provider.Add(item);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}