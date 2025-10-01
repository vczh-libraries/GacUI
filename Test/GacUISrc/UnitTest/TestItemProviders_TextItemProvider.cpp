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
		auto provider = Ptr(new TextItemProvider(&textCallback));
		provider->AttachCallback(&itemCallback);
		
		// Action: Add one TextItem
		auto item = Ptr(new TextItem(L"Test Item", false));
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
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			callbackLog.Clear();  // Clear OnAttached callback
			
			// Test empty TextItem
			auto emptyItem = CreateTextItem(L"", false);
			provider->Add(emptyItem);
			
			// Test TextItem with text only
			auto textItem = CreateTextItem(L"Text Only", false);
			provider->Add(textItem);
			
			// Test TextItem with checked state
			auto checkedItem = CreateTextItem(L"Checked Item", true);
			provider->Add(checkedItem);
			
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
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			// Add items first
			auto item1 = CreateTextItem(L"Item 1", false);
			auto item2 = CreateTextItem(L"Item 2", true);
			provider->Add(item1);
			provider->Add(item2);
			callbackLog.Clear();
			
			// Remove first item
			provider->RemoveAt(0);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"BulkOperations")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			callbackLog.Clear();
			
			// Add multiple items in sequence
			for (vint i = 0; i < 5; i++)
			{
				auto item = CreateTextItem(L"Item " + itow(i), i % 2 == 0);
				provider->Add(item);
			}
			
			// Verify all callbacks fired
			TEST_ASSERT(callbackLog.Count() == 5);
			for (vint i = 0; i < 5; i++)
			{
				WString expected = L"OnItemModified(start=" + itow(i) + L", count=0, newCount=1, itemReferenceUpdated=true)";
				TEST_ASSERT(callbackLog[i] == expected);
			}
			
			// Test RemoveRange - remove 3 items starting from index 1
			callbackLog.Clear();
			provider->RemoveRange(1, 3);
			
			const wchar_t* expectedRemove[] = {
				L"OnItemModified(start=1, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expectedRemove);
		});
	});

	TEST_CATEGORY(L"PropertyChangesAndCallbacks")
	{
		TEST_CASE(L"TextItemSetTextAfterInsertion")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			auto item = CreateTextItem(L"Original", false);
			provider->Add(item);
			callbackLog.Clear();
			
			// Change text - should trigger OnItemModified but NOT OnItemCheckedChanged
			item->SetText(L"Modified");
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(item->GetText() == L"Modified");
		});
		
		TEST_CASE(L"TextItemSetCheckedAfterInsertion")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			auto item = CreateTextItem(L"Test", false);
			provider->Add(item);
			callbackLog.Clear();
			
			// Change checked state - should trigger BOTH callbacks
			item->SetChecked(true);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemCheckedChanged(itemIndex=0)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(item->GetChecked() == true);
		});
		
		TEST_CASE(L"PropertyChangesBeforeInsertion")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			auto item = CreateTextItem(L"Original", false);
			
			// Change properties before insertion - no callbacks should fire
			item->SetText(L"Modified");
			item->SetChecked(true);
			
			TEST_ASSERT(callbackLog.Count() == 1);  // Only OnAttached
			TEST_ASSERT(callbackLog[0] == L"OnAttached(provider=valid)");
			
			// Now add to provider
			callbackLog.Clear();
			provider->Add(item);
			
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});
		
		TEST_CASE(L"CallbackDetachment")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			auto item = CreateTextItem(L"Test", false);
			provider->Add(item);
			callbackLog.Clear();
			
			// First verify both callbacks work when attached
			item->SetChecked(true);
			
			const wchar_t* expectedBoth[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemCheckedChanged(itemIndex=0)"
			};
			AssertCallbacks(callbackLog, expectedBoth);
			
			// Detach callback and test again
			callbackLog.Clear();
			provider->DetachCallback(&itemCallback);
			
			// Change properties - only text callback should fire
			item->SetChecked(false);
			
			const wchar_t* expectedDetachAndText[] = {
				L"OnAttached(provider=null)",
				L"OnItemCheckedChanged(itemIndex=0)"
			};
			AssertCallbacks(callbackLog, expectedDetachAndText);
		});
	});

	TEST_CATEGORY(L"InterfaceAndViewTesting")
	{
		TEST_CASE(L"ITextItemViewGetChecked")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			
			auto item1 = CreateTextItem(L"Unchecked", false);
			auto item2 = CreateTextItem(L"Checked", true);
			provider->Add(item1);
			provider->Add(item2);
			
			// Test GetChecked through ITextItemView interface
			auto textView = dynamic_cast<ITextItemView*>(provider.Obj());
			TEST_ASSERT(textView != nullptr);
			
			TEST_ASSERT(textView->GetChecked(0) == false);
			TEST_ASSERT(textView->GetChecked(1) == true);
		});
		
		TEST_CASE(L"ITextItemViewSetChecked")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			MockItemProviderCallback itemCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			provider->AttachCallback(&itemCallback);
			
			auto item = CreateTextItem(L"Test", false);
			provider->Add(item);
			callbackLog.Clear();
			
			// Test SetChecked through ITextItemView interface
			auto textView = dynamic_cast<ITextItemView*>(provider.Obj());
			TEST_ASSERT(textView != nullptr);
			
			textView->SetChecked(0, true);
			
			// Should trigger the same callbacks as item->SetChecked()
			const wchar_t* expected[] = {
				L"OnItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"OnItemCheckedChanged(itemIndex=0)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(item->GetChecked() == true);
			TEST_ASSERT(textView->GetChecked(0) == true);
		});
		
		TEST_CASE(L"RequestViewInterface")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			
			// Test RequestView with correct identifier
			auto textView = provider->RequestView(ITextItemView::Identifier);
			TEST_ASSERT(textView != nullptr);
			TEST_ASSERT(dynamic_cast<ITextItemView*>(textView) != nullptr);
			
			// Test RequestView with invalid identifier
			auto invalidView = provider->RequestView(L"InvalidInterface");
			TEST_ASSERT(invalidView == nullptr);
		});
	});

	TEST_CATEGORY(L"EdgeCasesAndErrorHandling")
	{
		TEST_CASE(L"InvalidIndexHandling")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			auto item = CreateTextItem(L"Test", false);
			provider->Add(item);
			
			auto textView = dynamic_cast<ITextItemView*>(provider.Obj());
			
			// Test invalid indices - should trigger CHECK_ERROR
			TEST_ERROR(textView->GetChecked(-1));
			TEST_ERROR(textView->GetChecked(1));
			TEST_ERROR(textView->SetChecked(-1, true));
			TEST_ERROR(textView->SetChecked(1, true));
		});
		
		TEST_CASE(L"NullAndEmptyTextHandling")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			
			// Test empty string
			auto emptyItem = CreateTextItem(L"", false);
			provider->Add(emptyItem);
			TEST_ASSERT(emptyItem->GetText() == L"");
			
			// Test setting to empty string
			auto normalItem = CreateTextItem(L"Normal", false);
			provider->Add(normalItem);
			normalItem->SetText(L"");
			TEST_ASSERT(normalItem->GetText() == L"");
		});
		
		TEST_CASE(L"DuplicateItemHandling")
		{
			List<WString> callbackLog;
			MockTextItemProviderCallback textCallback(callbackLog);
			
			auto provider = Ptr(new TextItemProvider(&textCallback));
			
			auto item = CreateTextItem(L"Test", false);
			provider->Add(item);
			callbackLog.Clear();
			
			// Try to add the same item again - should throw an exception
			TEST_EXCEPTION(provider->Add(item), ArgumentException, [](const ArgumentException&) {});
		});
	});
}