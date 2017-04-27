#include "GuiDataGridStructured.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{
				using namespace collections;
				
/***********************************************************************
StructuredDataFilterBase
***********************************************************************/

				void StructuredDataFilterBase::InvokeOnFilterChanged()
				{
					if(commandExecutor)
					{
						commandExecutor->OnFilterChanged();
					}
				}

				StructuredDataFilterBase::StructuredDataFilterBase()
					:commandExecutor(0)
				{
				}

				void StructuredDataFilterBase::SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)
				{
					commandExecutor=value;
				}
				
/***********************************************************************
StructuredDataMultipleFilter
***********************************************************************/

				StructuredDataMultipleFilter::StructuredDataMultipleFilter()
				{
				}

				bool StructuredDataMultipleFilter::AddSubFilter(Ptr<IStructuredDataFilter> value)
				{
					if(!value) return false;
					if(filters.Contains(value.Obj())) return false;
					filters.Add(value);
					value->SetCommandExecutor(commandExecutor);
					InvokeOnFilterChanged();
					return true;
				}

				bool StructuredDataMultipleFilter::RemoveSubFilter(Ptr<IStructuredDataFilter> value)
				{
					if(!value) return false;
					if(!filters.Contains(value.Obj())) return false;
					value->SetCommandExecutor(0);
					filters.Remove(value.Obj());
					InvokeOnFilterChanged();
					return true;
				}

				void StructuredDataMultipleFilter::SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)
				{
					StructuredDataFilterBase::SetCommandExecutor(value);
					for(vint i=0;i<filters.Count();i++)
					{
						filters[i]->SetCommandExecutor(value);
					}
				}
				
/***********************************************************************
StructuredDataAndFilter
***********************************************************************/

				StructuredDataAndFilter::StructuredDataAndFilter()
				{
				}

				bool StructuredDataAndFilter::Filter(vint row)
				{
					return From(filters)
						.All([row](Ptr<IStructuredDataFilter> filter)
						{
							return filter->Filter(row);
						});
				}
				
/***********************************************************************
StructuredDataOrFilter
***********************************************************************/
				
				StructuredDataOrFilter::StructuredDataOrFilter()
				{
				}

				bool StructuredDataOrFilter::Filter(vint row)
				{
					return From(filters)
						.Any([row](Ptr<IStructuredDataFilter> filter)
						{
							return filter->Filter(row);
						});
				}
				
/***********************************************************************
StructuredDataNotFilter
***********************************************************************/
				
				StructuredDataNotFilter::StructuredDataNotFilter()
				{
				}

				bool StructuredDataNotFilter::SetSubFilter(Ptr<IStructuredDataFilter> value)
				{
					if(filter==value) return false;
					if(filter)
					{
						filter->SetCommandExecutor(0);
					}
					filter=value;
					if(filter)
					{
						filter->SetCommandExecutor(commandExecutor);
					}
					InvokeOnFilterChanged();
					return true;
				}

				void StructuredDataNotFilter::SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)
				{
					StructuredDataFilterBase::SetCommandExecutor(value);
					if(filter)
					{
						filter->SetCommandExecutor(value);
					}
				}

				bool StructuredDataNotFilter::Filter(vint row)
				{
					return filter?true:!filter->Filter(row);
				}
				
/***********************************************************************
StructuredDataMultipleSorter
***********************************************************************/

				StructuredDataMultipleSorter::StructuredDataMultipleSorter()
				{
				}

				bool StructuredDataMultipleSorter::SetLeftSorter(Ptr<IStructuredDataSorter> value)
				{
					if(leftSorter==value) return false;
					leftSorter=value;
					return true;
				}

				bool StructuredDataMultipleSorter::SetRightSorter(Ptr<IStructuredDataSorter> value)
				{
					if(rightSorter==value) return false;
					rightSorter=value;
					return true;
				}

				vint StructuredDataMultipleSorter::Compare(vint row1, vint row2)
				{
					if(leftSorter)
					{
						vint result=leftSorter->Compare(row1, row2);
						if(result!=0) return result;
					}
					if(rightSorter)
					{
						vint result=rightSorter->Compare(row1, row2);
						if(result!=0) return result;
					}
					return 0;
				}
				
/***********************************************************************
StructuredDataReverseSorter
***********************************************************************/

				StructuredDataReverseSorter::StructuredDataReverseSorter()
				{
				}

				bool StructuredDataReverseSorter::SetSubSorter(Ptr<IStructuredDataSorter> value)
				{
					if(sorter==value) return false;
					sorter=value;
					return true;
				}

				vint StructuredDataReverseSorter::Compare(vint row1, vint row2)
				{
					return sorter?-sorter->Compare(row1, row2):0;
				}
				
/***********************************************************************
StructuredDataProvider
***********************************************************************/

				void StructuredDataProvider::OnDataProviderColumnChanged()
				{
					vint oldRowCount=GetRowCount();
					RebuildFilter(true);
					ReorderRows(true);
				}

				void StructuredDataProvider::OnDataProviderItemModified(vint start, vint count, vint newCount)
				{
					// optimized for cell editing
					if(!currentSorter && !currentFilter)
					{
						if(count!=newCount)
						{
							ReorderRows(false);
						}
						commandExecutor->OnDataProviderItemModified(start, count, newCount);
					}
					else
					{
						ReorderRows(true);
					}
				}

				void StructuredDataProvider::OnFilterChanged()
				{
					ReorderRows(true);
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
				}

				void StructuredDataProvider::RebuildFilter(bool invokeCallback)
				{
					if(currentFilter)
					{
						currentFilter->SetCommandExecutor(0);
						currentFilter=0;
					}

					List<Ptr<IStructuredDataFilter>> selectedFilters;
					CopyFrom(
						selectedFilters,
						Range<vint>(0, GetColumnCount())
							.Select([this](vint column){return structuredDataProvider->GetColumn(column)->GetInherentFilter();})
							.Where([](Ptr<IStructuredDataFilter> filter){return (bool)filter;})
						);
					if(additionalFilter)
					{
						selectedFilters.Add(additionalFilter);
					}
					if(selectedFilters.Count()>0)
					{
						Ptr<StructuredDataAndFilter> andFilter=new StructuredDataAndFilter;
						FOREACH(Ptr<IStructuredDataFilter>, filter, selectedFilters)
						{
							andFilter->AddSubFilter(filter);
						}
						currentFilter=andFilter;
					}

					if(currentFilter)
					{
						currentFilter->SetCommandExecutor(this);
					}

					if(invokeCallback && commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
				}

				void StructuredDataProvider::ReorderRows(bool invokeCallback)
				{
					vint oldRowCount=reorderedRows.Count();
					reorderedRows.Clear();
					vint rowCount=structuredDataProvider->GetRowCount();

					if(currentFilter)
					{
						for(vint i=0;i<rowCount;i++)
						{
							if(currentFilter->Filter(i))
							{
								reorderedRows.Add(i);
							}
						}
					}
					else
					{
						for(vint i=0;i<rowCount;i++)
						{
							reorderedRows.Add(i);
						}
					}

					if(currentSorter && reorderedRows.Count()>0)
					{
						IStructuredDataSorter* sorter=currentSorter.Obj();
						SortLambda(&reorderedRows[0], reorderedRows.Count(), [sorter](vint a, vint b){return sorter->Compare(a, b);});
					}

					if(invokeCallback && commandExecutor)
					{
						commandExecutor->OnDataProviderItemModified(0, oldRowCount, GetRowCount());
					}
				}

				vint StructuredDataProvider::TranslateRowNumber(vint row)
				{
					return reorderedRows[row];
				}

				Ptr<IStructuredDataProvider> StructuredDataProvider::GetStructuredDataProvider()
				{
					return structuredDataProvider;
				}

				Ptr<IStructuredDataFilter> StructuredDataProvider::GetAdditionalFilter()
				{
					return additionalFilter;
				}

				void StructuredDataProvider::SetAdditionalFilter(Ptr<IStructuredDataFilter> value)
				{
					additionalFilter=value;
					RebuildFilter(true);
					ReorderRows(true);
				}

				StructuredDataProvider::StructuredDataProvider(Ptr<IStructuredDataProvider> provider)
					:structuredDataProvider(provider)
					,commandExecutor(0)
				{
					structuredDataProvider->SetCommandExecutor(this);
					RebuildFilter(false);
					ReorderRows(false);
				}

				StructuredDataProvider::~StructuredDataProvider()
				{
				}

				void StructuredDataProvider::SetCommandExecutor(IDataProviderCommandExecutor* value)
				{
					commandExecutor=value;
				}

				description::Value StructuredDataProvider::GetViewModelContext()
				{
					return structuredDataProvider->GetViewModelContext();
				}

				vint StructuredDataProvider::GetColumnCount()
				{
					return structuredDataProvider->GetColumnCount();
				}

				WString StructuredDataProvider::GetColumnText(vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetText();
				}

				vint StructuredDataProvider::GetColumnSize(vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetSize();
				}

				void StructuredDataProvider::SetColumnSize(vint column, vint value)
				{
					structuredDataProvider->GetColumn(column)->SetSize(value);
				}

				GuiMenu* StructuredDataProvider::GetColumnPopup(vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetPopup();
				}

				bool StructuredDataProvider::IsColumnSortable(vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetInherentSorter();
				}

				void StructuredDataProvider::SortByColumn(vint column, bool ascending)
				{
					if(0<=column && column<structuredDataProvider->GetColumnCount())
					{
						Ptr<IStructuredDataSorter> sorter=structuredDataProvider->GetColumn(column)->GetInherentSorter();
						if(!sorter)
						{
							currentSorter=0;
						}
						else if(ascending)
						{
							currentSorter=sorter;
						}
						else
						{
							Ptr<StructuredDataReverseSorter> reverseSorter=new StructuredDataReverseSorter();
							reverseSorter->SetSubSorter(sorter);
							currentSorter=reverseSorter;
						}
					}
					else
					{
						currentSorter=0;
					}
					for(vint i=0;i<structuredDataProvider->GetColumnCount();i++)
					{
						structuredDataProvider->GetColumn(i)->SetSortingState(
							i!=column?GuiListViewColumnHeader::NotSorted:
							ascending?GuiListViewColumnHeader::Ascending:
							GuiListViewColumnHeader::Descending
							);
					}
					ReorderRows(true);
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
				}

				vint StructuredDataProvider::GetSortedColumn()
				{
					for(vint i=0;i<structuredDataProvider->GetColumnCount();i++)
					{
						GuiListViewColumnHeader::ColumnSortingState state=structuredDataProvider->GetColumn(i)->GetSortingState();
						if(state!=GuiListViewColumnHeader::NotSorted)
						{
							return i;
						}
					}
					return -1;
				}

				bool StructuredDataProvider::IsSortOrderAscending()
				{
					for(vint i=0;i<structuredDataProvider->GetColumnCount();i++)
					{
						GuiListViewColumnHeader::ColumnSortingState state=structuredDataProvider->GetColumn(i)->GetSortingState();
						if(state!=GuiListViewColumnHeader::NotSorted)
						{
							return state==GuiListViewColumnHeader::Ascending;
						}
					}
					return true;
				}
					
				vint StructuredDataProvider::GetRowCount()
				{
					return reorderedRows.Count();
				}

				Ptr<GuiImageData> StructuredDataProvider::GetRowLargeImage(vint row)
				{
					return structuredDataProvider->GetRowLargeImage(TranslateRowNumber(row));
				}

				Ptr<GuiImageData> StructuredDataProvider::GetRowSmallImage(vint row)
				{
					return structuredDataProvider->GetRowSmallImage(TranslateRowNumber(row));
				}

				vint StructuredDataProvider::GetCellSpan(vint row, vint column)
				{
					return 1;
				}

				WString StructuredDataProvider::GetCellText(vint row, vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetCellText(TranslateRowNumber(row));
				}

				IDataVisualizerFactory* StructuredDataProvider::GetCellDataVisualizerFactory(vint row, vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetCellDataVisualizerFactory(TranslateRowNumber(row));
				}

				void StructuredDataProvider::VisualizeCell(vint row, vint column, IDataVisualizer* dataVisualizer)
				{
					structuredDataProvider->GetColumn(column)->VisualizeCell(TranslateRowNumber(row), dataVisualizer);
				}

				IDataEditorFactory* StructuredDataProvider::GetCellDataEditorFactory(vint row, vint column)
				{
					return structuredDataProvider->GetColumn(column)->GetCellDataEditorFactory(TranslateRowNumber(row));
				}

				void StructuredDataProvider::BeforeEditCell(vint row, vint column, IDataEditor* dataEditor)
				{
					structuredDataProvider->GetColumn(column)->BeforeEditCell(TranslateRowNumber(row), dataEditor);
				}

				void StructuredDataProvider::SaveCellData(vint row, vint column, IDataEditor* dataEditor)
				{
					structuredDataProvider->GetColumn(column)->SaveCellData(TranslateRowNumber(row), dataEditor);
				}
				
/***********************************************************************
StructuredColummProviderBase
***********************************************************************/

				StructuredColummProviderBase::StructuredColummProviderBase()
					:commandExecutor(0)
					,size(0)
					,sortingState(GuiListViewColumnHeader::NotSorted)
					,popup(0)
				{
				}

				StructuredColummProviderBase::~StructuredColummProviderBase()
				{
				}
					
				void StructuredColummProviderBase::SetCommandExecutor(IDataProviderCommandExecutor* value)
				{
					commandExecutor=value;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetText(const WString& value)
				{
					text=value;
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return this;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetPopup(GuiMenu* value)
				{
					popup=value;
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return this;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetInherentFilter(Ptr<IStructuredDataFilter> value)
				{
					inherentFilter=value;
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return this;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetInherentSorter(Ptr<IStructuredDataSorter> value)
				{
					inherentSorter=value;
					if(commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return this;
				}

				Ptr<IDataVisualizerFactory> StructuredColummProviderBase::GetVisualizerFactory()
				{
					return visualizerFactory;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetVisualizerFactory(Ptr<IDataVisualizerFactory> value)
				{
					visualizerFactory=value;
					return this;
				}

				Ptr<IDataEditorFactory> StructuredColummProviderBase::GetEditorFactory()
				{
					return editorFactory;
				}

				StructuredColummProviderBase* StructuredColummProviderBase::SetEditorFactory(Ptr<IDataEditorFactory> value)
				{
					editorFactory=value;
					return this;
				}

				WString StructuredColummProviderBase::GetText()
				{
					return text;
				}

				vint StructuredColummProviderBase::GetSize()
				{
					return size;
				}

				void StructuredColummProviderBase::SetSize(vint value)
				{
					size=value;
				}

				GuiListViewColumnHeader::ColumnSortingState StructuredColummProviderBase::GetSortingState()
				{
					return sortingState;
				}

				void StructuredColummProviderBase::SetSortingState(GuiListViewColumnHeader::ColumnSortingState value)
				{
					sortingState=value;
				}

				GuiMenu* StructuredColummProviderBase::GetPopup()
				{
					return popup;
				}

				Ptr<IStructuredDataFilter> StructuredColummProviderBase::GetInherentFilter()
				{
					return inherentFilter;
				}

				Ptr<IStructuredDataSorter> StructuredColummProviderBase::GetInherentSorter()
				{
					return inherentSorter;
				}
					
				IDataVisualizerFactory* StructuredColummProviderBase::GetCellDataVisualizerFactory(vint row)
				{
					return visualizerFactory.Obj();
				}

				void StructuredColummProviderBase::VisualizeCell(vint row, IDataVisualizer* dataVisualizer)
				{
				}

				IDataEditorFactory* StructuredColummProviderBase::GetCellDataEditorFactory(vint row)
				{
					return editorFactory.Obj();
				}

				void StructuredColummProviderBase::BeforeEditCell(vint row, IDataEditor* dataEditor)
				{
				}

				void StructuredColummProviderBase::SaveCellData(vint row, IDataEditor* dataEditor)
				{
				}
				
/***********************************************************************
StructuredDataProviderBase
***********************************************************************/

				bool StructuredDataProviderBase::InsertColumnInternal(vint column, Ptr<StructuredColummProviderBase> value, bool callback)
				{
					if(column<0 || columns.Count()<column) return false;
					if(columns.Contains(value.Obj())) return false;
					columns.Insert(column, value);
					value->SetCommandExecutor(commandExecutor);
					if(callback && commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return true;
				}

				bool StructuredDataProviderBase::AddColumnInternal(Ptr<StructuredColummProviderBase> value, bool callback)
				{
					return InsertColumnInternal(columns.Count(), value, callback);
				}

				bool StructuredDataProviderBase::RemoveColumnInternal(Ptr<StructuredColummProviderBase> value, bool callback)
				{
					if(!columns.Contains(value.Obj())) return false;
					value->SetCommandExecutor(0);
					columns.Remove(value.Obj());
					if(callback && commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return true;
				}

				bool StructuredDataProviderBase::ClearColumnsInternal(bool callback)
				{
					FOREACH(Ptr<StructuredColummProviderBase>, column, columns)
					{
						column->SetCommandExecutor(0);
					}
					columns.Clear();
					if(callback && commandExecutor)
					{
						commandExecutor->OnDataProviderColumnChanged();
					}
					return true;
				}

				StructuredDataProviderBase::StructuredDataProviderBase()
					:commandExecutor(0)
				{
				}

				StructuredDataProviderBase::~StructuredDataProviderBase()
				{
				}

				void StructuredDataProviderBase::SetCommandExecutor(IDataProviderCommandExecutor* value)
				{
					commandExecutor=value;
					FOREACH(Ptr<StructuredColummProviderBase>, column, columns)
					{
						column->SetCommandExecutor(commandExecutor);
					}
				}

				description::Value StructuredDataProviderBase::GetViewModelContext()
				{
					return description::Value();
				}

				vint StructuredDataProviderBase::GetColumnCount()
				{
					return columns.Count();
				}

				IStructuredColumnProvider* StructuredDataProviderBase::GetColumn(vint column)
				{
					return columns[column].Obj();
				}

				Ptr<GuiImageData> StructuredDataProviderBase::GetRowLargeImage(vint row)
				{
					return 0;
				}

				Ptr<GuiImageData> StructuredDataProviderBase::GetRowSmallImage(vint row)
				{
					return 0;
				}

/***********************************************************************
GuiBindableDataColumn
***********************************************************************/

				BindableDataColumn::BindableDataColumn()
					:dataProvider(nullptr)
				{
				}

				BindableDataColumn::~BindableDataColumn()
				{
				}

				void BindableDataColumn::SaveCellData(vint row, IDataEditor* dataEditor)
				{
					if (auto editor = dynamic_cast<GuiBindableDataEditor*>(dataEditor))
					{
						SetCellValue(row, editor->GetEditedCellValue());
					}
					if (commandExecutor)
					{
						commandExecutor->OnDataProviderItemModified(row, 1, 1);
					}
				}

				WString BindableDataColumn::GetCellText(vint row)
				{
					if (dataProvider->itemSource)
					{
						if (0 <= row && row < dataProvider->itemSource->GetCount())
						{
							return ReadProperty(dataProvider->itemSource->Get(row), textProperty);
						}
					}
					return L"";
				}

				description::Value BindableDataColumn::GetCellValue(vint row)
				{
					if (dataProvider->itemSource)
					{
						if (0 <= row && row < dataProvider->itemSource->GetCount())
						{
							return ReadProperty(dataProvider->itemSource->Get(row), valueProperty);
						}
					}
					return Value();
				}

				void BindableDataColumn::SetCellValue(vint row, description::Value value)
				{
					if (dataProvider->itemSource)
					{
						if (0 <= row && row < dataProvider->itemSource->GetCount())
						{
							auto rowValue = dataProvider->itemSource->Get(row);
							return WriteProperty(rowValue, valueProperty, value);
						}
					}
				}

				ItemProperty<WString> BindableDataColumn::GetTextProperty()
				{
					return textProperty;
				}

				void BindableDataColumn::SetTextProperty(const ItemProperty<WString>& value)
				{
					if (textProperty != value)
					{
						textProperty = value;
						if (commandExecutor)
						{
							commandExecutor->OnDataProviderColumnChanged();
						}
						compositions::GuiEventArgs arguments;
						TextPropertyChanged.Execute(arguments);
					}
				}

				WritableItemProperty<Value> BindableDataColumn::GetValueProperty()
				{
					return valueProperty;
				}

				void BindableDataColumn::SetValueProperty(const WritableItemProperty<Value>& value)
				{
					if (valueProperty != value)
					{
						valueProperty = value;
						if (commandExecutor)
						{
							commandExecutor->OnDataProviderColumnChanged();
						}
						compositions::GuiEventArgs arguments;
						ValuePropertyChanged.Execute(arguments);
					}
				}

				description::Value BindableDataColumn::GetViewModelContext()
				{
					return dataProvider->viewModelContext;
				}

/***********************************************************************
GuiBindableDataProvider
***********************************************************************/

				BindableDataProvider::BindableDataProvider(const description::Value& _viewModelContext)
					:viewModelContext(_viewModelContext)
				{
				}

				BindableDataProvider::~BindableDataProvider()
				{
					SetItemSource(nullptr);
				}

				Ptr<description::IValueEnumerable> BindableDataProvider::GetItemSource()
				{
					return itemSource;
				}

				void BindableDataProvider::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
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
								commandExecutor->OnDataProviderItemModified(start, oldCount, newCount);
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

					commandExecutor->OnDataProviderItemModified(0, oldCount, itemSource ? itemSource->GetCount() : 0);
				}

				vint BindableDataProvider::GetRowCount()
				{
					if (!itemSource) return 0;
					return itemSource->GetCount();
				}

				description::Value BindableDataProvider::GetRowValue(vint row)
				{
					if (itemSource)
					{
						if (0 <= row && row < itemSource->GetCount())
						{
							return itemSource->Get(row);
						}
					}
					return Value();
				}

				bool BindableDataProvider::InsertBindableColumn(vint index, Ptr<BindableDataColumn> column)
				{
					if (InsertColumnInternal(index, column, true))
					{
						column->dataProvider = this;
						return true;
					}
					else
					{
						return false;
					}
				}

				bool BindableDataProvider::AddBindableColumn(Ptr<BindableDataColumn> column)
				{
					if (AddColumnInternal(column, true))
					{
						column->dataProvider = this;
						return true;
					}
					else
					{
						return false;
					}
				}

				bool BindableDataProvider::RemoveBindableColumn(Ptr<BindableDataColumn> column)
				{
					if (RemoveColumnInternal(column, true))
					{
						column->dataProvider = nullptr;
						return true;
					}
					else
					{
						return false;
					}
				}

				bool BindableDataProvider::ClearBindableColumns()
				{
					FOREACH(Ptr<StructuredColummProviderBase>, column, columns)
					{
						column.Cast<BindableDataColumn>()->dataProvider = nullptr;
					}
					return ClearColumnsInternal(true);
				}

				Ptr<BindableDataColumn> BindableDataProvider::GetBindableColumn(vint index)
				{
					if (0 <= index && index < GetColumnCount())
					{
						return columns[index].Cast<BindableDataColumn>();
					}
					else
					{
						return nullptr;
					}
				}

				description::Value BindableDataProvider::GetViewModelContext()
				{
					return viewModelContext;
				}
			}

/***********************************************************************
GuiBindableDataGrid
***********************************************************************/

			GuiBindableDataGrid::GuiBindableDataGrid(IStyleProvider* _styleProvider, const description::Value& _viewModelContext)
				:GuiVirtualDataGrid(_styleProvider, new BindableDataProvider(_viewModelContext))
			{
				bindableDataProvider = GetStructuredDataProvider()->GetStructuredDataProvider().Cast<BindableDataProvider>();
			}

			GuiBindableDataGrid::~GuiBindableDataGrid()
			{
			}

			Ptr<description::IValueEnumerable> GuiBindableDataGrid::GetItemSource()
			{
				return bindableDataProvider->GetItemSource();
			}

			void GuiBindableDataGrid::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				bindableDataProvider->SetItemSource(_itemSource);
			}

			bool GuiBindableDataGrid::InsertBindableColumn(vint index, Ptr<list::BindableDataColumn> column)
			{
				return bindableDataProvider->InsertBindableColumn(index, column);
			}

			bool GuiBindableDataGrid::AddBindableColumn(Ptr<list::BindableDataColumn> column)
			{
				return bindableDataProvider->AddBindableColumn(column);
			}

			bool GuiBindableDataGrid::RemoveBindableColumn(Ptr<list::BindableDataColumn> column)
			{
				return bindableDataProvider->RemoveBindableColumn(column);
			}

			bool GuiBindableDataGrid::ClearBindableColumns()
			{
				return bindableDataProvider->ClearBindableColumns();
			}

			Ptr<list::BindableDataColumn> GuiBindableDataGrid::GetBindableColumn(vint index)
			{
				return bindableDataProvider->GetBindableColumn(index);
			}

			description::Value GuiBindableDataGrid::GetSelectedRowValue()
			{
				return bindableDataProvider->GetRowValue(GetSelectedCell().row);
			}

			description::Value GuiBindableDataGrid::GetSelectedCellValue()
			{
				auto cell = GetSelectedCell();
				auto column = GetBindableColumn(cell.column);
				if (column)
				{
					return column->GetCellValue(cell.row);
				}
				return Value();
			}
		}
	}
}