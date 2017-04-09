#include "GuiGraphicsElement.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			using namespace collections;

/***********************************************************************
GuiSolidBorderElement
***********************************************************************/

			GuiSolidBorderElement::GuiSolidBorderElement()
				:color(0, 0, 0)
				,shape(ElementShape::Rectangle)
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
GuiRoundBorderElement
***********************************************************************/

			GuiRoundBorderElement::GuiRoundBorderElement()
				:color(0, 0, 0)
				,radius(10)
			{
			}

			Color GuiRoundBorderElement::GetColor()
			{
				return color;
			}

			void GuiRoundBorderElement::SetColor(Color value)
			{
				if(color!=value)
				{
					color=value;
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
				}
			}

			vint GuiRoundBorderElement::GetRadius()
			{
				return radius;
			}

			void GuiRoundBorderElement::SetRadius(vint value)
			{
				if(radius!=value)
				{
					radius=value;
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
				}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
				}
			}

/***********************************************************************
GuiSolidBackgroundElement
***********************************************************************/

			GuiSolidBackgroundElement::GuiSolidBackgroundElement()
				:color(255, 255, 255)
				,shape(ElementShape::Rectangle)
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
				,shape(ElementShape::Rectangle)
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
				if(renderer)
				{
					renderer->OnElementStateChanged();
				}
			}

			const GuiPolygonElement::PointArray& GuiPolygonElement::GetPointsArray()
			{
				return points;
			}

			void GuiPolygonElement::SetPointsArray(const PointArray& value)
			{
				CopyFrom(points, value);
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
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
					if(renderer)
					{
						renderer->OnElementStateChanged();
					}
				}
			}
		}
	}
}