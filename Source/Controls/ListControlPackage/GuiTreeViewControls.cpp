#include "GuiTreeViewControls.h"
#include "GuiListControlItemArrangers.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace reflection::description;

/***********************************************************************
GuiVirtualTreeListControl
***********************************************************************/

			void GuiVirtualTreeListControl::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiVirtualTreeListControl::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			void GuiVirtualTreeListControl::OnAttached(tree::INodeRootProvider* provider)
			{
			}

			void GuiVirtualTreeListControl::OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
			}

			void GuiVirtualTreeListControl::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
			}

			void GuiVirtualTreeListControl::OnItemExpanded(tree::INodeProvider* node)
			{
				GuiNodeEventArgs arguments;
				(GuiEventArgs&)arguments=GetNotifyEventArguments();
				arguments.node=node;
				NodeExpanded.Execute(arguments);
			}

			void GuiVirtualTreeListControl::OnItemCollapsed(tree::INodeProvider* node)
			{
				GuiNodeEventArgs arguments;
				(GuiEventArgs&)arguments=GetNotifyEventArguments();
				arguments.node=node;
				NodeCollapsed.Execute(arguments);
			}

			vint GuiVirtualTreeListControl::FindItemByVirtualKeyDirection(vint index, compositions::KeyDirection keyDirection)
			{
				vint newIndex = GuiSelectableListControl::FindItemByVirtualKeyDirection(index, keyDirection);
				if (newIndex != -1) return newIndex;

				auto selectedNode = nodeItemView->RequestNode(index);
				if (selectedNode)
				{
					bool hasChildren = selectedNode->GetChildCount() > 0;
					bool expanding = selectedNode->GetExpanding();
					switch (keyDirection)
					{
					case KeyDirection::Right:
						if (hasChildren)
						{
							if (expanding)
							{
								selectedNode = selectedNode->GetChild(0);
							}
							else
							{
								selectedNode->SetExpanding(true);
							}
						}
						break;
					case KeyDirection::Left:
						{
							selectedNode->SetExpanding(false);
							if (!expanding || !hasChildren)
							{
								selectedNode = selectedNode->GetParent();
							}
						}
						break;
					default:;
					}
				}

				return selectedNode ? nodeItemView->CalculateNodeVisibilityIndex(selectedNode.Obj()) : -1;
			}

			void GuiVirtualTreeListControl::OnItemMouseEvent(compositions::GuiNodeMouseEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				auto node = GetNodeItemView()->RequestNode(arguments.itemIndex);
				if (node)
				{
					GuiNodeMouseEventArgs redirectArguments;
					(GuiMouseEventArgs&)redirectArguments = arguments;
					redirectArguments.node = node.Obj();
					nodeEvent.Execute(redirectArguments);
					(GuiMouseEventArgs&)arguments = redirectArguments;
				}
			}

			void GuiVirtualTreeListControl::OnItemNotifyEvent(compositions::GuiNodeNotifyEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				if (auto node = GetNodeItemView()->RequestNode(arguments.itemIndex))
				{
					GuiNodeEventArgs redirectArguments;
					(GuiEventArgs&)redirectArguments = arguments;
					redirectArguments.node = node.Obj();
					nodeEvent.Execute(redirectArguments);
					(GuiEventArgs&)arguments = redirectArguments;
				}
			}

#define ATTACH_ITEM_MOUSE_EVENT(NODEEVENTNAME, ITEMEVENTNAME)\
					{\
						ITEMEVENTNAME.AttachFunction([this](GuiGraphicsComposition* sender, GuiItemMouseEventArgs& args){ OnItemMouseEvent(NODEEVENTNAME, sender, args); });\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(NODEEVENTNAME, ITEMEVENTNAME)\
					{\
						ITEMEVENTNAME.AttachFunction([this](GuiGraphicsComposition* sender, GuiItemEventArgs& args){ OnItemNotifyEvent(NODEEVENTNAME, sender, args); });\
					}\

			void GuiVirtualTreeListControl::OnNodeLeftButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiNodeMouseEventArgs& arguments)
			{
				if (arguments.node->GetChildCount() > 0)
				{
					arguments.node->SetExpanding(!arguments.node->GetExpanding());
				}
			}

			GuiVirtualTreeListControl::GuiVirtualTreeListControl(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiSelectableListControl(themeName, new tree::NodeItemProvider(_nodeRootProvider))
			{
				nodeItemProvider = dynamic_cast<tree::NodeItemProvider*>(GetItemProvider());
				nodeItemView = dynamic_cast<tree::INodeItemView*>(GetItemProvider()->RequestView(WString::Unmanaged(tree::INodeItemView::Identifier)));

				NodeLeftButtonDown.SetAssociatedComposition(boundsComposition);
				NodeLeftButtonUp.SetAssociatedComposition(boundsComposition);
				NodeLeftButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonDown.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonUp.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeRightButtonDown.SetAssociatedComposition(boundsComposition);
				NodeRightButtonUp.SetAssociatedComposition(boundsComposition);
				NodeRightButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeMouseMove.SetAssociatedComposition(boundsComposition);
				NodeMouseEnter.SetAssociatedComposition(boundsComposition);
				NodeMouseLeave.SetAssociatedComposition(boundsComposition);
				NodeExpanded.SetAssociatedComposition(boundsComposition);
				NodeCollapsed.SetAssociatedComposition(boundsComposition);

				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonDown, ItemLeftButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonUp, ItemLeftButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonDoubleClick, ItemLeftButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonDown, ItemMiddleButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonUp, ItemMiddleButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonDoubleClick, ItemMiddleButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonDown, ItemRightButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonUp, ItemRightButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonDoubleClick, ItemRightButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeMouseMove, ItemMouseMove);
				ATTACH_ITEM_NOTIFY_EVENT(NodeMouseEnter, ItemMouseEnter);
				ATTACH_ITEM_NOTIFY_EVENT(NodeMouseLeave, ItemMouseLeave);

				nodeItemProvider->GetRoot()->AttachCallback(this);
				NodeLeftButtonDoubleClick.AttachMethod(this, &GuiVirtualTreeListControl::OnNodeLeftButtonDoubleClick);
			}

#undef ATTACH_ITEM_MOUSE_EVENT
#undef ATTACH_ITEM_NOTIFY_EVENT

			GuiVirtualTreeListControl::~GuiVirtualTreeListControl()
			{
			}

			tree::INodeItemView* GuiVirtualTreeListControl::GetNodeItemView()
			{
				return nodeItemView;
			}

			tree::INodeRootProvider* GuiVirtualTreeListControl::GetNodeRootProvider()
			{
				return nodeItemProvider->GetRoot().Obj();
			}

/***********************************************************************
GuiVirtualTreeView
***********************************************************************/

			templates::GuiTreeItemTemplate* GuiVirtualTreeView::GetStyleFromNode(tree::INodeProvider* node)
			{
				if (itemArranger)
				{
					vint index = nodeItemView->CalculateNodeVisibilityIndex(node);
					if (index != -1)
					{
						auto style = itemArranger->GetVisibleStyle(index);
						return dynamic_cast<templates::GuiTreeItemTemplate*>(style);
					}
				}
				return nullptr;
			}

			void GuiVirtualTreeView::SetStyleExpanding(tree::INodeProvider* node, bool expanding)
			{
				if (auto treeItemStyle = GetStyleFromNode(node))
				{
					treeItemStyle->SetExpanding(expanding);
				}
			}

			void GuiVirtualTreeView::SetStyleExpandable(tree::INodeProvider* node, bool expandable)
			{
				if (auto treeItemStyle = GetStyleFromNode(node))
				{
					treeItemStyle->SetExpandable(expandable);
				}
			}

			void GuiVirtualTreeView::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				GuiVirtualTreeListControl::OnAfterItemModified(parentNode, start, count, newCount, itemReferenceUpdated);
				SetStyleExpandable(parentNode, parentNode->GetChildCount() > 0);
			}

			void GuiVirtualTreeView::OnItemExpanded(tree::INodeProvider* node)
			{
				GuiVirtualTreeListControl::OnItemExpanded(node);
				SetStyleExpanding(node, true);
			}

			void GuiVirtualTreeView::OnItemCollapsed(tree::INodeProvider* node)
			{
				GuiVirtualTreeListControl::OnItemCollapsed(node);
				SetStyleExpanding(node, false);
			}
			
			void GuiVirtualTreeView::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				GuiVirtualTreeListControl::OnStyleInstalled(itemIndex, style, refreshPropertiesOnly);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetTextColor());
				}
				if (auto treeItemStyle = dynamic_cast<templates::GuiTreeItemTemplate*>(style))
				{
					treeItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetTextColor());

					if (treeViewItemView)
					{
						if (auto node = nodeItemView->RequestNode(itemIndex))
						{
							treeItemStyle->SetImage(treeViewItemView->GetNodeImage(node.Obj()));
							treeItemStyle->SetExpanding(node->GetExpanding());
							treeItemStyle->SetExpandable(node->GetChildCount() > 0);
							{
								vint level = -1;
								auto current = node;
								while (current->GetParent())
								{
									level++;
									current = current->GetParent();
								}
								treeItemStyle->SetLevel(level);
							}
						}
					}
				}
				if (refreshPropertiesOnly)
				{
					if (auto predefinedItemStyle = dynamic_cast<tree::DefaultTreeItemTemplate*>(style))
					{
						predefinedItemStyle->RefreshItem();
					}
				}
			}

			GuiVirtualTreeView::GuiVirtualTreeView(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiVirtualTreeListControl(themeName, _nodeRootProvider)
			{
				treeViewItemView = dynamic_cast<tree::ITreeViewItemView*>(GetNodeRootProvider()->RequestView(WString::Unmanaged(tree::ITreeViewItemView::Identifier)));
				SetStyleAndArranger(
					[](const Value&) { return new tree::DefaultTreeItemTemplate; },
					Ptr(new list::FixedHeightItemArranger)
				);
			}

			GuiVirtualTreeView::~GuiVirtualTreeView()
			{
			}

/***********************************************************************
GuiTreeView
***********************************************************************/

			GuiTreeView::GuiTreeView(theme::ThemeName themeName)
				:GuiVirtualTreeView(themeName, Ptr(new tree::TreeViewItemRootProvider))
			{
				nodes = nodeItemProvider->GetRoot().Cast<tree::TreeViewItemRootProvider>();
			}

			GuiTreeView::~GuiTreeView()
			{
			}

			Ptr<tree::TreeViewItemRootProvider> GuiTreeView::Nodes()
			{
				return nodes;
			}

			Ptr<tree::TreeViewItem> GuiTreeView::GetSelectedItem()
			{
				Ptr<tree::TreeViewItem> result;
				vint index = GetSelectedItemIndex();
				if (index != -1)
				{
					if (auto node = nodeItemView->RequestNode(index))
					{
						if (auto memoryNode = node.Cast<tree::MemoryNodeProvider>())
						{
							result = memoryNode->GetData().Cast<tree::TreeViewItem>();
						}
					}
				}
				return result;
			}
		}
	}
}