# \<Table\>, \<Cell\>, \<RowSplitter\> and \<ColumnSplitter\>

**\<Table/\>** arrange all direct children **\<Cell/\>** compositions in a table with size configurations.

**\<Cell/\>**::MinSizeLimitation is **LimitToElementAndChildren** by default.

## Properties

A few more properties are provided by **\<Table/\>** and **\<Cell/\>** to control the details of how to ordering **\<Cell/\>**.

**\<Cell/\>** is not a **\<Bounds/\>**, there is no writable **ExpectedBounds** and **AlignmentToParent** in **\<Cell/\>**.

### \<Table\>::Rows and \<Table\>::Columns

They are collection properties that can only be used in GacUI XML Resource and Workflow. In order to access them in C++, please check out the following methods: - GetRows - GetRows - SetRowsAndColumns - GetRowOption - SetRowOption - GetColumnOption - SetColumnOption Call **SetRowsColumns** to allocate the numbers of rows and columns for this table, and then call **SetRowOption** and **SetColumnOption** to set the size configuration.

In GacUI XML Resource, in order to have a 5x3 table, it is expected to have 5 values in **Rows** and 3 values in **Columns**.

Values for these properties could be: - **composeType:MinSize**: the size of the cell is the minimum size for this composition. - **composeType:Absolute absolute:X**: the size of the cell cannot be smaller than X. X must be a non-negative integer. - **composeType:Percentage percentage:X**: the size of the cell is decided by all **Percentage** cells. X must be a non-negative number.

Before repositioning cells, the table will collect all rows and cells that are **MinSize** and **Absolute**, spaces are first allocate to these rows and cells. And then if there are still spaces, spaces will be allocate to **Percentage** cells in proportion.

If **\<Table/\>**::MinSizeLimitation is set to **LimitToElementAndChildren**, the minimum size of the table is the sum of all cells' minimum sizes with paddings. For **Absolute** cells, if the absolute value is larger than its minimum size, the absolute value becomes the minimum size for this cell.

The sum of all percentage values could be anything. for example, if a table has a 0 padding with these 5 row options: - **composeType:Percentage percentage:0.5** - **composeType:MinSize** - **composeType:Percentage percentage:1.0** - **composeType:Absolute absolute:200** - **composeType:Percentage percentage:0.5** and the minimum width for each cell is 100.

It is easy to know that the minimum width of this table is 500. If the table has a border and a cell padding of 10, its minimum width becomes 560.

In the space of 500: - 100 will be allocated to the 2nd cell - 200 will be allocated to the 4nd cell and there is still 200 left.

The sum of all percentage values is 0.5 + 1.0 + 0.5 = 2. - The 1st cell will be allocated 50 (200 * 0.5 / 2) - The 3rd cell will be allocated 100 (200 * 1.0 / 2) - The 5th cell will be allocated 50 (200 * 0.5 / 2)

### \<Table\>::CellPadding

The default value is 0.

**CellPadding** keeps an distance between each **\<Cell/\>**.

### \<Table\>::BorderVisible

The default value  is **true**.

If **BorderVisible** is true, cells are keep the distance of **CellPadding** to the border of the table, otherwise the distance is 0.

### \<Cell\>::Site

This is a required property in GacUI XML Resource.

This property can only be used in GacUI XML Resource and Workflow. In order to access them in C++, please check out the following methods: - GetRow - GetRowSpan - GetColumn - GetColumnSpan - SetSite**SetSite** requires 4 arguments, which are exactly the 4 components in this **Site** property.

Value for this property is **row:R column:C rowSpan:RS columnSpan:CS**. The default values for **rowSpan** and **columnSpan** are 1. - **row**: specify the row where this cell locates starting from 0. - **column**: specify the column where this cell locates starting from 0. - **rowSpan**: specify how many rows does this cell take. - **columnSpan**: specify how many columns does this cell take.

A table is split to multiple cell blocks, a cell could take multiple cell blocks at once, but they must form a rectangle space. A cell block can only be taken by one cell.

**\<Cell/\>** are processed according to their order in GacUI XML Resource. If a cell is found to be taking cell blocks that have already been taken, the **Site** property of this cell is canceled, the position of this cell is undefined.

## Adding Cells in C++

To create a table in C++, these steps must be taken in order: - Call **SetRowsAndColumns** of the table to allocate cell blocks. - Call **SetRowOption** and **SetColumnOption** of the table for size configuration. - Call **SetSite** for cells to define which cell blocks are taken by each cell. - Add cells as a direct child composition of the table. - Call **UpdateCellBounds** of the table to tell that the configuration is ready.

When contents of cells are changed, no action is required.

When size configurations are changed, or when cells are added or removed to the table, **UpdateCellBounds** must be called to tell the table to reorganize cells.

## Adjusting Cells by Mouse

### \<RowSplitter\>

A **\<RowSplitter/\>** takes the space between two rows. This requires the table to have a non-zero **CellPadding**.

One of a sibling row of the splitter is required to be **Absolute**.

**RowsToTheTop** specifies how many rows are above this splitter. If the table has 5 rows, valid values for **RowsToTheTop** are 1 to 4.

After adding a splitter to the table, it can be dragged by a mouse to adjust the size of an **Absolute** row around the splitter.

### \<ColumnSplitter\>

A **\<ColumnSplitter/\>** takes the space between two columns. This requires the table to have a non-zero **CellPadding**.

One of a sibling column of the splitter is required to be **Absolute**.

**ColumnsToTheLeft** specifies how many columns are above this splitter. If the table has 5 columns, valid values for **ColumnsToTheLeft** are 1 to 4.

After adding a splitter to the table, it can be dragged by a mouse to adjust the size of an **Absolute** column around the splitter.

## Sample

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Layout/Table) .

