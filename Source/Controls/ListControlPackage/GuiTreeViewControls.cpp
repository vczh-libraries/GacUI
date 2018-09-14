#include "GuiTreeViewControls.h"
#include "GuiListControlItemArrangers.h"
#include "../GuiButtonControls.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace reflection::description;

			namespace tree
			{
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

				void NodeItemProvider::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
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

				void NodeItemProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
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
						InvokeOnItemModified(firstChildStart, offsetBeforeChildModified, offset);
					}
				}

				void NodeItemProvider::OnItemExpanded(INodeProvider* node)
				{
					vint base = CalculateNodeVisibilityIndexInternal(node);
					if (base != -2)
					{
						vint visibility = node->CalculateTotalVisibleNodes();
						InvokeOnItemModified(base + 1, 0, visibility - 1);
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
						InvokeOnItemModified(base + 1, visibility, 0);
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

/***********************************************************************
MemoryNodeProvider::NodeCollection
***********************************************************************/

				void MemoryNodeProvider::NodeCollection::OnBeforeChildModified(vint start, vint count, vint newCount)
				{
					if (ownerProvider->expanding)
					{
						for (vint i = 0; i < count; i++)
						{
							offsetBeforeChildModified += items[start + i]->totalVisibleNodeCount;
						}
					}
					INodeProviderCallback* proxy = ownerProvider->GetCallbackProxyInternal();
					if (proxy)
					{
						proxy->OnBeforeItemModified(ownerProvider, start, count, newCount);
					}
				}

				void MemoryNodeProvider::NodeCollection::OnAfterChildModified(vint start, vint count, vint newCount)
				{
					ownerProvider->childCount += (newCount - count);
					if (ownerProvider->expanding)
					{
						vint offset = 0;
						for (vint i = 0; i < newCount; i++)
						{
							offset += items[start + i]->totalVisibleNodeCount;
						}
						ownerProvider->OnChildTotalVisibleNodesChanged(offset - offsetBeforeChildModified);
					}
					offsetBeforeChildModified = 0;
					INodeProviderCallback* proxy = ownerProvider->GetCallbackProxyInternal();
					if (proxy)
					{
						proxy->OnAfterItemModified(ownerProvider, start, count, newCount);
					}
				}

				bool MemoryNodeProvider::NodeCollection::QueryInsert(vint index, Ptr<MemoryNodeProvider> const& child)
				{
					return child->parent == 0;
				}

				bool MemoryNodeProvider::NodeCollection::QueryRemove(vint index, Ptr<MemoryNodeProvider> const& child)
				{
					return child->parent == ownerProvider;
				}

				void MemoryNodeProvider::NodeCollection::BeforeInsert(vint index, Ptr<MemoryNodeProvider> const& child)
				{
					OnBeforeChildModified(index, 0, 1);
					child->parent = ownerProvider;
				}

				void MemoryNodeProvider::NodeCollection::BeforeRemove(vint index, Ptr<MemoryNodeProvider> const& child)
				{
					OnBeforeChildModified(index, 1, 0);
					child->parent = 0;
				}

				void MemoryNodeProvider::NodeCollection::AfterInsert(vint index, Ptr<MemoryNodeProvider> const& child)
				{
					OnAfterChildModified(index, 0, 1);
				}

				void MemoryNodeProvider::NodeCollection::AfterRemove(vint index, vint count)
				{
					OnAfterChildModified(index, count, 0);
				}

				MemoryNodeProvider::NodeCollection::NodeCollection()
					:ownerProvider(0)
				{
				}

/***********************************************************************
MemoryNodeProvider
***********************************************************************/

				INodeProviderCallback* MemoryNodeProvider::GetCallbackProxyInternal()
				{
					if(parent)
					{
						return parent->GetCallbackProxyInternal();
					}
					else
					{
						return 0;
					}
				}

				void MemoryNodeProvider::OnChildTotalVisibleNodesChanged(vint offset)
				{
					totalVisibleNodeCount+=offset;
					if(parent)
					{
						parent->OnChildTotalVisibleNodesChanged(offset);
					}
				}

				MemoryNodeProvider::MemoryNodeProvider(Ptr<DescriptableObject> _data)
					:data(_data)
				{
					children.ownerProvider=this;
				}

				MemoryNodeProvider::~MemoryNodeProvider()
				{
				}

				Ptr<DescriptableObject> MemoryNodeProvider::GetData()
				{
					return data;
				}

				void MemoryNodeProvider::SetData(const Ptr<DescriptableObject>& value)
				{
					data=value;
					NotifyDataModified();
				}

				void MemoryNodeProvider::NotifyDataModified()
				{
					if(parent)
					{
						vint index=parent->children.IndexOf(this);
						INodeProviderCallback* proxy=GetCallbackProxyInternal();
						if(proxy)
						{
							proxy->OnBeforeItemModified(parent, index, 1, 1);
							proxy->OnAfterItemModified(parent, index, 1, 1);
						}
					}
				}

				MemoryNodeProvider::NodeCollection& MemoryNodeProvider::Children()
				{
					return children;
				}

				bool MemoryNodeProvider::GetExpanding()
				{
					return expanding;
				}

				void MemoryNodeProvider::SetExpanding(bool value)
				{
					if(expanding!=value)
					{
						expanding=value;
						vint offset=0;
						for(vint i=0;i<childCount;i++)
						{
							offset+=children[i]->totalVisibleNodeCount;
						}

						OnChildTotalVisibleNodesChanged(expanding?offset:-offset);
						INodeProviderCallback* proxy=GetCallbackProxyInternal();
						if(proxy)
						{
							if(expanding)
							{
								proxy->OnItemExpanded(this);
							}
							else
							{
								proxy->OnItemCollapsed(this);
							}
						}
					}
				}

				vint MemoryNodeProvider::CalculateTotalVisibleNodes()
				{
					return totalVisibleNodeCount;
				}

				vint MemoryNodeProvider::GetChildCount()
				{
					return childCount;
				}

				Ptr<INodeProvider> MemoryNodeProvider::GetParent()
				{
					return parent;
				}

				Ptr<INodeProvider> MemoryNodeProvider::GetChild(vint index)
				{
					if(0<=index && index<childCount)
					{
						return children[index].Obj();
					}
					else
					{
						return nullptr;
					}
				}

/***********************************************************************
NodeRootProviderBase
***********************************************************************/

				void NodeRootProviderBase::OnAttached(INodeRootProvider* provider)
				{
				}

				void NodeRootProviderBase::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnBeforeItemModified(parentNode, start, count, newCount);
					}
				}

				void NodeRootProviderBase::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnAfterItemModified(parentNode, start, count, newCount);
					}
				}

				void NodeRootProviderBase::OnItemExpanded(INodeProvider* node)
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnItemExpanded(node);
					}
				}

				void NodeRootProviderBase::OnItemCollapsed(INodeProvider* node)
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnItemCollapsed(node);
					}
				}

				NodeRootProviderBase::NodeRootProviderBase()
				{
				}

				NodeRootProviderBase::~NodeRootProviderBase()
				{
				}

				bool NodeRootProviderBase::CanGetNodeByVisibleIndex()
				{
					return false;
				}

				Ptr<INodeProvider> NodeRootProviderBase::GetNodeByVisibleIndex(vint index)
				{
					return nullptr;
				}

				bool NodeRootProviderBase::AttachCallback(INodeProviderCallback* value)
				{
					if(callbacks.Contains(value))
					{
						return false;
					}
					else
					{
						callbacks.Add(value);
						value->OnAttached(this);
						return true;
					}
				}

				bool NodeRootProviderBase::DetachCallback(INodeProviderCallback* value)
				{
					vint index=callbacks.IndexOf(value);
					if(index==-1)
					{
						return false;
					}
					else
					{
						value->OnAttached(0);
						callbacks.Remove(value);
						return true;
					}
				}

				IDescriptable* NodeRootProviderBase::RequestView(const WString& identifier)
				{
					return 0;
				}

/***********************************************************************
MemoryNodeRootProvider
***********************************************************************/

				INodeProviderCallback* MemoryNodeRootProvider::GetCallbackProxyInternal()
				{
					return this;
				}

				MemoryNodeRootProvider::MemoryNodeRootProvider()
				{
					SetExpanding(true);
				}

				MemoryNodeRootProvider::~MemoryNodeRootProvider()
				{
				}

				Ptr<INodeProvider> MemoryNodeRootProvider::GetRootNode()
				{
					return this;
				}

				MemoryNodeProvider* MemoryNodeRootProvider::GetMemoryNode(INodeProvider* node)
				{
					return dynamic_cast<MemoryNodeProvider*>(node);
				}
			}

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

			void GuiVirtualTreeListControl::OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)
			{
			}

			void GuiVirtualTreeListControl::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)
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
						Func<void(GuiNodeMouseEvent&, GuiGraphicsComposition*, GuiItemMouseEventArgs&)> func(this, &GuiVirtualTreeListControl::OnItemMouseEvent);\
						ITEMEVENTNAME.AttachFunction(Curry(func)(NODEEVENTNAME));\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(NODEEVENTNAME, ITEMEVENTNAME)\
					{\
						Func<void(GuiNodeNotifyEvent&, GuiGraphicsComposition*, GuiItemEventArgs&)> func(this, &GuiVirtualTreeListControl::OnItemNotifyEvent);\
						ITEMEVENTNAME.AttachFunction(Curry(func)(NODEEVENTNAME));\
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
				nodeItemView = dynamic_cast<tree::INodeItemView*>(GetItemProvider()->RequestView(tree::INodeItemView::Identifier));

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

			namespace tree
			{

/***********************************************************************
TreeViewItem
***********************************************************************/

				const wchar_t* const ITreeViewItemView::Identifier = L"vl::presentation::controls::tree::ITreeViewItemView";

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

				Ptr<GuiImageData> TreeViewItemRootProvider::GetNodeImage(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode=dynamic_cast<MemoryNodeProvider*>(node);
					if(memoryNode)
					{
						Ptr<TreeViewItem> data=memoryNode->GetData().Cast<TreeViewItem>();
						if(data)
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

				TreeViewItemRootProvider::TreeViewItemRootProvider()
				{
				}

				TreeViewItemRootProvider::~TreeViewItemRootProvider()
				{
				}

				IDescriptable* TreeViewItemRootProvider::RequestView(const WString& identifier)
				{
					if(identifier==ITreeViewItemView::Identifier)
					{
						return (ITreeViewItemView*)this;
					}
					else
					{
						return MemoryNodeRootProvider::RequestView(identifier);
					}
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

			void GuiVirtualTreeView::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount)
			{
				GuiVirtualTreeListControl::OnAfterItemModified(parentNode, start, count, newCount);
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
			
			void GuiVirtualTreeView::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				GuiVirtualTreeListControl::OnStyleInstalled(itemIndex, style);
				if (auto treeItemStyle = dynamic_cast<templates::GuiTreeItemTemplate*>(style))
				{
					treeItemStyle->SetTextColor(GetControlTemplateObject(true)->GetTextColor());

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
			}

			GuiVirtualTreeView::GuiVirtualTreeView(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiVirtualTreeListControl(themeName, _nodeRootProvider)
			{
				treeViewItemView = dynamic_cast<tree::ITreeViewItemView*>(GetNodeRootProvider()->RequestView(tree::ITreeViewItemView::Identifier));
				SetStyleAndArranger(
					[](const Value&) { return new tree::DefaultTreeItemTemplate; },
					new list::FixedHeightItemArranger
				);
			}

			GuiVirtualTreeView::~GuiVirtualTreeView()
			{
			}

/***********************************************************************
GuiTreeView
***********************************************************************/

			GuiTreeView::GuiTreeView(theme::ThemeName themeName)
				:GuiVirtualTreeView(themeName, new tree::TreeViewItemRootProvider)
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

			namespace tree
			{

/***********************************************************************
DefaultTreeItemTemplate
***********************************************************************/

				void DefaultTreeItemTemplate::OnInitialize()
				{
					templates::GuiTreeItemTemplate::OnInitialize();
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					table = new GuiTableComposition;
					AddChild(table);
					table->SetRowsAndColumns(3, 4);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());
					table->SetColumnOption(3, GuiCellOption::MinSizeOption());
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(2);
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetPreferredMinSize(Size(16, 16));

						expandingButton = new GuiSelectableButton(theme::ThemeName::TreeItemExpander);
						if (auto treeView = dynamic_cast<GuiVirtualTreeView*>(listControl))
						{
							if (auto expanderStyle = treeView->GetControlTemplateObject(true)->GetExpandingDecoratorTemplate())
							{
								expandingButton->SetControlTemplate(expanderStyle);
							}
						}
						expandingButton->SetAutoFocus(false);
						expandingButton->SetAutoSelection(false);
						expandingButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						expandingButton->GetBoundsComposition()->GetEventReceiver()->leftButtonDoubleClick.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonDoubleClick);
						expandingButton->Clicked.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonClicked);
						cell->AddChild(expandingButton->GetBoundsComposition());
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 2, 1, 1);
						cell->SetPreferredMinSize(Size(16, 16));

						imageElement = GuiImageFrameElement::Create();
						imageElement->SetStretch(true);
						cell->SetOwnedElement(imageElement);
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 3, 3, 1);
						cell->SetPreferredMinSize(Size(192, 0));

						textElement = GuiSolidLabelElement::Create();
						textElement->SetAlignments(Alignment::Left, Alignment::Center);
						textElement->SetEllipse(true);
						cell->SetOwnedElement(textElement);
					}

					FontChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnFontChanged);
					TextChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextChanged);
					TextColorChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextColorChanged);
					ExpandingChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingChanged);
					ExpandableChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandableChanged);
					LevelChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnLevelChanged);
					ImageChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnImageChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
					TextChanged.Execute(compositions::GuiEventArgs(this));
					TextColorChanged.Execute(compositions::GuiEventArgs(this));
					ExpandingChanged.Execute(compositions::GuiEventArgs(this));
					ExpandableChanged.Execute(compositions::GuiEventArgs(this));
					LevelChanged.Execute(compositions::GuiEventArgs(this));
					ImageChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DefaultTreeItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetFont(GetFont());
				}

				void DefaultTreeItemTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetText(GetText());
				}

				void DefaultTreeItemTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetColor(GetTextColor());
				}

				void DefaultTreeItemTemplate::OnExpandingChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					expandingButton->SetSelected(GetExpanding());
				}

				void DefaultTreeItemTemplate::OnExpandableChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					expandingButton->SetVisible(GetExpandable());
				}

				void DefaultTreeItemTemplate::OnLevelChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(GetLevel() * 12));
				}

				void DefaultTreeItemTemplate::OnImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (auto imageData = GetImage())
					{
						imageElement->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						imageElement->SetImage(nullptr);
					}
				}

				void DefaultTreeItemTemplate::OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					arguments.handled = true;
				}

				void DefaultTreeItemTemplate::OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (expandingButton->GetVisuallyEnabled())
					{
						if (auto treeControl = dynamic_cast<GuiVirtualTreeListControl*>(listControl))
						{
							if (auto view = treeControl->GetNodeItemView())
							{
								vint index = treeControl->GetArranger()->GetVisibleIndex(this);
								if (index != -1)
								{
									if (auto node = view->RequestNode(index))
									{
										bool expanding = node->GetExpanding();
										node->SetExpanding(!expanding);
									}
								}
							}
						}
					}
				}

				DefaultTreeItemTemplate::DefaultTreeItemTemplate()
				{
				}

				DefaultTreeItemTemplate::~DefaultTreeItemTemplate()
				{
				}
			}
		}
	}
}