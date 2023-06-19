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

			Size GuiBoundsComposition::Layout_CalculateMinSize()
			{
				Size minSize = Layout_CalculateMinSizeHelper();
				if (minSize.x < expectedBounds.Width()) minSize.x = expectedBounds.Width();
				if (minSize.y < expectedBounds.Height()) minSize.y = expectedBounds.Height();
				return minSize;
			}

			Size GuiBoundsComposition::Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin)
			{
				vint offsetW = 0;
				vint offsetH = 0;

				if (alignmentToParent.left == -1 && alignmentToParent.right == -1)
				{
					offsetW = expectedBounds.x1;
				}
				else
				{
					if (alignmentToParent.left != -1)
					{
						offsetW += alignmentToParent.left - parentInternalMargin.left;
					}
					if (alignmentToParent.right != -1)
					{
						offsetW += alignmentToParent.right - parentInternalMargin.right;
					}
				}

				if (alignmentToParent.top == -1 && alignmentToParent.bottom == -1)
				{
					offsetH = expectedBounds.y1;
				}
				else
				{
					if (alignmentToParent.top != -1)
					{
						offsetH += alignmentToParent.top - parentInternalMargin.top;
					}
					if (alignmentToParent.bottom != -1)
					{
						offsetH += alignmentToParent.bottom - parentInternalMargin.bottom;
					}
				}

				return { cachedMinSize.x + offsetW,cachedMinSize.y + offsetH };
			}

			Rect GuiBoundsComposition::Layout_CalculateBounds(Size parentSize)
			{
				if (auto parent = GetParent())
				{
					Rect result;
					Rect parentBounds({}, parentSize);
					Margin parentInternalMargin = parent->GetInternalMargin();

					if (alignmentToParent.left != -1 && alignmentToParent.right != -1)
					{
						result.x1 = parentBounds.x1 + alignmentToParent.left;
						result.x2 = parentBounds.x2 - alignmentToParent.right;
					}
					else if (alignmentToParent.left != -1)
					{
						result.x1 = parentBounds.x1 + alignmentToParent.left;
						result.x2 = result.x1 + cachedMinSize.x;
					}
					else if (alignmentToParent.right != -1)
					{
						result.x2 = parentBounds.x2 - alignmentToParent.right;
						result.x1 = result.x2 - cachedMinSize.x;
					}
					else
					{
						result.x1 = expectedBounds.x1 + parentInternalMargin.left;
						result.x2 = result.x1 + cachedMinSize.x;
					}

					if (alignmentToParent.top != -1 && alignmentToParent.bottom != -1)
					{
						result.y1 = parentBounds.y1 + alignmentToParent.top;
						result.y2 = parentBounds.y2 - alignmentToParent.bottom;
					}
					else if (alignmentToParent.top != -1)
					{
						result.y1 = parentBounds.y1 + alignmentToParent.top;
						result.y2 = result.y1 + cachedMinSize.y;
					}
					else if (alignmentToParent.bottom != -1)
					{
						result.y2 = parentBounds.y2 - alignmentToParent.bottom;
						result.y1 = result.y2 - cachedMinSize.y;
					}
					else
					{
						result.y1 = expectedBounds.y1 + parentInternalMargin.top;
						result.y2 = result.y1 + cachedMinSize.y;
					}

					return result;
				}
				else
				{
					return Rect(expectedBounds.LeftTop(), cachedMinSize);
				}
			}

			Rect GuiBoundsComposition::GetExpectedBounds()
			{
				return expectedBounds;
			}

			void GuiBoundsComposition::SetExpectedBounds(Rect value)
			{
				if (value.x2 < value.x1) value.x2 = value.x1;
				if (value.y2 < value.y1) value.y2 = value.y1;

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