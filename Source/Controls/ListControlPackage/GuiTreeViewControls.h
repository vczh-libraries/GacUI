/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS

#include "DataSourceImpl_IItemProvider_NodeItemProvider.h"
#include "DataSourceImpl_INodeProvider_MemoryNodeProvider.h"
#include "GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
/***********************************************************************
GuiVirtualTreeListControl
***********************************************************************/

			/// <summary>Tree list control in virtual node.</summary>
			class GuiVirtualTreeListControl : public GuiSelectableListControl, protected virtual tree::INodeProviderCallback, public Description<GuiVirtualTreeListControl>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TreeViewTemplate, GuiSelectableListControl)
			protected:
				void								OnAttached(tree::INodeRootProvider* provider)override;
				void								OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
				void								OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
				void								OnItemExpanded(tree::INodeProvider* node)override;
				void								OnItemCollapsed(tree::INodeProvider* node)override;

				vint								FindItemByVirtualKeyDirection(vint index, compositions::KeyDirection keyDirection)override;
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
				void													OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
				void													OnItemExpanded(tree::INodeProvider* node)override;
				void													OnItemCollapsed(tree::INodeProvider* node)override;
				void													OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)override;
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
				class DefaultTreeItemTemplate : public list::PredefinedListItemTemplate<templates::GuiTreeItemTemplate>
				{
				protected:
					GuiSelectableButton*					expandingButton = nullptr;
					compositions::GuiTableComposition*		table = nullptr;
					elements::GuiImageFrameElement*			imageElement = nullptr;
					elements::GuiSolidLabelElement*			textElement = nullptr;

					void									OnInitialize()override;
					void									OnRefresh()override;
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