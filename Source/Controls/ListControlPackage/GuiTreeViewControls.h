/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITREEVIEWCONTROLS

#include "ItemProvider_ITreeViewItemView.h"
#include "ItemTemplate_ITreeViewItemView.h"

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
		}
	}
}

#endif