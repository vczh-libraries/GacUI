/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSTABLECOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSTABLECOMPOSITION

#include "IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Table Compositions
***********************************************************************/

			/// <summary>
			/// Represnets a sizing configuration for a row or a column.
			/// </summary>
			struct GuiCellOption
			{
				/// <summary>Size configuration</summary>
				enum ComposeType
				{
					/// <summary>
					/// Set the size to an absolute value.
					/// The size will not change even if affected cell's minimum size is bigger that this.
					/// </summary>
					Absolute,
					/// <summary>
					/// Set the size to a percentage number of the whole table.
					/// </summary>
					Percentage,
					/// <summary>
					/// Set the size to the minimum size of the cell element.
					/// Only cells that take one row or column at this position are considered.
					/// </summary>
					MinSize,
				};

				/// <summary>Sizing algorithm</summary>
				ComposeType		composeType;
				/// <summary>The absolute size when <see cref="GuiCellOption::composeType"/> is <see cref="ComposeType"/>::Absolute.</summary>
				vint			absolute;
				/// <summary>The percentage number when <see cref="GuiCellOption::composeType"/> is <see cref="ComposeType"/>::Percentage.</summary>
				double			percentage;

				GuiCellOption()
					:composeType(Absolute)
					,absolute(20)
					,percentage(0)
				{
				}

				GUI_DEFINE_COMPARE_OPERATORS(GuiCellOption)

				/// <summary>Creates an absolute sizing option</summary>
				/// <returns>The created option.</returns>
				/// <param name="value">The absolute size.</param>
				static GuiCellOption AbsoluteOption(vint value)
				{
					GuiCellOption option;
					option.composeType=Absolute;
					option.absolute=value;
					return option;
				}
				
				/// <summary>Creates an percantage sizing option</summary>
				/// <returns>The created option.</returns>
				/// <param name="value">The percentage number.</param>
				static GuiCellOption PercentageOption(double value)
				{
					GuiCellOption option;
					option.composeType=Percentage;
					option.percentage=value;
					return option;
				}
				
				/// <summary>Creates an minimum sizing option</summary>
				/// <returns>The created option.</returns>
				static GuiCellOption MinSizeOption()
				{
					GuiCellOption option;
					option.composeType=MinSize;
					return option;
				}
			};

			/// <summary>
			/// Represents a table composition.
			/// </summary>
			class GuiTableComposition : public GuiBoundsComposition, public Description<GuiTableComposition>
			{
				friend class GuiCellComposition;
				friend class GuiTableSplitterCompositionBase;
				friend class GuiRowSplitterComposition;
				friend class GuiColumnSplitterComposition;
			private:
				bool										layout_invalid = true;
				bool										layout_invalidCellBounds = false;
				Size										layout_lastTableSize;

				collections::Array<GuiCellComposition*>		layout_cellCompositions;
				collections::Array<Rect>					layout_cellBounds;
				collections::Array<vint>					layout_rowOffsets;
				collections::Array<vint>					layout_columnOffsets;
				collections::Array<vint>					layout_rowSizes;
				collections::Array<vint>					layout_columnSizes;
				vint										layout_rowTotal = 0;
				vint										layout_columnTotal = 0;
				vint										layout_rowTotalWithPercentage = 0;
				vint										layout_columnTotalWithPercentage = 0;
				vint										layout_rowExtending = 0;
				vint										layout_columnExtending = 0;

				Rect										Layout_CalculateCellArea(Rect tableBounds);
				void										Layout_UpdateCellBoundsInternal(
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
																vint (*getCol)(vint, vint)
																);
				void										Layout_UpdateCellBoundsPercentages(
																collections::Array<vint>& dimSizes,
																vint dimSize,
																vint maxDimSize,
																collections::Array<GuiCellOption>& dimOptions
																);
				vint										Layout_UpdateCellBoundsOffsets(
																collections::Array<vint>& offsets,
																collections::Array<vint>& sizes,
																vint max
																);
			protected:
				vint										rows = 0;
				vint										columns = 0;
				vint										cellPadding = 0;
				bool										borderVisible = true;
				collections::Array<GuiCellOption>			rowOptions;
				collections::Array<GuiCellOption>			columnOptions;

				vint										GetSiteIndex(vint _rows, vint _columns, vint _row, vint _column);
				void										SetSitedCell(vint _row, vint _column, GuiCellComposition* cell);
				void										OnCompositionStateChanged() override;
				Size										Layout_CalculateMinSize() override;
				Rect										Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiTableComposition();
				~GuiTableComposition() = default;

				/// <summary>Event that will be raised with row numbers, column numbers or options are changed.</summary>
				compositions::GuiNotifyEvent		ConfigChanged;

				/// <summary>Get the number of rows.</summary>
				/// <returns>The number of rows.</returns>
				vint								GetRows();
				/// <summary>Get the number of columns.</summary>
				/// <returns>The number of columns.</returns>
				vint								GetColumns();
				/// <summary>Change the number of rows and columns.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="_rows">The number of rows.</param>
				/// <param name="_columns">The number of columns.</param>
				bool								SetRowsAndColumns(vint _rows, vint _columns);
				/// <summary>Get the cell composition that covers the specified cell location.</summary>
				/// <returns>The cell composition that covers the specified cell location.</returns>
				/// <param name="_row">The number of rows.</param>
				/// <param name="_column">The number of columns.</param>
				GuiCellComposition*					GetSitedCell(vint _row, vint _column);

				/// <summary>Get the sizing option of the specified row.</summary>
				/// <returns>The sizing option of the specified row.</returns>
				/// <param name="_row">The specified row number.</param>
				GuiCellOption						GetRowOption(vint _row);
				/// <summary>Set the sizing option of the specified row.</summary>
				/// <param name="_row">The specified row number.</param>
				/// <param name="option">The sizing option of the specified row.</param>
				void								SetRowOption(vint _row, GuiCellOption option);
				/// <summary>Get the sizing option of the specified column.</summary>
				/// <returns>The sizing option of the specified column.</returns>
				/// <param name="_column">The specified column number.</param>
				GuiCellOption						GetColumnOption(vint _column);
				/// <summary>Set the sizing option of the specified column.</summary>
				/// <param name="_column">The specified column number.</param>
				/// <param name="option">The sizing option of the specified column.</param>
				void								SetColumnOption(vint _column, GuiCellOption option);

				/// <summary>Get the cell padding. A cell padding is the distance between a table client area and a cell, or between two cells.</summary>
				/// <returns>The cell padding.</returns>
				vint								GetCellPadding();
				/// <summary>Set the cell padding. A cell padding is the distance between a table client area and a cell, or between two cells.</summary>
				/// <param name="value">The cell padding.</param>
				void								SetCellPadding(vint value);
				/// <summary>Get the border visibility.</summary>
				/// <returns>Returns true means the border thickness equals to the cell padding, otherwise zero.</returns>
				bool								GetBorderVisible();
				/// <summary>Set the border visibility.</summary>
				/// <param name="value">Set to true to let the border thickness equal to the cell padding, otherwise zero.</param>
				void								SetBorderVisible(bool value);
			};

			/// <summary>
			/// Represents a cell composition of a <see cref="GuiTableComposition"/>.
			/// </summary>
			class GuiCellComposition : public GuiGraphicsComposition_Controlled, public Description<GuiCellComposition>
			{
				friend class GuiTableComposition;
			private:
				GuiTableComposition*				layout_tableParent = nullptr;
			protected:
				vint								row = -1;
				vint								rowSpan = 1;
				vint								column = -1;
				vint								columnSpan = 1;
				
				void								ClearSitedCells(GuiTableComposition* table);
				void								SetSitedCells(GuiTableComposition* table);
				void								ResetSiteInternal();
				bool								SetSiteInternal(vint _row, vint _column, vint _rowSpan, vint _columnSpan);
				void								OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)override;
				void								OnTableRowsAndColumnsChanged();
				void								Layout_SetCellBounds();
			public:
				GuiCellComposition();
				~GuiCellComposition() = default;

				/// <summary>Get the owner table composition.</summary>
				/// <returns>The owner table composition.</returns>
				GuiTableComposition*				GetTableParent();

				/// <summary>Get the row number for this cell composition.</summary>
				/// <returns>The row number for this cell composition.</returns>
				vint								GetRow();
				/// <summary>Get the total numbers of acrossed rows for this cell composition.</summary>
				/// <returns>The total numbers of acrossed rows for this cell composition.</returns>
				vint								GetRowSpan();
				/// <summary>Get the column number for this cell composition.</summary>
				/// <returns>The column number for this cell composition.</returns>
				vint								GetColumn();
				/// <summary>Get the total numbers of acrossed columns for this cell composition.</summary>
				/// <returns>The total numbers of acrossed columns for this cell composition.</returns>
				vint								GetColumnSpan();
				/// <summary>Set the position for this cell composition in the table.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="_row">The row number for this cell composition.</param>
				/// <param name="_column">The column number for this cell composition.</param>
				/// <param name="_rowSpan">The total numbers of acrossed rows for this cell composition.</param>
				/// <param name="_columnSpan">The total numbers of acrossed columns for this cell composition.</param>
				bool								SetSite(vint _row, vint _column, vint _rowSpan, vint _columnSpan);
			};

			class GuiTableSplitterCompositionBase : public GuiGraphicsComposition_Specialized, public Description<GuiTableSplitterCompositionBase>
			{
			protected:
				GuiTableComposition*				tableParent = nullptr;

				bool								dragging = false;
				Point								draggingPoint;
				
				void								OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)override;
				void								OnLeftButtonDown(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments);
				void								OnLeftButtonUp(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments);

				void								OnMouseMoveHelper(
														vint cellsBefore,
														vint GuiTableComposition::* cells,
														collections::Array<vint>& cellSizes,
														vint offset,
														GuiCellOption(GuiTableComposition::*getOption)(vint),
														void(GuiTableComposition::*setOption)(vint, GuiCellOption)
														);

				Rect								GetBoundsHelper(
														vint cellsBefore,
														vint GuiTableComposition::* cells,
														vint(Rect::* dimSize)()const,
														collections::Array<vint>& cellOffsets,
														vint Rect::* dimU1,
														vint Rect::* dimU2,
														vint Rect::* dimV1,
														vint Rect::* dimV2
														);
			public:
				GuiTableSplitterCompositionBase();
				~GuiTableSplitterCompositionBase() = default;

				/// <summary>Get the owner table composition.</summary>
				/// <returns>The owner table composition.</returns>
				GuiTableComposition*				GetTableParent();
			};
			
			/// <summary>
			/// Represents a row splitter composition of a <see cref="GuiTableComposition"/>.
			/// </summary>
			class GuiRowSplitterComposition : public GuiTableSplitterCompositionBase, public Description<GuiRowSplitterComposition>
			{
			protected:
				vint								rowsToTheTop = 0;
				
				void								OnMouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments);
				Rect								Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiRowSplitterComposition();
				~GuiRowSplitterComposition() = default;

				/// <summary>Get the number of rows that above the splitter.</summary>
				/// <returns>The number of rows that above the splitter.</returns>
				vint								GetRowsToTheTop();
				/// <summary>Set the number of rows that above the splitter.</summary>
				/// <param name="value">The number of rows that above the splitter</param>
				void								SetRowsToTheTop(vint value);
			};
			
			/// <summary>
			/// Represents a column splitter composition of a <see cref="GuiTableComposition"/>.
			/// </summary>
			class GuiColumnSplitterComposition : public GuiTableSplitterCompositionBase, public Description<GuiColumnSplitterComposition>
			{
			protected:
				vint								columnsToTheLeft = 0;
				
				void								OnMouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments);
				Rect								Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiColumnSplitterComposition();
				~GuiColumnSplitterComposition() = default;

				/// <summary>Get the number of columns that before the splitter.</summary>
				/// <returns>The number of columns that before the splitter.</returns>
				vint								GetColumnsToTheLeft();
				/// <summary>Set the number of columns that before the splitter.</summary>
				/// <param name="value">The number of columns that before the splitter</param>
				void								SetColumnsToTheLeft(vint value);
			};
		}
	}
}

#endif