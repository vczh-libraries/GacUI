#include "TestControls.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITextItemView.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation::controls::list;

template<vint Count>
static void PrintCallbacks(List<WString>& expecteds, const wchar_t* (&actuals)[Count])
{
	TEST_PRINT(L"Expected: (" + itow(expecteds.Count()) + L")");
	for (vint i = 0; i < expecteds.Count(); i++)
	{
		TEST_PRINT(L"  " + expecteds[i]);
	}
	
	TEST_PRINT(L"Actual: (" + itow(Count) + L")");
	for (vint i = 0; i < Count; i++)
	{
		TEST_PRINT(L"  " + WString::Unmanaged(actuals[i]));
	}
}

template<vint Count>
static void AssertCallbacks(List<WString>& callbackLog, const wchar_t* (&expected)[Count])
{
	bool matches = (callbackLog.Count() == Count);
	if (matches)
	{
		for (vint i = 0; i < Count; i++)
		{
			if (callbackLog[i] != WString::Unmanaged(expected[i]))
			{
				matches = false;
				break;
			}
		}
	}
	
	if (!matches)
	{
		PrintCallbacks(callbackLog, expected);
	}
	TEST_ASSERT(matches);
}

TEST_FILE
{

	class MockItemProviderCallback : public IItemProviderCallback  
	{
		List<WString>& callbackLog;
	public:
		MockItemProviderCallback(List<WString>& log) : callbackLog(log) {}
		
		void OnAttached(IItemProvider* provider) override
		{
			callbackLog.Add(provider ? L"OnAttached(provider=valid)" : L"OnAttached(provider=null)");
		}
		
		void OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated) override
		{
			callbackLog.Add(L"OnItemModified(start=" + itow(start) + L", count=" + itow(count) + 
						   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
						   (itemReferenceUpdated ? L"true" : L"false") + L")");
		}
	};

	class MockTextItemProviderCallback : public ITextItemProviderCallback
	{
		List<WString>& callbackLog;
	public:
		MockTextItemProviderCallback(List<WString>& log) : callbackLog(log) {}
		
		void OnItemCheckedChanged(vint itemIndex) override
		{
			callbackLog.Add(L"OnItemCheckedChanged(itemIndex=" + itow(itemIndex) + L")");
		}
	};

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