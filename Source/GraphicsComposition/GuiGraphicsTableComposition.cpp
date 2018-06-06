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

			vint GuiTableComposition::GetSiteIndex(vint _rows, vint _columns, vint _row, vint _column)
			{
				return _row*_columns + _column;
			}

			void GuiTableComposition::SetSitedCell(vint _row, vint _column, GuiCellComposition* cell)
			{
				cellCompositions[GetSiteIndex(rows, columns, _row, _column)] = cell;
			}

			void GuiTableComposition::UpdateCellBoundsInternal(
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
				vint(*getCol)(vint, vint),
				vint maxPass
			)
			{
				for (vint pass = 0; pass < maxPass; pass++)
				{
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (pass == 0)
						{
							dimSizes[i] = 0;
						}
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
									GuiCellComposition* cell = GetSitedCell(getRow(i, j), getCol(i, j));
									if (cell)
									{
										bool accept = false;
										if (pass == 0)
										{
											accept = getSpan(cell) == 1;
										}
										else
										{
											accept = getLocation(cell) + getSpan(cell) == i + 1;
										}
										if (accept)
										{
											vint size = getSize(cell->GetPreferredBounds().GetSize());
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
				}

				bool percentageExists = false;
				for (vint i = 0; i < this->*dim1; i++)
				{
					GuiCellOption option = dimOptions[i];
					if (option.composeType == GuiCellOption::Percentage)
					{
						if (0.001 < option.percentage)
						{
							percentageExists = true;
						}
					}
				}

				if (percentageExists)
				{
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							if (0.001 < option.percentage)
							{
								for (vint j = 0; j < this->*dim2; j++)
								{
									GuiCellComposition* cell = GetSitedCell(getRow(i, j), getCol(i, j));
									if (cell)
									{
										vint size = getSize(cell->GetPreferredBounds().GetSize());
										vint start = getLocation(cell);
										vint span = getSpan(cell);
										size -= (span - 1)*cellPadding;
										double totalPercentage = 0;

										for (vint k = start; k < start + span; k++)
										{
											if (dimOptions[k].composeType == GuiCellOption::Percentage)
											{
												if (0.001 < dimOptions[k].percentage)
												{
													totalPercentage += dimOptions[k].percentage;
												}
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
					}

					vint percentageTotalSize = 0;
					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							if (0.001 < option.percentage)
							{
								vint size = (vint)ceil(dimSizes[i] / option.percentage);
								if (percentageTotalSize < size)
								{
									percentageTotalSize = size;
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
							if (0.001 < option.percentage)
							{
								totalPercentage += option.percentage;
							}
						}
					}

					for (vint i = 0; i < this->*dim1; i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							if (0.001 < option.percentage)
							{
								vint size = (vint)ceil(percentageTotalSize*option.percentage / totalPercentage);
								if (dimSizes[i] < size)
								{
									dimSizes[i] = size;
								}
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

			void GuiTableComposition::UpdateCellBoundsPercentages(
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
					for (vint i = 0; i < dimOptions.Count(); i++)
					{
						GuiCellOption option = dimOptions[i];
						if (option.composeType == GuiCellOption::Percentage)
						{
							totalPercentage += option.percentage;
							percentageCount++;
						}
					}
					if (percentageCount > 0 && totalPercentage > 0.001)
					{
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

			vint GuiTableComposition::UpdateCellBoundsOffsets(
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
				return max - right;
			}

			void GuiTableComposition::OnRenderContextChanged()
			{
				if(GetRenderTarget())
				{
					UpdateCellBounds();
				}
			}

			GuiTableComposition::GuiTableComposition()
				:rows(0)
				, columns(0)
				, cellPadding(0)
				, borderVisible(true)
				, rowExtending(0)
				, columnExtending(0)
			{
				ConfigChanged.SetAssociatedComposition(this);
				SetRowsAndColumns(1, 1);
			}

			GuiTableComposition::~GuiTableComposition()
			{
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
				cellCompositions.Resize(_rows*_columns);
				cellBounds.Resize(_rows*_columns);
				for (vint i = 0; i < _rows*_columns; i++)
				{
					cellCompositions[i] = 0;
					cellBounds[i] = Rect();
				}
				rows = _rows;
				columns = _columns;
				vint childCount = Children().Count();
				for (vint i = 0; i < childCount; i++)
				{
					GuiCellComposition* cell = dynamic_cast<GuiCellComposition*>(Children().Get(i));
					if (cell)
					{
						cell->OnTableRowsAndColumnsChanged();
					}
				}
				ConfigChanged.Execute(GuiEventArgs(this));
				UpdateCellBounds();
				return true;
			}

			GuiCellComposition* GuiTableComposition::GetSitedCell(vint _row, vint _column)
			{
				return cellCompositions[GetSiteIndex(rows, columns, _row, _column)];
			}

			GuiCellOption GuiTableComposition::GetRowOption(vint _row)
			{
				return rowOptions[_row];
			}

			void GuiTableComposition::SetRowOption(vint _row, GuiCellOption option)
			{
				rowOptions[_row] = option;
				UpdateCellBounds();
				ConfigChanged.Execute(GuiEventArgs(this));
			}

			GuiCellOption GuiTableComposition::GetColumnOption(vint _column)
			{
				return columnOptions[_column];
			}

			void GuiTableComposition::SetColumnOption(vint _column, GuiCellOption option)
			{
				columnOptions[_column] = option;
				UpdateCellBounds();
				ConfigChanged.Execute(GuiEventArgs(this));
			}

			vint GuiTableComposition::GetCellPadding()
			{
				return cellPadding;
			}

			void GuiTableComposition::SetCellPadding(vint value)
			{
				if (value < 0) value = 0;
				cellPadding = value;
				UpdateCellBounds();
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
					UpdateCellBounds();
				}
			}

			Rect GuiTableComposition::GetCellArea()
			{
				Rect bounds(Point(0, 0), GuiBoundsComposition::GetBounds().GetSize());
				vint borderThickness = borderVisible ? cellPadding : 0;
				bounds.x1 += margin.left + internalMargin.left + borderThickness;
				bounds.y1 += margin.top + internalMargin.top + borderThickness;
				bounds.x2 -= margin.right + internalMargin.right + borderThickness;
				bounds.y2 -= margin.bottom + internalMargin.bottom + borderThickness;
				if (bounds.x2 < bounds.x1) bounds.x2 = bounds.x1;
				if (bounds.y2 < bounds.y1) bounds.y2 = bounds.y1;
				return bounds;
			}

			void GuiTableComposition::UpdateCellBounds()
			{
				rowOffsets.Resize(rows);
				rowSizes.Resize(rows);
				columnOffsets.Resize(columns);
				columnSizes.Resize(columns);

				vint rowTotal = (rows - 1) * cellPadding;
				vint columnTotal = (columns - 1) * cellPadding;
				vint rowTotalWithPercentage = rowTotal;
				vint columnTotalWithPercentage = columnTotal;

				UpdateCellBoundsInternal(
					rowSizes,
					rowTotal,
					rowTotalWithPercentage,
					rowOptions,
					&GuiTableComposition::rows,
					&GuiTableComposition::columns,
					&Y,
					&RL,
					&RS,
					&First,
					&Second,
					1
				);
				UpdateCellBoundsInternal(
					columnSizes,
					columnTotal,
					columnTotalWithPercentage,
					columnOptions,
					&GuiTableComposition::columns,
					&GuiTableComposition::rows,
					&X,
					&CL,
					&CS,
					&Second,
					&First,
					1
				);

				Rect area = GetCellArea();
				UpdateCellBoundsPercentages(rowSizes, rowTotal, area.Height(), rowOptions);
				UpdateCellBoundsPercentages(columnSizes, columnTotal, area.Width(), columnOptions);
				rowExtending = UpdateCellBoundsOffsets(rowOffsets, rowSizes, area.Height());
				columnExtending = UpdateCellBoundsOffsets(columnOffsets, columnSizes, area.Width());

				for (vint i = 0; i < rows; i++)
				{
					for (vint j = 0; j < columns; j++)
					{
						vint index = GetSiteIndex(rows, columns, i, j);
						cellBounds[index] = Rect(Point(columnOffsets[j], rowOffsets[i]), Size(columnSizes[j], rowSizes[i]));
					}
				}

				tableContentMinSize = Size(columnTotalWithPercentage, rowTotalWithPercentage);
				InvokeOnCompositionStateChanged();
			}

			void GuiTableComposition::ForceCalculateSizeImmediately()
			{
				GuiBoundsComposition::ForceCalculateSizeImmediately();
				UpdateCellBounds();
				UpdateCellBounds();
			}

			Size GuiTableComposition::GetMinPreferredClientSize()
			{
				vint offset = (borderVisible ? 2 * cellPadding : 0);
				return Size(tableContentMinSize.x + offset, tableContentMinSize.y + offset);
			}

			Rect GuiTableComposition::GetBounds()
			{
				Rect cached = previousBounds;
				Rect result = GuiBoundsComposition::GetBounds();

				bool cellMinSizeModified = false;
				SortedList<GuiCellComposition*> cells;
				FOREACH(GuiCellComposition*, cell, cellCompositions)
				{
					if (cell && !cells.Contains(cell))
					{
						cells.Add(cell);
						Size newSize = cell->GetPreferredBounds().GetSize();
						if (cell->lastPreferredSize != newSize)
						{
							cell->lastPreferredSize = newSize;
							cellMinSizeModified = true;
						}
					}
				}

				if (cached != result || cellMinSizeModified)
				{
					UpdateCellBounds();
				}
				return result;
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
							table->SetSitedCell(row + r, column + c, 0);
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
				if (tableParent)
				{
					if (_row < 0 || _row >= tableParent->rows || _column < 0 || _column >= tableParent->columns) return false;
					if (_rowSpan<1 || _row + _rowSpan>tableParent->rows || _columnSpan<1 || _column + _columnSpan>tableParent->columns) return false;

					for (vint r = 0; r < _rowSpan; r++)
					{
						for (vint c = 0; c < _columnSpan; c++)
						{
							GuiCellComposition* cell = tableParent->GetSitedCell(_row + r, _column + c);
							if (cell && cell != this)
							{
								return false;
							}
						}
					}
					ClearSitedCells(tableParent);
				}

				row = _row;
				column = _column;
				rowSpan = _rowSpan;
				columnSpan = _columnSpan;

				if (tableParent)
				{
					SetSitedCells(tableParent);
				}
				return true;
			}

			void GuiCellComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
				if (tableParent)
				{
					ClearSitedCells(tableParent);
				}
				tableParent = dynamic_cast<GuiTableComposition*>(newParent);
				if (!tableParent || !SetSiteInternal(row, column, rowSpan, columnSpan))
				{
					ResetSiteInternal();
				}
				if (tableParent)
				{
					if (row != -1 && column != -1)
					{
						SetSiteInternal(row, column, rowSpan, columnSpan);
					}
					tableParent->UpdateCellBounds();
				}
			}

			void GuiCellComposition::OnTableRowsAndColumnsChanged()
			{
				if(!SetSiteInternal(row, column, rowSpan, columnSpan))
				{
					ResetSiteInternal();
				}
			}

			GuiCellComposition::GuiCellComposition()
				:row(-1)
				,column(-1)
				,rowSpan(1)
				,columnSpan(1)
				,tableParent(0)
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiCellComposition::~GuiCellComposition()
			{
			}

			GuiTableComposition* GuiCellComposition::GetTableParent()
			{
				return tableParent;
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

				if (tableParent)
				{
					tableParent->UpdateCellBounds();
				}
				return true;
			}

			Rect GuiCellComposition::GetBounds()
			{
				Rect result;
				if(tableParent && row!=-1 && column!=-1)
				{
					Rect bounds1, bounds2;
					{
						vint index=tableParent->GetSiteIndex(tableParent->rows, tableParent->columns, row, column);
						bounds1=tableParent->cellBounds[index];
					}
					{
						vint index=tableParent->GetSiteIndex(tableParent->rows, tableParent->columns, row+rowSpan-1, column+columnSpan-1);
						bounds2=tableParent->cellBounds[index];
						if(tableParent->GetMinSizeLimitation()==GuiGraphicsComposition::NoLimit)
						{
							if(row+rowSpan==tableParent->rows)
							{
								bounds2.y2+=tableParent->rowExtending;
							}
							if(column+columnSpan==tableParent->columns)
							{
								bounds2.x2+=tableParent->columnExtending;
							}
						}
					}
					vint offset = tableParent->borderVisible ? tableParent->cellPadding : 0;
					result = Rect(bounds1.x1 + offset, bounds1.y1 + offset, bounds2.x2 + offset, bounds2.y2 + offset);
				}
				else
				{
					result = Rect();
				}
				UpdatePreviousBounds(result);
				return result;
			}

/***********************************************************************
GuiTableSplitterCompositionBase
***********************************************************************/

			void GuiTableSplitterCompositionBase::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
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
				Rect result(0, 0, 0, 0);
				if (tableParent)
				{
					if (0 < cellsBefore && cellsBefore < tableParent->*cells)
					{
						vint offset = tableParent->borderVisible ? tableParent->cellPadding : 0;
						result.*dimU1 = offset;
						result.*dimU2 = offset + (tableParent->GetCellArea().*dimSize)();
						result.*dimV1 = offset + cellOffsets[cellsBefore] - tableParent->cellPadding;
						result.*dimV2 = (result.*dimV1) + tableParent->cellPadding;
					}
				}
				UpdatePreviousBounds(result);
				return result;
			}
			
			GuiTableSplitterCompositionBase::GuiTableSplitterCompositionBase()
				:tableParent(0)
				, dragging(false)
			{
				SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeNS));
				GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiTableSplitterCompositionBase::OnLeftButtonDown);
				GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiTableSplitterCompositionBase::OnLeftButtonUp);
			}

			GuiTableSplitterCompositionBase::~GuiTableSplitterCompositionBase()
			{
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
					tableParent->rowSizes,
					arguments.y - draggingPoint.y,
					&GuiTableComposition::GetRowOption,
					&GuiTableComposition::SetRowOption
					);
			}
			
			GuiRowSplitterComposition::GuiRowSplitterComposition()
				:rowsToTheTop(0)
			{
				SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeNS));
				GetEventReceiver()->mouseMove.AttachMethod(this, &GuiRowSplitterComposition::OnMouseMove);
			}

			GuiRowSplitterComposition::~GuiRowSplitterComposition()
			{
			}

			vint GuiRowSplitterComposition::GetRowsToTheTop()
			{
				return rowsToTheTop;
			}

			void GuiRowSplitterComposition::SetRowsToTheTop(vint value)
			{
				rowsToTheTop = value;
				InvokeOnCompositionStateChanged();
			}

			Rect GuiRowSplitterComposition::GetBounds()
			{
				return GetBoundsHelper(
					rowsToTheTop,
					&GuiTableComposition::rows,
					&Rect::Width,
					tableParent->rowOffsets,
					&Rect::x1,
					&Rect::x2,
					&Rect::y1,
					&Rect::y2
					);
			}

/***********************************************************************
GuiColumnSplitterComposition
***********************************************************************/

			void GuiColumnSplitterComposition::OnMouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
			{
				OnMouseMoveHelper(
					columnsToTheLeft,
					&GuiTableComposition::columns,
					tableParent->columnSizes,
					arguments.x - draggingPoint.x,
					&GuiTableComposition::GetColumnOption,
					&GuiTableComposition::SetColumnOption
					);
			}
			
			GuiColumnSplitterComposition::GuiColumnSplitterComposition()
				:columnsToTheLeft(0)
			{
				SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeWE));
				GetEventReceiver()->mouseMove.AttachMethod(this, &GuiColumnSplitterComposition::OnMouseMove);
			}

			GuiColumnSplitterComposition::~GuiColumnSplitterComposition()
			{
			}

			vint GuiColumnSplitterComposition::GetColumnsToTheLeft()
			{
				return columnsToTheLeft;
			}

			void GuiColumnSplitterComposition::SetColumnsToTheLeft(vint value)
			{
				columnsToTheLeft = value;
				InvokeOnCompositionStateChanged();
			}

			Rect GuiColumnSplitterComposition::GetBounds()
			{
				return GetBoundsHelper(
					columnsToTheLeft,
					&GuiTableComposition::columns,
					&Rect::Height,
					tableParent->columnOffsets,
					&Rect::y1,
					&Rect::y2,
					&Rect::x1,
					&Rect::x2
					);
			}
		}
	}
}