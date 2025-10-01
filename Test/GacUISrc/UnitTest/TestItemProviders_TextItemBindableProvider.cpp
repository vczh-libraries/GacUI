#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	auto InitProvider = [](Ptr<TextItemBindableProvider> provider, ObservableList<Ptr<BindableItem>>& items, bool bindCheckedProperty = true)
	{
		// Set text property
		provider->textProperty = BindableItem::Prop_name();
		
		// Set checked property if requested
		if (bindCheckedProperty)
		{
			provider->checkedProperty = BindableItem::Prop_checked();
		}

		// Set item source
		provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
	};

	TEST_CASE(L"SimpleItemAddition")
	{
		// Setup: Create callback log and mock objects
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		
		// Create TextItemBindableProvider and attach callbacks
		auto provider = Ptr(new TextItemBindableProvider());

		// Create ObservableList<Ptr<BindableItem>> and set as item source
		ObservableList<Ptr<BindableItem>> items;
		InitProvider(provider, items);
		provider->AttachCallback(&itemCallback);
		
		// Action: Add one BindableItem to the list
		auto item = Ptr(new BindableItem());
		item->name = L"Test Item";
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
		TEST_CASE(L"TextPropertyBinding")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items, false); // Don't bind checked property
			provider->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			item->checked = true;
			items.Add(item);
			
			// Verify text property binding works
			TEST_ASSERT(provider->GetTextValue(0) == L"TestItem");
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"CheckedPropertyBinding")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items, true); // Bind checked property
			provider->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			item->checked = true;
			items.Add(item);
			
			// Verify checked property binding works for reading
			TEST_ASSERT(provider->GetChecked(0) == true);
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"WritablePropertyBindingValidation")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items, true);
			provider->AttachCallback(&itemCallback);
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			item->checked = false;
			items.Add(item);
			
			callbackLog.Clear();
			
			// Test WritableItemProperty functionality through SetChecked
			provider->SetChecked(0, true);
			
			// Verify the change was applied to the underlying item
			TEST_ASSERT(item->checked == true);
			TEST_ASSERT(provider->GetChecked(0) == true);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"ObservableListIntegration")
	{
		TEST_CASE(L"ItemSourceTransitions")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			provider->AttachCallback(&itemCallback);
			
			// Test setting from null to valid list
			ObservableList<Ptr<BindableItem>> items;
			provider->textProperty = BindableItem::Prop_name();
			provider->checkedProperty = BindableItem::Prop_checked();
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
			
			// Test changing between different lists
			ObservableList<Ptr<BindableItem>> newItems;
			auto item = Ptr(new BindableItem());
			item->name = L"NewItem";
			newItems.Add(item);
			
			callbackLog.Clear();
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(newItems)));
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ObservableListChangePropagation")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			provider->AttachCallback(&itemCallback);
			
			callbackLog.Clear();
			
			// Add multiple items
			auto item1 = Ptr(new BindableItem());
			item1->name = L"Item1";
			items.Add(item1);
			
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Item2";
			items.Add(item2);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"OnItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ObservableListRemovalPropagation")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			provider->AttachCallback(&itemCallback);
			
			// Add some items first
			auto item1 = Ptr(new BindableItem());
			item1->name = L"Item1";
			items.Add(item1);
			auto item2 = Ptr(new BindableItem());
			item2->name = L"Item2";
			items.Add(item2);
			
			callbackLog.Clear();
			
			// Remove first item
			items.RemoveAt(0);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"ObservableListRemoveRange")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			provider->AttachCallback(&itemCallback);
			
			// Add 5 items
			for (vint i = 0; i < 5; i++)
			{
				auto item = Ptr(new BindableItem());
				item->name = L"Item " + itow(i);
				items.Add(item);
			}
			
			callbackLog.Clear();
			
			// Test RemoveRange - remove 3 items starting from index 1
			items.RemoveRange(1, 3);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=1, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"InterfaceMethodValidation")
	{
		TEST_CASE(L"ITextItemViewRequestView")
		{
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			// Test RequestView functionality - merge RequestView and dynamic_cast in one line
			auto textItemView = dynamic_cast<ITextItemView*>(provider->RequestView(ITextItemView::Identifier));
			TEST_ASSERT(textItemView);
			
			// Verify it's the same object
			TEST_ASSERT(textItemView == provider.Obj());
		});

		TEST_CASE(L"ITextItemViewInterfaceMethods")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			provider->AttachCallback(&itemCallback);
			
			auto textItemView = dynamic_cast<ITextItemView*>(provider->RequestView(ITextItemView::Identifier));
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			item->checked = false;
			items.Add(item);
			
			callbackLog.Clear();
			
			// Test interface methods
			TEST_ASSERT(textItemView->GetChecked(0) == false);
			
			textItemView->SetChecked(0, true);
			TEST_ASSERT(textItemView->GetChecked(0) == true);
			TEST_ASSERT(item->checked == true);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"InterfaceMethodEquivalence")
		{
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto textItemView = dynamic_cast<ITextItemView*>(provider->RequestView(ITextItemView::Identifier));
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			item->checked = true;
			items.Add(item);
			
			// Verify interface methods produce same results as direct provider methods
			TEST_ASSERT(provider->GetChecked(0) == textItemView->GetChecked(0));
			
			provider->SetChecked(0, false);
			TEST_ASSERT(textItemView->GetChecked(0) == false);
			
			textItemView->SetChecked(0, true);
			TEST_ASSERT(provider->GetChecked(0) == true);
		});
	});

	TEST_CATEGORY(L"EdgeCasesAndErrorHandling")
	{
		TEST_CASE(L"EmptyItemSourceScenarios")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			provider->AttachCallback(&itemCallback);
			
			// Test with null ItemSource
			TEST_ASSERT(provider->Count() == 0);
			
			// Test with empty ObservableList
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			TEST_ASSERT(provider->Count() == 0);
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"InvalidPropertyBindingConfigurations")
		{
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			
			// Test with only ItemSource set (no property bindings)
			provider->SetItemSource(UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(items)));
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			items.Add(item);
			
			// GetTextValue should return empty string when textProperty is not set
			TEST_ASSERT(provider->GetTextValue(0) == L"");
			
			// GetChecked should return false when checkedProperty is not set
			TEST_ASSERT(provider->GetChecked(0) == false);
		});

		TEST_CASE(L"OutOfBoundsOperations")
		{
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			
			auto item = Ptr(new BindableItem());
			item->name = L"TestItem";
			items.Add(item);
			
			// Test out-of-bounds access - should trigger CHECK_ERROR
			TEST_ERROR(provider->GetTextValue(-1));
			TEST_ERROR(provider->GetTextValue(1)); // Beyond available items
			TEST_ERROR(provider->GetChecked(-1));
			TEST_ERROR(provider->GetChecked(1));
			
			// Test interface methods with invalid indices
			auto textItemView = dynamic_cast<ITextItemView*>(provider->RequestView(ITextItemView::Identifier));
			TEST_ERROR(textItemView->GetChecked(-1));
			TEST_ERROR(textItemView->GetChecked(1));
			TEST_ERROR(textItemView->SetChecked(-1, true));
			TEST_ERROR(textItemView->SetChecked(1, true));
		});

		TEST_CASE(L"NullItemInObservableList")
		{
			List<WString> callbackLog;
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemBindableProvider());
			ObservableList<Ptr<BindableItem>> items;
			InitProvider(provider, items);
			provider->AttachCallback(&itemCallback);
			
			// Add null item to test graceful handling
			items.Add(nullptr);
			
			// Provider should handle null items gracefully
			TEST_ASSERT(provider->Count() == 1);
			TEST_ASSERT(provider->GetTextValue(0) == L""); // Should return empty for null item
			TEST_ASSERT(provider->GetChecked(0) == false); // Should return false for null item
			
			const wchar_t* expected[] = {
				L"OnAttached(provider=valid)",
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});
}