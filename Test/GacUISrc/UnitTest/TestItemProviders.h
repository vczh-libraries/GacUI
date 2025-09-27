#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITextItemView.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_IListViewItemView.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITreeViewItemView.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::controls::tree;
using namespace vl::reflection::description;

template<typename T>
using ItemProperty = vl::presentation::ItemProperty<T>;

namespace gacui_unittest_template
{
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

	class MockItemProviderCallback : public Object, public virtual IItemProviderCallback
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

	class MockTextItemProviderCallback : public Object, public virtual ITextItemProviderCallback
	{
		List<WString>& callbackLog;
	public:
		MockTextItemProviderCallback(List<WString>& log) : callbackLog(log) {}

		void OnItemCheckedChanged(vint itemIndex) override
		{
			callbackLog.Add(L"OnItemCheckedChanged(itemIndex=" + itow(itemIndex) + L")");
		}
	};

	class MockColumnItemViewCallback : public Object, public virtual IColumnItemViewCallback
	{
		List<WString>& callbackLog;
	public:
		MockColumnItemViewCallback(List<WString>& log) : callbackLog(log) {}

		void OnColumnRebuilt() override
		{
			callbackLog.Add(L"OnColumnRebuilt()");
		}

		void OnColumnChanged(bool needToRefreshItems) override
		{
			callbackLog.Add(L"OnColumnChanged(needToRefreshItems=" + WString::Unmanaged(needToRefreshItems ? L"true" : L"false") + L")");
		}
	};

	class MockNodeProviderCallback : public Object, public virtual INodeProviderCallback
	{
		List<WString>& callbackLog;
	public:
		MockNodeProviderCallback(List<WString>& log) : callbackLog(log) {}

		void OnAttached(INodeRootProvider* provider) override
		{
			callbackLog.Add(provider ? L"OnAttached(provider=valid)" : L"OnAttached(provider=null)");
		}

		void OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated) override
		{
			callbackLog.Add(L"OnBeforeItemModified(start=" + itow(start) + L", count=" + itow(count) + 
						   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
						   (itemReferenceUpdated ? L"true" : L"false") + L")");
		}

		void OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated) override
		{
			callbackLog.Add(L"OnAfterItemModified(start=" + itow(start) + L", count=" + itow(count) + 
						   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
						   (itemReferenceUpdated ? L"true" : L"false") + L")");
		}

		void OnItemExpanded(INodeProvider* node) override
		{
			callbackLog.Add(L"OnItemExpanded()");
		}

		void OnItemCollapsed(INodeProvider* node) override
		{
			callbackLog.Add(L"OnItemCollapsed()");
		}
	};

	class BindableItem : public Object
	{
	public:
		WString											name;
		WString											title;
		WString											desc;
		bool											checked = false;
		ObservableList<Ptr<BindableItem>>				children;

		static ItemProperty<WString> Prop_name()
		{
			return [](const reflection::description::Value& value) -> WString
			{
				auto item = UnboxValue<Ptr<BindableItem>>(value);
				return item->name;
			};
		}

		static ItemProperty<WString> Prop_title()
		{
			return [](const reflection::description::Value& value) -> WString
			{
				auto item = UnboxValue<Ptr<BindableItem>>(value);
				return item->title;
			};
		}

		static ItemProperty<WString> Prop_desc()
		{
			return [](const reflection::description::Value& value) -> WString
			{
				auto item = UnboxValue<Ptr<BindableItem>>(value);
				return item->desc;
			};
		}

		static ItemProperty<bool> Prop_checked()
		{
			return [](const reflection::description::Value& value) -> bool
			{
				auto item = UnboxValue<Ptr<BindableItem>>(value);
				return item->checked;
			};
		}

		static ItemProperty<Ptr<reflection::description::IValueEnumerable>> Prop_children()
		{
			return [](const reflection::description::Value& value) -> Ptr<reflection::description::IValueEnumerable>
			{
				auto item = UnboxValue<Ptr<BindableItem>>(value);
				return UnboxValue<Ptr<IValueObservableList>>(BoxParameter(item->children));
			};
		}
	};
}