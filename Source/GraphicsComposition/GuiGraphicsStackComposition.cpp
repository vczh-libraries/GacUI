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
				for (auto item : layout_stackItems)
				{
					item->Layout_SetCachedMinSize(item->Layout_CalculateMinSizeHelper());
				}

				if (!layout_invalid) return;
				layout_invalid = false;

				Point virtualOffset;
				layout_stackItemTotalSize = Size(0, 0);
				for (auto [item, index] : indexed(layout_stackItems))
				{
					Size itemSize = item->GetCachedMinSize();

					switch (direction)
					{
					case GuiStackComposition::Horizontal:
					case GuiStackComposition::ReversedHorizontal:
						{
							if (layout_stackItemTotalSize.y < itemSize.y)
							{
								layout_stackItemTotalSize.y = itemSize.y;
							}
							if (index > 0)
							{
								layout_stackItemTotalSize.x += padding;
							}
							layout_stackItemTotalSize.x += itemSize.x;
						}
						break;
					case GuiStackComposition::Vertical:
					case GuiStackComposition::ReversedVertical:
						{
							if (layout_stackItemTotalSize.x < itemSize.x)
							{
								layout_stackItemTotalSize.x = itemSize.x;
							}
							if (index > 0)
							{
								layout_stackItemTotalSize.y += padding;
							}
							layout_stackItemTotalSize.y += itemSize.y;
						}
						break;
					}

					item->layout_virtualOffset = virtualOffset;
					virtualOffset.x += itemSize.x + padding;
					virtualOffset.y += itemSize.y + padding;
				}
			}

			void GuiStackComposition::Layout_UpdateStackItemBounds(Rect contentBounds)
			{
				if (layout_ensuringVisibleStackItem)
				{
					Rect itemBounds(
						layout_ensuringVisibleStackItem->layout_virtualOffset,
						layout_ensuringVisibleStackItem->GetCachedMinSize()
					);

					switch (direction)
					{
					case Horizontal:
					case ReversedHorizontal:
						if (itemBounds.x1 <= 0)
						{
							layout_adjustment = -itemBounds.x1;
						}
						else
						{
							vint overflow = itemBounds.x2 - contentBounds.x2;
							if (overflow > 0)
							{
								layout_adjustment = -overflow;
							}
						}
						break;
					case Vertical:
					case ReversedVertical:
						if (itemBounds.y1 <= 0)
						{
							layout_adjustment = -itemBounds.y1;
						}
						else
						{
							vint overflow = itemBounds.y2 - contentBounds.y2;
							if (overflow > 0)
							{
								layout_adjustment = -overflow;
							}
						}
						break;
					}
				}

				for (auto item : layout_stackItems)
				{
					item->Layout_SetStackItemBounds(contentBounds, item->layout_virtualOffset);
				}
			}

			void GuiStackComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				GuiStackItemComposition* item = dynamic_cast<GuiStackItemComposition*>(child);
				if (item)
				{
					if (!layout_stackItems.Contains(item))
					{
						layout_stackItems.Add(item);
					}
				}
			}

			void GuiStackComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				GuiStackItemComposition* item = dynamic_cast<GuiStackItemComposition*>(child);
				if (item)
				{
					layout_stackItems.Remove(item);
					if (item == layout_ensuringVisibleStackItem)
					{
						layout_ensuringVisibleStackItem = nullptr;
					}
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
					if (!layout_ensuringVisibleStackItem || direction == Vertical || direction == ReversedVertical)
					{
						minStackSize.x = layout_stackItemTotalSize.x;
					}
					if (!layout_ensuringVisibleStackItem || direction == Horizontal || direction == ReversedHorizontal)
					{
						minStackSize.y = layout_stackItemTotalSize.y;
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
				return layout_stackItems;
			}

			bool GuiStackComposition::InsertStackItem(vint index, GuiStackItemComposition* item)
			{
				index = layout_stackItems.Insert(index, item);
				if (AddChild(item))
				{
					return true;
				}
				layout_stackItems.RemoveAt(index);
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
						vint width = layout_stackItemTotalSize.x + extraMargin.left + extraMargin.right;
						return width > clientArea.Width();
					}
					break;
				case Vertical:
				case ReversedVertical:
					{
						vint height = layout_stackItemTotalSize.y + extraMargin.top + extraMargin.bottom;
						return height > clientArea.Height();
					}
					break;
				}
				return false;
			}

			bool GuiStackComposition::EnsureVisible(vint index)
			{
				if (0 <= index && index < layout_stackItems.Count())
				{
					layout_ensuringVisibleStackItem = layout_stackItems[index];
				}
				else
				{
					layout_ensuringVisibleStackItem = nullptr;
				}
				InvokeOnCompositionStateChanged();
				return layout_ensuringVisibleStackItem != nullptr;
			}

/***********************************************************************
GuiStackItemComposition
***********************************************************************/

			void GuiStackItemComposition::Layout_SetStackItemBounds(Rect contentBounds, Point virtualOffset)
			{
				vint x = 0;
				vint y = 0;
				switch (layout_stackParent->direction)
				{
				case GuiStackComposition::Horizontal:
					x = contentBounds.x1 + virtualOffset.x;
					y = contentBounds.y1;
					break;
				case GuiStackComposition::ReversedHorizontal:
					x = contentBounds.x2 - virtualOffset.x - cachedMinSize.x;
					y = contentBounds.y1;
					break;
				case GuiStackComposition::Vertical:
					x = contentBounds.x1;
					y = contentBounds.y1 + virtualOffset.y;
					break;
				case GuiStackComposition::ReversedVertical:
					x = contentBounds.x1;
					y = contentBounds.y2 - virtualOffset.y - cachedMinSize.y;
					break;
				}
				switch (layout_stackParent->direction)
				{
				case GuiStackComposition::Horizontal:
					x += layout_stackParent->layout_adjustment;
					break;
				case GuiStackComposition::ReversedHorizontal:
					x -= layout_stackParent->layout_adjustment;
					break;
				case GuiStackComposition::Vertical:
					y += layout_stackParent->layout_adjustment;
					break;
				case GuiStackComposition::ReversedVertical:
					y -= layout_stackParent->layout_adjustment;
					break;
				}

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
