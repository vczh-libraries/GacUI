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
DefaultDataGridItemTemplate
***********************************************************************/

				IDataVisualizerFactory* DefaultDataGridItemTemplate::GetDataVisualizerFactory(vint row, vint column)
				{
					if (auto view = dynamic_cast<IDataGridView*>(listControl->GetItemProvider()->RequestView(IDataGridView::Identifier)))
					{
						return view->GetCellDataVisualizerFactory(row, column);
					}
					return nullptr;
				}

				IDataEditorFactory* DefaultDataGridItemTemplate::GetDataEditorFactory(vint row, vint column)
				{
					if (auto view = dynamic_cast<IDataGridView*>(listControl->GetItemProvider()->RequestView(IDataGridView::Identifier)))
					{
						return view->GetCellDataEditorFactory(row, column);
					}
					return nullptr;
				}

				vint DefaultDataGridItemTemplate::GetCellColumnIndex(compositions::GuiGraphicsComposition* composition)
				{
					for (vint i = 0; i < textTable->GetColumns(); i++)
					{
						auto cell = textTable->GetSitedCell(0, i);
						if (composition == cell)
						{
							return i;
						}
					}
					return -1;
				}

				void DefaultDataGridItemTemplate::OnCellButtonUp(compositions::GuiGraphicsComposition* sender, bool openEditor)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						vint index = GetCellColumnIndex(sender);
						if (index != -1)
						{
							if (currentEditor && dataGrid->GetSelectedCell().column == index)
							{
								return;
							}

							vint currentRow = GetIndex();
							auto factory = openEditor ? GetDataEditorFactory(currentRow, index) : nullptr;
							currentEditor = dataGrid->OpenEditor(currentRow, index, factory);
							if (currentEditor)
							{
								currentEditor->GetTemplate()->SetAlignmentToParent(Margin(0, 0, 0, 0));
								sender->AddChild(currentEditor->GetTemplate());
							}
						}
					}
				}

				bool DefaultDataGridItemTemplate::IsInEditor(compositions::GuiMouseEventArgs& arguments)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						if (!dataGrid->currentEditor) return false;
						auto editorComposition = dataGrid->currentEditor->GetTemplate();
						auto stopComposition = backgroundButton->GetContainerComposition();
						auto currentComposition = arguments.eventSource;

						while (currentComposition)
						{
							if (currentComposition == editorComposition)
							{
								arguments.handled = true;
								return true;
							}
							else if (currentComposition == stopComposition)
							{
								break;
							}
							else
							{
								currentComposition = currentComposition->GetParent();
							}
						}

					}
					return false;
				}

				void DefaultDataGridItemTemplate::OnCellButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					IsInEditor(arguments);
				}

				void DefaultDataGridItemTemplate::OnCellLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						if (!IsInEditor(arguments))
						{
							if (dataGrid->GetVisuallyEnabled())
							{
								OnCellButtonUp(sender, true);
							}
						}
					}
				}

				void DefaultDataGridItemTemplate::OnCellRightButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						if (!IsInEditor(arguments))
						{
							if (dataGrid->GetVisuallyEnabled())
							{
								OnCellButtonUp(sender, false);
							}
						}
					}
				}

				void DefaultDataGridItemTemplate::OnColumnChanged()
				{
					UpdateSubItemSize();
				}

				void DefaultDataGridItemTemplate::OnInitialize()
				{
					DefaultListViewItemTemplate::OnInitialize();
					{
						textTable = new GuiTableComposition;
						textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						textTable->SetRowsAndColumns(1, 1);
						textTable->SetRowOption(0, GuiCellOption::MinSizeOption());
						textTable->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
						backgroundButton->GetContainerComposition()->AddChild(textTable);
					}

					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						vint columnCount = dataGrid->columnItemView->GetColumnCount();
						vint itemIndex = GetIndex();

						dataVisualizers.Resize(columnCount);
						for (vint i = 0; i < dataVisualizers.Count(); i++)
						{
							auto factory = GetDataVisualizerFactory(itemIndex, i);
							dataVisualizers[i] = factory->CreateVisualizer(dataGrid->GetListViewStyleProvider(), dataGrid->dataGridView->GetViewModelContext());
						}

						textTable->SetRowsAndColumns(1, columnCount);
						for (vint i = 0; i < columnCount; i++)
						{
							auto cell = new GuiCellComposition;
							textTable->AddChild(cell);
							cell->SetSite(0, i, 1, 1);
							cell->GetEventReceiver()->leftButtonDown.AttachMethod(this, &DefaultDataGridItemTemplate::OnCellButtonDown);
							cell->GetEventReceiver()->rightButtonDown.AttachMethod(this, &DefaultDataGridItemTemplate::OnCellButtonDown);
							cell->GetEventReceiver()->leftButtonUp.AttachMethod(this, &DefaultDataGridItemTemplate::OnCellLeftButtonUp);
							cell->GetEventReceiver()->rightButtonUp.AttachMethod(this, &DefaultDataGridItemTemplate::OnCellRightButtonUp);

							auto composition = dataVisualizers[i]->GetTemplate();
							composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(composition);
						}

						for (vint i = 0; i < dataVisualizers.Count(); i++)
						{
							auto dataVisualizer = dataVisualizers[i].Obj();
							dataVisualizer->BeforeVisualizeCell(dataGrid->GetItemProvider(), itemIndex, i);
							dataGrid->dataGridView->VisualizeCell(itemIndex, i, dataVisualizer);
						}

						GridPos selectedCell = dataGrid->GetSelectedCell();
						if (selectedCell.row == itemIndex)
						{
							NotifySelectCell(selectedCell.column);
						}
						else
						{
							NotifySelectCell(-1);
						}
						UpdateSubItemSize();
					}

					SelectedChanged.AttachMethod(this, &DefaultDataGridItemTemplate::OnSelectedChanged);
					FontChanged.AttachMethod(this, &DefaultDataGridItemTemplate::OnFontChanged);

					SelectedChanged.Execute(compositions::GuiEventArgs(this));
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DefaultDataGridItemTemplate::OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (!GetSelected())
					{
						NotifySelectCell(-1);
					}
				}

				void DefaultDataGridItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FOREACH(Ptr<IDataVisualizer>, visualizer, dataVisualizers)
					{
						visualizer->GetTemplate()->SetFont(GetFont());
					}
					if (currentEditor)
					{
						currentEditor->GetTemplate()->SetFont(GetFont());
					}
				}

				DefaultDataGridItemTemplate::DefaultDataGridItemTemplate()
				{
				}

				DefaultDataGridItemTemplate::~DefaultDataGridItemTemplate()
				{
					FOREACH(Ptr<IDataVisualizer>, visualizer, dataVisualizers)
					{
						auto composition = visualizer->GetTemplate();
						if (composition->GetParent())
						{
							composition->GetParent()->RemoveChild(composition);
						}
					}
				}

				void DefaultDataGridItemTemplate::UpdateSubItemSize()
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						vint columnCount = dataGrid->columnItemView->GetColumnCount();
						if (columnCount > textTable->GetColumns())
						{
							columnCount = textTable->GetColumns();
						}
						for (vint i = 0; i < columnCount; i++)
						{
							textTable->SetColumnOption(i, GuiCellOption::AbsoluteOption(dataGrid->columnItemView->GetColumnSize(i)));
						}
						textTable->UpdateCellBounds();
					}
				}

				void DefaultDataGridItemTemplate::ForceSetEditor(vint column, IDataEditor* editor)
				{
					currentEditor = editor;
					if (currentEditor)
					{
						auto cell = textTable->GetSitedCell(0, column);
						auto* editorBounds = currentEditor->GetTemplate();
						if (editorBounds->GetParent() && editorBounds->GetParent() != cell)
						{
							editorBounds->GetParent()->RemoveChild(editorBounds);
						}
						editorBounds->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(editorBounds);
					}
				}

				void DefaultDataGridItemTemplate::NotifyCloseEditor()
				{
					if (currentEditor)
					{
						auto composition = currentEditor->GetTemplate();
						if (composition->GetParent())
						{
							composition->GetParent()->RemoveChild(composition);
						}
						currentEditor = nullptr;
					}
				}

				void DefaultDataGridItemTemplate::NotifySelectCell(vint column)
				{
					for (vint i = 0; i < dataVisualizers.Count(); i++)
					{
						dataVisualizers[i]->SetSelected(i == column);
					}
				}
			}
				
/***********************************************************************
GuiVirtualDataGrid (Editor)
***********************************************************************/

			using namespace list;

			void GuiVirtualDataGrid::OnItemModified(vint start, vint count, vint newCount)
			{
				GuiVirtualListView::OnItemModified(start, count, newCount);
				if(!GetItemProvider()->IsEditing())
				{
					CloseEditor(false);
				}
			}

			void GuiVirtualDataGrid::NotifyCloseEditor()
			{
				if (currentEditorPos.row != -1 && GetArranger())
				{
					auto style = GetArranger()->GetVisibleStyle(currentEditorPos.row);
					if (auto itemStyle = dynamic_cast<DefaultDataGridItemTemplate*>(style))
					{
						itemStyle->NotifyCloseEditor();
					}
				}
			}

			void GuiVirtualDataGrid::NotifySelectCell(vint row, vint column)
			{
				selectedCell = { row, column };
				SelectedCellChanged.Execute(GetNotifyEventArguments());

				auto style = GetArranger()->GetVisibleStyle(currentEditorPos.row);
				if (auto itemStyle = dynamic_cast<DefaultDataGridItemTemplate*>(style))
				{
					itemStyle->NotifySelectCell(column);
				}
			}

			void GuiVirtualDataGrid::RequestSaveData()
			{
				if (currentEditor && !currentEditorOpeningEditor)
				{
					GuiControl* focusedControl = nullptr;
					if (auto controlHost = GetRelatedControlHost())
					{
						if (auto graphicsHost = controlHost->GetGraphicsHost())
						{
							if (auto focusComposition = graphicsHost->GetFocusedComposition())
							{
								focusedControl = focusComposition->GetRelatedControl();
							}
						}
					}

					GetItemProvider()->PushEditing();
					dataGridView->SaveCellData(currentEditorPos.row, currentEditorPos.column, currentEditor.Obj());
					GetItemProvider()->PopEditing();

					if (currentEditor)
					{
						if (auto arranger = GetArranger())
						{
							auto style = arranger->GetVisibleStyle(currentEditorPos.column);
							if (auto itemStyle = dynamic_cast<DefaultDataGridItemTemplate*>(style))
							{
								itemStyle->ForceSetEditor(currentEditorPos.column, currentEditor.Obj());
								currentEditor->ReinstallEditor();
								if (focusedControl)
								{
									focusedControl->SetFocus();
								}
							}
						}
					}
				}
			}

			list::IDataEditor* GuiVirtualDataGrid::OpenEditor(vint row, vint column, list::IDataEditorFactory* editorFactory)
			{
				CloseEditor(true);
				NotifySelectCell(row, column);
				if (editorFactory)
				{
					currentEditorOpeningEditor = true;
					currentEditorPos = { row,column };
					currentEditor = editorFactory->CreateEditor(dataGridView->GetViewModelContext());
					currentEditor->BeforeEditCell(GetItemProvider(), row, column);
					dataGridView->EditCell(row, column, currentEditor.Obj());
					currentEditorOpeningEditor = false;
				}
				return currentEditor.Obj();
			}

			void GuiVirtualDataGrid::CloseEditor(bool forOpenNewEditor)
			{
				if (GetItemProvider()->IsEditing())
				{
					NotifyCloseEditor();
				}
				else
				{
					if (currentEditorPos != GridPos{-1, -1})
					{
						if (currentEditor)
						{
							NotifyCloseEditor();
							currentEditor = nullptr;
						}
						if (!forOpenNewEditor)
						{
							NotifySelectCell(-1, -1);
						}
					}
				}
				currentEditorPos = { -1,-1 };
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			void GuiVirtualDataGrid::OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				if(dataGridView->IsColumnSortable(arguments.itemIndex))
				{
					switch(columnItemView->GetSortingState(arguments.itemIndex))
					{
					case ColumnSortingState::NotSorted:
						dataGridView->SortByColumn(arguments.itemIndex, true);
						break;
					case ColumnSortingState::Ascending:
						dataGridView->SortByColumn(arguments.itemIndex, false);
						break;
					case ColumnSortingState::Descending:
						dataGridView->SortByColumn(-1, false);
						break;
					}
				}
			}

			GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider)
				:GuiVirtualListView(_styleProvider, _itemProvider)
			{
				listViewItemView = dynamic_cast<IListViewItemView*>(_itemProvider->RequestView(IListViewItemView::Identifier));
				columnItemView = dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(_itemProvider->RequestView(ListViewColumnItemArranger::IColumnItemView::Identifier));
				dataGridView = dynamic_cast<IDataGridView*>(_itemProvider->RequestView(IDataGridView::Identifier));

				CHECK_ERROR(listViewItemView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing IListViewItemView from item provider.");
				CHECK_ERROR(columnItemView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing ListViewColumnItemArranger::IColumnItemView from item provider.");
				CHECK_ERROR(dataGridView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing IDataGridView from item provider.");

				SetStyleAndArranger(
					[](const Value&) { return new list::DefaultDataGridItemTemplate; },
					new list::ListViewColumnItemArranger
				);

				ColumnClicked.AttachMethod(this, &GuiVirtualDataGrid::OnColumnClicked);

				SelectedCellChanged.SetAssociatedComposition(GetBoundsComposition());
			}

			GuiVirtualDataGrid::~GuiVirtualDataGrid()
			{
				SetSelectedCell({ -1, -1 });
			}

			GridPos GuiVirtualDataGrid::GetSelectedCell()
			{
				return selectedCell;
			}

			void GuiVirtualDataGrid::SetSelectedCell(const GridPos& value)
			{
				if (selectedCell != value)
				{
					if (value.row == -1 || value.column == -1)
					{
						CloseEditor(false);
					}
					else if (0 <= value.row && value.row < GetItemProvider()->Count() && 0 <= value.column && value.column < columnItemView->GetColumnCount())
					{
						OpenEditor(value.row, value.column, nullptr);
					}
					else
					{
						return;
					}
				}

				if (0 <= value.row && value.row < GetItemProvider()->Count())
				{
					EnsureItemVisible(value.row);
					ClearSelection();
					SetSelected(value.row, true);
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