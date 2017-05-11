#include "GuiBindableDataGrid.h"
#include "GuiBindableListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace description;
			using namespace templates;

			namespace list
			{

/***********************************************************************
DataFilterBase
***********************************************************************/

				void DataFilterBase::InvokeOnFilterChanged()
				{
					if (callback)
					{
						callback->OnFilterChanged();
					}
				}

				DataFilterBase::DataFilterBase()
				{
				}

				void DataFilterBase::SetCallback(IDataFilterCallback* value)
				{
					callback = value;
				}

/***********************************************************************
DataMultipleFilter
***********************************************************************/

				DataMultipleFilter::DataMultipleFilter()
				{
				}

				bool DataMultipleFilter::AddSubFilter(Ptr<IDataFilter> value)
				{
					if (!value) return false;
					if (filters.Contains(value.Obj())) return false;
					filters.Add(value);
					value->SetCallback(callback);
					InvokeOnFilterChanged();
					return true;
				}

				bool DataMultipleFilter::RemoveSubFilter(Ptr<IDataFilter> value)
				{
					if (!value) return false;
					if (!filters.Contains(value.Obj())) return false;
					value->SetCallback(nullptr);
					filters.Remove(value.Obj());
					InvokeOnFilterChanged();
					return true;
				}

				void DataMultipleFilter::SetCallback(IDataFilterCallback* value)
				{
					DataFilterBase::SetCallback(value);
					for (vint i = 0; i < filters.Count(); i++)
					{
						filters[i]->SetCallback(value);
					}
				}

/***********************************************************************
DataAndFilter
***********************************************************************/

				DataAndFilter::DataAndFilter()
				{
				}

				bool DataAndFilter::Filter(vint row)
				{
					return From(filters)
						.All([row](Ptr<IDataFilter> filter)
					{
						return filter->Filter(row);
					});
				}

/***********************************************************************
DataOrFilter
***********************************************************************/

				DataOrFilter::DataOrFilter()
				{
				}

				bool DataOrFilter::Filter(vint row)
				{
					return From(filters)
						.Any([row](Ptr<IDataFilter> filter)
					{
						return filter->Filter(row);
					});
				}

/***********************************************************************
DataNotFilter
***********************************************************************/

				DataNotFilter::DataNotFilter()
				{
				}

				bool DataNotFilter::SetSubFilter(Ptr<IDataFilter> value)
				{
					if (filter == value) return false;
					if (filter)
					{
						filter->SetCallback(nullptr);
					}
					filter = value;
					if (filter)
					{
						filter->SetCallback(callback);
					}
					InvokeOnFilterChanged();
					return true;
				}

				void DataNotFilter::SetCallback(IDataFilterCallback* value)
				{
					DataFilterBase::SetCallback(value);
					if (filter)
					{
						filter->SetCallback(value);
					}
				}

				bool DataNotFilter::Filter(vint row)
				{
					return filter ? true : !filter->Filter(row);
				}

/***********************************************************************
DataMultipleSorter
***********************************************************************/

				DataMultipleSorter::DataMultipleSorter()
				{
				}

				bool DataMultipleSorter::SetLeftSorter(Ptr<IDataSorter> value)
				{
					if (leftSorter == value) return false;
					leftSorter = value;
					return true;
				}

				bool DataMultipleSorter::SetRightSorter(Ptr<IDataSorter> value)
				{
					if (rightSorter == value) return false;
					rightSorter = value;
					return true;
				}

				vint DataMultipleSorter::Compare(vint row1, vint row2)
				{
					if (leftSorter)
					{
						vint result = leftSorter->Compare(row1, row2);
						if (result != 0) return result;
					}
					if (rightSorter)
					{
						vint result = rightSorter->Compare(row1, row2);
						if (result != 0) return result;
					}
					return 0;
				}

/***********************************************************************
DataReverseSorter
***********************************************************************/

				DataReverseSorter::DataReverseSorter()
				{
				}

				bool DataReverseSorter::SetSubSorter(Ptr<IDataSorter> value)
				{
					if (sorter == value) return false;
					sorter = value;
					return true;
				}

				vint DataReverseSorter::Compare(vint row1, vint row2)
				{
					return sorter ? -sorter->Compare(row1, row2) : 0;
				}

/***********************************************************************
DataColumn
***********************************************************************/

				void DataColumn::NotifyAllColumnsUpdate()
				{
					if (dataProvider)
					{
						dataProvider->NotifyAllColumnsUpdate();
					}
				}

				DataColumn::DataColumn()
				{
				}

				DataColumn::~DataColumn()
				{
				}

				WString DataColumn::GetText()
				{
					return text;
				}

				void DataColumn::SetText(const WString& value)
				{
					if (text != value)
					{
						text = value;
						NotifyAllColumnsUpdate();
					}
				}

				vint DataColumn::GetSize()
				{
					return size;
				}

				void DataColumn::SetSize(vint value)
				{
					size = value;
				}

				GuiMenu* DataColumn::GetPopup()
				{
					return popup;
				}

				void DataColumn::SetPopup(GuiMenu* value)
				{
					if (popup != value)
					{
						popup = value;
						NotifyAllColumnsUpdate();
					}
				}

				Ptr<IDataFilter> DataColumn::GetInherentFilter()
				{
					return inherentFilter;
				}

				void DataColumn::SetInherentFilter(Ptr<IDataFilter> value)
				{
					inherentFilter = value;
					NotifyAllColumnsUpdate();
				}

				Ptr<IDataSorter> DataColumn::GetInherentSorter()
				{
					return inherentSorter;
				}

				void DataColumn::SetInherentSorter(Ptr<IDataSorter> value)
				{
					inherentSorter = value;
					NotifyAllColumnsUpdate();
				}

				Ptr<IDataVisualizerFactory> DataColumn::GetVisualizerFactory()
				{
					return visualizerFactory;
				}

				void DataColumn::SetVisualizerFactory(Ptr<IDataVisualizerFactory> value)
				{
					visualizerFactory = value;
					NotifyAllColumnsUpdate();
				}

				Ptr<IDataEditorFactory> DataColumn::GetEditorFactory()
				{
					return editorFactory;
				}

				void DataColumn::SetEditorFactory(Ptr<IDataEditorFactory> value)
				{
					editorFactory = value;
					NotifyAllColumnsUpdate();
				}

				WString DataColumn::GetCellText(vint row)
				{
					if (0 <= row && row < dataProvider->Count())
					{
						return ReadProperty(dataProvider->GetBindingValue(row), textProperty);
					}
					return L"";
				}

				description::Value DataColumn::GetCellValue(vint row)
				{
					if (0 <= row && row < dataProvider->Count())
					{
						return ReadProperty(dataProvider->GetBindingValue(row), valueProperty);
					}
					return Value();
				}

				void DataColumn::SetCellValue(vint row, description::Value value)
				{
					if (0 <= row && row < dataProvider->Count())
					{
						auto rowValue = dataProvider->GetBindingValue(row);
						return WriteProperty(rowValue, valueProperty, value);
					}
				}

				ItemProperty<WString> DataColumn::GetTextProperty()
				{
					return textProperty;
				}

				void DataColumn::SetTextProperty(const ItemProperty<WString>& value)
				{
					if (textProperty != value)
					{
						textProperty = value;
						NotifyAllColumnsUpdate();
						compositions::GuiEventArgs arguments;
						TextPropertyChanged.Execute(arguments);
					}
				}

				WritableItemProperty<description::Value> DataColumn::GetValueProperty()
				{
					return valueProperty;
				}

				void DataColumn::SetValueProperty(const WritableItemProperty<description::Value>& value)
				{
					if (valueProperty != value)
					{
						valueProperty = value;
						NotifyAllColumnsUpdate();
						compositions::GuiEventArgs arguments;
						ValuePropertyChanged.Execute(arguments);
					}
				}

/***********************************************************************
DataColumns
***********************************************************************/

				void DataColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					dataProvider->NotifyAllColumnsUpdate();
				}

				bool DataColumns::QueryInsert(vint index, const Ptr<DataColumn>& value)
				{
					return !items.Contains(value.Obj());
				}

				void DataColumns::AfterInsert(vint index, const Ptr<DataColumn>& value)
				{
					value->dataProvider = dataProvider;
				}

				void DataColumns::BeforeRemove(vint index, const Ptr<DataColumn>& value)
				{
					value->dataProvider = nullptr;
				}

				DataColumns::DataColumns(DataProvider* _dataProvider)
					:dataProvider(_dataProvider)
				{
				}

				DataColumns::~DataColumns()
				{
				}

/***********************************************************************
DataProvider
***********************************************************************/

				void DataProvider::NotifyAllItemsUpdate()
				{
					InvokeOnItemModified(0, Count(), Count());
				}

				void DataProvider::NotifyAllColumnsUpdate()
				{
					NotifyAllItemsUpdate();
					if (columnItemViewCallback)
					{
						columnItemViewCallback->OnColumnChanged();
					}
				}

				GuiListControl::IItemProvider* DataProvider::GetItemProvider()
				{
					return this;
				}

				void DataProvider::OnFilterChanged()
				{
					ReorderRows(true);
				}

				void DataProvider::OnItemSourceModified(vint start, vint count, vint newCount)
				{
					if (!currentSorter && !currentFilter && count == newCount)
					{
						InvokeOnItemModified(start, count, newCount);
					}
					else
					{
						ReorderRows(true);
					}
				}

				ListViewDataColumns& DataProvider::GetDataColumns()
				{
					return dataColumns;
				}

				DataColumns& DataProvider::GetColumns()
				{
					return columns;
				}

				Ptr<description::IValueEnumerable> DataProvider::GetItemSource()
				{
					return itemSource;
				}

				void DataProvider::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
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
								OnItemSourceModified(start, oldCount, newCount);
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

					OnItemSourceModified(0, oldCount, itemSource ? itemSource->GetCount() : 0);
				}

				void DataProvider::RebuildFilter()
				{
					if (currentFilter)
					{
						currentFilter->SetCallback(nullptr);
						currentFilter = nullptr;
					}

					List<Ptr<IDataFilter>> selectedFilters;
					CopyFrom(
						selectedFilters,
						From(columns)
						.Select([](Ptr<DataColumn> column) {return column->GetInherentFilter(); })
						.Where([](Ptr<IDataFilter> filter) {return (bool)filter; })
					);
					if (additionalFilter)
					{
						selectedFilters.Add(additionalFilter);
					}
					if (selectedFilters.Count() > 0)
					{
						auto andFilter = MakePtr<DataAndFilter>();
						FOREACH(Ptr<IDataFilter>, filter, selectedFilters)
						{
							andFilter->AddSubFilter(filter);
						}
						currentFilter = andFilter;
					}

					if (currentFilter)
					{
						currentFilter->SetCallback(this);
					}
				}

				void DataProvider::ReorderRows(bool invokeCallback)
				{
					vint oldRowCount = virtualRowToSourceRow.Count();
					virtualRowToSourceRow.Clear();
					vint rowCount = itemSource ? itemSource->GetCount() : 0;

					if (currentFilter)
					{
						for (vint i = 0; i < rowCount; i++)
						{
							if (currentFilter->Filter(i))
							{
								virtualRowToSourceRow.Add(i);
							}
						}
					}
					else
					{
						for (vint i = 0; i < rowCount; i++)
						{
							virtualRowToSourceRow.Add(i);
						}
					}

					if (currentSorter && virtualRowToSourceRow.Count() > 0)
					{
						IDataSorter* sorter = currentSorter.Obj();
						SortLambda(&virtualRowToSourceRow[0], virtualRowToSourceRow.Count(), [sorter](vint a, vint b) {return sorter->Compare(a, b); });
					}

					if (invokeCallback)
					{
						NotifyAllItemsUpdate();
					}
				}

				DataProvider::DataProvider(const description::Value& _viewModelContext)
					:dataColumns(this)
					, columns(this)
					, viewModelContext(_viewModelContext)
				{
					RebuildFilter();
					ReorderRows(false);
				}

				DataProvider::~DataProvider()
				{
				}

				Ptr<IDataFilter> DataProvider::GetAdditionalFilter()
				{
					return additionalFilter;
				}

				void DataProvider::SetAdditionalFilter(Ptr<IDataFilter> value)
				{
					additionalFilter = value;
					RebuildFilter();
					ReorderRows(true);
				}

				// ===================== GuiListControl::IItemProvider =====================

				vint DataProvider::Count()
				{
					return virtualRowToSourceRow.Count();
				}

				WString DataProvider::GetTextValue(vint itemIndex)
				{
					return GetText(itemIndex);
				}

				description::Value DataProvider::GetBindingValue(vint itemIndex)
				{
					return itemSource ? itemSource->Get(virtualRowToSourceRow[itemIndex]) : Value();
				}

				IDescriptable* DataProvider::RequestView(const WString& identifier)
				{
					if (identifier == IListViewItemView::Identifier)
					{
						return (IListViewItemView*)this;
					}
					else if (identifier == ListViewColumnItemArranger::IColumnItemView::Identifier)
					{
						return (ListViewColumnItemArranger::IColumnItemView*)this;
					}
					else if (identifier == IDataGridView::Identifier)
					{
						return (IDataGridView*)this;
					}
					else
					{
						return nullptr;
					}
				}

				// ===================== list::IListViewItemProvider =====================

				Ptr<GuiImageData> DataProvider::GetSmallImage(vint itemIndex)
				{
					if (0 <= itemIndex && itemIndex < Count())
					{
						return ReadProperty(GetBindingValue(itemIndex), smallImageProperty);
					}
					return nullptr;
				}

				Ptr<GuiImageData> DataProvider::GetLargeImage(vint itemIndex)
				{
					if (0 <= itemIndex && itemIndex < Count())
					{
						return ReadProperty(GetBindingValue(itemIndex), largeImageProperty);
					}
					return nullptr;
				}

				WString DataProvider::GetText(vint itemIndex)
				{
					if (columns.Count() == 0)return L"";
					return columns[0]->GetCellText(itemIndex);
				}

				WString DataProvider::GetSubItem(vint itemIndex, vint index)
				{
					return columns[index + 1]->GetCellText(itemIndex);
				}

				vint DataProvider::GetDataColumnCount()
				{
					return dataColumns.Count();
				}

				vint DataProvider::GetDataColumn(vint index)
				{
					return dataColumns[index];
				}

				vint DataProvider::GetColumnCount()
				{
					return columns.Count();
				}

				WString DataProvider::GetColumnText(vint index)
				{
					return columns[index]->GetText();
				}

				// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================

				bool DataProvider::AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					if (columnItemViewCallback)return false;
					columnItemViewCallback = value;
					return true;
				}

				bool DataProvider::DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					if (!columnItemViewCallback) return false;
					columnItemViewCallback = nullptr;
					return true;
				}

				vint DataProvider::GetColumnSize(vint index)
				{
					return columns[index]->GetSize();
				}

				void DataProvider::SetColumnSize(vint index, vint value)
				{
					columns[index]->SetSize(value);
				}

				GuiMenu* DataProvider::GetDropdownPopup(vint index)
				{
					return columns[index]->GetPopup();
				}

				ColumnSortingState DataProvider::GetSortingState(vint index)
				{
					return columns[index]->sortingState;
				}

				// ===================== list::IDataGridView =====================

				description::Value DataProvider::GetViewModelContext()
				{
					return viewModelContext;
				}

				bool DataProvider::IsColumnSortable(vint column)
				{
					return columns[column]->GetInherentSorter();
				}

				void DataProvider::SortByColumn(vint column, bool ascending)
				{
					if (0 <= column && column < columns.Count())
					{
						auto sorter = columns[column]->GetInherentSorter();
						if (!sorter)
						{
							currentSorter = nullptr;
						}
						else if (ascending)
						{
							currentSorter = sorter;
						}
						else
						{
							Ptr<DataReverseSorter> reverseSorter = new DataReverseSorter();
							reverseSorter->SetSubSorter(sorter);
							currentSorter = reverseSorter;
						}
					}
					else
					{
						currentSorter = nullptr;
					}

					for (vint i = 0; i < columns.Count(); i++)
					{
						columns[column]->sortingState =
							i != column ? ColumnSortingState::NotSorted :
							ascending ? ColumnSortingState::Ascending :
							ColumnSortingState::Descending
							;
					}
					ReorderRows(true);
				}

				vint DataProvider::GetSortedColumn()
				{
					for (vint i = 0; i < columns.Count(); i++)
					{
						auto state = columns[i]->sortingState;
						if (state != ColumnSortingState::NotSorted)
						{
							return i;
						}
					}
					return -1;
				}

				bool DataProvider::IsSortOrderAscending()
				{
					for (vint i = 0; i < columns.Count(); i++)
					{
						auto state = columns[i]->sortingState;
						if (state != ColumnSortingState::NotSorted)
						{
							return state == ColumnSortingState::Ascending;
						}
					}
					return true;
				}

				vint DataProvider::GetCellSpan(vint row, vint column)
				{
					return 1;
				}

				IDataVisualizerFactory* DataProvider::GetCellDataVisualizerFactory(vint row, vint column)
				{
					return columns[row]->GetVisualizerFactory().Obj();
				}

				IDataEditorFactory* DataProvider::GetCellDataEditorFactory(vint row, vint column)
				{
					return columns[row]->GetEditorFactory().Obj();
				}

				description::Value DataProvider::GetBindingCellValue(vint row, vint column)
				{
					return columns[column]->GetCellValue(row);
				}

				void DataProvider::SaveCellData(vint row, vint column, IDataEditor* dataEditor)
				{
					if (auto dataTemplate = dynamic_cast<GuiGridEditorTemplate*>(dataEditor->GetTemplate()))
					{
						columns[column]->SetCellValue(row, dataTemplate->GetCellValue());
					}
				}
			}

/***********************************************************************
GuiBindableDataGrid
***********************************************************************/

			GuiBindableDataGrid::GuiBindableDataGrid(IStyleProvider* _styleProvider, const description::Value& _viewModelContext)
				:GuiVirtualDataGrid(_styleProvider, new list::DataProvider(_viewModelContext))
			{
				dataProvider = dynamic_cast<list::DataProvider*>(GetItemProvider());
			}

			GuiBindableDataGrid::~GuiBindableDataGrid()
			{
			}
			
			list::ListViewDataColumns& GuiBindableDataGrid::GetDataColumns()
			{
				return dataProvider->GetDataColumns();
			}

			list::DataColumns& GuiBindableDataGrid::GetColumns()
			{
				return dataProvider->GetColumns();
			}

			Ptr<description::IValueEnumerable> GuiBindableDataGrid::GetItemSource()
			{
				return dataProvider->GetItemSource();
			}

			void GuiBindableDataGrid::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				dataProvider->SetItemSource(_itemSource);
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableDataGrid::GetLargeImageProperty()
			{
				return dataProvider->largeImageProperty;
			}

			void GuiBindableDataGrid::SetLargeImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (dataProvider->largeImageProperty != value)
				{
					dataProvider->largeImageProperty = value;
					dataProvider->NotifyAllItemsUpdate();
					LargeImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableDataGrid::GetSmallImageProperty()
			{
				return dataProvider->smallImageProperty;
			}

			void GuiBindableDataGrid::SetSmallImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (dataProvider->smallImageProperty != value)
				{
					dataProvider->smallImageProperty = value;
					dataProvider->NotifyAllItemsUpdate();
					SmallImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableDataGrid::GetSelectedRowValue()
			{
				auto pos = GetSelectedCell();
				if (pos.row == -1 || pos.column == -1)
				{
					return Value();
				}
				return dataProvider->GetBindingValue(GetSelectedCell().row);
			}

			description::Value GuiBindableDataGrid::GetSelectedCellValue()
			{
				auto pos = GetSelectedCell();
				if (pos.row == -1 || pos.column == -1)
				{
					return Value();
				}
				return dataProvider->GetColumns()[pos.column]->GetCellValue(pos.row);
			}
		}
	}
}