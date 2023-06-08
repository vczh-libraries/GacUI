#include "GuiGraphicsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiGraphicsComposition
***********************************************************************/

			Size GuiGraphicsComposition::Layout_CalculateMinSize()
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			Size GuiGraphicsComposition::Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin)
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			Rect GuiGraphicsComposition::Layout_CalculateBounds(Rect parentBounds)
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void GuiGraphicsComposition::Layout_SetCachedMinSize(Size value)
			{
				if (cachedMinSize != value)
				{
					cachedMinSize = value;
					CachedMinSizeChanged.Execute(GuiEventArgs(this));
				}
			}

			void GuiGraphicsComposition::Layout_SetCachedBounds(Rect value)
			{
				if (cachedBounds != value)
				{
					cachedBounds = value;
					CachedBoundsChanged.Execute(GuiEventArgs(this));
				}
			}

			void GuiGraphicsComposition::Layout_UpdateMinSize()
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void GuiGraphicsComposition::Layout_UpdateBounds(Rect parentBounds)
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			Size GuiGraphicsComposition::GetCachedMinSize()
			{
				return cachedMinSize;
			}

			Size GuiGraphicsComposition::GetCachedMinClientSize()
			{
				auto minSize = cachedMinSize;
				minSize.x += internalMargin.left + internalMargin.right;
				minSize.y += internalMargin.top + internalMargin.bottom;
				return minSize;
			}

			Rect GuiGraphicsComposition::GetCachedBounds()
			{
				return cachedBounds;
			}

			Rect GuiGraphicsComposition::GetCachedClientArea()
			{
				Rect bounds = cachedBounds;
				bounds.x1 += internalMargin.left;
				bounds.y1 += internalMargin.top;
				bounds.x2 -= internalMargin.right;
				bounds.y2 -= internalMargin.bottom;
				return bounds;
			}
		}
	}
}