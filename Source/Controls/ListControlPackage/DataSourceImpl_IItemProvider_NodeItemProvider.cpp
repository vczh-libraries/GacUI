#include "DataSourceImpl_IItemProvider_NodeItemProvider.h"

namespace vl::presentation::controls::tree
{
	using namespace reflection::description;

	const wchar_t* const INodeItemView::Identifier = L"vl::presentation::controls::tree::INodeItemView";

/***********************************************************************
NodeItemProvider
***********************************************************************/

	Ptr<INodeProvider> NodeItemProvider::GetNodeByOffset(Ptr<INodeProvider> provider, vint offset)
	{
		if (offset == 0) return provider;
		Ptr<INodeProvider> result;
		if (provider->GetExpanding() && offset > 0)
		{
			offset -= 1;
			vint count = provider->GetChildCount();
			for (vint i = 0; (!result && i < count); i++)
			{
				auto child = provider->GetChild(i);
				vint visibleCount = child->CalculateTotalVisibleNodes();
				if (offset < visibleCount)
				{
					result = GetNodeByOffset(child, offset);
				}
				else
				{
					offset -= visibleCount;
				}
			}
		}
		return result;
	}

	void NodeItemProvider::OnAttached(INodeRootProvider* provider)
	{
	}

	void NodeItemProvider::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		vint offset = 0;
		vint base = CalculateNodeVisibilityIndexInternal(parentNode);
		if (base != -2 && parentNode->GetExpanding())
		{
			for (vint i = 0; i < count; i++)
			{
				auto child = parentNode->GetChild(start + i);
				offset += child->CalculateTotalVisibleNodes();
			}
		}
		offsetBeforeChildModifieds.Set(parentNode, offset);
	}

	void NodeItemProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		vint offsetBeforeChildModified = 0;
		{
			vint index = offsetBeforeChildModifieds.Keys().IndexOf(parentNode);
			if (index != -1)
			{
				offsetBeforeChildModified = offsetBeforeChildModifieds.Values().Get(index);
				offsetBeforeChildModifieds.Remove(parentNode);
			}
		}

		vint base = CalculateNodeVisibilityIndexInternal(parentNode);
		if (base != -2 && parentNode->GetExpanding())
		{
			vint offset = 0;
			vint firstChildStart = -1;
			for (vint i = 0; i < newCount; i++)
			{
				auto child = parentNode->GetChild(start + i);
				if (i == 0)
				{
					firstChildStart = CalculateNodeVisibilityIndexInternal(child.Obj());
				}
				offset += child->CalculateTotalVisibleNodes();
			}

			if (firstChildStart == -1)
			{
				vint childCount = parentNode->GetChildCount();
				if (childCount == 0)
				{
					firstChildStart = base + 1;
				}
				else if (start < childCount)
				{
					auto child = parentNode->GetChild(start);
					firstChildStart = CalculateNodeVisibilityIndexInternal(child.Obj());
				}
				else
				{
					auto child = parentNode->GetChild(start - 1);
					firstChildStart = CalculateNodeVisibilityIndexInternal(child.Obj());
					firstChildStart += child->CalculateTotalVisibleNodes();
				}
			}
			InvokeOnItemModified(firstChildStart, offsetBeforeChildModified, offset, itemReferenceUpdated);
		}
	}

	void NodeItemProvider::OnItemExpanded(INodeProvider* node)
	{
		vint base = CalculateNodeVisibilityIndexInternal(node);
		if (base != -2)
		{
			vint visibility = node->CalculateTotalVisibleNodes();
			InvokeOnItemModified(base + 1, 0, visibility - 1, true);
		}
	}

	void NodeItemProvider::OnItemCollapsed(INodeProvider* node)
	{
		vint base = CalculateNodeVisibilityIndexInternal(node);
		if (base != -2)
		{
			vint visibility = 0;
			vint count = node->GetChildCount();
			for (vint i = 0; i < count; i++)
			{
				auto child = node->GetChild(i);
				visibility += child->CalculateTotalVisibleNodes();
			}
			InvokeOnItemModified(base + 1, visibility, 0, true);
		}
	}

	vint NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider* node)
	{
		auto parent = node->GetParent();
		if (!parent)
		{
			return -1;
		}
		if (!parent->GetExpanding())
		{
			return -2;
		}

		vint index = CalculateNodeVisibilityIndexInternal(parent.Obj());
		if (index == -2)
		{
			return -2;
		}

		vint count = parent->GetChildCount();
		for (vint i = 0; i < count; i++)
		{
			auto child = parent->GetChild(i);
			bool findResult = child == node;
			if (findResult)
			{
				index++;
			}
			else
			{
				index += child->CalculateTotalVisibleNodes();
			}
			if (findResult)
			{
				return index;
			}
		}
		return -1;
	}

	vint NodeItemProvider::CalculateNodeVisibilityIndex(INodeProvider* node)
	{
		vint result = CalculateNodeVisibilityIndexInternal(node);
		return result < 0 ? -1 : result;
	}

	Ptr<INodeProvider> NodeItemProvider::RequestNode(vint index)
	{
		if(root->CanGetNodeByVisibleIndex())
		{
			return root->GetNodeByVisibleIndex(index+1);
		}
		else
		{
			return GetNodeByOffset(root->GetRootNode(), index+1);
		}
	}

	NodeItemProvider::NodeItemProvider(Ptr<INodeRootProvider> _root)
		:root(_root)
	{
		root->AttachCallback(this);
	}

	NodeItemProvider::~NodeItemProvider()
	{
		root->DetachCallback(this);
	}

	Ptr<INodeRootProvider> NodeItemProvider::GetRoot()
	{
		return root;
	}

	vint NodeItemProvider::Count()
	{
		return root->GetRootNode()->CalculateTotalVisibleNodes()-1;
	}

	WString NodeItemProvider::GetTextValue(vint itemIndex)
	{
		if (auto node = RequestNode(itemIndex))
		{
			return root->GetTextValue(node.Obj());
		}
		return L"";
	}

	description::Value NodeItemProvider::GetBindingValue(vint itemIndex)
	{
		if (auto node = RequestNode(itemIndex))
		{
			return root->GetBindingValue(node.Obj());
		}
		return Value();
	}

	IDescriptable* NodeItemProvider::RequestView(const WString& identifier)
	{
		if(identifier==INodeItemView::Identifier)
		{
			return (INodeItemView*)this;
		}
		else
		{
			return root->RequestView(identifier);
		}
	}
}