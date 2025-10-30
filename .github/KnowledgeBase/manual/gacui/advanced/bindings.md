# Data Bindings

Data bindings in GacUI relies on [Workflow expression](../.././workflow/lang/bind.md). Basically, an expression is observable if it uses properties, and these properties must be observable by having their property changing events on the object. You are still able to combine some events on a property temporary just for this expression to make it observable.

In GacUI XML Resource, [-bind](../.././gacui/xmlres/instance/properties.md), [-format](../.././gacui/xmlres/instance/properties.md) and [-str](../.././gacui/xmlres/instance/properties.md) are based on data bindings.

## Binding to Control Properties

This is the most basic kind of data binding.

If you are binding something to a text property which will be displayed on the UI, e.g. **Text** of a **\<Label/\>**, make sure that: - The layout of controls is properly handled, so that they will re-arrange themselves to make sure that no truncation of text rendering will happen. - If the application could be send to people using different language, please consider using **Localization** from the very beginning.

Binding view models to control is also very straight-forward. A **\<ref.Paramter Name="Something" Class="..."/\>** creates a property that will never change once the window is created. The **X** property of **Something** can be binded to a control property just by: ``` <AControl AProperty-bind="Something.X"/> ```

It is also very simple to use data binding with your own objects like: ``` <Instance ref.Class="..."> <ref.Members><![CDATA[ var myObject = new MyObject(); ]]></ref.Members> <Window ref.Name="self"> <AControl AProperty-bind="self.myObject.X"/> </Window> </Instance> ``` Since **myObject** is a member of the window, you must first give the window a name **self**, to make XML knows what is **myObject**.

## Binding between Composition Properties

**JUST DON'T DO THAT.**

The built-in layout algorithm already make compositions bidirectionally react to each other. Binding one property in a composition to another property in (maybe) another composition could cause the reaction never end.

Here is a very bad example: ``` <Bounds ref.Name="parent" MinSizeLimitation="LimitToElementAndChildren"> <Bounds ref.Name="child" PreferredMinSize-bind="{x:(parent.Bounds.x2 - parent.Bounds.x1} y:0}" AlignmentToParent="{left:1 top:1 right:1 bottom:1}" /> </Bounds> ```

When **parent.Bounds** is changed, the observed expression is notified to re-evaluate, and than cause **child.PreferredMinSize** to change.

**child.PreferredMinSize** causes **parent.Bounds** to re-evaluate because **MinSizeLimitation="LimitToElementAndChildren"**.

Now parent grows bigger because **child.AlignmentToParent** is not **0** or **-1**, which causes **parent.Bounds** to change again.

The loop is infinite, and when it begins to happen (usually because of the window border is dragging to resize), **parent** will soon grows to super big and you are no longer able to properly operate against the window.

## Binding to ViewModel Properties

You could also bind something back to the view model by using **-set** property: ``` <Instance ref.Class="..."> <ref.Parameter Name="Something" Class="..."/> <Window ref.Name="self"> <att.Something-set X-bind="aControl.AProperty"/> <AControl ref.Name="aControl"/> </Window> </Instance> ``` **-set** here tells XML that, properties referenced in the tag **\<att.Something/\>** are in the object returning from the **Something** property of the window, which is the view model in **\<ref.Parameter/\>**. Now when **aControl.AProperty** is changed, the value will be updated to **Something.X** immediately.

## Bidirectional Binding

Bidirectional binding is a special type of binding, when two properties update each other.

[<CommonDatePickerLook/>](../.././gacui/components/ctemplates/commondatepickerlook.md) is a good example: ``` <Instance ref.CodeBehind="false" ref.Class="darkskin::DatePickerTemplate" ref.Styles="res://DarkSkin/Style"> <DatePickerTemplate ref.Name="self" Date-bind="look.Date" ...> <CommonDatePickerLook ref.Name="look" Date-bind="self.Date" .../> </DatePickerTemplate> </Instance> ``` In this example, when **self.Date** is changed, the value will be updates to **look.Date**, when **look.Date** is changed, the value will be updates to **self.Date**. Such biditional binding keep these two properties in sync, by using **-bind** twice on each other.

This doesn't cause the update loop to run infinitely because these two **Date** properties don't trigger their property changing event if the updated value is the same to the value before updating.

Properties create by the **prop Name : TYPE {}** syntax will not trigger the **NameChanged** event if the same value is updated to the property again. But for properties with explicit getter and setter, you must handle the setter properly: ``` class YourObject { var name: string = ""; event NameChanged(); func GetName(): string { return name; } func SetName(value: string): void { if (name != value) { name = value; NameChanged(); } } prop Name: string {GetName, SetName : NameChanged} } ``` Otherwise using bidirectional binding on this property will trigger an infinite loop.

