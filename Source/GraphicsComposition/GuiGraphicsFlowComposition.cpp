#include "GuiGraphicsFlowComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

/***********************************************************************
GuiFlowComposition
***********************************************************************/

			void GuiFlowComposition::Layout_UpdateFlowItemLayout(vint width)
			{
				for (auto item : layout_flowItems)
				{
					item->Layout_SetCachedMinSize(item->Layout_CalculateMinSizeHelper());
				}

				if (layout_LastWidth != width)
				{
					layout_Invalid = true;
					layout_LastWidth = width;
				}

				if (!layout_Invalid) return;

				auto clientMargin = axis->RealMarginToVirtualMargin(extraMargin);
				if (clientMargin.left < 0) clientMargin.left = 0;
				if (clientMargin.top < 0) clientMargin.top = 0;
				if (clientMargin.right < 0) clientMargin.right = 0;
				if (clientMargin.bottom < 0) clientMargin.bottom = 0;

				auto clientSize = axis->RealSizeToVirtualSize(flowBounds.GetSize());
				clientSize.x -= (clientMargin.left + clientMargin.right);
				clientSize.y -= (clientMargin.top + clientMargin.bottom);

				vint currentIndex = 0;
				vint rowTop = 0;

				while (currentIndex < flowItems.Count())
				{
					auto currentItemVirtualMinSize = axis->RealSizeToVirtualSize(flowItems[currentIndex]->GetCachedMinSize());
					vint rowWidth = currentItemVirtualMinSize.x;
					vint rowHeight = currentItemVirtualMinSize.y;
					vint rowItemCount = 1;

					for (vint i = currentIndex + 1; i < flowItems.Count(); i++)
					{
						auto itemSize = axis->RealSizeToVirtualSize(flowItems[i]->GetCachedMinSize());
						vint itemWidth = itemSize.x + columnPadding;
						if (rowWidth + itemWidth > clientSize.x)
						{
							break;
						}
						rowWidth += itemWidth;
						if (rowHeight < itemSize.y)
						{
							rowHeight = itemSize.y;
						}
						rowItemCount++;
					}

					vint baseLine = 0;
					Array<vint> itemBaseLines(rowItemCount);
					for (vint i = 0; i < rowItemCount; i++)
					{
						vint index = currentIndex + i;
						vint itemBaseLine = 0;
						auto itemSize = axis->RealSizeToVirtualSize(flowItems[index]->GetCachedMinSize());

						auto option = flowItems[index]->GetFlowOption();
						switch (option.baseline)
						{
						case GuiFlowOption::FromTop:
							itemBaseLine = option.distance;
							break;
						case GuiFlowOption::FromBottom:
							itemBaseLine = itemSize.y - option.distance;
							break;
						case GuiFlowOption::Percentage:
							itemBaseLine = (vint)(itemSize.y*option.percentage);
							break;
						}

						itemBaseLines[i] = itemBaseLine;
						if (baseLine < itemBaseLine)
						{
							baseLine = itemBaseLine;
						}
					}

					vint rowUsedWidth = 0;
					for (vint i = 0; i < rowItemCount; i++)
					{
						vint index = currentIndex + i;
						auto itemSize = axis->RealSizeToVirtualSize(flowItems[index]->GetCachedMinSize());

						vint itemLeft = 0;
						vint itemTop = rowTop + baseLine - itemBaseLines[i];

						switch (alignment)
						{
						case FlowAlignment::Left:
							itemLeft = rowUsedWidth + i * columnPadding;
							break;
						case FlowAlignment::Center:
							itemLeft = rowUsedWidth + i * columnPadding + (clientSize.x - rowWidth) / 2;
							break;
						case FlowAlignment::Right:
							itemLeft = rowUsedWidth + i * columnPadding + (clientSize.x - rowWidth);
							break;
						case FlowAlignment::Extend:
							if (i == 0)
							{
								itemLeft = rowUsedWidth;
							}
							else
							{
								itemLeft = rowUsedWidth + (vint)((double)(clientSize.x - rowWidth) * i / (rowItemCount - 1)) + i * columnPadding;
							}
							break;
						}

						Rect flowItemBounds = axis->VirtualRectToRealRect(
							flowBounds.GetSize(),
							Rect(
								Point(
									itemLeft + clientMargin.left,
									itemTop + clientMargin.top
								),
								itemSize
							)
						);
						flowItems[index]->Layout_SetFlowItemBounds(this, flowItemBounds);
						rowUsedWidth += itemSize.x;
					}

					rowTop += rowHeight + rowPadding;
					currentIndex += rowItemCount;
				}

				minHeight = rowTop == 0 ? 0 : rowTop - rowPadding;
			}

			void GuiFlowComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item && !layout_flowItems.Contains(item))
				{
					layout_flowItems.Add(item);
					layout_Invalid = true;
				}
			}

			void GuiFlowComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item)
				{
					layout_flowItems.Remove(item);
					layout_Invalid = true;
				}
			}

			Size GuiFlowComposition::Layout_CalculateMinSize()
			{
				UpdateFlowItemMinSizes();

				Size minFlowSize;
				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					minFlowSize = axis->VirtualSizeToRealSize(Size(0, minHeight));
					for (auto item : flowItems)
					{
						auto itemSize = item->GetCachedMinSize();
						if (minFlowSize.x < itemSize.x) minFlowSize.x = itemSize.x;
						if (minFlowSize.y < itemSize.y) minFlowSize.y = itemSize.y;
					}
				}

				if (extraMargin.left > 0) minFlowSize.x += extraMargin.left;
				if (extraMargin.right > 0) minFlowSize.x += extraMargin.right;
				if (extraMargin.top > 0) minFlowSize.y += extraMargin.top;
				if (extraMargin.bottom > 0) minFlowSize.y += extraMargin.bottom;

				Size minClientSize = GuiBoundsComposition::Layout_CalculateMinSize();
				return Size(
					minFlowSize.x > minClientSize.x ? minFlowSize.x : minClientSize.x,
					minFlowSize.y > minClientSize.y ? minFlowSize.y : minClientSize.y
					);
			}

			Rect GuiFlowComposition::Layout_CalculateBounds(Rect parentBounds)
			{
				Rect bounds = GuiBoundsComposition::Layout_CalculateBounds(parentBounds);
				UpdateFlowItemBounds(bounds);
				return bounds;
			}

			const GuiFlowComposition::ItemCompositionList& GuiFlowComposition::GetFlowItems()
			{
				return flowItems;
			}

			bool GuiFlowComposition::InsertFlowItem(vint index, GuiFlowItemComposition* item)
			{
				index = flowItems.Insert(index, item);
				if (AddChild(item)) return true;
				flowItems.RemoveAt(index);
				return false;
			}


			Margin GuiFlowComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowComposition::SetExtraMargin(Margin value)
			{
				if (extraMargin != value)
				{
					extraMargin = value;
					InvokeOnCompositionStateChanged();
				}
			}

			vint GuiFlowComposition::GetRowPadding()
			{
				return rowPadding;
			}

			void GuiFlowComposition::SetRowPadding(vint value)
			{
				if (rowPadding != value)
				{
					rowPadding = value;
					InvokeOnCompositionStateChanged();
				}
			}

			vint GuiFlowComposition::GetColumnPadding()
			{
				return columnPadding;
			}

			void GuiFlowComposition::SetColumnPadding(vint value)
			{
				if (columnPadding != value)
				{
					columnPadding = value;
					InvokeOnCompositionStateChanged();
				}
			}

			Ptr<IGuiAxis> GuiFlowComposition::GetAxis()
			{
				return axis;
			}

			void GuiFlowComposition::SetAxis(Ptr<IGuiAxis> value)
			{
				if (value)
				{
					axis = value;
					InvokeOnCompositionStateChanged();
				}
			}

			FlowAlignment GuiFlowComposition::GetAlignment()
			{
				return alignment;
			}

			void GuiFlowComposition::SetAlignment(FlowAlignment value)
			{
				if (alignment != value)
				{
					alignment = value;
					InvokeOnCompositionStateChanged();
				}
			}

/***********************************************************************
GuiFlowItemComposition
***********************************************************************/

			void GuiFlowItemComposition::OnParentLineChanged()
			{
				flowParent = dynamic_cast<GuiFlowComposition*>(GetParent());
				GuiGraphicsComposition::OnParentLineChanged();
			}

			void GuiFlowItemComposition::Layout_SetFlowItemBounds(Rect bounds)
			{
				Rect result = bounds;
				result = Rect(
					result.Left() - extraMargin.left,
					result.Top() - extraMargin.top,
					result.Right() + extraMargin.right,
					result.Bottom() + extraMargin.bottom
					);
				Layout_SetCachedBounds(result);
			}

			GuiFlowItemComposition::GuiFlowItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				CachedMinSizeChanged.AttachLambda([this](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					if (flowParent) flowParent->layout_Invalid = true;
				});
			}

			Margin GuiFlowItemComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowItemComposition::SetExtraMargin(Margin value)
			{
				if (extraMargin != value)
				{
					extraMargin = value;
					InvokeOnCompositionStateChanged();
				}
			}

			GuiFlowOption GuiFlowItemComposition::GetFlowOption()
			{
				return option;
			}

			void GuiFlowItemComposition::SetFlowOption(GuiFlowOption value)
			{
				if (option != value)
				{
					option = value;
					InvokeOnCompositionStateChanged();
				}
			}
		}
	}
}