# \<RepeatStack\> and \<RepeatFlow\>

**\<RepeatStack/\>** and **\<RepeatFlow/\>** are **\<Stack/\>** and **\<Flow/\>** with data binding.

You are not required to create **\<StackItem/\>** or **\<FlowItem/\>** for each item. Instead, you bind a collection object to the **ItemSource** property, assign an item template to the **ItemTemplate** property, **\<RepeatStack/\>** or **\<RepeatFlow/\>** will create **\<StackItem/\>** or **\<FlowItem/\>** for each item in **ItemSource**, each containing an instance from **ItemTemplate** to display an item in **ItemSource**.

**\<RepeatStack/\>** inherits from **\<Stack/\>**, **\<RepeatFlow/\>** inherits from **\<Flow/\>**, all properties in **\<Stack/\>** or **\<Flow/\>** are available.

## Accessing auto-managed \<StackItem/\> or \<FlowItem/\>

**\<RepeatStack/\>::StackItems** property or **\<RepeatFlow/\>::FlowItems** property contains the same number of **\<StackItem/\>** or **\<FlowItem/\>** to items in **ItemSource** in exactly the same order.

If a **system::ObservableList^** is assigned to **ItemSource**, the items and the order will be kept throught out the running time. Otherwise these compositions only reflect items in the collection at the moment of assigning to **ItemSource**.

Each **\<StackItem/\>** or **\<FlowItem/\>** has an only child composition which is an selected instance from **ItemTemplate**.

To access the item template instance of a specified item in **ItemSource**, just use **StackItems** or **FlowItems** with the same index, and find the first direct child composition of the returned **\<StackItem/\>** or **\<FlowItem/\>**.

To access **\<StackItem/\>** or **\<FlowItem/\>** inside the item template instance, just find its direct parent composition.

When **null** is assigned to **ItemSource**, all items will be deleted.

