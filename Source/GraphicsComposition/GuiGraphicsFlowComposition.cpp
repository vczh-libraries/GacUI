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

			void GuiFlowComposition::Layout_UpdateFlowItemLayout(vint maxVirtualWidth)
			{
				for (auto item : layout_flowItems)
				{
					item->Layout_SetCachedMinSize(item->Layout_CalculateMinSizeHelper());
				}

				if (layout_lastVirtualWidth != maxVirtualWidth)
				{
					layout_invalid = true;
					layout_lastVirtualWidth = maxVirtualWidth;
				}

				if (!layout_invalid) return;
				layout_invalid = false;

				vint currentIndex = 0;
				vint rowTop = 0;

				while (currentIndex < layout_flowItems.Count())
				{
					auto currentItemVirtualMinSize = axis->RealSizeToVirtualSize(layout_flowItems[currentIndex]->GetCachedMinSize());
					vint rowWidth = currentItemVirtualMinSize.x;
					vint rowHeight = currentItemVirtualMinSize.y;
					vint rowItemCount = 1;

					for (vint i = currentIndex + 1; i < layout_flowItems.Count(); i++)
					{
						auto itemSize = axis->RealSizeToVirtualSize(layout_flowItems[i]->GetCachedMinSize());
						vint itemWidth = itemSize.x + columnPadding;
						if (rowWidth + itemWidth > maxVirtualWidth)
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
						auto itemSize = axis->RealSizeToVirtualSize(layout_flowItems[index]->GetCachedMinSize());

						auto option = layout_flowItems[index]->GetFlowOption();
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
						auto itemSize = axis->RealSizeToVirtualSize(layout_flowItems[index]->GetCachedMinSize());

						vint itemLeft = 0;
						vint itemTop = rowTop + baseLine - itemBaseLines[i];

						switch (alignment)
						{
						case FlowAlignment::Left:
							itemLeft = rowUsedWidth + i * columnPadding;
							break;
						case FlowAlignment::Center:
							itemLeft = rowUsedWidth + i * columnPadding + (maxVirtualWidth - rowWidth) / 2;
							break;
						case FlowAlignment::Right:
							itemLeft = rowUsedWidth + i * columnPadding + (maxVirtualWidth - rowWidth);
							break;
						case FlowAlignment::Extend:
							if (i == 0)
							{
								itemLeft = rowUsedWidth;
							}
							else
							{
								itemLeft = rowUsedWidth + (vint)((double)(maxVirtualWidth - rowWidth) * i / (rowItemCount - 1)) + i * columnPadding;
							}
							break;
						}

						layout_flowItems[index]->layout_virtualBounds = Rect({ itemLeft,itemTop }, itemSize);
						rowUsedWidth += itemSize.x;
					}

					rowTop += rowHeight + rowPadding;
					currentIndex += rowItemCount;
				}

				layout_minVirtualHeight = rowTop == 0 ? 0 : rowTop - rowPadding;
			}

			Size GuiFlowComposition::Layout_UpdateFlowItemLayoutByConstraint(Size constraintSize)
			{
				Size extraSize(
					extraMargin.left + extraMargin.right,
					extraMargin.top + extraMargin.bottom
				);
				constraintSize.x -= extraSize.x;
				constraintSize.y -= extraSize.y;
				if (constraintSize.x < 0) constraintSize.x = 0;
				if (constraintSize.y < 0) constraintSize.y = 0;

				vint maxVirtualWidth = axis->RealSizeToVirtualSize(constraintSize).x;
				Layout_UpdateFlowItemLayout(maxVirtualWidth);
				return extraSize;
			}

			void GuiFlowComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item && !layout_flowItems.Contains(item))
				{
					layout_flowItems.Add(item);
				}
			}

			void GuiFlowComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item)
				{
					layout_flowItems.Remove(item);
				}
			}

			void GuiFlowComposition::OnCompositionStateChanged()
			{
				GuiBoundsComposition::OnCompositionStateChanged();
				layout_invalid = true;
			}

			Size GuiFlowComposition::Layout_CalculateMinSize()
			{
				Size minSize = GuiBoundsComposition::Layout_CalculateMinSize();

				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren && layout_flowItems.Count() > 0)
				{
					Size cachedSize = cachedBounds.GetSize();
					Size constraintSize(
						minSize.x > cachedSize.x ? minSize.x : cachedSize.x,
						minSize.y > cachedSize.y ? minSize.y : cachedSize.y
					);

					Size extraSize = Layout_UpdateFlowItemLayoutByConstraint(constraintSize);
					Size minFlowSize = axis->VirtualSizeToRealSize(Size(0, layout_minVirtualHeight));
					minFlowSize.x += extraSize.x;
					minFlowSize.y += extraSize.y;

					if (minSize.x < minFlowSize.x) minSize.x = minFlowSize.x;
					if (minSize.y < minFlowSize.y) minSize.y = minFlowSize.y;
				}

				return minSize;
			}

			Rect GuiFlowComposition::Layout_CalculateBounds(Size parentSize)
			{
				Rect bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);
				Size extraSize = Layout_UpdateFlowItemLayoutByConstraint(bounds.GetSize());
				Size contentSize(
					bounds.Width() - extraSize.x,
					bounds.Height() - extraSize.y
				);
				for (auto item : layout_flowItems)
				{
					item->Layout_SetFlowItemBounds(contentSize, item->layout_virtualBounds);
				}
				return bounds;
			}

			const GuiFlowComposition::ItemCompositionList& GuiFlowComposition::GetFlowItems()
			{
				return layout_flowItems;
			}

			bool GuiFlowComposition::InsertFlowItem(vint index, GuiFlowItemComposition* item)
			{
				index = layout_flowItems.Insert(index, item);
				if (AddChild(item))
				{
					return true;
				}
				layout_flowItems.RemoveAt(index);
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

			void GuiFlowItemComposition::Layout_SetFlowItemBounds(Size contentSize, Rect virtualBounds)
			{
				Rect result = layout_flowParent->axis->VirtualRectToRealRect(contentSize, virtualBounds);

				result.x1 += layout_flowParent->extraMargin.left;
				result.x2 += layout_flowParent->extraMargin.left;
				result.y1 += layout_flowParent->extraMargin.top;
				result.y2 += layout_flowParent->extraMargin.top;

				result.x1 -= extraMargin.left;
				result.y1 -= extraMargin.top;
				result.x2 += extraMargin.right;
				result.y2 += extraMargin.bottom;

				Layout_SetCachedBounds(result);
			}

			void GuiFlowItemComposition::OnParentLineChanged()
			{
				layout_flowParent = dynamic_cast<GuiFlowComposition*>(GetParent());
				GuiGraphicsComposition::OnParentLineChanged();
			}

			GuiFlowItemComposition::GuiFlowItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				CachedMinSizeChanged.AttachLambda([this](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					if (layout_flowParent) layout_flowParent->layout_invalid = true;
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
					if (layout_flowParent) layout_flowParent->layout_invalid = true;
					InvokeOnCompositionStateChanged();
				}
			}
		}
	}
}