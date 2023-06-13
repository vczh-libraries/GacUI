#include "GuiGraphicsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace elements;

/***********************************************************************
GuiGraphicsComposition
***********************************************************************/

			Size GuiGraphicsComposition::Layout_CalculateMinSizeHelper()
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

				vint offsetW = internalMargin.left + internalMargin.right;
				vint offsetH = internalMargin.top + internalMargin.bottom;
				minSize.x += offsetW;
				minSize.y += offsetH;

				for (auto child : children)
				{
					child->Layout_UpdateMinSize();
				}

				if (minSizeLimitation == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					for (auto child : children)
					{
						Size minClientSize = child->Layout_CalculateMinClientSizeForParent(internalMargin);
						Size minBoundsSize(minClientSize.x + offsetW, minClientSize.y + offsetH);
						if (minSize.x < minBoundsSize.x) minSize.x = minBoundsSize.x;
						if (minSize.y < minBoundsSize.y) minSize.y = minBoundsSize.y;
					}
				}
				return minSize;
			}

			void GuiGraphicsComposition::Layout_SetCachedMinSize(Size value)
			{
				if (value.x < 0) value.x = 0;
				if (value.y < 0) value.y = 0;

				if (cachedMinSize != value)
				{
					cachedMinSize = value;
					CachedMinSizeChanged.Execute(GuiEventArgs(this));
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiGraphicsComposition::Layout_SetCachedBounds(Rect value)
			{
				if (value.x2 < value.x1) value.x2 = value.x1;
				if (value.y2 < value.y1) value.y2 = value.y1;

				if (cachedBounds != value)
				{
					cachedBounds = value;
					CachedBoundsChanged.Execute(GuiEventArgs(this));
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiGraphicsComposition::Layout_UpdateMinSize()
			{
				Layout_SetCachedMinSize(Layout_CalculateMinSize());
			}

			void GuiGraphicsComposition::Layout_UpdateBounds(Size parentSize)
			{
				Layout_SetCachedBounds(Layout_CalculateBounds(parentSize));
				for (auto child : children)
				{
					child->Layout_UpdateBounds(cachedBounds.GetSize());
				}
			}

			Size GuiGraphicsComposition::GetCachedMinSize()
			{
				return cachedMinSize;
			}

			Size GuiGraphicsComposition::GetCachedMinClientSize()
			{
				auto minSize = cachedMinSize;
				minSize.x -= internalMargin.left + internalMargin.right;
				minSize.y -= internalMargin.top + internalMargin.bottom;
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
				if (bounds.x2 < bounds.x1) bounds.x2 = bounds.x1;
				if (bounds.y2 < bounds.y1) bounds.y2 = bounds.y1;
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

			void GuiGraphicsComposition::ForceCalculateSizeImmediately()
			{
				Size parentSize;
				if (parent)
				{
					parentSize = parent->cachedBounds.GetSize();
				}

				Layout_UpdateMinSize();
				Layout_UpdateBounds(parentSize);
			}
		}
	}
}