/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_IITEMPROVIDER_NODEITEMPROVIDER
#define VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_IITEMPROVIDER_NODEITEMPROVIDER

#include "DataSourceImpl_IItemProvider_ItemProviderBase.h"
#include "DataSource_INodeProvider.h"

namespace vl::presentation::controls::tree
{
/***********************************************************************
INodeItemView
***********************************************************************/

	/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for [T:vl.presentation.controls.tree.GuiVirtualTreeView]. [T:vl.presentation.controls.tree.NodeItemProvider] provides this view. In most of the cases, the NodeItemProvider class and this view is not required users to create, or even to touch. [T:vl.presentation.controls.GuiVirtualTreeListControl] already handled all of this.</summary>
	class INodeItemView : public virtual IDescriptable, public Description<INodeItemView>
	{
	public:
		/// <summary>The identifier of this view.</summary>
		static const wchar_t* const		Identifier;

		/// <summary>Get an instance of a node by the index in all visible nodes.</summary>
		/// <returns>The instance of a node by the index in all visible nodes.</returns>
		/// <param name="index">The index in all visible nodes.</param>
		virtual Ptr<INodeProvider>		RequestNode(vint index)=0;
		/// <summary>Get the index in all visible nodes of a node.</summary>
		/// <returns>The index in all visible nodes of a node.</returns>
		/// <param name="node">The node to calculate the index.</param>
		virtual vint					CalculateNodeVisibilityIndex(INodeProvider* node)=0;
	};

/***********************************************************************
NodeItemProvider
***********************************************************************/

	/// <summary>This is a general implementation to convert an <see cref="INodeRootProvider"/> to a <see cref="GuiListControl::IItemProvider"/>.</summary>
	class NodeItemProvider
		: public list::ItemProviderBase
		, protected virtual INodeProviderCallback
		, public virtual INodeItemView
		, public Description<NodeItemProvider>
	{
		typedef collections::Dictionary<INodeProvider*, vint>			NodeIntMap;
	protected:
		Ptr<INodeRootProvider>			root;
		NodeIntMap						offsetBeforeChildModifieds;

		Ptr<INodeProvider>				GetNodeByOffset(Ptr<INodeProvider> provider, vint offset);
		void							OnAttached(INodeRootProvider* provider)override;
		void							OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void							OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void							OnItemExpanded(INodeProvider* node)override;
		void							OnItemCollapsed(INodeProvider* node)override;
		vint							CalculateNodeVisibilityIndexInternal(INodeProvider* node);
		vint							CalculateNodeVisibilityIndex(INodeProvider* node)override;
		
		Ptr<INodeProvider>				RequestNode(vint index)override;
	public:
		/// <summary>Create an item provider using a node root provider.</summary>
		/// <param name="_root">The node root provider.</param>
		NodeItemProvider(Ptr<INodeRootProvider> _root);
		~NodeItemProvider();
		
		/// <summary>Get the owned node root provider.</summary>
		/// <returns>The node root provider.</returns>
		Ptr<INodeRootProvider>			GetRoot();
		vint							Count()override;
		WString							GetTextValue(vint itemIndex)override;
		description::Value				GetBindingValue(vint itemIndex)override;
		IDescriptable*					RequestView(const WString& identifier)override;
	};
}

#endif