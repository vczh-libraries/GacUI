# Elements

[Elements](../../.././gacui/kb/elements.md) offer rendering of geometry shapes and texts.

Elements render differently under different DPI and text scaling.

GacUI with Windows GDI renderer does not support high DPI. Rendering will be performed under 96 DPI with 100% text scaling, and Windows will blur the window using its default behavior.

GacUI with Windows Direct2D renderer supports high DPI. Text size will scale with the system, and line width will be enlarged with higher DPI. For example, **\<SolidBorder/\>** draw a one pixel border in 96 DPI. But for 192 DPI, the border becomes 2 pixels. The differences between Direct2D and GDI with higher DPI is that, texts and images are not blurred for Direct2D.

Some renderers do not support alpha channel. For example, GDI ignores the value in the alpha channel. But when the alpha channel has a value of zero for geometry elements, it doesn't render.

## Format of Shape property

Some elements has a **Shape** property to define its shape. It accepts 3 kinds of values:
- **shapeType:Rectangle**: A rectangle.
- **shapeType:Ellipse**: An ellipse.
- **shapeType:RoundRect radiusX:x radiusY:y**: A rectangle with round coners. Radius on X and Y axis for corners are required in the value.

## Format of Color property

Some elements has one or more color properties to define colors. The value could be **#RRGGBB** or **#RRGGBBAA**.

## \<FocusRectangle\>

**\<FocusRectangle\>** renders an OS dependent rectangle for a focused visual effect. In Windows it is formed by multiple dots, which are applied with XOR operator.

## \<SolidBorder\>

**\<SolidBorder\>** renders a border of a specified shape.

- **Shape** defines the shape of this element.
- **Color** property defines the color of the border.

## \<SolidBackground\>

**\<SolidBorder\>** fills color in a specified shape.

- **Shape** defines the shape of this element.
- **Color** property defines the color of the shape.

## \<Gui3DBorderElement\>

**\<Gui3DBorderElement\>** renders a border of a 3D-like rectangle.

- **Color1** property defines the bright color.
- **Color2** property defines the dark color.

If **Color1** is brighter than **Color2**, the element looks like a raised rectangle.

If **Color1** is darker than **Color2**, the element looks like a sink rectangle.

## \<Gui3DSplitterElement\>

**\<Gui3DSplitterElement\>** renders a border of a 3D-like line.

- **Color1** property defines the bright color.
- **Color2** property defines the dark color.
- **Direction** property defines the direction of the line. The value could be **Horizontal** or **Vertical**.

If **Color1** is brighter than **Color2**, the element looks like a raised line.

If **Color1** is darker than **Color2**, the element looks like a sink line.

## \<GradientBackground\>

**\<GradientBackground\>** fills gradient colors in a specified shape.

- **Shape** defines the shape of this element.
- **Color1** property defines the beginning color.
- **Color2** property defines the ending color.
- **Direction** property defines the direction of the gradient. The value could be **Horizontal**, **Vertical**, **Slash** or **Backslash**.

## \<InnerShadow\>

**\<SolidBorder\>** renders a border with gradient colors of a specified shape.

- **Thickness** defines the width of the border.
- **Color** property defines the outer color of the border. It fades to **#00000000** towards the center.

## \<SolidLabel\>

**\<SolidLabel\>** renders text.

- **Color** defines the color of the text.
- **Font** defines the font of the text. It could be binded to the **DisplayFont** property of a control or the **Font** property of a control template. The type of this property is **presentation::FontProperties**, which is a [struct](../../.././gacui/xmlres/instance/properties.md).
- **Thickness** defines the width of the text.
- **Text** defines the content of the text.
- **HorizontalAlignment** defines the horizontal alignment of the text. The value could be **Center**, **Left**, or **Right**. The default value is **Left**.
- **VerticalAlignment** defines the vertical alignment of the text. The value could be **Center**, **Top**, or **Bottom**. The default value is **Bottom**.
- **WrapLine** defaults to **false**. When it is **true**, the element wraps the text to multiple line if it exceeds the expected width.
- **Ellipse** defaults to **false**. When it is **true**, the element renders out-of-bound text to **...** line if it exceeds the expected width.
- **Multiline** defaults to **false**. When it is **true**, the element interpret **\n** as a line break.
- **WrapLineHeightCalculation** defaults to **false**. When it is **false**, the minimum size of the element does not consider **WrapLine**. When it is **true**, the minimum height of the element depends on the expected width, and the minimum width is the expected width.

## \<ImageFrame\>

**\<ImageFrame\>** renders an image.

- **Image** defines the source of an image. It could be binded to image properties of a control or a control template. The [-uri](../../.././gacui/xmlres/instance/properties.md) binding could be used to specify an image from the resource.
- **FrameIndex** defaults to 0. It specified the target frame in the image to render. All image contains only one frame except **gif**.
- **HorizontalAlignment** defines the horizontal alignment of the text. The value could be **Center**, **Left**, or **Right**. The default value is **Left**.
- **VerticalAlignment** defines the vertical alignment of the text. The value could be **Center**, **Top**, or **Bottom**. The default value is **Bottom**.
- **Stretch** defaults to **false**. When it is **true**, image fills the expected size instead of keeping its original size. And the minimum size of this element becomes 0.
- **Enabled** defaults to **true**. When it is **false**, the element renders a gray-scaled image. Typically this is used on icons of toolstrip buttons.

## \<Polygon\>

**\<Polygon\>** renders a polygon with a border color and a filling color.

- **BorderColor** defaults to **#00000000**. It defines the color of the border.
- **BackgroundColor** defaults to **#00000000**. It defines the color of the shape.
- **Size** defines the size of the polygon. It also becomes the minimum size of this element.
- **Points** is an array of **presentation::Point**. It defines all vertices of the polygon in order.

The **Size** property creates a coordinate space for **Points** with (0,0) in the top-left corner. When the size of the element is larger than the minimum size, the coordinate space is aligned to the center.

## \<Raw\>: owner drawing

`<Raw/>` is the standard, recommended way to implement owner drawing in GacUI, both in native applications and through the remote protocol. It places application-defined drawing inside a composition while keeping layout and input handling in GacUI. TUI does not support `<Raw/>`.

Place one `<Raw/>` inside the composition that owns the drawing area. Its optional string properties are `Id` and `Data`; both default to an empty string. `ref.Name` exposes the element to generated code as `GuiRawElementBase`. The element has no intrinsic minimum size, so set a size on the composition when needed. For example, this fragment can appear inside an instance's composition tree:
```xml
<Bounds PreferredMinSize="x:240 y:240">
  <Raw ref.Name="clock" Id="Example.Clock" Data="10:20:30"/>
</Bounds>
```

`Id` selects the application's drawing convention; it is not a unique element key. Empty and duplicate IDs are valid, so track drawing attachments by element identity. `Data` carries application-defined state, such as the clock's `HH:mm:ss` string. Changing `Data` requests a repaint. Changing `Id` alone does not request one, and equal assignments do nothing. With the remote protocol, an ID-only change is sent on the next frame.

### Native rendering APIs

The active graphics resource manager chooses the concrete element for `<Raw/>`. All four native implementations derive from `GuiRawElementBase` and expose a C++ `Rendering` event:
- `GuiDirect2DElement` on Windows provides `GuiDirect2DElementEventArgs` with `element`, `rt` (`ID2D1RenderTarget*`), Direct2D/DirectWrite factories and `bounds`. Release target-bound resources in `BeforeRenderTargetChanged` and recreate them for the new target in `AfterRenderTargetChanged`; late subscribers can initialize them on their first render.
- `GuiGDIElement` on Windows provides `GuiGDIElementEventArgs` with `element`, `dc` (`windows::WinDC*`) and `bounds`. Use the supplied wrapper or its `GetHandle()` for GDI drawing.
- `GuiWaylandElement` in wGac provides `GuiWaylandElementEventArgs` with `element`, `context` (`cairo_t*`) and `bounds`.
- `GuiCocoaElement` in iGac provides `GuiCocoaElementEventArgs` with `element`, `context` (`CGContextRef`) and `bounds`.

Attach handlers to the concrete native element after creating the UI, and keep them attached through its drawing lifetime. Draw within the supplied bounds using the current clip and coordinate transform. Borrow drawing contexts only for the callback; do not retain or destroy them, start another frame, or present the target. In particular, Wayland and Cocoa contexts may change between paint calls. Detach handlers before destroying their owning drawing objects.

The [RawClock tutorial](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Windows/RawClock) demonstrates this API with Direct2D by default and GDI when launched with `/Gdi`. Its XML owns the clock state; native callbacks paint the supplied state.

### Remote rendering

A remote core creates a portable `GuiRemoteRawElement`, exposing the shared `GuiRawElementBase` state. The protocol sends the numeric renderer element `id`, application string `rawId` and string `data`. The receiving renderer draws locally; native pointers, drawing callbacks and rendered pixels are not transferred. Install a matching drawing implementation in each renderer that the application supports.

The native `GuiRemoteRendererSingle` exposes `GetRawElements()` and `RawElementInstalled`, `RawElementUninstalled` and `RawElementChanged`. Subscribe before processing messages and enumerate existing elements for late attachment. An installed element initially has empty state; select or replace its drawing attachment when its ID changes. Changed callbacks see both updated descriptor fields, including data-only changes. Uninstallation is the point to remove attachments; a temporarily hidden element or retained disconnected picture may still exist.

GacJS exposes `getRawElements()`, `rawElementInstalled`, `rawElementUninstalled` and `rawElementUpdated` on `IGacUIRenderer`. A `GacUIRawElement` contains a persistent `element` div plus application `id` and `data` strings. Application code owns any canvas or other drawing content placed inside that div, and must dispose its subscriptions and observers at teardown.

## \<ColorizedTextElement\> and \<DocumentElement\>

These elements are for implementing editing and rendering of all text box controls. They are not supposed to be used directly.

