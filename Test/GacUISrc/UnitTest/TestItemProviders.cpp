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

	void MockNodeProviderCallback::OnAttached(INodeRootProvider* provider)
	{
		callbackLog.Add(provider ? L"OnAttached(provider=valid)" : L"OnAttached(provider=null)");
	}

	void MockNodeProviderCallback::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		callbackLog.Add(L"OnBeforeItemModified(start=" + itow(start) + L", count=" + itow(count) + 
					   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
					   (itemReferenceUpdated ? L"true" : L"false") + L")");
	}

	void MockNodeProviderCallback::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		callbackLog.Add(L"OnAfterItemModified(start=" + itow(start) + L", count=" + itow(count) + 
					   L", newCount=" + itow(newCount) + L", itemReferenceUpdated=" + 
					   (itemReferenceUpdated ? L"true" : L"false") + L")");
	}

	void MockNodeProviderCallback::OnItemExpanded(INodeProvider* node)
	{
		callbackLog.Add(L"OnItemExpanded()");
	}

	void MockNodeProviderCallback::OnItemCollapsed(INodeProvider* node)
	{
		callbackLog.Add(L"OnItemCollapsed()");
	}

/***********************************************************************
BindableItem
***********************************************************************/

	// Static member implementations are kept in header since they're templates

}

namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION

	BEGIN_CLASS_MEMBER(::gacui_unittest_template::BindableItem)
		CLASS_MEMBER_BASE(::vl::reflection::DescriptableObject)
		CLASS_MEMBER_CONSTRUCTOR(::vl::Ptr<::gacui_unittest_template::BindableItem>(), NO_PARAMETER)
	END_CLASS_MEMBER(::gacui_unittest_template::BindableItem)

#endif
}