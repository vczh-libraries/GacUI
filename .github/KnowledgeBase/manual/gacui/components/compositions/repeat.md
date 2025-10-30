# Repeat Compositions

**GuiRepeatCompositionBase** is the base class of all repeat compositions, which takes an item template and an item source, displays items in the item source using the item template, and maintain the order automatically when items in the item source are add/remove/change.

There are two kinds of repeat compositions: - **GuiNonVirtialRepeatCompositionBase**: Create and maintain instances of item template for all items. - [<RepeatStack/>](../../.././gacui/components/compositions/repeat_nonvirtual.md) - [<RepeatFlow/>](../../.././gacui/components/compositions/repeat_nonvirtual.md) - **GuiVirtialRepeatCompositionBase**: Create and maintain instances of item template for only visible items. Visible items mean items that are in the area of this composition, all items that are scrolled out will get their instance of item template released from the memory, and recreate when they are scrolled in again. - [<RepeatFreeHeightItem/>](../../.././gacui/components/compositions/repeat_virtual_freeheight.md) - [<RepeatFixedHeightItem/>](../../.././gacui/components/compositions/repeat_virtual_fixedheight.md) - [<RepeatFixedHeightMultiColumnItem/>](../../.././gacui/components/compositions/repeat_virtual_fixedheightmc.md) - [<RepeatFixedSizeMultiColumnItem/>](../../.././gacui/components/compositions/repeat_virtual_fixedsizemc.md)

## ItemSource property

A valid object to assign to the **ItemSource** should be a **system::Enumerable^**. Here are built-in interfaces that inherits from **system::Enumerable**: - system::ReadonlyList - system::List - system::ObservableList

In this page`missing document: /home/registered/vlppreflection.html` you could find C++ class names for these interfaces.

In [this page](../../.././workflow/lang/type.md) you could find **Workflow** syntax for these types.

Values in the collection must match **\<ref.Parameter/\>** of the [instance](../../.././gacui/xmlres/tag_instance.md) that is specified in the **ItemTemplate** property.

If there are [multiple instances](../../.././gacui/xmlres/instance/properties.md) in the **ItemTemplate** property, the first one whose **\<ref.Parameter/\>** is able to accept the value will be selected for the item in the collection. If none of them can, an exception will raise.

To create a **system::Enumerable^** in C++, either call **vl::reflection::description::IValueEnumerable::Create**, or call **vl::reflection::description::BoxParameter** on an **vl::collections::IEnumerable\<T\>&**.

To create a **system::List^** in C++, either call **vl::reflection::description::IValueList::Create**, or call **vl::reflection::description::BoxParameter** on an **vl::collections::IList\<T\>&**.

The **BoxParameter** function creates a reference to the C++ collection, the C++ collection and the created object share items in the collection, and one can observe changes from another.

## Binding system::ObservableList to ItemSource property

If an **ObservableList** is assigned to the **ItemSource** property, when items in the list changed, associated instance of item templates will be automatically created or destroyed.

To create a **system::ObservableList^** in C++, either call **vl::reflection::description::IValueObservableList::Create**, or use **vl::collections::ObservableList\<T\>** as your collection class and call the **BoxParameter** function on it.

The **BoxParameter** function creates a reference to the C++ collection, the C++ collection and the created object share items in the collection, and one can observe changes from another.

## ItemTemplate property

The [ItemTemplate](../../.././gacui/xmlres/instance/properties.md) property accepts one or a list of [instances](../../.././gacui/xmlres/tag_instance.md) in their full names or XML names.

Instances here are required to have exactly one **\<ref.Parameter/\>** as the only argument in their constructor.

When **ItemSource** is being assigned, or when **ItemSource** is a **system::ObservableList^** and new items are inserted to the list, instances in **ItemTemplate** will be tested one by one to see if its constructor argument could accept the item in the list.

The first instance of a successful test will be created with the item in the list, with **MinSizeLimitation** set to **LimitToElementAndChildren** and **Margin** set to **left:0 top:0 right:0 bottom:0**.

## Context property

**Context** property of any living **ItemTemplate** instance will always sync to **Context** property of the repeat composition.

**\<RepeatStack/\>** and **\<RepeatFlow/\>** are **\<Stack/\>** and **\<Flow/\>** with data binding.

## Sample

Please check out [ the demo for <RepeatStack/> and <RepeatFlow/>](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/RepeatTabPage.xml) and its [ sample item templates ](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/RepeatComponents.xml) .

