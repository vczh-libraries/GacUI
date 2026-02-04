# GuiSelectableButton


- **C++/Workflow**: (vl::)presentation::controls::GuiSelectableButton*
- **Template Tag**: \<SelectableButtonTemplate/\>
- **\<CheckBox/\>**
  - **Template Name**: CheckBox
- **\<RadioButton/\>**
  - **Template Name**: RadioButton

**GuiSelectableButton**is a**\<Button/\>**with the ability to be selected.

Usually,**\<CheckBox/\>**represents an independent option, and multiple**\<RadioButton/\>**represent a group of exclusive choices.

## GuiSelectableButton Properties

### AutoSelection (AutoSelectionChanged)

The default value for this property is**true**, but it could be different in sub classes.

When this property is**true**, the**Selected**property reverses when**Clicked**raises, otherwise, the**Selected**property is not changed automatically, but it could be changed by code.

### Selected (SelectedChanged)

The default value for this property is**false**.

This property doesn't change behaviors, it is used to record a selection and tell the control template to render differently.

### GroupController (GroupControllerChanged)

The default value for this property is**null**.

A**GuiSelectableButton::GroupController**instance is required for this property.**GuiSelectableButton::MutexGroupController**is a predefined group controller to make**\<RadioButton/\>**exclusive to each other.

typical steps to create a group of**\<RadioButton/\>**:
- Create a namespace mapping if necessary, like**xmlns:x="presentation::controls::GuiSelectableButton::*"**
- Create one**\<x:MutexGroupController/\>**for each group of radio buttons, as a component for the current**\<Instance/\>**.
- Assign**GroupController**of radio buttons in the same group to the same**\<x:MutexGroupController/\>****NOTE:****"xmlns:x"**is just a name, you can map the namespace to any name you like. If you call it**"xmlns:abc"**, then the controller becomes**\<abc:MutexGroupController/\>**.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/TextListTabPage.xml)for**\<CheckBox/\>**,**\<RadioButton/\>**and**\<x:MutexGroupController/\>**

You could create a new class and inherit from**GuiSelectableButton::GroupController**and use it in the**GroupController**property, for a scenario like "only 3 options could be selected", and automatically canceled the oldest option when 4 options are selected. But in order to prevent users from being confused, error messages are recommended instead of changing users' input.

## Sample


- Source code:[control_basic_checkbox](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_checkbox/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_basic_checkbox.gif)

