# \<DocumentLabel\> and \<DocumentTextBox\>


- **C++/Workflow**: (vl::)presentation::controls::GuiDocumentLabel*
- **Template Tag**: \<DocumentLabelTemplate/\>
- **\<DocumentLabel/\>**
  - **Template Name**: DocumentLabel
- **\<DocumentTextBox/\>**
  - **Template Name**: DocumentTextBox

The control template doesn't need to care about rendering characters. When it is installed to the control, a fully managed**GuiDocumentElement**will be installed to the**ContainerComposition**of the control template.

**\<DocumentLabel/\>**and**\<DocumentTextBox/\>**don't provide any scroll bar. When a paragraph is too long to display, auto line-wrapping is automatically done so that the horizontal scroll bar is always not needed. Height of these controls will grow to display the whole document. But such default behavior can be changed.

There is no functionality difference between**\<DocumentLabel/\>**and**\<DocumentTextBox/\>**.**\<DocumentLabel/\>**is expected to look like a**\<Label/\>**.**\<DocumentTextBox/\>**is expected to look like**\<SinglelineTextBox/\>**.

