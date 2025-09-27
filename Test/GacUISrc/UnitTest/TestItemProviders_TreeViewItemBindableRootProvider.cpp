#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	auto InitProvider = [](Ptr<TreeViewItemBindableRootProvider> provider)
	{
		// Set text property first
		provider->textProperty = BindableItem::Prop_name();
		provider->childrenProperty = BindableItem::Prop_children();

		// Create a root BindableItem and set as item source
		auto rootItem = Ptr(new BindableItem());
		rootItem->name = L"Root Item";
		provider->SetItemSource(BoxValue(rootItem));

		return rootItem;
	};

	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		// Create TreeViewItemBindableRootProvider and attach callbacks
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		auto rootItem = InitProvider(provider);
		provider->AttachCallback(&nodeCallback);
		
		// Action: Add another BindableItem to its children
		auto childItem = Ptr(new BindableItem());
		childItem->name = L"Child Item";
		rootItem->children.Add(childItem);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
			L"OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}