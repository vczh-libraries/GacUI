# Bindable List Controls

Instead of using a certain fixed type for items in a certain list control, bindable list controls allow items to be any class or interface type.

## ItemSource property

All bindable list controls have a**ItemSource**property. This property could be any collection type. But if it is an**observe T[]**, or**Ptr\<IValueObservableList\>**in C++, the bindable list control will keep itself always sync to the collection.

Creating a**Ptr\<IValueObservableList\>**in C++ is easy. Use**vl::collections::ObservableList\<T\>**instead of**vl::collections::List\<T\>**, you can get a**Ptr\<IValueObservableList\>**by calling the**BoxParameter**function on that list.

## Property and Writable Property

After assigning a collection to a bindable list control, you also needs to teach the control how to read/write your item object.

For the most common cases, if a property in items in**ItemSource**is exactly what you need, for example a**Name**property returning**string**in**YOUR_ITEM_CLASS**will be displayed directly in a**\<BindableTextList/\>**, just write the property name in the XML for**TextProperty**like:
```
<BindableTextList env.ItemType="YOUR_ITEM_CLASS^" ...>
  <att.TextProperty>Name</att.TextProperty>
  <att.ItemSource-eval>COLLECTION_CONTAINING_MANY_YOUR_ITEM_CLASS</att.ItemSource-eval>
  ...
</BindableTextList>
```


By doing this,**\<BindableTextList/\>**will read the**Name**property in all**YOUR_CLASS_CLASS**in its**ItemSource**, and shows all names in the list control.**IMPORTANT**: If there are items in**ItemSource**that is not**YOUR_ITEM_CLASS**, which is specified in**env.ItemType**, it will crash.

**env.ItemType**and**env.ItemName**can be put in any tag inside**\<Instance/\>**. These attributes only control how to interpret the string for**Property and Writable Property**. In this case they control what does**Name**mean for**TextProperty**:

assume all items in**ItemSource**is or inherits from**YOUR_ITEM_CLASS**,
- call**(case (YOUR_ITEM_CLASS^) item).GetName()**if**Name**is a property.
- call**(case (YOUR_ITEM_CLASS^) item).Name**if**Name**is a field.

Different bindable list controls have different requirements. For example,**\<BindableTextList/\>**has**TextProperty**and**CheckedProperty**properties.**TextProperty**is a[readable property](../../../.././gacui/xmlres/instance/properties.md), which means the control will read a text from an item.**CheckedProperty**is a[writable property](../../../.././gacui/xmlres/instance/properties.md), which means the control will not only read a checked mark from an item, it will also set a checked mark to an item.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml)for details about using**TextProperty**.

## Updating Items

If**ItemSource**is an observable collection, when you add an item to the collection, or remove an item from the collection, or replace an item in the collection, the list control will know and show the latest result. But changing properties in items will not trigger refreshing the list control, because**Property and Write Property are not data binding**.

When data in an item is modified, there are 3 ways to notify the list control to recreate the instance of the item template associated with these items:
- When using**vl::collections::ObservableList\<T\>**, call**NotifyUpdate**to notify the list control. This method accepts a range of indices, all items in that range will be treated like they are changed.
- Trigger the**IValueObservableList::ItemChanged**event to notify the list control. When data in items are modified, it means there is no adding to or removing from the collection, so always keep the second and the third argument having the same value. This event accepts a range of indices, all items in that range will be treated like they are changed.
- When using**vl::collections::ObservableList\<T\>**, or when the observable collection is created using**Workflow**script, replacing an item with itself also notify the list control.There are another way to notify the list control to updated the item instead of recreating it:
- Call**NotifyItemDataModified**of**\<BindableTextList/\>**or**\<BindableListView/\>**.
- Call**NotifyNodeDataModified**of**\<BindableTreeView/\>**.

## ItemTemplate Property

Item objects in**ItemSource**will be used to create instances from**ItemTemplate**. Some properties of the item template will be assigned to values that are read from the item object using**Property and Writable Property**. But since the item template already knows the item object, it could read from the object directly.

