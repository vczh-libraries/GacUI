#include "GuiDataGridControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{
				using namespace compositions;
				using namespace collections;
				using namespace description;

				const wchar_t* const IDataGridView::Identifier = L"vl::presentation::controls::list::IDataGridView";
				
/***********************************************************************
DataGridContentProvider::ItemContent
***********************************************************************/

				void DataGridContentProvider::ItemContent::RemoveCellsAndDataVisualizers()
				{
					for(vint i=0;i<dataVisualizers.Count();i++)
					{
						IDataVisualizer* visualizer=dataVisualizers[i].Obj();
						GuiGraphicsComposition* composition=visualizer->GetBoundsComposition();
						if(composition->GetParent())
						{
							composition->GetParent()->RemoveChild(composition);
						}
						dataVisualizers[i]=0;
					}
					dataVisualizers.Resize(0);

					for(vint i=0;i<textTable->GetColumns();i++)
					{
						GuiCellComposition* cell=textTable->GetSitedCell(0, i);
						SafeDeleteComposition(cell);
					}
				}

				IDataVisualizerFactory* DataGridContentProvider::ItemContent::GetDataVisualizerFactory(vint row, vint column)
				{
					IDataVisualizerFactory* factory=contentProvider->dataProvider->GetCellDataVisualizerFactory(row, column);
					if(factory)
					{
						return factory;
					}
					else if(column==0)
					{
						return &contentProvider->mainColumnDataVisualizerFactory;
					}
					else
					{
						return &contentProvider->subColumnDataVisualizerFactory;
					}
				}

				vint DataGridContentProvider::ItemContent::GetCellColumnIndex(compositions::GuiGraphicsComposition* composition)
				{
					for(vint i=0;i<textTable->GetColumns();i++)
					{
						GuiCellComposition* cell=textTable->GetSitedCell(0, i);
						if(composition==cell)
						{
							return i;
						}
					}
					return -1;
				}

				void DataGridContentProvider::ItemContent::OnCellButtonUp(compositions::GuiGraphicsComposition* sender, bool openEditor)
				{
					vint index=GetCellColumnIndex(sender);
					if(index!=-1)
					{
						if(currentEditor && contentProvider->currentCell.column==index)
						{
							return;
						}
						GuiListControl::IItemStyleController* itemStyle=contentProvider->listViewItemStyleProvider->GetItemStyleControllerFromItemContent(this);
						vint currentRow=contentProvider->dataGrid->GetArranger()->GetVisibleIndex(itemStyle);
						IDataEditorFactory* factory=openEditor?contentProvider->dataProvider->GetCellDataEditorFactory(currentRow, index):0;
						currentEditor=contentProvider->OpenEditor(currentRow, index, factory);
						if(currentEditor)
						{
							GuiCellComposition* cell=dynamic_cast<GuiCellComposition*>(sender);
							currentEditor->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(currentEditor->GetBoundsComposition());
						}
					}
				}

				bool DataGridContentProvider::ItemContent::IsInEditor(compositions::GuiMouseEventArgs& arguments)
				{
					if(!contentProvider->currentEditor) return false;
					GuiGraphicsComposition* editorComposition=contentProvider->currentEditor->GetBoundsComposition();
					GuiGraphicsComposition* stopComposition=GetContentComposition();
					GuiGraphicsComposition* currentComposition=arguments.eventSource;

					while(currentComposition)
					{
						if(currentComposition==editorComposition)
						{
							arguments.handled=true;
							return true;
						}
						else if(currentComposition==stopComposition)
						{
							break;
						}
						else
						{
							currentComposition=currentComposition->GetParent();
						}
					}
					return false;
				}

				void DataGridContentProvider::ItemContent::OnCellButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					IsInEditor(arguments);
				}

				void DataGridContentProvider::ItemContent::OnCellLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(!IsInEditor(arguments))
					{
						if(contentProvider->dataGrid->GetVisuallyEnabled())
						{
							OnCellButtonUp(sender, true);
						}
					}
				}

				void DataGridContentProvider::ItemContent::OnCellRightButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(!IsInEditor(arguments))
					{
						if(contentProvider->dataGrid->GetVisuallyEnabled())
						{
							OnCellButtonUp(sender, false);
						}
					}
				}

				DataGridContentProvider::ItemContent::ItemContent(DataGridContentProvider* _contentProvider, const FontProperties& _font)
					:contentComposition(0)
					,contentProvider(_contentProvider)
					,font(_font)
					,currentEditor(0)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					textTable=new GuiTableComposition;
					textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
					textTable->SetRowsAndColumns(1, 1);
					textTable->SetRowOption(0, GuiCellOption::MinSizeOption());
					textTable->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
					contentComposition->AddChild(textTable);
				}

				DataGridContentProvider::ItemContent::~ItemContent()
				{
					RemoveCellsAndDataVisualizers();
				}

				compositions::GuiBoundsComposition* DataGridContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* DataGridContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void DataGridContentProvider::ItemContent::UpdateSubItemSize()
				{
					vint columnCount=contentProvider->columnItemView->GetColumnCount();
					if(columnCount>textTable->GetColumns())
					{
						columnCount=textTable->GetColumns();
					}
					for(vint i=0;i<columnCount;i++)
					{
						textTable->SetColumnOption(i, GuiCellOption::AbsoluteOption(contentProvider->columnItemView->GetColumnSize(i)));
					}
					textTable->UpdateCellBounds();
				}

				void DataGridContentProvider::ItemContent::ForceSetEditor(vint column, IDataEditor* editor)
				{
					currentEditor=editor;
					if(currentEditor)
					{
						GuiCellComposition* cell=textTable->GetSitedCell(0, column);
						GuiBoundsComposition* editorBounds=currentEditor->GetBoundsComposition();
						if(editorBounds->GetParent() && editorBounds->GetParent()!=cell)
						{
							editorBounds->GetParent()->RemoveChild(editorBounds);
						}
						currentEditor->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(currentEditor->GetBoundsComposition());
					}
				}

				void DataGridContentProvider::ItemContent::NotifyCloseEditor()
				{
					if(currentEditor)
					{
						GuiGraphicsComposition* composition=currentEditor->GetBoundsComposition();
						if(composition->GetParent())
						{
							composition->GetParent()->RemoveChild(composition);
						}
						currentEditor=0;
					}
				}

				void DataGridContentProvider::ItemContent::NotifySelectCell(vint column)
				{
					for(vint i=0;i<dataVisualizers.Count();i++)
					{
						dataVisualizers[i]->SetSelected(i==column);
					}
				}

				void DataGridContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					bool refresh=false;
					if(dataVisualizers.Count()!=contentProvider->columnItemView->GetColumnCount())
					{
						refresh=true;
					}
					else
					{
						for(vint i=0;i<dataVisualizers.Count();i++)
						{
							IDataVisualizerFactory* factory=dataVisualizers[i]->GetFactory();
							if(factory!=GetDataVisualizerFactory(itemIndex, i))
							{
								refresh=true;
								break;
							}
						}
					}

					if(refresh)
					{
						RemoveCellsAndDataVisualizers();
						vint columnCount=contentProvider->columnItemView->GetColumnCount();

						dataVisualizers.Resize(columnCount);
						for(vint i=0;i<dataVisualizers.Count();i++)
						{
							IDataVisualizerFactory* factory=GetDataVisualizerFactory(itemIndex, i);
							dataVisualizers[i] = factory->CreateVisualizer(font, styleProvider, contentProvider->dataProvider->GetViewModelContext());
						}

						textTable->SetRowsAndColumns(1, columnCount);
						for(vint i=0;i<columnCount;i++)
						{
							GuiCellComposition* cell=new GuiCellComposition;
							textTable->AddChild(cell);
							cell->SetSite(0, i, 1, 1);
							cell->GetEventReceiver()->leftButtonDown.AttachMethod(this, &ItemContent::OnCellButtonDown);
							cell->GetEventReceiver()->rightButtonDown.AttachMethod(this, &ItemContent::OnCellButtonDown);
							cell->GetEventReceiver()->leftButtonUp.AttachMethod(this, &ItemContent::OnCellLeftButtonUp);
							cell->GetEventReceiver()->rightButtonUp.AttachMethod(this, &ItemContent::OnCellRightButtonUp);

							GuiBoundsComposition* composition=dataVisualizers[i]->GetBoundsComposition();
							composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(composition);
						}
					}

					for(vint i=0;i<dataVisualizers.Count();i++)
					{
						IDataVisualizer* dataVisualizer=dataVisualizers[i].Obj();
						dataVisualizer->BeforeVisualizeCell(contentProvider->dataProvider, itemIndex, i);
						contentProvider->dataProvider->VisualizeCell(itemIndex, i, dataVisualizer);
					}

					GridPos selectedCell=contentProvider->GetSelectedCell();
					if(selectedCell.row==itemIndex)
					{
						NotifySelectCell(selectedCell.column);
					}
					else
					{
						NotifySelectCell(-1);
					}
					UpdateSubItemSize();
				}

				void DataGridContentProvider::ItemContent::Uninstall()
				{
					if(currentEditor)
					{
						contentProvider->CloseEditor(false);
					}
					currentEditor=0;
				}
				
/***********************************************************************
DataGridContentProvider
***********************************************************************/

				void DataGridContentProvider::OnColumnChanged()
				{
					vint count=listViewItemStyleProvider->GetCreatedItemStyles().Count();
					for(vint i=0;i<count;i++)
					{
						GuiListControl::IItemStyleController* itemStyleController=listViewItemStyleProvider->GetCreatedItemStyles().Get(i);
						ItemContent* itemContent=listViewItemStyleProvider->GetItemContent<ItemContent>(itemStyleController);
						if(itemContent)
						{
							itemContent->UpdateSubItemSize();
						}
					}
				}

				void DataGridContentProvider::OnAttached(GuiListControl::IItemProvider* provider)
				{
				}

				void DataGridContentProvider::OnItemModified(vint start, vint count, vint newCount)
				{
					if(!currentEditorRequestingSaveData)
					{
						CloseEditor(false);
					}
				}

				void DataGridContentProvider::NotifyCloseEditor()
				{
					if(listViewItemStyleProvider)
					{
						vint count=listViewItemStyleProvider->GetCreatedItemStyles().Count();
						for(vint i=0;i<count;i++)
						{
							GuiListControl::IItemStyleController* itemStyleController=listViewItemStyleProvider->GetCreatedItemStyles().Get(i);
							ItemContent* itemContent=listViewItemStyleProvider->GetItemContent<ItemContent>(itemStyleController);
							if(itemContent)
							{
								itemContent->NotifyCloseEditor();
							}
						}
					}
				}

				void DataGridContentProvider::NotifySelectCell(vint row, vint column)
				{
					currentCell=GridPos(row, column);
					dataGrid->NotifySelectedCellChanged();

					if(listViewItemStyleProvider)
					{
						GuiListControl::IItemStyleController* selectedStyleController=0;
						GuiListControl::IItemArranger* arranger=dataGrid->GetArranger();
						if(arranger)
						{
							selectedStyleController=arranger->GetVisibleStyle(row);
						}
						
						vint count=listViewItemStyleProvider->GetCreatedItemStyles().Count();
						for(vint i=0;i<count;i++)
						{
							GuiListControl::IItemStyleController* itemStyleController=listViewItemStyleProvider->GetCreatedItemStyles().Get(i);
							ItemContent* itemContent=listViewItemStyleProvider->GetItemContent<ItemContent>(itemStyleController);
							if(itemContent)
							{
								if(itemStyleController==selectedStyleController)
								{
									itemContent->NotifySelectCell(column);
								}
								else
								{
									itemContent->NotifySelectCell(-1);
								}
							}
						}
					}
				}

				void DataGridContentProvider::RequestSaveData()
				{
					if(currentEditor && !currentEditorOpening)
					{
						GuiControl* focusedControl = nullptr;
						if (auto controlHost = dataGrid->GetRelatedControlHost())
						{
							if (auto graphicsHost = controlHost->GetGraphicsHost())
							{
								if (auto focusComposition = graphicsHost->GetFocusedComposition())
								{
									focusedControl = focusComposition->GetRelatedControl();
								}
							}
						}

						currentEditorRequestingSaveData=true;
						dataProvider->SaveCellData(currentCell.row, currentCell.column, currentEditor.Obj());
						currentEditorRequestingSaveData=false;
						if(currentEditor)
						{
							GuiListControl::IItemArranger* arranger=dataGrid->GetArranger();
							if(!arranger) return;
							GuiListControl::IItemStyleController* styleController=arranger->GetVisibleStyle(currentCell.row);
							if(!styleController) return;
							ItemContent* itemContent=listViewItemStyleProvider->GetItemContent<ItemContent>(styleController);
							if(!itemContent) return;
							itemContent->ForceSetEditor(currentCell.column, currentEditor.Obj());
							currentEditor->ReinstallEditor();
							if (focusedControl)
							{
								focusedControl->SetFocus();
							}
						}
					}
				}

				IDataEditor* DataGridContentProvider::OpenEditor(vint row, vint column, IDataEditorFactory* editorFactory)
				{
					CloseEditor(true);
					NotifySelectCell(row, column);
					if(editorFactory)
					{
						currentEditorOpening=true;
						currentEditor = editorFactory->CreateEditor(this, dataProvider->GetViewModelContext());
						currentEditor->BeforeEditCell(dataProvider, row, column);
						dataProvider->BeforeEditCell(row, column, currentEditor.Obj());
						currentEditorOpening=false;
					}
					return currentEditor.Obj();
				}

				void DataGridContentProvider::CloseEditor(bool forOpenNewEditor)
				{
					if(currentEditorRequestingSaveData)
					{
						NotifyCloseEditor();
					}
					else
					{
						if(currentCell!=GridPos(-1, -1))
						{
							if(currentEditor)
							{
								NotifyCloseEditor();
								currentEditor=0;
							}
							if(!forOpenNewEditor)
							{
								NotifySelectCell(-1, -1);
							}
						}
					}
				}

				DataGridContentProvider::DataGridContentProvider()
					:dataGrid(0)
					,itemProvider(0)
					,columnItemView(0)
					,dataProvider(0)
					,listViewItemStyleProvider(0)
					,currentCell(-1, -1)
					,currentEditorRequestingSaveData(false)
					,currentEditorOpening(false)
				{
				}

				DataGridContentProvider::~DataGridContentProvider()
				{
					CloseEditor(false);
				}

				compositions::IGuiAxis* DataGridContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* DataGridContentProvider::CreatePreferredArranger()
				{
					return new ListViewColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* DataGridContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(this, font);
				}

				void DataGridContentProvider::AttachListControl(GuiListControl* value)
				{
					dataGrid = dynamic_cast<GuiVirtualDataGrid*>(value);
					listViewItemStyleProvider = dynamic_cast<ListViewItemStyleProvider*>(value->GetStyleProvider());
					itemProvider = value->GetItemProvider();
					itemProvider->AttachCallback(this);
					columnItemView = dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(itemProvider->RequestView(ListViewColumnItemArranger::IColumnItemView::Identifier));
					if (columnItemView)
					{
						columnItemView->AttachCallback(this);
					}
					dataProvider = dynamic_cast<IDataProvider*>(itemProvider->RequestView(IDataProvider::Identifier));
				}

				void DataGridContentProvider::DetachListControl()
				{
					if (dataProvider)
					{
						itemProvider->ReleaseView(dataProvider);
						dataProvider = nullptr;
					}
					if (columnItemView)
					{
						columnItemView->DetachCallback(this);
						itemProvider->ReleaseView(columnItemView);
						columnItemView = nullptr;
					}
					itemProvider->DetachCallback(this);
					itemProvider = nullptr;
					listViewItemStyleProvider = nullptr;
				}

				GridPos DataGridContentProvider::GetSelectedCell()
				{
					return currentCell;
				}

				bool DataGridContentProvider::SetSelectedCell(const GridPos& value, bool openEditor)
				{
					if(currentCell!=value)
					{
						if(value==GridPos(-1, -1))
						{
							CloseEditor(false);
							return true;
						}
						else if(0<=value.row && value.row<dataProvider->GetRowCount() && 0<=value.column && value.column<dataProvider->GetColumnCount())
						{
							IDataEditorFactory* editorFactory=openEditor?dataProvider->GetCellDataEditorFactory(value.row, value.column):0;
							OpenEditor(value.row, value.column, editorFactory);
							return true;
						}
						return false;
					}
					return true;
				}
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			using namespace list;

			void GuiVirtualDataGrid::OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				if(dataProvider->IsColumnSortable(arguments.itemIndex))
				{
					switch(itemProvider->GetSortingState(arguments.itemIndex))
					{
					case GuiListViewColumnHeader::NotSorted:
						itemProvider->SortByColumn(arguments.itemIndex, true);
						break;
					case GuiListViewColumnHeader::Ascending:
						itemProvider->SortByColumn(arguments.itemIndex, false);
						break;
					case GuiListViewColumnHeader::Descending:
						itemProvider->SortByColumn(-1);
						break;
					}
				}
			}

			void GuiVirtualDataGrid::Initialize()
			{
				itemProvider=dynamic_cast<DataGridItemProvider*>(GetItemProvider());
				dataProvider=itemProvider->GetDataProvider();
				structuredDataProvider=dataProvider.Cast<StructuredDataProvider>();

				ChangeItemStyle(new DataGridContentProvider);
				ColumnClicked.AttachMethod(this, &GuiVirtualDataGrid::OnColumnClicked);

				SelectedCellChanged.SetAssociatedComposition(GetBoundsComposition());
			}

			void GuiVirtualDataGrid::NotifySelectedCellChanged()
			{
				SelectedCellChanged.Execute(GetNotifyEventArguments());
			}

			GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider* _styleProvider, list::IDataProvider* _dataProvider)
				:GuiVirtualListView(_styleProvider, new DataGridItemProvider(_dataProvider))
			{
				Initialize();
			}

			GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider* _styleProvider, list::IStructuredDataProvider* _dataProvider)
				:GuiVirtualListView(_styleProvider, new DataGridItemProvider(new StructuredDataProvider(_dataProvider)))
				,itemProvider(0)
				,contentProvider(0)
			{
				Initialize();
			}

			GuiVirtualDataGrid::~GuiVirtualDataGrid()
			{
				SetSelectedCell(GridPos(-1, -1));
			}

			list::IDataProvider* GuiVirtualDataGrid::GetDataProvider()
			{
				return dataProvider.Obj();
			}

			list::StructuredDataProvider* GuiVirtualDataGrid::GetStructuredDataProvider()
			{
				return structuredDataProvider.Obj();
			}

			GridPos GuiVirtualDataGrid::GetSelectedCell()
			{
				return contentProvider->GetSelectedCell();
			}

			void GuiVirtualDataGrid::SetSelectedCell(const GridPos& value)
			{
				if(contentProvider->SetSelectedCell(value, false))
				{
					if(0<=value.row && value.row<dataProvider->GetRowCount())
					{
						EnsureItemVisible(value.row);
						ClearSelection();
						SetSelected(value.row, true);
					}
				}
			}

			Ptr<GuiListControl::IItemStyleProvider> GuiVirtualDataGrid::SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)
			{
				Ptr<list::ListViewItemStyleProvider> styleProvider=value.Cast<list::ListViewItemStyleProvider>();
				if(styleProvider)
				{
					list::DataGridContentProvider* dataGridContentProvider=dynamic_cast<list::DataGridContentProvider*>(styleProvider->GetItemContentProvider());
					if(dataGridContentProvider)
					{
						Ptr<GuiListControl::IItemStyleProvider> result=GuiVirtualListView::SetStyleProvider(value);
						if(result)
						{
							contentProvider=dataGridContentProvider;
							return result;
						}
					}
				}
				return 0;
			}

			bool GuiVirtualDataGrid::ChangeItemStyle(Ptr<list::ListViewItemStyleProvider::IListViewItemContentProvider> contentProvider)
			{
				if(contentProvider.Cast<list::DataGridContentProvider>())
				{
					return GuiVirtualListView::ChangeItemStyle(contentProvider);
				}
				else
				{
					return false;
				}
			}

			namespace list
			{

/***********************************************************************
DataGridItemProvider
***********************************************************************/
				/*
				void DataGridItemProvider::InvokeOnItemModified(vint start, vint count, vint newCount)
				{
					for (vint i = 0; i<itemProviderCallbacks.Count(); i++)
					{
						itemProviderCallbacks[i]->OnItemModified(start, count, newCount);
					}
				}

				void DataGridItemProvider::InvokeOnColumnChanged()
				{
					for (vint i = 0; i<columnItemViewCallbacks.Count(); i++)
					{
						columnItemViewCallbacks[i]->OnColumnChanged();
					}
				}

				void DataGridItemProvider::OnDataProviderColumnChanged()
				{
					InvokeOnColumnChanged();
				}

				void DataGridItemProvider::OnDataProviderItemModified(vint start, vint count, vint newCount)
				{
					InvokeOnItemModified(start, count, newCount);
				}

				DataGridItemProvider::DataGridItemProvider(IDataProvider* _dataProvider)
					:dataProvider(_dataProvider)
				{
					dataProvider->SetCommandExecutor(this);
				}

				DataGridItemProvider::~DataGridItemProvider()
				{
				}

				IDataProvider* DataGridItemProvider::GetDataProvider()
				{
					return dataProvider;
				}

				void DataGridItemProvider::SortByColumn(vint column, bool ascending)
				{
					dataProvider->SortByColumn(column, ascending);
				}

				// ===================== GuiListControl::IItemProvider =====================

				bool DataGridItemProvider::AttachCallback(GuiListControl::IItemProviderCallback* value)
				{
					if (itemProviderCallbacks.Contains(value))
					{
						return false;
					}
					else
					{
						itemProviderCallbacks.Add(value);
						value->OnAttached(this);
						return true;
					}
				}

				bool DataGridItemProvider::DetachCallback(GuiListControl::IItemProviderCallback* value)
				{
					vint index = itemProviderCallbacks.IndexOf(value);
					if (index == -1)
					{
						return false;
					}
					else
					{
						value->OnAttached(0);
						itemProviderCallbacks.Remove(value);
						return true;
					}
				}

				vint DataGridItemProvider::Count()
				{
					return dataProvider->GetRowCount();
				}

				WString DataGridItemProvider::GetTextValue(vint itemIndex)
				{
					return GetText(itemIndex);
				}

				description::Value DataGridItemProvider::GetBindingValue(vint itemIndex)
				{
					return Value();
				}

				IDescriptable* DataGridItemProvider::RequestView(const WString& identifier)
				{
					if (identifier == IDataProvider::Identifier)
					{
						return dataProvider;
					}
					else if (identifier == ListViewItemStyleProvider::IListViewItemView::Identifier)
					{
						return (ListViewItemStyleProvider::IListViewItemView*)this;
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

				void DataGridItemProvider::ReleaseView(IDescriptable* view)
				{
				}

				Ptr<GuiImageData> DataGridItemProvider::GetSmallImage(vint itemIndex)
				{
					return dataProvider->GetRowSmallImage(itemIndex);
				}

				Ptr<GuiImageData> DataGridItemProvider::GetLargeImage(vint itemIndex)
				{
					return dataProvider->GetRowLargeImage(itemIndex);
				}

				WString DataGridItemProvider::GetText(vint itemIndex)
				{
					return dataProvider->GetCellText(itemIndex, 0);
				}

				WString DataGridItemProvider::GetSubItem(vint itemIndex, vint index)
				{
					return dataProvider->GetCellText(itemIndex, index + 1);
				}

				vint DataGridItemProvider::GetDataColumnCount()
				{
					return 0;
				}

				vint DataGridItemProvider::GetDataColumn(vint index)
				{
					return 0;
				}

				// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================

				bool DataGridItemProvider::AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					if (columnItemViewCallbacks.Contains(value))
					{
						return false;
					}
					else
					{
						columnItemViewCallbacks.Add(value);
						return true;
					}
				}

				bool DataGridItemProvider::DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
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

				vint DataGridItemProvider::GetColumnCount()
				{
					return dataProvider->GetColumnCount();
				}

				WString DataGridItemProvider::GetColumnText(vint index)
				{
					return dataProvider->GetColumnText(index);
				}

				vint DataGridItemProvider::GetColumnSize(vint index)
				{
					return dataProvider->GetColumnSize(index);
				}

				void DataGridItemProvider::SetColumnSize(vint index, vint value)
				{
					dataProvider->SetColumnSize(index, value);
					for (vint i = 0; i<columnItemViewCallbacks.Count(); i++)
					{
						columnItemViewCallbacks[i]->OnColumnChanged();
					}
				}

				GuiMenu* DataGridItemProvider::GetDropdownPopup(vint index)
				{
					return dataProvider->GetColumnPopup(index);
				}

				GuiListViewColumnHeader::ColumnSortingState DataGridItemProvider::GetSortingState(vint index)
				{
					if (index == dataProvider->GetSortedColumn())
					{
						return dataProvider->IsSortOrderAscending()
							? GuiListViewColumnHeader::Ascending
							: GuiListViewColumnHeader::Descending;
					}
					else
					{
						return GuiListViewColumnHeader::NotSorted;
					}
				}
				*/
			}
		}
	}
}