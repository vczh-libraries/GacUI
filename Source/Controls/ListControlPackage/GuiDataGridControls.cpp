#include "GuiDataGridControls.h"
#include "GuiDataGridExtensions.h"

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
				using namespace templates;

				const wchar_t* const IDataGridView::Identifier = L"vl::presentation::controls::list::IDataGridView";

/***********************************************************************
DefaultDataGridItemTemplate
***********************************************************************/

				IDataVisualizerFactory* DefaultDataGridItemTemplate::GetDataVisualizerFactory(vint row, vint column)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						if (auto factory = dataGrid->dataGridView->GetCellDataVisualizerFactory(row, column))
						{
							return factory;
						}

						if (column == 0)
						{
							return dataGrid->defaultMainColumnVisualizerFactory.Obj();
						}
						else
						{
							return dataGrid->defaultSubColumnVisualizerFactory.Obj();
						}

					}

					return nullptr;
				}

				IDataEditorFactory* DefaultDataGridItemTemplate::GetDataEditorFactory(vint row, vint column)
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						return dataGrid->dataGridView->GetCellDataEditorFactory(row, column);
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
						vint columnCount = dataGrid->listViewItemView->GetColumnCount();
						vint itemIndex = GetIndex();

						dataVisualizers.Resize(columnCount);
						for (vint i = 0; i < dataVisualizers.Count(); i++)
						{
							auto factory = GetDataVisualizerFactory(itemIndex, i);
							dataVisualizers[i] = factory->CreateVisualizer(dataGrid);
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
						visualizer->NotifyDeletedTemplate();
					}
					if (currentEditor)
					{
						currentEditor->NotifyDeletedTemplate();
					}
				}

				void DefaultDataGridItemTemplate::UpdateSubItemSize()
				{
					if (auto dataGrid = dynamic_cast<GuiVirtualDataGrid*>(listControl))
					{
						vint columnCount = dataGrid->listViewItemView->GetColumnCount();
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

			list::IDataEditor* GuiVirtualDataGrid::OpenEditor(vint row, vint column, list::IDataEditorFactory* editorFactory)
			{
				CloseEditor(true);
				NotifySelectCell(row, column);
				if (editorFactory)
				{
					currentEditorOpeningEditor = true;
					currentEditorPos = { row,column };
					currentEditor = editorFactory->CreateEditor(this);
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
GuiVirtualDataGrid (IDataGridContext)
***********************************************************************/

			GuiListViewBase::IStyleProvider* GuiVirtualDataGrid::GetListViewStyleProvider()
			{
				return GuiVirtualListView::GetListViewStyleProvider();
			}

			description::Value GuiVirtualDataGrid::GetViewModelContext()
			{
				return dataGridView->GetViewModelContext();
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

				{
					auto mainProperty = [](const Value&) { return new MainColumnVisualizerTemplate; };
					auto subProperty = [](const Value&) { return new SubColumnVisualizerTemplate; };
					auto cellBorderProperty = [](const Value&) { return new CellBorderVisualizerTemplate; };

					auto mainFactory = MakePtr<GuiBindableDataVisualizer::Factory>(mainProperty);
					auto subFactory = MakePtr<GuiBindableDataVisualizer::Factory>(subProperty);
					defaultMainColumnVisualizerFactory = MakePtr<GuiBindableDataVisualizer::DecoratedFactory>(cellBorderProperty, mainFactory);
					defaultSubColumnVisualizerFactory = MakePtr<GuiBindableDataVisualizer::DecoratedFactory>(cellBorderProperty, subFactory);
				}

				CHECK_ERROR(listViewItemView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing IListViewItemView from item provider.");
				CHECK_ERROR(columnItemView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing ListViewColumnItemArranger::IColumnItemView from item provider.");
				CHECK_ERROR(dataGridView != nullptr, L"GuiVirtualDataGrid::GuiVirtualDataGrid(IStyleProvider*, GuiListControl::IItemProvider*)#Missing IDataGridView from item provider.");

				SetViewToDefault();
				ColumnClicked.AttachMethod(this, &GuiVirtualDataGrid::OnColumnClicked);
				SelectedCellChanged.SetAssociatedComposition(GetBoundsComposition());
			}

			GuiVirtualDataGrid::~GuiVirtualDataGrid()
			{
				SetSelectedCell({ -1, -1 });
			}

			GuiListControl::IItemProvider* GuiVirtualDataGrid::GetItemProvider()
			{
				return GuiVirtualListView::GetItemProvider();
			}

			void GuiVirtualDataGrid::SetViewToDefault()
			{
				SetStyleAndArranger(
					[](const Value&) { return new list::DefaultDataGridItemTemplate; },
					new list::ListViewColumnItemArranger
				);
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
					else if (0 <= value.row && value.row < GetItemProvider()->Count() && 0 <= value.column && value.column < listViewItemView->GetColumnCount())
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
		}
	}
}