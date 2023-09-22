#include "GuiGraphicsRepeatComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

/***********************************************************************
GuiVirtualRepeatCompositionBase
***********************************************************************/

			void GuiVirtualRepeatCompositionBase::Layout_UpdateIndex(ItemStyleRecord style, vint index)
			{
			}

			void GuiVirtualRepeatCompositionBase::Layout_UpdateViewBounds(Rect value)
			{
				auto old = GetViewLocation();
				Rect oldBounds = viewBounds;
				viewBounds = value;
				OnViewChangedInternal(oldBounds, value);
				RearrangeItemBounds();
				if (old != GetViewLocation())
				{
					ViewLocationChanged.Execute(GuiEventArgs(this));
				}
			}

			void GuiVirtualRepeatCompositionBase::Layout_UpdateViewLocation(Point value)
			{
				Layout_UpdateViewBounds(Rect(value, viewBounds.GetSize()));
			}

			Rect GuiVirtualRepeatCompositionBase::Layout_CalculateBounds(Size parentSize)
			{
				auto bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);
				auto size = axis->RealSizeToVirtualSize(bounds.GetSize());
				if (size != viewBounds.GetSize() || itemSourceUpdated)
				{
					itemSourceUpdated = false;
					Layout_UpdateViewBounds(Rect(viewBounds.LeftTop(), size));
				}
				return bounds;
			}

			void GuiVirtualRepeatCompositionBase::Layout_SetStyleAlignmentToParent(ItemStyleRecord style, Margin value)
			{
				style->SetAlignmentToParent(axis->VirtualMarginToRealMargin(value));
			}

			Size GuiVirtualRepeatCompositionBase::Layout_GetStylePreferredSize(ItemStyleRecord style)
			{
				return axis->RealSizeToVirtualSize(style->GetCachedMinSize());
			}

			Rect GuiVirtualRepeatCompositionBase::Layout_GetStyleBounds(ItemStyleRecord style)
			{
				return axis->RealRectToVirtualRect(axis->VirtualSizeToRealSize(viewBounds.GetSize()), style->GetCachedBounds());
			}

			void GuiVirtualRepeatCompositionBase::Layout_SetStyleBounds(ItemStyleRecord style, Rect value)
			{
				return style->SetExpectedBounds(axis->VirtualRectToRealRect(axis->VirtualSizeToRealSize(viewBounds.GetSize()), value));
			}

			void GuiVirtualRepeatCompositionBase::OnItemChanged(vint start, vint oldCount, vint newCount)
			{
				itemSourceUpdated = true;

				vint visibleCount = visibleStyles.Count();
				vint itemCount = itemSource->GetCount();
				SortedList<ItemStyleRecord> reusedStyles;
				for (vint i = 0; i < visibleCount; i++)
				{
					vint index = startIndex + i;
					if (index >= itemCount)
					{
						break;
					}

					vint oldIndex = -1;
					if (index < start)
					{
						oldIndex = index;
					}
					else if (index >= start + newCount)
					{
						oldIndex = index - newCount + oldCount;
					}

					if (oldIndex != -1)
					{
						if (oldIndex >= startIndex && oldIndex < startIndex + visibleCount)
						{
							auto style = visibleStyles[oldIndex - startIndex];
							reusedStyles.Add(style);
							visibleStyles.Add(style);
						}
						else
						{
							oldIndex = -1;
						}
					}
					if (oldIndex == -1)
					{
						visibleStyles.Add(CreateStyle(index));
					}
				}

				for (vint i = 0; i < visibleCount; i++)
				{
					auto style = visibleStyles[i];
					if (!reusedStyles.Contains(style))
					{
						DeleteStyle(style);
					}
				}

				visibleStyles.RemoveRange(0, visibleCount);
				for (auto [style, i] : indexed(visibleStyles))
				{
					Layout_UpdateIndex(style, startIndex + 1);
				}
			}

			void GuiVirtualRepeatCompositionBase::ClearItems()
			{
				startIndex = 0;
				for (auto style : visibleStyles)
				{
					DeleteStyle(style);
				}
				visibleStyles.Clear();
				viewBounds = Rect(0, 0, 0, 0);
				Layout_InvalidateItemSizeCache();
				AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
			}

			void GuiVirtualRepeatCompositionBase::InstallItems()
			{
				// nothing needs to be done here
				// visibleStyles will be recreated in the next round of layout
			}

			void GuiVirtualRepeatCompositionBase::UpdateContext()
			{
				for (auto style : visibleStyles)
				{
					style->SetContext(itemContext);
				}
			}

			vint GuiVirtualRepeatCompositionBase::CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize)
			{
				vint visibleCount = expectedSize / itemSize;
				if (count < visibleCount)
				{
					visibleCount = count;
				}
				else if (count > visibleCount)
				{
					vint deltaA = expectedSize - count * itemSize;
					vint deltaB = itemSize - deltaA;
					if (deltaB < deltaA)
					{
						visibleCount++;
					}
				}
				return visibleCount * itemSize;
			}

			GuiVirtualRepeatCompositionBase::ItemStyleRecord GuiVirtualRepeatCompositionBase::CreateStyle(vint index)
			{
				auto source = itemSource->Get(index);
				auto itemStyle = itemTemplate(source);
				itemStyle->SetContext(itemContext);
				AddChild(itemStyle);
				itemStyle->ForceCalculateSizeImmediately();
				return itemStyle;
			}

			void GuiVirtualRepeatCompositionBase::DeleteStyle(ItemStyleRecord style)
			{
				SafeDeleteComposition(style);
			}

			void GuiVirtualRepeatCompositionBase::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
			{
				if (itemTemplate && itemSource)
				{
					vint endIndex = startIndex + visibleStyles.Count() - 1;
					vint newStartIndex = 0;
					vint itemCount = itemSource->GetCount();
					Layout_BeginPlaceItem(true, newBounds, newStartIndex);
					if (newStartIndex < 0) newStartIndex = 0;

					StyleList newVisibleStyles;
					for (vint i = newStartIndex; i < itemCount; i++)
					{
						bool reuseOldStyle = startIndex <= i && i <= endIndex;
						auto style = reuseOldStyle ? visibleStyles[i - startIndex] : CreateStyle(i);
						newVisibleStyles.Add(style);

						Rect bounds;
						Margin alignmentToParent;
						Layout_PlaceItem(true, !reuseOldStyle, i, style, newBounds, bounds, alignmentToParent);
						if (Layout_IsItemCouldBeTheLastVisibleInBounds(i, style, bounds, newBounds))
						{
							break;
						}
					}

					vint newEndIndex = newStartIndex + newVisibleStyles.Count() - 1;
					for (auto [style, i] : indexed(visibleStyles))
					{
						vint index = startIndex + i;
						if (index < newStartIndex || index > newEndIndex)
						{
							DeleteStyle(visibleStyles[i]);
						}
					}
					CopyFrom(visibleStyles, newVisibleStyles);

					if (Layout_EndPlaceItem(true, newBounds, newStartIndex))
					{
						realFullSize = axis->VirtualSizeToRealSize(Layout_CalculateTotalSize());
						TotalSizeChanged.Execute(GuiEventArgs(this));
						AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
					}
					startIndex = newStartIndex;
				}
			}

			void GuiVirtualRepeatCompositionBase::RearrangeItemBounds()
			{
				vint newStartIndex = startIndex;
				Layout_BeginPlaceItem(false, viewBounds, newStartIndex);
				for (auto [style, i] : indexed(visibleStyles))
				{
					Rect bounds;
					Margin alignmentToParent(-1, -1, -1, -1);
					Layout_PlaceItem(false, false, startIndex + i, style, viewBounds, bounds, alignmentToParent);

					bounds.x1 -= viewBounds.x1;
					bounds.x2 -= viewBounds.x1;
					bounds.y1 -= viewBounds.y1;
					bounds.y2 -= viewBounds.y1;

					Layout_SetStyleAlignmentToParent(style, alignmentToParent);
					Layout_SetStyleBounds(style, bounds);
				}
				Layout_EndPlaceItem(false, viewBounds, startIndex);
			}

			GuiVirtualRepeatCompositionBase::GuiVirtualRepeatCompositionBase()
			{
				AxisChanged.SetAssociatedComposition(this);
				TotalSizeChanged.SetAssociatedComposition(this);
				ViewLocationChanged.SetAssociatedComposition(this);
				AdoptedSizeInvalidated.SetAssociatedComposition(this);
			}

			GuiVirtualRepeatCompositionBase::~GuiVirtualRepeatCompositionBase()
			{
			}

			Ptr<IGuiAxis> GuiVirtualRepeatCompositionBase::GetAxis()
			{
				return axis;
			}

			void GuiVirtualRepeatCompositionBase::SetAxis(Ptr<IGuiAxis> value)
			{
				if (axis != value)
				{
					ClearItems();
					if (!value) value = Ptr(new GuiDefaultAxis);
					axis = value;
					if (itemTemplate && itemSource)
					{
						InstallItems();
					}
					AxisChanged.Execute(GuiEventArgs(this));
				}
			}

			Size GuiVirtualRepeatCompositionBase::GetTotalSize()
			{
				return realFullSize;
			}

			Point GuiVirtualRepeatCompositionBase::GetViewLocation()
			{
				return axis->VirtualRectToRealRect(realFullSize, viewBounds).LeftTop();
			}

			void GuiVirtualRepeatCompositionBase::SetViewLocation(Point value)
			{
				Layout_UpdateViewBounds(axis->RealRectToVirtualRect(realFullSize, Rect(value, axis->VirtualSizeToRealSize(viewBounds.GetSize()))));
			}

			GuiVirtualRepeatCompositionBase::ItemStyleRecord GuiVirtualRepeatCompositionBase::GetVisibleStyle(vint itemIndex)
			{
				if (startIndex <= itemIndex && itemIndex < startIndex + visibleStyles.Count())
				{
					return visibleStyles[itemIndex - startIndex];
				}
				else
				{
					return nullptr;
				}
			}

			vint GuiVirtualRepeatCompositionBase::GetVisibleIndex(ItemStyleRecord style)
			{
				for (auto [s, i] : indexed(visibleStyles))
				{
					if (s == style)
					{
						return i + startIndex;
					}
				}
				return -1;
			}

			void GuiVirtualRepeatCompositionBase::ReloadVisibleStyles()
			{
				ClearItems();
			}

/***********************************************************************
GuiRepeatFreeHeightItemComposition
***********************************************************************/

			void GuiRepeatFreeHeightItemComposition::EnsureOffsetForItem(vint itemIndex)
			{
				if (heights.Count() == 0) return;

				if (availableOffsetCount == 0)
				{
					availableOffsetCount = 1;
					offsets[0] = 0;
				}

				for (vint i = availableOffsetCount; i < itemIndex && i < heights.Count(); i++)
				{
					offsets[i] = offsets[i - 1] + heights[i - 1];
				}
			}

			void GuiRepeatFreeHeightItemComposition::Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
			{
				pim_heightUpdated = false;
				EnsureOffsetForItem(heights.Count() - 1);
				if (forMoving)
				{
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < heights.Count(); i++)
					{
						if (offsets[i] + heights[i] >= newBounds.Top())
						{
							newStartIndex = i;
							break;
						}
					}
				}
			}

			void GuiRepeatFreeHeightItemComposition::Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				vint styleHeight = heights[index];
				{
					auto currentBounds = Layout_GetStyleBounds(style);
					Layout_SetStyleBounds(style, Rect(bounds.LeftTop(), Size(viewBounds.Width(), bounds.Height())));
					vint newStyleHeight = Layout_GetStylePreferredSize(style).y;
					Layout_SetStyleBounds(style, currentBounds);

					if (!newCreatedStyle || styleHeight < newStyleHeight)
					{
						styleHeight = newStyleHeight;
					}
				}

				if (heights[index] != styleHeight)
				{
					heights[index] = styleHeight;
					pim_heightUpdated = true;
				}

				vint styleOffset = index == 0 ? 0 : offsets[index - 1] + heights[index - 1];
				if (availableOffsetCount <= index || offsets[index] != styleOffset)
				{
					offsets[index] = styleOffset;
					availableOffsetCount = index;
				}

				bounds = Rect(Point(0, offsets[index]), Size(viewBounds.Width(), heights[index]));
			}

			bool GuiRepeatFreeHeightItemComposition::Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
			{
				return bounds.Bottom() >= viewBounds.Bottom();
			}

			bool GuiRepeatFreeHeightItemComposition::Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
			{
				if (forMoving)
				{
					return pim_heightUpdated;
				}
				return false;
			}

			void GuiRepeatFreeHeightItemComposition::Layout_InvalidateItemSizeCache()
			{
				availableOffsetCount = 0;
				for (vint i = 0; i < heights.Count(); i++)
				{
					heights[i] = 1;
				}
			}

			Size GuiRepeatFreeHeightItemComposition::Layout_CalculateTotalSize()
			{
				if (heights.Count() == 0) return Size(0, 0);
				EnsureOffsetForItem(heights.Count());
				return Size(viewBounds.Width(), offsets[heights.Count() - 1] + heights[heights.Count() - 1]);
			}

			void GuiRepeatFreeHeightItemComposition::OnItemChanged(vint start, vint oldCount, vint newCount)
			{
				availableOffsetCount = start;
				vint itemCount = heights.Count() + newCount - oldCount;

				if (oldCount < newCount)
				{
					heights.Resize(itemCount);
					if (start + newCount < itemCount)
					{
						memmove(&heights[start + newCount], &heights[start + oldCount], sizeof(vint) * (itemCount - start - newCount));
					}
				}
				else if (oldCount > newCount)
				{
					if (start + newCount < itemCount)
					{
						memmove(&heights[start + newCount], &heights[start + oldCount], sizeof(vint) * (itemCount - start - newCount));
					}
					heights.Resize(itemCount);
				}

				for (vint i = 0; i < newCount; i++)
				{
					heights[start + i] = 1;
				}
				offsets.Resize(itemCount);

				GuiVirtualRepeatCompositionBase::OnItemChanged(start, oldCount, newCount);
			}

			vint GuiRepeatFreeHeightItemComposition::FindItem(vint itemIndex, compositions::KeyDirection key)
			{
				if (!itemSource) return -1;
				vint count = itemSource->GetCount();
				if (count == 0) return -1;
				switch (key)
				{
				case KeyDirection::Up:
					itemIndex--;
					break;
				case KeyDirection::Down:
					itemIndex++;
					break;
				case KeyDirection::Home:
					itemIndex = 0;
					break;
				case KeyDirection::End:
					itemIndex = count;
					break;
				case KeyDirection::PageUp:
					itemIndex -= visibleStyles.Count();
					break;
				case KeyDirection::PageDown:
					itemIndex += visibleStyles.Count();
					break;
				default:
					return -1;
				}

				if (itemIndex < 0) return 0;
				else if (itemIndex >= count) return count - 1;
				else return itemIndex;
			}

			VirtualRepeatEnsureItemVisibleResult GuiRepeatFreeHeightItemComposition::EnsureItemVisible(vint itemIndex)
			{
				if (!itemSource) return VirtualRepeatEnsureItemVisibleResult::NotMoved;
				bool moved = false;
				while (true)
				{
					if (itemIndex < 0 || itemIndex >= itemSource->GetCount())
					{
						return VirtualRepeatEnsureItemVisibleResult::ItemNotExists;
					}

					EnsureOffsetForItem(itemIndex);
					vint offset = viewBounds.y1;
					vint top = offsets[itemIndex];
					vint bottom = top + heights[itemIndex];
					vint height = viewBounds.Height();

					Point location = viewBounds.LeftTop();

					if (offset >= top && offset + height <= bottom)
					{
						break;
					}
					else if (offset > top)
					{
						location.y = top;
					}
					else if (offset < bottom - height)
					{
						location.y = bottom - height;
					}
					else
					{
						break;
					}

					auto oldLeftTop = viewBounds.LeftTop();
					Layout_UpdateViewLocation(location);
					moved |= viewBounds.LeftTop() != oldLeftTop;
					if (viewBounds.LeftTop() != location) break;
				}
				return moved ? VirtualRepeatEnsureItemVisibleResult::Moved : VirtualRepeatEnsureItemVisibleResult::NotMoved;
			}

			Size GuiRepeatFreeHeightItemComposition::GetAdoptedSize(Size expectedSize)
			{
				vint h = expectedSize.x * 2;
				if (expectedSize.y < h) expectedSize.y = h;
				return expectedSize;
			}
		}
	}
}