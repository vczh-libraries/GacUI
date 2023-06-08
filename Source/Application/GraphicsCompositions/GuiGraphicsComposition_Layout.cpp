#include "GuiGraphicsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

			Rect GuiGraphicsComposition::GetBoundsInternal(Rect expectedBounds, bool considerPreferredMinSize)
			{
				Size minSize = GetMinPreferredClientSizeInternal(considerPreferredMinSize);
				minSize.x += internalMargin.left + internalMargin.right;
				minSize.y += internalMargin.top + internalMargin.bottom;
				vint w = expectedBounds.Width();
				vint h = expectedBounds.Height();
				if (minSize.x < w) minSize.x = w;
				if (minSize.y < h) minSize.y = h;
				return Rect(expectedBounds.LeftTop(), minSize);
			}

			Size GuiGraphicsComposition::GetMinPreferredClientSizeInternal(bool considerPreferredMinSize)
			{
				Size minSize;
				if (minSize.x < preferredMinSize.x) minSize.x = preferredMinSize.x;
				if (minSize.y < preferredMinSize.y) minSize.y = preferredMinSize.y;

				if (minSizeLimitation != GuiGraphicsComposition::NoLimit)
				{
					if (ownedElement)
					{
						IGuiGraphicsRenderer* renderer = ownedElement->GetRenderer();
						if (renderer)
						{
							auto elementSize = renderer->GetMinSize();
							if (minSize.x < elementSize.x) minSize.x = elementSize.x;
							if (minSize.y < elementSize.y) minSize.y = elementSize.y;
						}
					}
				}
				if (minSizeLimitation == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					for (auto child : children)
					{
						if (child->IsTrivialComposition())
						{
							Rect childBounds = InvokeGetPreferredBoundsInternal(child, considerPreferredMinSize);
							if (minSize.x < childBounds.x2) minSize.x = childBounds.x2;
							if (minSize.y < childBounds.y2) minSize.y = childBounds.y2;
						}
					}
				}
				return minSize;
			}

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
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiGraphicsComposition::Layout_SetCachedBounds(Rect value)
			{
				if (cachedBounds != value)
				{
					cachedBounds = value;
					CachedBoundsChanged.Execute(GuiEventArgs(this));
					InvokeOnCompositionStateChanged();
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

			Rect GuiGraphicsComposition::GetGlobalBounds()
			{
				Rect bounds = cachedBounds;
				GuiGraphicsComposition* composition = parent;
				while (composition)
				{
					Rect clientArea = composition->GetCachedClientArea();
					bounds.x1 += clientArea.x1;
					bounds.x2 += clientArea.x1;
					bounds.y1 += clientArea.y1;
					bounds.y2 += clientArea.y1;
					composition = composition->parent;
				}
				return bounds;
			}
		}
	}
}