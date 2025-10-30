# \<Button\>

- **\<Button/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiButton* - **Template Tag**: \<ButtonTemplate/\> - **Template Name**: Button

**\<Button/\>** is a control for user to execute a command.

## \<Button\> Properties

### ClickOnMouseUp

The default value for this property is **true**, but it could be different in sub classes.

When this property is **true**, **Clicked** raises when the mouse button is released. When this property is **false**, **Clicked** raises when the mouse button is pressed.

### AutoFocus

The default value for this property is **true**, but it could be different in sub classes.

When this property is **true**: - The button is focused when the mouse button is pressed. - The button is focused when it is executed by a **ALT** sequence. - The button accepts focus by **TAB**.

### IgnoreChildControlMouseEvents

The default value for this property is **true**, but it could be different in sub classes.

When mouse operations happen in child controls which are contained in a button, the button ignores these operations so that it behaves like not being clicked. But once the **IgnoreChildControlMouseEvents** property is set to **false**, this behavior changes.

This property is very useful when adding child controls for visual effect only.

#### Sample

- Source code: [control_basic_button](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_button/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_button.gif)

## \<Button\> Events

### Clicked

This event raises when it is clicked, or it is activated by a **ALT** sequence.

