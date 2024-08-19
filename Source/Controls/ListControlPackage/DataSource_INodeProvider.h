/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_DATASOURCE_INODEPROVIDER
#define VCZH_PRESENTATION_CONTROLS_DATASOURCE_INODEPROVIDER

#include "../../GuiTypes.h"

namespace vl::presentation::controls::tree
{
	class INodeProvider;
	class INodeRootProvider;

/***********************************************************************
INodeProviderCallback
***********************************************************************/

	/// <summary>Callback object for <see cref="INodeProvider"/>. A node will invoke this callback to notify any content modification.</summary>
	class INodeProviderCallback : public virtual IDescriptable, public Description<INodeProviderCallback>
	{
	public:
		/// <summary>Called when this callback is attached to a node root.</summary>
		/// <param name="provider">The root node.</param>
		virtual void					OnAttached(INodeRootProvider* provider)=0;
		/// <summary>Called before sub items of a node are modified.</summary>
		/// <param name="parentNode">The node containing modified sub items.</param>
		/// <param name="start">The index of the first sub item.</param>
		/// <param name="count">The number of sub items to be modified.</param>
		/// <param name="newCount">The new number of modified sub items.</param>
		/// <param name="itemReferenceUpdated">True when items are replaced, false when only content in items are updated.</param>
		virtual void					OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)=0;
		/// <summary>Called after sub items of a node are modified.</summary>
		/// <param name="parentNode">The node containing modified sub items.</param>
		/// <param name="start">The index of the first sub item.</param>
		/// <param name="count">The number of sub items to be modified.</param>
		/// <param name="newCount">The new number of modified sub items.</param>
		/// <param name="itemReferenceUpdated">True when items are replaced, false when only content in items are updated.</param>
		virtual void					OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)=0;
		/// <summary>Called when a node is expanded.</summary>
		/// <param name="node">The node.</param>
		virtual void					OnItemExpanded(INodeProvider* node)=0;
		/// <summary>Called when a node is collapsed.</summary>
		/// <param name="node">The node.</param>
		virtual void					OnItemCollapsed(INodeProvider* node)=0;
	};

/***********************************************************************
INodeProvider
***********************************************************************/

	/// <summary>Represents a node.</summary>
	class INodeProvider : public virtual IDescriptable, public Description<INodeProvider>
	{
	public:
		/// <summary>Get the expanding state of this node.</summary>
		/// <returns>Returns true if this node is expanded.</returns>
		virtual bool					GetExpanding()=0;
		/// <summary>Set the expanding state of this node.</summary>
		/// <param name="value">Set to true to expand this node.</param>
		virtual void					SetExpanding(bool value)=0;
		/// <summary>Calculate the number of total visible nodes of this node. The number of total visible nodes includes the node itself, and all total visible nodes of all visible sub nodes. If this node is collapsed, this number will be 1.</summary>
		/// <returns>The number of total visible nodes.</returns>
		virtual vint					CalculateTotalVisibleNodes()=0;
		/// <summary>Notify that the state in the binded data object is modified.</summary>
		virtual void					NotifyDataModified()=0;

		/// <summary>Get the number of all sub nodes.</summary>
		/// <returns>The number of all sub nodes.</returns>
		virtual vint					GetChildCount()=0;
		/// <summary>Get the parent node.</summary>
		/// <returns>The parent node.</returns>
		virtual Ptr<INodeProvider>		GetParent()=0;
		/// <summary>Get the instance of a specified sub node.</summary>
		/// <returns>The instance of a specified sub node.</returns>
		/// <param name="index">The index of the sub node.</param>
		virtual Ptr<INodeProvider>		GetChild(vint index)=0;
	};

/***********************************************************************
INodeRootProvider
***********************************************************************/
				
	/// <summary>Represents a root node provider.</summary>
	class INodeRootProvider : public virtual IDescriptable, public Description<INodeRootProvider>
	{
	public:
		/// <summary>Get the instance of the root node.</summary>
		/// <returns>Returns the instance of the root node.</returns>
		virtual Ptr<INodeProvider>		GetRootNode()=0;
		/// <summary>Test does the provider provided an optimized algorithm to get an instance of a node by the index of all visible nodes. If this function returns true, [M:vl.presentation.controls.tree.INodeRootProvider.GetNodeByVisibleIndex] can be used.</summary>
		/// <returns>Returns true if such an algorithm is provided.</returns>
		virtual bool					CanGetNodeByVisibleIndex()=0;
		/// <summary>Get a node by the index in all visible nodes. This requires [M:vl.presentation.controls.tree.INodeRootProvider.CanGetNodeByVisibleIndex] returning true.</summary>
		/// <returns>The node for the index in all visible nodes.</returns>
		/// <param name="index">The index in all visible nodes.</param>
		virtual Ptr<INodeProvider>		GetNodeByVisibleIndex(vint index)=0;
		/// <summary>Attach an node provider callback to this node provider.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The node provider callback.</param>
		virtual bool					AttachCallback(INodeProviderCallback* value)=0;
		/// <summary>Detach an node provider callback from this node provider.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The node provider callback.</param>
		virtual bool					DetachCallback(INodeProviderCallback* value)=0;
		/// <summary>Get the primary text of a node.</summary>
		/// <returns>The primary text of a node.</returns>
		/// <param name="node">The node.</param>
		virtual WString					GetTextValue(INodeProvider* node) = 0;
		/// <summary>Get the binding value of a node.</summary>
		/// <returns>The binding value of a node.</returns>
		/// <param name="node">The node.</param>
		virtual description::Value		GetBindingValue(INodeProvider* node) = 0;
		/// <summary>Request a view for this node provider. If the specified view is not supported, it returns null. If you want to get a view of type IXXX, use IXXX::Identifier as the identifier.</summary>
		/// <returns>The view object.</returns>
		/// <param name="identifier">The identifier for the requested view.</param>
		virtual IDescriptable*			RequestView(const WString& identifier)=0;
	};
}

#endif