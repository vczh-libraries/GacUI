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

			void GuiStackComposition::UpdateStackItemBounds()
			{
				if (stackItemBounds.Count() != stackItems.Count())
				{
					stackItemBounds.Resize(stackItems.Count());
				}

				stackItemTotalSize = Size(0, 0);
				Point offset;
				for (vint i = 0; i < stackItems.Count(); i++)
				{
					vint offsetX = 0;
					vint offsetY = 0;
					Size itemSize = stackItems[i]->GetMinSize();
					stackItemBounds[i] = Rect(offset, itemSize);

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
				EnsureStackItemVisible();
			}

			void GuiStackComposition::EnsureStackItemVisible()
			{
#define ADJUSTMENT(U, V)															\
				if (itemBounds.U() <= 0)											\
				{																	\
					adjustment -= itemBounds.U();									\
					InvokeOnCompositionStateChanged();								\
				}																	\
				else																\
				{																	\
					vint overflow = itemBounds.V() - previousBounds.V();			\
					if (overflow > 0)												\
					{																\
						adjustment -= overflow;										\
						InvokeOnCompositionStateChanged();							\
					}																\
				}																	\

				if (ensuringVisibleStackItem)
				{
					Rect itemBounds = ensuringVisibleStackItem->GetBounds();
					switch (direction)
					{
					case Horizontal:
					case ReversedHorizontal:
						ADJUSTMENT(Left, Right)
						break;
					case Vertical:
					case ReversedVertical:
						ADJUSTMENT(Top, Bottom)
						break;
					}
				}
#undef ADJUSTMENT
			}

			void GuiStackComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				EnsureStackItemVisible();
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
					}
					UpdateStackItemBounds();
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
						ensuringVisibleStackItem = 0;
					}
					UpdateStackItemBounds();
				}
			}
			
			Size GuiStackComposition::GetMinPreferredClientSizeInternal(bool considerPreferredMinSize)
			{
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

				Size minClientSize = GuiBoundsComposition::GetMinPreferredClientSizeInternal(considerPreferredMinSize);
				return Size(
					minStackSize.x > minClientSize.x ? minStackSize.x : minClientSize.x,
					minStackSize.y > minClientSize.y ? minStackSize.y : minClientSize.y
					);
			}

			GuiStackComposition::GuiStackComposition()
			{
				BoundsChanged.AttachMethod(this, &GuiStackComposition::OnBoundsChanged);
			}

			GuiStackComposition::~GuiStackComposition()
			{
			}

			const GuiStackComposition::ItemCompositionList& GuiStackComposition::GetStackItems()
			{
				return stackItems;
			}

			bool GuiStackComposition::InsertStackItem(vint index, GuiStackItemComposition* item)
			{
				index = stackItems.Insert(index, item);
				if (!AddChild(item))
				{
					stackItems.RemoveAt(index);
					return false;
				}
				else
				{
					return true;
				}
			}

			GuiStackComposition::Direction GuiStackComposition::GetDirection()
			{
				return direction;
			}

			void GuiStackComposition::SetDirection(Direction value)
			{
				direction = value;
				needUpdate = true;
				EnsureStackItemVisible();
			}

			vint GuiStackComposition::GetPadding()
			{
				return padding;
			}

			void GuiStackComposition::SetPadding(vint value)
			{
				padding = value;
				needUpdate = true;
				EnsureStackItemVisible();
			}

			void GuiStackComposition::ForceCalculateSizeImmediately()
			{
				GuiBoundsComposition::ForceCalculateSizeImmediately();
				UpdateStackItemBounds();
			}

			Rect GuiStackComposition::GetBounds()
			{
				if (!needUpdate)
				{
					for (vint i = 0; i < stackItems.Count(); i++)
					{
						if (stackItemBounds[i].GetSize() != stackItems[i]->GetMinSize())
						{
							needUpdate = true;
							break;
						}
					}
				}

				if (needUpdate)
				{
					UpdateStackItemBounds();
				}

				Rect bounds = GuiBoundsComposition::GetBounds();
				previousBounds = bounds;
				UpdatePreviousBounds(previousBounds);
				return bounds;
			}

			Margin GuiStackComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
				needUpdate = true;
				EnsureStackItemVisible();
			}

			bool GuiStackComposition::IsStackItemClipped()
			{
				Rect clientArea = GetClientArea();
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
					ensuringVisibleStackItem = 0;
				}
				EnsureStackItemVisible();
				return ensuringVisibleStackItem != 0;
			}

/***********************************************************************
GuiStackItemComposition
***********************************************************************/

			void GuiStackItemComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsComposition::OnParentChanged(oldParent, newParent);
				stackParent = newParent == 0 ? 0 : dynamic_cast<GuiStackComposition*>(newParent);
			}

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

			GuiStackItemComposition::GuiStackItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
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
