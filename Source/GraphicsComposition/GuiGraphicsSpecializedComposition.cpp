#include "GuiGraphicsSpecializedComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiSideAlignedComposition
***********************************************************************/

			GuiSideAlignedComposition::GuiSideAlignedComposition()
				:direction(Top)
				,maxLength(10)
				,maxRatio(1.0)
			{
			}

			GuiSideAlignedComposition::~GuiSideAlignedComposition()
			{
			}

			GuiSideAlignedComposition::Direction GuiSideAlignedComposition::GetDirection()
			{
				return direction;
			}

			void GuiSideAlignedComposition::SetDirection(Direction value)
			{
				direction=value;
			}

			vint GuiSideAlignedComposition::GetMaxLength()
			{
				return maxLength;
			}

			void GuiSideAlignedComposition::SetMaxLength(vint value)
			{
				if(value<0) value=0;
				maxLength=value;
			}

			double GuiSideAlignedComposition::GetMaxRatio()
			{
				return maxRatio;
			}

			void GuiSideAlignedComposition::SetMaxRatio(double value)
			{
				maxRatio=
					value<0?0:
					value>1?1:
					value;
			}

			bool GuiSideAlignedComposition::IsSizeAffectParent()
			{
				return false;
			}

			Rect GuiSideAlignedComposition::GetBounds()
			{
				Rect result;
				GuiGraphicsComposition* parent=GetParent();
				if(parent)
				{
					Rect bounds=parent->GetBounds();
					vint w=(vint)(bounds.Width()*maxRatio);
					vint h=(vint)(bounds.Height()*maxRatio);
					if(w>maxLength) w=maxLength;
					if(h>maxLength) h=maxLength;
					switch(direction)
					{
					case Left:
						{
							bounds.x2=bounds.x1+w;
						}
						break;
					case Top:
						{
							bounds.y2=bounds.y1+h;
						}
						break;
					case Right:
						{
							bounds.x1=bounds.x2-w;
						}
						break;
					case Bottom:
						{
							bounds.y1=bounds.y2-h;
						}
						break;
					}
					result = bounds;
				}
				UpdatePreviousBounds(result);
				return result;
			}

/***********************************************************************
GuiPartialViewComposition
***********************************************************************/

			GuiPartialViewComposition::GuiPartialViewComposition()
				:wRatio(0.0)
				,wPageSize(1.0)
				,hRatio(0.0)
				,hPageSize(1.0)
			{
			}

			GuiPartialViewComposition::~GuiPartialViewComposition()
			{
			}

			double GuiPartialViewComposition::GetWidthRatio()
			{
				return wRatio;
			}

			double GuiPartialViewComposition::GetWidthPageSize()
			{
				return wPageSize;
			}

			double GuiPartialViewComposition::GetHeightRatio()
			{
				return hRatio;
			}

			double GuiPartialViewComposition::GetHeightPageSize()
			{
				return hPageSize;
			}

			void GuiPartialViewComposition::SetWidthRatio(double value)
			{
				wRatio=value;
			}

			void GuiPartialViewComposition::SetWidthPageSize(double value)
			{
				wPageSize=value;
			}

			void GuiPartialViewComposition::SetHeightRatio(double value)
			{
				hRatio=value;
			}

			void GuiPartialViewComposition::SetHeightPageSize(double value)
			{
				hPageSize=value;
			}

			bool GuiPartialViewComposition::IsSizeAffectParent()
			{
				return false;
			}

			Rect GuiPartialViewComposition::GetBounds()
			{
				Rect result;
				GuiGraphicsComposition* parent=GetParent();
				if(parent)
				{
					Rect bounds=parent->GetBounds();
					vint w=bounds.Width();
					vint h=bounds.Height();
					vint pw=(vint)(wPageSize*w);
					vint ph=(vint)(hPageSize*h);

					vint ow=preferredMinSize.x-pw;
					if(ow<0) ow=0;
					vint oh=preferredMinSize.y-ph;
					if(oh<0) oh=0;

					w-=ow;
					h-=oh;
					pw+=ow;
					ph+=oh;

					result = Rect(Point((vint)(wRatio*w), (vint)(hRatio*h)), Size(pw, ph));
				}
				UpdatePreviousBounds(result);
				return result;
			}
		}
	}
}
