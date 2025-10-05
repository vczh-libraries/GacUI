#include "ItemProvider_IListViewItemView.h"

namespace vl::presentation::controls::list
{
	using namespace collections;
	using namespace reflection::description;

	const wchar_t* const IListViewItemView::Identifier = L"vl::presentation::controls::list::IListViewItemView";
	const wchar_t* const IColumnItemView::Identifier = L"vl::presentation::controls::list::IColumnItemView";

/***********************************************************************
ListViewSubItems
***********************************************************************/

	void ListViewSubItems::NotifyUpdateInternal(vint start, vint count, vint newCount)
	{
		owner->NotifyUpdate();
	}

/***********************************************************************
ListViewItem
***********************************************************************/

	void ListViewItem::NotifyUpdate()
	{
		if (owner)
		{
			vint index = owner->IndexOf(this);
			owner->InvokeOnItemModified(index, 1, 1, false);
		}
	}

	ListViewItem::ListViewItem()
		:owner(0)
	{
		subItems.owner = this;
	}

	ListViewSubItems& ListViewItem::GetSubItems()
	{
		return subItems;
	}

	Ptr<GuiImageData> ListViewItem::GetSmallImage()
	{
		return smallImage;
	}

	void ListViewItem::SetSmallImage(Ptr<GuiImageData> value)
	{
		smallImage = value;
		NotifyUpdate();
	}

	Ptr<GuiImageData> ListViewItem::GetLargeImage()
	{
		return largeImage;
	}
	
	void ListViewItem::SetLargeImage(Ptr<GuiImageData> value)
	{
		largeImage = value;
		NotifyUpdate();
	}

	const WString& ListViewItem::GetText()
	{
		return text;
	}

	void ListViewItem::SetText(const WString& value)
	{
		text = value;
		NotifyUpdate();
	}

	description::Value ListViewItem::GetTag()
	{
		return tag;
	}

	void ListViewItem::SetTag(const description::Value& value)
	{
		tag = value;
		NotifyUpdate();
	}

/***********************************************************************
ListViewColumn
***********************************************************************/

	void ListViewColumn::NotifyRebuilt()
	{
		if (owner)
		{
			vint index = owner->IndexOf(this);
			if (index != -1)
			{
				owner->NotifyColumnRebuilt(index);
			}
		}
	}

	void ListViewColumn::NotifyChanged(bool needToRefreshItems)
	{
		if (owner)
		{
			vint index = owner->IndexOf(this);
			if (index != -1)
			{
				owner->NotifyColumnChanged(index, needToRefreshItems);
			}
		}
	}

	ListViewColumn::ListViewColumn(const WString& _text, vint _size)
		:text(_text)
		,size(_size)
	{
	}

	ListViewColumn::~ListViewColumn()
	{
		if (dropdownPopup && ownPopup)
		{
			SafeDeleteControl(dropdownPopup);
		}
	}

	const WString& ListViewColumn::GetText()
	{
		return text;
	}

	void ListViewColumn::SetText(const WString& value)
	{
		if (text != value)
		{
			text = value;
			NotifyChanged(false);
		}
	}

	ItemProperty<WString> ListViewColumn::GetTextProperty()
	{
		return textProperty;
	}

	void ListViewColumn::SetTextProperty(const ItemProperty<WString>& value)
	{
		textProperty = value;
		NotifyChanged(true);
	}

	vint ListViewColumn::GetSize()
	{
		return size;
	}

	void ListViewColumn::SetSize(vint value)
	{
		if (size != value)
		{
			size = value;
			NotifyChanged(true);
		}
	}

	bool ListViewColumn::GetOwnPopup()
	{
		return ownPopup;
	}

	void ListViewColumn::SetOwnPopup(bool value)
	{
		ownPopup = value;
	}

	GuiMenu* ListViewColumn::GetDropdownPopup()
	{
		return dropdownPopup;
	}

	void ListViewColumn::SetDropdownPopup(GuiMenu* value)
	{
		if (dropdownPopup != value)
		{
			dropdownPopup = value;
			NotifyChanged(false);
		}
	}

	ColumnSortingState ListViewColumn::GetSortingState()
	{
		return sortingState;
	}

	void ListViewColumn::SetSortingState(ColumnSortingState value)
	{
		if (sortingState != value)
		{
			sortingState = value;
			NotifyChanged(false);
		}
	}

/***********************************************************************
ListViewDataColumns
***********************************************************************/

	void ListViewDataColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
	{
		itemProvider->RefreshAllItems();
	}

	ListViewDataColumns::ListViewDataColumns(IListViewItemProvider* _itemProvider)
		:itemProvider(_itemProvider)
	{
	}

	ListViewDataColumns::~ListViewDataColumns()
	{
	}

/***********************************************************************
ListViewColumns
***********************************************************************/

	void ListViewColumns::NotifyColumnRebuilt(vint column)
	{
		NotifyUpdate(column, 1);
	}

	void ListViewColumns::NotifyColumnChanged(vint column, bool needToRefreshItems)
	{
		itemProvider->NotifyColumnChanged();
	}

	void ListViewColumns::BeforeInsert(vint index, const Ptr<ListViewColumn>& value)
	{
		// Check if this column is already in the provider
		if (value->owner)
		{
			throw ArgumentException(L"The ListViewColumn is already belong to a ListViewColumns.", L"vl::presentation::controls::list::ListViewColumns::BeforeInsert", L"value");
		}
	}

	void ListViewColumns::AfterInsert(vint index, const Ptr<ListViewColumn>& value)
	{
		collections::ObservableListBase<Ptr<ListViewColumn>>::AfterInsert(index, value);
		value->owner = this;
	}

	void ListViewColumns::BeforeRemove(vint index, const Ptr<ListViewColumn>& value)
	{
		value->owner = 0;
		collections::ObservableListBase<Ptr<ListViewColumn>>::BeforeRemove(index, value);
	}

	void ListViewColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
	{
		itemProvider->NotifyColumnRebuilt();
	}

	ListViewColumns::ListViewColumns(IListViewItemProvider* _itemProvider)
		:itemProvider(_itemProvider)
	{
	}

	ListViewColumns::~ListViewColumns()
	{
	}

/***********************************************************************
ListViewItemProvider
***********************************************************************/

	void ListViewItemProvider::BeforeInsert(vint index, const Ptr<ListViewItem>& value)
	{
		// Check if this item is already in the provider
		if (value->owner)
		{
			throw ArgumentException(L"The ListViewItem is already belong to a ListViewItemProvider.", L"vl::presentation::controls::list::ListViewItemProvider::BeforeInsert", L"value");
		}
	}

	void ListViewItemProvider::AfterInsert(vint index, const Ptr<ListViewItem>& value)
	{
		ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
		value->owner = this;
	}

	void ListViewItemProvider::BeforeRemove(vint index, const Ptr<ListViewItem>& value)
	{
		value->owner = 0;
		ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
	}

	void ListViewItemProvider::RebuildAllItems()
	{
		InvokeOnItemModified(0, Count(), Count(), true);
	}

	void ListViewItemProvider::RefreshAllItems()
	{
		InvokeOnItemModified(0, Count(), Count(), false);
	}

	void ListViewItemProvider::NotifyColumnRebuilt()
	{
		for (auto callback : columnItemViewCallbacks)
		{
			callback->OnColumnRebuilt();
		}
		RefreshAllItems();
	}

	void ListViewItemProvider::NotifyColumnChanged()
	{
		for (auto callback : columnItemViewCallbacks)
		{
			callback->OnColumnChanged(true);
		}
		RefreshAllItems();
	}

	ListViewItemProvider::ListViewItemProvider()
		:columns(this)
		, dataColumns(this)
	{
	}

	ListViewItemProvider::~ListViewItemProvider()
	{
	}

	WString ListViewItemProvider::GetTextValue(vint itemIndex)
	{
		return GetText(itemIndex);
	}

	description::Value ListViewItemProvider::GetBindingValue(vint itemIndex)
	{
		return Value::From(Get(itemIndex));
	}

	IDescriptable* ListViewItemProvider::RequestView(const WString& identifier)
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

	Ptr<GuiImageData> ListViewItemProvider::GetSmallImage(vint itemIndex)
	{
		return Get(itemIndex)->smallImage;
	}

	Ptr<GuiImageData> ListViewItemProvider::GetLargeImage(vint itemIndex)
	{
		return Get(itemIndex)->largeImage;
	}

	WString ListViewItemProvider::GetText(vint itemIndex)
	{
		return Get(itemIndex)->text;
	}

	WString ListViewItemProvider::GetSubItem(vint itemIndex, vint index)
	{
#define ERROR_MESSAGE_PREFIX L"ListViewItemProvider::GetSubItem(vint, vint)#"
		CHECK_ERROR(0 <= index && index < columns.Count() - 1, ERROR_MESSAGE_PREFIX L"Sub item index out of range.");
		Ptr<ListViewItem> item = Get(itemIndex);
		if (index < 0 || index >= item->GetSubItems().Count())
		{
			return WString::Empty;
		}
		else
		{
			return item->GetSubItems()[index];
		}
#undef ERROR_MESSAGE_PREFIX
	}

	vint ListViewItemProvider::GetDataColumnCount()
	{
		return dataColumns.Count();
	}

	vint ListViewItemProvider::GetDataColumn(vint index)
	{
		return dataColumns[index];
	}

	vint ListViewItemProvider::GetColumnCount()
	{
		return columns.Count();
	}

	WString ListViewItemProvider::GetColumnText(vint index)
	{
		if (index<0 || index >= columns.Count())
		{
			return L"";
		}
		else
		{
			return columns[index]->GetText();
		}
	}

	bool ListViewItemProvider::AttachCallback(IColumnItemViewCallback* value)
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

	bool ListViewItemProvider::DetachCallback(IColumnItemViewCallback* value)
	{
		vint index=columnItemViewCallbacks.IndexOf(value);
		if(index==-1)
		{
			return false;
		}
		else
		{
			columnItemViewCallbacks.Remove(value);
			return true;
		}
	}

	vint ListViewItemProvider::GetColumnSize(vint index)
	{
		if(index<0 || index>=columns.Count())
		{
			return 0;
		}
		else
		{
			return columns[index]->GetSize();
		}
	}

	void ListViewItemProvider::SetColumnSize(vint index, vint value)
	{
		if(index>=0 && index<columns.Count())
		{
			columns[index]->SetSize(value);
		}
	}

	GuiMenu* ListViewItemProvider::GetDropdownPopup(vint index)
	{
		if(index<0 || index>=columns.Count())
		{
			return 0;
		}
		else
		{
			return columns[index]->GetDropdownPopup();
		}
	}

	ColumnSortingState ListViewItemProvider::GetSortingState(vint index)
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

	ListViewDataColumns& ListViewItemProvider::GetDataColumns()
	{
		return dataColumns;
	}

	ListViewColumns& ListViewItemProvider::GetColumns()
	{
		return columns;
	}
}