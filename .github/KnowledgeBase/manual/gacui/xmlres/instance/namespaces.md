# Namespaces

Object types must be registered before using in XML. For**GacBuild.ps1**,registered types`missing document: /home/registered.html`are hardcoded.

There are two ways to use your own types in XML:
- Declare your interfaces in**\<Script/\>**. You can implement them in C++ and pass them to**\<ref.Parameter/Windowgt;**.
- Declare your classes in**\<Script/\>**. These classes will be immediately available in XML. Using[code generation attributes](../../.././workflow/codegen.md)correctly and you will be able to implement them in C++.
- Register`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION.html`your types in C++, and either call the**GacUI XML Resource**compiler defined in**GacUICompiler.h**, or guild your own**GacGen.exe**to replace the on in release for both x86 and x64.

Registered classes are named like**presentation::controls::GuiWindow**. As described in[<Instance>](../../.././gacui/xmlres/tag_instance.md), the default namespace is
```
presentation::controls::GuiWindow;
presentation::elements::GuiWindowElement;
presentation::compositions::GuiWindowComposition;
presentation::compositions::GuiWindow;
presentation::templates::GuiWindow;
system::Window;
system::reflection::Window;
presentation::Window;
presentation::GuiWindow;
presentation::controls::Window;
presentation::controls::list::Window;
presentation::controls::tree::Window;
presentation::elements::Window;
presentation::elements::GuiWindow;
presentation::elements::text::Window;
presentation::compositions::Window;
presentation::templates::Window;
presentation::theme::Window
```
When you write**\<Window/\>**, it replaces*****by**Window**in the list and results in:
```
presentation::controls::GuiWindow;
presentation::elements::GuiWindowElement;
presentation::compositions::GuiWindowComposition;
presentation::compositions::GuiWindow;
presentation::templates::GuiWindow;
system::Window;
system::reflection::Window;
presentation::Window;
presentation::GuiWindow;
presentation::controls::Window;
presentation::controls::list::Window;
presentation::controls::tree::Window;
presentation::elements::Window;
presentation::elements::GuiWindow;
presentation::elements::text::Window;
presentation::compositions::Window;
presentation::templates::Window;
presentation::theme::Window
```
Now only**presentation::controls::GuiWindow**exist, so**\<Window/\>**is**presentation::controls::GuiWindow**. An error will be generated if zero or multiple of them exist.

For types that are not able to be found in the default namespace, like**presentation::controls::GuiSelectableButton::MutexGroupController**which is a very useful component to define a group of**\<RadioButton/\>**, you need to define your own namespace like:
```
<Instance ref.Class="sample::MainWindow" xmlns:x="presentation::controls::GuiSelectableButton::*">
  ...
</Instance>
```
Now**\<x:MutexGroupController/\>**becomes available, it is pointing to**presentation::controls::GuiSelectableButton::MutexGroupController**.

**xmlns:NAMESPACE**only apply to the current**\<Instance/\>**.

Besides of tag names,**NAMESPACE:TYPE**is also available in[properties that expect type names as their values](../../.././gacui/xmlres/instance/properties.md).

