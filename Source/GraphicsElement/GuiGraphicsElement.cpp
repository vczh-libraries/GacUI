#include "GuiGraphicsElement.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			using namespace collections;

/***********************************************************************
GuiFocusRectangleElement
***********************************************************************/

			GuiFocusRectangleElement::GuiFocusRectangleElement()
			{
			}

/***********************************************************************
GuiSolidBorderElement
***********************************************************************/

			GuiSolidBorderElement::GuiSolidBorderElement()
				:color(0, 0, 0)
			{
			}

			Color GuiSolidBorderElement::GetColor()
			{
				return color;
			}

			void GuiSolidBorderElement::SetColor(Color value)
			{
				if(color!=value)
				{
					color=value;
					InvokeOnElementStateChanged();
				}
			}
			
			ElementShape GuiSolidBorderElement::GetShape()
			{
				return shape;
			}

			void GuiSolidBorderElement::SetShape(ElementShape value)
			{
				shape=value;
			}

/***********************************************************************
Gui3DBorderElement
***********************************************************************/

			Gui3DBorderElement::Gui3DBorderElement()
			{
			}

			Color Gui3DBorderElement::GetColor1()
			{
				return color1;
			}

			void Gui3DBorderElement::SetColor1(Color value)
			{
				SetColors(value, color2);
			}

			Color Gui3DBorderElement::GetColor2()
			{
				return color2;
			}

			void Gui3DBorderElement::SetColor2(Color value)
			{
				SetColors(color1, value);
			}

			void Gui3DBorderElement::SetColors(Color value1, Color value2)
			{
				if(color1!=value1 || color2!=value2)
				{
					color1=value1;
					color2=value2;
					InvokeOnElementStateChanged();
				}
			}

/***********************************************************************
Gui3DSplitterElement
***********************************************************************/

			Gui3DSplitterElement::Gui3DSplitterElement()
				:direction(Horizontal)
			{
			}

			Color Gui3DSplitterElement::GetColor1()
			{
				return color1;
			}

			void Gui3DSplitterElement::SetColor1(Color value)
			{
				SetColors(value, color2);
			}

			Color Gui3DSplitterElement::GetColor2()
			{
				return color2;
			}

			void Gui3DSplitterElement::SetColor2(Color value)
			{
				SetColors(color1, value);
			}

			void Gui3DSplitterElement::SetColors(Color value1, Color value2)
			{
				if(color1!=value1 || color2!=value2)
				{
					color1=value1;
					color2=value2;
					InvokeOnElementStateChanged();
				}
			}

			Gui3DSplitterElement::Direction Gui3DSplitterElement::GetDirection()
			{
				return direction;
			}

			void Gui3DSplitterElement::SetDirection(Direction value)
			{
				if(direction!=value)
				{
					direction=value;
					InvokeOnElementStateChanged();
				}
			}

/***********************************************************************
GuiSolidBackgroundElement
***********************************************************************/

			GuiSolidBackgroundElement::GuiSolidBackgroundElement()
				:color(255, 255, 255)
			{
			}

			Color GuiSolidBackgroundElement::GetColor()
			{
				return color;
			}

			void GuiSolidBackgroundElement::SetColor(Color value)
			{
				if(color!=value)
				{
					color=value;
					InvokeOnElementStateChanged();
				}
			}
			
			ElementShape GuiSolidBackgroundElement::GetShape()
			{
				return shape;
			}

			void GuiSolidBackgroundElement::SetShape(ElementShape value)
			{
				shape=value;
			}

/***********************************************************************
GuiGradientBackgroundElement
***********************************************************************/

			GuiGradientBackgroundElement::GuiGradientBackgroundElement()
				:direction(Horizontal)
			{
			}

			Color GuiGradientBackgroundElement::GetColor1()
			{
				return color1;
			}

			void GuiGradientBackgroundElement::SetColor1(Color value)
			{
				SetColors(value, color2);
			}

			Color GuiGradientBackgroundElement::GetColor2()
			{
				return color2;
			}

			void GuiGradientBackgroundElement::SetColor2(Color value)
			{
				SetColors(color1, value);
			}

			void GuiGradientBackgroundElement::SetColors(Color value1, Color value2)
			{
				if(color1!=value1 || color2!=value2)
				{
					color1=value1;
					color2=value2;
					InvokeOnElementStateChanged();
				}
			}

			GuiGradientBackgroundElement::Direction GuiGradientBackgroundElement::GetDirection()
			{
				return direction;
			}

			void GuiGradientBackgroundElement::SetDirection(Direction value)
			{
				if(direction!=value)
				{
					direction=value;
					InvokeOnElementStateChanged();
				}
			}
			
			ElementShape GuiGradientBackgroundElement::GetShape()
			{
				return shape;
			}

			void GuiGradientBackgroundElement::SetShape(ElementShape value)
			{
				shape=value;
			}

/***********************************************************************
GuiInnerShadowElement
***********************************************************************/

			GuiInnerShadowElement::GuiInnerShadowElement()
			{
			}

			Color GuiInnerShadowElement::GetColor()
			{
				return color;
			}

			void GuiInnerShadowElement::SetColor(Color value)
			{
				if (color != value)
				{
					color = value;
					InvokeOnElementStateChanged();
				}
			}

			vint GuiInnerShadowElement::GetThickness()
			{
				return thickness;
			}

			void GuiInnerShadowElement::SetThickness(vint value)
			{
				if (thickness != value)
				{
					thickness = value;
					InvokeOnElementStateChanged();
				}
			}

/***********************************************************************
GuiSolidLabelElement
***********************************************************************/

			GuiSolidLabelElement::GuiSolidLabelElement()
				:color(0, 0, 0)
				,hAlignment(Alignment::Left)
				,vAlignment(Alignment::Top)
				,wrapLine(false)
				,ellipse(false)
				,multiline(false)
				,wrapLineHeightCalculation(false)
			{
				fontProperties.fontFamily=L"Lucida Console";
				fontProperties.size=12;
			}

			Color GuiSolidLabelElement::GetColor()
			{
				return color;
			}

			void GuiSolidLabelElement::SetColor(Color value)
			{
				if(color!=value)
				{
					color=value;
					InvokeOnElementStateChanged();
				}
			}

			const FontProperties& GuiSolidLabelElement::GetFont()
			{
				return fontProperties;
			}

			void GuiSolidLabelElement::SetFont(const FontProperties& value)
			{
				if(fontProperties!=value)
				{
					fontProperties=value;
					InvokeOnElementStateChanged();
				}
			}

			const WString& GuiSolidLabelElement::GetText()
			{
				return text;
			}

			void GuiSolidLabelElement::SetText(const WString& value)
			{
				if(text!=value)
				{
					text=value;
					InvokeOnElementStateChanged();
				}
			}

			Alignment GuiSolidLabelElement::GetHorizontalAlignment()
			{
				return hAlignment;
			}

			Alignment GuiSolidLabelElement::GetVerticalAlignment()
			{
				return vAlignment;
			}

			void GuiSolidLabelElement::SetHorizontalAlignment(Alignment value)
			{
				SetAlignments(value, vAlignment);
			}

			void GuiSolidLabelElement::SetVerticalAlignment(Alignment value)
			{
				SetAlignments(hAlignment, value);
			}

			void GuiSolidLabelElement::SetAlignments(Alignment horizontal, Alignment vertical)
			{
				if(hAlignment!=horizontal || vAlignment!=vertical)
				{
					hAlignment=horizontal;
					vAlignment=vertical;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiSolidLabelElement::GetWrapLine()
			{
				return wrapLine;
			}

			void GuiSolidLabelElement::SetWrapLine(bool value)
			{
				if(wrapLine!=value)
				{
					wrapLine=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiSolidLabelElement::GetEllipse()
			{
				return ellipse;
			}

			void GuiSolidLabelElement::SetEllipse(bool value)
			{
				if(ellipse!=value)
				{
					ellipse=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiSolidLabelElement::GetMultiline()
			{
				return multiline;
			}

			void GuiSolidLabelElement::SetMultiline(bool value)
			{
				if(multiline!=value)
				{
					multiline=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiSolidLabelElement::GetWrapLineHeightCalculation()
			{
				return wrapLineHeightCalculation;
			}

			void GuiSolidLabelElement::SetWrapLineHeightCalculation(bool value)
			{
				if(wrapLineHeightCalculation!=value)
				{
					wrapLineHeightCalculation=value;
					InvokeOnElementStateChanged();
				}
			}

/***********************************************************************
GuiImageFrameElement
***********************************************************************/

			GuiImageFrameElement::GuiImageFrameElement()
				:frameIndex(0)
				,hAlignment(Alignment::Left)
				,vAlignment(Alignment::Top)
				,stretch(false)
				,enabled(true)
			{
			}

			Ptr<INativeImage> GuiImageFrameElement::GetImage()
			{
				return image;
			}

			vint GuiImageFrameElement::GetFrameIndex()
			{
				return frameIndex;
			}

			void GuiImageFrameElement::SetImage(Ptr<INativeImage> value)
			{
				SetImage(value, frameIndex);
			}

			void GuiImageFrameElement::SetFrameIndex(vint value)
			{
				SetImage(image, value);
			}

			void GuiImageFrameElement::SetImage(Ptr<INativeImage> _image, vint _frameIndex)
			{
				if(image!=_image || frameIndex!=_frameIndex)
				{
					if(!_image)
					{
						image=0;
						frameIndex=0;
					}
					else if(0<=_frameIndex && _frameIndex<_image->GetFrameCount())
					{
						image=_image;
						frameIndex=_frameIndex;
					}
					InvokeOnElementStateChanged();
				}
			}

			Alignment GuiImageFrameElement::GetHorizontalAlignment()
			{
				return hAlignment;
			}

			Alignment GuiImageFrameElement::GetVerticalAlignment()
			{
				return vAlignment;
			}

			void GuiImageFrameElement::SetHorizontalAlignment(Alignment value)
			{
				SetAlignments(value, vAlignment);
			}

			void GuiImageFrameElement::SetVerticalAlignment(Alignment value)
			{
				SetAlignments(hAlignment, value);
			}

			void GuiImageFrameElement::SetAlignments(Alignment horizontal, Alignment vertical)
			{
				if(hAlignment!=horizontal || vAlignment!=vertical)
				{
					hAlignment=horizontal;
					vAlignment=vertical;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiImageFrameElement::GetStretch()
			{
				return stretch;
			}

			void GuiImageFrameElement::SetStretch(bool value)
			{
				if(stretch!=value)
				{
					stretch=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiImageFrameElement::GetEnabled()
			{
				return enabled;
			}

			void GuiImageFrameElement::SetEnabled(bool value)
			{
				if(enabled!=value)
				{
					enabled=value;
					InvokeOnElementStateChanged();
				}
			}

/***********************************************************************
GuiPolygonElement
***********************************************************************/

			GuiPolygonElement::GuiPolygonElement()
			{
			}

			Size GuiPolygonElement::GetSize()
			{
				return size;
			}

			void GuiPolygonElement::SetSize(Size value)
			{
				if(size!=value)
				{
					size=value;
					InvokeOnElementStateChanged();
				}
			}

			const Point& GuiPolygonElement::GetPoint(vint index)
			{
				return points[index];
			}

			vint GuiPolygonElement::GetPointCount()
			{
				return points.Count();
			}

			void GuiPolygonElement::SetPoints(const Point* p, vint count)
			{
				points.Resize(count);
				if(count>0)
				{
					memcpy(&points[0], p, sizeof(*p)*count);
				}
				InvokeOnElementStateChanged();
			}

			const GuiPolygonElement::PointArray& GuiPolygonElement::GetPointsArray()
			{
				return points;
			}

			void GuiPolygonElement::SetPointsArray(const PointArray& value)
			{
				CopyFrom(points, value);
				InvokeOnElementStateChanged();
			}

			Color GuiPolygonElement::GetBorderColor()
			{
				return borderColor;
			}

			void GuiPolygonElement::SetBorderColor(Color value)
			{
				if(borderColor!=value)
				{
					borderColor=value;
					InvokeOnElementStateChanged();
				}
			}

			Color GuiPolygonElement::GetBackgroundColor()
			{
				return backgroundColor;
			}

			void GuiPolygonElement::SetBackgroundColor(Color value)
			{
				if(backgroundColor!=value)
				{
					backgroundColor=value;
					InvokeOnElementStateChanged();
				}
			}
		}
	}
}