#include "GuiTreeViewControls.h"

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
				const wchar_t* const INodeItemPrimaryTextView::Identifier = L"vl::presentation::cotnrols::tree::INodeItemPrimaryTextView";
				const wchar_t* const INodeItemBindingView::Identifier = L"vl::presentation::cotnrols::tree::INodeItemBindingView";

/***********************************************************************
NodeItemProvider
***********************************************************************/

				INodeProvider* NodeItemProvider::GetNodeByOffset(INodeProvider* provider, vint offset)
				{
					if(offset==0) return provider;
					INodeProvider* result=0;
					if(provider->GetExpanding() && offset>0)
					{
						offset-=1;
						vint count=provider->GetChildCount();
						for(vint i=0;(!result && i<count);i++)
						{
							INodeProvider* child=provider->GetChild(i);
							vint visibleCount=child->CalculateTotalVisibleNodes();
							if(offset<visibleCount)
							{
								result=GetNodeByOffset(child, offset);
							}
							else
							{
								offset-=visibleCount;
							}
						}
					}
					ReleaseNode(provider);
					return result;
				}

				void NodeItemProvider::OnAttached(INodeRootProvider* provider)
				{
				}

				void NodeItemProvider::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
					offsetBeforeChildModified=0;
					vint base=CalculateNodeVisibilityIndexInternal(parentNode);
					if(base!=-2 && parentNode->GetExpanding())
					{
						for(vint i=0;i<count;i++)
						{
							INodeProvider* child=parentNode->GetChild(start+i);
							offsetBeforeChildModified+=child->CalculateTotalVisibleNodes();
							child->Release();
						}
					}
				}

				void NodeItemProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
					vint base=CalculateNodeVisibilityIndexInternal(parentNode);
					if(base!=-2 && parentNode->GetExpanding())
					{
						vint offset=0;
						vint firstChildStart=-1;
						for(vint i=0;i<newCount;i++)
						{
							INodeProvider* child=parentNode->GetChild(start+i);
							if(i==0)
							{
								firstChildStart=CalculateNodeVisibilityIndexInternal(child);
							}
							offset+=child->CalculateTotalVisibleNodes();
							child->Release();
						}

						if(firstChildStart==-1)
						{
							vint childCount=parentNode->GetChildCount();
							if(childCount==0)
							{
								firstChildStart=base+1;
							}
							else if(start<childCount)
							{
								INodeProvider* child=parentNode->GetChild(start);
								firstChildStart=CalculateNodeVisibilityIndexInternal(child);
								child->Release();
							}
							else
							{
								INodeProvider* child=parentNode->GetChild(start-1);
								firstChildStart=CalculateNodeVisibilityIndexInternal(child);
								firstChildStart+=child->CalculateTotalVisibleNodes();
								child->Release();
							}
						}
						InvokeOnItemModified(firstChildStart, offsetBeforeChildModified, offset);
					}
				}

				void NodeItemProvider::OnItemExpanded(INodeProvider* node)
				{
					vint base=CalculateNodeVisibilityIndexInternal(node);
					if(base!=-2)
					{
						vint visibility=node->CalculateTotalVisibleNodes();
						InvokeOnItemModified(base+1, 0, visibility-1);
					}
				}

				void NodeItemProvider::OnItemCollapsed(INodeProvider* node)
				{
					vint base=CalculateNodeVisibilityIndexInternal(node);
					if(base!=-2)
					{
						vint visibility=0;
						vint count=node->GetChildCount();
						for(vint i=0;i<count;i++)
						{
							INodeProvider* child=node->GetChild(i);
							visibility+=child->CalculateTotalVisibleNodes();
							child->Release();
						}
						InvokeOnItemModified(base+1, visibility, 0);
					}
				}

				vint NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider* node)
				{
					INodeProvider* parent=node->GetParent();
					if(parent==0)
					{
						return -1;
					}
					if(!parent->GetExpanding())
					{
						return -2;
					}

					vint index=CalculateNodeVisibilityIndexInternal(parent);
					if(index==-2)
					{
						return -2;
					}

					vint count=parent->GetChildCount();
					for(vint i=0;i<count;i++)
					{
						INodeProvider* child=parent->GetChild(i);
						bool findResult=child==node;
						if(findResult)
						{
							index++;
						}
						else
						{
							index+=child->CalculateTotalVisibleNodes();
						}
						child->Release();
						if(findResult)
						{
							return index;
						}
					}
					return -1;
				}

				vint NodeItemProvider::CalculateNodeVisibilityIndex(INodeProvider* node)
				{
					vint result=CalculateNodeVisibilityIndexInternal(node);
					return result<0?-1:result;
				}

				bool NodeItemProvider::ContainsPrimaryText(vint itemIndex)
				{
					if(nodeItemPrimaryTextView)
					{
						INodeProvider* node=RequestNode(itemIndex);
						if(node)
						{
							bool result=node->GetChildCount()==0;
							ReleaseNode(node);
							return result;
						}
					}
					return true;
				}

				WString NodeItemProvider::GetPrimaryTextViewText(vint itemIndex)
				{
					if(nodeItemPrimaryTextView)
					{
						INodeProvider* node=RequestNode(itemIndex);
						if(node)
						{
							WString result=nodeItemPrimaryTextView->GetPrimaryTextViewText(node);
							ReleaseNode(node);
							return result;
						}
					}
					return L"";
				}

				INodeProvider* NodeItemProvider::RequestNode(vint index)
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

				void NodeItemProvider::ReleaseNode(INodeProvider* node)
				{
					if(node)
					{
						node->Release();
					}
				}

				NodeItemProvider::NodeItemProvider(Ptr<INodeRootProvider> _root)
					:root(_root)
				{
					root->AttachCallback(this);
					nodeItemPrimaryTextView=dynamic_cast<INodeItemPrimaryTextView*>(root->RequestView(INodeItemPrimaryTextView::Identifier));
				}

				NodeItemProvider::~NodeItemProvider()
				{
					if(nodeItemPrimaryTextView)
					{
						root->ReleaseView(nodeItemPrimaryTextView);
					}
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

				IDescriptable* NodeItemProvider::RequestView(const WString& identifier)
				{
					if(identifier==INodeItemView::Identifier)
					{
						return (INodeItemView*)this;
					}
					else if(identifier==GuiListControl::IItemPrimaryTextView::Identifier)
					{
						return (GuiListControl::IItemPrimaryTextView*)this;
					}
					else
					{
						return root->RequestView(identifier);
					}
				}

				void NodeItemProvider::ReleaseView(IDescriptable* view)
				{
					if(dynamic_cast<INodeItemView*>(view)==0)
					{
						root->ReleaseView(view);
					}
				}

/***********************************************************************
NodeItemProvider
***********************************************************************/

				NodeItemStyleProvider::NodeItemStyleProvider(Ptr<INodeItemStyleProvider> provider)
					:nodeItemStyleProvider(provider)
					,listControl(0)
					,nodeItemView(0)
				{
					nodeItemStyleProvider->BindItemStyleProvider(this);
				}

				NodeItemStyleProvider::~NodeItemStyleProvider()
				{
				}

				void NodeItemStyleProvider::AttachListControl(GuiListControl* value)
				{
					listControl=value;
					nodeItemView=dynamic_cast<INodeItemView*>(listControl->GetItemProvider()->RequestView(INodeItemView::Identifier));
					nodeItemStyleProvider->AttachListControl(value);
				}

				void NodeItemStyleProvider::DetachListControl()
				{
					nodeItemStyleProvider->DetachListControl();
					if(nodeItemView)
					{
						listControl->GetItemProvider()->ReleaseView(nodeItemView);
						nodeItemView=0;
					}
					listControl=0;
				}

				vint NodeItemStyleProvider::GetItemStyleId(vint itemIndex)
				{
					vint result=-1;
					if(nodeItemView)
					{
						INodeProvider* node=nodeItemView->RequestNode(itemIndex);
						if(node)
						{
							result=nodeItemStyleProvider->GetItemStyleId(node);
							nodeItemView->ReleaseNode(node);
						}
					}
					return result;
				}

				GuiListControl::IItemStyleController* NodeItemStyleProvider::CreateItemStyle(vint styleId)
				{
					return nodeItemStyleProvider->CreateItemStyle(styleId);
				}

				void NodeItemStyleProvider::DestroyItemStyle(GuiListControl::IItemStyleController* style)
				{
					INodeItemStyleController* nodeStyle=dynamic_cast<INodeItemStyleController*>(style);
					if(nodeStyle)
					{
						nodeItemStyleProvider->DestroyItemStyle(nodeStyle);
					}
				}

				void NodeItemStyleProvider::Install(GuiListControl::IItemStyleController* style, vint itemIndex)
				{
					if(nodeItemView)
					{
						INodeItemStyleController* nodeStyle=dynamic_cast<INodeItemStyleController*>(style);
						if(nodeStyle)
						{
							INodeProvider* node=nodeItemView->RequestNode(itemIndex);
							if(node)
							{
								nodeItemStyleProvider->Install(nodeStyle, node, itemIndex);
							}
						}
					}
				}

				void NodeItemStyleProvider::SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)
				{
					if(nodeItemView)
					{
						INodeItemStyleController* nodeStyle=dynamic_cast<INodeItemStyleController*>(style);
						if(nodeStyle)
						{
							nodeItemStyleProvider->SetStyleIndex(nodeStyle, value);
						}
					}
				}

				void NodeItemStyleProvider::SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)
				{
					if(nodeItemView)
					{
						INodeItemStyleController* nodeStyle=dynamic_cast<INodeItemStyleController*>(style);
						if(nodeStyle)
						{
							nodeItemStyleProvider->SetStyleSelected(nodeStyle, value);
						}
					}
				}

/***********************************************************************
MemoryNodeProvider::NodeCollection
***********************************************************************/

				void MemoryNodeProvider::NodeCollection::OnBeforeChildModified(vint start, vint count, vint newCount)
				{
					ownerProvider->offsetBeforeChildModified=0;
					if(ownerProvider->expanding)
					{
						for(vint i=0;i<count;i++)
						{
							ownerProvider->offsetBeforeChildModified+=items[start+i]->totalVisibleNodeCount;
						}
					}
					INodeProviderCallback* proxy=ownerProvider->GetCallbackProxyInternal();
					if(proxy)
					{
						proxy->OnBeforeItemModified(ownerProvider, start, count, newCount);
					}
				}

				void MemoryNodeProvider::NodeCollection::OnAfterChildModified(vint start, vint count, vint newCount)
				{
					ownerProvider->childCount+=(newCount-count);
					if(ownerProvider->expanding)
					{
						vint offset=0;
						for(vint i=0;i<newCount;i++)
						{
							offset+=items[start+i]->totalVisibleNodeCount;
						}
						ownerProvider->OnChildTotalVisibleNodesChanged(offset-ownerProvider->offsetBeforeChildModified);
					}
					INodeProviderCallback* proxy=ownerProvider->GetCallbackProxyInternal();
					if(proxy)
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

				MemoryNodeProvider::MemoryNodeProvider()
					:parent(0)
					,expanding(false)
					,childCount(0)
					,totalVisibleNodeCount(1)
					,offsetBeforeChildModified(0)
				{
					children.ownerProvider=this;
				}

				MemoryNodeProvider::MemoryNodeProvider(const Ptr<IMemoryNodeData>& _data)
					:parent(0)
					,expanding(false)
					,childCount(0)
					,totalVisibleNodeCount(1)
					,offsetBeforeChildModified(0)
					,data(_data)
				{
					children.ownerProvider=this;
				}

				MemoryNodeProvider::~MemoryNodeProvider()
				{
				}

				Ptr<IMemoryNodeData> MemoryNodeProvider::GetData()
				{
					return data;
				}

				void MemoryNodeProvider::SetData(const Ptr<IMemoryNodeData>& value)
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

				INodeProvider* MemoryNodeProvider::GetParent()
				{
					return parent;
				}

				INodeProvider* MemoryNodeProvider::GetChild(vint index)
				{
					if(0<=index && index<childCount)
					{
						return children[index].Obj();
					}
					else
					{
						return 0;
					}
				}

				void MemoryNodeProvider::Increase()
				{
				}

				void MemoryNodeProvider::Release()
				{
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

				INodeProvider* NodeRootProviderBase::GetNodeByVisibleIndex(vint index)
				{
					return 0;
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

				void NodeRootProviderBase::ReleaseView(IDescriptable* view)
				{
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

				INodeProvider* MemoryNodeRootProvider::GetRootNode()
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
				tree::INodeProvider* node=GetNodeItemView()->RequestNode(arguments.itemIndex);
				if(node)
				{
					GuiNodeMouseEventArgs redirectArguments;
					(GuiMouseEventArgs&)redirectArguments=arguments;
					redirectArguments.node=node;
					nodeEvent.Execute(redirectArguments);
					(GuiMouseEventArgs&)arguments=redirectArguments;
					GetNodeItemView()->ReleaseNode(node);
				}
			}

			void GuiVirtualTreeListControl::OnItemNotifyEvent(compositions::GuiNodeNotifyEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				tree::INodeProvider* node=GetNodeItemView()->RequestNode(arguments.itemIndex);
				if(node)
				{
					GuiNodeEventArgs redirectArguments;
					(GuiEventArgs&)redirectArguments=arguments;
					redirectArguments.node=node;
					nodeEvent.Execute(redirectArguments);
					(GuiEventArgs&)arguments=redirectArguments;
					GetNodeItemView()->ReleaseNode(node);
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

			GuiVirtualTreeListControl::GuiVirtualTreeListControl(IStyleProvider* _styleProvider, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiSelectableListControl(_styleProvider, new tree::NodeItemProvider(_nodeRootProvider))
			{
				nodeItemProvider=dynamic_cast<tree::NodeItemProvider*>(GetItemProvider());
				nodeItemView=dynamic_cast<tree::INodeItemView*>(GetItemProvider()->RequestView(tree::INodeItemView::Identifier));
				
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
			}

#undef ATTACH_ITEM_MOUSE_EVENT
#undef ATTACH_ITEM_NOTIFY_EVENT

			GuiVirtualTreeListControl::~GuiVirtualTreeListControl()
			{
				GetItemProvider()->ReleaseView(nodeItemView);
			}

			tree::INodeItemView* GuiVirtualTreeListControl::GetNodeItemView()
			{
				return nodeItemView;
			}

			tree::INodeRootProvider* GuiVirtualTreeListControl::GetNodeRootProvider()
			{
				return nodeItemProvider->GetRoot().Obj();
			}

			tree::INodeItemStyleProvider* GuiVirtualTreeListControl::GetNodeStyleProvider()
			{
				return nodeStyleProvider.Obj();
			}

			Ptr<tree::INodeItemStyleProvider> GuiVirtualTreeListControl::SetNodeStyleProvider(Ptr<tree::INodeItemStyleProvider> styleProvider)
			{
				Ptr<tree::INodeItemStyleProvider> old=nodeStyleProvider;
				nodeStyleProvider=styleProvider;
				GuiSelectableListControl::SetStyleProvider(new tree::NodeItemStyleProvider(nodeStyleProvider));
				return old;
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

				WString TreeViewItemRootProvider::GetPrimaryTextViewText(INodeProvider* node)
				{
					return GetNodeText(node);
				}

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

				WString	TreeViewItemRootProvider::GetNodeText(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode=dynamic_cast<MemoryNodeProvider*>(node);
					if(memoryNode)
					{
						Ptr<TreeViewItem> data=memoryNode->GetData().Cast<TreeViewItem>();
						if(data)
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
					else if(identifier==INodeItemPrimaryTextView::Identifier)
					{
						return (INodeItemPrimaryTextView*)this;
					}
					else if(identifier==INodeItemBindingView::Identifier)
					{
						return (INodeItemBindingView*)this;
					}
					else
					{
						return MemoryNodeRootProvider::RequestView(identifier);
					}
				}

				void TreeViewItemRootProvider::ReleaseView(IDescriptable* view)
				{
					return MemoryNodeRootProvider::ReleaseView(view);
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

			GuiVirtualTreeView::GuiVirtualTreeView(IStyleProvider* _styleProvider, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiVirtualTreeListControl(_styleProvider, _nodeRootProvider)
			{
				styleProvider=dynamic_cast<IStyleProvider*>(styleController->GetStyleProvider());
				SetNodeStyleProvider(new tree::TreeViewNodeItemStyleProvider);
				SetArranger(new list::FixedHeightItemArranger);
			}

			GuiVirtualTreeView::~GuiVirtualTreeView()
			{
			}

			GuiVirtualTreeView::IStyleProvider* GuiVirtualTreeView::GetTreeViewStyleProvider()
			{
				return styleProvider;
			}

/***********************************************************************
GuiTreeView
***********************************************************************/

			GuiTreeView::GuiTreeView(IStyleProvider* _styleProvider)
				:GuiVirtualTreeView(_styleProvider, new tree::TreeViewItemRootProvider)
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
						if (auto memoryNode = dynamic_cast<tree::MemoryNodeProvider*>(node))
						{
							result = memoryNode->GetData().Cast<tree::TreeViewItem>();
						}
						nodeItemView->ReleaseNode(node);
					}
				}
				return result;
			}

			namespace tree
			{
/***********************************************************************
TreeViewNodeItemStyleProvider::ItemController
***********************************************************************/

				void TreeViewNodeItemStyleProvider::ItemController::SwitchNodeExpanding()
				{
					if(backgroundButton->GetBoundsComposition()->GetParent())
					{
						GuiListControl::IItemArranger* arranger=styleProvider->treeControl->GetArranger();
						vint index=arranger->GetVisibleIndex(this);
						if(index!=-1)
						{
							INodeItemView* view=styleProvider->treeControl->GetNodeItemView();
							INodeProvider* node=view->RequestNode(index);
							if(node)
							{
								bool expanding=node->GetExpanding();
								node->SetExpanding(!expanding);
								view->ReleaseNode(node);
							}
						}
					}
				}

				void TreeViewNodeItemStyleProvider::ItemController::OnBackgroundButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(backgroundButton->GetVisuallyEnabled())
					{
						SwitchNodeExpanding();
					}
				}

				void TreeViewNodeItemStyleProvider::ItemController::OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					arguments.handled=true;
				}

				void TreeViewNodeItemStyleProvider::ItemController::OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if(expandingButton->GetVisuallyEnabled())
					{
						SwitchNodeExpanding();
					}
				}

				TreeViewNodeItemStyleProvider::ItemController::ItemController(TreeViewNodeItemStyleProvider* _styleProvider, Size minIconSize, bool fitImage)
					:list::ItemStyleControllerBase(_styleProvider->GetBindedItemStyleProvider(), 0)
					,styleProvider(_styleProvider)
				{
					backgroundButton=new GuiSelectableButton(styleProvider->treeControl->GetTreeViewStyleProvider()->CreateItemBackground());
					backgroundButton->SetAutoSelection(false);
					backgroundButton->GetBoundsComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					backgroundButton->GetEventReceiver()->leftButtonDoubleClick.AttachMethod(this, &ItemController::OnBackgroundButtonDoubleClick);

					table=new GuiTableComposition;
					backgroundButton->GetBoundsComposition()->AddChild(table);
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
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetPreferredMinSize(Size(16, 16));

						expandingButton=new GuiSelectableButton(styleProvider->treeControl->GetTreeViewStyleProvider()->CreateItemExpandingDecorator());
						expandingButton->SetAutoSelection(false);
						expandingButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						expandingButton->GetEventReceiver()->leftButtonDoubleClick.AttachMethod(this, &ItemController::OnExpandingButtonDoubleClick);
						expandingButton->Clicked.AttachMethod(this, &ItemController::OnExpandingButtonClicked);
						cell->AddChild(expandingButton->GetBoundsComposition());
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 2, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 3, 3, 1);
						cell->SetPreferredMinSize(Size(192, 0));

						text=GuiSolidLabelElement::Create();
						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetFont(styleProvider->treeControl->GetFont());
						text->SetEllipse(true);
						cell->SetOwnedElement(text);
					}
					Initialize(backgroundButton->GetBoundsComposition(), backgroundButton);
				}

				INodeItemStyleProvider* TreeViewNodeItemStyleProvider::ItemController::GetNodeStyleProvider()
				{
					return styleProvider;
				}

				void TreeViewNodeItemStyleProvider::ItemController::Install(INodeProvider* node)
				{
					ITreeViewItemView* view=styleProvider->treeViewItemView;
					Ptr<GuiImageData> imageData=view->GetNodeImage(node);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetNodeText(node));
					text->SetColor(styleProvider->treeControl->GetTreeViewStyleProvider()->GetTextColor());
					UpdateExpandingButton(node);

					vint level=-2;
					while(node)
					{
						node=node->GetParent();
						level++;
					}
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(level*16));
				}

				bool TreeViewNodeItemStyleProvider::ItemController::GetSelected()
				{
					return backgroundButton->GetSelected();
				}

				void TreeViewNodeItemStyleProvider::ItemController::SetSelected(bool value)
				{
					backgroundButton->SetSelected(value);
				}

				void TreeViewNodeItemStyleProvider::ItemController::UpdateExpandingButton(INodeProvider* associatedNode)
				{
					expandingButton->SetSelected(associatedNode->GetExpanding());
					expandingButton->SetVisible(associatedNode->GetChildCount()>0);
				}

/***********************************************************************
TreeViewNodeItemStyleProvider
***********************************************************************/

				TreeViewNodeItemStyleProvider::ItemController* TreeViewNodeItemStyleProvider::GetRelatedController(INodeProvider* node)
				{
					vint index=treeControl->GetNodeItemView()->CalculateNodeVisibilityIndex(node);
					if(index!=-1)
					{
						GuiListControl::IItemStyleController* style=treeControl->GetArranger()->GetVisibleStyle(index);
						if(style)
						{
							ItemController* itemController=dynamic_cast<ItemController*>(style);
							return itemController;
						}
					}
					return 0;
				}

				void TreeViewNodeItemStyleProvider::UpdateExpandingButton(INodeProvider* node)
				{
					ItemController* itemController=GetRelatedController(node);
					if(itemController)
					{
						itemController->UpdateExpandingButton(node);
					}
				}

				void TreeViewNodeItemStyleProvider::OnAttached(INodeRootProvider* provider)
				{
				}

				void TreeViewNodeItemStyleProvider::OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
				}

				void TreeViewNodeItemStyleProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount)
				{
					UpdateExpandingButton(parentNode);
				}

				void TreeViewNodeItemStyleProvider::OnItemExpanded(INodeProvider* node)
				{
					UpdateExpandingButton(node);
				}

				void TreeViewNodeItemStyleProvider::OnItemCollapsed(INodeProvider* node)
				{
					UpdateExpandingButton(node);
				}

				TreeViewNodeItemStyleProvider::TreeViewNodeItemStyleProvider(Size _minIconSize, bool _fitImage)
					:treeControl(0)
					,bindedItemStyleProvider(0)
					,treeViewItemView(0)
					, minIconSize(_minIconSize)
					, fitImage(_fitImage)
				{
				}

				TreeViewNodeItemStyleProvider::~TreeViewNodeItemStyleProvider()
				{
				}

				void TreeViewNodeItemStyleProvider::BindItemStyleProvider(GuiListControl::IItemStyleProvider* styleProvider)
				{
					bindedItemStyleProvider=styleProvider;
				}

				GuiListControl::IItemStyleProvider* TreeViewNodeItemStyleProvider::GetBindedItemStyleProvider()
				{
					return bindedItemStyleProvider;
				}

				void TreeViewNodeItemStyleProvider::AttachListControl(GuiListControl* value)
				{
					treeControl=dynamic_cast<GuiVirtualTreeView*>(value);
					if(treeControl)
					{
						treeViewItemView=dynamic_cast<ITreeViewItemView*>(treeControl->GetItemProvider()->RequestView(ITreeViewItemView::Identifier));
						treeControl->GetNodeRootProvider()->AttachCallback(this);
					}
				}

				void TreeViewNodeItemStyleProvider::DetachListControl()
				{
					if(treeViewItemView)
					{
						treeControl->GetItemProvider()->ReleaseView(treeViewItemView);
						treeViewItemView=0;
					}
					if(treeControl)
					{
						treeControl->GetNodeRootProvider()->DetachCallback(this);
						treeControl=0;
					}
				}

				vint TreeViewNodeItemStyleProvider::GetItemStyleId(INodeProvider* node)
				{
					return 0;
				}

				INodeItemStyleController* TreeViewNodeItemStyleProvider::CreateItemStyle(vint styleId)
				{
					return new ItemController(this, minIconSize, fitImage);
				}

				void TreeViewNodeItemStyleProvider::DestroyItemStyle(INodeItemStyleController* style)
				{
					ItemController* itemController=dynamic_cast<ItemController*>(style);
					if(itemController)
					{
						delete itemController;
					}
				}

				void TreeViewNodeItemStyleProvider::Install(INodeItemStyleController* style, INodeProvider* node, vint itemIndex)
				{
					ItemController* itemController=dynamic_cast<ItemController*>(style);
					if(itemController)
					{
						itemController->Install(node);
					}
				}

				void TreeViewNodeItemStyleProvider::SetStyleIndex(INodeItemStyleController* style, vint value)
				{
				}

				void TreeViewNodeItemStyleProvider::SetStyleSelected(INodeItemStyleController* style, bool value)
				{
					ItemController* itemController=dynamic_cast<ItemController*>(style);
					if(itemController)
					{
						itemController->SetSelected(value);
					}
				}
			}
		}
	}
}