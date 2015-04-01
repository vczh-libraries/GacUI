/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATASTRUCTURED
#define VCZH_PRESENTATION_CONTROLS_GUIDATASTRUCTURED

#include "GuiDataGridInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{

/***********************************************************************
Filter Extensions
***********************************************************************/

				/// <summary>Base class for <see cref="IStructuredDataFilter"/>.</summary>
				class StructuredDataFilterBase : public Object, public virtual IStructuredDataFilter, public Description<StructuredDataFilterBase>
				{
				protected:
					IStructuredDataFilterCommandExecutor*				commandExecutor;

					/// <summary>Called when the structure or properties for this filter is changed.</summary>
					void												InvokeOnFilterChanged();
				public:
					StructuredDataFilterBase();

					void												SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)override;
				};
				
				/// <summary>Base class for a <see cref="IStructuredDataFilter"/> that contains multiple sub filters.</summary>
				class StructuredDataMultipleFilter : public StructuredDataFilterBase, public Description<StructuredDataMultipleFilter>
				{
				protected:
					collections::List<Ptr<IStructuredDataFilter>>		filters;

				public:
					StructuredDataMultipleFilter();

					/// <summary>Add a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												AddSubFilter(Ptr<IStructuredDataFilter> value);
					/// <summary>Remove a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												RemoveSubFilter(Ptr<IStructuredDataFilter> value);
					void												SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)override;
				};

				/// <summary>A filter that keep a row if all sub filters agree.</summary>
				class StructuredDataAndFilter : public StructuredDataMultipleFilter, public Description<StructuredDataAndFilter>
				{
				public:
					/// <summary>Create the filter.</summary>
					StructuredDataAndFilter();

					bool												Filter(vint row)override;
				};
				
				/// <summary>A filter that keep a row if one of all sub filters agrees.</summary>
				class StructuredDataOrFilter : public StructuredDataMultipleFilter, public Description<StructuredDataOrFilter>
				{
				public:
					/// <summary>Create the filter.</summary>
					StructuredDataOrFilter();

					bool												Filter(vint row)override;
				};
				
				/// <summary>A filter that keep a row if the sub filter not agrees.</summary>
				class StructuredDataNotFilter : public StructuredDataFilterBase, public Description<StructuredDataNotFilter>
				{
				protected:
					Ptr<IStructuredDataFilter>							filter;
				public:
					/// <summary>Create the filter.</summary>
					StructuredDataNotFilter();
					
					/// <summary>Set a sub filter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub filter.</param>
					bool												SetSubFilter(Ptr<IStructuredDataFilter> value);
					void												SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)override;
					bool												Filter(vint row)override;
				};

/***********************************************************************
Sorter Extensions
***********************************************************************/
				
				/// <summary>A multi-level <see cref="IStructuredDataSorter"/>.</summary>
				class StructuredDataMultipleSorter : public Object, public virtual IStructuredDataSorter, public Description<StructuredDataMultipleSorter>
				{
				protected:
					Ptr<IStructuredDataSorter>							leftSorter;
					Ptr<IStructuredDataSorter>							rightSorter;
				public:
					/// <summary>Create the sorter.</summary>
					StructuredDataMultipleSorter();
					
					/// <summary>Set the first sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetLeftSorter(Ptr<IStructuredDataSorter> value);
					/// <summary>Set the second sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetRightSorter(Ptr<IStructuredDataSorter> value);
					vint												Compare(vint row1, vint row2)override;
				};
				
				/// <summary>A reverse order <see cref="IStructuredDataSorter"/>.</summary>
				class StructuredDataReverseSorter : public Object, public virtual IStructuredDataSorter, public Description<StructuredDataReverseSorter>
				{
				protected:
					Ptr<IStructuredDataSorter>							sorter;
				public:
					/// <summary>Create the sorter.</summary>
					StructuredDataReverseSorter();
					
					/// <summary>Set the sub sorter.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The sub sorter.</param>
					bool												SetSubSorter(Ptr<IStructuredDataSorter> value);
					vint												Compare(vint row1, vint row2)override;
				};

/***********************************************************************
Structured DataSource Extensions
***********************************************************************/

				/// <summary>An <see cref="IDataProvider"/> wrapper for <see cref="IStructuredDataProvider"/>.</summary>
				class StructuredDataProvider
					: public Object
					, public virtual IDataProvider
					, protected virtual IDataProviderCommandExecutor
					, protected virtual IStructuredDataFilterCommandExecutor
					, public Description<StructuredDataProvider>
				{
				protected:
					Ptr<IStructuredDataProvider>						structuredDataProvider;
					IDataProviderCommandExecutor*						commandExecutor;
					Ptr<IStructuredDataFilter>							additionalFilter;
					Ptr<IStructuredDataFilter>							currentFilter;
					Ptr<IStructuredDataSorter>							currentSorter;
					collections::List<vint>								reorderedRows;
					
					void												OnDataProviderColumnChanged()override;
					void												OnDataProviderItemModified(vint start, vint count, vint newCount)override;
					void												OnFilterChanged()override;
					void												RebuildFilter(bool invokeCallback);
					void												ReorderRows(bool invokeCallback);
					vint												TranslateRowNumber(vint row);
				public:
					/// <summary>Create a data provider from a <see cref="IStructuredDataProvider"/>.</summary>
					/// <param name="provider">The structured data provider.</param>
					StructuredDataProvider(Ptr<IStructuredDataProvider> provider);
					~StructuredDataProvider();
					
					/// <summary>Get the <see cref="IStructuredDataProvider"/> object.</summary>
					/// <returns>The provider.</returns>
					Ptr<IStructuredDataProvider>						GetStructuredDataProvider();
					/// <summary>Get the additional filter.</summary>
					/// <returns>The additional filter.</returns>
					Ptr<IStructuredDataFilter>							GetAdditionalFilter();
					/// <summary>Set the additional filter. This filter will be composed with inherent filters of all column to be the final filter.</summary>
					/// <param name="value">The additional filter.</param>
					void												SetAdditionalFilter(Ptr<IStructuredDataFilter> value);

					void												SetCommandExecutor(IDataProviderCommandExecutor* value)override;
					vint												GetColumnCount()override;
					WString												GetColumnText(vint column)override;
					vint												GetColumnSize(vint column)override;
					void												SetColumnSize(vint column, vint value)override;
					GuiMenu*											GetColumnPopup(vint column)override;
					bool												IsColumnSortable(vint column)override;
					void												SortByColumn(vint column, bool ascending)override;
					vint												GetSortedColumn()override;
					bool												IsSortOrderAscending()override;
					
					vint												GetRowCount()override;
					Ptr<GuiImageData>									GetRowLargeImage(vint row)override;
					Ptr<GuiImageData>									GetRowSmallImage(vint row)override;
					vint												GetCellSpan(vint row, vint column)override;
					WString												GetCellText(vint row, vint column)override;
					IDataVisualizerFactory*								GetCellDataVisualizerFactory(vint row, vint column)override;
					void												VisualizeCell(vint row, vint column, IDataVisualizer* dataVisualizer)override;
					IDataEditorFactory*									GetCellDataEditorFactory(vint row, vint column)override;
					void												BeforeEditCell(vint row, vint column, IDataEditor* dataEditor)override;
					void												SaveCellData(vint row, vint column, IDataEditor* dataEditor)override;
				};

				/// <summary>Base class for <see cref="IStructuredColumnProvider"/>.</summary>
				class StructuredColummProviderBase : public Object, public virtual IStructuredColumnProvider, public Description<StructuredColummProviderBase>
				{
				protected:
					IDataProviderCommandExecutor*						commandExecutor;
					WString												text;
					vint												size;
					GuiListViewColumnHeader::ColumnSortingState			sortingState;
					GuiMenu*											popup;
					Ptr<IStructuredDataFilter>							inherentFilter;
					Ptr<IStructuredDataSorter>							inherentSorter;
					Ptr<IDataVisualizerFactory>							visualizerFactory;
					Ptr<IDataEditorFactory>								editorFactory;

				public:
					StructuredColummProviderBase();
					~StructuredColummProviderBase();
					
					/// <summary>Set the command executor.</summary>
					/// <param name="value">The command executor.</param>
					void												SetCommandExecutor(IDataProviderCommandExecutor* value);
					/// <summary>Set the text for the column.</summary>
					/// <param name="value">The text for the column.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetText(const WString& value);
					/// <summary>Set the popup for the column.</summary>
					/// <param name="value">The popup for the column.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetPopup(GuiMenu* value);
					/// <summary>Set the inherent filter for the column.</summary>
					/// <param name="value">The filter.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetInherentFilter(Ptr<IStructuredDataFilter> value);
					/// <summary>Set the inherent sorter for the column.</summary>
					/// <param name="value">The sorter.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetInherentSorter(Ptr<IStructuredDataSorter> value);
					/// <summary>Get the visualizer factory for the column.</summary>
					/// <returns>The the visualizer factory for the column.</returns>
					Ptr<IDataVisualizerFactory>							GetVisualizerFactory();
					/// <summary>Set the visualizer factory for the column.</summary>
					/// <param name="value">The visualizer factory.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetVisualizerFactory(Ptr<IDataVisualizerFactory> value);
					/// <summary>Get the editor factory for the column.</summary>
					/// <returns>The the editor factory for the column.</returns>
					Ptr<IDataEditorFactory>								GetEditorFactory();
					/// <summary>Set the editor factory for the column.</summary>
					/// <param name="value">The editor factory.</param>
					/// <returns>The current column provider itself.</returns>
					StructuredColummProviderBase*						SetEditorFactory(Ptr<IDataEditorFactory> value);

					WString												GetText()override;
					vint												GetSize()override;
					void												SetSize(vint value)override;
					GuiListViewColumnHeader::ColumnSortingState			GetSortingState()override;
					void												SetSortingState(GuiListViewColumnHeader::ColumnSortingState value)override;
					GuiMenu*											GetPopup()override;
					Ptr<IStructuredDataFilter>							GetInherentFilter()override;
					Ptr<IStructuredDataSorter>							GetInherentSorter()override;
					
					IDataVisualizerFactory*								GetCellDataVisualizerFactory(vint row)override;
					void												VisualizeCell(vint row, IDataVisualizer* dataVisualizer)override;
					IDataEditorFactory*									GetCellDataEditorFactory(vint row)override;
					void												BeforeEditCell(vint row, IDataEditor* dataEditor)override;
					void												SaveCellData(vint row, IDataEditor* dataEditor)override;
				};

				/// <summary>Base class for <see cref="IStructuredDataProvider"/>.</summary>
				class StructuredDataProviderBase : public Object, public virtual IStructuredDataProvider, public Description<StructuredDataProviderBase>
				{
					typedef collections::List<Ptr<StructuredColummProviderBase>>		ColumnList;
				protected:
					IDataProviderCommandExecutor*						commandExecutor;
					ColumnList											columns;

					/// <summary>Insert a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="column">The column index.</param>
					/// <param name="value">The column.</param>
					/// <param name="callback">Set to true to invoke the command executor.</param>
					bool												InsertColumnInternal(vint column, Ptr<StructuredColummProviderBase> value, bool callback);
					/// <summary>Add a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The column.</param>
					/// <param name="callback">Set to true to invoke the command executor.</param>
					bool												AddColumnInternal(Ptr<StructuredColummProviderBase> value, bool callback);
					/// <summary>Remove a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The column.</param>
					/// <param name="callback">Set to true to invoke the command executor.</param>
					bool												RemoveColumnInternal(Ptr<StructuredColummProviderBase> value, bool callback);
					/// <summary>Clear all columns.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="callback">Set to true to invoke the command executor.</param>
					bool												ClearColumnsInternal(bool callback);
				public:
					StructuredDataProviderBase();
					~StructuredDataProviderBase();

					void												SetCommandExecutor(IDataProviderCommandExecutor* value)override;
					vint												GetColumnCount()override;
					IStructuredColumnProvider*							GetColumn(vint column)override;
					Ptr<GuiImageData>									GetRowLargeImage(vint row)override;
					Ptr<GuiImageData>									GetRowSmallImage(vint row)override;
				};

/***********************************************************************
Strong Typed DataSource Extensions
***********************************************************************/

				template<typename TRow>
				class StrongTypedDataProvider;

				template<typename TRow, typename TColumn>
				class StrongTypedColumnProviderBase : public StructuredColummProviderBase
				{
				public:
					class FilterBase : public StructuredDataFilterBase
					{
					protected:
						StrongTypedColumnProviderBase<TRow, TColumn>*		ownerColumn;
						StrongTypedDataProvider<TRow>*						dataProvider;

						virtual bool										FilterData(const TRow& rowData, const TColumn& cellData)=0;
					public:
						FilterBase(StrongTypedColumnProviderBase<TRow, TColumn>* _ownerColumn)
							:ownerColumn(_ownerColumn)
							,dataProvider(_ownerColumn->dataProvider)
						{
						}

						bool Filter(vint row)override
						{
							TRow rowData;
							TColumn cellData;
							dataProvider->GetRowData(row, rowData);
							ownerColumn->GetCellData(rowData, cellData);
							return FilterData(rowData, cellData);
						}
					};

					class SorterBase : public Object, public virtual IStructuredDataSorter
					{
					protected:
						StrongTypedColumnProviderBase<TRow, TColumn>*		ownerColumn;
						StrongTypedDataProvider<TRow>*						dataProvider;

						virtual vint										CompareData(const TRow& rowData1, const TColumn& cellData1, const TRow& rowData2, const TColumn& cellData2)=0;
					public:
						SorterBase(StrongTypedColumnProviderBase<TRow, TColumn>* _ownerColumn)
							:ownerColumn(_ownerColumn)
							,dataProvider(_ownerColumn->dataProvider)
						{
						}

						vint Compare(vint row1, vint row2)
						{
							TRow rowData1, rowData2;
							TColumn cellData1, cellData2;
							dataProvider->GetRowData(row1, rowData1);
							dataProvider->GetRowData(row2, rowData2);
							ownerColumn->GetCellData(rowData1, cellData1);
							ownerColumn->GetCellData(rowData2, cellData2);
							return CompareData(rowData1, cellData1, rowData2, cellData2);
						}
					};

					class Sorter : public SorterBase
					{
					protected:

						vint CompareData(const TRow& rowData1, const TColumn& cellData1, const TRow& rowData2, const TColumn& cellData2)override
						{
							if(cellData1<cellData2) return -1;
							if(cellData1>cellData2) return 1;
							return 0;
						}
					public:
						Sorter(StrongTypedColumnProviderBase<TRow, TColumn>* _ownerColumn)
							:SorterBase(_ownerColumn)
						{
						}
					};

				protected:
					StrongTypedDataProvider<TRow>*						dataProvider;

				public:
					StrongTypedColumnProviderBase(StrongTypedDataProvider<TRow>* _dataProvider)
						:dataProvider(_dataProvider)
					{
					}

					virtual void										GetCellData(const TRow& rowData, TColumn& cellData)=0;
					virtual WString										GetCellDataText(const TColumn& cellData)=0;

					WString GetCellText(vint row)override
					{
						TRow rowData;
						TColumn cellData;
						dataProvider->GetRowData(row, rowData);
						GetCellData(rowData, cellData);
						return GetCellDataText(cellData);
					}
				};

				template<typename TRow, typename TColumn>
				class StrongTypedColumnProvider : public StrongTypedColumnProviderBase<TRow, TColumn>
				{
				public:
					StrongTypedColumnProvider(StrongTypedDataProvider<TRow>* _dataProvider)
						:StrongTypedColumnProviderBase<TRow, TColumn>(_dataProvider)
					{
					}

					WString GetCellDataText(const TColumn& cellData)override
					{
						return description::BoxValue<TColumn>(cellData).GetText();
					}
				};

				template<typename TRow, typename TColumn>
				class StrongTypedFieldColumnProvider : public StrongTypedColumnProvider<TRow, TColumn>
				{
				protected:
					TColumn TRow::*										field;

				public:
					StrongTypedFieldColumnProvider(StrongTypedDataProvider<TRow>* _dataProvider, TColumn TRow::* _field)
						:StrongTypedColumnProvider<TRow, TColumn>(_dataProvider)
						,field(_field)
					{
					}

					void GetCellData(const TRow& rowData, TColumn& cellData)override
					{
						cellData=rowData.*field;
					}
				};

				template<typename TRow>
				class StrongTypedDataProvider : public StructuredDataProviderBase
				{
				protected:

					template<typename TColumn>
					Ptr<StrongTypedColumnProvider<TRow, TColumn>> AddStrongTypedColumn(const WString& text, Ptr<StrongTypedColumnProvider<TRow, TColumn>> column)
					{
						column->SetText(text);
						return AddColumnInternal(column, true)?column:nullptr;
					}

					template<typename TColumn>
					Ptr<StrongTypedColumnProvider<TRow, TColumn>> AddSortableStrongTypedColumn(const WString& text, Ptr<StrongTypedColumnProvider<TRow, TColumn>> column)
					{
						if(AddStrongTypedColumn(text, column))
						{
							typedef typename StrongTypedColumnProvider<TRow, TColumn>::Sorter ColumnSorter;
							column->SetInherentSorter(new ColumnSorter(column.Obj()));
						}
						return column;
					}

					template<typename TColumn>
					Ptr<StrongTypedColumnProvider<TRow, TColumn>> AddFieldColumn(const WString& text, TColumn TRow::* field)
					{
						Ptr<StrongTypedFieldColumnProvider<TRow, TColumn>> column=new StrongTypedFieldColumnProvider<TRow, TColumn>(this, field);
						return AddStrongTypedColumn<TColumn>(text, column);
					}

					template<typename TColumn>
					Ptr<StrongTypedColumnProvider<TRow, TColumn>> AddSortableFieldColumn(const WString& text, TColumn TRow::* field)
					{
						Ptr<StrongTypedFieldColumnProvider<TRow, TColumn>> column=new StrongTypedFieldColumnProvider<TRow, TColumn>(this, field);
						return AddSortableStrongTypedColumn<TColumn>(text, column);
					}
				public:
					StrongTypedDataProvider()
					{
					}

					virtual void										GetRowData(vint row, TRow& rowData)=0;
				};
			}
		}
	}
}

#endif