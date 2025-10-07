/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ITREEVIEWITEMVIEW
#define VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ITREEVIEWITEMVIEW

#include "DataSourceImpl_IItemProvider_NodeItemProvider.h"
#include "DataSourceImpl_INodeProvider_MemoryNodeProvider.h"
#include "../../Resources/GuiResource.h"

namespace vl::presentation::controls::tree
{

/***********************************************************************
ITreeViewItemView
***********************************************************************/

	/// <summary>The required <see cref="INodeRootProvider"/> view for [T:vl.presentation.controls.GuiVirtualTreeView].</summary>
	class ITreeViewItemView : public virtual IDescriptable, public Description<ITreeViewItemView>
	{
	public:
		/// <summary>The identifier of this view.</summary>
		static const wchar_t* const		Identifier;

		/// <summary>Get the image of a node.</summary>
		/// <returns>Get the image of a node.</returns>
		/// <param name="node">The node.</param>
		virtual Ptr<GuiImageData>		GetNodeImage(INodeProvider* node)=0;
	};

/***********************************************************************
TreeViewItem
***********************************************************************/

	/// <summary>A tree view item. This data structure is used in [T:vl.presentation.controls.tree.TreeViewItemRootProvider].</summary>
	class TreeViewItem : public Object, public Description<TreeViewItem>
	{
	public:
		/// <summary>The image of this item.</summary>
		Ptr<GuiImageData>				image;
		/// <summary>The text of this item.</summary>
		WString							text;
		/// <summary>Tag object.</summary>
		description::Value				tag;

		/// <summary>Create a tree view item.</summary>
		TreeViewItem();
		/// <summary>Create a tree view item with specified image and text.</summary>
		/// <param name="_image">The specified image.</param>
		/// <param name="_text">The specified text.</param>
		TreeViewItem(const Ptr<GuiImageData>& _image, const WString& _text);
	};

/***********************************************************************
TreeViewItemRootProvider
***********************************************************************/

	/// <summary>The default implementation of <see cref="INodeRootProvider"/> for [T:vl.presentation.controls.GuiVirtualTreeView].</summary>
	class TreeViewItemRootProvider
		: public MemoryNodeRootProvider
		, public virtual ITreeViewItemView
		, public Description<TreeViewItemRootProvider>
	{
	public:
		/// <summary>Create a item root provider.</summary>
		TreeViewItemRootProvider();
		~TreeViewItemRootProvider();

		/// <summary>Get the <see cref="TreeViewItem"/> object from a node.</summary>
		/// <returns>The <see cref="TreeViewItem"/> object.</returns>
		/// <param name="node">The node to get the tree view item.</param>
		Ptr<TreeViewItem>				GetTreeViewData(INodeProvider* node);
		/// <summary>Set the <see cref="TreeViewItem"/> object to a node.</summary>
		/// <param name="node">The node.</param>
		/// <param name="value">The <see cref="TreeViewItem"/> object.</param>
		void							SetTreeViewData(INodeProvider* node, Ptr<TreeViewItem> value);
		/// <summary>Notify the tree view control that the node is changed. This is required when content in a <see cref="TreeViewItem"/> is modified, but both <see cref="SetTreeViewData"/> or [M:vl.presentation.controls.tree.MemoryNodeProvider.SetData] are not called.</summary>
		/// <param name="node">The node.</param>
		void							UpdateTreeViewData(INodeProvider* node);

		// ===================== list::ITreeViewItemView =====================

		Ptr<GuiImageData>				GetNodeImage(INodeProvider* node)override;
		WString							GetTextValue(INodeProvider* node)override;
		description::Value				GetBindingValue(INodeProvider* node)override;

		// ===================== list::IItemProvider =====================

		IDescriptable*					RequestView(const WString& identifier)override;
	};
}

#endif