/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System and Infrastructure Interfaces

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSELEMENT
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSELEMENT

#include "GuiGraphicsResourceManager.h"
#include "../Resources/GuiResource.h"
#include "../Resources/GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
Elements
***********************************************************************/

			/// <summary>
			/// Defines a shape for some <see cref="IGuiGraphicsElement"></see>.
			/// </summary>
			enum class ElementShapeType
			{
				/// <summary>Rectangle shape.</summary>
				Rectangle,
				/// <summary>Ellipse shape.</summary>
				Ellipse,
				/// <summary>Round rectangle shape.</summary>
				RoundRect,
			};

			/// <summary>
			/// Defines a shape for some <see cref="IGuiGraphicsElement"></see>.
			/// </summary>
			struct ElementShape
			{
				ElementShapeType		shapeType = ElementShapeType::Rectangle;
				int						radiusX = 0;
				int						radiusY = 0;

				bool operator==(const ElementShape& value)const { return shapeType == value.shapeType && radiusX == value.radiusX && radiusY == value.radiusY; }
				bool operator!=(const ElementShape& value)const { return !(*this == value); }
			};

			/// <summary>
			/// Defines a focus rectangle with a thickness of one pixel.
			/// </summary>
			class GuiFocusRectangleElement : public GuiElementBase<GuiFocusRectangleElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiFocusRectangleElement, L"FocusRectangle")
			protected:

				GuiFocusRectangleElement();
			public:
			};

			/// <summary>
			/// Defines a border element with a thickness of one pixel.
			/// </summary>
			class GuiSolidBorderElement : public GuiElementBase<GuiSolidBorderElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiSolidBorderElement, L"SolidBorder")
			protected:
				Color					color;
				ElementShape			shape;

				GuiSolidBorderElement();
			public:
				/// <summary>
				/// Get the border color.
				/// </summary>
				/// <returns>The border color.</returns>
				Color					GetColor();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new border color.</param>
				void					SetColor(Color value);
				/// <summary>
				/// Get the shape.
				/// </summary>
				/// <returns>The shape.</returns>
				ElementShape			GetShape();
				/// <summary>
				/// Set the shape.
				/// </summary>
				/// <param name="value">The new shape.</param>
				void					SetShape(ElementShape value);
			};
			
			/// <summary>
			/// Defines a 3D-like rectangle element with a thickness of two pixels.
			/// </summary>
			class Gui3DBorderElement : public GuiElementBase<Gui3DBorderElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(Gui3DBorderElement, L"3DBorder")
			protected:
				Color					color1;
				Color					color2;

				Gui3DBorderElement();
			public:
				/// <summary>
				/// Get the left-top color.
				/// </summary>
				/// <returns>The left-top color.</returns>
				Color					GetColor1();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new left-top color.</param>
				void					SetColor1(Color value);
				/// <summary>
				/// Get the right-bottom color.
				/// </summary>
				/// <returns>The right-bottom color.</returns>
				Color					GetColor2();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new right-bottom color.</param>
				void					SetColor2(Color value);
				/// <summary>
				/// Set colors of the element.
				/// </summary>
				/// <param name="value1">The new left-top color.</param>
				/// <param name="value2">The new right bottom color.</param>
				void					SetColors(Color value1, Color value2);
			};
			
			/// <summary>
			/// Defines a 3D-like splitter element with a thickness of two pixels.
			/// </summary>
			class Gui3DSplitterElement : public GuiElementBase<Gui3DSplitterElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(Gui3DSplitterElement, L"3DSplitter")
			public:
				/// <summary>
				/// Defines a direction of the <see cref="Gui3DSplitterElement"></see>.
				/// </summary>
				enum Direction
				{
					/// <summary>Horizontal direction.</summary>
					Horizontal,
					/// <summary>Vertical direction.</summary>
					Vertical,
				};
			protected:
				Color					color1;
				Color					color2;
				Direction				direction;

				Gui3DSplitterElement();
			public:
				/// <summary>
				/// Get the left-top color.
				/// </summary>
				/// <returns>The left-top color.</returns>
				Color					GetColor1();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new left-top color.</param>
				void					SetColor1(Color value);
				/// <summary>
				/// Get the right-bottom color.
				/// </summary>
				/// <returns>The right-bottom color.</returns>
				Color					GetColor2();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new right-bottom color.</param>
				void					SetColor2(Color value);
				/// <summary>
				/// Set colors of the element.
				/// </summary>
				/// <param name="value1">The new left-top color.</param>
				/// <param name="value2">The new right bottom color.</param>
				void					SetColors(Color value1, Color value2);
				
				/// <summary>
				/// Get the direction.
				/// </summary>
				/// <returns>The direction.</returns>
				Direction				GetDirection();
				/// <summary>
				/// Set the direction.
				/// </summary>
				/// <param name="value">The new direction.</param>
				void					SetDirection(Direction value);
			};

			/// <summary>
			/// Defines a color-filled element without border.
			/// </summary>
			class GuiSolidBackgroundElement : public GuiElementBase<GuiSolidBackgroundElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiSolidBackgroundElement, L"SolidBackground")
			protected:
				Color					color;
				ElementShape			shape;

				GuiSolidBackgroundElement();
			public:
				/// <summary>
				/// Get the border color.
				/// </summary>
				/// <returns>The border color.</returns>
				Color					GetColor();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new border color.</param>
				void					SetColor(Color value);
				/// <summary>
				/// Get the shape.
				/// </summary>
				/// <returns>The shape.</returns>
				ElementShape			GetShape();
				/// <summary>
				/// Set the shape.
				/// </summary>
				/// <param name="value">The new shape.</param>
				void					SetShape(ElementShape value);
			};
			
			/// <summary>
			/// Defines a color-filled gradient element without border.
			/// </summary>
			class GuiGradientBackgroundElement : public GuiElementBase<GuiGradientBackgroundElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiGradientBackgroundElement, L"GradientBackground")
			public:
				/// <summary>
				/// Defines a direction of the <see cref="GuiGradientBackgroundElement"></see>.
				/// </summary>
				enum Direction
				{
					/// <summary>Horizontal direction.</summary>
					Horizontal,
					/// <summary>vertical direction.</summary>
					Vertical,
					/// <summary>Slash direction.</summary>
					Slash,
					/// <summary>Back slash direction.</summary>
					Backslash,
				};
			protected:
				Color					color1;
				Color					color2;
				Direction				direction;
				ElementShape			shape;

				GuiGradientBackgroundElement();
			public:
				/// <summary>
				/// Get the left-top color.
				/// </summary>
				/// <returns>The left-top color.</returns>
				Color					GetColor1();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new left-top color.</param>
				void					SetColor1(Color value);
				/// <summary>
				/// Get the right bottom color.
				/// </summary>
				/// <returns>The right-bottom color.</returns>
				Color					GetColor2();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new right-bottom color.</param>
				void					SetColor2(Color value);
				/// <summary>
				/// Set colors of the element.
				/// </summary>
				/// <param name="value1">The new left-top color.</param>
				/// <param name="value2">The new right bottom color.</param>
				void					SetColors(Color value1, Color value2);
				
				/// <summary>
				/// Get the direction.
				/// </summary>
				/// <returns>The direction.</returns>
				Direction				GetDirection();
				/// <summary>
				/// Set the direction.
				/// </summary>
				/// <param name="value">The new direction.</param>
				void					SetDirection(Direction value);
				/// <summary>
				/// Get the shape.
				/// </summary>
				/// <returns>The shape.</returns>
				ElementShape			GetShape();
				/// <summary>
				/// Set the shape.
				/// </summary>
				/// <param name="value">The new shape.</param>
				void					SetShape(ElementShape value);
			};

			/// <summary>
			/// Defines a gradient border for shadow.
			/// </summary>
			class GuiInnerShadowElement : public GuiElementBase<GuiInnerShadowElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiInnerShadowElement, L"InnerShadow")
			protected:
				Color					color;
				vint					thickness = 0;

				GuiInnerShadowElement();
			public:
				/// <summary>
				/// Get the shadow color.
				/// </summary>
				/// <returns>The shadow color.</returns>
				Color					GetColor();
				/// <summary>
				/// Set the shadow color.
				/// </summary>
				/// <param name="value">The new shadow color.</param>
				void					SetColor(Color value);

				/// <summary>
				/// Get the thickness.
				/// </summary>
				/// <returns>The thickness.</returns>
				vint					GetThickness();
				/// <summary>
				/// Set the thickness.
				/// </summary>
				/// <param name="value">The new thickness.</param>
				void					SetThickness(vint value);
			};
			
			/// <summary>
			/// Defines an element of a plain text.
			/// </summary>
			class GuiSolidLabelElement : public GuiElementBase<GuiSolidLabelElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiSolidLabelElement, L"SolidLabel");
			protected:
				Color					color;
				FontProperties			fontProperties;
				WString					text;
				Alignment				hAlignment;
				Alignment				vAlignment;
				bool					wrapLine;
				bool					ellipse;
				bool					multiline;
				bool					wrapLineHeightCalculation;

				GuiSolidLabelElement();
			public:
				/// <summary>
				/// Get the text color.
				/// </summary>
				/// <returns>The text color.</returns>
				Color					GetColor();
				/// <summary>
				/// Set the text color.
				/// </summary>
				/// <param name="value">The new text color.</param>
				void					SetColor(Color value);
				
				/// <summary>
				/// Get the text font.
				/// </summary>
				/// <returns>The text font.</returns>
				const FontProperties&	GetFont();
				/// <summary>
				/// Set the text font.
				/// </summary>
				/// <param name="value">The new text font.</param>
				void					SetFont(const FontProperties& value);
				
				/// <summary>
				/// Get the text.
				/// </summary>
				/// <returns>The text.</returns>
				const WString&			GetText();
				/// <summary>
				/// Set the text.
				/// </summary>
				/// <param name="value">The new text.</param>
				void					SetText(const WString& value);
				
				/// <summary>
				/// Get the horizontal alignment of the text.
				/// </summary>
				/// <returns>The horizontal alignment of the text.</returns>
				Alignment				GetHorizontalAlignment();
				/// <summary>
				/// Get the vertical alignment of the text.
				/// </summary>
				/// <returns>The vertical alignment of the text.</returns>
				Alignment				GetVerticalAlignment();
				/// <summary>
				/// Set the horizontal alignment of the text.
				/// </summary>
				/// <param name="value">The new horizontal alignment of the text.</param>
				void					SetHorizontalAlignment(Alignment value);
				/// <summary>
				/// Set the vertical alignment of the text.
				/// </summary>
				/// <param name="value">The vertical alignment of the text.</param>
				void					SetVerticalAlignment(Alignment value);
				/// <summary>
				/// Set alignments in both directions of the text.
				/// </summary>
				/// <param name="horizontal">The new horizontal alignment of the text.</param>
				/// <param name="vertical">The vertical alignment of the text.</param>
				void					SetAlignments(Alignment horizontal, Alignment vertical);
				
				/// <summary>
				/// Get if line auto-wrapping is enabled for this text.
				/// </summary>
				/// <returns>Return true if line auto-wrapping is enabled for this text.</returns>
				bool					GetWrapLine();
				/// <summary>
				/// Set if line auto-wrapping is enabled for this text.
				/// </summary>
				/// <param name="value">True if line auto-wrapping is enabled for this text.</param>
				void					SetWrapLine(bool value);
				
				/// <summary>
				/// Get if ellipse is enabled for this text. Ellipse will appear when the text is clipped.
				/// </summary>
				/// <returns>Return true if ellipse is enabled for this text.</returns>
				bool					GetEllipse();
				/// <summary>
				/// Set if ellipse is enabled for this text. Ellipse will appear when the text is clipped.
				/// </summary>
				/// <param name="value">True if ellipse is enabled for this text.</param>
				void					SetEllipse(bool value);
				
				/// <summary>
				/// Get if multiple lines is enabled for this text.
				/// </summary>
				/// <returns>Return true if multiple lines is enabled for this text.</returns>
				bool					GetMultiline();
				/// <summary>
				/// Set if multiple lines is enabled for this text.
				/// </summary>
				/// <param name="value">True if multiple lines is enabled for this text.</param>
				void					SetMultiline(bool value);

				/// <summary>
				/// Get if the element calculates the min height when wrap line is enabled.
				/// </summary>
				/// <returns>Return true if the element calculates the min height when wrap line is enabled.</returns>
				bool					GetWrapLineHeightCalculation();
				/// <summary>
				/// Set if the element calculates the min height when wrap line is enabled.
				/// </summary>
				/// <param name="value">True if the element calculates the min height when wrap line is enabled.</param>
				void					SetWrapLineHeightCalculation(bool value);
			};

			/// <summary>
			/// Defines an element containing an image.
			/// </summary>
			class GuiImageFrameElement : public GuiElementBase<GuiImageFrameElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiImageFrameElement, L"ImageFrame");
			protected:
				Ptr<INativeImage>		image;
				vint					frameIndex;
				Alignment				hAlignment;
				Alignment				vAlignment;
				bool					stretch;
				bool					enabled;

				GuiImageFrameElement();
			public:
				/// <summary>
				/// Get the containing image.
				/// </summary>
				/// <returns>The contining picture.</returns>
				Ptr<INativeImage>		GetImage();
				/// <summary>
				/// Get the index of the frame in the containing image.
				/// </summary>
				/// <returns>The index of the frame in the containing image</returns>
				vint					GetFrameIndex();
				/// <summary>
				/// Set the containing image.
				/// </summary>
				/// <param name="value">The new containing image.</param>
				void					SetImage(Ptr<INativeImage> value);
				/// <summary>
				/// Set  the frame index.
				/// </summary>
				/// <param name="value">The new frameIndex.</param>
				void					SetFrameIndex(vint value);
				/// <summary>
				/// Set the containing image and the frame index.
				/// </summary>
				/// <param name="_image">The new containing image.</param>
				/// <param name="_frameIndex">The new frameIndex.</param>
				void					SetImage(Ptr<INativeImage> _image, vint _frameIndex);
				
				/// <summary>
				/// Get the horizontal alignment of the image.
				/// </summary>
				/// <returns>The horizontal alignment of the image.</returns>
				Alignment				GetHorizontalAlignment();
				/// <summary>
				/// Get the vertical alignment of the image.
				/// </summary>
				/// <returns>The vertical alignment of the image.</returns>
				Alignment				GetVerticalAlignment();
				/// <summary>
				/// Set the horizontal alignment of the text.
				/// </summary>
				/// <param name="value">The new horizontal alignment of the text.</param>
				void					SetHorizontalAlignment(Alignment value);
				/// <summary>
				/// Set the vertical alignment of the text.
				/// </summary>
				/// <param name="value">The vertical alignment of the text.</param>
				void					SetVerticalAlignment(Alignment value);
				/// <summary>
				/// Set alignments in both directions of the image.
				/// </summary>
				/// <param name="horizontal">The new horizontal alignment of the image.</param>
				/// <param name="vertical">The vertical alignment of the image.</param>
				void					SetAlignments(Alignment horizontal, Alignment vertical);

				/// <summary>
				/// Get if stretching is enabled for this image.
				/// </summary>
				/// <returns>Return true if stretching is enabled for this image.</returns>
				bool					GetStretch();
				/// <summary>
				/// Set if stretching is enabled for this image.
				/// </summary>
				/// <param name="value">True if stretching is enabled for this image.</param>
				void					SetStretch(bool value);

				/// <summary>
				/// Get if the image is rendered as enabled.
				/// </summary>
				/// <returns>Return true if the image is rendered as enabled.</returns>
				bool					GetEnabled();
				/// <summary>
				/// Set if the image is rendered as enabled.
				/// </summary>
				/// <param name="value">True if the image is rendered as enabled.</param>
				void					SetEnabled(bool value);
			};

			/// <summary>
			/// Defines a polygon element with a thickness of one pixel.
			/// </summary>
			class GuiPolygonElement : public GuiElementBase<GuiPolygonElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiPolygonElement, L"Polygon");

				typedef collections::Array<Point>			PointArray;
			protected:
				Size							size;
				PointArray						points;
				Color							borderColor;
				Color							backgroundColor;

				GuiPolygonElement();
			public:
				/// <summary>
				/// Get a suggested size. The polygon element will be layouted to the center of the required bounds using this size.
				/// </summary>
				/// <returns>The suggested size.</returns>
				Size					GetSize();
				/// <summary>
				/// Set a suggested size. The polygon element will be layouted to the center of the required bounds using this size.
				/// </summary>
				/// <param name="value">The new size.</param>
				void					SetSize(Size value);

				/// <summary>
				/// Get a point of the polygon element using an index.
				/// </summary>
				/// <param name="index">The index to access a point.</param>
				/// <returns>The point of the polygon element associated with the index.</returns>
				const Point&			GetPoint(vint index);
				/// <summary>
				/// Get the number of points
				/// </summary>
				/// <returns>The number of points.</returns>
				vint					GetPointCount();
				/// <summary>
				/// Set all points to the polygon element.
				/// </summary>
				/// <param name="p">A pointer to a buffer that stores all points.</param>
				/// <param name="count">The number of points.</param>
				void					SetPoints(const Point* p, vint count);


				/// <summary>
				/// Get all points.
				/// </summary>
				/// <returns>All points</returns>
				const PointArray&		GetPointsArray();
				/// <summary>
				/// Set all points.
				/// </summary>
				/// <param name="value">All points</param>
				void					SetPointsArray(const PointArray& value);
				
				/// <summary>
				/// Get the border color.
				/// </summary>
				/// <returns>The border color.</returns>
				Color					GetBorderColor();
				/// <summary>
				/// Set the border color.
				/// </summary>
				/// <param name="value">The new border color.</param>
				void					SetBorderColor(Color value);
				/// <summary>
				/// Get the background color.
				/// </summary>
				/// <returns>The background color.</returns>
				Color					GetBackgroundColor();
				/// <summary>
				/// Set the background color.
				/// </summary>
				/// <param name="value">The new background color.</param>
				void					SetBackgroundColor(Color value);
			};
		}
	}
}

#endif