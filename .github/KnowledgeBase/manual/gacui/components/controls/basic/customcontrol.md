# \<CustomControl\>


- **\<CustomControl/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiCustomControl*
  - **Template Tag**: \<ControlTemplate/\>
  - **Template Name**: CustomControl

**\<CustomControl/\>**is a control for making new controls in GacUI XML Resource.
```
<Instance ref.CodeBehind="false" ref.Class="demo::MyControl>
  <CustomControl/>
</Instance>
```
**\<CustomControl/\>**is usually a base class of an[<Instance>](../../../.././gacui/xmlres/tag_instance.md). Here we create a class**demo::MyControl**inheriting from**\<CustomControl/\>**.

To add new properties and other members to the new control, just define them in**Workflow**and put all of them in[<ref.Members/>](../../../.././gacui/xmlres/tag_instance.md).

To make such control focusable, use the**FocusableComposition**to specify a composition to receive keyboard and other related events.

## Adding something to a custom control

**\<CustomControl/\>**is just a control. Compositions and controls in the window will be added to its**ContainerComposition**. You don't have to explicitly use**att.ContainerComposition**.

As a**GuiInstanceRootObject**, components can also be added to a window.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/TriplePhaseImageButton)for details.

