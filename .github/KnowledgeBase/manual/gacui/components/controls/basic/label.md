# \<Label\>

- **\<Label/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiLabel* - **Template Tag**: \<LabelTemplate/\> - **Template Name**: Label

**\<Label/\>** displays text in a specified font and color.

**\<Label/\>** uses **Text** and **Font** properties to display text. If no minimum size is specified for **\<Label/\>**, the control always resize itself just enough to display all characters. Multiple lines of text are allowed, but one line doesn't wrap to multiple lines if the width of the control is not long enough.

**TextColor** defines the color of the text, the default value is the **DefaultTextColor** from its control template. You can change **\<Label/\>::TextColor** to override **\<LabelTemplate/\>::DefaultTextColor**, but if **TextColor** have the same value of **DefaultTextColor**, it is marked as "not overriding", and it changes if the control switch to another control template with a different **DefaultTextColor**.

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Xml/Instance_Window/UI/Resource.xml) for details.

