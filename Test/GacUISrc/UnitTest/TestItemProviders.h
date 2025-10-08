#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITextItemView.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_IListViewItemView.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_ITreeViewItemView.h"
#include "../../../Source/Controls/ListControlPackage/ItemProvider_Binding.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::controls::tree;
using namespace vl::reflection::description;

template<typename T>
using ItemProperty = vl::presentation::ItemProperty<T>;

template<typename T>
using WritableItemProperty = vl::presentation::WritableItemProperty<T>;

namespace gacui_unittest_template
{
	template<vint Count>
	void PrintCallbacks(List<WString>& callbackLog, const wchar_t* (&expected)[Count])
	{
		TEST_PRINT(L"Actual: (" + itow(callbackLog.Count()) + L")");
		for (vint i = 0; i < callbackLog.Count(); i++)
		{
			TEST_PRINT(L"  " + callbackLog[i]);
		}

		TEST_PRINT(L"Expected: (" + itow(Count) + L")");
		for (vint i = 0; i < Count; i++)
		{
			TEST_PRINT(L"  " + WString::Unmanaged(expected[i]));
		}
	}

	template<vint Count>
	void AssertCallbacks(List<WString>& callbackLog, const wchar_t* (&expected)[Count])
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

	template<vint Count>
	void AssertItems(Ptr<NodeItemProvider> provider, const wchar_t* (&expected)[Count])
	{
		// Build actual items list from the provider
		List<WString> actualItems;
		for (vint i = 0; i < provider->Count(); i++)
		{
			actualItems.Add(provider->GetTextValue(i));
		}

		// Compare with expected items using AssertCallbacks
		AssertCallbacks(actualItems, expected);
	}

	class MockItemProviderCallback : public Object, public virtual IItemProviderCallback
	{
		List<WString>& callbackLog;
	public:
		MockItemProviderCallback(List<WString>& log);

		void OnAttached(IItemProvider* provider) override;
		void OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated) override;
	};

	class MockTextItemProviderCallback : public Object, public virtual ITextItemProviderCallback
	{
		List<WString>& callbackLog;
	public:
		MockTextItemProviderCallback(List<WString>& log);

		void OnItemCheckedChanged(vint itemIndex) override;
	};

	class MockColumnItemViewCallback : public Object, public virtual IColumnItemViewCallback
	{
		List<WString>& callbackLog;
	public:
		MockColumnItemViewCallback(List<WString>& log);

		void OnColumnRebuilt() override;
		void OnColumnChanged(bool needToRefreshItems) override;
	};

	class MockNodeProviderCallback : public Object, public virtual INodeProviderCallback
	{
		INodeRootProvider* provider = nullptr;
		List<WString>& callbackLog;
	public:
		MockNodeProviderCallback(List<WString>& log);

		void OnAttached(INodeRootProvider* _provider) override;
		void OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated) override;
		void OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated) override;
		void OnItemExpanded(INodeProvider* node) override;
		void OnItemCollapsed(INodeProvider* node) override;
	};

	class BindableItem : public Object, public ::vl::reflection::Description<BindableItem>
	{
	public:
		WString											name;
		WString											title;
		WString											desc;
		bool											checked = false;
		ObservableList<Ptr<BindableItem>>				children;

		static ItemProperty<WString> Prop_name();
		static ItemProperty<WString> Prop_title();
		static ItemProperty<WString> Prop_desc();
		static WritableItemProperty<bool> Prop_checked();
		static ItemProperty<Ptr<reflection::description::IValueEnumerable>> Prop_children();
	};

/***********************************************************************
Helper Functions
***********************************************************************/

	Ptr<TextItem> CreateTextItem(const WString& text, bool checked = false);
	Ptr<ListViewItem> CreateListViewItem(const WString& text);
	Ptr<ListViewColumn> CreateListViewColumn(const WString& text, vint size = 100);
	// Helper function to create MemoryNodeProvider with TreeViewItem (null image)
	Ptr<MemoryNodeProvider> CreateTreeViewItem(const WString& text);
}

namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	DECL_TYPE_INFO(gacui_unittest_template::BindableItem)
#endif
}