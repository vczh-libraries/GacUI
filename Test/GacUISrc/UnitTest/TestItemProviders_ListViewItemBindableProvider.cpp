#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	auto InitProvider = [](Ptr<ListViewItemBindableProvider> provider, ObservableList<Ptr<BindableItem>>& items)
	{
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

		// Set item source
		provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
	};

	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		MockColumnItemViewCallback columnCallback(callbackLog);
		
		// Create ListViewItemBindableProvider and attach callbacks
		auto provider = Ptr(new ListViewItemBindableProvider());
		
		// Create ObservableList<Ptr<BindableItem>> and set as item source
		ObservableList<Ptr<BindableItem>> items;
		InitProvider(provider, items);
		static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
		provider->AttachCallback(&columnCallback);
		
		// Action: Add one BindableItem to the list
		auto item = Ptr(new BindableItem());
		item->name = L"Test Name";
		item->title = L"Test Title";
		item->desc = L"Test Description";
		items.Add(item);
		
		// Verification: Use helper function for better diagnostics
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)",
			L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});

	TEST_CATEGORY(L"PropertyBindingFunctionality")
	{
		TEST_CASE(L"TextPropertyBindingForMainColumn")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestName";
			items.Add(item);
			
			TEST_ASSERT(provider->GetText(0) == L"TestName");
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"TextPropertyBindingForSubItems")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Name1";
			item->title = L"Title1";
			item->desc = L"Desc1";
			items.Add(item);
			
			TEST_ASSERT(provider->GetSubItem(0, 0) == L"Title1");
			TEST_ASSERT(provider->GetSubItem(0, 1) == L"Desc1");
		});
		
		TEST_CASE(L"MultipleColumnPropertyBindings")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			
			auto& columns = provider->GetColumns();
			columns.Add(Ptr(new ListViewColumn(L"Extra", 100)));
			columns[3]->SetTextProperty(BindableItem::Prop_name());
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"N";
			item->title = L"T";
			item->desc = L"D";
			items.Add(item);
			
			TEST_ASSERT(provider->GetText(0) == L"N");
			TEST_ASSERT(provider->GetSubItem(0, 0) == L"T");
			TEST_ASSERT(provider->GetSubItem(0, 1) == L"D");
			TEST_ASSERT(provider->GetSubItem(0, 2) == L"N");
		});
		
		TEST_CASE(L"PropertyBindingWithVaryingSubitems")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item1 = Ptr(new BindableItem());
			item1->name = L"Item1";
			item1->title = L"";
			item1->desc = L"Desc1";
			items.Add(item1);
			
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Item2";
			item2->title = L"Title2";
			item2->desc = L"";
			items.Add(item2);
			
			TEST_ASSERT(provider->GetText(0) == L"Item1");
			TEST_ASSERT(provider->GetSubItem(0, 0) == L"");
			TEST_ASSERT(provider->GetSubItem(0, 1) == L"Desc1");
			TEST_ASSERT(provider->GetText(1) == L"Item2");
			TEST_ASSERT(provider->GetSubItem(1, 0) == L"Title2");
			TEST_ASSERT(provider->GetSubItem(1, 1) == L"");
		});
		
		TEST_CASE(L"PropertyBindingNullValues")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			items.Add(item);
			
			TEST_ASSERT(provider->GetText(0) == L"");
			TEST_ASSERT(provider->GetSubItem(0, 0) == L"");
			TEST_ASSERT(provider->GetSubItem(0, 1) == L"");
		});
	});

	TEST_CATEGORY(L"ColumnManagementIntegration")
	{
		TEST_CASE(L"ColumnAdditionWithoutItems")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns.Add(Ptr(new ListViewColumn(L"Extra", 100)));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"ColumnAdditionWithItems")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns.Add(Ptr(new ListViewColumn(L"Extra", 100)));
			columns[3]->SetTextProperty(BindableItem::Prop_name());
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=true)",
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"ColumnRemoval")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns.RemoveAt(1);
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"ColumnTextChange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns[0]->SetText(L"NewName");
			
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"ColumnSizeChange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			callbackLog.Clear();
			
			provider->SetColumnSize(0, 200);
			
			TEST_ASSERT(provider->GetColumnSize(0) == 200);
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"ColumnTextPropertyBinding")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			item->title = L"Title1";
			items.Add(item);
			
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns[1]->SetTextProperty(BindableItem::Prop_desc());
			
			TEST_ASSERT(provider->GetSubItem(0, 0) == L"");
			
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"DataColumnOperations")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			callbackLog.Clear();
			
			TEST_ASSERT(provider->GetDataColumnCount() == 2);
			TEST_ASSERT(provider->GetDataColumn(0) == 1);
			TEST_ASSERT(provider->GetDataColumn(1) == 2);
			
			auto& dataColumns = provider->GetDataColumns();
			dataColumns.Add(0);
			
			TEST_ASSERT(provider->GetDataColumnCount() == 3);
			TEST_ASSERT(provider->GetDataColumn(2) == 0);
		});
		
		TEST_CASE(L"MultipleColumnsAddedSequentially")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			
			columns.Add(Ptr(new ListViewColumn(L"Col1", 100)));
			columns.Add(Ptr(new ListViewColumn(L"Col2", 100)));
			columns.Add(Ptr(new ListViewColumn(L"Col3", 100)));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(columns.Count() == 3);
		});
	});

	TEST_CATEGORY(L"ObservableListIntegration")
	{
		TEST_CASE(L"ItemSourceTransitionNullToItems")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			auto& columns = provider->GetColumns();
			columns.Add(Ptr(new ListViewColumn(L"Name", 100)));
			columns.Add(Ptr(new ListViewColumn(L"Title", 100)));
			columns.Add(Ptr(new ListViewColumn(L"Description", 100)));
			columns[0]->SetTextProperty(BindableItem::Prop_name());
			columns[1]->SetTextProperty(BindableItem::Prop_title());
			columns[2]->SetTextProperty(BindableItem::Prop_desc());
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 1);
			TEST_ASSERT(provider->GetText(0) == L"Test");
		});
		
		TEST_CASE(L"ItemSourceTransitionItemsToItems")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items1;
			
			InitProvider(provider, items1);
			
			auto item1 = Ptr(new BindableItem());
			item1->name = L"First";
			items1.Add(item1);
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			callbackLog.Clear();
			
			ObservableList<Ptr<BindableItem>> items2;
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Second";
			items2.Add(item2);
			items2.Add(Ptr(new BindableItem()));
			
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items2)));
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 2);
			TEST_ASSERT(provider->GetText(0) == L"Second");
		});
		
		TEST_CASE(L"ObservableListAddPropagation")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item1 = Ptr(new BindableItem());
			item1->name = L"Item1";
			items.Add(item1);
			
			callbackLog.Clear();
			
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Item2";
			items.Add(item2);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 2);
			TEST_ASSERT(provider->GetText(1) == L"Item2");
		});
		
		TEST_CASE(L"ObservableListRemovalPropagation")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item1 = Ptr(new BindableItem());
			item1->name = L"Item1";
			items.Add(item1);
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Item2";
			items.Add(item2);
			
			callbackLog.Clear();
			
			items.RemoveAt(0);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 1);
			TEST_ASSERT(provider->GetText(0) == L"Item2");
		});
		
		TEST_CASE(L"ObservableListRemoveRange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			for (vint i = 0; i < 5; i++)
			{
				auto item = Ptr(new BindableItem());
				item->name = L"Item" + itow(i);
				items.Add(item);
			}
			
			callbackLog.Clear();
			
			items.RemoveRange(1, 2);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 3);
			TEST_ASSERT(provider->GetText(0) == L"Item0");
			TEST_ASSERT(provider->GetText(1) == L"Item3");
			TEST_ASSERT(provider->GetText(2) == L"Item4");
		});
		
		TEST_CASE(L"ManualPropertyChangeNotification")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Original";
			items.Add(item);
			
			callbackLog.Clear();
			
			item->name = L"Modified";
			
			provider->NotifyUpdate(0, 1, false);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetText(0) == L"Modified");
		});
	});

	TEST_CATEGORY(L"DualCallbackSystem")
	{
		TEST_CASE(L"BothCallbacksCoordination")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"OnColumnRebuiltTriggersItemRebuild")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			items.Add(item);
			
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns.Add(Ptr(new ListViewColumn(L"New", 100)));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"OnColumnChangedTriggersItemRefresh")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			items.Add(item);
			
			callbackLog.Clear();
			
			auto& columns = provider->GetColumns();
			columns[0]->SetSize(200);
			
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"CallbackDetachmentBothTypes")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			callbackLog.Clear();
			
			static_cast<IItemProvider*>(provider.Obj())->DetachCallback(&itemCallback);
			provider->DetachCallback(&columnCallback);
			
			auto item = Ptr(new BindableItem());
			items.Add(item);

			const wchar_t* expected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"InterfaceMethodValidation")
	{
		TEST_CASE(L"RequestViewIListViewItemView")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			item->title = L"Title";
			items.Add(item);
			
			auto view = dynamic_cast<IListViewItemView*>(provider->RequestView(IListViewItemView::Identifier));
			TEST_ASSERT(view != nullptr);
			
			TEST_ASSERT(view->GetText(0) == L"Test");
			TEST_ASSERT(view->GetSubItem(0, 0) == L"Title");
			TEST_ASSERT(view->GetColumnCount() == 3);
		});
		
		TEST_CASE(L"RequestViewIColumnItemView")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto view = dynamic_cast<IColumnItemView*>(provider->RequestView(IColumnItemView::Identifier));
			TEST_ASSERT(view != nullptr);
			
			TEST_ASSERT(view->GetColumnSize(0) == 100);
			view->SetColumnSize(0, 150);
			TEST_ASSERT(view->GetColumnSize(0) == 150);
		});
		
		TEST_CASE(L"IListViewItemViewMethods")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Name1";
			item->title = L"Title1";
			item->desc = L"Desc1";
			items.Add(item);
			
			auto view = dynamic_cast<IListViewItemView*>(provider->RequestView(IListViewItemView::Identifier));
			
			TEST_ASSERT(view->GetText(0) == L"Name1");
			
			TEST_ASSERT(view->GetSubItem(0, 0) == L"Title1");
			TEST_ASSERT(view->GetSubItem(0, 1) == L"Desc1");
			
			TEST_ASSERT(view->GetColumnCount() == 3);
			
			TEST_ASSERT(view->GetColumnText(0) == L"Name");
			TEST_ASSERT(view->GetColumnText(1) == L"Title");
			TEST_ASSERT(view->GetColumnText(2) == L"Description");
			
			TEST_ASSERT(view->GetDataColumnCount() == 2);
			TEST_ASSERT(view->GetDataColumn(0) == 1);
			TEST_ASSERT(view->GetDataColumn(1) == 2);
		});
		
		TEST_CASE(L"IColumnItemViewMethods")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto view = dynamic_cast<IColumnItemView*>(provider->RequestView(IColumnItemView::Identifier));
			
			TEST_ASSERT(view->GetColumnSize(0) == 100);
			view->SetColumnSize(0, 200);
			TEST_ASSERT(view->GetColumnSize(0) == 200);
			
			TEST_ASSERT(view->GetColumnSize(1) == 100);
			view->SetColumnSize(1, 150);
			TEST_ASSERT(view->GetColumnSize(1) == 150);
			
			TEST_ASSERT(view->GetSortingState(0) == ColumnSortingState::NotSorted);
			
			List<WString> callbackLog;
			MockColumnItemViewCallback columnCallback(callbackLog);
			TEST_ASSERT(view->AttachCallback(&columnCallback) == true);
			TEST_ASSERT(view->AttachCallback(&columnCallback) == false);
			TEST_ASSERT(view->DetachCallback(&columnCallback) == true);
			TEST_ASSERT(view->DetachCallback(&columnCallback) == false);
		});
		
		TEST_CASE(L"InterfaceMethodEquivalence")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			item->title = L"Title";
			items.Add(item);
			
			auto listViewView = dynamic_cast<IListViewItemView*>(provider->RequestView(IListViewItemView::Identifier));
			auto columnView = dynamic_cast<IColumnItemView*>(provider->RequestView(IColumnItemView::Identifier));
			
			TEST_ASSERT(listViewView->GetText(0) == provider->GetText(0));
			TEST_ASSERT(listViewView->GetSubItem(0, 0) == provider->GetSubItem(0, 0));
			TEST_ASSERT(listViewView->GetColumnCount() == provider->GetColumnCount());
			TEST_ASSERT(listViewView->GetColumnText(0) == provider->GetColumnText(0));
			
			TEST_ASSERT(columnView->GetColumnSize(0) == provider->GetColumnSize(0));
			
			columnView->SetColumnSize(0, 250);
			TEST_ASSERT(provider->GetColumnSize(0) == 250);
			
			provider->SetColumnSize(1, 175);
			TEST_ASSERT(columnView->GetColumnSize(1) == 175);
		});
	});

	TEST_CATEGORY(L"EdgeCasesAndErrorHandling")
	{
		TEST_CASE(L"SubitemAccessBeyondColumns")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			TEST_ASSERT(provider->GetSubItem(0, 0) != L"INVALID");
			TEST_ASSERT(provider->GetSubItem(0, 1) != L"INVALID");
			
			TEST_ERROR(provider->GetSubItem(0, 2));
			TEST_ERROR(provider->GetSubItem(0, 5));
			TEST_ERROR(provider->GetSubItem(0, -1));
		});
		
		TEST_CASE(L"SubitemAccessExtremeIndex")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			items.Add(item);
			
			TEST_ERROR(provider->GetSubItem(0, 100));
			TEST_ERROR(provider->GetSubItem(0, -100));
		});
		
		TEST_CASE(L"EmptyItemSourceScenarios")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 0);
		});
		
		TEST_CASE(L"NullItemSourceTransition")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			callbackLog.Clear();
			
			provider->SetItemSource(nullptr);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->Count() == 0);
		});
		
		TEST_CASE(L"DuplicateColumnDetection")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			InitProvider(provider, items);
			
			auto& columns = provider->GetColumns();
			
			auto duplicateColumn = columns[0];
			TEST_EXCEPTION(
				columns.Add(duplicateColumn),
				ArgumentException,
				[](const ArgumentException&) {}
			);
		});
		
		TEST_CASE(L"OutOfBoundsItemAccess")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"Test";
			items.Add(item);
			
			TEST_ERROR(provider->GetText(-1));
			TEST_ERROR(provider->GetText(1));
			TEST_ERROR(provider->GetSubItem(-1, 0));
			TEST_ERROR(provider->GetSubItem(1, 0));
		});
		
		TEST_CASE(L"ColumnOperationsWithInvalidIndices")
		{
			auto provider = Ptr(new ListViewItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			TEST_ASSERT(provider->GetColumnText(-1) == L"");
			TEST_ASSERT(provider->GetColumnText(3) == L"");
			TEST_ASSERT(provider->GetColumnText(100) == L"");
			
			TEST_ERROR(provider->SetColumnSize(-1, 100));
			TEST_ERROR(provider->SetColumnSize(3, 100));
			
			TEST_ERROR(provider->GetColumnSize(-1));
			TEST_ERROR(provider->GetColumnSize(3));
			
			TEST_ERROR(provider->GetDataColumn(-1));
			TEST_ERROR(provider->GetDataColumn(2));
		});
	});
}