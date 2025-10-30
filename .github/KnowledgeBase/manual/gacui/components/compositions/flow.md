# \<Flow\> and \<FlowItem\>

**\<Flow/\>** arrange all direct children **\<FlowItem/\>** compositions in multiple rows with auto line wrapping.

**\<FlowItem/\>**::MinSizeLimitation is **LimitToElementAndChildren** by default.

## Properties

A few more properties are provided by **\<Flow/\>** and **\<FlowItem/\>** to control the details of how to ordering **\<FlowItem/\>**.

**\<FlowItem/\>** is not a **\<Bounds/\>**, there is no writable **ExpectedBounds** and **AlignmentToParent** in **\<FlowItem/\>**.

### \<Flow\>::Axis

The default value is **\<DefaultAxis/\>**, which equivalents to **\<Axis AxisDirection="RightDown"/\>**.

**LeftDown**, **RightDown**, **LeftUp**, **RightUp**, **DownLeft**, **DownRight**, **UpLeft** and **UpRight** are all valid values for the **AxisDirection** property.

Among these values, the first word describes the direction how **\<FlowItem/\>** are line up one by one, the second word describes the direction how auto line wrapping does. For example, **RightDown** means the next item is put on the **right** side of the previous item, and when there is no more space for this item in the current row, it creates a new row on the **down** side of the current row.

### \<Flow\>::Alignment

The default value is **Left**.

**Left**, **Center**, **Right** and **Extend** are all valid values for the this property.

This property describes how **\<FlowItem/\>** are positioned in one row.

### \<Flow\>::RowPadding and \<Flow\>::ColumnPadding

The default value is 0.

This property keeps an distance between each **\<FlowItem/\>**.

**IMPORTANT**: **Row** here doesn't necessary mean a line in horizontal direction, it depends on the value of the **Axis** property.

### \<Flow\>::ExtraMargin

The default value is 0 for all its components.

This property keeps an distance between **\<Flow/\>** and **\<FlowItem/\>**.

### \<FlowItem\>::ExtraMargin

The default value is 0 for all its components.

This property adds a border to a **\<FlowItem/\>**.

**IMPORTANT**: **ExtraMargin** does not affect how **other \<FlowItem/\>** is positioned. Instead, after the position of a **\<FlowItem/\>** is decided, **ExtraMargin** kicks in and add a border to enlarge it.

Adding an **ExtraMargin** to a **\<FlowItem/\>** does not increase the minimum size of its parent **\<Flow/\>**.

### \<FlowItem\>::FlowOption

The default value is **baseline:FromBottom distance:0**.

This property describes how **\<FlowItem/\>** are positioned in one row.

**baseline** could be **FromTop**, **FromBottom** or **Percentage**: - **FromTop**: the **\<FlowItem/\>** keeps the **distance** from the top of the row. - **FromBottom**: the **\<FlowItem/\>** keeps the **distance** from the bottom of the row. - **Percentage**: the **\<FlowItem/\>** keeps the distahce, which is **pencentage** of its height, from the top of the row.

**IMPORTANT**: **Top** here doesn't necessary mean the top border of a row, it depends on the value of the **Axis** property.

## Adding Flow Items

### \<Flow\>::Children()

When a new **\<FlowItem/\>** is added to **\<Flow/\>** as a child, this **\<FlowItem/\>** is always appended to the end of the last row, regardless of its position in **\<Flow\>::Children**.

### \<Flow\>::InsertFlowItem(index, item)

To control the position of a **\<FlowItem/\>** in the auto-wrapped line, call **\<Flow\>::InsertFlowItem()** instead of **\<Flow\>::Children()**.

This function also adds a **\<FlowItem/\>** to the **\<Flow/\>**, but it allows the position of this **\<FlowItem/\>** in the auto-wrapped line to be specified, instead of adding it to the end of the last row.

### \<Flow\>::GetFlowItems()

Call this function to get all direct children **\<FlowItem/\>** in the line order.

## Sample

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Layout/Flow) .

