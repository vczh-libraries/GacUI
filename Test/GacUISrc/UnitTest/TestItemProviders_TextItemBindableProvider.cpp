#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		
		// Create TextItemBindableProvider and attach callbacks
		auto provider = Ptr(new TextItemBindableProvider());
		provider->AttachCallback(&itemCallback);
		
		// Set text property first
		provider->textProperty = BindableItem::Prop_name();
		
		// Create ObservableList<Ptr<BindableItem>> and set as item source
		auto items = Ptr(new ObservableList<Ptr<BindableItem>>());
		provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
		
		// Action: Add one BindableItem to the list
		auto item = Ptr(new BindableItem());
		item->name = L"Test Item";
		items->Add(item);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
			L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}