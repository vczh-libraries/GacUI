/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ILISTVIEWITEMVIEW
#define VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ILISTVIEWITEMVIEW

#include "DataSourceImpl_IItemProvider_ItemProviderBase.h"
#include "../ToolstripPackage/GuiMenuControls.h"

namespace vl::presentation::controls::list
{
	class IListViewItemProvider : public virtual Interface
	{
	public:
		virtual void									RebuildAllItems() = 0;
		virtual void									RefreshAllItems() = 0;
		virtual void									NotifyColumnRebuilt() = 0;
		virtual void									NotifyColumnChanged() = 0;
	};

/***********************************************************************
IListViewItemView
***********************************************************************/

	/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for <see cref="GuiVirtualListView"/>.</summary>
	class IListViewItemView : public virtual IDescriptable, public Description<IListViewItemView>
	{
	public:
		/// <summary>The identifier for this view.</summary>
		static const wchar_t* const				Identifier;

		/// <summary>Get the small image of an item.</summary>
		/// <returns>The small image.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		virtual Ptr<GuiImageData>				GetSmallImage(vint itemIndex) = 0;
		/// <summary>Get the large image of an item.</summary>
		/// <returns>The large image.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		virtual Ptr<GuiImageData>				GetLargeImage(vint itemIndex) = 0;
		/// <summary>Get the text of an item.</summary>
		/// <returns>The text.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		virtual WString							GetText(vint itemIndex) = 0;
		/// <summary>Get the sub item text of an item. If the sub item index out of range, it returns an empty string.</summary>
		/// <returns>The sub item text.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		/// <param name="index">The sub item index of the item.</param>
		virtual WString							GetSubItem(vint itemIndex, vint index) = 0;

		/// <summary>Get the number of data columns.</summary>
		/// <returns>The number of data columns.</returns>
		virtual vint											GetDataColumnCount() = 0;
		/// <summary>Get the column index of the index-th data column.</summary>
		/// <returns>The column index.</returns>
		/// <param name="index">The order of the data column.</param>
		virtual vint											GetDataColumn(vint index) = 0;

		/// <summary>Get the number of all columns.</summary>
		/// <returns>The number of all columns.</returns>
		virtual vint											GetColumnCount() = 0;
		/// <summary>Get the text of the column.</summary>
		/// <returns>The text of the column.</returns>
		/// <param name="index">The index of the column.</param>
		virtual WString											GetColumnText(vint index) = 0;
	};

/***********************************************************************
IColumnItemView
***********************************************************************/
					
	/// <summary>Callback for [T:vl.presentation.controls.list.ListViewColumnItemArranger.IColumnItemView]. Column item view use this interface to notify column related modification.</summary>
	class IColumnItemViewCallback : public virtual IDescriptable, public Description<IColumnItemViewCallback>
	{
	public:
		/// <summary>Called when any column object is changed (inserted, removed, updated binding, etc.).</summary>
		virtual void							OnColumnRebuilt()=0;

		/// <summary>Called when any property of a column is changed (size changed, text changed, etc.).</summary>
		virtual void							OnColumnChanged(bool needToRefreshItems)=0;
	};
	
	/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for <see cref="ListViewColumnItemArranger"/>.</summary>
	class IColumnItemView : public virtual IDescriptable, public Description<IColumnItemView>
	{
	public:
		/// <summary>The identifier for this view.</summary>
		static const wchar_t* const								Identifier;
		
		/// <summary>Attach an column item view callback to this view.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The column item view callback.</param>
		virtual bool											AttachCallback(IColumnItemViewCallback* value)=0;
		/// <summary>Detach an column item view callback from this view.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The column item view callback.</param>
		virtual bool											DetachCallback(IColumnItemViewCallback* value)=0;

		/// <summary>Get the size of the column.</summary>
		/// <returns>The size of the column.</returns>
		/// <param name="index">The index of the column.</param>
		virtual vint											GetColumnSize(vint index)=0;
		/// <summary>Set the size of the column.</summary>
		/// <param name="index">The index of the column.</param>
		/// <param name="value">The new size of the column.</param>
		virtual void											SetColumnSize(vint index, vint value)=0;

		/// <summary>Get the popup binded to the column.</summary>
		/// <returns>The popup binded to the column.</returns>
		/// <param name="index">The index of the column.</param>
		virtual GuiMenu*										GetDropdownPopup(vint index)=0;
		/// <summary>Get the sorting state of the column.</summary>
		/// <returns>The sorting state of the column.</returns>
		/// <param name="index">The index of the column.</param>
		virtual ColumnSortingState								GetSortingState(vint index)=0;
	};

/***********************************************************************
ListViewItem
***********************************************************************/

	class ListViewItem;

	class ListViewSubItems : public collections::ObservableListBase<WString>
	{
		friend class ListViewItem;
	protected:
		ListViewItem*									owner;
		
		void											NotifyUpdateInternal(vint start, vint count, vint newCount)override;
	public:
	};

	class ListViewItemProvider;

	/// <summary>List view item.</summary>
	class ListViewItem : public Object, public Description<ListViewItem>
	{
		friend class ListViewSubItems;
		friend class ListViewItemProvider;
	protected:
		ListViewItemProvider*							owner;
		ListViewSubItems								subItems;
		Ptr<GuiImageData>								smallImage;
		Ptr<GuiImageData>								largeImage;
		WString											text;
		description::Value								tag;
		
		void											NotifyUpdate();
	public:
		/// <summary>Create a list view item.</summary>
		ListViewItem();
		
		/// <summary>Get all sub items of this item.</summary>
		/// <returns>All sub items of this item.</returns>
		ListViewSubItems&								GetSubItems();
		/// <summary>Get the small image of this item.</summary>
		/// <returns>The small image of this item.</returns>
		Ptr<GuiImageData>								GetSmallImage();
		/// <summary>Set the small image of this item.</summary>
		/// <param name="value">The small image of this item.</param>
		void											SetSmallImage(Ptr<GuiImageData> value);
		/// <summary>Get the large image of this item.</summary>
		/// <returns>The large image of this item.</returns>
		Ptr<GuiImageData>								GetLargeImage();
		/// <summary>Set the large image of this item.</summary>
		/// <param name="value">The large image of this item.</param>
		void											SetLargeImage(Ptr<GuiImageData> value);
		/// <summary>Get the text of this item.</summary>
		/// <returns>The text of this item.</returns>
		const WString&									GetText();
		/// <summary>Set the text of this item.</summary>
		/// <param name="value">The text of this item.</param>
		void											SetText(const WString& value);
		/// <summary>Get the tag of this item.</summary>
		/// <returns>The tag of this item.</returns>
		description::Value								GetTag();
		/// <summary>Set the tag of this item.</summary>
		/// <param name="value">The tag of this item.</param>
		void											SetTag(const description::Value& value);
	};

/***********************************************************************
ListViewColumn
***********************************************************************/

	class ListViewColumns;
	
	/// <summary>List view column.</summary>
	class ListViewColumn : public Object, public Description<ListViewColumn>
	{
		friend class ListViewColumns;
	protected:
		ListViewColumns*								owner = nullptr;
		WString											text;
		ItemProperty<WString>							textProperty;
		vint											size;
		bool											ownPopup = true;
		GuiMenu*										dropdownPopup = nullptr;
		ColumnSortingState								sortingState = ColumnSortingState::NotSorted;
		
		void											NotifyRebuilt();
		void											NotifyChanged(bool needToRefreshItems);
	public:
		/// <summary>Create a column with the specified text and size.</summary>
		/// <param name="_text">The specified text.</param>
		/// <param name="_size">The specified size.</param>
		ListViewColumn(const WString& _text=L"", vint _size=160);
		~ListViewColumn();
		
		/// <summary>Get the text of this item.</summary>
		/// <returns>The text of this item.</returns>
		const WString&									GetText();
		/// <summary>Set the text of this item.</summary>
		/// <param name="value">The text of this item.</param>
		void											SetText(const WString& value);
		/// <summary>Get the text property of this item.</summary>
		/// <returns>The text property of this item.</returns>
		ItemProperty<WString>							GetTextProperty();
		/// <summary>Set the text property of this item.</summary>
		/// <param name="value">The text property of this item.</param>
		void											SetTextProperty(const ItemProperty<WString>& value);
		/// <summary>Get the size of this item.</summary>
		/// <returns>The size of this item.</returns>
		vint											GetSize();
		/// <summary>Set the size of this item.</summary>
		/// <param name="value">The size of this item.</param>
		void											SetSize(vint value);
		/// <summary>Test if the column owns the popup. Owned popup will be deleted in the destructor.</summary>
		/// <returns>Returns true if the column owns the popup.</returns>
		bool											GetOwnPopup();
		/// <summary>Set if the column owns the popup.</summary>
		/// <param name="value">Set to true to let the column own the popup.</param>
		void											SetOwnPopup(bool value);
		/// <summary>Get the dropdown context menu of this item.</summary>
		/// <returns>The dropdown context menu of this item.</returns>
		GuiMenu*										GetDropdownPopup();
		/// <summary>Set the dropdown context menu of this item.</summary>
		/// <param name="value">The dropdown context menu of this item.</param>
		void											SetDropdownPopup(GuiMenu* value);
		/// <summary>Get the sorting state of this item.</summary>
		/// <returns>The sorting state of this item.</returns>
		ColumnSortingState								GetSortingState();
		/// <summary>Set the sorting state of this item.</summary>
		/// <param name="value">The sorting state of this item.</param>
		void											SetSortingState(ColumnSortingState value);
	};

/***********************************************************************
ListViewDataColumns
***********************************************************************/

	/// <summary>List view data column container.</summary>
	class ListViewDataColumns : public collections::ObservableListBase<vint>
	{
	protected:
		IListViewItemProvider*							itemProvider;

		void											NotifyUpdateInternal(vint start, vint count, vint newCount)override;
	public:
		/// <summary>Create a container.</summary>
		/// <param name="_itemProvider">The item provider in the same control to receive notifications.</param>
		ListViewDataColumns(IListViewItemProvider* _itemProvider);
		~ListViewDataColumns();
	};

/***********************************************************************
ListViewColumns
***********************************************************************/
	
	/// <summary>List view column container.</summary>
	class ListViewColumns : public collections::ObservableListBase<Ptr<ListViewColumn>>
	{
		friend class ListViewColumn;
	protected:
		IListViewItemProvider*							itemProvider;
		bool											affectItemFlag = true;

		void											NotifyColumnRebuilt(vint column);
		void											NotifyColumnChanged(vint column, bool needToRefreshItems);
		void											BeforeInsert(vint index, const Ptr<ListViewColumn>& value)override;
		void											AfterInsert(vint index, const Ptr<ListViewColumn>& value)override;
		void											BeforeRemove(vint index, const Ptr<ListViewColumn>& value)override;
		void											NotifyUpdateInternal(vint start, vint count, vint newCount)override;
	public:
		/// <summary>Create a container.</summary>
		/// <param name="_itemProvider">The item provider in the same control to receive notifications.</param>
		ListViewColumns(IListViewItemProvider* _itemProvider);
		~ListViewColumns();
	};

/***********************************************************************
ListViewItemProvider
***********************************************************************/
				
	/// <summary>Item provider for <see cref="GuiListViewBase"/>.</summary>
	class ListViewItemProvider
		: public ListProvider<Ptr<ListViewItem>>
		, protected virtual IListViewItemProvider
		, public virtual IListViewItemView
		, public virtual IColumnItemView
		, public Description<ListViewItemProvider>
	{
		friend class ListViewItem;
		typedef collections::List<IColumnItemViewCallback*>		ColumnItemViewCallbackList;
	protected:
		ListViewDataColumns									dataColumns;
		ListViewColumns										columns;
		ColumnItemViewCallbackList							columnItemViewCallbacks;

		void												BeforeInsert(vint index, const Ptr<ListViewItem>& value)override;
		void												AfterInsert(vint index, const Ptr<ListViewItem>& value)override;
		void												BeforeRemove(vint index, const Ptr<ListViewItem>& value)override;

		// ===================== list::IListViewItemProvider =====================

		void												RebuildAllItems() override;
		void												RefreshAllItems() override;
		void												NotifyColumnRebuilt() override;
		void												NotifyColumnChanged() override;

	public:
		ListViewItemProvider();
		~ListViewItemProvider();

		// ===================== GuiListControl::IItemProvider =====================

		WString												GetTextValue(vint itemIndex)override;
		description::Value									GetBindingValue(vint itemIndex)override;
		IDescriptable*										RequestView(const WString& identifier)override;

		// ===================== list::ListViewItemStyleProvider::IListViewItemView =====================

		Ptr<GuiImageData>									GetSmallImage(vint itemIndex)override;
		Ptr<GuiImageData>									GetLargeImage(vint itemIndex)override;
		WString												GetText(vint itemIndex)override;
		WString												GetSubItem(vint itemIndex, vint index)override;
		vint												GetDataColumnCount()override;
		vint												GetDataColumn(vint index)override;
		vint												GetColumnCount()override;
		WString												GetColumnText(vint index)override;

		// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================

		bool												AttachCallback(IColumnItemViewCallback* value)override;
		bool												DetachCallback(IColumnItemViewCallback* value)override;
		vint												GetColumnSize(vint index)override;
		void												SetColumnSize(vint index, vint value)override;
		GuiMenu*											GetDropdownPopup(vint index)override;
		ColumnSortingState									GetSortingState(vint index)override;

		/// <summary>Get all data columns indices in columns.</summary>
		/// <returns>All data columns indices in columns.</returns>
		ListViewDataColumns&								GetDataColumns();
		/// <summary>Get all columns.</summary>
		/// <returns>All columns.</returns>
		ListViewColumns&									GetColumns();
	};
}

#endif