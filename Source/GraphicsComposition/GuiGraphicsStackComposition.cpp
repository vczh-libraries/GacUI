#include "GuiGraphicsStackComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiStackComposition
***********************************************************************/

			void GuiStackComposition::Layout_UpdateStackItemMinSizes()
			{
				for (auto item : stackItems)
				{
					item->Layout_SetCachedMinSize(item->Layout_CalculateMinSizeHelper());
				}

				stackItemTotalSize = Size(0, 0);
				for (vint i = 0; i < stackItems.Count(); i++)
				{
					Size itemSize = stackItems[i]->GetCachedMinSize();

					switch (direction)
					{
					case GuiStackComposition::Horizontal:
					case GuiStackComposition::ReversedHorizontal:
						{
							if (stackItemTotalSize.y < itemSize.y)
							{
								stackItemTotalSize.y = itemSize.y;
							}
							if (i > 0)
							{
								stackItemTotalSize.x += padding;
							}
							stackItemTotalSize.x += itemSize.x;
						}
						break;
					case GuiStackComposition::Vertical:
					case GuiStackComposition::ReversedVertical:
						{
							if (stackItemTotalSize.x < itemSize.x)
							{
								stackItemTotalSize.x = itemSize.x;
							}
							if (i > 0)
							{
								stackItemTotalSize.y += padding;
							}
							stackItemTotalSize.y += itemSize.y;
						}
						break;
					}
				}
			}

			void GuiStackComposition::Layout_UpdateStackItemBounds(Rect contentBounds)
			{
				Point virtualOffset;
				for (vint i = 0; i < stackItems.Count(); i++)
				{
					Size itemSize = stackItems[i]->GetCachedMinSize();
					stackItems[i]->Layout_SetStackItemBounds(contentBounds, virtualOffset);
					virtualOffset.x += itemSize.x + padding;
					virtualOffset.y += itemSize.y + padding;
				}

				if (ensuringVisibleStackItem)
				{
					Rect itemBounds = ensuringVisibleStackItem->GetCachedBounds();
					switch (direction)
					{
					case Horizontal:
					case ReversedHorizontal:
						if (itemBounds.x1 <= 0)
						{
							adjustment -= itemBounds.x1;
						}
						else
						{
							vint overflow = itemBounds.x2 - cachedBounds.x2;
							if (overflow > 0)
							{
								adjustment -= overflow;
							}
						}
						break;
					case Vertical:
					case ReversedVertical:
						if (itemBounds.y1 <= 0)
						{
							adjustment -= itemBounds.y1;
						}
						else
						{
							vint overflow = itemBounds.y2 - cachedBounds.y2;
							if (overflow > 0)
							{
								adjustment -= overflow;
							}
						}
						break;
					}
				}
			}

			void GuiStackComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				GuiStackItemComposition* item = dynamic_cast<GuiStackItemComposition*>(child);
				if (item)
				{
					if (!stackItems.Contains(item))
					{
						stackItems.Add(item);
						layout_invalid = true;
					}
				}
			}

			void GuiStackComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				GuiStackItemComposition* item = dynamic_cast<GuiStackItemComposition*>(child);
				if (item)
				{
					stackItems.Remove(item);
					if (item == ensuringVisibleStackItem)
					{
						ensuringVisibleStackItem = nullptr;
					}
					layout_invalid = true;
				}
			}

			void GuiStackComposition::OnCompositionStateChanged()
			{
				GuiBoundsComposition::OnCompositionStateChanged();
				layout_invalid = true;
			}

			Size GuiStackComposition::Layout_CalculateMinSize()
			{
				Layout_UpdateStackItemMinSizes();

				Size minStackSize;
				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					if (!ensuringVisibleStackItem || direction == Vertical || direction == ReversedVertical)
					{
						minStackSize.x = stackItemTotalSize.x;
					}
					if (!ensuringVisibleStackItem || direction == Horizontal || direction == ReversedHorizontal)
					{
						minStackSize.y = stackItemTotalSize.y;
					}
				}

				minStackSize.x += extraMargin.left;
				minStackSize.x += extraMargin.right;
				minStackSize.y += extraMargin.top;
				minStackSize.y += extraMargin.bottom;

				Size minClientSize = GuiBoundsComposition::Layout_CalculateMinSize();
				return Size(
					minStackSize.x > minClientSize.x ? minStackSize.x : minClientSize.x,
					minStackSize.y > minClientSize.y ? minStackSize.y : minClientSize.y
					);
				return minStackSize;
			}

			Rect GuiStackComposition::Layout_CalculateBounds(Size parentSize)
			{
				Rect bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);
				Rect contentBounds(
					extraMargin.left,
					extraMargin.top,
					bounds.Width() - extraMargin.right,
					bounds.Height() - extraMargin.bottom
				);
				Layout_UpdateStackItemBounds(contentBounds);
				return bounds;
			}

			const GuiStackComposition::ItemCompositionList& GuiStackComposition::GetStackItems()
			{
				return stackItems;
			}

			bool GuiStackComposition::InsertStackItem(vint index, GuiStackItemComposition* item)
			{
				index = stackItems.Insert(index, item);
				if (AddChild(item))
				{
					layout_invalid = true;
					return true;
				}
				stackItems.RemoveAt(index);
				return false;
			}

			GuiStackComposition::Direction GuiStackComposition::GetDirection()
			{
				return direction;
			}

			void GuiStackComposition::SetDirection(Direction value)
			{
				if (direction != value)
				{
					direction = value;
					InvokeOnCompositionStateChanged();
				}
			}

			vint GuiStackComposition::GetPadding()
			{
				return padding;
			}

			void GuiStackComposition::SetPadding(vint value)
			{
				if (padding != value)
				{
					padding = value;
					InvokeOnCompositionStateChanged();
				}
			}

			Margin GuiStackComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackComposition::SetExtraMargin(Margin value)
			{
				if (value.left < 0) value.left = 0;
				if (value.top < 0) value.top = 0;
				if (value.right < 0) value.right = 0;
				if (value.bottom < 0) value.bottom = 0;

				if (extraMargin != value)
				{
					extraMargin = value;
					InvokeOnCompositionStateChanged();
				}
			}

			bool GuiStackComposition::IsStackItemClipped()
			{
				Rect clientArea = GetCachedClientArea();
				switch(direction)
				{
				case Horizontal:
				case ReversedHorizontal:
					{
						vint width = stackItemTotalSize.x + extraMargin.left + extraMargin.right;
						return width > clientArea.Width();
					}
					break;
				case Vertical:
				case ReversedVertical:
					{
						vint height = stackItemTotalSize.y + extraMargin.top + extraMargin.bottom;
						return height > clientArea.Height();
					}
					break;
				}
				return false;
			}

			bool GuiStackComposition::EnsureVisible(vint index)
			{
				if (0 <= index && index < stackItems.Count())
				{
					ensuringVisibleStackItem = stackItems[index];
				}
				else
				{
					ensuringVisibleStackItem = nullptr;
				}
				InvokeOnCompositionStateChanged();
				return ensuringVisibleStackItem != nullptr;
			}

/***********************************************************************
GuiStackItemComposition
***********************************************************************/

			void GuiStackItemComposition::Layout_SetStackItemBounds(Rect contentBounds, Point virtualOffset)
			{
				vint x = 0;
				vint y = 0;
				vint w = 0;
				vint h = 0;

				switch (layout_stackParent->direction)
				{
				case GuiStackComposition::Horizontal:
				case GuiStackComposition::ReversedHorizontal:
					w = cachedMinSize.x;
					h = contentBounds.Height();
					break;
				case GuiStackComposition::Vertical:
				case GuiStackComposition::ReversedVertical:
					w = contentBounds.Width();
					h = cachedMinSize.y;
					break;
				}

				switch (layout_stackParent->direction)
				{
				case GuiStackComposition::Horizontal:
					x = virtualOffset.x;
					y = 0;
					break;
				case GuiStackComposition::ReversedHorizontal:
					x = contentBounds.x2 - virtualOffset.x - w;
					y = 0;
					break;
				case GuiStackComposition::Vertical:
					x = 0;
					y = virtualOffset.y;
					break;
				case GuiStackComposition::ReversedVertical:
					x = 0;
					y = contentBounds.y2 - virtualOffset.y - h;
					break;
				}

				Rect result(
					x - extraMargin.left,
					y - extraMargin.top,
					x + w + extraMargin.right,
					y + h + extraMargin.bottom
					);
				Layout_SetCachedBounds(result);
			}

			void GuiStackItemComposition::OnParentLineChanged()
			{
				layout_stackParent = dynamic_cast<GuiStackComposition*>(GetParent());
				GuiGraphicsComposition::OnParentLineChanged();
			}

			GuiStackItemComposition::GuiStackItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				CachedMinSizeChanged.AttachLambda([this](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					if (layout_stackParent) layout_stackParent->layout_invalid = true;
				});
			}

			Margin GuiStackItemComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackItemComposition::SetExtraMargin(Margin value)
			{
				if (extraMargin != value)
				{
					extraMargin = value;
					InvokeOnCompositionStateChanged();
				}
			}
		}
	}
}
