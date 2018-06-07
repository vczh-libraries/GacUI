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
NodeItemProvider
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
					/// <summary>Get the parent node.</summary>
					/// <returns>The parent node.</returns>
					virtual Ptr<INodeProvider>		GetParent()=0;
					/// <summary>Get the instance of a specified sub node.</summary>
					/// <returns>The instance of a specified sub node.</returns>
					/// <param name="index">The index of the sub node.</param>
					virtual Ptr<INodeProvider>		GetChild(vint index)=0;
					/// <summary>Increase the reference counter.</summary>
				};
				
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

			namespace tree
			{
				//-----------------------------------------------------------
				// Tree to ListControl (IItemProvider)
				//-----------------------------------------------------------

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
					void							OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
					void							OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)override;
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

/***********************************************************************
MemoryNodeProvider
***********************************************************************/

			namespace tree
			{
				/// <summary>An in-memory <see cref="INodeProvider"/> implementation.</summary>
				class MemoryNodeProvider
					: public Object
					, public virtual INodeProvider
					, public Description<MemoryNodeProvider>
				{
					typedef collections::List<Ptr<MemoryNodeProvider>> ChildList;
					typedef collections::IEnumerator<Ptr<MemoryNodeProvider>> ChildListEnumerator;

				public:
					class NodeCollection : public collections::ObservableListBase<Ptr<MemoryNodeProvider>>
					{
						friend class MemoryNodeProvider;
					protected:
						vint						offsetBeforeChildModified = 0;
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
					MemoryNodeProvider*				parent = nullptr;
					bool							expanding = false;
					vint							childCount = 0;
					vint							totalVisibleNodeCount = 1;
					Ptr<DescriptableObject>			data;
					NodeCollection					children;

					virtual INodeProviderCallback*	GetCallbackProxyInternal();
					void							OnChildTotalVisibleNodesChanged(vint offset);
				public:
					/// <summary>Create a node provider with a data object.</summary>
					/// <param name="_data">The data object.</param>
					MemoryNodeProvider(Ptr<DescriptableObject> _data = nullptr);
					~MemoryNodeProvider();

					/// <summary>Get the data object.</summary>
					/// <returns>The data object.</returns>
					Ptr<DescriptableObject>			GetData();
					/// <summary>Set the data object.</summary>
					/// <param name="value">The data object.</param>
					void							SetData(const Ptr<DescriptableObject>& value);
					/// <summary>Notify that the state in the binded data object is modified.</summary>
					void							NotifyDataModified();
					/// <summary>Get all sub nodes.</summary>
					/// <returns>All sub nodes.</returns>
					NodeCollection&					Children();

					bool							GetExpanding()override;
					void							SetExpanding(bool value)override;
					vint							CalculateTotalVisibleNodes()override;

					vint							GetChildCount()override;
					Ptr<INodeProvider>				GetParent()override;
					Ptr<INodeProvider>				GetChild(vint index)override;
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
					Ptr<INodeProvider>				GetNodeByVisibleIndex(vint index)override;
					bool							AttachCallback(INodeProviderCallback* value)override;
					bool							DetachCallback(INodeProviderCallback* value)override;
					IDescriptable*					RequestView(const WString& identifier)override;
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

					Ptr<INodeProvider>				GetRootNode()override;
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
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TreeViewTemplate, GuiSelectableListControl)
			protected:
				void								OnAttached(tree::INodeRootProvider* provider)override;
				void								OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void								OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void								OnItemExpanded(tree::INodeProvider* node)override;
				void								OnItemCollapsed(tree::INodeProvider* node)override;

			protected:
				tree::NodeItemProvider*				nodeItemProvider;
				tree::INodeItemView*				nodeItemView;

				void								OnItemMouseEvent(compositions::GuiNodeMouseEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void								OnItemNotifyEvent(compositions::GuiNodeNotifyEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void								OnNodeLeftButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiNodeMouseEventArgs& arguments);
			public:
				/// <summary>Create a tree list control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_nodeRootProvider">The node root provider for this control.</param>
				GuiVirtualTreeListControl(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider);
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
			};

/***********************************************************************
TreeViewItemRootProvider
***********************************************************************/

			namespace tree
			{
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

				/// <summary>The default implementation of <see cref="INodeRootProvider"/> for [T:vl.presentation.controls.GuiVirtualTreeView].</summary>
				class TreeViewItemRootProvider
					: public MemoryNodeRootProvider
					, public virtual ITreeViewItemView
					, public Description<TreeViewItemRootProvider>
				{
				protected:

					Ptr<GuiImageData>				GetNodeImage(INodeProvider* node)override;
					WString							GetTextValue(INodeProvider* node)override;
					description::Value				GetBindingValue(INodeProvider* node)override;
				public:
					/// <summary>Create a item root provider.</summary>
					TreeViewItemRootProvider();
					~TreeViewItemRootProvider();

					IDescriptable*					RequestView(const WString& identifier)override;

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

/***********************************************************************
GuiVirtualTreeView
***********************************************************************/
			
			/// <summary>Tree view control in virtual mode.</summary>
			class GuiVirtualTreeView : public GuiVirtualTreeListControl, public Description<GuiVirtualTreeView>
			{
			protected:
				tree::ITreeViewItemView*								treeViewItemView = nullptr;

				templates::GuiTreeItemTemplate*							GetStyleFromNode(tree::INodeProvider* node);
				void													SetStyleExpanding(tree::INodeProvider* node, bool expanding);
				void													SetStyleExpandable(tree::INodeProvider* node, bool expandable);
				void													OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void													OnItemExpanded(tree::INodeProvider* node)override;
				void													OnItemCollapsed(tree::INodeProvider* node)override;
				void													OnStyleInstalled(vint itemIndex, ItemStyle* style)override;
			public:
				/// <summary>Create a tree view control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_nodeRootProvider">The node root provider for this control.</param>
				GuiVirtualTreeView(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider);
				~GuiVirtualTreeView();
			};

/***********************************************************************
GuiTreeView
***********************************************************************/
			
			/// <summary>Tree view control.</summary>
			class GuiTreeView : public GuiVirtualTreeView, public Description<GuiTreeView>
			{
			protected:
				Ptr<tree::TreeViewItemRootProvider>						nodes;
			public:
				/// <summary>Create a tree view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiTreeView(theme::ThemeName themeName);
				~GuiTreeView();

				/// <summary>Get the <see cref="tree::TreeViewItemRootProvider"/> as a node root providerl.</summary>
				/// <returns>The <see cref="tree::TreeViewItemRootProvider"/> as a node root provider.</returns>
				Ptr<tree::TreeViewItemRootProvider>						Nodes();

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				Ptr<tree::TreeViewItem>									GetSelectedItem();
			};

/***********************************************************************
DefaultTreeItemTemplate
***********************************************************************/

			namespace tree
			{
				class DefaultTreeItemTemplate : public templates::GuiTreeItemTemplate
				{
				protected:
					GuiSelectableButton*					expandingButton = nullptr;
					compositions::GuiTableComposition*		table = nullptr;
					elements::GuiImageFrameElement*			imageElement = nullptr;
					elements::GuiSolidLabelElement*			textElement = nullptr;

					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnExpandingChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnExpandableChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnLevelChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void									OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultTreeItemTemplate();
					~DefaultTreeItemTemplate();
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