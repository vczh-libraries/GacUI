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
				return _row*_columns+_column;
			}

			void GuiTableComposition::SetSitedCell(vint _row, vint _column, GuiCellComposition* cell)
			{
				cellCompositions[GetSiteIndex(rows, columns, _row, _column)]=cell;
			}

			void GuiTableComposition::UpdateCellBoundsInternal(
				collections::Array<vint>& dimSizes,
				vint& dimSize,
				vint& dimSizeWithPercentage,
				collections::Array<GuiCellOption>& dimOptions,
				vint GuiTableComposition::* dim1,
				vint GuiTableComposition::* dim2,
				vint (*getSize)(Size),
				vint (*getLocation)(GuiCellComposition*),
				vint (*getSpan)(GuiCellComposition*),
				vint (*getRow)(vint, vint),
				vint (*getCol)(vint, vint),
				vint maxPass
				)
			{
				for(vint pass=0;pass<maxPass;pass++)
				{
					for(vint i=0;i<this->*dim1;i++)
					{
						GuiCellOption option=dimOptions[i];
						if(pass==0)
						{
							dimSizes[i]=0;
						}
						switch(option.composeType)
						{
						case GuiCellOption::Absolute:
							{
								dimSizes[i]=option.absolute;
							}
							break;
						case GuiCellOption::MinSize:
							{
								for(vint j=0;j<this->*dim2;j++)
								{
									GuiCellComposition* cell=GetSitedCell(getRow(i, j), getCol(i, j));
									if(cell)
									{
										bool accept=false;
										if(pass==0)
										{
											accept=getSpan(cell)==1;
										}
										else
										{
											accept=getLocation(cell)+getSpan(cell)==i+1;
										}
										if(accept)
										{
											vint size=getSize(cell->GetPreferredBounds().GetSize());
											vint span=getSpan(cell);
											for(vint k=1;k<span;k++)
											{
												size-=dimSizes[i-k]+cellPadding;
											}
											if(dimSizes[i]<size)
											{
												dimSizes[i]=size;
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
				
				bool percentageExists=false;
				for(vint i=0;i<this->*dim1;i++)
				{
					GuiCellOption option=dimOptions[i];
					if(option.composeType==GuiCellOption::Percentage)
					{
						if(0.001<option.percentage)
						{
							percentageExists=true;
						}
					}
				}

				if(percentageExists)
				{
					for(vint i=0;i<this->*dim1;i++)
					{
						GuiCellOption option=dimOptions[i];
						if(option.composeType==GuiCellOption::Percentage)
						{
							if(0.001<option.percentage)
							{
								for(vint j=0;j<this->*dim2;j++)
								{
									GuiCellComposition* cell=GetSitedCell(getRow(i, j), getCol(i, j));
									if(cell)
									{
										vint size=getSize(cell->GetPreferredBounds().GetSize());
										vint start=getLocation(cell);
										vint span=getSpan(cell);
										size-=(span-1)*cellPadding;
										double totalPercentage=0;

										for(vint k=start;k<start+span;k++)
										{
											if(dimOptions[k].composeType==GuiCellOption::Percentage)
											{
												if(0.001<dimOptions[k].percentage)
												{
													totalPercentage+=dimOptions[k].percentage;
												}
											}
											else
											{
												size-=dimSizes[k];
											}
										}

										size=(vint)ceil(size*option.percentage/totalPercentage);
										if(dimSizes[i]<size)
										{
											dimSizes[i]=size;
										}
									}
								}
							}
						}
					}

					vint percentageTotalSize=0;
					for(vint i=0;i<this->*dim1;i++)
					{
						GuiCellOption option=dimOptions[i];
						if(option.composeType==GuiCellOption::Percentage)
						{
							if(0.001<option.percentage)
							{
								vint size=(vint)ceil(dimSizes[i]/option.percentage);
								if(percentageTotalSize<size)
								{
									percentageTotalSize=size;
								}
							}
						}
					}

					double totalPercentage=0;
					for(vint i=0;i<this->*dim1;i++)
					{
						GuiCellOption option=dimOptions[i];
						if(option.composeType==GuiCellOption::Percentage)
						{
							if(0.001<option.percentage)
							{
								totalPercentage+=option.percentage;
							}
						}
					}
					
					for(vint i=0;i<this->*dim1;i++)
					{
						GuiCellOption option=dimOptions[i];
						if(option.composeType==GuiCellOption::Percentage)
						{
							if(0.001<option.percentage)
							{
								vint size=(vint)ceil(percentageTotalSize*option.percentage/totalPercentage);
								if(dimSizes[i]<size)
								{
									dimSizes[i]=size;
								}
							}
						}
					}
				}

				for(vint i=0;i<this->*dim1;i++)
				{
					if(dimOptions[i].composeType!=GuiCellOption::Percentage)
					{
						dimSize+=dimSizes[i];
					}
					dimSizeWithPercentage+=dimSizes[i];
				}
			}

			void GuiTableComposition::UpdateCellBoundsPercentages(
				collections::Array<vint>& dimSizes,
				vint dimSize,
				vint maxDimSize,
				collections::Array<GuiCellOption>& dimOptions
				)
			{
				if(maxDimSize>dimSize)
				{
					double totalPercentage=0;
					vint percentageCount=0;
					for(vint i=0;i<dimOptions.Count();i++)
					{
						GuiCellOption option=dimOptions[i];
						if(option.composeType==GuiCellOption::Percentage)
						{
							totalPercentage+=option.percentage;
							percentageCount++;
						}
					}
					if(percentageCount>0 && totalPercentage>0.001)
					{
						for(vint i=0;i<dimOptions.Count();i++)
						{
							GuiCellOption option=dimOptions[i];
							if(option.composeType==GuiCellOption::Percentage)
							{
								dimSizes[i]=(vint)((maxDimSize-dimSize)*option.percentage/totalPercentage);
							}
						}
					}
				}
			}

			vint GuiTableComposition::UpdateCellBoundsOffsets(
				collections::Array<vint>& offsets,
				collections::Array<vint>& sizes,
				vint start,
				vint max
				)
			{
				offsets[0]=start;
				for(vint i=1;i<offsets.Count();i++)
				{
					start+=cellPadding+sizes[i-1];
					offsets[i]=start;
				}

				vint last=offsets.Count()-1;
				vint right=offsets[last]+sizes[last];
				return max-right;
			}

			void GuiTableComposition::UpdateCellBoundsInternal()
			{
				Array<vint> rowOffsets, columnOffsets, rowSizes, columnSizes;
				rowOffsets.Resize(rows);
				rowSizes.Resize(rows);
				columnOffsets.Resize(columns);
				columnSizes.Resize(columns);
				{
					vint rowTotal=(rows-1)*cellPadding;
					vint columnTotal=(columns-1)*cellPadding;
					vint rowTotalWithPercentage=rowTotal;
					vint columnTotalWithPercentage=columnTotal;

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

					Rect area=GetCellArea();
					UpdateCellBoundsPercentages(rowSizes, rowTotal, area.Height(), rowOptions);
					UpdateCellBoundsPercentages(columnSizes, columnTotal, area.Width(), columnOptions);
					rowExtending=UpdateCellBoundsOffsets(rowOffsets, rowSizes, cellPadding, cellPadding+area.Height());
					columnExtending=UpdateCellBoundsOffsets(columnOffsets, columnSizes, cellPadding, cellPadding+area.Width());

					for(vint i=0;i<rows;i++)
					{
						for(vint j=0;j<columns;j++)
						{
							vint index=GetSiteIndex(rows, columns, i, j);
							cellBounds[index]=Rect(Point(columnOffsets[j], rowOffsets[i]), Size(columnSizes[j], rowSizes[i]));
						}
					}
				}
			}

			void GuiTableComposition::UpdateTableContentMinSize()
			{
				Array<vint> rowSizes, columnSizes;
				rowSizes.Resize(rows);
				columnSizes.Resize(columns);
				{
					vint rowTotal=(rows+1)*cellPadding;
					vint columnTotal=(columns+1)*cellPadding;
					vint rowTotalWithPercentage=rowTotal;
					vint columnTotalWithPercentage=columnTotal;

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
						2
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
						2
						);
					tableContentMinSize=Size(columnTotalWithPercentage, rowTotalWithPercentage);
				}
				if(previousContentMinSize!=tableContentMinSize)
				{
					previousContentMinSize=tableContentMinSize;
					UpdateCellBoundsInternal();
				}
			}

			void GuiTableComposition::OnRenderTargetChanged()
			{
				if(GetRenderTarget())
				{
					UpdateTableContentMinSize();
				}
			}

			GuiTableComposition::GuiTableComposition()
				:rows(0)
				,columns(0)
				,cellPadding(0)
				,rowExtending(0)
				,columnExtending(0)
			{
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
				if(_rows<=0 || _columns<=0) return false;
				rowOptions.Resize(_rows);
				columnOptions.Resize(_columns);
				cellCompositions.Resize(_rows*_columns);
				cellBounds.Resize(_rows*_columns);
				for(vint i=0;i<_rows*_columns;i++)
				{
					cellCompositions[i]=0;
					cellBounds[i]=Rect();
				}
				rows=_rows;
				columns=_columns;
				vint childCount=Children().Count();
				for(vint i=0;i<childCount;i++)
				{
					GuiCellComposition* cell=dynamic_cast<GuiCellComposition*>(Children().Get(i));
					if(cell)
					{
						cell->OnTableRowsAndColumnsChanged();
					}
				}
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
				rowOptions[_row]=option;
			}

			GuiCellOption GuiTableComposition::GetColumnOption(vint _column)
			{
				return columnOptions[_column];
			}

			void GuiTableComposition::SetColumnOption(vint _column, GuiCellOption option)
			{
				columnOptions[_column]=option;
			}

			vint GuiTableComposition::GetCellPadding()
			{
				return cellPadding;
			}

			void GuiTableComposition::SetCellPadding(vint value)
			{
				if(value<0) value=0;
				cellPadding=value;
			}

			Rect GuiTableComposition::GetCellArea()
			{
				Rect bounds(Point(0, 0), GuiBoundsComposition::GetBounds().GetSize());
				bounds.x1+=margin.left+internalMargin.left+cellPadding;
				bounds.y1+=margin.top+internalMargin.top+cellPadding;
				bounds.x2-=margin.right+internalMargin.right+cellPadding;
				bounds.y2-=margin.bottom+internalMargin.bottom+cellPadding;
				if(bounds.x2<bounds.x1) bounds.x2=bounds.x1;
				if(bounds.y2<bounds.y1) bounds.y2=bounds.y1;
				return bounds;
			}

			void GuiTableComposition::UpdateCellBounds()
			{
				UpdateCellBoundsInternal();
				UpdateTableContentMinSize();
			}

			void GuiTableComposition::ForceCalculateSizeImmediately()
			{
				GuiBoundsComposition::ForceCalculateSizeImmediately();
				UpdateCellBounds();
				UpdateCellBounds();
			}

			Size GuiTableComposition::GetMinPreferredClientSize()
			{
				return tableContentMinSize;
			}

			Rect GuiTableComposition::GetBounds()
			{
				Rect result;
				if(!IsAlignedToParent() && GetMinSizeLimitation()!=GuiGraphicsComposition::NoLimit)
				{
					result=Rect(compositionBounds.LeftTop(), compositionBounds.GetSize()-Size(columnExtending, rowExtending));
				}
				else
				{
					result=GuiBoundsComposition::GetBounds();
				}

				bool cellMinSizeModified=false;
				SortedList<GuiCellComposition*> cells;
				FOREACH(GuiCellComposition*, cell, cellCompositions)
				{
					if(cell && !cells.Contains(cell))
					{
						cells.Add(cell);
						Size newSize=cell->GetPreferredBounds().GetSize();
						if(cell->lastPreferredSize!=newSize)
						{
							cell->lastPreferredSize=newSize;
							cellMinSizeModified=true;
						}
					}
				}

				if(previousBounds!=result || cellMinSizeModified)
				{
					previousBounds=result;
					UpdateCellBounds();
				}
				return result;
			}

/***********************************************************************
GuiCellComposition
***********************************************************************/

			void GuiCellComposition::ClearSitedCells(GuiTableComposition* table)
			{
				if(row!=-1 && column!=-1)
				{
					for(vint r=0;r<rowSpan;r++)
					{
						for(vint c=0;c<columnSpan;c++)
						{
							table->SetSitedCell(row+r, column+c, 0);
						}
					}
				}
			}

			void GuiCellComposition::SetSitedCells(GuiTableComposition* table)
			{
				for(vint r=0;r<rowSpan;r++)
				{
					for(vint c=0;c<columnSpan;c++)
					{
						table->SetSitedCell(row+r, column+c, this);
					}
				}
			}

			void GuiCellComposition::ResetSiteInternal()
			{
				row=-1;
				column=-1;
				rowSpan=1;
				columnSpan=1;
			}

			bool GuiCellComposition::SetSiteInternal(vint _row, vint _column, vint _rowSpan, vint _columnSpan)
			{
				if (tableParent)
				{
					if(_row<0 || _row>=tableParent->rows || _column<0 || _column>=tableParent->columns) return false;
					if(_rowSpan<1 || _row+_rowSpan>tableParent->rows || _columnSpan<1 || _column+_columnSpan>tableParent->columns) return false;

					for(vint r=0;r<_rowSpan;r++)
					{
						for(vint c=0;c<_columnSpan;c++)
						{
							GuiCellComposition* cell=tableParent->GetSitedCell(_row+r, _column+c);
							if(cell && cell!=this)
							{
								return false;
							}
						}
					}
					ClearSitedCells(tableParent);
				}

				row=_row;
				column=_column;
				rowSpan=_rowSpan;
				columnSpan=_columnSpan;

				if (tableParent)
				{
					SetSitedCells(tableParent);
				}
				return true;
			}

			void GuiCellComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
				if(tableParent)
				{
					ClearSitedCells(tableParent);
				}
				tableParent=dynamic_cast<GuiTableComposition*>(newParent);
				if(!tableParent || !SetSiteInternal(row, column, rowSpan, columnSpan))
				{
					ResetSiteInternal();
				}
				if(tableParent)
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
				if(SetSiteInternal(_row, _column, _rowSpan, _columnSpan))
				{
					if (tableParent)
					{
						tableParent->UpdateCellBounds();
					}
					return true;
				}
				else
				{
					return false;
				}
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
					result = Rect(bounds1.x1, bounds1.y1, bounds2.x2, bounds2.y2);
				}
				else
				{
					result = Rect();
				}
				UpdatePreviousBounds(result);
				return result;
			}
		}
	}
}