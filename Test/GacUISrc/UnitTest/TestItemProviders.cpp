#include "TestItemProviders.h"

namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	IMPL_CPP_TYPE_INFO(gacui_unittest_template::BindableItem)
#endif
}

namespace gacui_unittest_template
{

/***********************************************************************
MockItemProviderCallback
***********************************************************************/

	MockItemProviderCallback::MockItemProviderCallback(List<WString>& log) : callbackLog(log) {}

	void MockItemProviderCallback::OnAttached(IItemProvider* provider)
	{
		callbackLog.Add(provider ? L"OnAttached(provider=valid)" : L"OnAttached(provider=null)");
	}

	void MockItemProviderCallback::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		callbackLog.Add(L"OnItemModified(start=" + itow(start) + L", count=" + itow(count) + 
					   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
					   (itemReferenceUpdated ? L"true" : L"false") + L")");
	}

/***********************************************************************
MockTextItemProviderCallback
***********************************************************************/

	MockTextItemProviderCallback::MockTextItemProviderCallback(List<WString>& log) : callbackLog(log) {}

	void MockTextItemProviderCallback::OnItemCheckedChanged(vint itemIndex)
	{
		callbackLog.Add(L"OnItemCheckedChanged(itemIndex=" + itow(itemIndex) + L")");
	}

/***********************************************************************
MockColumnItemViewCallback
***********************************************************************/

	MockColumnItemViewCallback::MockColumnItemViewCallback(List<WString>& log) : callbackLog(log) {}

	void MockColumnItemViewCallback::OnColumnRebuilt()
	{
		callbackLog.Add(L"OnColumnRebuilt()");
	}

	void MockColumnItemViewCallback::OnColumnChanged(bool needToRefreshItems)
	{
		callbackLog.Add(L"OnColumnChanged(needToRefreshItems=" + WString::Unmanaged(needToRefreshItems ? L"true" : L"false") + L")");
	}

/***********************************************************************
MockNodeProviderCallback
***********************************************************************/

	MockNodeProviderCallback::MockNodeProviderCallback(List<WString>& log) : callbackLog(log) {}

	void MockNodeProviderCallback::OnAttached(INodeRootProvider* _provider)
	{
		provider = _provider;
		callbackLog.Add(provider ? L"OnAttached(provider=valid)" : L"OnAttached(provider=null)");
	}

	void MockNodeProviderCallback::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		auto parentName = provider->GetRootNode() == parentNode ? L"[ROOT]" : provider->GetTextValue(parentNode);
		callbackLog.Add(parentName + L"->" +
						L"OnBeforeItemModified(start=" + itow(start) + L", count=" + itow(count) + 
						L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
						(itemReferenceUpdated ? L"true" : L"false") + L")");
	}

	void MockNodeProviderCallback::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		auto parentName = provider->GetRootNode() == parentNode ? L"[ROOT]" : provider->GetTextValue(parentNode);
		callbackLog.Add(parentName + L"->" +
						L"OnAfterItemModified(start=" + itow(start) + L", count=" + itow(count) + 
						L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
						(itemReferenceUpdated ? L"true" : L"false") + L")");
	}

	void MockNodeProviderCallback::OnItemExpanded(INodeProvider* node)
	{
		auto nodeName = provider->GetRootNode() == node ? L"[ROOT]" : provider->GetTextValue(node);
		callbackLog.Add(nodeName + L"->OnItemExpanded()");
	}

	void MockNodeProviderCallback::OnItemCollapsed(INodeProvider* node)
	{
		auto nodeName = provider->GetRootNode() == node ? L"[ROOT]" : provider->GetTextValue(node);
		callbackLog.Add(nodeName + L"->OnItemCollapsed()");
	}

/***********************************************************************
BindableItem
***********************************************************************/

	ItemProperty<WString> BindableItem::Prop_name()
	{
		return [](const reflection::description::Value& value) -> WString
		{
			auto item = UnboxValue<Ptr<BindableItem>>(value);
			return item->name;
		};
	}

	ItemProperty<WString> BindableItem::Prop_title()
	{
		return [](const reflection::description::Value& value) -> WString
		{
			auto item = UnboxValue<Ptr<BindableItem>>(value);
			return item->title;
		};
	}

	ItemProperty<WString> BindableItem::Prop_desc()
	{
		return [](const reflection::description::Value& value) -> WString
		{
			auto item = UnboxValue<Ptr<BindableItem>>(value);
			return item->desc;
		};
	}

	WritableItemProperty<bool> BindableItem::Prop_checked()
	{
		return [](const reflection::description::Value& value, bool newValue, bool isWrite) -> bool
		{
			auto item = UnboxValue<Ptr<BindableItem>>(value);
			if (isWrite)
			{
				item->checked = newValue;
				return newValue;
			}
			else
			{
				return item->checked;
			}
		};
	}

	ItemProperty<Ptr<reflection::description::IValueEnumerable>> BindableItem::Prop_children()
	{
		return [](const reflection::description::Value& value) -> Ptr<reflection::description::IValueEnumerable>
		{
			auto item = UnboxValue<Ptr<BindableItem>>(value);
			return UnboxValue<Ptr<IValueObservableList>>(BoxParameter(item->children));
		};
	}

/***********************************************************************
Helper Functions
***********************************************************************/

	Ptr<TextItem> CreateTextItem(const WString& text, bool checked)
	{
		return Ptr(new TextItem(text, checked));
	}

	Ptr<ListViewItem> CreateListViewItem(const WString& text)
	{
		auto item = Ptr(new ListViewItem);
		item->SetText(text);
		return item;
	}

	Ptr<ListViewColumn> CreateListViewColumn(const WString& text, vint size)
	{
		return Ptr(new ListViewColumn(text, size));
	}

	Ptr<MemoryNodeProvider> CreateTreeViewItem(const WString& text)
	{
		// Always pass nullptr for image since creating GuiImageData is impractical in unit tests
		// TreeViewItem and MemoryNodeProvider must work together, so create both at once
		auto item = Ptr(new TreeViewItem(nullptr, text));
		return Ptr(new MemoryNodeProvider(item));
	}

}

namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION

	BEGIN_CLASS_MEMBER(::gacui_unittest_template::BindableItem)
		CLASS_MEMBER_CONSTRUCTOR(::vl::Ptr<::gacui_unittest_template::BindableItem>(), NO_PARAMETER)
		CLASS_MEMBER_FIELD(name)
		CLASS_MEMBER_FIELD(title)
		CLASS_MEMBER_FIELD(desc)
		CLASS_MEMBER_FIELD(checked)
		CLASS_MEMBER_FIELD(children)
	END_CLASS_MEMBER(::gacui_unittest_template::BindableItem)

#endif
}