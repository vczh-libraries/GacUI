#include "ItemProvider_ITreeViewItemView.h"

namespace vl::presentation::controls::tree
{
	using namespace reflection::description;

	const wchar_t* const ITreeViewItemView::Identifier = L"vl::presentation::controls::tree::ITreeViewItemView";

/***********************************************************************
TreeViewItem
***********************************************************************/

	TreeViewItem::TreeViewItem()
	{
	}

	TreeViewItem::TreeViewItem(const Ptr<GuiImageData>& _image, const WString& _text)
		:image(_image)
		,text(_text)
	{
	}

/***********************************************************************
TreeViewItemRootProvider
***********************************************************************/

	TreeViewItemRootProvider::TreeViewItemRootProvider()
	{
	}

	TreeViewItemRootProvider::~TreeViewItemRootProvider()
	{
	}

	Ptr<TreeViewItem> TreeViewItemRootProvider::GetTreeViewData(INodeProvider* node)
	{
		MemoryNodeProvider* memoryNode=GetMemoryNode(node);
		if(memoryNode)
		{
			return memoryNode->GetData().Cast<TreeViewItem>();
		}
		else
		{
			return 0;
		}
	}

	void TreeViewItemRootProvider::SetTreeViewData(INodeProvider* node, Ptr<TreeViewItem> value)
	{
		MemoryNodeProvider* memoryNode=GetMemoryNode(node);
		if(memoryNode)
		{
			memoryNode->SetData(value);
		}
	}
	
	void TreeViewItemRootProvider::UpdateTreeViewData(INodeProvider* node)
	{
		MemoryNodeProvider* memoryNode=GetMemoryNode(node);
		if(memoryNode)
		{
			memoryNode->NotifyDataModified();
		}
	}

	Ptr<GuiImageData> TreeViewItemRootProvider::GetNodeImage(INodeProvider* node)
	{
		MemoryNodeProvider* memoryNode = dynamic_cast<MemoryNodeProvider*>(node);
		if (memoryNode)
		{
			Ptr<TreeViewItem> data = memoryNode->GetData().Cast<TreeViewItem>();
			if (data)
			{
				return data->image;
			}
		}
		return 0;
	}

	WString TreeViewItemRootProvider::GetTextValue(INodeProvider* node)
	{
		MemoryNodeProvider* memoryNode = dynamic_cast<MemoryNodeProvider*>(node);
		if (memoryNode)
		{
			Ptr<TreeViewItem> data = memoryNode->GetData().Cast<TreeViewItem>();
			if (data)
			{
				return data->text;
			}
		}
		return L"";
	}

	description::Value TreeViewItemRootProvider::GetBindingValue(INodeProvider* node)
	{
		return Value::From(GetTreeViewData(node));
	}

	IDescriptable* TreeViewItemRootProvider::RequestView(const WString& identifier)
	{
		if (identifier == ITreeViewItemView::Identifier)
		{
			return (ITreeViewItemView*)this;
		}
		else
		{
			return MemoryNodeRootProvider::RequestView(identifier);
		}
	}
}