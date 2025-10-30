# Virtual Repeat Compositions

**GuiVirtualRepeatCompositionBase** maintain instances of item template automatically to render items in the item source.

## Axis property

All repeat compositions maintain items in a default order. To change the order, set a new axis to the **Axis** property. **Axis** in **GuiVirtualRepeatCompositionBase** is very similar to [Axis in <Flow/>](../../.././gacui/components/compositions/flow.md).

## UseMinimumTotalSize property

The default value is **false**.

When **UseMinimumTotalSize** is **true**, **TotalSize** returns a smallest size but large enough to make scrolling right.

For example, in **\<RepeatFixedHeightItem/\>**, The width of **TotalSize** becomes **0** when **UseMinimumTotalSize** is **true** and **ItemWidth** is **-1**. Because under such configuration, widths of items always equal to the width of **\<RepeatFixedHeightItem/\>** itself, it provides no information to control the scrolling, and it is very useful to make the horizontal scroll disappear forever.

But the height of **TotalSize** is not affected, the value is necessary to control the scrolling.

**NOTE**: Scroll bars are not part of this composition, instead the following properties could be combined to configure scroll bars: - TotalSize - ViewLocation - Size of CachedBounds

## TotalSize property

**TotalSize** property measures the area occupied by all items, regardless visible and non visible.

## ViewLocation property

**ViewLocation** defines the left-top corner of the visible area in **TotalSize**. The size of the visible area is the size of this composition. Items that could render in this visible area becomes visible, otherwise invisible.

## Accessing auto-managed instances of item template

**GuiVirtualRepeatCompositionBase** only creates instances of item template for only visible items. Visible items mean items that are in the area of this composition, all items that are scrolled out will get their instance of item template released from the memory, and recreate when they are scrolled in again.

When an item is not visible, it is not able to find the instance of item template for this item.

If a **system::ObservableList^** is assigned to **ItemSource**, the items and the order will be kept throught out the running time. Otherwise these compositions only reflect items in the collection at the moment of assigning to **ItemSource**.

The **GetVisibleStyle** function gives you the instance of item template for an item. If the item is not visible, it returns **nullptr**.

The **GetVisibleIndex** function gives you the index of an item from an instance of item template. If it doesn't belong to this composition, it returns **-1**.

The **FindItemByRealKeyDirection** and **FindItemByVirtualKeyDirection** function find an item that is related to the specified item using a key. **VirtualKey** uses the logical order of the composition. **RealKey** uses the render order of the composition. When **Axis** is unchanged or set using an instance of **GuiDefaultAxis**, they are identical.

The **EnsureItemVisible** function tries its best to update **ViewLocation** to make the specified item visible.

When **null** is assigned to **ItemSource**, all items will be deleted.

