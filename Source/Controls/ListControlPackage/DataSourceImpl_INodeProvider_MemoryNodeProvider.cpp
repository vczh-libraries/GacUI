#include "DataSourceImpl_INodeProvider_MemoryNodeProvider.h"

namespace vl::presentation::controls::tree
{
/***********************************************************************
MemoryNodeProvider::NodeCollection
***********************************************************************/

	void MemoryNodeProvider::NodeCollection::OnBeforeChildModified(vint start, vint count, vint newCount)
	{
		if (ownerProvider->expanding)
		{
			for (vint i = 0; i < count; i++)
			{
				offsetBeforeChildModified += items[start + i]->totalVisibleNodeCount;
			}
		}
		INodeProviderCallback* proxy = ownerProvider->GetCallbackProxyInternal();
		if (proxy)
		{
			proxy->OnBeforeItemModified(ownerProvider, start, count, newCount, true);
		}
	}

	void MemoryNodeProvider::NodeCollection::OnAfterChildModified(vint start, vint count, vint newCount)
	{
		ownerProvider->childCount += (newCount - count);
		if (ownerProvider->expanding)
		{
			vint offset = 0;
			for (vint i = 0; i < newCount; i++)
			{
				offset += items[start + i]->totalVisibleNodeCount;
			}
			ownerProvider->OnChildTotalVisibleNodesChanged(offset - offsetBeforeChildModified);
		}
		offsetBeforeChildModified = 0;
		INodeProviderCallback* proxy = ownerProvider->GetCallbackProxyInternal();
		if (proxy)
		{
			proxy->OnAfterItemModified(ownerProvider, start, count, newCount, true);
		}
	}

	bool MemoryNodeProvider::NodeCollection::QueryInsert(vint index, Ptr<MemoryNodeProvider> const& child)
	{
		return child->parent == 0;
	}

	bool MemoryNodeProvider::NodeCollection::QueryRemove(vint index, Ptr<MemoryNodeProvider> const& child)
	{
		return child->parent == ownerProvider;
	}

	void MemoryNodeProvider::NodeCollection::BeforeInsert(vint index, Ptr<MemoryNodeProvider> const& child)
	{
		OnBeforeChildModified(index, 0, 1);
		child->parent = ownerProvider;
	}

	void MemoryNodeProvider::NodeCollection::BeforeRemove(vint index, Ptr<MemoryNodeProvider> const& child)
	{
		OnBeforeChildModified(index, 1, 0);
		child->parent = 0;
	}

	void MemoryNodeProvider::NodeCollection::AfterInsert(vint index, Ptr<MemoryNodeProvider> const& child)
	{
		OnAfterChildModified(index, 0, 1);
	}

	void MemoryNodeProvider::NodeCollection::AfterRemove(vint index, vint count)
	{
		OnAfterChildModified(index, count, 0);
	}

	MemoryNodeProvider::NodeCollection::NodeCollection()
		:ownerProvider(0)
	{
	}

/***********************************************************************
MemoryNodeProvider
***********************************************************************/

	INodeProviderCallback* MemoryNodeProvider::GetCallbackProxyInternal()
	{
		if(parent)
		{
			return parent->GetCallbackProxyInternal();
		}
		else
		{
			return 0;
		}
	}

	void MemoryNodeProvider::OnChildTotalVisibleNodesChanged(vint offset)
	{
		totalVisibleNodeCount+=offset;
		if(parent)
		{
			parent->OnChildTotalVisibleNodesChanged(offset);
		}
	}

	MemoryNodeProvider::MemoryNodeProvider(Ptr<DescriptableObject> _data)
		:data(_data)
	{
		children.ownerProvider=this;
	}

	MemoryNodeProvider::~MemoryNodeProvider()
	{
	}

	Ptr<DescriptableObject> MemoryNodeProvider::GetData()
	{
		return data;
	}

	void MemoryNodeProvider::SetData(const Ptr<DescriptableObject>& value)
	{
		data=value;
		NotifyDataModified();
	}

	MemoryNodeProvider::NodeCollection& MemoryNodeProvider::Children()
	{
		return children;
	}

	bool MemoryNodeProvider::GetExpanding()
	{
		return expanding;
	}

	void MemoryNodeProvider::SetExpanding(bool value)
	{
		if(expanding!=value)
		{
			expanding=value;
			vint offset=0;
			for(vint i=0;i<childCount;i++)
			{
				offset+=children[i]->totalVisibleNodeCount;
			}

			OnChildTotalVisibleNodesChanged(expanding?offset:-offset);
			INodeProviderCallback* proxy=GetCallbackProxyInternal();
			if(proxy)
			{
				if(expanding)
				{
					proxy->OnItemExpanded(this);
				}
				else
				{
					proxy->OnItemCollapsed(this);
				}
			}
		}
	}

	vint MemoryNodeProvider::CalculateTotalVisibleNodes()
	{
		return totalVisibleNodeCount;
	}

	void MemoryNodeProvider::NotifyDataModified()
	{
		if (parent)
		{
			vint index = parent->children.IndexOf(this);
			INodeProviderCallback* proxy = GetCallbackProxyInternal();
			if (proxy)
			{
				proxy->OnBeforeItemModified(parent, index, 1, 1, false);
				proxy->OnAfterItemModified(parent, index, 1, 1, false);
			}
		}
	}

	vint MemoryNodeProvider::GetChildCount()
	{
		return childCount;
	}

	Ptr<INodeProvider> MemoryNodeProvider::GetParent()
	{
		return Ptr(parent);
	}

	Ptr<INodeProvider> MemoryNodeProvider::GetChild(vint index)
	{
		if (0 <= index && index < childCount)
		{
			return children[index];
		}
		else
		{
			return nullptr;
		}
	}

/***********************************************************************
NodeRootProviderBase
***********************************************************************/

	void NodeRootProviderBase::OnAttached(INodeRootProvider* provider)
	{
	}

	void NodeRootProviderBase::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		// TODO: (enumerable) foreach
		for(vint i=0;i<callbacks.Count();i++)
		{
			callbacks[i]->OnBeforeItemModified(parentNode, start, count, newCount, itemReferenceUpdated);
		}
	}

	void NodeRootProviderBase::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		// TODO: (enumerable) foreach
		for(vint i=0;i<callbacks.Count();i++)
		{
			callbacks[i]->OnAfterItemModified(parentNode, start, count, newCount, itemReferenceUpdated);
		}
	}

	void NodeRootProviderBase::OnItemExpanded(INodeProvider* node)
	{
		// TODO: (enumerable) foreach
		for(vint i=0;i<callbacks.Count();i++)
		{
			callbacks[i]->OnItemExpanded(node);
		}
	}

	void NodeRootProviderBase::OnItemCollapsed(INodeProvider* node)
	{
		// TODO: (enumerable) foreach
		for(vint i=0;i<callbacks.Count();i++)
		{
			callbacks[i]->OnItemCollapsed(node);
		}
	}

	NodeRootProviderBase::NodeRootProviderBase()
	{
	}

	NodeRootProviderBase::~NodeRootProviderBase()
	{
	}

	bool NodeRootProviderBase::CanGetNodeByVisibleIndex()
	{
		return false;
	}

	Ptr<INodeProvider> NodeRootProviderBase::GetNodeByVisibleIndex(vint index)
	{
		return nullptr;
	}

	bool NodeRootProviderBase::AttachCallback(INodeProviderCallback* value)
	{
		if(callbacks.Contains(value))
		{
			return false;
		}
		else
		{
			callbacks.Add(value);
			value->OnAttached(this);
			return true;
		}
	}

	bool NodeRootProviderBase::DetachCallback(INodeProviderCallback* value)
	{
		vint index=callbacks.IndexOf(value);
		if(index==-1)
		{
			return false;
		}
		else
		{
			value->OnAttached(0);
			callbacks.Remove(value);
			return true;
		}
	}

	IDescriptable* NodeRootProviderBase::RequestView(const WString& identifier)
	{
		return 0;
	}

/***********************************************************************
MemoryNodeRootProvider
***********************************************************************/

	INodeProviderCallback* MemoryNodeRootProvider::GetCallbackProxyInternal()
	{
		return this;
	}

	MemoryNodeRootProvider::MemoryNodeRootProvider()
	{
		SetExpanding(true);
	}

	MemoryNodeRootProvider::~MemoryNodeRootProvider()
	{
	}

	Ptr<INodeProvider> MemoryNodeRootProvider::GetRootNode()
	{
		return Ptr(this);
	}

	MemoryNodeProvider* MemoryNodeRootProvider::GetMemoryNode(INodeProvider* node)
	{
		return dynamic_cast<MemoryNodeProvider*>(node);
	}
}