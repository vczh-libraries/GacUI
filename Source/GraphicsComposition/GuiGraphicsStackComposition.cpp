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

			void GuiStackComposition::UpdateStackItemMinSizes()
			{
				for (auto item : stackItems)
				{
					item->Layout_SetCachedMinSize(item->Layout_CalculateMinSizeHelper());
				}

				stackItemTotalSize = Size(0, 0);
				Point offset;
				for (vint i = 0; i < stackItems.Count(); i++)
				{
					Size itemSize = stackItems[i]->GetCachedMinSize();

#define ACCUMULATE(U, V)										\
					{											\
						if (stackItemTotalSize.V < itemSize.V)	\
						{										\
							stackItemTotalSize.V = itemSize.V;	\
						}										\
						if (i > 0)								\
						{										\
							stackItemTotalSize.U += padding;	\
						}										\
						stackItemTotalSize.U += itemSize.U;		\
					}											\

					switch (direction)
					{
					case GuiStackComposition::Horizontal:
					case GuiStackComposition::ReversedHorizontal:
						ACCUMULATE(x, y)
						break;
					case GuiStackComposition::Vertical:
					case GuiStackComposition::ReversedVertical:
						ACCUMULATE(y, x)
						break;
					}

#undef ACCUMULATE
					offset.x += itemSize.x + padding;
					offset.y += itemSize.y + padding;
				}
			}

			void GuiStackComposition::UpdateStackItemBounds()
			{
				Point offset;
				for (vint i = 0; i < stackItems.Count(); i++)
				{
					Size itemSize = stackItems[i]->GetCachedMinSize();
					Rect itemBounds = Rect(offset, itemSize);
					stackItems[i]->Layout_SetStackItemBounds(this, itemBounds);
					offset.x += itemSize.x + padding;
					offset.y += itemSize.y + padding;
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
				UpdateStackItemMinSizes();

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

				if (extraMargin.left > 0) minStackSize.x += extraMargin.left;
				if (extraMargin.right > 0) minStackSize.x += extraMargin.right;
				if (extraMargin.top > 0) minStackSize.y += extraMargin.top;
				if (extraMargin.bottom > 0) minStackSize.y += extraMargin.bottom;

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
				UpdateStackItemBounds();
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
				direction = value;
				InvokeOnCompositionStateChanged();
			}

			vint GuiStackComposition::GetPadding()
			{
				return padding;
			}

			void GuiStackComposition::SetPadding(vint value)
			{
				padding = value;
				InvokeOnCompositionStateChanged();
			}

			Margin GuiStackComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
				InvokeOnCompositionStateChanged();
			}

			bool GuiStackComposition::IsStackItemClipped()
			{
				Rect clientArea = GetCachedClientArea();
				switch(direction)
				{
				case Horizontal:
				case ReversedHorizontal:
					{
						vint width = stackItemTotalSize.x
							+ (extraMargin.left > 0 ? extraMargin.left : 0)
							+ (extraMargin.right > 0 ? extraMargin.right : 0)
							;
						return width > clientArea.Width();
					}
					break;
				case Vertical:
				case ReversedVertical:
					{
						vint height = stackItemTotalSize.y
							+ (extraMargin.top > 0 ? extraMargin.top : 0)
							+ (extraMargin.bottom > 0 ? extraMargin.bottom : 0)
							;
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

			void GuiStackItemComposition::Layout_SetStackItemBounds(GuiStackComposition* stackParent, Rect bounds)
			{
				Rect result = bounds;
				Rect parentBounds = stackParent->cachedBounds;
				Margin margin = stackParent->extraMargin;
				if (margin.left <= 0) margin.left = 0;
				if (margin.top <= 0) margin.top = 0;
				if (margin.right <= 0) margin.right = 0;
				if (margin.bottom <= 0) margin.bottom = 0;

				auto x = result.Left();
				auto y = result.Top();
				auto w = result.Width();
				auto h = result.Height();

				switch (stackParent->direction)
				{
				case GuiStackComposition::Horizontal:
					x += margin.left + stackParent->adjustment;
					y = margin.top;
					h = parentBounds.Height() - margin.top - margin.bottom;
					break;
				case GuiStackComposition::ReversedHorizontal:
					x = parentBounds.Width() - margin.right - x - w + stackParent->adjustment;
					y = margin.top;
					h = parentBounds.Height() - margin.top - margin.bottom;
					break;
				case GuiStackComposition::Vertical:
					x = margin.left;
					y += margin.top + stackParent->adjustment;
					w = parentBounds.Width() - margin.left - margin.right;
					break;
				case GuiStackComposition::ReversedVertical:
					x = margin.left;
					y = parentBounds.Height() - margin.bottom - y - h + stackParent->adjustment;
					w = parentBounds.Width() - margin.left - margin.right;
					break;
				}

				result = Rect(
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
