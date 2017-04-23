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
			class GuiListViewBase;

			namespace list
			{

/***********************************************************************
ListView Base
***********************************************************************/

				/// <summary>Item style provider base for <see cref="GuiListViewBase"/>.</summary>
				class ListViewItemStyleProviderBase: public Object, public GuiSelectableListControl::IItemStyleProvider, public Description<ListViewItemStyleProviderBase>
				{
				public:
					/// <summary>Item style controller base for <see cref="GuiListViewBase"/>.</summary>
					class ListViewItemStyleController : public ItemStyleControllerBase, public Description<ListViewItemStyleController>
					{
					protected:
						GuiSelectableButton*					backgroundButton;
						ListViewItemStyleProviderBase*			listViewItemStyleProvider;

					public:
						ListViewItemStyleController(ListViewItemStyleProviderBase* provider);
						~ListViewItemStyleController();

						/// <summary>Get the selection state.</summary>
						/// <returns>Returns true if this item is selected.</returns>
						bool									GetSelected();
						/// <summary>Set the selection state.</summary>
						/// <param name="value">Set to true to render this item as selected.</param>
						void									SetSelected(bool value);
					};

				protected:
					GuiListViewBase*							listControl;

				public:
					/// <summary>Create the item style provider.</summary>
					ListViewItemStyleProviderBase();
					~ListViewItemStyleProviderBase();

					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;
					void										SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)override;
				};
			}

			///<summary>List view column header control for detailed view.</summary>
			class GuiListViewColumnHeader : public GuiMenuButton, public Description<GuiListViewColumnHeader>
			{
			public:
				/// <summary>Style provider for <see cref="GuiListViewColumnHeader"/>.</summary>
				class IStyleController : public virtual GuiMenuButton::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Notify that the column sorting state is changed.</summary>
					/// <param name="value">The new column sorting state.</param>
					virtual void								SetColumnSortingState(ColumnSortingState value)=0;
				};

			protected:
				IStyleController*								styleController;
				ColumnSortingState								columnSortingState;

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiListViewColumnHeader(IStyleController* _styleController);
				~GuiListViewColumnHeader();

				bool											IsAltAvailable()override;

				/// <summary>Get the column sorting state.</summary>
				/// <returns>The column sorting state.</returns>
				ColumnSortingState								GetColumnSortingState();
				/// <summary>Set the column sorting state.</summary>
				/// <param name="value">The new column sorting state.</param>
				void											SetColumnSortingState(ColumnSortingState value);
			};

			/// <summary>List view base control. All list view controls inherit from this class. <see cref="list::ListViewItemStyleProviderBase"/> is suggested to be the base class of item style providers for list view control.</summary>
			class GuiListViewBase : public GuiSelectableListControl, public Description<GuiListViewBase>
			{
			public:
				/// <summary>Style provider for <see cref="GuiListViewBase"/>.</summary>
				class IStyleProvider : public virtual GuiSelectableListControl::IStyleProvider, public Description<IStyleProvider>
				{
				public:
					/// <summary>Create a style controller for an item background.</summary>
					/// <returns>The created style controller for an item background.</returns>
					virtual GuiSelectableButton::IStyleController*		CreateItemBackground()=0;
					/// <summary>Create a style controller for a column header.</summary>
					/// <returns>The created style controller for a column header.</returns>
					virtual GuiListViewColumnHeader::IStyleController*	CreateColumnStyle()=0;
					/// <summary>Get the primary text color.</summary>
					/// <returns>The primary text color.</returns>
					virtual Color										GetPrimaryTextColor()=0;
					/// <summary>Get the secondary text color.</summary>
					/// <returns>The secondary text color.</returns>
					virtual Color										GetSecondaryTextColor()=0;
					/// <summary>Get the item peparator text color.</summary>
					/// <returns>The item peparator text color.</returns>
					virtual Color										GetItemSeparatorColor()=0;
				};

			protected:
				IStyleProvider*									styleProvider;

			public:
				/// <summary>Create a list view base control.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiListViewBase(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider);
				~GuiListViewBase();

				/// <summary>Column clicked event.</summary>
				compositions::GuiItemNotifyEvent				ColumnClicked;
				
				/// <summary>Get the associated style provider.</summary>
				/// <returns>The style provider.</returns>
				IStyleProvider*									GetListViewStyleProvider();
				Ptr<GuiListControl::IItemStyleProvider>			SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)override;
			};

/***********************************************************************
ListView ItemStyleProvider
***********************************************************************/

			namespace list
			{
				/// <summary>Base class for all predefined list view item style.</summary>
				class ListViewItemStyleProvider : public ListViewItemStyleProviderBase, public Description<ListViewItemStyleProvider>
				{
				public:
					/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for <see cref="ListViewItemStyleProvider"/>.</summary>
					class IListViewItemView : public virtual IDescriptable, public Description<IListViewItemView>
					{
					public:
						/// <summary>The identifier for this view.</summary>
						static const wchar_t* const				Identifier;

						/// <summary>Get the small image of an item.</summary>
						/// <returns>The small image.</returns>
						/// <param name="itemIndex">The index of the item.</param>
						virtual Ptr<GuiImageData>				GetSmallImage(vint itemIndex)=0;
						/// <summary>Get the large image of an item.</summary>
						/// <returns>The large image.</returns>
						/// <param name="itemIndex">The index of the item.</param>
						virtual Ptr<GuiImageData>				GetLargeImage(vint itemIndex)=0;
						/// <summary>Get the text of an item.</summary>
						/// <returns>The text.</returns>
						/// <param name="itemIndex">The index of the item.</param>
						virtual WString							GetText(vint itemIndex)=0;
						/// <summary>Get the sub item text of an item. If the sub item index out of range, it returns an empty string.</summary>
						/// <returns>The sub item text.</returns>
						/// <param name="itemIndex">The index of the item.</param>
						/// <param name="index">The sub item index of the item.</param>
						virtual WString							GetSubItem(vint itemIndex, vint index)=0;

						/// <summary>Get the number of data columns.</summary>
						/// <returns>The number of data columns.</returns>
						virtual vint							GetDataColumnCount()=0;
						/// <summary>Get the column index of the index-th data column.</summary>
						/// <returns>The column index.</returns>
						/// <param name="index">The order of the data column.</param>
						virtual vint							GetDataColumn(vint index)=0;

						/// <summary>Get the number of columns.</summary>
						/// <returns>The number of columns.</returns>
						virtual vint							GetColumnCount()=0;
						/// <summary>Get the text of a column.</summary>
						/// <returns>The text.</returns>
						/// <param name="index">The index of the column.</param>
						virtual WString							GetColumnText(vint index)=0;
					};

					/// <summary>Represents the extra item content information of a list view item.</summary>
					class IListViewItemContent : public virtual IDescriptable, public Description<IListViewItemContent>
					{
					public:
						/// <summary>Get the composition representing the whole item.</summary>
						/// <returns>The composition representing the whole item.</returns>
						virtual compositions::GuiBoundsComposition*				GetContentComposition()=0;
						/// <summary>Get the composition for the extra background decorator. If there is no decorator, it returns null.</summary>
						/// <returns>The composition for the extra background decorator.</returns>
						virtual compositions::GuiBoundsComposition*				GetBackgroundDecorator()=0;
						/// <summary>Install data of an item to the item content for rendering.</summary>
						/// <param name="styleProvider">Style provider for the list view control.</param>
						/// <param name="view">The <see cref="IListViewItemView"/> for the list view control.</param>
						/// <param name="itemIndex">The index of the item to install.</param>
						virtual void											Install(GuiListViewBase::IStyleProvider* styleProvider, IListViewItemView* view, vint itemIndex)=0;
						/// <summary>Called when the item content is uninstalled from the list view control.</summary>
						virtual void											Uninstall()=0;
					};

					/// <summary>List view item content provider.</summary>
					class IListViewItemContentProvider : public virtual IDescriptable, public Description<IListViewItemContentProvider>
					{
					public:
						/// <summary>Create a default and preferred <see cref="compositions::IGuiAxis"/> for the related item style provider.</summary>
						/// <returns>The created item coordinate transformer.</returns>
						virtual compositions::IGuiAxis*							CreatePreferredAxis()=0;
						/// <summary>Create a default and preferred <see cref="GuiListControl::IItemArranger"/> for the related item style provider.</summary>
						/// <returns>The created item coordinate arranger.</returns>
						virtual GuiListControl::IItemArranger*					CreatePreferredArranger()=0;
						/// <summary>Create a <see cref="IListViewItemContent"/>.</summary>
						/// <returns>The created list view item content.</returns>
						/// <param name="font">The expected font of the created item content.</param>
						virtual IListViewItemContent*							CreateItemContent(const FontProperties& font)=0;
						/// <summary>Called when the owner item style provider in installed to a <see cref="GuiListControl"/>.</summary>
						/// <param name="value">The list control.</param>
						virtual void											AttachListControl(GuiListControl* value)=0;
						/// <summary>Called when the owner item style provider in uninstalled from a <see cref="GuiListControl"/>.</summary>
						virtual void											DetachListControl()=0;
					};

					/// <summary>A list view item style controller with extra item content information.</summary>
					class ListViewContentItemStyleController : public ListViewItemStyleController, public Description<ListViewContentItemStyleController>
					{
					protected:
						ListViewItemStyleProvider*				listViewItemStyleProvider;
						Ptr<IListViewItemContent>				content;
					public:
						/// <summary>Create the item style controller.</summary>
						/// <param name="provider">The owner item style provider.</param>
						ListViewContentItemStyleController(ListViewItemStyleProvider* provider);
						~ListViewContentItemStyleController();

						void									OnUninstalled()override;

						/// <summary>Get the extra item content information.</summary>
						/// <returns>The extra item content information.</returns>
						IListViewItemContent*					GetItemContent();
						/// <summary>Install data of an item to the item style controller for rendering.</summary>
						/// <param name="view">The <see cref="IListViewItemView"/> for the list view control.</param>
						/// <param name="itemIndex">The index of the item to install.</param>
						void									Install(IListViewItemView* view, vint itemIndex);
					};

				protected:

					typedef collections::List<GuiListControl::IItemStyleController*>				ItemStyleList;

					IListViewItemView*							listViewItemView = nullptr;
					Ptr<IListViewItemContentProvider>			listViewItemContentProvider;
					ItemStyleList								itemStyles;
				public:
					/// <summary>Create a list view item style provider using an item content provider.</summary>
					/// <param name="itemContentProvider">The item content provider.</param>
					ListViewItemStyleProvider(Ptr<IListViewItemContentProvider> itemContentProvider);
					~ListViewItemStyleProvider();

					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;
					GuiListControl::IItemStyleController*		CreateItemStyle()override;
					void										DestroyItemStyle(GuiListControl::IItemStyleController* style)override;
					void										Install(GuiListControl::IItemStyleController* style, vint itemIndex)override;
					void										SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)override;

					/// <summary>Get the item content provider.</summary>
					/// <returns>The item content provider.</returns>
					IListViewItemContentProvider*				GetItemContentProvider();

					/// <summary>Get all created item styles.</summary>
					/// <returns>All created item styles.</returns>
					const ItemStyleList&						GetCreatedItemStyles();
					/// <summary>Test is an item style controller placed in the list view control. If not, maybe the style controller is cached for reusing.</summary>
					/// <returns>Returns true if an item style controller is placed in the list view control.</returns>
					/// <param name="itemStyle">The item style controller to test.</param>
					bool										IsItemStyleAttachedToListView(GuiListControl::IItemStyleController* itemStyle);

					/// <summary>Get item content from item style controller.</summary>
					/// <returns>The item content.</returns>
					/// <param name="itemStyleController">The item style controller.</param>
					IListViewItemContent*						GetItemContentFromItemStyleController(GuiListControl::IItemStyleController* itemStyleController);
					/// <summary>Get item style controller from item content.</summary>
					/// <returns>The item style controller.</returns>
					/// <param name="itemContent">The item content.</param>
					GuiListControl::IItemStyleController*		GetItemStyleControllerFromItemContent(IListViewItemContent* itemContent);

					template<typename T>
					T* GetItemContent(GuiListControl::IItemStyleController* itemStyleController)
					{
						return dynamic_cast<T*>(GetItemContentFromItemStyleController(itemStyleController));
					}
				};
			}

/***********************************************************************
ListView ItemContentProvider(Detailed)
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
						/// <summary>Get the number of all columns.</summary>
						/// <returns>The number of all columns.</returns>
						virtual vint											GetColumnCount()=0;
						/// <summary>Get the text of the column.</summary>
						/// <returns>The text of the column.</returns>
						/// <param name="index">The index of the column.</param>
						virtual WString											GetColumnText(vint index)=0;
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
						virtual GuiListViewColumnHeader::ColumnSortingState		GetSortingState(vint index)=0;
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

					GuiListViewBase*							listView;
					GuiListViewBase::IStyleProvider*			styleProvider;
					IColumnItemView*							columnItemView;
					Ptr<ColumnItemViewCallback>					columnItemViewCallback;
					compositions::GuiStackComposition*			columnHeaders;
					ColumnHeaderButtonList						columnHeaderButtons;
					ColumnHeaderSplitterList					columnHeaderSplitters;
					bool										splitterDragging;
					vint										splitterLatestX;

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
ListView
***********************************************************************/

			namespace list
			{
				class ListViewItem;

				class ListViewSubItems : public ItemsBase<WString>
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
					ListViewColumns*								owner;
					WString											text;
					ItemProperty<WString>							textProperty;
					vint											size;
					GuiMenu*										dropdownPopup;
					GuiListViewColumnHeader::ColumnSortingState		sortingState;
					
					void											NotifyUpdate();
				public:
					/// <summary>Create a column with the specified text and size.</summary>
					/// <param name="_text">The specified text.</param>
					/// <param name="_size">The specified size.</param>
					ListViewColumn(const WString& _text=L"", vint _size=160);
					
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
					/// <summary>Get the dropdown context menu of this item.</summary>
					/// <returns>The dropdown context menu of this item.</returns>
					GuiMenu*										GetDropdownPopup();
					/// <summary>Set the dropdown context menu of this item.</summary>
					/// <param name="value">The dropdown context menu of this item.</param>
					void											SetDropdownPopup(GuiMenu* value);
					/// <summary>Get the sorting state of this item.</summary>
					/// <returns>The sorting state of this item.</returns>
					GuiListViewColumnHeader::ColumnSortingState		GetSortingState();
					/// <summary>Set the sorting state of this item.</summary>
					/// <param name="value">The sorting state of this item.</param>
					void											SetSortingState(GuiListViewColumnHeader::ColumnSortingState value);
				};

				class IListViewItemProvider : public virtual Interface
				{
				public:
					virtual void									NotifyAllItemsUpdate() = 0;
					virtual void									NotifyAllColumnsUpdate() = 0;
				};

				/// <summary>List view data column container.</summary>
				class ListViewDataColumns : public ItemsBase<vint>
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
				class ListViewColumns : public ItemsBase<Ptr<ListViewColumn>>
				{
				protected:
					IListViewItemProvider*							itemProvider;

					void											AfterInsert(vint index, const Ptr<ListViewColumn>& value)override;
					void											BeforeRemove(vint index, const Ptr<ListViewColumn>& value)override;
					void											NotifyUpdateInternal(vint start, vint count, vint newCount)override;
				public:
					/// <summary>Create a container.</summary>
					/// <param name="_itemProvider">The item provider in the same control to receive notifications.</param>
					ListViewColumns(IListViewItemProvider* _itemProvider);
					~ListViewColumns();
				};
				
				/// <summary>Item provider for <see cref="GuiListViewBase"/> and <see cref="ListViewItemStyleProvider"/>.</summary>
				class ListViewItemProvider
					: public ListProvider<Ptr<ListViewItem>>
					, protected virtual IListViewItemProvider
					, protected virtual ListViewItemStyleProvider::IListViewItemView
					, protected virtual ListViewColumnItemArranger::IColumnItemView
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

					bool												AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					bool												DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					vint												GetColumnCount()override;
					WString												GetColumnText(vint index)override;
					vint												GetColumnSize(vint index)override;
					void												SetColumnSize(vint index, vint value)override;
					GuiMenu*											GetDropdownPopup(vint index)override;
					GuiListViewColumnHeader::ColumnSortingState			GetSortingState(vint index)override;

					WString												GetTextValue(vint itemIndex)override;
					description::Value									GetBindingValue(vint itemIndex)override;
				public:
					ListViewItemProvider();
					~ListViewItemProvider();

					IDescriptable*										RequestView(const WString& identifier)override;
					void												ReleaseView(IDescriptable* view)override;

					/// <summary>Get all data columns indices in columns.</summary>
					/// <returns>All data columns indices in columns.</returns>
					ListViewDataColumns&								GetDataColumns();
					/// <summary>Get all columns.</summary>
					/// <returns>All columns.</returns>
					ListViewColumns&									GetColumns();
				};
			}
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiVirtualListView : public GuiListViewBase, public Description<GuiVirtualListView>
			{
			public:
				/// <summary>Create a list view control in virtual mode.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualListView(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider);
				~GuiVirtualListView();
				
				/// <summary>Set the item content provider.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="contentProvider">The new item content provider.</param>
				virtual bool											ChangeItemStyle(Ptr<list::ListViewItemStyleProvider::IListViewItemContentProvider> contentProvider);
			};
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiListView : public GuiVirtualListView, public Description<GuiListView>
			{
			protected:
				list::ListViewItemProvider*								items;
			public:
				/// <summary>Create a list view control.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				GuiListView(IStyleProvider* _styleProvider);
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