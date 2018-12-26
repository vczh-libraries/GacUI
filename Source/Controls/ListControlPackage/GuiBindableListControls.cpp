#include "GuiBindableListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace list;
			using namespace tree;
			using namespace reflection::description;
			using namespace templates;

/***********************************************************************
GuiBindableTextList::ItemSource
***********************************************************************/

			GuiBindableTextList::ItemSource::ItemSource()
			{
			}

			GuiBindableTextList::ItemSource::~ItemSource()
			{
				SetItemSource(nullptr);
			}

			Ptr<description::IValueEnumerable> GuiBindableTextList::ItemSource::GetItemSource()
			{
				return itemSource;
			}

			void GuiBindableTextList::ItemSource::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				vint oldCount = 0;
				if (itemSource)
				{
					oldCount = itemSource->GetCount();
				}
				if (itemChangedEventHandler)
				{
					auto ol = itemSource.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
				}

				itemSource = nullptr;
				itemChangedEventHandler = nullptr;

				if (_itemSource)
				{
					if (auto ol = _itemSource.Cast<IValueObservableList>())
					{
						itemSource = ol;
						itemChangedEventHandler = ol->ItemChanged.Add([this](vint start, vint oldCount, vint newCount)
						{
							InvokeOnItemModified(start, oldCount, newCount);
						});
					}
					else if (auto rl = _itemSource.Cast<IValueReadonlyList>())
					{
						itemSource = rl;
					}
					else
					{
						itemSource = IValueList::Create(GetLazyList<Value>(_itemSource));
					}
				}

				InvokeOnItemModified(0, oldCount, itemSource ? itemSource->GetCount() : 0);
			}

			description::Value GuiBindableTextList::ItemSource::Get(vint index)
			{
				if (!itemSource) return Value();
				return itemSource->Get(index);
			}

			void GuiBindableTextList::ItemSource::UpdateBindingProperties()
			{
				InvokeOnItemModified(0, Count(), Count());
			}
					
			// ===================== GuiListControl::IItemProvider =====================
			
			vint GuiBindableTextList::ItemSource::Count()
			{
				if (!itemSource) return 0;
				return itemSource->GetCount();
			}

			WString GuiBindableTextList::ItemSource::GetTextValue(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return ReadProperty(itemSource->Get(itemIndex), textProperty);
					}
				}
				return L"";
			}
			
			IDescriptable* GuiBindableTextList::ItemSource::RequestView(const WString& identifier)
			{
				if (identifier == ITextItemView::Identifier)
				{
					return (ITextItemView*)this;
				}
				else
				{
					return 0;
				}
			}
					
			// ===================== GuiListControl::IItemBindingView =====================

			description::Value GuiBindableTextList::ItemSource::GetBindingValue(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return itemSource->Get(itemIndex);
					}
				}
				return Value();
			}
					
			// ===================== list::TextItemStyleProvider::ITextItemView =====================
			
			bool GuiBindableTextList::ItemSource::GetChecked(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return ReadProperty(itemSource->Get(itemIndex), checkedProperty);
					}
				}
				return false;
			}
			
			void GuiBindableTextList::ItemSource::SetChecked(vint itemIndex, bool value)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						auto thisValue = itemSource->Get(itemIndex);
						WriteProperty(thisValue, checkedProperty, value);
						InvokeOnItemModified(itemIndex, 1, 1);
					}
				}
			}

/***********************************************************************
GuiBindableTextList
***********************************************************************/

			GuiBindableTextList::GuiBindableTextList(theme::ThemeName themeName)
				:GuiVirtualTextList(themeName, new ItemSource)
			{
				itemSource = dynamic_cast<ItemSource*>(GetItemProvider());

				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableTextList::~GuiBindableTextList()
			{
			}

			Ptr<description::IValueEnumerable> GuiBindableTextList::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableTextList::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}

			ItemProperty<WString> GuiBindableTextList::GetTextProperty()
			{
				return itemSource->textProperty;
			}

			void GuiBindableTextList::SetTextProperty(const ItemProperty<WString>& value)
			{
				if (itemSource->textProperty != value)
				{
					itemSource->textProperty = value;
					itemSource->UpdateBindingProperties();
					TextPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			WritableItemProperty<bool> GuiBindableTextList::GetCheckedProperty()
			{
				return itemSource->checkedProperty;
			}

			void GuiBindableTextList::SetCheckedProperty(const WritableItemProperty<bool>& value)
			{
				if (itemSource->checkedProperty != value)
				{
					itemSource->checkedProperty = value;
					itemSource->UpdateBindingProperties();
					CheckedPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableTextList::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();
				return itemSource->Get(index);
			}

/***********************************************************************
GuiBindableListView::ItemSource
***********************************************************************/

			GuiBindableListView::ItemSource::ItemSource()
				:columns(this)
				, dataColumns(this)
			{
			}

			GuiBindableListView::ItemSource::~ItemSource()
			{
				SetItemSource(nullptr);
			}

			Ptr<description::IValueEnumerable> GuiBindableListView::ItemSource::GetItemSource()
			{
				return itemSource;
			}

			void GuiBindableListView::ItemSource::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				vint oldCount = 0;
				if (itemSource)
				{
					oldCount = itemSource->GetCount();
				}
				if (itemChangedEventHandler)
				{
					auto ol = itemSource.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
				}

				itemSource = nullptr;
				itemChangedEventHandler = nullptr;

				if (_itemSource)
				{
					if (auto ol = _itemSource.Cast<IValueObservableList>())
					{
						itemSource = ol;
						itemChangedEventHandler = ol->ItemChanged.Add([this](vint start, vint oldCount, vint newCount)
						{
							InvokeOnItemModified(start, oldCount, newCount);
						});
					}
					else if (auto rl = _itemSource.Cast<IValueReadonlyList>())
					{
						itemSource = rl;
					}
					else
					{
						itemSource = IValueList::Create(GetLazyList<Value>(_itemSource));
					}
				}

				InvokeOnItemModified(0, oldCount, itemSource ? itemSource->GetCount() : 0);
			}

			description::Value GuiBindableListView::ItemSource::Get(vint index)
			{
				if (!itemSource) return Value();
				return itemSource->Get(index);
			}

			void GuiBindableListView::ItemSource::UpdateBindingProperties()
			{
				InvokeOnItemModified(0, Count(), Count());
			}

			bool GuiBindableListView::ItemSource::NotifyUpdate(vint start, vint count)
			{
				if (!itemSource) return false;
				if (start<0 || start >= itemSource->GetCount() || count <= 0 || start + count > itemSource->GetCount())
				{
					return false;
				}
				else
				{
					InvokeOnItemModified(start, count, count);
					return true;
				}
			}

			list::ListViewDataColumns& GuiBindableListView::ItemSource::GetDataColumns()
			{
				return dataColumns;
			}

			list::ListViewColumns& GuiBindableListView::ItemSource::GetColumns()
			{
				return columns;
			}
					
			// ===================== list::IListViewItemProvider =====================

			void GuiBindableListView::ItemSource::NotifyAllItemsUpdate()
			{
				NotifyUpdate(0, Count());
			}

			void GuiBindableListView::ItemSource::NotifyAllColumnsUpdate()
			{
				for (vint i = 0; i < columnItemViewCallbacks.Count(); i++)
				{
					columnItemViewCallbacks[i]->OnColumnChanged();
				}
			}

			// ===================== GuiListControl::IItemProvider =====================

			vint GuiBindableListView::ItemSource::Count()
			{
				if (!itemSource) return 0;
				return itemSource->GetCount();
			}

			WString GuiBindableListView::ItemSource::GetTextValue(vint itemIndex)
			{
				return GetText(itemIndex);
			}

			description::Value GuiBindableListView::ItemSource::GetBindingValue(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return itemSource->Get(itemIndex);
					}
				}
				return Value();
			}

			IDescriptable* GuiBindableListView::ItemSource::RequestView(const WString& identifier)
			{
				if (identifier == IListViewItemView::Identifier)
				{
					return (IListViewItemView*)this;
				}
				else if (identifier == ListViewColumnItemArranger::IColumnItemView::Identifier)
				{
					return (ListViewColumnItemArranger::IColumnItemView*)this;
				}
				else
				{
					return 0;
				}
			}

			// ===================== list::ListViewItemStyleProvider::IListViewItemView =====================

			Ptr<GuiImageData> GuiBindableListView::ItemSource::GetSmallImage(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return ReadProperty(itemSource->Get(itemIndex), smallImageProperty);
					}
				}
				return nullptr;
			}

			Ptr<GuiImageData> GuiBindableListView::ItemSource::GetLargeImage(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount())
					{
						return ReadProperty(itemSource->Get(itemIndex), largeImageProperty);
					}
				}
				return nullptr;
			}

			WString GuiBindableListView::ItemSource::GetText(vint itemIndex)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount() && columns.Count()>0)
					{
						return ReadProperty(itemSource->Get(itemIndex), columns[0]->GetTextProperty());
					}
				}
				return L"";
			}

			WString GuiBindableListView::ItemSource::GetSubItem(vint itemIndex, vint index)
			{
				if (itemSource)
				{
					if (0 <= itemIndex && itemIndex < itemSource->GetCount() && 0 <= index && index < columns.Count() - 1)
					{
						return ReadProperty(itemSource->Get(itemIndex), columns[index + 1]->GetTextProperty());
					}
				}
				return L"";
			}

			vint GuiBindableListView::ItemSource::GetDataColumnCount()
			{
				return dataColumns.Count();
			}

			vint GuiBindableListView::ItemSource::GetDataColumn(vint index)
			{
				return dataColumns[index];
			}

			vint GuiBindableListView::ItemSource::GetColumnCount()
			{
				return columns.Count();
			}

			WString GuiBindableListView::ItemSource::GetColumnText(vint index)
			{
				if (index < 0 || index >= columns.Count())
				{
					return L"";
				}
				else
				{
					return columns[index]->GetText();
				}
			}

			// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================

			bool GuiBindableListView::ItemSource::AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
			{
				if(columnItemViewCallbacks.Contains(value))
				{
					return false;
				}
				else
				{
					columnItemViewCallbacks.Add(value);
					return true;
				}
			}

			bool GuiBindableListView::ItemSource::DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
			{
				vint index = columnItemViewCallbacks.IndexOf(value);
				if (index == -1)
				{
					return false;
				}
				else
				{
					columnItemViewCallbacks.Remove(value);
					return true;
				}
			}

			vint GuiBindableListView::ItemSource::GetColumnSize(vint index)
			{
				if (index < 0 || index >= columns.Count())
				{
					return 0;
				}
				else
				{
					return columns[index]->GetSize();
				}
			}

			void GuiBindableListView::ItemSource::SetColumnSize(vint index, vint value)
			{
				if (index >= 0 && index < columns.Count())
				{
					columns[index]->SetSize(value);
				}
			}

			GuiMenu* GuiBindableListView::ItemSource::GetDropdownPopup(vint index)
			{
				if (index < 0 || index >= columns.Count())
				{
					return 0;
				}
				else
				{
					return columns[index]->GetDropdownPopup();
				}
			}

			ColumnSortingState GuiBindableListView::ItemSource::GetSortingState(vint index)
			{
				if (index < 0 || index >= columns.Count())
				{
					return ColumnSortingState::NotSorted;
				}
				else
				{
					return columns[index]->GetSortingState();
				}
			}

/***********************************************************************
GuiBindableListView
***********************************************************************/

			GuiBindableListView::GuiBindableListView(theme::ThemeName themeName)
				:GuiVirtualListView(themeName, new ItemSource)
			{
				itemSource = dynamic_cast<ItemSource*>(GetItemProvider());

				LargeImagePropertyChanged.SetAssociatedComposition(boundsComposition);
				SmallImagePropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableListView::~GuiBindableListView()
			{
			}

			list::ListViewDataColumns& GuiBindableListView::GetDataColumns()
			{
				return itemSource->GetDataColumns();
			}

			list::ListViewColumns& GuiBindableListView::GetColumns()
			{
				return itemSource->GetColumns();
			}

			Ptr<description::IValueEnumerable> GuiBindableListView::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableListView::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableListView::GetLargeImageProperty()
			{
				return itemSource->largeImageProperty;
			}

			void GuiBindableListView::SetLargeImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->largeImageProperty != value)
				{
					itemSource->largeImageProperty = value;
					itemSource->UpdateBindingProperties();
					LargeImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableListView::GetSmallImageProperty()
			{
				return itemSource->smallImageProperty;
			}

			void GuiBindableListView::SetSmallImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->smallImageProperty != value)
				{
					itemSource->smallImageProperty = value;
					itemSource->UpdateBindingProperties();
					SmallImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableListView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();
				return itemSource->Get(index);
			}

/***********************************************************************
GuiBindableTreeView::ItemSourceNode
***********************************************************************/

			Ptr<description::IValueReadonlyList> GuiBindableTreeView::ItemSourceNode::PrepareValueList(const description::Value& inputItemSource)
			{
				if (auto value = ReadProperty(inputItemSource, rootProvider->childrenProperty))
				{
					if (auto ol = value.Cast<IValueObservableList>())
					{
						return ol;
					}
					else if (auto rl = value.Cast<IValueReadonlyList>())
					{
						return rl;
					}
					else
					{
						return IValueList::Create(GetLazyList<Value>(value));
					}
				}
				else
				{
					return IValueList::Create();
				}
			}

			void GuiBindableTreeView::ItemSourceNode::PrepareChildren(Ptr<description::IValueReadonlyList> newValueList)
			{
				if (!childrenVirtualList)
				{
					childrenVirtualList = newValueList;
					if (auto ol = childrenVirtualList.Cast<IValueObservableList>())
					{
						itemChangedEventHandler = ol->ItemChanged.Add([this](vint start, vint oldCount, vint newCount)
						{
							callback->OnBeforeItemModified(this, start, oldCount, newCount);
							children.RemoveRange(start, oldCount);
							for (vint i = 0; i < newCount; i++)
							{
								Value value = childrenVirtualList->Get(start + i);
								auto node = new ItemSourceNode(value, this);
								children.Insert(start + i, node);
							}
							callback->OnAfterItemModified(this, start, oldCount, newCount);
						});
					}

					vint count = childrenVirtualList->GetCount();
					for (vint i = 0; i < count; i++)
					{
						Value value = childrenVirtualList->Get(i);
						auto node = new ItemSourceNode(value, this);
						children.Add(node);
					}
				}
			}

			void GuiBindableTreeView::ItemSourceNode::UnprepareChildren()
			{
				if (itemChangedEventHandler)
				{
					auto ol = childrenVirtualList.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
					itemChangedEventHandler = nullptr;
				}
				childrenVirtualList = nullptr;
				FOREACH(Ptr<ItemSourceNode>, node, children)
				{
					node->UnprepareChildren();
				}
				children.Clear();
			}

			GuiBindableTreeView::ItemSourceNode::ItemSourceNode(const description::Value& _itemSource, ItemSourceNode* _parent)
				:itemSource(_itemSource)
				, rootProvider(_parent->rootProvider)
				, parent(_parent)
				, callback(_parent->callback)
			{
			}

			GuiBindableTreeView::ItemSourceNode::ItemSourceNode(ItemSource* _rootProvider)
				:rootProvider(_rootProvider)
				, parent(nullptr)
				, callback(_rootProvider)
			{
			}

			GuiBindableTreeView::ItemSourceNode::~ItemSourceNode()
			{
			}

			description::Value GuiBindableTreeView::ItemSourceNode::GetItemSource()
			{
				return itemSource;
			}

			void GuiBindableTreeView::ItemSourceNode::SetItemSource(const description::Value& _itemSource)
			{
				auto newVirtualList = PrepareValueList(_itemSource);
				vint oldCount = childrenVirtualList ? childrenVirtualList->GetCount() : 0;
				vint newCount = newVirtualList->GetCount();

				callback->OnBeforeItemModified(this, 0, oldCount, newCount);
				UnprepareChildren();
				itemSource = _itemSource;
				PrepareChildren(newVirtualList);
				callback->OnAfterItemModified(this, 0, oldCount, newCount);
			}

			bool GuiBindableTreeView::ItemSourceNode::GetExpanding()
			{
				return this == rootProvider->rootNode.Obj() ? true : expanding;
			}

			void GuiBindableTreeView::ItemSourceNode::SetExpanding(bool value)
			{
				if (this != rootProvider->rootNode.Obj() && expanding != value)
				{
					expanding = value;
					if (expanding)
					{
						callback->OnItemExpanded(this);
					}
					else
					{
						callback->OnItemCollapsed(this);
					}
				}
			}

			vint GuiBindableTreeView::ItemSourceNode::CalculateTotalVisibleNodes()
			{
				if (!GetExpanding())
				{
					return 1;
				}

				if (!childrenVirtualList)
				{
					PrepareChildren(PrepareValueList(itemSource));
				}
				vint count = 1;
				FOREACH(Ptr<ItemSourceNode>, child, children)
				{
					count += child->CalculateTotalVisibleNodes();
				}
				return count;
			}

			vint GuiBindableTreeView::ItemSourceNode::GetChildCount()
			{
				if (!childrenVirtualList)
				{
					PrepareChildren(PrepareValueList(itemSource));
				}
				return children.Count();
			}

			Ptr<tree::INodeProvider> GuiBindableTreeView::ItemSourceNode::GetParent()
			{
				return parent;
			}

			Ptr<tree::INodeProvider> GuiBindableTreeView::ItemSourceNode::GetChild(vint index)
			{
				if (!childrenVirtualList)
				{
					PrepareChildren(PrepareValueList(itemSource));
				}
				if (0 <= index && index < children.Count())
				{
					return children[index];
				}
				return nullptr;
			}

/***********************************************************************
GuiBindableTreeView::ItemSource
***********************************************************************/

			GuiBindableTreeView::ItemSource::ItemSource()
			{
				rootNode = new ItemSourceNode(this);
			}

			GuiBindableTreeView::ItemSource::~ItemSource()
			{
			}

			description::Value GuiBindableTreeView::ItemSource::GetItemSource()
			{
				return rootNode->GetItemSource();
			}

			void GuiBindableTreeView::ItemSource::SetItemSource(const description::Value& _itemSource)
			{
				rootNode->SetItemSource(_itemSource);
			}

			void GuiBindableTreeView::ItemSource::UpdateBindingProperties(bool updateChildrenProperty)
			{
				vint oldCount = rootNode->GetChildCount();
				if (updateChildrenProperty)
				{
					rootNode->UnprepareChildren();
				}
				vint newCount = rootNode->GetChildCount();
				OnBeforeItemModified(rootNode.Obj(), 0, oldCount, newCount);
				OnAfterItemModified(rootNode.Obj(), 0, oldCount, newCount);
			}

			// ===================== tree::INodeRootProvider =====================

			Ptr<tree::INodeProvider> GuiBindableTreeView::ItemSource::GetRootNode()
			{
				return rootNode;
			}

			WString GuiBindableTreeView::ItemSource::GetTextValue(tree::INodeProvider* node)
			{
				return ReadProperty(GetBindingValue(node), textProperty);
			}

			description::Value GuiBindableTreeView::ItemSource::GetBindingValue(tree::INodeProvider* node)
			{
				if (auto itemSourceNode = dynamic_cast<ItemSourceNode*>(node))
				{
					return itemSourceNode->GetItemSource();
				}
				return Value();
			}

			IDescriptable* GuiBindableTreeView::ItemSource::RequestView(const WString& identifier)
			{
				if(identifier==ITreeViewItemView::Identifier)
				{
					return (ITreeViewItemView*)this;
				}
				else
				{
					return 0;
				}
			}

			// ===================== tree::ITreeViewItemView =====================

			Ptr<GuiImageData> GuiBindableTreeView::ItemSource::GetNodeImage(tree::INodeProvider* node)
			{
				if (auto itemSourceNode = dynamic_cast<ItemSourceNode*>(node))
				{
					return ReadProperty(itemSourceNode->GetItemSource(), imageProperty);
				}
				return nullptr;
			}

/***********************************************************************
GuiBindableTreeView
***********************************************************************/

			GuiBindableTreeView::GuiBindableTreeView(theme::ThemeName themeName)
				:GuiVirtualTreeView(themeName, new ItemSource)
			{
				itemSource = dynamic_cast<ItemSource*>(GetNodeRootProvider());

				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
				ImagePropertyChanged.SetAssociatedComposition(boundsComposition);
				ChildrenPropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableTreeView::~GuiBindableTreeView()
			{
			}

			description::Value GuiBindableTreeView::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableTreeView::SetItemSource(description::Value _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}

			ItemProperty<WString> GuiBindableTreeView::GetTextProperty()
			{
				return itemSource->textProperty;
			}

			void GuiBindableTreeView::SetTextProperty(const ItemProperty<WString>& value)
			{
				if (itemSource->textProperty != value)
				{
					itemSource->textProperty = value;
					itemSource->UpdateBindingProperties(false);
					TextPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableTreeView::GetImageProperty()
			{
				return itemSource->imageProperty;
			}

			void GuiBindableTreeView::SetImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->imageProperty != value)
				{
					itemSource->imageProperty = value;
					itemSource->UpdateBindingProperties(false);
					ImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<IValueEnumerable>> GuiBindableTreeView::GetChildrenProperty()
			{
				return itemSource->childrenProperty;
			}

			void GuiBindableTreeView::SetChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value)
			{
				if (itemSource->childrenProperty != value)
				{
					itemSource->childrenProperty = value;
					itemSource->UpdateBindingProperties(true);
					ChildrenPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableTreeView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();

				Value result;
				if (auto node = nodeItemView->RequestNode(index))
				{
					if (auto itemSourceNode = node.Cast<ItemSourceNode>())
					{
						result = itemSourceNode->GetItemSource();
					}
				}
				return result;
			}
		}
	}
}
