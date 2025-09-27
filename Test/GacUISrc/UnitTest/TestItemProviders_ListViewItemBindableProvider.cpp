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
		
		// Create ListViewItemBindableProvider and attach callbacks
		auto provider = Ptr(new ListViewItemBindableProvider());
		provider->AttachCallback(&itemCallback);
		provider->AttachCallback(&columnCallback);
		
		// Create 3 columns for 3 WString members
		auto& columns = provider->GetColumns();
		columns.Add(Ptr(new ListViewColumn(L"Name", 100)));
		columns.Add(Ptr(new ListViewColumn(L"Title", 100)));
		columns.Add(Ptr(new ListViewColumn(L"Description", 100)));
		
		// Set text properties for the columns
		columns[0]->SetTextProperty(BindableItem::Prop_name());
		columns[1]->SetTextProperty(BindableItem::Prop_title());
		columns[2]->SetTextProperty(BindableItem::Prop_desc());
		
		// Add data column indices to display columns 1 and 2 (0 is the main text column)
		auto& dataColumns = provider->GetDataColumns();
		dataColumns.Add(1);
		dataColumns.Add(2);
		
		// Create ObservableList<Ptr<BindableItem>> and set as item source
		auto items = Ptr(new ObservableList<Ptr<BindableItem>>());
		provider->SetItemSource(BoxParameter(items));
		
		// Action: Add one BindableItem to the list
		auto item = Ptr(new BindableItem());
		item->name = L"Test Name";
		item->title = L"Test Title";
		item->desc = L"Test Description";
		items->Add(item);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnColumnRebuilt()",
			L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
			L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}