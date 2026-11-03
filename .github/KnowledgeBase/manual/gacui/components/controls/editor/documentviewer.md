# \<DocumentViewer\>

- **\<DocumentViewer/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiDocumentViewer*
  - **Template Tag**: \<DocumentViewerTemplate/\>
  - **Template Name**: DocumentViewer

**\<GuiDocumentViewer/\>** is a **GuiScrollContainer**. The control template doesn't need to care about rendering characters. When it is installed to the control, a fully managed **GuiDocumentElement** will be installed to the **ContainerComposition** of the control template.

**\<CommonScrollViewLook/\>** is very useful to implement the control template.

**\<GuiDocumentViewer/\>** provides both horizontal and vertical scroll bars. When a paragraph is too long to display, auto line-wrapping is automatically done so that the horizontal scroll bar is always not needed. But such default behavior can be changed.

This family, including `GuiMultilineTextBox`, accepts touch by default. [Short/long presses select text](../../../.././gacui/components/controls/editor/home.md) according to `EditMode`; accepted drags scroll while preserving the range. The optional [MomentumScrolling component](../../../.././gacui/components/components/momentumscrolling.md) continues scrolling after release.

