#include "ItemProvider_Binding.h"

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

/***********************************************************************
TextItemBindableProvider
***********************************************************************/

			TextItemBindableProvider::TextItemBindableProvider()
			{
			}

			TextItemBindableProvider::~TextItemBindableProvider()
			{
				if (itemChangedEventHandler)
				{
					auto ol = itemSource.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
				}
			}

			Ptr<description::IValueEnumerable> TextItemBindableProvider::GetItemSource()
			{
				return itemSource;
			}

			void TextItemBindableProvider::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
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
							InvokeOnItemModified(start, oldCount, newCount, true);
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

				InvokeOnItemModified(0, oldCount, itemSource ? itemSource->GetCount() : 0, true);
			}

			description::Value TextItemBindableProvider::Get(vint index)
			{
				if (!itemSource) return Value();
				return itemSource->Get(index);
			}

			void TextItemBindableProvider::UpdateBindingProperties()
			{
				InvokeOnItemModified(0, Count(), Count(), false);
			}

			bool TextItemBindableProvider::NotifyUpdate(vint start, vint count, bool itemReferenceUpdated)
			{
				if (!itemSource) return false;
				if (start<0 || start >= itemSource->GetCount() || count <= 0 || start + count > itemSource->GetCount())
				{
					return false;
				}
				else
				{
					InvokeOnItemModified(start, count, count, itemReferenceUpdated);
					return true;
				}
			}

			// ===================== GuiListControl::IItemProvider =====================
			
			vint TextItemBindableProvider::Count()
			{
				if (!itemSource) return 0;
				return itemSource->GetCount();
			}

			WString TextItemBindableProvider::GetTextValue(vint itemIndex)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::TextItemBindableProvider::GetTextValue(vint)#"
				CHECK_ERROR(itemSource, ERROR_MESSAGE_PREFIX L"ItemSource is not set.");
				return ReadProperty(itemSource->Get(itemIndex), textProperty);
#undef ERROR_MESSAGE_PREFIX
			}
			
			IDescriptable* TextItemBindableProvider::RequestView(const WString& identifier)
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

			description::Value TextItemBindableProvider::GetBindingValue(vint itemIndex)
			{
				return itemSource->Get(itemIndex);
			}
					
			// ===================== list::TextItemStyleProvider::ITextItemView =====================
			
			bool TextItemBindableProvider::GetChecked(vint itemIndex)
			{
				return ReadProperty(itemSource->Get(itemIndex), checkedProperty);
			}
			
			void TextItemBindableProvider::SetChecked(vint itemIndex, bool value)
			{
				auto thisValue = itemSource->Get(itemIndex);
				WriteProperty(thisValue, checkedProperty, value);
				InvokeOnItemModified(itemIndex, 1, 1, false);
			}

/***********************************************************************
ListViewItemBindableProvider
***********************************************************************/

			ListViewItemBindableProvider::ListViewItemBindableProvider()
				:columns(this)
				, dataColumns(this)
			{
			}

			ListViewItemBindableProvider::~ListViewItemBindableProvider()
			{
				if (itemChangedEventHandler)
				{
					auto ol = itemSource.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
				}
			}

			Ptr<description::IValueEnumerable> ListViewItemBindableProvider::GetItemSource()
			{
				return itemSource;
			}

			void ListViewItemBindableProvider::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
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
							InvokeOnItemModified(start, oldCount, newCount, true);
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

				InvokeOnItemModified(0, oldCount, itemSource ? itemSource->GetCount() : 0, true);
			}

			description::Value ListViewItemBindableProvider::Get(vint index)
			{
				if (!itemSource) return Value();
				return itemSource->Get(index);
			}

			void ListViewItemBindableProvider::UpdateBindingProperties()
			{
				InvokeOnItemModified(0, Count(), Count(), false);
			}

			bool ListViewItemBindableProvider::NotifyUpdate(vint start, vint count, bool itemReferenceUpdated)
			{
				if (!itemSource) return false;
				if (start<0 || start >= itemSource->GetCount() || count <= 0 || start + count > itemSource->GetCount())
				{
					return false;
				}
				else
				{
					InvokeOnItemModified(start, count, count, itemReferenceUpdated);
					return true;
				}
			}

			list::ListViewDataColumns& ListViewItemBindableProvider::GetDataColumns()
			{
				return dataColumns;
			}

			list::ListViewColumns& ListViewItemBindableProvider::GetColumns()
			{
				return columns;
			}
					
			// ===================== list::IListViewItemProvider =====================

			void ListViewItemBindableProvider::RebuildAllItems()
			{
				InvokeOnItemModified(0, Count(), Count(), true);
			}

			void ListViewItemBindableProvider::RefreshAllItems()
			{
				InvokeOnItemModified(0, Count(), Count(), false);
			}

			void ListViewItemBindableProvider::NotifyColumnRebuilt()
			{
				for (auto callback : columnItemViewCallbacks)
				{
					callback->OnColumnRebuilt();
				}
				RebuildAllItems();
			}

			void ListViewItemBindableProvider::NotifyColumnChanged()
			{
				for (auto callback : columnItemViewCallbacks)
				{
					callback->OnColumnChanged(true);
				}
				RefreshAllItems();
			}

			// ===================== GuiListControl::IItemProvider =====================

			vint ListViewItemBindableProvider::Count()
			{
				if (!itemSource) return 0;
				return itemSource->GetCount();
			}

			WString ListViewItemBindableProvider::GetTextValue(vint itemIndex)
			{
				return GetText(itemIndex);
			}

			description::Value ListViewItemBindableProvider::GetBindingValue(vint itemIndex)
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

			IDescriptable* ListViewItemBindableProvider::RequestView(const WString& identifier)
			{
				if (identifier == IListViewItemView::Identifier)
				{
					return (IListViewItemView*)this;
				}
				else if (identifier == IColumnItemView::Identifier)
				{
					return (IColumnItemView*)this;
				}
				else
				{
					return 0;
				}
			}

			// ===================== list::ListViewItemStyleProvider::IListViewItemView =====================

			Ptr<GuiImageData> ListViewItemBindableProvider::GetSmallImage(vint itemIndex)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::ListViewItemBindableProvider::GetSmallImage(vint)#"
				CHECK_ERROR(itemSource, ERROR_MESSAGE_PREFIX L"ItemSource is not set.");
				return ReadProperty(itemSource->Get(itemIndex), smallImageProperty);
#undef ERROR_MESSAGE_PREFIX
			}

			Ptr<GuiImageData> ListViewItemBindableProvider::GetLargeImage(vint itemIndex)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::ListViewItemBindableProvider::GetLargeImage(vint)#"
				CHECK_ERROR(itemSource, ERROR_MESSAGE_PREFIX L"ItemSource is not set.");
				return ReadProperty(itemSource->Get(itemIndex), largeImageProperty);
#undef ERROR_MESSAGE_PREFIX
			}

			WString ListViewItemBindableProvider::GetText(vint itemIndex)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::ListViewItemBindableProvider::GetText(vint)#"
				CHECK_ERROR(itemSource, ERROR_MESSAGE_PREFIX L"ItemSource is not set.");
				return ReadProperty(itemSource->Get(itemIndex), columns[0]->GetTextProperty());
#undef ERROR_MESSAGE_PREFIX
			}

			WString ListViewItemBindableProvider::GetSubItem(vint itemIndex, vint index)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::ListViewItemBindableProvider::GetSubItem(vint, vint)#"
				CHECK_ERROR(itemSource, ERROR_MESSAGE_PREFIX L"ItemSource is not set.");
				CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"column index cannot be -1, use GetText(itemIndex) instead.");
				return ReadProperty(itemSource->Get(itemIndex), columns[index + 1]->GetTextProperty());
#undef ERROR_MESSAGE_PREFIX
			}

			vint ListViewItemBindableProvider::GetDataColumnCount()
			{
				return dataColumns.Count();
			}

			vint ListViewItemBindableProvider::GetDataColumn(vint index)
			{
				return dataColumns[index];
			}

			vint ListViewItemBindableProvider::GetColumnCount()
			{
				return columns.Count();
			}

			WString ListViewItemBindableProvider::GetColumnText(vint index)
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

			bool ListViewItemBindableProvider::AttachCallback(IColumnItemViewCallback* value)
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

			bool ListViewItemBindableProvider::DetachCallback(IColumnItemViewCallback* value)
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

			vint ListViewItemBindableProvider::GetColumnSize(vint index)
			{
				return columns[index]->GetSize();
			}

			void ListViewItemBindableProvider::SetColumnSize(vint index, vint value)
			{
				columns[index]->SetSize(value);
			}

			GuiMenu* ListViewItemBindableProvider::GetDropdownPopup(vint index)
			{
				return columns[index]->GetDropdownPopup();
			}

			ColumnSortingState ListViewItemBindableProvider::GetSortingState(vint index)
			{
				return columns[index]->GetSortingState();
			}

/***********************************************************************
TreeViewItemBindableNode
***********************************************************************/

			Ptr<description::IValueReadonlyList> TreeViewItemBindableNode::PrepareValueList(const description::Value& inputItemSource)
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

			void TreeViewItemBindableNode::PrepareChildren(Ptr<description::IValueReadonlyList> newValueList)
			{
				if (!childrenVirtualList)
				{
					childrenVirtualList = newValueList;
					if (auto ol = childrenVirtualList.Cast<IValueObservableList>())
					{
						itemChangedEventHandler = ol->ItemChanged.Add([this](vint start, vint oldCount, vint newCount)
						{
							callback->OnBeforeItemModified(this, start, oldCount, newCount, true);
							children.RemoveRange(start, oldCount);
							for (vint i = 0; i < newCount; i++)
							{
								Value value = childrenVirtualList->Get(start + i);
								auto node = Ptr(new TreeViewItemBindableNode(value, this));
								children.Insert(start + i, node);
							}
							callback->OnAfterItemModified(this, start, oldCount, newCount, true);
						});
					}

					vint count = childrenVirtualList->GetCount();
					for (vint i = 0; i < count; i++)
					{
						Value value = childrenVirtualList->Get(i);
						auto node = Ptr(new TreeViewItemBindableNode(value, this));
						children.Add(node);
					}
				}
			}

			void TreeViewItemBindableNode::UnprepareChildren()
			{
				if (itemChangedEventHandler)
				{
					auto ol = childrenVirtualList.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
					itemChangedEventHandler = nullptr;
				}
				childrenVirtualList = nullptr;
				for (auto node : children)
				{
					node->UnprepareChildren();
				}
				children.Clear();
			}

			void TreeViewItemBindableNode::PrepareReverseMapping()
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::TreeViewItemBindableNode::PrepareReverseMapping()#"
				if (rootProvider->reverseMappingProperty && !itemSource.IsNull())
				{
					auto oldValue = ReadProperty(itemSource, rootProvider->reverseMappingProperty);
					CHECK_ERROR(oldValue.IsNull(), ERROR_MESSAGE_PREFIX L"The reverse mapping property of an item has been unexpectedly changed.");
					WriteProperty(itemSource, rootProvider->reverseMappingProperty, Value::From(this));
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void TreeViewItemBindableNode::UnprepareReverseMapping()
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::TreeViewItemBindableNode::PrepareReverseMapping()#"
				if (rootProvider->reverseMappingProperty && !itemSource.IsNull())
				{
					auto oldValue = ReadProperty(itemSource, rootProvider->reverseMappingProperty);
					CHECK_ERROR(oldValue.GetRawPtr() == this, ERROR_MESSAGE_PREFIX L"The reverse mapping property of an item has been unexpectedly changed.");
					WriteProperty(itemSource, rootProvider->reverseMappingProperty, {});
				}
#undef ERROR_MESSAGE_PREFIX
			}

			TreeViewItemBindableNode::TreeViewItemBindableNode(const description::Value& _itemSource, TreeViewItemBindableNode* _parent)
				:itemSource(_itemSource)
				, rootProvider(_parent->rootProvider)
				, parent(_parent)
				, callback(_parent->callback)
			{
				PrepareReverseMapping();
			}

			TreeViewItemBindableNode::TreeViewItemBindableNode(TreeViewItemBindableRootProvider* _rootProvider)
				:rootProvider(_rootProvider)
				, parent(nullptr)
				, callback(_rootProvider)
			{
			}

			TreeViewItemBindableNode::~TreeViewItemBindableNode()
			{
				UnprepareReverseMapping();
				if (itemChangedEventHandler)
				{
					auto ol = childrenVirtualList.Cast<IValueObservableList>();
					ol->ItemChanged.Remove(itemChangedEventHandler);
				}
			}

			description::Value TreeViewItemBindableNode::GetItemSource()
			{
				return itemSource;
			}

			void TreeViewItemBindableNode::SetItemSource(const description::Value& _itemSource)
			{
				auto newVirtualList = PrepareValueList(_itemSource);
				vint oldCount = childrenVirtualList ? childrenVirtualList->GetCount() : 0;
				vint newCount = newVirtualList->GetCount();

				callback->OnBeforeItemModified(this, 0, oldCount, newCount, true);
				UnprepareChildren();
				UnprepareReverseMapping();
				itemSource = _itemSource;
				PrepareReverseMapping();
				PrepareChildren(newVirtualList);
				callback->OnAfterItemModified(this, 0, oldCount, newCount, true);
			}

			bool TreeViewItemBindableNode::GetExpanding()
			{
				return this == rootProvider->rootNode.Obj() ? true : expanding;
			}

			void TreeViewItemBindableNode::SetExpanding(bool value)
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

			vint TreeViewItemBindableNode::CalculateTotalVisibleNodes()
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
				for (auto child : children)
				{
					count += child->CalculateTotalVisibleNodes();
				}
				return count;
			}

			void TreeViewItemBindableNode::NotifyDataModified()
			{
				if (parent)
				{
					vint index = parent->children.IndexOf(this);
					callback->OnBeforeItemModified(parent, index, 1, 1, false);
					callback->OnAfterItemModified(parent, index, 1, 1, false);
				}
			}

			vint TreeViewItemBindableNode::GetChildCount()
			{
				if (!childrenVirtualList)
				{
					PrepareChildren(PrepareValueList(itemSource));
				}
				return children.Count();
			}

			Ptr<tree::INodeProvider> TreeViewItemBindableNode::GetParent()
			{
				return Ptr(parent);
			}

			Ptr<tree::INodeProvider> TreeViewItemBindableNode::GetChild(vint index)
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
TreeViewItemBindableRootProvider
***********************************************************************/

			TreeViewItemBindableRootProvider::TreeViewItemBindableRootProvider()
			{
				rootNode = Ptr(new TreeViewItemBindableNode(this));
			}

			TreeViewItemBindableRootProvider::~TreeViewItemBindableRootProvider()
			{
			}

			description::Value TreeViewItemBindableRootProvider::GetItemSource()
			{
				return rootNode->GetItemSource();
			}

			void TreeViewItemBindableRootProvider::SetItemSource(const description::Value& _itemSource)
			{
				rootNode->SetItemSource(_itemSource);
			}

			void TreeViewItemBindableRootProvider::UpdateBindingProperties(bool updateChildrenProperty)
			{
				vint oldCount = rootNode->GetChildCount();
				if (updateChildrenProperty)
				{
					rootNode->UnprepareChildren();
				}
				vint newCount = rootNode->GetChildCount();
				OnBeforeItemModified(rootNode.Obj(), 0, oldCount, newCount, updateChildrenProperty);
				OnAfterItemModified(rootNode.Obj(), 0, oldCount, newCount, updateChildrenProperty);
			}

			// ===================== tree::INodeRootProvider =====================

			Ptr<tree::INodeProvider> TreeViewItemBindableRootProvider::GetRootNode()
			{
				return rootNode;
			}

			WString TreeViewItemBindableRootProvider::GetTextValue(tree::INodeProvider* node)
			{
				return ReadProperty(GetBindingValue(node), textProperty);
			}

			description::Value TreeViewItemBindableRootProvider::GetBindingValue(tree::INodeProvider* node)
			{
				if (auto itemSourceNode = dynamic_cast<TreeViewItemBindableNode*>(node))
				{
					return itemSourceNode->GetItemSource();
				}
				return Value();
			}

			IDescriptable* TreeViewItemBindableRootProvider::RequestView(const WString& identifier)
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

			Ptr<GuiImageData> TreeViewItemBindableRootProvider::GetNodeImage(tree::INodeProvider* node)
			{
				if (auto itemSourceNode = dynamic_cast<TreeViewItemBindableNode*>(node))
				{
					return ReadProperty(itemSourceNode->GetItemSource(), imageProperty);
				}
				return nullptr;
			}
		}
	}
}
