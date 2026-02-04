# Elements

GacUI offer a rich set of predefined elements including:
- **SolidBorder**
- **SolidBackground**
- **GradientBackground**
- **FocusRectangle**
- **Gui3DBorderElement**
- **Gui3DSplitterElement**
- **InnerShadow**
- **SolidLabel**
- **ImageFrame**
- **Polygon**

Before placing an element, a composition is required to be placed at the expected location, and the element will fill the whole area of that composition. When a composition moves, the element is also going to move. The element is also able to set a minimum size constraint to the composition. If every compositions are configured correctly, eventually it sets a calculated constraint to the whole window.

Usually when decorations are needed, control templates are always highly recommended if decorations respond to user interactions. If existing controls do not satisfy the requirement, custom controls with control templates are also recommended, especially when the application is expected to have multiple skin. Control templates are easy to maintain.

The**SolidLabel**element is also very similar to the**Label**control. But as what is mentioned above, a typical scenario to use**Label**controls instead of**SolidLabel**elements is making hyperlinks. Hyperlink reacts to user interactions, the decoration (usually underscores or colors) also reacts to user interactions. So this is better to be made as a control template for**Label**controls.

Another advantages for control templates is that, the same control template can be shared with multiple instances of controls.

Only when control templates do not fit your need, you are recommended to use elements directly.


- Source code:[kb_elements](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_elements/Resource.xml)
- ![](https://gaclib.net/doc/gacui/kb_elements.gif)

There is**DocumentElement**. This element offer full featured document rendering. But they are not designed to be used directly, instead you are expected to use these controls:
- **SinglelineTextBox**
- **MultilineTextBox**
- **DocumentLabel**
- **DocumentViewer**

