# \<SinglelineTextBox\>

- **\<SinglelineTextBox/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiSinglelineTextBox* - **Template Tag**: \<DocumentLabelTemplate/\> - **Template Name**: SinglelineTextBox

**\<SinglelineTextBox/\>** is a **GuiDocumentLabel**. The control template doesn't need to care about rendering characters. When it is installed to the control, a fully managed **GuiDocumentElement** will be installed to the **ContainerComposition** of the control template.

**\<SinglelineTextBox/\>** doesn't provide any scroll bar. The control only keep the first line of any input. When multiple lines of text are assigned to **Text**, text from the second line are all discarded.

The **PasswordChar** property is **L'\0'** by default. When any other value is assigned to this property, all characters of the text will be rendered as **PasswordChar**.

