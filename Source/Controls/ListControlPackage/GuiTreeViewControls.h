/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS

#include "GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiVirtualTreeListControl NodeProvider
***********************************************************************/

			namespace tree
			{
				class INodeProvider;
				class INodeRootProvider;

				//-----------------------------------------------------------
				// Callback Interfaces
				//-----------------------------------------------------------

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
					virtual void					OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)=0;
					/// <summary>Called after sub items of a node are modified.</summary>
					/// <param name="parentNode">The node containing modified sub items.</param>
					/// <param name="start">The index of the first sub item.</param>
					/// <param name="count">The number of sub items to be modified.</param>
					/// <param name="newCount">The new number of modified sub items.</param>
					virtual void					OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)=0;
					/// <summary>Called when a node is expanded.</summary>
					/// <param name="node">The node.</param>
					virtual void					OnItemExpanded(INodeProvider* node)=0;
					/// <summary>Called when a node is collapsed.</summary>
					/// <param name="node">The node.</param>
					virtual void					OnItemCollapsed(INodeProvider* node)=0;
				};

				//-----------------------------------------------------------
				// Provider Interfaces
				//-----------------------------------------------------------

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

					/// <summary>Get the number of all sub nodes.</summary>
					/// <returns>The number of all sub nodes.</returns>
					virtual vint					GetChildCount()=0;
					/// <summary>Get the parent node. This function increases the reference counter to the result node. If the sub node is not longer needed, a call to [M:vl.presentation.controls.tree.INodeProvider.Release] is required.</summary>
					/// <returns>The parent node.</returns>
					virtual INodeProvider*			GetParent()=0;
					/// <summary>Get the instance of a specified sub node. This function increases the reference counter to the result node. If the sub node is not longer needed, a call to [M:vl.presentation.controls.tree.INodeProvider.Release] is required.</summary>
					/// <returns>The instance of a specified sub node.</returns>
					/// <param name="index">The index of the sub node.</param>
					virtual INodeProvider*			GetChild(vint index)=0;
					/// <summary>Increase the reference counter. Use [M:vl.presentation.controls.tree.INodeProvider.Release] to decrease the reference counter.</summary>
					virtual void					Increase()=0;
					/// <summary>Decrease the reference counter. If the counter is zero, the node will be deleted. Use [M:vl.presentation.controls.tree.INodeProvider.Increase] to increase the reference counter.</summary>
					virtual void					Release()=0;
				};
				
				/// <summary>Represents a root node provider.</summary>
				class INodeRootProvider : public virtual IDescriptable, public Description<INodeRootProvider>
				{
				public:
					/// <summary>Get the instance of the root node.</summary>
					/// <returns>Returns the instance of the root node.</returns>
					virtual INodeProvider*			GetRootNode()=0;
					/// <summary>Test does the provider provided an optimized algorithm to get an instance of a node by the index of all visible nodes. If this function returns true, [M:vl.presentation.controls.tree.INodeRootProvider.GetNodeByVisibleIndex] can be used.</summary>
					/// <returns>Returns true if such an algorithm is provided.</returns>
					virtual bool					CanGetNodeByVisibleIndex()=0;
					/// <summary>Get a node by the index in all visible nodes. This requires [M:vl.presentation.controls.tree.INodeRootProvider.CanGetNodeByVisibleIndex] returning true. If the node is no longer needed, a call to the [M:vl.presentation.controls.tree.INodeProvider.Release] is needed, unless this is a root node so that its parent is null.</summary>
					/// <returns>The node for the index in all visible nodes.</returns>
					/// <param name="index">The index in all visible nodes.</param>
					virtual INodeProvider*			GetNodeByVisibleIndex(vint index)=0;
					/// <summary>Attach an node provider callback to this node provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The node provider callback.</param>
					virtual bool					AttachCallback(INodeProviderCallback* value)=0;
					/// <summary>Detach an node provider callback from this node provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The node provider callback.</param>
					virtual bool					DetachCallback(INodeProviderCallback* value)=0;
					/// <summary>Request a view for this node provider. If the specified view is not supported, it returns null. If you want to get a view of type IXXX, use IXXX::Identifier as the identifier. When the view object is no longer needed, A call to the [M:vl.presentation.controls.tree.INodeRootProvider.ReleaseView] is needed.</summary>
					/// <returns>The view object.</returns>
					/// <param name="identifier">The identifier for the requested view.</param>
					virtual IDescriptable*			RequestView(const WString& identifier)=0;
					/// <summary>Release a requested view.</summary>
					/// <param name="view">The view to release.</param>
					virtual void					ReleaseView(IDescriptable* view)=0;
				};
			}

			namespace tree
			{
				//-----------------------------------------------------------
				// Tree to ListControl (IItemProvider)
				//-----------------------------------------------------------

				/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for [T:vl.presentation.controls.tree.NodeItemStyleProvider]. [T:vl.presentation.controls.tree.NodeItemProvider] provides this view. In most of the cases, the NodeItemProvider class and this view is not required users to create, or even to touch. [T:vl.presentation.controls.GuiVirtualTreeListControl] already handled all of this.</summary>
				class INodeItemView : public virtual GuiListControl::IItemPrimaryTextView, public Description<INodeItemView>
				{
				public:
					/// <summary>The identifier of this view.</summary>
					static const wchar_t* const		Identifier;

					/// <summary>Get an instance of a node by the index in all visible nodes. If the node is no longer needed, a call to [M:vl.presentation.controls.tree.INodeItemView.ReleaseNode] is required.</summary>
					/// <returns>The instance of a node by the index in all visible nodes.</returns>
					/// <param name="index">The index in all visible nodes.</param>
					virtual INodeProvider*			RequestNode(vint index)=0;
					/// <summary>Release an instance of a node.</summary>
					/// <param name="node">The instance of a node.</param>
					virtual void					ReleaseNode(INodeProvider* node)=0;
					/// <summary>Get the index in all visible nodes of a node.</summary>
					/// <returns>The index in all visible nodes of a node.</returns>
					/// <param name="node">The node to calculate the index.</param>
					virtual vint					CalculateNodeVisibilityIndex(INodeProvider* node)=0;
				};

				/// <summary>The required <see cref="INodeRootProvider"/> view for [T:vl.presentation.controls.tree.NodeItemProvider]. This view is always needed to create any customized <see cref="INodeRootProvider"/> implementation.</summary>
				class INodeItemPrimaryTextView : public virtual IDescriptable, public Description<INodeItemPrimaryTextView>
				{
				public:
					/// <summary>The identifier of this view.</summary>
					static const wchar_t* const		Identifier;
					
					/// <summary>Get the primary text of a node.</summary>
					/// <returns>The primary text of a node.</returns>
					/// <param name="node">The node.</param>
					virtual WString					GetPrimaryTextViewText(INodeProvider* node)=0;
				};

				/// <summary>The Binding view for <see cref="INodeRootProvider"/>.</summary>
				class INodeItemBindingView : public virtual IDescriptable, public Description<INodeItemPrimaryTextView>
				{
				public:
					/// <summary>The identifier of this view.</summary>
					static const wchar_t* const		Identifier;
					
					/// <summary>Get the binding value of a node.</summary>
					/// <returns>The binding value of a node.</returns>
					/// <param name="node">The node.</param>
					virtual description::Value		GetBindingValue(INodeProvider* node)=0;
				};

				/// <summary>This is a general implementation to convert an <see cref="INodeRootProvider"/> to a <see cref="GuiListControl::IItemProvider"/>. It requires the <see cref="INodeItemPrimaryTextView"/> to provide a <see cref="GuiListControl::IItemPrimaryTextView"/>.</summary>
				class NodeItemProvider
					: public list::ItemProviderBase
					, protected virtual INodeProviderCallback
					, protected virtual INodeItemView
					, public Description<NodeItemProvider>
				{
				protected:
					Ptr<INodeRootProvider>			root;
					INodeItemPrimaryTextView*		nodeItemPrimaryTextView;
					vint							offsetBeforeChildModified;

					INodeProvider*					GetNodeByOffset(INodeProvider* provider, vint offset);
					void							OnAttached(INodeRootProvider* provider)override;
					void							OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void							OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void							OnItemExpanded(INodeProvider* node)override;
					void							OnItemCollapsed(INodeProvider* node)override;
					vint							CalculateNodeVisibilityIndexInternal(INodeProvider* node);
					vint							CalculateNodeVisibilityIndex(INodeProvider* node)override;
					
					bool							ContainsPrimaryText(vint itemIndex)override;
					WString							GetPrimaryTextViewText(vint itemIndex)override;
					INodeProvider*					RequestNode(vint index)override;
					void							ReleaseNode(INodeProvider* node)override;
				public:
					/// <summary>Create an item provider using a node root provider.</summary>
					/// <param name="_root">The node root provider.</param>
					NodeItemProvider(Ptr<INodeRootProvider> _root);
					~NodeItemProvider();
					
					/// <summary>Get the owned node root provider.</summary>
					/// <returns>The node root provider.</returns>
					Ptr<INodeRootProvider>			GetRoot();
					vint							Count()override;
					IDescriptable*					RequestView(const WString& identifier)override;
					void							ReleaseView(IDescriptable* view)override;
				};

				//-----------------------------------------------------------
				// Tree to ListControl (IItemStyleProvider)
				//-----------------------------------------------------------

				class INodeItemStyleProvider;

				/// <summary>Node item style controller for a [T:vl.presentation.controls.GuiVirtualTreeListControl] to render a node.</summary>
				class INodeItemStyleController : public virtual GuiListControl::IItemStyleController, public Description<INodeItemStyleController>
				{
				public:
					/// <summary>Get the owner node style provider.</summary>
					/// <returns>The owner node style provider.</returns>
					virtual INodeItemStyleProvider*					GetNodeStyleProvider()=0;
				};
				
				/// <summary>Node item style provider for a [T:vl.presentation.controls.GuiVirtualTreeListControl] to render visible nodes.</summary>
				class INodeItemStyleProvider : public virtual IDescriptable, public Description<INodeItemStyleProvider>
				{
				public:
					/// <summary>Bind a item style provider. Generally the item style provider will be used to create a item style controller for a list control, because a tree view control is implemented using a list control.</summary>
					/// <param name="styleProvider">The item style provider.</param>
					virtual void									BindItemStyleProvider(GuiListControl::IItemStyleProvider* styleProvider)=0;
					/// <summary>Get the binded item style provider.</summary>
					/// <returns>The binded item style provider.</returns>
					virtual GuiListControl::IItemStyleProvider*		GetBindedItemStyleProvider()=0;
					/// <summary>Called when a node item style provider in installed to a <see cref="GuiListControl"/>.</summary>
					/// <param name="value">The list control.</param>
					virtual void									AttachListControl(GuiListControl* value)=0;
					/// <summary>Called when a node item style provider in uninstalled from a <see cref="GuiListControl"/>.</summary>
					virtual void									DetachListControl()=0;
					/// <summary>Get a node item style id from a node.</summary>
					/// <returns>The node item style id.</returns>
					/// <param name="node">The node.</param>
					virtual vint									GetItemStyleId(INodeProvider* node)=0;
					/// <summary>Create a node item style controller from a node item style id.</summary>
					/// <returns>The created node item style controller.</returns>
					/// <param name="styleId">The node item style id.</param>
					virtual INodeItemStyleController*				CreateItemStyle(vint styleId)=0;
					/// <summary>Destroy a node item style controller.</summary>
					/// <param name="style">The node item style controller.</param>
					virtual void									DestroyItemStyle(INodeItemStyleController* style)=0;
					/// <summary>Bind a node to a node item style controller.</summary>
					/// <param name="style">The node item style controller.</param>
					/// <param name="node">The node item style id.</param>
					/// <param name="itemIndex">The item index.</param>
					virtual void									Install(INodeItemStyleController* style, INodeProvider* node, vint itemIndex)=0;
					/// <summary>Update the visual affect of a node item style controller to a new item index.</summary>
					/// <param name="style">The node item style controller.</param>
					/// <param name="value">The new item index.</param>
					virtual void									SetStyleIndex(INodeItemStyleController* style, vint value)=0;
					/// <summary>Change the visual affect of a node item style controller to be selected or unselected.</summary>
					/// <param name="style">The node item style controller.</param>
					/// <param name="value">Set to true if the node item is expected to be rendered as selected.</param>
					virtual void									SetStyleSelected(INodeItemStyleController* style, bool value)=0;
				};
				
				/// <summary>This is a general implementation to convert an <see cref="INodeItemStyleProvider"/> to a <see cref="GuiSelectableListControl::IItemStyleProvider"/>.</summary>
				class NodeItemStyleProvider : public Object, public virtual GuiSelectableListControl::IItemStyleProvider, public Description<NodeItemStyleProvider>
				{
				protected:
					Ptr<INodeItemStyleProvider>						nodeItemStyleProvider;
					GuiListControl*									listControl;
					INodeItemView*									nodeItemView;
				public:
					/// <summary>Create an item style provider using a node item style provider.</summary>
					/// <param name="provider">The node item style provider.</param>
					NodeItemStyleProvider(Ptr<INodeItemStyleProvider> provider);
					~NodeItemStyleProvider();

					void											AttachListControl(GuiListControl* value)override;
					void											DetachListControl()override;
					vint											GetItemStyleId(vint itemIndex)override;
					GuiListControl::IItemStyleController*			CreateItemStyle(vint styleId)override;
					void											DestroyItemStyle(GuiListControl::IItemStyleController* style)override;
					void											Install(GuiListControl::IItemStyleController* style, vint itemIndex)override;
					void											SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)override;
					void											SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)override;
				};
			}

/***********************************************************************
GuiVirtualTreeListControl Predefined NodeProvider
***********************************************************************/

			namespace tree
			{
				/// <summary>Base type for tree view node data.</summary>
				class IMemoryNodeData : public virtual IDescriptable, public Description<IMemoryNodeData>
				{
				};

				/// <summary>An in-memory <see cref="INodeProvider"/> implementation.</summary>
				class MemoryNodeProvider
					: public Object
					, public virtual INodeProvider
					, public Description<MemoryNodeProvider>
				{
					typedef collections::List<Ptr<MemoryNodeProvider>> ChildList;
					typedef collections::IEnumerator<Ptr<MemoryNodeProvider>> ChildListEnumerator;

				public:
					class NodeCollection : public list::ItemsBase<Ptr<MemoryNodeProvider>>
					{
						friend class MemoryNodeProvider;
					protected:
						MemoryNodeProvider*			ownerProvider;

						void						OnBeforeChildModified(vint start, vint count, vint newCount);
						void						OnAfterChildModified(vint start, vint count, vint newCount);
						bool						QueryInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
						bool						QueryRemove(vint index, Ptr<MemoryNodeProvider> const& child)override;
						void						BeforeInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
						void						BeforeRemove(vint index, Ptr<MemoryNodeProvider> const& child)override;
						void						AfterInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
						void						AfterRemove(vint index, vint count)override;

						NodeCollection();
					public:
					};

				protected:
					MemoryNodeProvider*				parent;
					bool							expanding;
					vint							childCount;
					vint							totalVisibleNodeCount;
					vint							offsetBeforeChildModified;
					Ptr<IMemoryNodeData>			data;
					NodeCollection					children;

					virtual INodeProviderCallback*	GetCallbackProxyInternal();
					void							OnChildTotalVisibleNodesChanged(vint offset);
				public:
					/// <summary>Create a node provider.</summary>
					MemoryNodeProvider();
					/// <summary>Create a node provider with a data object.</summary>
					/// <param name="_data">The data object.</param>
					MemoryNodeProvider(const Ptr<IMemoryNodeData>& _data);
					~MemoryNodeProvider();

					/// <summary>Get the data object.</summary>
					/// <returns>The data object.</returns>
					Ptr<IMemoryNodeData>			GetData();
					/// <summary>Set the data object.</summary>
					/// <param name="value">The data object.</param>
					void							SetData(const Ptr<IMemoryNodeData>& value);
					/// <summary>Notify that the state in the binded data object is modified.</summary>
					void							NotifyDataModified();
					/// <summary>Get all sub nodes.</summary>
					/// <returns>All sub nodes.</returns>
					NodeCollection&					Children();

					bool							GetExpanding()override;
					void							SetExpanding(bool value)override;
					vint							CalculateTotalVisibleNodes()override;

					vint							GetChildCount()override;
					INodeProvider*					GetParent()override;
					INodeProvider*					GetChild(vint index)override;
					void							Increase()override;
					void							Release()override;
				};

				/// <summary>A general implementation for <see cref="INodeRootProvider"/>.</summary>
				class NodeRootProviderBase : public virtual INodeRootProvider, protected virtual INodeProviderCallback, public Description<NodeRootProviderBase>
				{
					collections::List<INodeProviderCallback*>			callbacks;
				protected:
					void							OnAttached(INodeRootProvider* provider)override;
					void							OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void							OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void							OnItemExpanded(INodeProvider* node)override;
					void							OnItemCollapsed(INodeProvider* node)override;
				public:
					/// <summary>Create a node root provider.</summary>
					NodeRootProviderBase();
					~NodeRootProviderBase();
					
					bool							CanGetNodeByVisibleIndex()override;
					INodeProvider*					GetNodeByVisibleIndex(vint index)override;
					bool							AttachCallback(INodeProviderCallback* value)override;
					bool							DetachCallback(INodeProviderCallback* value)override;
					IDescriptable*					RequestView(const WString& identifier)override;
					void							ReleaseView(IDescriptable* view)override;
				};
				
				/// <summary>An in-memory <see cref="INodeRootProvider"/> implementation.</summary>
				class MemoryNodeRootProvider
					: public MemoryNodeProvider
					, public NodeRootProviderBase
					, public Description<MemoryNodeRootProvider>
				{
				protected:
					INodeProviderCallback*			GetCallbackProxyInternal()override;
				public:
					/// <summary>Create a node root provider.</summary>
					MemoryNodeRootProvider();
					~MemoryNodeRootProvider();

					INodeProvider*					GetRootNode()override;
					/// <summary>Get the <see cref="MemoryNodeProvider"/> object from an <see cref="INodeProvider"/> object.</summary>
					/// <returns>The corresponding <see cref="MemoryNodeProvider"/> object.</returns>
					/// <param name="node">The node to get the memory node.</param>
					MemoryNodeProvider*				GetMemoryNode(INodeProvider* node);
				};
			}

/***********************************************************************
GuiVirtualTreeListControl
***********************************************************************/

			/// <summary>Tree list control in virtual node.</summary>
			class GuiVirtualTreeListControl : public GuiSelectableListControl, protected virtual tree::INodeProviderCallback, public Description<GuiVirtualTreeListControl>
			{
			private:
				void								OnAttached(tree::INodeRootProvider* provider)override;
				void								OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void								OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void								OnItemExpanded(tree::INodeProvider* node)override;
				void								OnItemCollapsed(tree::INodeProvider* node)override;
			protected:
				tree::NodeItemProvider*				nodeItemProvider;
				tree::INodeItemView*				nodeItemView;
				Ptr<tree::INodeItemStyleProvider>	nodeStyleProvider;

				void								OnItemMouseEvent(compositions::GuiNodeMouseEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void								OnItemNotifyEvent(compositions::GuiNodeNotifyEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
			public:
				/// <summary>Create a tree list control in virtual mode.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_nodeRootProvider">The node root provider for this control.</param>
				GuiVirtualTreeListControl(IStyleProvider* _styleProvider, Ptr<tree::INodeRootProvider> _nodeRootProvider);
				~GuiVirtualTreeListControl();

				/// <summary>Node left mouse button down event.</summary>
				compositions::GuiNodeMouseEvent		NodeLeftButtonDown;
				/// <summary>Node left mouse button up event.</summary>
				compositions::GuiNodeMouseEvent		NodeLeftButtonUp;
				/// <summary>Node left mouse button double click event.</summary>
				compositions::GuiNodeMouseEvent		NodeLeftButtonDoubleClick;
				/// <summary>Node middle mouse button down event.</summary>
				compositions::GuiNodeMouseEvent		NodeMiddleButtonDown;
				/// <summary>Node middle mouse button up event.</summary>
				compositions::GuiNodeMouseEvent		NodeMiddleButtonUp;
				/// <summary>Node middle mouse button double click event.</summary>
				compositions::GuiNodeMouseEvent		NodeMiddleButtonDoubleClick;
				/// <summary>Node right mouse button down event.</summary>
				compositions::GuiNodeMouseEvent		NodeRightButtonDown;
				/// <summary>Node right mouse button up event.</summary>
				compositions::GuiNodeMouseEvent		NodeRightButtonUp;
				/// <summary>Node right mouse button double click event.</summary>
				compositions::GuiNodeMouseEvent		NodeRightButtonDoubleClick;
				/// <summary>Node mouse move event.</summary>
				compositions::GuiNodeMouseEvent		NodeMouseMove;
				/// <summary>Node mouse enter event.</summary>
				compositions::GuiNodeNotifyEvent	NodeMouseEnter;
				/// <summary>Node mouse leave event.</summary>
				compositions::GuiNodeNotifyEvent	NodeMouseLeave;
				/// <summary>Node expanded event.</summary>
				compositions::GuiNodeNotifyEvent	NodeExpanded;
				/// <summary>Node collapsed event.</summary>
				compositions::GuiNodeNotifyEvent	NodeCollapsed;

				/// <summary>Get the <see cref="tree::INodeItemView"/> from the item provider.</summary>
				/// <returns>The <see cref="tree::INodeItemView"/> from the item provider.</returns>
				tree::INodeItemView*				GetNodeItemView();
				/// <summary>Get the binded node root provider.</summary>
				/// <returns>The binded node root provider.</returns>
				tree::INodeRootProvider*			GetNodeRootProvider();
				/// <summary>Get the node item style provider.</summary>
				/// <returns>The node item style provider.</returns>
				tree::INodeItemStyleProvider*		GetNodeStyleProvider();
				/// <summary>Set the node item style provider.</summary>
				/// <returns>The old node item style provider.</returns>
				/// <param name="styleProvider">The new node item style provider.</param>
				Ptr<tree::INodeItemStyleProvider>	SetNodeStyleProvider(Ptr<tree::INodeItemStyleProvider> styleProvider);
			};

/***********************************************************************
TreeView
***********************************************************************/

			namespace tree
			{
				/// <summary>The required <see cref="INodeRootProvider"/> view for [T:vl.presentation.controls.tree.TreeViewNodeItemStyleProvider].</summary>
				class ITreeViewItemView : public virtual INodeItemPrimaryTextView, public Description<ITreeViewItemView>
				{
				public:
					/// <summary>The identifier of this view.</summary>
					static const wchar_t* const		Identifier;

					/// <summary>Get the image of a node.</summary>
					/// <returns>Get the image of a node.</returns>
					/// <param name="node">The node.</param>
					virtual Ptr<GuiImageData>		GetNodeImage(INodeProvider* node)=0;
					/// <summary>Get the text of a node.</summary>
					/// <returns>Get the text of a node.</returns>
					/// <param name="node">The node.</param>
					virtual WString					GetNodeText(INodeProvider* node)=0;
				};

				/// <summary>A tree view item. This data structure is used in [T:vl.presentation.controls.tree.TreeViewItemRootProvider].</summary>
				class TreeViewItem : public Object, public virtual IMemoryNodeData, public Description<TreeViewItem>
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

				/// <summary>The default implementation of <see cref="INodeRootProvider"/> for [T:vl.presentation.controls.GuiVirtualTreeView].</summary>
				class TreeViewItemRootProvider
					: public MemoryNodeRootProvider
					, protected virtual ITreeViewItemView
					, protected virtual INodeItemBindingView
					, public Description<TreeViewItemRootProvider>
				{
				protected:

					WString							GetPrimaryTextViewText(INodeProvider* node)override;
					Ptr<GuiImageData>				GetNodeImage(INodeProvider* node)override;
					WString							GetNodeText(INodeProvider* node)override;
					description::Value				GetBindingValue(INodeProvider* node)override;
				public:
					/// <summary>Create a item root provider.</summary>
					TreeViewItemRootProvider();
					~TreeViewItemRootProvider();

					IDescriptable*					RequestView(const WString& identifier)override;
					void							ReleaseView(IDescriptable* view)override;

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
				};
			}
			
			/// <summary>Tree view control in virtual mode.</summary>
			class GuiVirtualTreeView : public GuiVirtualTreeListControl, public Description<GuiVirtualTreeView>
			{
			public:
				/// <summary>Style provider interface for <see cref="GuiVirtualTreeView"/>.</summary>
				class IStyleProvider : public virtual GuiVirtualTreeListControl::IStyleProvider, public Description<IStyleProvider>
				{
				public:
					/// <summary>Create a style controller for an item background. The selection state is used to render the selection state of a node.</summary>
					/// <returns>The created style controller for an item background.</returns>
					virtual GuiSelectableButton::IStyleController*		CreateItemBackground()=0;
					/// <summary>Create a style controller for an item expanding decorator. The selection state is used to render the expanding state of a node</summary>
					/// <returns>The created style controller for an item expanding decorator.</returns>
					virtual GuiSelectableButton::IStyleController*		CreateItemExpandingDecorator()=0;
					/// <summary>Get the text color.</summary>
					/// <returns>The text color.</returns>
					virtual Color										GetTextColor()=0;
				};
			protected:
				IStyleProvider*											styleProvider;
			public:
				/// <summary>Create a tree view control in virtual mode. A [T:vl.presentation.controls.tree.TreeViewNodeItemStyleProvider] is created as a node item style provider by default.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_nodeRootProvider">The node root provider for this control.</param>
				GuiVirtualTreeView(IStyleProvider* _styleProvider, Ptr<tree::INodeRootProvider> _nodeRootProvider);
				~GuiVirtualTreeView();

				/// <summary>Get the style provider for this control.</summary>
				/// <returns>The style provider for this control.</returns>
				IStyleProvider*											GetTreeViewStyleProvider();
			};
			
			/// <summary>Tree view control.</summary>
			class GuiTreeView : public GuiVirtualTreeView, public Description<GuiTreeView>
			{
			protected:
				Ptr<tree::TreeViewItemRootProvider>						nodes;
			public:
				/// <summary>Create a tree view control.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				GuiTreeView(IStyleProvider* _styleProvider);
				~GuiTreeView();

				/// <summary>Get the <see cref="tree::TreeViewItemRootProvider"/> as a node root providerl.</summary>
				/// <returns>The <see cref="tree::TreeViewItemRootProvider"/> as a node root provider.</returns>
				Ptr<tree::TreeViewItemRootProvider>						Nodes();

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				Ptr<tree::TreeViewItem>									GetSelectedItem();
			};

			namespace tree
			{
				/// <summary>The default <see cref="INodeItemStyleProvider"/> implementation for <see cref="GuiVirtualTreeView"/>.</summary>
				class TreeViewNodeItemStyleProvider
					: public Object
					, public virtual INodeItemStyleProvider
					, protected virtual INodeProviderCallback
					, public Description<TreeViewNodeItemStyleProvider>
				{
				protected:
#pragma warning(push)
#pragma warning(disable:4250)
					class ItemController : public list::ItemStyleControllerBase, public virtual INodeItemStyleController
					{
					protected:
						TreeViewNodeItemStyleProvider*		styleProvider;
						GuiSelectableButton*				backgroundButton;
						GuiSelectableButton*				expandingButton;
						compositions::GuiTableComposition*	table;
						elements::GuiImageFrameElement*		image;
						elements::GuiSolidLabelElement*		text;

						void								SwitchNodeExpanding();
						void								OnBackgroundButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
						void								OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
						void								OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					public:
						ItemController(TreeViewNodeItemStyleProvider* _styleProvider, Size minIconSize, bool fitImage);

						INodeItemStyleProvider*				GetNodeStyleProvider()override;
						void								Install(INodeProvider* node);

						bool								GetSelected();
						void								SetSelected(bool value);
						void								UpdateExpandingButton(INodeProvider* associatedNode);
					};
#pragma warning(pop)

					GuiVirtualTreeView*						treeControl;
					GuiListControl::IItemStyleProvider*		bindedItemStyleProvider;
					ITreeViewItemView*						treeViewItemView;
					Size									minIconSize;
					bool									fitImage;

				protected:
					ItemController*							GetRelatedController(INodeProvider* node);
					void									UpdateExpandingButton(INodeProvider* node);
					void									OnAttached(INodeRootProvider* provider)override;
					void									OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void									OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void									OnItemExpanded(INodeProvider* node)override;
					void									OnItemCollapsed(INodeProvider* node)override;
				public:
					/// <summary>Create a node item style provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					TreeViewNodeItemStyleProvider(Size _minIconSize = Size(16, 16), bool _fitImage = true);
					~TreeViewNodeItemStyleProvider();

					void									BindItemStyleProvider(GuiListControl::IItemStyleProvider* styleProvider)override;
					GuiListControl::IItemStyleProvider*		GetBindedItemStyleProvider()override;
					void									AttachListControl(GuiListControl* value)override;
					void									DetachListControl()override;
					vint									GetItemStyleId(INodeProvider* node)override;
					INodeItemStyleController*				CreateItemStyle(vint styleId)override;
					void									DestroyItemStyle(INodeItemStyleController* style)override;
					void									Install(INodeItemStyleController* style, INodeProvider* node, vint itemIndex)override;
					void									SetStyleIndex(INodeItemStyleController* style, vint value)override;
					void									SetStyleSelected(INodeItemStyleController* style, bool value)override;
				};
			}
		}
	}

	namespace collections
	{
		namespace randomaccess_internal
		{
			template<>
			struct RandomAccessable<presentation::controls::tree::MemoryNodeProvider>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};
		}
	}
}

#endif