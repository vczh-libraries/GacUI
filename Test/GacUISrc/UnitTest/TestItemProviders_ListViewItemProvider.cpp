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

	TEST_CATEGORY(L"ItemLifecycle")
	{
		TEST_CASE(L"ItemAdditionVariations")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			auto item1 = CreateListViewItem(L"Item1");
			provider->Add(item1);
			
			auto item2 = CreateListViewItem(L"Item2");
			item2->SetTag(BoxValue<vint>(42));
			provider->Add(item2);
			
			auto item3 = CreateListViewItem(L"Item3");
			item3->GetSubItems().Add(L"Sub1");
			item3->GetSubItems().Add(L"Sub2");
			provider->Add(item3);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"OnItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemRemoval")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			provider->Add(CreateListViewItem(L"Item3"));
			
			callbackLog.Clear();
			provider->RemoveAt(1);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemRemoveRange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			for (vint i = 0; i < 5; i++)
			{
				provider->Add(CreateListViewItem(L"Item" + itow(i)));
			}
			
			callbackLog.Clear();
			provider->RemoveRange(1, 3);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemTextModification")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Original");
			provider->Add(item);
			
			callbackLog.Clear();
			item->SetText(L"Modified");
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemTagModification")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Item");
			item->SetTag(BoxValue<vint>(100));
			provider->Add(item);
			
			callbackLog.Clear();
			item->SetTag(BoxValue<vint>(200));
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"SubitemOperations")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Main");
			provider->Add(item);
			
			callbackLog.Clear();
			
			item->GetSubItems().Add(L"Sub1");
			item->GetSubItems().Add(L"Sub2");
			item->GetSubItems().Add(L"Sub3");
			
			item->GetSubItems().Set(1, L"Modified");
			
			item->GetSubItems().RemoveAt(2);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"MultipleItemPropertiesChanged")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Item");
			provider->Add(item);
			
			callbackLog.Clear();
			item->SetText(L"NewText");
			item->SetTag(BoxValue<vint>(123));
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"ColumnManagement")
	{
		TEST_CASE(L"ColumnAddition")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			
			callbackLog.Clear();
			provider->GetColumns().Add(CreateListViewColumn(L"Col1", 100));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ColumnRemoval")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			
			callbackLog.Clear();
			provider->GetColumns().RemoveAt(0);
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ColumnTextChange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto column = CreateListViewColumn(L"Original");
			provider->GetColumns().Add(column);
			
			callbackLog.Clear();
			column->SetText(L"Modified");
			
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
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto column = CreateListViewColumn(L"Col", 100);
			provider->GetColumns().Add(column);
			provider->Add(CreateListViewItem(L"Item1"));
			
			callbackLog.Clear();
			column->SetSize(200);
			
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ColumnSortingStateChange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto column = CreateListViewColumn(L"Col");
			provider->GetColumns().Add(column);
			
			callbackLog.Clear();
			column->SetSortingState(ColumnSortingState::Ascending);
			
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
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto column = CreateListViewColumn(L"Col");
			provider->GetColumns().Add(column);
			provider->Add(CreateListViewItem(L"Item"));
			
			callbackLog.Clear();
			column->SetTextProperty([](const Value& value)
			{
				return WString::Unmanaged(L"TestProperty");
			});
			
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
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			
			callbackLog.Clear();
			provider->GetDataColumns().Add(0);
			provider->GetDataColumns().Add(2);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"OnItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"MultipleColumnsAdded")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			callbackLog.Clear();
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)",
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)",
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"CallbackCoordination")
	{
		TEST_CASE(L"ItemCallbackDetachment")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			provider->Add(CreateListViewItem(L"Item1"));
			callbackLog.Clear();
			
			static_cast<IItemProvider*>(provider.Obj())->DetachCallback(&itemCallback);
			
			// Detachment triggers OnAttached(provider=null)
			const wchar_t* expectedDetach[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, expectedDetach);
			
			callbackLog.Clear();
			provider->Add(CreateListViewItem(L"Item2"));
			
			// After detachment, no more callbacks should be triggered
			TEST_ASSERT(callbackLog.Count() == 0);
		});

		TEST_CASE(L"ColumnCallbackDetachment")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			callbackLog.Clear();
			
			provider->DetachCallback(&columnCallback);
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"SimultaneousItemAndColumnAddition")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			provider->Add(CreateListViewItem(L"Item3"));
			
			callbackLog.Clear();
			provider->AttachCallback(&columnCallback);
			provider->GetColumns().Add(CreateListViewColumn(L"Col"));
			
			const wchar_t* expectedColumnAdd[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=3, newCount=3, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expectedColumnAdd);
			
			callbackLog.Clear();
			provider->Add(CreateListViewItem(L"Item4"));
			
			const wchar_t* expectedItemAdd[] = {
				L"OnItemModified(start=3, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expectedItemAdd);
		});

		TEST_CASE(L"ColumnChangeTriggersItemRefresh")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto col1 = CreateListViewColumn(L"Col1");
			auto col2 = CreateListViewColumn(L"Col2");
			provider->GetColumns().Add(col1);
			provider->GetColumns().Add(col2);
			
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			provider->Add(CreateListViewItem(L"Item3"));
			
			callbackLog.Clear();
			col1->SetSize(200);
			
			const wchar_t* expected[] = {
				L"OnColumnChanged(needToRefreshItems=true)",
				L"OnItemModified(start=0, count=3, newCount=3, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemChangeDoesNotTriggerColumnCallback")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			provider->GetColumns().Add(CreateListViewColumn(L"Col"));
			auto item = CreateListViewItem(L"Item");
			provider->Add(item);
			
			callbackLog.Clear();
			item->SetText(L"Modified");
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"MultipleCallbacksAttached")
		{
			List<WString> callbackLog1, callbackLog2, callbackLog3, callbackLog4;
			MockItemProviderCallback itemCallback1(callbackLog1);
			MockItemProviderCallback itemCallback2(callbackLog2);
			MockColumnItemViewCallback columnCallback1(callbackLog3);
			MockColumnItemViewCallback columnCallback2(callbackLog4);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback1);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback2);
			provider->AttachCallback(&columnCallback1);
			provider->AttachCallback(&columnCallback2);
			
			provider->Add(CreateListViewItem(L"Item"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col"));
			
			TEST_ASSERT(callbackLog1.Count() > 0);
			TEST_ASSERT(callbackLog2.Count() > 0);
			TEST_ASSERT(callbackLog3.Count() > 0);
			TEST_ASSERT(callbackLog4.Count() > 0);
		});

		TEST_CASE(L"CallbackOrderingWithBothTypes")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			callbackLog.Clear();
			provider->GetColumns().Add(CreateListViewColumn(L"Col"));
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ItemPropertyChangeAfterRemoval")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Item");
			provider->Add(item);
			provider->RemoveAt(0);
			
			callbackLog.Clear();
			item->SetText(L"Modified");
			
			TEST_ASSERT(callbackLog.Count() == 0);
		});
	});

	TEST_CATEGORY(L"InterfaceValidation")
	{
		TEST_CASE(L"RequestViewForListViewItemView")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto view = dynamic_cast<IListViewItemView*>(provider->RequestView(IListViewItemView::Identifier));
			
			TEST_ASSERT(view != nullptr);
			TEST_ASSERT(view == static_cast<IListViewItemView*>(provider.Obj()));
		});

		TEST_CASE(L"RequestViewForColumnItemView")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto view = dynamic_cast<IColumnItemView*>(provider->RequestView(IColumnItemView::Identifier));
			
			TEST_ASSERT(view != nullptr);
			TEST_ASSERT(view == static_cast<IColumnItemView*>(provider.Obj()));
		});

		TEST_CASE(L"RequestViewForUnknownIdentifier")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto view = provider->RequestView(L"UnknownInterface");
			
			TEST_ASSERT(view == nullptr);
		});

		TEST_CASE(L"IListViewItemViewMethods")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto view = static_cast<IListViewItemView*>(provider.Obj());
			
			auto item0 = CreateListViewItem(L"Item0");
			item0->GetSubItems().Add(L"Sub0_0");
			item0->GetSubItems().Add(L"Sub0_1");
			provider->Add(item0);
			
			auto item1 = CreateListViewItem(L"Item1");
			item1->GetSubItems().Add(L"Sub1_0");
			provider->Add(item1);
			
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			
			provider->GetDataColumns().Add(0);
			provider->GetDataColumns().Add(1);
			
			TEST_ASSERT(view->GetText(0) == provider->Get(0)->GetText());
			TEST_ASSERT(view->GetText(1) == provider->Get(1)->GetText());
			
			TEST_ASSERT(view->GetSubItem(0, 0) == provider->Get(0)->GetSubItems()[0]);
			TEST_ASSERT(view->GetSubItem(0, 1) == provider->Get(0)->GetSubItems()[1]);
			TEST_ASSERT(view->GetSubItem(1, 0) == provider->Get(1)->GetSubItems()[0]);
			
			TEST_ASSERT(view->GetDataColumnCount() == provider->GetDataColumns().Count());
			TEST_ASSERT(view->GetDataColumn(0) == provider->GetDataColumns()[0]);
			TEST_ASSERT(view->GetDataColumn(1) == provider->GetDataColumns()[1]);
			
			TEST_ASSERT(view->GetColumnCount() == provider->GetColumns().Count());
			TEST_ASSERT(view->GetColumnText(0) == provider->GetColumns()[0]->GetText());
			TEST_ASSERT(view->GetColumnText(1) == provider->GetColumns()[1]->GetText());
		});

		TEST_CASE(L"IColumnItemViewMethods")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto view = static_cast<IColumnItemView*>(provider.Obj());
			
			auto col1 = CreateListViewColumn(L"Col1", 100);
			auto col2 = CreateListViewColumn(L"Col2", 150);
			provider->GetColumns().Add(col1);
			provider->GetColumns().Add(col2);
			
			col2->SetSortingState(ColumnSortingState::Descending);
			
			TEST_ASSERT(view->GetColumnSize(0) == 100);
			TEST_ASSERT(view->GetColumnSize(1) == 150);
			
			view->SetColumnSize(0, 250);
			TEST_ASSERT(provider->GetColumns()[0]->GetSize() == 250);
			
			TEST_ASSERT(view->GetSortingState(1) == ColumnSortingState::Descending);
			
			List<WString> callbackLog;
			MockColumnItemViewCallback callback(callbackLog);
			
			TEST_ASSERT(view->AttachCallback(&callback) == true);
			TEST_ASSERT(view->AttachCallback(&callback) == false);
			
			TEST_ASSERT(view->DetachCallback(&callback) == true);
			TEST_ASSERT(view->DetachCallback(&callback) == false);
		});

		TEST_CASE(L"GetSubItemMethodEquivalence")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col4"));
			auto view = static_cast<IListViewItemView*>(provider.Obj());
			
			auto item = CreateListViewItem(L"Main");
			item->GetSubItems().Add(L"Sub0");
			item->GetSubItems().Add(L"Sub1");
			item->GetSubItems().Add(L"Sub2");
			provider->Add(item);
			
			for (vint i = 0; i < 3; i++)
			{
				auto interfaceValue = view->GetSubItem(0, i);
				auto directValue = provider->Get(0)->GetSubItems()[i];
				TEST_ASSERT(interfaceValue == directValue);
			}
		});
	});

	TEST_CATEGORY(L"EdgeCases")
	{
		TEST_CASE(L"InvalidItemIndex")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->Add(CreateListViewItem(L"Item1"));
			provider->Add(CreateListViewItem(L"Item2"));
			
			TEST_ERROR(provider->GetText(-1));
			TEST_ERROR(provider->GetText(2));
			TEST_ERROR(provider->GetSubItem(-1, 0));
			TEST_ERROR(provider->GetSubItem(2, 0));
		});

		TEST_CASE(L"InvalidSubitemIndex")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			auto item = CreateListViewItem(L"Item");
			item->GetSubItems().Add(L"Sub0");
			item->GetSubItems().Add(L"Sub1");
			provider->Add(item);
			
			TEST_ERROR(provider->GetSubItem(0, -1));
			TEST_ERROR(provider->GetSubItem(0, 2));
			TEST_ERROR(provider->GetSubItem(0, 999));
		});

		TEST_CASE(L"InvalidColumnIndex")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			
			TEST_ASSERT(provider->GetColumnText(-1) == L"");
			TEST_ASSERT(provider->GetColumnText(2) == L"");
			
			TEST_ASSERT(provider->GetColumnSize(-1) == 0);
			TEST_ASSERT(provider->GetColumnSize(2) == 0);
			
			provider->SetColumnSize(-1, 100);
			provider->SetColumnSize(2, 100);
			
			TEST_ASSERT(provider->GetSortingState(-1) == ColumnSortingState::NotSorted);
			TEST_ASSERT(provider->GetSortingState(2) == ColumnSortingState::NotSorted);
		});

		TEST_CASE(L"InvalidDataColumnIndex")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->GetDataColumns().Add(0);
			provider->GetDataColumns().Add(1);
			
			TEST_ERROR(provider->GetDataColumn(-1));
			TEST_ERROR(provider->GetDataColumn(2));
		});

		TEST_CASE(L"EmptyTextAndNullValues")
		{
			auto provider = Ptr(new ListViewItemProvider);
			
			auto item = Ptr(new ListViewItem);
			provider->Add(item);
			
			TEST_ASSERT(provider->GetText(0) == L"");
			
			auto view = static_cast<IListViewItemView*>(provider.Obj());
			TEST_ASSERT(view->GetText(0) == L"");
		});

		TEST_CASE(L"SubitemsBeyondColumnCount")
		{
			auto provider = Ptr(new ListViewItemProvider);
			provider->GetColumns().Add(CreateListViewColumn(L"Col1"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col2"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col3"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col4"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col5"));
			provider->GetColumns().Add(CreateListViewColumn(L"Col6"));
			
			auto item = CreateListViewItem(L"Main");
			for (vint i = 0; i < 5; i++)
			{
				item->GetSubItems().Add(L"Sub" + itow(i));
			}
			provider->Add(item);
			
			for (vint i = 0; i < 5; i++)
			{
				TEST_ASSERT(provider->GetSubItem(0, i) == L"Sub" + itow(i));
			}
			
			auto view = static_cast<IListViewItemView*>(provider.Obj());
			for (vint i = 0; i < 5; i++)
			{
				TEST_ASSERT(view->GetSubItem(0, i) == L"Sub" + itow(i));
			}
		});

		TEST_CASE(L"AddDuplicateItemReference")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto item = CreateListViewItem(L"Item");
			
			provider->Add(item);
			
			TEST_EXCEPTION(provider->Add(item), ArgumentException, [](const ArgumentException&) {});
		});

		TEST_CASE(L"AddDuplicateColumnReference")
		{
			auto provider = Ptr(new ListViewItemProvider);
			auto column = CreateListViewColumn(L"Col");
			
			provider->GetColumns().Add(column);
			
			TEST_EXCEPTION(provider->GetColumns().Add(column), ArgumentException, [](const ArgumentException&) {});
		});

		TEST_CASE(L"PropertyChangeBeforeAttachment")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			
			auto item = CreateListViewItem(L"Original");
			item->SetText(L"Modified");
			item->SetTag(BoxValue<vint>(100));
			item->GetSubItems().Add(L"Sub1");
			item->GetSubItems().Add(L"Sub2");
			
			callbackLog.Clear();
			provider->Add(item);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ColumnPropertyChangeBeforeAttachment")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			MockColumnItemViewCallback columnCallback(callbackLog);
			
			auto provider = Ptr(new ListViewItemProvider);
			static_cast<IItemProvider*>(provider.Obj())->AttachCallback(&itemCallback);
			provider->AttachCallback(&columnCallback);
			
			auto column = CreateListViewColumn(L"Original", 100);
			column->SetText(L"Modified");
			column->SetSize(200);
			column->SetSortingState(ColumnSortingState::Ascending);
			
			callbackLog.Clear();
			provider->GetColumns().Add(column);
			
			const wchar_t* expected[] = {
				L"OnColumnRebuilt()",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});
}