# \<Stack\> and \<StackItem\>

**\<Stack/\>**arrange all direct children**\<StackItem/\>**compositions in a line following the same direction.

If all**\<StackItem/\>**lines of horizontally, all of them share the same height. If all**\<StackItem/\>**lines of vertically, all of them share the same width.

**\<StackItem/\>**::MinSizeLimitation is**LimitToElementAndChildren**by default.

## Properties

A few more properties are provided by**\<Stack/\>**and**\<StackItem/\>**to control the details of how to ordering**\<StackItem/\>**.

**\<StackItem/\>**is not a**\<Bounds/\>**, there is no writable**ExpectedBounds**and**AlignmentToParent**in**\<StackItem/\>**.

### \<Stack\>::Direction

The default value is**Horizontal**.

**Horizontal**,**ReversedHorizontal**,**Vertical**and**ReversedVertical**are all valid values for this property, deciding how**\<StackItem/\>**are line up in a**\<Stack/\>**.

### \<Stack\>::Padding

The default value is 0.

This property keeps an distance between each**\<StackItem/\>**.

### \<Stack\>::ExtraMargin

The default value is 0 for all its components.

This property keeps an distance between**\<Stack/\>**and**\<StackItem/\>**.

### \<StackItem\>::ExtraMargin

The default value is 0 for all its components.

This property adds a border to a**\<StackItem/\>**.

**IMPORTANT**:**ExtraMargin**does not affect how**other \<StackItem/\>**is positioned. Instead, after the position of a**\<StackItem/\>**is decided,**ExtraMargin**kicks in and add a border to enlarge it.

Adding an**ExtraMargin**to a**\<StackItem/\>**does not increase the minimum size of its parent**\<Stack/\>**.

## Adding Stack Items

### \<Stack\>::Children()

When a new**\<StackItem/\>**is added to**\<Stack/\>**as a child, this**\<StackItem/\>**is always appended to the end of the line, regardless of its position in**\<Stack\>::Children**.

### \<Stack\>::InsertStackItem(index, item)

To control the position of a**\<StackItem/\>**in a line, call**\<Stack\>::InsertStackItem()**instead of**\<Stack\>::Children()**.

This function also adds a**\<StackItem/\>**to the**\<Stack/\>**, but it allows the position of this**\<StackItem/\>**in a line to be specified, instead of adding it to the end of the line.

### \<Stack\>::GetStackItems()

Call this function to get all direct children**\<StackItem/\>**in the line order.

## Visibility of Stack Items

### \<Stack\>::IsStackItemClipped()

This function returns**false**when any part of any**\<StackItem/\>**is invisible or clipped by this**\<Stack/\>**.

### \<Stack\>::EnsureVisible(index)

When**MinSizeLimitation**of**\<Stack/\>**is not**LimitToElementAndChildren**, it is possible that some**\<StackItem/\>**are not visible because of the**\<Stack/\>**is too small.

This function**"scrolls"**all items to make sure that the specified one is visible.

## Sample

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Layout/Stack).

