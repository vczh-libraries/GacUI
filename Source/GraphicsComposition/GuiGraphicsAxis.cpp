#include "GuiGraphicsAxis.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiDefaultAxis
***********************************************************************/

			GuiDefaultAxis::GuiDefaultAxis()
			{
			}

			GuiDefaultAxis::~GuiDefaultAxis()
			{
			}

			Size GuiDefaultAxis::RealSizeToVirtualSize(Size size)
			{
				return size;
			}

			Size GuiDefaultAxis::VirtualSizeToRealSize(Size size)
			{
				return size;
			}

			Point GuiDefaultAxis::RealPointToVirtualPoint(Size realFullSize, Point point)
			{
				return point;
			}

			Point GuiDefaultAxis::VirtualPointToRealPoint(Size realFullSize, Point point)
			{
				return point;
			}

			Rect GuiDefaultAxis::RealRectToVirtualRect(Size realFullSize, Rect rect)
			{
				return rect;
			}

			Rect GuiDefaultAxis::VirtualRectToRealRect(Size realFullSize, Rect rect)
			{
				return rect;
			}

			Margin GuiDefaultAxis::RealMarginToVirtualMargin(Margin margin)
			{
				return margin;
			}

			Margin GuiDefaultAxis::VirtualMarginToRealMargin(Margin margin)
			{
				return margin;
			}

			KeyDirection GuiDefaultAxis::RealKeyDirectionToVirtualKeyDirection(KeyDirection key)
			{
				return key;
			}

/***********************************************************************
GuiAxis
***********************************************************************/

			GuiAxis::GuiAxis(AxisDirection _axisDirection)
				:axisDirection(_axisDirection)
			{
			}

			GuiAxis::~GuiAxis()
			{
			}

			AxisDirection GuiAxis::GetAlignment()
			{
				return axisDirection;
			}

			Size GuiAxis::RealSizeToVirtualSize(Size size)
			{
				switch(axisDirection)
				{
				case AxisDirection::LeftDown:
				case AxisDirection::RightDown:
				case AxisDirection::LeftUp:
				case AxisDirection::RightUp:
					return Size(size.x, size.y);
				case AxisDirection::DownLeft:
				case AxisDirection::DownRight:
				case AxisDirection::UpLeft:
				case AxisDirection::UpRight:
					return Size(size.y, size.x);
				}
				return size;
			}

			Size GuiAxis::VirtualSizeToRealSize(Size size)
			{
				return RealSizeToVirtualSize(size);
			}

			Point GuiAxis::RealPointToVirtualPoint(Size realFullSize, Point point)
			{
				Rect rect(point, Size(0, 0));
				return RealRectToVirtualRect(realFullSize, rect).LeftTop();
			}

			Point GuiAxis::VirtualPointToRealPoint(Size realFullSize, Point point)
			{
				Rect rect(point, Size(0, 0));
				return VirtualRectToRealRect(realFullSize, rect).LeftTop();
			}

			Rect GuiAxis::RealRectToVirtualRect(Size realFullSize, Rect rect)
			{
				vint x1=rect.x1;
				vint x2=realFullSize.x-rect.x2;
				vint y1=rect.y1;
				vint y2=realFullSize.y-rect.y2;
				vint w=rect.Width();
				vint h=rect.Height();
				switch(axisDirection)
				{
				case AxisDirection::LeftDown:
					return Rect(Point(x2, y1), Size(w, h));
				case AxisDirection::RightDown:
					return Rect(Point(x1, y1), Size(w, h));
				case AxisDirection::LeftUp:
					return Rect(Point(x2, y2), Size(w, h));
				case AxisDirection::RightUp:
					return Rect(Point(x1, y2), Size(w, h));
				case AxisDirection::DownLeft:
					return Rect(Point(y1, x2), Size(h, w));
				case AxisDirection::DownRight:
					return Rect(Point(y1, x1), Size(h, w));
				case AxisDirection::UpLeft:
					return Rect(Point(y2, x2), Size(h, w));
				case AxisDirection::UpRight:
					return Rect(Point(y2, x1), Size(h, w));
				}
				return rect;
			}

			Rect GuiAxis::VirtualRectToRealRect(Size realFullSize, Rect rect)
			{
				realFullSize=RealSizeToVirtualSize(realFullSize);
				vint x1=rect.x1;
				vint x2=realFullSize.x-rect.x2;
				vint y1=rect.y1;
				vint y2=realFullSize.y-rect.y2;
				vint w=rect.Width();
				vint h=rect.Height();
				switch(axisDirection)
				{
				case AxisDirection::LeftDown:
					return Rect(Point(x2, y1), Size(w, h));
				case AxisDirection::RightDown:
					return Rect(Point(x1, y1), Size(w, h));
				case AxisDirection::LeftUp:
					return Rect(Point(x2, y2), Size(w, h));
				case AxisDirection::RightUp:
					return Rect(Point(x1, y2), Size(w, h));
				case AxisDirection::DownLeft:
					return Rect(Point(y2, x1), Size(h, w));
				case AxisDirection::DownRight:
					return Rect(Point(y1, x1), Size(h, w));
				case AxisDirection::UpLeft:
					return Rect(Point(y2, x2), Size(h, w));
				case AxisDirection::UpRight:
					return Rect(Point(y1, x2), Size(h, w));
				}
				return rect;
			}

			Margin GuiAxis::RealMarginToVirtualMargin(Margin margin)
			{
				vint x1=margin.left;
				vint x2=margin.right;
				vint y1=margin.top;
				vint y2=margin.bottom;
				switch(axisDirection)
				{
				case AxisDirection::LeftDown:
					return Margin(x2, y1, x1, y2);
				case AxisDirection::RightDown:
					return Margin(x1, y1, x2, y2);
				case AxisDirection::LeftUp:
					return Margin(x2, y2, x1, y1);
				case AxisDirection::RightUp:
					return Margin(x1, y2, x2, y1);
				case AxisDirection::DownLeft:
					return Margin(y1, x2, y2, x1);
				case AxisDirection::DownRight:
					return Margin(y1, x1, y2, x2);
				case AxisDirection::UpLeft:
					return Margin(y2, x2, y1, x1);
				case AxisDirection::UpRight:
					return Margin(y2, x1, y1, x2);
				}
				return margin;
			}

			Margin GuiAxis::VirtualMarginToRealMargin(Margin margin)
			{
				vint x1=margin.left;
				vint x2=margin.right;
				vint y1=margin.top;
				vint y2=margin.bottom;
				switch(axisDirection)
				{
				case AxisDirection::LeftDown:
					return Margin(x2, y1, x1, y2);
				case AxisDirection::RightDown:
					return Margin(x1, y1, x2, y2);
				case AxisDirection::LeftUp:
					return Margin(x2, y2, x1, y1);
				case AxisDirection::RightUp:
					return Margin(x1, y2, x2, y1);
				case AxisDirection::DownLeft:
					return Margin(y2, x1, y1, x2);
				case AxisDirection::DownRight:
					return Margin(y1, x1, y2, x2);
				case AxisDirection::UpLeft:
					return Margin(y2, x2, y1, x1);
				case AxisDirection::UpRight:
					return Margin(y1, x2, y2, x1);
				default:;
				}
				return margin;
			}

			KeyDirection GuiAxis::RealKeyDirectionToVirtualKeyDirection(KeyDirection key)
			{
				bool pageKey=false;
				switch(key)
				{
				case KeyDirection::PageUp:
					pageKey=true;
					key=KeyDirection::Up;
					break;
				case KeyDirection::PageDown:
					pageKey=true;
					key=KeyDirection::Down;
					break;
				case KeyDirection::PageLeft:
					pageKey=true;
					key=KeyDirection::Left;
					break;
				case KeyDirection::PageRight:
					pageKey=true;
					key=KeyDirection::Right;
					break;
				default:;
				}

				switch(key)
				{
				case KeyDirection::Up:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::Up;		break;
					case AxisDirection::RightDown:	key=KeyDirection::Up;		break;
					case AxisDirection::LeftUp:		key=KeyDirection::Down;	break;
					case AxisDirection::RightUp:	key=KeyDirection::Down;	break;
					case AxisDirection::DownLeft:	key=KeyDirection::Left;	break;
					case AxisDirection::DownRight:	key=KeyDirection::Left;	break;
					case AxisDirection::UpLeft:		key=KeyDirection::Right;	break;
					case AxisDirection::UpRight:	key=KeyDirection::Right;	break;
					}
					break;
				case KeyDirection::Down:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::Down;	break;
					case AxisDirection::RightDown:	key=KeyDirection::Down;	break;
					case AxisDirection::LeftUp:		key=KeyDirection::Up;		break;
					case AxisDirection::RightUp:	key=KeyDirection::Up;		break;
					case AxisDirection::DownLeft:	key=KeyDirection::Right;	break;
					case AxisDirection::DownRight:	key=KeyDirection::Right;	break;
					case AxisDirection::UpLeft:		key=KeyDirection::Left;	break;
					case AxisDirection::UpRight:	key=KeyDirection::Left;	break;
					}
					break;
				case KeyDirection::Left:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::Right;	break;
					case AxisDirection::RightDown:	key=KeyDirection::Left;	break;
					case AxisDirection::LeftUp:		key=KeyDirection::Right;	break;
					case AxisDirection::RightUp:	key=KeyDirection::Left;	break;
					case AxisDirection::DownLeft:	key=KeyDirection::Down;	break;
					case AxisDirection::DownRight:	key=KeyDirection::Up;		break;
					case AxisDirection::UpLeft:		key=KeyDirection::Down;	break;
					case AxisDirection::UpRight:	key=KeyDirection::Up;		break;
					}
					break;
				case KeyDirection::Right:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::Left;	break;
					case AxisDirection::RightDown:	key=KeyDirection::Right;	break;
					case AxisDirection::LeftUp:		key=KeyDirection::Left;	break;
					case AxisDirection::RightUp:	key=KeyDirection::Right;	break;
					case AxisDirection::DownLeft:	key=KeyDirection::Up;		break;
					case AxisDirection::DownRight:	key=KeyDirection::Down;	break;
					case AxisDirection::UpLeft:		key=KeyDirection::Up;		break;
					case AxisDirection::UpRight:	key=KeyDirection::Down;	break;
					}
					break;
				case KeyDirection::Home:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::Home;	break;
					case AxisDirection::RightDown:	key=KeyDirection::Home;	break;
					case AxisDirection::LeftUp:		key=KeyDirection::End;	break;
					case AxisDirection::RightUp:	key=KeyDirection::End;	break;
					case AxisDirection::DownLeft:	key=KeyDirection::Home;	break;
					case AxisDirection::DownRight:	key=KeyDirection::Home;	break;
					case AxisDirection::UpLeft:		key=KeyDirection::End;	break;
					case AxisDirection::UpRight:	key=KeyDirection::End;	break;
					}
					break;
				case KeyDirection::End:
					switch(axisDirection)
					{
					case AxisDirection::LeftDown:	key=KeyDirection::End;	break;
					case AxisDirection::RightDown:	key=KeyDirection::End;	break;
					case AxisDirection::LeftUp:		key=KeyDirection::Home;	break;
					case AxisDirection::RightUp:	key=KeyDirection::Home;	break;
					case AxisDirection::DownLeft:	key=KeyDirection::End;	break;
					case AxisDirection::DownRight:	key=KeyDirection::End;	break;
					case AxisDirection::UpLeft:		key=KeyDirection::Home;	break;
					case AxisDirection::UpRight:	key=KeyDirection::Home;	break;
					}
					break;
				default:;
				}

				if(pageKey)
				{
					switch(key)
					{
					case KeyDirection::Up:
						key=KeyDirection::PageUp;
						break;
					case KeyDirection::Down:
						key=KeyDirection::PageDown;
						break;
					case KeyDirection::Left:
						key=KeyDirection::PageLeft;
						break;
					case KeyDirection::Right:
						key=KeyDirection::PageRight;
						break;
					default:;
					}
				}
				return key;
			}
		}
	}
}