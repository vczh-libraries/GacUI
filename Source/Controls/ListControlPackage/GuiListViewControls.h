/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTVIEWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUILISTVIEWCONTROLS

#include "GuiListControlItemArrangers.h"
#include "../ToolstripPackage/GuiMenuControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			///<summary>List view column header control for detailed view.</summary>
			class GuiListViewColumnHeader : public GuiMenuButton, public Description<GuiListViewColumnHeader>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ListViewColumnHeaderTemplate, GuiMenuButton)
			protected:
				ColumnSortingState								columnSortingState = ColumnSortingState::NotSorted;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiListViewColumnHeader(theme::ThemeName themeName);
				~GuiListViewColumnHeader();

				bool											IsAltAvailable()override;

				/// <summary>Get the column sorting state.</summary>
				/// <returns>The column sorting state.</returns>
				ColumnSortingState								GetColumnSortingState();
				/// <summary>Set the column sorting state.</summary>
				/// <param name="value">The new column sorting state.</param>
				void											SetColumnSortingState(ColumnSortingState value);
			};

			/// <summary>List view base control. All list view controls inherit from this class.</summary>
			class GuiListViewBase : public GuiSelectableListControl, public Description<GuiListViewBase>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ListViewTemplate, GuiSelectableListControl)
			public:
				/// <summary>Create a list view base control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiListViewBase(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider);
				~GuiListViewBase();

				/// <summary>Column clicked event.</summary>
				compositions::GuiItemNotifyEvent				ColumnClicked;
			};

/***********************************************************************
ListView ItemStyleProvider
***********************************************************************/

			namespace list
			{
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
ListViewColumnItemArranger
***********************************************************************/

				/// <summary>List view column item arranger. This arranger contains column headers. When an column header is resized, all items will be notified via the [T:vl.presentation.controls.list.ListViewColumnItemArranger.IColumnItemView] for <see cref="GuiListControl::IItemProvider"/>.</summary>
				class ListViewColumnItemArranger : public FixedHeightItemArranger, public Description<ListViewColumnItemArranger>
				{
					typedef collections::List<GuiListViewColumnHeader*>					ColumnHeaderButtonList;
					typedef collections::List<compositions::GuiBoundsComposition*>		ColumnHeaderSplitterList;
				public:
					static const vint							SplitterWidth=8;
					
					/// <summary>Callback for [T:vl.presentation.controls.list.ListViewColumnItemArranger.IColumnItemView]. Column item view use this interface to notify column related modification.</summary>
					class IColumnItemViewCallback : public virtual IDescriptable, public Description<IColumnItemViewCallback>
					{
					public:
						/// <summary>Called when any column is changed (inserted, removed, text changed, etc.).</summary>
						virtual void							OnColumnChanged()=0;
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
				protected:
					class ColumnItemViewCallback : public Object, public virtual IColumnItemViewCallback
					{
					protected:
						ListViewColumnItemArranger*				arranger;
					public:
						ColumnItemViewCallback(ListViewColumnItemArranger* _arranger);
						~ColumnItemViewCallback();

						void									OnColumnChanged();
					};

					GuiListViewBase*							listView = nullptr;
					IListViewItemView*							listViewItemView = nullptr;
					IColumnItemView*							columnItemView = nullptr;
					Ptr<ColumnItemViewCallback>					columnItemViewCallback;
					compositions::GuiStackComposition*			columnHeaders = nullptr;
					ColumnHeaderButtonList						columnHeaderButtons;
					ColumnHeaderSplitterList					columnHeaderSplitters;
					bool										splitterDragging = false;
					vint										splitterLatestX = 0;

					void										ColumnClicked(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void										ColumnBoundsChanged(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void										ColumnHeaderSplitterLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void										ColumnHeaderSplitterLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void										ColumnHeaderSplitterMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);

					void										RearrangeItemBounds()override;
					vint										GetWidth()override;
					vint										GetYOffset()override;
					Size										OnCalculateTotalSize()override;
					void										DeleteColumnButtons();
					void										RebuildColumns();
				public:
					ListViewColumnItemArranger();
					~ListViewColumnItemArranger();

					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;
				};
			}

/***********************************************************************
ListViewItemProvider
***********************************************************************/

			namespace list
			{
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
					
					void											NotifyUpdate(bool affectItem);
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

				class IListViewItemProvider : public virtual Interface
				{
				public:
					virtual void									NotifyAllItemsUpdate() = 0;
					virtual void									NotifyAllColumnsUpdate() = 0;
				};

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
				
				/// <summary>List view column container.</summary>
				class ListViewColumns : public collections::ObservableListBase<Ptr<ListViewColumn>>
				{
					friend class ListViewColumn;
				protected:
					IListViewItemProvider*							itemProvider;
					bool											affectItemFlag = true;

					void											NotifyColumnUpdated(vint column, bool affectItem);
					void											AfterInsert(vint index, const Ptr<ListViewColumn>& value)override;
					void											BeforeRemove(vint index, const Ptr<ListViewColumn>& value)override;
					void											NotifyUpdateInternal(vint start, vint count, vint newCount)override;
				public:
					/// <summary>Create a container.</summary>
					/// <param name="_itemProvider">The item provider in the same control to receive notifications.</param>
					ListViewColumns(IListViewItemProvider* _itemProvider);
					~ListViewColumns();
				};
				
				/// <summary>Item provider for <see cref="GuiListViewBase"/>.</summary>
				class ListViewItemProvider
					: public ListProvider<Ptr<ListViewItem>>
					, protected virtual IListViewItemProvider
					, public virtual IListViewItemView
					, public virtual ListViewColumnItemArranger::IColumnItemView
					, public Description<ListViewItemProvider>
				{
					friend class ListViewItem;
					friend class ListViewColumns;
					friend class ListViewDataColumns;
					typedef collections::List<ListViewColumnItemArranger::IColumnItemViewCallback*>		ColumnItemViewCallbackList;
				protected:
					ListViewDataColumns									dataColumns;
					ListViewColumns										columns;
					ColumnItemViewCallbackList							columnItemViewCallbacks;

					void												AfterInsert(vint index, const Ptr<ListViewItem>& value)override;
					void												BeforeRemove(vint index, const Ptr<ListViewItem>& value)override;

					void												NotifyAllItemsUpdate()override;
					void												NotifyAllColumnsUpdate()override;

					Ptr<GuiImageData>									GetSmallImage(vint itemIndex)override;
					Ptr<GuiImageData>									GetLargeImage(vint itemIndex)override;
					WString												GetText(vint itemIndex)override;
					WString												GetSubItem(vint itemIndex, vint index)override;
					vint												GetDataColumnCount()override;
					vint												GetDataColumn(vint index)override;
					vint												GetColumnCount()override;
					WString												GetColumnText(vint index)override;;

					bool												AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					bool												DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					vint												GetColumnSize(vint index)override;
					void												SetColumnSize(vint index, vint value)override;
					GuiMenu*											GetDropdownPopup(vint index)override;
					ColumnSortingState									GetSortingState(vint index)override;

					WString												GetTextValue(vint itemIndex)override;
					description::Value									GetBindingValue(vint itemIndex)override;
				public:
					ListViewItemProvider();
					~ListViewItemProvider();

					IDescriptable*										RequestView(const WString& identifier)override;

					/// <summary>Get all data columns indices in columns.</summary>
					/// <returns>All data columns indices in columns.</returns>
					ListViewDataColumns&								GetDataColumns();
					/// <summary>Get all columns.</summary>
					/// <returns>All columns.</returns>
					ListViewColumns&									GetColumns();
				};
			}

/***********************************************************************
GuiVirtualListView
***********************************************************************/

			enum class ListViewView
			{
				BigIcon,
				SmallIcon,
				List,
				Tile,
				Information,
				Detail,
				Unknown,
			};
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiVirtualListView : public GuiListViewBase, public Description<GuiVirtualListView>
			{
			protected:
				ListViewView											view = ListViewView::Unknown;

				void													OnStyleInstalled(vint itemIndex, ItemStyle* style)override;
				void													OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a list view control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualListView(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider);
				~GuiVirtualListView();

				/// <summary>Get the current view.</summary>
				/// <returns>The current view. After [M:vl.presentation.controls.GuiListControl.SetItemTemplate] is called, the current view is reset to Unknown.</returns>
				ListViewView											GetView();
				/// <summary>Set the current view.</summary>
				/// <param name="_view">The current view.</param>
				void													SetView(ListViewView _view);
			};

/***********************************************************************
GuiListView
***********************************************************************/
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiListView : public GuiVirtualListView, public Description<GuiListView>
			{
			protected:
				list::ListViewItemProvider*								items;
			public:
				/// <summary>Create a list view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiListView(theme::ThemeName themeName);
				~GuiListView();
				
				/// <summary>Get all list view items.</summary>
				/// <returns>All list view items.</returns>
				list::ListViewItemProvider&								GetItems();
				/// <summary>Get all data columns indices in columns.</summary>
				/// <returns>All data columns indices in columns.</returns>
				list::ListViewDataColumns&								GetDataColumns();
				/// <summary>Get all columns.</summary>
				/// <returns>All columns.</returns>
				list::ListViewColumns&									GetColumns();

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				Ptr<list::ListViewItem>									GetSelectedItem();
			};
		}
	}
}

#endif