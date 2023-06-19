#include "GuiGraphicsTableComposition.h"
#include <math.h>

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;
			using namespace elements;

/***********************************************************************
GuiTableComposition
***********************************************************************/

			namespace update_cell_bounds_helpers
			{
				vint First(vint a, vint b)
				{
					return a;
				}

				vint Second(vint a, vint b)
				{
					return b;
				}

				vint X(Size s)
				{
					return s.x;
				}

				vint Y(Size s)
				{
					return s.y;
				}

				vint RL(GuiCellComposition* cell)
				{
					return cell->GetRow();
				}

				vint CL(GuiCellComposition* cell)
				{
					return cell->GetColumn();
				}

				vint RS(GuiCellComposition* cell)
				{
					return cell->GetRowSpan();
				}

				vint CS(GuiCellComposition* cell)
				{
					return cell->GetColumnSpan();
				}
			}
			using namespace update_cell_bounds_helpers;

			Rect GuiTableComposition::Layout_CalculateCellArea(Rect tableBounds)
			{
				Rect bounds(Point(0, 0), tableBounds.GetSize());
				vint borderThickness = borderVisible ? cellPadding : 0;
				bounds.x1 += borderThickness;
				bounds.y1 += borderThickness;
				bounds.x2 -= borderThickness;
				bounds.y2 -= borderThickness;
				if (bounds.x2 < bounds.x1) bounds.x2 = bounds.x1;
				if (bounds.y2 < bounds.y1) bounds.y2 = bounds.y1;
				return bounds;
			}

			void GuiTableComposition::Layout_UpdateCellBoundsInternal(
				collections::Array<vint>& dimSizes,
				vint& dimSize,
				vint& dimSizeWithPercentage,
				collections::Array<GuiCellOption>& dimOptions,
				vint GuiTableComposition::* dim1,
				vint GuiTableComposition::* dim2,
				vint(*getSize)(Size),
				vint(*getLocation)(GuiCellComposition*),
				vint(*getSpan)(GuiCellComposition*),
				vint(*getRow)(vint, vint),
				vint(*getCol)(vint, vint)
			)
			{
				for (vint i = 0; i < this->*dim1; i++)
				{
					dimSizes[i] = 0;
				}

				for (vint i = 0; i < this->*dim1; i++)
				{
					GuiCellOption option = dimOptions[i];
					switch (option.composeType)
					{
					case GuiCellOption::Absolute:
						{
							dimSizes[i] = option.absolute;
						}
						break;
					case GuiCellOption::MinSize:
						{
							for (vint j = 0; j < this->*dim2; j++)
							{
								if (auto cell = GetSitedCell(getRow(i, j), getCol(i, j)))
								{
									if (getSpan(cell) == 1)
									{
										vint size = getSize(cell->GetCachedMinSize());
										vint span = getSpan(cell);
										for (vint k = 1; k < span; k++)
										{
											size -= dimSizes[i - k] + cellPadding;
										}
										if (dimSizes[i] < size)
										{
											dimSizes[i] = size;
										}
									}
								}
							}
						}
						break;
					default:;
					}
				}

				{
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							for (vint j = 0; j < this->*dim2; j++)
							{
								GuiCellComposition* cell = GetSitedCell(getRow(i, j), getCol(i, j));
								if (cell)
								{
									vint size = getSize(cell->GetCachedMinSize());
									vint start = getLocation(cell);
									vint span = getSpan(cell);
									size -= (span - 1)*cellPadding;
									double totalPercentage = 0;

									for (vint k = start; k < start + span; k++)
									{
										if (dimOptions[k].composeType == GuiCellOption::Percentage)
										{
											totalPercentage += dimOptions[k].percentage;
										}
										else
										{
											size -= dimSizes[k];
										}
									}

									size = (vint)ceil(size*option.percentage / totalPercentage);
									if (dimSizes[i] < size)
									{
										dimSizes[i] = size;
									}
								}
							}
						}
					}

					double totalPercentage = 0;
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							totalPercentage += option.percentage;
						}
					}

					vint percentageTotalSize = 0;
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							vint size = (vint)ceil(dimSizes[i] * totalPercentage / option.percentage);
							if (percentageTotalSize < size)
							{
								percentageTotalSize = size;
							}
						}
					}

					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							vint size = (vint)ceil(percentageTotalSize * option.percentage / totalPercentage);
							if (dimSizes[i] < size)
							{
								dimSizes[i] = size;
							}
						}
					}
				}

				for (vint i = 0; i < this->*dim1; i++)
				{
					if (dimOptions[i].composeType != GuiCellOption::Percentage)
					{
						dimSize += dimSizes[i];
					}
					dimSizeWithPercentage += dimSizes[i];
				}
			}

			void GuiTableComposition::Layout_UpdateCellBoundsPercentages(
				collections::Array<vint>& dimSizes,
				vint dimSize,
				vint maxDimSize,
				collections::Array<GuiCellOption>& dimOptions
			)
			{
				if (maxDimSize > dimSize)
				{
					double totalPercentage = 0;
					vint percentageCount = 0;
					// TODO: (enumerable) foreach
					for (vint i = 0; i < dimOptions.Count(); i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							totalPercentage += option.percentage;
							percentageCount++;
						}
					}
					if (percentageCount > 0 && totalPercentage > 0)
					{
						// TODO: (enumerable) foreach
						for (vint i = 0; i < dimOptions.Count(); i++)
						{
							GuiCellOption option = dimOptions[i];
							if (option.composeType == GuiCellOption::Percentage)
							{
								dimSizes[i] = (vint)((maxDimSize - dimSize)*option.percentage / totalPercentage);
							}
						}
					}
				}
			}

			vint GuiTableComposition::GetSiteIndex(vint _rows, vint _columns, vint _row, vint _column)
			{
				return _row * _columns + _column;
			}

			void GuiTableComposition::SetSitedCell(vint _row, vint _column, GuiCellComposition* cell)
			{
				layout_cellCompositions[GetSiteIndex(rows, columns, _row, _column)] = cell;
				layout_invalid = true;
			}

			void GuiTableComposition::OnCompositionStateChanged()
			{
				GuiBoundsComposition::OnCompositionStateChanged();
				ConfigChanged.Execute(GuiEventArgs(this));
				layout_invalid = true;
			}

			Size GuiTableComposition::Layout_CalculateMinSize()
			{
				for (auto child : Children())
				{
					if (auto cell = dynamic_cast<GuiCellComposition*>(child))
					{
						cell->Layout_SetCachedMinSize(cell->Layout_CalculateMinSizeHelper());
					}
				}

				if (layout_invalid)
				{
					layout_invalid = false;
					layout_invalidCellBounds = true;

					layout_rowOffsets.Resize(rows);
					layout_rowSizes.Resize(rows);
					layout_columnOffsets.Resize(columns);
					layout_columnSizes.Resize(columns);
					layout_rowTotal = (rows - 1) * cellPadding;
					layout_columnTotal = (columns - 1) * cellPadding;
					layout_rowTotalWithPercentage = layout_rowTotal;
					layout_columnTotalWithPercentage = layout_columnTotal;

					Layout_UpdateCellBoundsInternal(
						layout_rowSizes,
						layout_rowTotal,
						layout_rowTotalWithPercentage,
						rowOptions,
						&GuiTableComposition::rows,
						&GuiTableComposition::columns,
						&Y,
						&RL,
						&RS,
						&First,
						&Second
					);
					Layout_UpdateCellBoundsInternal(
						layout_columnSizes,
						layout_columnTotal,
						layout_columnTotalWithPercentage,
						columnOptions,
						&GuiTableComposition::columns,
						&GuiTableComposition::rows,
						&X,
						&CL,
						&CS,
						&Second,
						&First
					);
				}

				Size minTableSize;
				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					vint offset = (borderVisible ? 2 * cellPadding : 0);
					minTableSize.x = layout_columnTotalWithPercentage + offset;
					minTableSize.y = layout_rowTotalWithPercentage + offset;
				}

				Size minClientSize = GuiBoundsComposition::Layout_CalculateMinSize();
				return Size(
					minTableSize.x > minClientSize.x ? minTableSize.x : minClientSize.x,
					minTableSize.y > minClientSize.y ? minTableSize.y : minClientSize.y
					);
			}

			Rect GuiTableComposition::Layout_CalculateBounds(Size parentSize)
			{
				Rect bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);

				if (layout_lastTableSize != bounds.GetSize())
				{
					layout_invalidCellBounds = true;
					layout_lastTableSize = bounds.GetSize();
				}

				if (layout_invalidCellBounds)
				{
					layout_invalidCellBounds = false;

					Size area = Layout_CalculateCellArea(bounds).GetSize();
					Layout_UpdateCellBoundsPercentages(
						layout_rowSizes,
						layout_rowTotal,
						area.y,
						rowOptions
						);
					Layout_UpdateCellBoundsPercentages(
						layout_columnSizes,
						layout_columnTotal,
						area.x,
						columnOptions
						);

					layout_rowExtending = Layout_UpdateCellBoundsOffsets(layout_rowOffsets, layout_rowSizes, area.y);
					layout_columnExtending = Layout_UpdateCellBoundsOffsets(layout_columnOffsets, layout_columnSizes, area.x);

					for (vint i = 0; i < rows; i++)
					{
						for (vint j = 0; j < columns; j++)
						{
							vint index = GetSiteIndex(rows, columns, i, j);
							layout_cellBounds[index] = Rect(Point(layout_columnOffsets[j], layout_rowOffsets[i]), Size(layout_columnSizes[j], layout_rowSizes[i]));
						}
					}

					for (auto child : Children())
					{
						if (auto cell = dynamic_cast<GuiCellComposition*>(child))
						{
							cell->Layout_SetCellBounds();
						}
					}
				}

				return bounds;
			}

			vint GuiTableComposition::Layout_UpdateCellBoundsOffsets(
				collections::Array<vint>& offsets,
				collections::Array<vint>& sizes,
				vint max
				)
			{
				offsets[0] = 0;
				for (vint i = 1; i < offsets.Count(); i++)
				{
					offsets[i] = offsets[i - 1] + cellPadding + sizes[i - 1];
				}

				vint last = offsets.Count() - 1;
				vint right = offsets[last] + sizes[last];
				return max > right ? max - right : 0;
			}

			GuiTableComposition::GuiTableComposition()
			{
				ConfigChanged.SetAssociatedComposition(this);
				SetRowsAndColumns(1, 1);
			}

			vint GuiTableComposition::GetRows()
			{
				return rows;
			}

			vint GuiTableComposition::GetColumns()
			{
				return columns;
			}

			bool GuiTableComposition::SetRowsAndColumns(vint _rows, vint _columns)
			{
				if (_rows <= 0 || _columns <= 0) return false;
				rowOptions.Resize(_rows);
				columnOptions.Resize(_columns);
				layout_cellCompositions.Resize(_rows*_columns);
				layout_cellBounds.Resize(_rows*_columns);
				for (vint i = 0; i < _rows*_columns; i++)
				{
					layout_cellCompositions[i] = nullptr;
					layout_cellBounds[i] = Rect();
				}
				rows = _rows;
				columns = _columns;
				// TODO: (enumerable) foreach
				vint childCount = Children().Count();
				for (vint i = 0; i < childCount; i++)
				{
					GuiCellComposition* cell = dynamic_cast<GuiCellComposition*>(Children().Get(i));
					if (cell)
					{
						cell->OnTableRowsAndColumnsChanged();
					}
				}
				InvokeOnCompositionStateChanged();
				return true;
			}

			GuiCellComposition* GuiTableComposition::GetSitedCell(vint _row, vint _column)
			{
				return layout_cellCompositions[GetSiteIndex(rows, columns, _row, _column)];
			}

			GuiCellOption GuiTableComposition::GetRowOption(vint _row)
			{
				return rowOptions[_row];
			}

			void GuiTableComposition::SetRowOption(vint _row, GuiCellOption option)
			{
				if (option.composeType == GuiCellOption::Percentage && option.percentage < 0.001)
				{
					option.percentage = 0;
				}

				if (rowOptions[_row] != option)
				{
					rowOptions[_row] = option;
					InvokeOnCompositionStateChanged();
				}
			}

			GuiCellOption GuiTableComposition::GetColumnOption(vint _column)
			{
				return columnOptions[_column];
			}

			void GuiTableComposition::SetColumnOption(vint _column, GuiCellOption option)
			{
				if (option.composeType == GuiCellOption::Percentage && option.percentage < 0.001)
				{
					option.percentage = 0;
				}

				if (columnOptions[_column] != option)
				{
					columnOptions[_column] = option;
					InvokeOnCompositionStateChanged();
				}
			}

			vint GuiTableComposition::GetCellPadding()
			{
				return cellPadding;
			}

			void GuiTableComposition::SetCellPadding(vint value)
			{
				if (value < 0) value = 0;
				if (cellPadding != value)
				{
					cellPadding = value;
					InvokeOnCompositionStateChanged();
				}
			}

			bool GuiTableComposition::GetBorderVisible()
			{
				return borderVisible;
			}

			void GuiTableComposition::SetBorderVisible(bool value)
			{
				if (borderVisible != value)
				{
					borderVisible = value;
					InvokeOnCompositionStateChanged();
				}
			}

/***********************************************************************
GuiCellComposition
***********************************************************************/

			void GuiCellComposition::ClearSitedCells(GuiTableComposition* table)
			{
				if (row != -1 && column != -1)
				{
					for (vint r = 0; r < rowSpan; r++)
					{
						for (vint c = 0; c < columnSpan; c++)
						{
							table->SetSitedCell(row + r, column + c, nullptr);
						}
					}
				}
			}

			void GuiCellComposition::SetSitedCells(GuiTableComposition* table)
			{
				for (vint r = 0; r < rowSpan; r++)
				{
					for (vint c = 0; c < columnSpan; c++)
					{
						table->SetSitedCell(row + r, column + c, this);
					}
				}
			}

			void GuiCellComposition::ResetSiteInternal()
			{
				row = -1;
				column = -1;
				rowSpan = 1;
				columnSpan = 1;
			}

			bool GuiCellComposition::SetSiteInternal(vint _row, vint _column, vint _rowSpan, vint _columnSpan)
			{
				if (layout_tableParent)
				{
					if (_row < 0 || _row >= layout_tableParent->rows || _column < 0 || _column >= layout_tableParent->columns) return false;
					if (_rowSpan<1 || _row + _rowSpan>layout_tableParent->rows || _columnSpan<1 || _column + _columnSpan>layout_tableParent->columns) return false;

					for (vint r = 0; r < _rowSpan; r++)
					{
						for (vint c = 0; c < _columnSpan; c++)
						{
							GuiCellComposition* cell = layout_tableParent->GetSitedCell(_row + r, _column + c);
							if (cell && cell != this)
							{
								return false;
							}
						}
					}
					ClearSitedCells(layout_tableParent);
				}

				row = _row;
				column = _column;
				rowSpan = _rowSpan;
				columnSpan = _columnSpan;

				if (layout_tableParent)
				{
					SetSitedCells(layout_tableParent);
				}
				return true;
			}

			void GuiCellComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsComposition::OnParentChanged(oldParent, newParent);
				if (layout_tableParent)
				{
					ClearSitedCells(layout_tableParent);
				}
				layout_tableParent = dynamic_cast<GuiTableComposition*>(newParent);
				if (!layout_tableParent || !SetSiteInternal(row, column, rowSpan, columnSpan))
				{
					ResetSiteInternal();
				}
				if (layout_tableParent)
				{
					if (row != -1 && column != -1)
					{
						SetSiteInternal(row, column, rowSpan, columnSpan);
					}
				}
			}

			void GuiCellComposition::OnTableRowsAndColumnsChanged()
			{
				if(!SetSiteInternal(row, column, rowSpan, columnSpan))
				{
					ResetSiteInternal();
				}
			}

			void GuiCellComposition::Layout_SetCellBounds()
			{
				Rect result;
				if (layout_tableParent && row != -1 && column != -1)
				{
					Rect bounds1, bounds2;
					{
						vint index = layout_tableParent->GetSiteIndex(layout_tableParent->rows, layout_tableParent->columns, row, column);
						bounds1 = layout_tableParent->layout_cellBounds[index];
					}
					{
						vint index = layout_tableParent->GetSiteIndex(layout_tableParent->rows, layout_tableParent->columns, row + rowSpan - 1, column + columnSpan - 1);
						bounds2 = layout_tableParent->layout_cellBounds[index];

						if (row + rowSpan == layout_tableParent->rows)
						{
							bounds2.y2 += layout_tableParent->layout_rowExtending;
						}
						if (column + columnSpan == layout_tableParent->columns)
						{
							bounds2.x2 += layout_tableParent->layout_columnExtending;
						}
					}
					vint offset = layout_tableParent->borderVisible ? layout_tableParent->cellPadding : 0;
					result = Rect(bounds1.x1 + offset, bounds1.y1 + offset, bounds2.x2 + offset, bounds2.y2 + offset);
				}
				else
				{
					result = Rect();
				}
				Layout_SetCachedBounds(result);
			}

			GuiCellComposition::GuiCellComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				CachedMinSizeChanged.AttachLambda([this](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					if (layout_tableParent) layout_tableParent->layout_invalid = true;
				});
			}

			GuiTableComposition* GuiCellComposition::GetTableParent()
			{
				return layout_tableParent;
			}

			vint GuiCellComposition::GetRow()
			{
				return row;
			}

			vint GuiCellComposition::GetRowSpan()
			{
				return rowSpan;
			}

			vint GuiCellComposition::GetColumn()
			{
				return column;
			}

			vint GuiCellComposition::GetColumnSpan()
			{
				return columnSpan;
			}

			bool GuiCellComposition::SetSite(vint _row, vint _column, vint _rowSpan, vint _columnSpan)
			{
				if (!SetSiteInternal(_row, _column, _rowSpan, _columnSpan))
				{
					return false;
				}

				InvokeOnCompositionStateChanged();
				return true;
			}

/***********************************************************************
GuiTableSplitterCompositionBase
***********************************************************************/

			void GuiTableSplitterCompositionBase::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsComposition::OnParentChanged(oldParent, newParent);
				tableParent = dynamic_cast<GuiTableComposition*>(newParent);
			}

			void GuiTableSplitterCompositionBase::OnLeftButtonDown(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
			{
				dragging = true;
				draggingPoint = Point(arguments.x, arguments.y);
			}

			void GuiTableSplitterCompositionBase::OnLeftButtonUp(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
			{
				dragging = false;
			}

			void GuiTableSplitterCompositionBase::OnMouseMoveHelper(
				vint cellsBefore,
				vint GuiTableComposition::* cells,
				collections::Array<vint>& cellSizes,
				vint offset,
				GuiCellOption(GuiTableComposition::*getOption)(vint),
				void(GuiTableComposition::*setOption)(vint, GuiCellOption)
				)
			{
				if (dragging)
				{
					if (tableParent)
					{
						if (0 < cellsBefore && cellsBefore < tableParent->*cells)
						{
							auto o1 = (tableParent->*getOption)(cellsBefore - 1);
							auto o2 = (tableParent->*getOption)(cellsBefore);

							vint indexStart = -1;
							vint indexEnd = -1;
							vint indexStep = -1;
							vint max = 0;

							if (offset < 0)
							{
								indexStart = cellsBefore - 1;
								indexEnd = -1;
								indexStep = -1;
							}
							else if (offset > 0)
							{
								indexStart = cellsBefore;
								indexEnd = tableParent->*cells;
								indexStep = 1;
							}
							else
							{
								return;
							}

							{
								auto o = (tableParent->*getOption)(indexStart);
								if (o.composeType == GuiCellOption::Absolute)
								{
									max = o.absolute - 1;
								}
								else
								{
									for (vint i = indexStart; i != indexEnd; i += indexStep)
									{
										o = (tableParent->*getOption)(i);
										if (o.composeType == GuiCellOption::Absolute)
										{
											break;
										}
										else if (o.composeType == GuiCellOption::Percentage)
										{
											max += cellSizes[i] - 1;
										}
									}
								}

								if (max <= 0)
								{
									return;
								}
							}

							if (offset < 0)
							{
								if (max < -offset)
								{
									offset = -max;
								}
							}
							else
							{
								if (max < offset)
								{
									offset = max;
								}
							}

							if (o1.composeType == GuiCellOption::Absolute)
							{
								o1.absolute += offset;
								(tableParent->*setOption)(cellsBefore - 1, o1);
							}
							if (o2.composeType == GuiCellOption::Absolute)
							{
								o2.absolute -= offset;
								(tableParent->*setOption)(cellsBefore, o2);
							}
							tableParent->ForceCalculateSizeImmediately();
						}
					}
				}
			}

			Rect GuiTableSplitterCompositionBase::GetBoundsHelper(
				vint cellsBefore,
				vint GuiTableComposition::* cells,
				vint(Rect::* dimSize)()const,
				collections::Array<vint>& cellOffsets,
				vint Rect::* dimU1,
				vint Rect::* dimU2,
				vint Rect::* dimV1,
				vint Rect::* dimV2
				)
			{
				Rect result;
				if (tableParent)
				{
					if (0 < cellsBefore && cellsBefore < tableParent->*cells)
					{
						vint offset = tableParent->borderVisible ? tableParent->cellPadding : 0;
						result.*dimU1 = offset;
						result.*dimU2 = offset + (tableParent->Layout_CalculateCellArea(tableParent->GetCachedBounds()).*dimSize)();
						result.*dimV1 = offset + cellOffsets[cellsBefore] - tableParent->cellPadding;
						result.*dimV2 = (result.*dimV1) + tableParent->cellPadding;
					}
				}
				return result;
			}
			
			GuiTableSplitterCompositionBase::GuiTableSplitterCompositionBase()
			{
				GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiTableSplitterCompositionBase::OnLeftButtonDown);
				GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiTableSplitterCompositionBase::OnLeftButtonUp);
			}

			GuiTableComposition* GuiTableSplitterCompositionBase::GetTableParent()
			{
				return tableParent;
			}

/***********************************************************************
GuiRowSplitterComposition
***********************************************************************/

			void GuiRowSplitterComposition::OnMouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
			{
				OnMouseMoveHelper(
					rowsToTheTop,
					&GuiTableComposition::rows,
					tableParent->layout_rowSizes,
					arguments.y - draggingPoint.y,
					&GuiTableComposition::GetRowOption,
					&GuiTableComposition::SetRowOption
					);
			}

			Rect GuiRowSplitterComposition::Layout_CalculateBounds(Size parentSize)
			{
				return GetBoundsHelper(
					rowsToTheTop,
					&GuiTableComposition::rows,
					&Rect::Width,
					tableParent->layout_rowOffsets,
					&Rect::x1,
					&Rect::x2,
					&Rect::y1,
					&Rect::y2
					);
			}
			
			GuiRowSplitterComposition::GuiRowSplitterComposition()
			{
				if (auto controller = GetCurrentController())
				{
					SetAssociatedCursor(controller->ResourceService()->GetSystemCursor(INativeCursor::SizeNS));
				}
				GetEventReceiver()->mouseMove.AttachMethod(this, &GuiRowSplitterComposition::OnMouseMove);
			}

			vint GuiRowSplitterComposition::GetRowsToTheTop()
			{
				return rowsToTheTop;
			}

			void GuiRowSplitterComposition::SetRowsToTheTop(vint value)
			{
				if (rowsToTheTop != value)
				{
					rowsToTheTop = value;
					InvokeOnCompositionStateChanged();
				}
			}

/***********************************************************************
GuiColumnSplitterComposition
***********************************************************************/

			void GuiColumnSplitterComposition::OnMouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
			{
				OnMouseMoveHelper(
					columnsToTheLeft,
					&GuiTableComposition::columns,
					tableParent->layout_columnSizes,
					arguments.x - draggingPoint.x,
					&GuiTableComposition::GetColumnOption,
					&GuiTableComposition::SetColumnOption
					);
			}

			Rect GuiColumnSplitterComposition::Layout_CalculateBounds(Size parentSize)
			{
				return GetBoundsHelper(
					columnsToTheLeft,
					&GuiTableComposition::columns,
					&Rect::Height,
					tableParent->layout_columnOffsets,
					&Rect::y1,
					&Rect::y2,
					&Rect::x1,
					&Rect::x2
					);
			}
			
			GuiColumnSplitterComposition::GuiColumnSplitterComposition()
			{
				if (auto controller = GetCurrentController())
				{
					SetAssociatedCursor(controller->ResourceService()->GetSystemCursor(INativeCursor::SizeWE));
				}
				GetEventReceiver()->mouseMove.AttachMethod(this, &GuiColumnSplitterComposition::OnMouseMove);
			}

			vint GuiColumnSplitterComposition::GetColumnsToTheLeft()
			{
				return columnsToTheLeft;
			}

			void GuiColumnSplitterComposition::SetColumnsToTheLeft(vint value)
			{
				if (columnsToTheLeft != value)
				{
					columnsToTheLeft = value;
					InvokeOnCompositionStateChanged();
				}
			}
		}
	}
}