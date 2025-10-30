# Properties

## Syntax

There are two ways to assign values to a property: ``` <Object PROP1="VALUE"> <att.PROP2>VALUE</att.PROP2> </Object> ``` Or using data binding: ``` <Object PROP1-BINDING="VALUE"> <att.PROP2-BINDING>VALUE</att.PROP2-BINDING> </Object> ```

Sometimes you want to assign values to a property in an object which is returned from another property: ``` <Object> <att.PROP1-set PROP2="VALUE"> <!-- all the above four ways are valid --> </att.PROP1-set> </Object> ``` What it do is "OBJECT.PROP1.PROP2 = VALUE". This is very useful when configuring **BoundsComposition** and **ContainerComposition** of a control.

## Kinds of Properties

An object have four kinds of properties all of them are in the same syntax: - `empty list item` **Constructor Parameter**: The value of the parameter must be evaluated before calling the constructor of the object. In **\<ComboBox/\>**, **ListControl** is such kind of property. A constructor parameter is not a real property, it cannot be changed after the object is created. Only assignment, **-uri** binding and **-eval** binding is valid on such properties. - `empty list item` **Virtual Property**: A virtual property is created to make XML more readable. In **\<Table/\>**, **Rows** and **Columns** are such kinds of properties. A virtual parameter is not a real property, it cannot be changed after the object is created. Only assignment, **-ref** binding, **-uri** binding and **-eval** binding is valid on such properties. - `empty list item` **Standard Property**: A standard property is a real property of the object. Typically, assignment to the property **X** is converted to a calling of **SetX**, but the actual generated code can be customized by using reflection`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION.html`.

## Binding

Besides assignment or **-set**, there are several kinds of data binding:

### -uri

This binding retrive an object from a resource and assign to the property. This is a one-time assignment of the property.

Please checkout the following topics for details: - [<Text>](../../.././gacui/xmlres/tag_text.md) - [<Image>](../../.././gacui/xmlres/tag_image.md) - [<Doc>](../../.././gacui/xmlres/tag_doc.md) - [<Xml>](../../.././gacui/xmlres/tag_xml.md)

### -ref

This binding assign an object to the property. This is a one-time assignment of the property.

The value of the property is the name of the object. The name of an object is defined by its **ref.Name** property.

**-ref="self"** on the root instance is a very common pattern, so that you can access the root instance with all custom members in **\<ref.Members/\>**.

### -eval

This binding assign a value to the property. This is a one-time assignment of the property.

The value of the property is a [Workflow expression](../../.././workflow/lang/expr.md).

### -bind

This binding bind the property to a [Workflow expression](../../.././workflow/lang/expr.md). When the expression is changed, the property is assigned again.

Writing **-bind="EXPR"** means observing the **[bind(EXPR)](../../.././workflow/lang/bind.md)** expression. You can also use **-bind="a.observe as b(EXPR on E1, E2, ...)"** since **bind(a.observe as b(EXPR on E1, E2, ...))** is a valid Workflow expression.

### -format

This binding bind the property to [formatted string expression](../../.././workflow/lang/expr.md). When the expression is changed, the property is assigned again.

For example, to bind a text box's content to a label, you can use: ``` <DocumentTextBox ref.Name="textBox"/> <Label Text-format="Hello, $(textBox.Text)!"/> ``` It is equivalent to: ``` <DocumentTextBox ref.Name="textBox"/> <Label Text-bind="$'Hello, $(textBox.Text)!'"/> ``` Note that, both **'** and **"** are valid for strings, so it is very convenient to use **'** in XML attributes to avoid escaping.

### -str

This binding bind the property to string resource. When the expression is changed, the property is assigned again.

Please checkout [<LocalizedStrings>](../../.././gacui/xmlres/tag_localizedstrings.md) for details.

## Deserialization

For any **non-binding** property assignment, GacUI interpret the property value differently for different types of propertyes.

Here are a list of types and how they interpret property values:

### Primitive Types

- For string, the value becomes the string literal. - For bool, integers and numbers, it must be a valid [Workflow literal](../../.././workflow/lang/expr.md). - For DateTime, it must be "YYYY-MM-DD hh:mm:ss.sss"

### Structs

The value must contains multiple "FIELD:VALUE". **FIELD** is a field name in the struct, **VALUE** is a valid [Workflow expression](../../.././workflow/lang/expr.md) with a compatible type. Multiple items a separated by spaces.

It is actually a [Workflow constructor expression](../../.././workflow/lang/expr.md) without the surrounding braces.

### Enums

- For enums, the value must be one of enum items. - For flags, multiple enum item names are seperated by "|".

### Nullable Types

The default value is **null**, but you cannot explicitly assign **null** in XML.

### Classes or Interfaces

An object is expected for these properties. But since an object cannot be created in XML attributes, so you need to use **\<att.NAME/\>**.

If the object type is exactly the property type, you can use **_** as the tag name for that object.

The above rule is also valid for primitive types, for example: ``` <Window Text="The Title"/> ``` is equivalent to ``` <Window> <att.Text> <String>The Title</String> </att.Text> </Window> ``` and ``` <Window> <att.Text> <_>The Title</_> </att.Text> </Window> ```

### Collections

Zero to multiple values is expected for these properties. But since a collection cannot be created in XML attributes, so you need to use **\<att.NAME/\>**.

If the object type is exactly the item type, you can use **_** as the tag name for that object.

For example: ``` <Table> <att.Rows> <CellOption>composeType:MinSize</CellOption> <CellOption>composeType:Percentage percentage:1.0</CellOption> </att.Rows> </Table> ``` is equivalent to ``` <Table> <att.Rows> <_>composeType:MinSize</_> <_>composeType:Percentage percentage:1.0</_> </att.Rows> </Table> ```

### Control/Item Template: func(data: Object): TEMPLATE_CLASS*

The value must be a type name, or multiple type names separated by ";".

A type name can be a full name of a class, like **my::namespace::MyClass**. Or an equivalent tag name with property namespace setting, like **my:MyClass** when there is **\<Instance xmlns:my="my::namespace::*"/\>**.

For item templates, there can be one constructor parameter, which is typically a **\<ref.Parameter/\>**.

The item type is unknown, because for a bindable list control, each item can be different type. If there are multiple item templates with different constructor parameter type, each constructor parameter type will be tested until there is the first one that is compatible with the actual item type.

The above test always succeeds for item templates that have no constructor parameter.

This is a common pattern to apply different item templates on different items in a list control.

### Property Name: func(data: Object): PROPERTY_TYPE

When there is a **env.ItemType** attribute on the object having this property of or on any parent object, the type of the object to retrieve the property value at runtime is expected to be **env.ItemType**. In this case, the value of this property must be a valid property or field name.

When there is also a **env.ItemName** attribute, the value of this property can also be a valid [Workflow expression](../../.././workflow/lang/expr.md), the **env.ItemName** becomes a valid identifier representing the object to retrieve the property.

For example: when you bind a collection to a **\<BindableDataGrid/\>**, different columns can be configured to displays different properies of the collection item like this: ``` <BindableDataGrid env.ItemType="demo::MyDataItem^" env.ItemName="itemToBind"> <att.Columns> <DataColumn Text="Name" Size="120"> <att.TextProperty>Name</att.TextProperty> </DataColumn> <DataColumn Text="Gender" Size="80"> <att.TextProperty>demo::ToString(itemToBind.Gender)</att.TextProperty> </DataColumn> </att.Columns> </BindableDataGrid> ```

### Writable Property Name: func(data: Object, value: PROPERTY_TYPE, isWrite: bool): PROPERTY_TYPE

**Writable Property Name** is very similar to **Property Name**. The only difference is that, regardless the property value is an identifier or an expression, it must be assignable.

**NOTE:** struct fields and readonly properties are not assignable.

### \<BindableDataGrid\> column properties

#### presentation::controls::list::IDataVisualizerFactory^

A **IDataVisualizerFactory** property is very similar to **Item Template**. There is no input object to select a visualizer. Instead, multiple visualizer are all created and nested together, so that different visualizers could share some decoration like borders. The first one is the most inner one.

Each visualizer must inherit from **presentation::templates::GuiGridVisualizerTemplate**. The **CellValue** property will be assigned according to the **ValueProperty** property of the column.

#### presentation::controls::list::IDataEditorFactory^

A **IDataEditorFactory** property is very similar to **Item Template**. There is no input object to select an editor.

Each editor must inherit from **presentation::templates::GuiGridEditorTemplate**. The **CellValue** property will be assigned according to the **ValueProperty** property of the column.

#### presentation::controls::list::IDataFilter^

The property value of a **IDataFilter^** property is expected to be a [Workflow lambda expression](../../.././workflow/lang/expr.md). The first argument is the row object, which is an item in the **ItemSource** collection of the list control. Returns true to make the row visible.

**NOTE**: for **-eval** binding, the expression must returns an object implementing the **IDataFilter** interface.

#### presentation::controls::list::IDataSorter^

The property value of a **IDataSorter^** property is expected to be a [Workflow lambda expression](../../.././workflow/lang/expr.md). The first and second arguments are two row objects to compare, which are two items in the **ItemSource** collection of the list control. Returns a negative, zero, positive number for "less than", "equal", "greater than".

**NOTE**: for **-eval** binding, the expression must returns an object implementing the **IDataSorter** interface.

