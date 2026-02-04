# Elements

[Elements](../../.././gacui/kb/elements.md)offer rendering of geometry shapes and texts.

Elements render differently under different DPI and text scaling.

GacUI with Windows GDI renderer does not support high DPI. Rendering will be performed under 96 DPI with 100% text scaling, and Windows will blur the window using its default behavior.

GacUI with Windows Direct2D renderer supports high DPI. Text size will scale with the system, and line width will be enlarged with higher DPI. For example,**\<SolidBorder/\>**draw a one pixel border in 96 DPI. But for 192 DPI, the border becomes 2 pixels. The differences between Direct2D and GDI with higher DPI is that, texts and images are not blurred for Direct2D.

Some renderers do not support alpha channel. For example, GDI ignores the value in the alpha channel. But when the alpha channel has a value of zero for geometry elements, it doesn't render.

## Format of Shape property

Some elements has a**Shape**property to define its shape. It accepts 3 kinds of values:
- **shapeType:Rectangle**: A rectangle.
- **shapeType:Ellipse**: An ellipse.
- **shapeType:RoundRect radiusX:x radiusY:y**: A rectangle with round coners. Radius on X and Y axis for corners are required in the value.

## Format of Color property

Some elements has one or more color properties to define colors. The value could be**#RRGGBB**or**#RRGGBBAA**.

## \<FocusRectangle\>

**\<FocusRectangle\>**renders an OS dependent rectangle for a focused visual effect. In Windows it is formed by multiple dots, which are applied with XOR operator.

## \<SolidBorder\>

**\<SolidBorder\>**renders a border of a specified shape.


- **Shape**defines the shape of this element.
- **Color**property defines the color of the border.

## \<SolidBackground\>

**\<SolidBorder\>**fills color in a specified shape.


- **Shape**defines the shape of this element.
- **Color**property defines the color of the shape.

## \<Gui3DBorderElement\>

**\<Gui3DBorderElement\>**renders a border of a 3D-like rectangle.


- **Color1**property defines the bright color.
- **Color2**property defines the dark color.

If**Color1**is brighter than**Color2**, the element looks like a raised rectangle.

If**Color1**is darker than**Color2**, the element looks like a sink rectangle.

## \<Gui3DSplitterElement\>

**\<Gui3DSplitterElement\>**renders a border of a 3D-like line.


- **Color1**property defines the bright color.
- **Color2**property defines the dark color.
- **Direction**property defines the direction of the line. The value could be**Horizontal**or**Vertical**.

If**Color1**is brighter than**Color2**, the element looks like a raised line.

If**Color1**is darker than**Color2**, the element looks like a sink line.

## \<GradientBackground\>

**\<GradientBackground\>**fills gradient colors in a specified shape.


- **Shape**defines the shape of this element.
- **Color1**property defines the beginning color.
- **Color2**property defines the ending color.
- **Direction**property defines the direction of the gradient. The value could be**Horizontal**,**Vertical**,**Slash**or**Backslash**.

## \<InnerShadow\>

**\<SolidBorder\>**renders a border with gradient colors of a specified shape.


- **Thickness**defines the width of the border.
- **Color**property defines the outer color of the border. It fades to**#00000000**towards the center.

## \<SolidLabel\>

**\<SolidLabel\>**renders text.


- **Color**defines the color of the text.
- **Font**defines the font of the text. It could be binded to the**DisplayFont**property of a control or the**Font**property of a control template. The type of this property is**presentation::FontProperties**, which is a[struct](../../.././gacui/xmlres/instance/properties.md).
- **Thickness**defines the width of the text.
- **Text**defines the content of the text.
- **HorizontalAlignment**defines the horizontal alignment of the text. The value could be**Center**,**Left**, or**Right**. The default value is**Left**.
- **VerticalAlignment**defines the vertical alignment of the text. The value could be**Center**,**Top**, or**Bottom**. The default value is**Bottom**.
- **WrapLine**defaults to**false**. When it is**true**, the element wraps the text to multiple line if it exceeds the expected width.
- **Ellipse**defaults to**false**. When it is**true**, the element renders out-of-bound text to**...**line if it exceeds the expected width.
- **Multiline**defaults to**false**. When it is**true**, the element interpret**\n**as a line break.
- **WrapLineHeightCalculation**defaults to**false**. When it is**false**, the minimum size of the element does not consider**WrapLine**. When it is**true**, the minimum height of the element depends on the expected width, and the minimum width is the expected width.

## \<ImageFrame\>

**\<ImageFrame\>**renders an image.


- **Image**defines the source of an image. It could be binded to image properties of a control or a control template. The[-uri](../../.././gacui/xmlres/instance/properties.md)binding could be used to specify an image from the resource.
- **FrameIndex**defaults to 0. It specified the target frame in the image to render. All image contains only one frame except**gif**.
- **HorizontalAlignment**defines the horizontal alignment of the text. The value could be**Center**,**Left**, or**Right**. The default value is**Left**.
- **VerticalAlignment**defines the vertical alignment of the text. The value could be**Center**,**Top**, or**Bottom**. The default value is**Bottom**.
- **Stretch**defaults to**false**. When it is**true**, image fills the expected size instead of keeping its original size. And the minimum size of this element becomes 0.
- **Enabled**defaults to**true**. When it is**false**, the element renders a gray-scaled image. Typically this is used on icons of toolstrip buttons.

## \<Polygon\>

**\<Polygon\>**renders a polygon with a border color and a filling color.


- **BorderColor**defaults to**#00000000**. It defines the color of the border.
- **BackgroundColor**defaults to**#00000000**. It defines the color of the shape.
- **Size**defines the size of the polygon. It also becomes the minimum size of this element.
- **Points**is an array of**presentation::Point**. It defines all vertices of the polygon in order.

The**Size**property creates a coordinate space for**Points**with (0,0) in the top-left corner. When the size of the element is larger than the minimum size, the coordinate space is aligned to the center.

## \<ColorizedTextElement\> and \<DocumentElement\>

These elements are for implementing editing and rendering of all text box controls. They are not supposed to be used directly.

