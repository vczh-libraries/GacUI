/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATASTRUCTURED
#define VCZH_PRESENTATION_CONTROLS_GUIDATASTRUCTURED

#include "GuiDataGridControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{

/***********************************************************************
Interfaces
***********************************************************************/

				class IDataFilterCallback : public virtual IDescriptable, public Description<IDataFilterCallback>
				{
				public:
					virtual GuiListControl::IItemProvider*				GetItemProvider() = 0;
					virtual void										OnFilterChanged() = 0;
				};

				class IDataFilter : public virtual IDescriptable, public Description<IDataFilter>
				{
				public:
					virtual void										SetCallback(IDataFilterCallback* value) = 0;
					virtual bool										Filter(vint row) = 0;
				};


				class IDataSorter : public virtual IDescriptable, public Description<IDataSorter>
				{
				public:
					virtual vint										Compare(vint row1, vint row2) = 0;
				};

/***********************************************************************
Filter Extensions
***********************************************************************/

				/// <summary>Base class for <see cref="IDataFilter"/>.</summary>
				class DataFilterBase : public Object, public virtual IDataFilter, public Description<DataFilterBase>
				{
				protected:
					IDataFilterCallback*								callback = nullptr;

					/// <summary>Called when the structure or properties for this filter is changed.</summary>
					void												InvokeOnFilterChanged();
				public:
					DataFilterBase();

					void												SetCallback(IDataFilterCallback* value)override;
				};
				
				/// <summary>Base class for a <see cref="IDataFilter"/> that contains multiple sub filters.</summary>
				class DataMultipleFilter : public DataFilterBase, public Description<DataMultipleFilter>
				{
				protected:
					collections::List<Ptr<IDataFilter>>		filters;

				public:
					DataMultipleFilter();

					/// <summary>Add a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												AddSubFilter(Ptr<IDataFilter> value);
					/// <summary>Remove a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												RemoveSubFilter(Ptr<IDataFilter> value);
					void												SetCallback(IDataFilterCallback* value)override;
				};

				/// <summary>A filter that keep a row if all sub filters agree.</summary>
				class DataAndFilter : public DataMultipleFilter, public Description<DataAndFilter>
				{
				public:
					/// <summary>Create the filter.</summary>
					DataAndFilter();

					bool												Filter(vint row)override;
				};
				
				/// <summary>A filter that keep a row if one of all sub filters agrees.</summary>
				class DataOrFilter : public DataMultipleFilter, public Description<DataOrFilter>
				{
				public:
					/// <summary>Create the filter.</summary>
					DataOrFilter();

					bool												Filter(vint row)override;
				};
				
				/// <summary>A filter that keep a row if the sub filter not agrees.</summary>
				class DataNotFilter : public DataFilterBase, public Description<DataNotFilter>
				{
				protected:
					Ptr<IDataFilter>							filter;
				public:
					/// <summary>Create the filter.</summary>
					DataNotFilter();
					
					/// <summary>Set a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												SetSubFilter(Ptr<IDataFilter> value);
					void												SetCallback(IDataFilterCallback* value)override;
					bool												Filter(vint row)override;
				};

/***********************************************************************
Sorter Extensions
***********************************************************************/
				
				/// <summary>A multi-level <see cref="IDataSorter"/>.</summary>
				class DataMultipleSorter : public Object, public virtual IDataSorter, public Description<DataMultipleSorter>
				{
				protected:
					Ptr<IDataSorter>							leftSorter;
					Ptr<IDataSorter>							rightSorter;
				public:
					/// <summary>Create the sorter.</summary>
					DataMultipleSorter();
					
					/// <summary>Set the first sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetLeftSorter(Ptr<IDataSorter> value);
					/// <summary>Set the second sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetRightSorter(Ptr<IDataSorter> value);
					vint												Compare(vint row1, vint row2)override;
				};
				
				/// <summary>A reverse order <see cref="IDataSorter"/>.</summary>
				class DataReverseSorter : public Object, public virtual IDataSorter, public Description<DataReverseSorter>
				{
				protected:
					Ptr<IDataSorter>							sorter;
				public:
					/// <summary>Create the sorter.</summary>
					DataReverseSorter();
					
					/// <summary>Set the sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetSubSorter(Ptr<IDataSorter> value);
					vint												Compare(vint row1, vint row2)override;
				};

/***********************************************************************
DataColumn
***********************************************************************/

				class DataColumns;
				class DataProvider;

				/// <summary>Datagrid Column.</summary>
				class DataColumn : public Object, public Description<DataColumn>
				{
					friend class DataColumns;
					friend class DataProvider;
				protected:
					DataProvider*										dataProvider = nullptr;
					ItemProperty<WString>								textProperty;
					WritableItemProperty<description::Value>			valueProperty;
					WString												text;
					vint												size = 0;
					ColumnSortingState									sortingState = ColumnSortingState::NotSorted;
					GuiMenu*											popup = nullptr;
					Ptr<IDataFilter>									inherentFilter;
					Ptr<IDataSorter>									inherentSorter;
					Ptr<IDataVisualizerFactory>							visualizerFactory;
					Ptr<IDataEditorFactory>								editorFactory;

					void												NotifyColumnChanged();
				public:
					DataColumn();
					~DataColumn();

					/// <summary>Value property name changed event.</summary>
					compositions::GuiNotifyEvent						ValuePropertyChanged;
					/// <summary>Text property name changed event.</summary>
					compositions::GuiNotifyEvent						TextPropertyChanged;

					/// <summary>Get the text for the column.</summary>
					/// <returns>The text for the column.</returns>
					WString												GetText();
					/// <summary>Set the text for the column.</summary>
					/// <param name="value">The text for the column.</param>
					void												SetText(const WString& value);

					/// <summary>Get the size for the column.</summary>
					/// <returns>The size for the column.</returns>
					vint												GetSize();
					/// <summary>Set the size for the column.</summary>
					/// <param name="value">The size for the column.</param>
					void												SetSize(vint value);

					/// <summary>Get the popup for the column.</summary>
					/// <returns>The popup for the column.</returns>
					GuiMenu*											GetPopup();
					/// <summary>Set the popup for the column.</summary>
					/// <param name="value">The popup for the column.</param>
					void												SetPopup(GuiMenu* value);

					/// <summary>Get the inherent filter for the column.</summary>
					/// <returns>The inherent filter for the column.</returns>
					Ptr<IDataFilter>									GetInherentFilter();
					/// <summary>Set the inherent filter for the column.</summary>
					/// <param name="value">The filter.</param>
					void												SetInherentFilter(Ptr<IDataFilter> value);

					/// <summary>Get the inherent sorter for the column.</summary>
					/// <returns>The inherent sorter for the column.</returns>
					Ptr<IDataSorter>									GetInherentSorter();
					/// <summary>Set the inherent sorter for the column.</summary>
					/// <param name="value">The sorter.</param>
					void												SetInherentSorter(Ptr<IDataSorter> value);

					/// <summary>Get the visualizer factory for the column.</summary>
					/// <returns>The the visualizer factory for the column.</returns>
					Ptr<IDataVisualizerFactory>							GetVisualizerFactory();
					/// <summary>Set the visualizer factory for the column.</summary>
					/// <param name="value">The visualizer factory.</param>
					/// <returns>The current column provider itself.</returns>
					void												SetVisualizerFactory(Ptr<IDataVisualizerFactory> value);

					/// <summary>Get the editor factory for the column.</summary>
					/// <returns>The the editor factory for the column.</returns>
					Ptr<IDataEditorFactory>								GetEditorFactory();
					/// <summary>Set the editor factory for the column.</summary>
					/// <param name="value">The editor factory.</param>
					/// <returns>The current column provider itself.</returns>
					void												SetEditorFactory(Ptr<IDataEditorFactory> value);

					/// <summary>Get the text value from an item.</summary>
					/// <returns>The text value.</returns>
					/// <param name="row">The row index of the item.</param>
					WString												GetCellText(vint row);
					/// <summary>Get the cell value from an item.</summary>
					/// <returns>The cell value.</returns>
					/// <param name="row">The row index of the item.</param>
					description::Value									GetCellValue(vint row);
					/// <summary>Set the cell value to an item.</summary>
					/// <param name="row">The row index of the item.</param>
					/// <param name="value">The value property name.</param>
					void												SetCellValue(vint row, description::Value value);

					/// <summary>Get the text property name to get the cell text from an item.</summary>
					/// <returns>The text property name.</returns>
					ItemProperty<WString>								GetTextProperty();
					/// <summary>Set the text property name to get the cell text from an item.</summary>
					/// <param name="value">The text property name.</param>
					void												SetTextProperty(const ItemProperty<WString>& value);

					/// <summary>Get the value property name to get the cell value from an item.</summary>
					/// <returns>The value property name.</returns>
					WritableItemProperty<description::Value>			GetValueProperty();
					/// <summary>Set the value property name to get the cell value from an item.</summary>
					/// <param name="value">The value property name.</param>
					void												SetValueProperty(const WritableItemProperty<description::Value>& value);
				};

				class DataColumns : public ItemsBase<Ptr<DataColumn>>
				{
				protected:
					DataProvider*										dataProvider = nullptr;

					void												NotifyUpdateInternal(vint start, vint count, vint newCount)override;
					bool												QueryInsert(vint index, const Ptr<DataColumn>& value)override;
					void												AfterInsert(vint index, const Ptr<DataColumn>& value)override;
					void												BeforeRemove(vint index, const Ptr<DataColumn>& value)override;
				public:
					DataColumns(DataProvider* _dataProvider);
					~DataColumns();
				};

/***********************************************************************
DataProvider
***********************************************************************/

				class DataProvider
					: public virtual ItemProviderBase
					, public virtual IListViewItemView
					, public virtual ListViewColumnItemArranger::IColumnItemView
					, public virtual IDataGridView
					, protected virtual IDataFilterCallback
					, public Description<DataProvider>
				{
					friend class DataColumn;
					friend class DataColumns;
				protected:
					DataColumns												columns;
					ListViewColumnItemArranger::IColumnItemViewCallback*	columnItemViewCallback = nullptr;
					Ptr<description::IValueReadonlyList>					itemSource;
					Ptr<EventHandler>										itemChangedEventHandler;

					description::Value										viewModelContext;
					Ptr<IDataFilter>										additionalFilter;
					Ptr<IDataFilter>										currentFilter;
					Ptr<IDataSorter>										currentSorter;
					collections::List<vint>									virtualRowToSourceRow;

					void													NotifyAllItemsChanged();
					void													NotifyColumnChanged();
					GuiListControl::IItemProvider*							GetItemProvider()override;

					void													OnFilterChanged()override;
					void													OnItemSourceModified(vint start, vint count, vint newCount);

					void													RebuildFilter();
					void													ReorderRows(bool invokeCallback);
				public:
					/// <summary>Create a data provider from a <see cref="IDataProvider"/>.</summary>
					/// <param name="provider">The structured data provider.</param>
					DataProvider(const description::Value& _viewModelContext);
					~DataProvider();

					DataColumns&										GetColumns();
					Ptr<description::IValueEnumerable>					GetItemSource();
					void												SetItemSource(Ptr<description::IValueEnumerable> _itemSource);
					
					/// <summary>Get the additional filter.</summary>
					/// <returns>The additional filter.</returns>
					Ptr<IDataFilter>									GetAdditionalFilter();
					/// <summary>Set the additional filter. This filter will be composed with inherent filters of all column to be the final filter.</summary>
					/// <param name="value">The additional filter.</param>
					void												SetAdditionalFilter(Ptr<IDataFilter> value);

					// ===================== GuiListControl::IItemProvider =====================

					vint												Count()override;
					WString												GetTextValue(vint itemIndex)override;
					description::Value									GetBindingValue(vint itemIndex)override;
					IDescriptable*										RequestView(const WString& identifier)override;
					
					// ===================== list::IListViewItemProvider =====================

					Ptr<GuiImageData>									GetSmallImage(vint itemIndex)override;
					Ptr<GuiImageData>									GetLargeImage(vint itemIndex)override;
					WString												GetText(vint itemIndex)override;
					WString												GetSubItem(vint itemIndex, vint index)override;
					vint												GetDataColumnCount()override;
					vint												GetDataColumn(vint index)override;
					vint												GetColumnCount()override;
					WString												GetColumnText(vint index)override;
					
					// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================
						
					bool												AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					bool												DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					vint												GetColumnSize(vint index)override;
					void												SetColumnSize(vint index, vint value)override;
					GuiMenu*											GetDropdownPopup(vint index)override;
					ColumnSortingState									GetSortingState(vint index)override;
					
					// ===================== list::IDataGridView =====================

					description::Value									GetViewModelContext()override;
					bool												IsColumnSortable(vint column)override;
					void												SortByColumn(vint column, bool ascending)override;
					vint												GetSortedColumn()override;
					bool												IsSortOrderAscending()override;
					
					vint												GetCellSpan(vint row, vint column)override;
					IDataVisualizerFactory*								GetCellDataVisualizerFactory(vint row, vint column)override;
					void												VisualizeCell(vint row, vint column, IDataVisualizer* dataVisualizer)override;
					IDataEditorFactory*									GetCellDataEditorFactory(vint row, vint column)override;
					void												EditCell(vint row, vint column, IDataEditor* dataEditor)override;
					void												SaveCellData(vint row, vint column, IDataEditor* dataEditor)override;
				};
			}

/***********************************************************************
GuiBindableDataGrid
***********************************************************************/

			/// <summary>A bindable Data grid control.</summary>
			class GuiBindableDataGrid : public GuiVirtualDataGrid, public Description<GuiBindableDataGrid>
			{
			protected:
				list::DataProvider*									dataProvider = nullptr;

			public:
				/// <summary>Create a bindable Data grid control.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_viewModelContext">The view mode context, which will be passed to every visualizers and editors in this grid.</param>
				GuiBindableDataGrid(IStyleProvider* _styleProvider, const description::Value& _viewModelContext = description::Value());
				~GuiBindableDataGrid();

				/// <summary>Get all columns.</summary>
				list::DataColumns&									GetColumns();

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				Ptr<description::IValueEnumerable>					GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="_itemSource">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(Ptr<description::IValueEnumerable> _itemSource);


				/// <summary>Get the selected cell.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value									GetSelectedRowValue();

				/// <summary>Get the selected cell.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value									GetSelectedCellValue();
			};
		}
	}
}

#endif