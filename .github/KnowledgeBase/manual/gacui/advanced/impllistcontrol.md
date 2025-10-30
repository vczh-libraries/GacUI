# Creating New List Controls

Customizing **GuiTextList** or **GuiBindableTextList** by providing new item templates , should be the most common way to create a new list control experience. But you could still create your new list control.

In GacUI 1.0, any new classes created completely by C++ (instead of by **\<Instance/\>** in GacUI XML Resource) is not accessible in the GacUI XML Resource, because the compiler doesn't know about that.

Any new list control is expected to derives from **GuiSelectableListControl** directly. If list items are not selectable, then **GuiListControl** could be used to reduce runtime overhead.

## list::IItemProvider, the constructor argument

An instance of **IItemProvider** is required in the constructor of **GuiListControl** and **GuiSelectableListControl**. The base class doesn't control the life cycle of this instance, it should be properly disposed in the derived class.

This interface offers a bridge so that **GuiListControl** could access its items. - **AttachCallback**: Multiple **IItemProviderCallback** could be installed to an **IItemProvider**. When a callback is properly installed, its **OnAttached(this)** must be called immediately. - **DetachCallback**: This method is called to remove an installed **IItemProviderCallback**. When a callback is properly uninstalled, its **OnAttached(nullptr)** must be called immediately. - **PushEditing**: This method is called when the list control enters another level of editing mode. - **PopEditing**: This method is called when the list control leaves one level of editing mode. - **IsEditing**: This method should return **true** if at least one level of editing mode is still in the list control. - **Count**: This method should return the total number of items in the list control. In **GuiTreeView**, all collapsed items are not counted. Although they exist in the logical tree, but if a node is collapsed, all sub items will be treated as removed by calling **IItemProviderCallback::OnItemModified**. And when a node is expanded, all sub items come back in the same way. - **GetTextValue**: This method should return a text representation for a given item. The value will be copied to the **Text** property of a item template. - **GetBindingValue**: This method should return a **non-null** object for a given item, if data binding for item source is available in this control. For non-bindable list controls, a **null** object should be returned by calling the constructor of the return value with no argument, for example, **{}** in C++. - **RequestView**: This method provides a way to decouple between the list control and the details of items. **IItemProvider** should handle the life cycle of all returned views.

**list::ItemProviderBase** could be used to create an **IItemProvider**. The following methods are implemented in this class: - **AttachCallback** - **DetachCallback** - **PushEditing** - **PopEditing** - **IsEditing** You need to implement the rest to complete an **IItemProvider**. The **InvokeOnItemModified** method is provider to call **OnItemModified** in all installed callbacks.

## list::IItemProviderCallback, the constructor argument

The **OnItemModified** allow **IItemProvider** to report one change of consecutive items: - **start**: The index of the first item in all changing consecutive items. - **count**: The numbers of all changing consecutive items. - **newCount**: The numbers of new items that all changing consecutive items change into.

If changes happens in multiple groups of consecutive items, each group should be reported in separate call to **OnItemModified**. You must try your best to ensure that, **IItemProvider::Count**, **IItemProvider::GetTextValue**, **IItemProvider::GetBindingValue** and all related methods in views from **RequestView** should not see any unreported changes.

Call **OnItemModified(start, 0, count)** when a group of items are inserted to the position **start**. If one item is inserted, **count** should be **1**.

Call **OnItemModified(start, count, 0)** when a group of items are removed from the position **start**. If one item is removed, **count** should be **1**.

Call **OnItemModified(start, count, count)** when a group of items are updated beginning from the position **start**. If one item is updated, **count** should be **1**. An item is updated, if the item is replaced, or any interested content inside this item is updated.

## GuiListControl::IItemArranger, the item layouting

**IItemArranger** handles rendering of list items. An **IItemArranger** is attached to a list control using its **Arranger** property or its protected method **SetStyleAndArranger**.

There are already some [ built-in IItemArranger implementations ](../.././gacui/components/controls/list/guilistcontrol.md). Check them out to see if any of the built-in things work, so you don't have to implement this interface.

It is recommended to inherited from **[GuiVirtualRepeatCompositionBase](../.././gacui/components/compositions/repeat_virtual.md)** when you have to create a new layout. When such new composition is ready, use **VirtualRepeatRangedItemArrangerBase\<T\>** to turn it into an **IItemArranger**.

**IItemProviderCallback** is also the base class of **IItemArranger**. When an **IItemArranger** is attached to a list control, it will be installed as a callback to **IItemProvider**. So that **IItemArranger** could update the rendering when any item is changed.

A **IItemArranger** should be aware of the [ DisplayItemBackground ](../.././gacui/components/controls/list/guilistcontrol.md) property of a list control. If this property returns **true**, all item template instances returned from **IItemArrangerCallback::ReleaseItem** should be contained in a **GuiSelectableButton** as the item background using - The **ThemeNameLLListItemBackground** as the theme of this button. - **listControl-\>TypedControlTemplateObject(true)-\>GetBackgroundTemplate()** as the control template of this button.

All locations, sizes and margins involved in this interface are all **virtual**. **IItemArranger** just need to provide item layout in the **LeftDown** direction. The [Axis](../.././gacui/components/controls/list/guilistcontrol.md) property will be used by the list control to adjust locations and sizes of all visible items to layout them in the expected direction.

Here are all methods to implemement: - `empty list item` **AttachListControl**: This method is called when it is attached to a list control. - `empty list item` **DetachListControl**: This method is called when it is detached from a list control. - `empty list item` **GetCallback**: Get the argument from the last **SetCallback** call. - `empty list item` **SetCallback**: This method is called when it is attached to a list control. Only one callback will be installed to **IItemArranger**. This callback provides everything that an **IItemArranger** could do the a list control. - `empty list item` **GetTotalSize**: Return the total size of all items in their layout. - `empty list item` **GetVisibleStyle**: This method should return the item template instance for a give item. If the item is invisible because it is scrolled out of the visible area of the list control, it should return **nullptr**. Living item template instances for items could be managed using **IItemArrangerCallback::RequestItem** and **IItemArrangerCallback::ReleaseItem**. - `empty list item` **GetVisibleIndex**: This method should return the item index for a given item template instance. If such an instance is unknown to this **IItemArranger**, it should return **-1**. - `empty list item` **ReloadVisibleStyles**: Recreate all UI objects for visible items. Items don't need to be recreated immediately, but all UI objects for items should be disposed in the call to this method. **GetVisibleStyle** and **GetVisibleIndex** could pretend that all items are invisible until the next call to **OnViewChanged**. - `empty list item` **OnViewChanged**: This method is called to tell the **IItemArranger** what is the visible part in **GetTotalSize**. **IItemArranger** could only create UI objects for items in this area, to reduce the memory pressure and improve the performance. **IItemArranger** could also create UI objects for all items if performance is not important. All items with their UI objects created are **visible items**. Living item template instances for items could be managed using **IItemArrangerCallback::RequestItem** and **IItemArrangerCallback::ReleaseItem**. - `empty list item` **FindItem**: Search a next item from the specified item **itemIndex** using a keyboard operation **key**. **-1** could be returned if no reasonable item is definded as **the next item**. - **EnsureItemVisible**: This method is called if the list control want to scroll to an area to make a given item visible. **IItemArrangerCallback::SetViewLocation** is usually called as a respond. **OnViewChanged** could be called in **SetViewLocation**. - **GetAdoptedSize**: Return a reasonable size for the visible area of items, if the list control is used as a combo box dropdown list.

## GuiListControl::IItemArrangerCallback, the item layouting

All locations, sizes and margins involved in this interface are all **virtual**. **IItemArranger** just need to provide item layout in the **LeftDown** direction. The [Axis](../.././gacui/components/controls/list/guilistcontrol.md) property will be used by the list control to adjust locations and sizes of all visible items to layout them in the expected direction.

- **RequestItem**: Ask the list control to create an item template instance for a specific item. An item template instance that is released by calling **ReleaseItem** could be reused for a different item. - **ReleaseItem**: Ask the list control to dispose an item template instance for a specific item. The item template instance will be deleted from the memory later if necessary, the pointer should not be used anymore after calling this method. - **SetViewLocation**: Tell the list control what should be the left-top corner of the visible area. But the list control could move the visible area to a different location closed by if it thinks this is better. - **GetStylePreferredSize**: Read the size of the **CachedMinSize** property of a compositon contained by list control. **IMPORTANT**: the result is **virtual**. Do not use this property directly, use this method instead. - **SetStyleAlignmentToParent**: Update the **AlignmentToParent** property of a composition contained by list control. **IMPORTANT**: the result is **virtual**. Do not use this property directly, use this method instead. - **GetStyleBounds**: Read the **Bounds** property of a composition contained by list control. **IMPORTANT**: the result is **virtual**. Do not use this property directly, use this method instead. - **SetStyleBounds**: Update the **Bounds** property of a composition contained by list control. **IMPORTANT**: the result is **virtual**. Do not use this property directly, use this method instead. - **GetContainerComposition**: Get the composition as a container to store all UI objects for items. - **OnTotalSizeChanged**: Call this method to tell the list control that **IItemArranger::GetTotalSize** has been changed.

## Using IItemProvider::RequestView Properly

A list control could override **OnStyleInstalled** from **GuiListControl**. This method is called when an item template instance is binded to a list item.

A new list control class typlically defines a new item template class for an item. the new item template class adds new properties to define how an item for this new list control should look like.

One or more views could be defined to read or write details for an item for this particular list control class. Then the list control class could call **RequestView** for a view using a unique identifier string, to bind data from the view to item template instances.

For example: **GuiTextList** defines the **ITextItemView** view to add the **Checked** property to items.

**IItemArranger** could also defines its own views. For example, **ListViewColumnItemArranger** defines **IColumnItemView** to read columns from the list control to render column headers. Any list control using **ListViewColumnItemArranger** should make sure that **IColumnItemView** is implemented and accessible from **IItemProvider::RequestView**.

You can define your own view for any purpose, as long as it is only used to read or write details of for items.

## Data Binding on Items

A data bindable list control could read or write items from a item source implementing **Ptr\<IValueEnumerable\>**. This object could also implement any derived interfaces like: - **IValueReadonlyList** - **IValueList** - **IValueObservableList** If the item source implements **IValueObservableList**, then the list control is expected to update automatically when items are changed.

[ The readable property type ](../.././gacui/xmlres/instance/properties.md) and [ the writable property type ](../.././gacui/xmlres/instance/properties.md) could be used to let the user tell you how to access details from items. **vl::presentation::controls::ReadProperty** and **vl::presentation::controls::WriteProperty** could be used on these property types in C++.

## Sample

You are strongly recommended to read the source code of **GuiTextList** and **GuiBindableTextList** before creating your own list control classes.

