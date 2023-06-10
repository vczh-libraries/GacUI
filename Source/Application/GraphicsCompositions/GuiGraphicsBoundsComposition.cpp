#include "GuiGraphicsBoundsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace elements;

/***********************************************************************
GuiBoundsComposition
***********************************************************************/

			Size GuiBoundsComposition::Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin)
			{
				vint offsetW = 0;
				vint offsetH = 0;

				if (alignmentToParent.left >= 0)
				{
					offsetW += alignmentToParent.left - parentInternalMargin.left;
				}
				if (alignmentToParent.right >= 0)
				{
					offsetW += alignmentToParent.right - parentInternalMargin.right;
				}
				if (alignmentToParent.top >= 0)
				{
					offsetH += alignmentToParent.top - parentInternalMargin.top;
				}
				if (alignmentToParent.bottom >= 0)
				{
					offsetH += alignmentToParent.bottom - parentInternalMargin.bottom;
				}

				return { cachedMinSize.x + offsetW,cachedMinSize.y + offsetH };
			}

			Rect GuiBoundsComposition::Layout_CalculateBounds(Rect parentBounds)
			{
				Rect result = expectedBounds;
				if (result.Width() < cachedMinSize.x) result.x2 = result.x1 + cachedMinSize.x;
				if (result.Height() < cachedMinSize.y) result.y2 = result.y1 + cachedMinSize.y;

				if (auto parent = GetParent())
				{
					Margin clientMargin = GetParent()->GetInternalMargin();
					result.x1 += clientMargin.left;
					result.x2 += clientMargin.left;
					result.y1 += clientMargin.top;
					result.y2 += clientMargin.top;

					Size clientSize = GetParent()->GetCachedClientArea().GetSize();
					if (alignmentToParent.left >= 0 && alignmentToParent.right >= 0)
					{
						result.x1 = alignmentToParent.left;
						result.x2 = clientSize.x - alignmentToParent.right;
					}
					else if (alignmentToParent.left >= 0)
					{
						vint width = result.Width();
						result.x1 = alignmentToParent.left;
						result.x2 = result.x1 + width;
					}
					else if (alignmentToParent.right >= 0)
					{
						vint width = result.Width();
						result.x2 = clientSize.x - alignmentToParent.right;
						result.x1 = result.x2 - width;
					}

					if (alignmentToParent.top >= 0 && alignmentToParent.bottom >= 0)
					{
						result.y1 = alignmentToParent.top;
						result.y2 = clientSize.y - alignmentToParent.bottom;
					}
					else if (alignmentToParent.top >= 0)
					{
						vint height = result.Height();
						result.y1 = alignmentToParent.top;
						result.y2 = result.y1 + height;
					}
					else if (alignmentToParent.bottom >= 0)
					{
						vint height = result.Height();
						result.y2 = clientSize.y - alignmentToParent.bottom;
						result.y1 = result.y2 - height;
					}
				}

				return result;
			}

			Rect GuiBoundsComposition::GetExpectedBounds()
			{
				return expectedBounds;
			}

			void GuiBoundsComposition::SetExpectedBounds(Rect value)
			{
				if (expectedBounds != value)
				{
					expectedBounds = value;
					InvokeOnCompositionStateChanged();
				}
			}

			Margin GuiBoundsComposition::GetAlignmentToParent()
			{
				return alignmentToParent;
			}

			void GuiBoundsComposition::SetAlignmentToParent(Margin value)
			{
				if (alignmentToParent != value)
				{
					alignmentToParent = value;
					InvokeOnCompositionStateChanged();
				}
			}

			bool GuiBoundsComposition::IsAlignedToParent()
			{
				return alignmentToParent != Margin(-1, -1, -1, -1);
			}
		}
	}
}