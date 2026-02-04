# \<Instance\>

After a**GacUI XML Resource**is converted to a resource binary:
- This resource does not exist in the resource binary.
- The equivalent Workflow script will be included in the Workflow assembly part in the resource binary.
- If you use the one specified in**GacGenConfig/Cpp**, the Workflow assembly is also not included in the resource binary, because you are supposed to use the generated C++ code.

**\<Instance/\>**creates a derived class inheriting from a base class which is specified by the root object. Any[inheritable class](../.././workflow/running/invoking3.md)with a default constructors could be derived by a**\<Instance/\>**.

You can use**\<Instance/\>**to create classes inheriting from:
- A window.
- A tab page.
- A custom control.
- A control template.
- A theme.
- Another**\<Instance/\>**.
- Any other inheritable class, even it is defined in a**\<Script/\>**.

Typically, an**\<Instance/\>**looks like:
```
<Instance name="MainWindowResource">
  <Instance ref.CodeBehind="false" ref.Class="helloworld::MainWindow">
    <!-- multiple -->    <ref.Parameter .../>
    <!-- multiple -->    <ref.LocalizedStrings .../>
    <!-- zero or one --> <ref.Members> <![CDATA[  ...  ]]<![CDATA[></ref.Members>
    <!-- zero or one --> <ref.Ctor>    <![CDATA[ {...} ]]<![CDATA[></ref.Ctor>
    <!-- zero or one --> <ref.Dtor>    <![CDATA[ {...} ]]<![CDATA[></ref.Dtor>
    <Window Text="Hello, world!" ClientSize="x:480 y:320">
      <att.BoundsComposition-set PreferredMinSize="x:480 y:320"/>
      <Label Text="Welcome to GacUI Library!"/>
    </Window>
  </Instance>
</Instance>
```


Here are tags that can be put before the root object:
- **\<ref.Parameter/\>**: A constructor parameter and readonly property.
- **\<ref.LocalizedStrings/\>**: Adding a string resource to the current instance, please check out[<LocalizedStrings>](../.././gacui/xmlres/tag_localizedstrings.md).
- **\<ref.Members/\>**: Multiple Workflow class members that will be added to the created class.
- **\<ref.Ctor/\>**: A Workflow statement containing code that will be executed in the constructor after the instance is created.
- **\<ref.Dtor/\>**: A workflow statement containing code that will be executed in the destructor.

You can define as many**\<ref.Parameter/\>**as you want in an**\<Instance/\>**. For item template, there must be exactly one**\<ref.Parameter/\>**to access the item object. For grid visualizer and grid editor,**RowValue**and**CellValue**properties to access the item object. To inherit from an object with**\<ref.Parameter/\>**, assign all of them like normal properties.

There are two attributes for**\<ref.Parameter/\>**:
- **Name**: The name of the constructor parameter. It can be used inside the root object.
- **Class**: The type of the constructor parameter. If it is a class or an interface, unless[all constructors return raw pointers](../.././workflow/lang/class_new.md), it becomes a shared pointer to the type.

Here are attributes for an**\<Instance/\>**:
- **ref.CodeBehind**: true or false. If it is true, an extra pair of C++ files will be created specifically for this instance.
- **ref.Class**: The class name for this instance. Namespaces are separated by "::".
- **ref.Styles**: Apply one or multiple[<InstanceStyle>](../.././gacui/xmlres/tag_instancestyle.md)on objects in this instance.
- **xmlns**: Default namespaces. If it is not specified, it will be:
```
presentation::controls::Gui*;
presentation::elements::Gui*Element;
presentation::compositions::Gui*Composition;
presentation::compositions::Gui*;
presentation::templates::Gui*;
system::*;
system::reflection::*;
presentation::*;
presentation::Gui*;
presentation::controls::*;
presentation::controls::list::*;
presentation::controls::tree::*;
presentation::elements::*;
presentation::elements::Gui*;
presentation::elements::text::*;
presentation::compositions::*;
presentation::templates::*;
presentation::theme::*
```
Multiple namespaces are separated by ";". The format is similar to[Workflow's using declaration](../.././workflow/lang/module.md).
- **xmlns:NAME**: Specify namespaces for**NAME**, so that you can use**NAME:TYPE**for types that are not in default namespaces. Custom namespaces are available in tag names for objects or control/item template properties. Multiple namespaces are separated by ";". The format is similar to[Workflow's using declaration](../.././workflow/lang/module.md).

Here are attributes for any object inside a**\<Instance/\>**:
- **ref.Name**: Create a private field in this instance holding this object, so that it can be accessed by Workflow code or bindings.

Here are context attributes for any object inside a**\<Instance/\>**:
- **env.ItemName**.
- **env.ItemType**.These are for data binding for list items, please check out[Properties](../.././gacui/xmlres/instance/properties.md)and[Data Bindings](../.././gacui/advanced/bindings.md).

