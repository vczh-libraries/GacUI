# \<RepeatFixedHeightItem\>

This composition render items in a default order from top to bottom. All items have the same height, which is defined by the largest value in all visible items. All items are stretched to the width of this composition by default.

## ItemWidth

The default value of this property is **-1**. When it is set to any non-negative number, it defines the width of all items.

One of an example is the **Detail** view in **\<ListView\>**. When the size of the column bar is changed, item widths are changed.

## ItemYOffset

The default value of this property is **0**. When it is set to any positive number, it addes some space before the first item.

One of an example is the **Detail** view in **\<ListView\>**. When the list is scrolled to the top, the first item is just below the column bar.

