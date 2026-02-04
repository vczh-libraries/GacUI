# Root Instance

Root instance is a direct child in an**\<Instance/\>**, whose tag name does not begin with "ref.". There must be exactly one root instance in an**\<Instance/\>**

The type of the root instance is the base class of this**\<Instance/\>**. To create a class:
```
namespace helloworld
{
    class MainWindow : presentation::controls::GuiWindow
    {
        new*(){}
    }
}
```
The following**\<Instance/\>**is expected:
```
<Instance name="MainWindowResource">
  <Instance ref.Class="helloworld::MainWindow">
    <Window/>
  </Instance>
</Instance>
```


**IMPORTANT**: The generated constructor of instances always return a raw pointer.

Only[inheritable classs](../../.././workflow/running/invoking3.md)can be used as a type of root instances. The type of a root instance is also required to have a constructor without parameters. But GacUI also knows how to create UI objects, so**\<Window/\>**is also a valid type of a root instance even it has no default constructor.

Here are of valid root instance types:

## \<Window\>

Window is a control that can be displayed in your OS. Unlike other controls, it doesn't need to have a parent control.

There are a few other controls that inherit from Window, but the only valid type for a root instance is Window.

## \<TabPage\>

TabPage is a child control for Tab. When a TabPage is added to a Tab, a tab header containing TabPage's**Text**property will appear in the Tab.

It is very common for application that, TabPages in a Tab are dynamically created based on context. For example, pages in a wizard can be changed when you select different options in some pages.

In this scenario, by creating TabPages as instances, with the power of data binding and MVVM, it is very easy to organize the application.

## \<RibbonTabPage\>

RibbonTabPage is a child control for RibbonTab.

## \<CustomControl\>

CustomControl can be a child control for any control. CustomControl itself doesn't provide any features, it is another way to organize the application in multiple instances.

When the content of the application is also dynamically controlled based on a context, but Tab doesn't fit the design. For these scenarios, CustomControls can be organized with a few more Workflow script, which is also very convenient.

## Templates and \<ThemeTemplates\>

All control templates and item templates can be types of root items.

If you have already created a new design for multiple and even all controls, you can use**\<ThemeTemplates/\>**[like this](../../.././gacui/kb/dtemplates.md)to make your design a default one for your application.

## Another Instance

An instance is allowed to inherit from another instance. See[Namespaces](../../.././gacui/xmlres/instance/namespaces.md)and[Instance Inheriting](../../.././gacui/xmlres/instance/inheriting.md)for more details.

## Others

Any inheritable classes with a default constructor can be the type of a root instance. You can define a class in**\<Script/\>**and use it as an instance.

This is a typical scenario for creating configurations. By defining a configuration as a Workflow class, you can create different options for that configuration as different instances, or create another class with a fixed amount of options as properties, and assigning option details to these properties.

The[animation sample](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_animation/Resource.xml)is a good example. Please check out and see how**AnimationState**,**AnimationConfig**and**RadioButtonAnimationConfig**are used, they form a very common pattern of using instances as configurations.

