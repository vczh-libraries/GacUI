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

			void GuiVirtualRepeatCompositionBase::Layout_UpdateViewBounds(Rect value, bool forceUpdateTotalSize)
			{
				auto old = GetViewLocation();
				Rect oldBounds = viewBounds;
				viewBounds = value;
				OnViewChangedInternal(oldBounds, value, forceUpdateTotalSize);
				if (old != GetViewLocation())
				{
					ViewLocationChanged.Execute(GuiEventArgs(this));
				}
			}

			void GuiVirtualRepeatCompositionBase::Layout_UpdateViewLocation(Point value)
			{
				Layout_UpdateViewBounds(Rect(value, viewBounds.GetSize()), false);
			}

			Rect GuiVirtualRepeatCompositionBase::Layout_CalculateBounds(Size parentSize)
			{
				auto bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);
				auto size = axis->RealSizeToVirtualSize(bounds.GetSize());
				bool viewBoundsSizeChanged = size != viewBounds.GetSize();
				if (viewBoundsSizeChanged || itemSourceUpdated)
				{
					itemSourceUpdated = false;
					Layout_UpdateViewBounds(Rect(viewBounds.LeftTop(), size), viewBoundsSizeChanged);
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

			void GuiVirtualRepeatCompositionBase::OnClearItems()
			{
				startIndex = 0;
				for (auto style : visibleStyles)
				{
					DeleteStyle(style);
				}
				visibleStyles.Clear();
				viewBounds = Rect({ 0,0 },{ 0,0 });
				OnResetViewLocation();
				itemSourceUpdated = true;
				Layout_InvalidateItemSizeCache();
				AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
			}

			void GuiVirtualRepeatCompositionBase::OnInstallItems()
			{
				// nothing needs to be done here
				// visibleStyles will be recreated in the next round of layout
			}

			void GuiVirtualRepeatCompositionBase::OnUpdateContext()
			{
				for (auto style : visibleStyles)
				{
					style->SetContext(itemContext);
				}
			}

			void GuiVirtualRepeatCompositionBase::OnResetViewLocation()
			{
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

			void GuiVirtualRepeatCompositionBase::OnViewChangedInternal(Rect oldBounds, Rect newBounds, bool forceUpdateTotalSize)
			{
				bool needToUpdateTotalSize = forceUpdateTotalSize;

				if (itemTemplate && itemSource)
				{
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

						needToUpdateTotalSize = Layout_EndPlaceItem(true, newBounds, newStartIndex) || needToUpdateTotalSize;
						startIndex = newStartIndex;
					}
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

						needToUpdateTotalSize = Layout_EndPlaceItem(false, viewBounds, startIndex) || needToUpdateTotalSize;
					}
				}

				if (needToUpdateTotalSize)
				{
					realFullSize = axis->VirtualSizeToRealSize(Layout_CalculateTotalSize());
					TotalSizeChanged.Execute(GuiEventArgs(this));
					AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
				}
				Layout_EndLayout(needToUpdateTotalSize);
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
					OnClearItems();
					if (!value) value = Ptr(new GuiDefaultAxis);
					axis = value;
					if (itemTemplate && itemSource)
					{
						OnInstallItems();
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
				Size realSize = axis->VirtualSizeToRealSize(viewBounds.GetSize());
				Rect realBounds = Rect(value, realSize);
				Layout_UpdateViewBounds(axis->RealRectToVirtualRect(realFullSize, realBounds), false);
				OnResetViewLocation();
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
				OnClearItems();
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
				pi_heightUpdated = false;
				EnsureOffsetForItem(heights.Count() - 1);
				if (forMoving)
				{
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < heights.Count(); i++)
					{
						if (heights[i] == 1 && startIndex <= i && i < startIndex + visibleStyles.Count() && visibleStyles[i - startIndex])
						{
							vint h = visibleStyles[i - startIndex]->GetCachedMinSize().y;
							if (h > 1)
							{
								heights[i] = h;
							}
						}
						if (offsets[i] + heights[i] > newBounds.Top())
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
					vint newStyleHeight = Layout_GetStylePreferredSize(style).y;
					if (!newCreatedStyle || styleHeight < newStyleHeight)
					{
						styleHeight = newStyleHeight;
					}
				}

				if (heights[index] != styleHeight)
				{
					heights[index] = styleHeight;
					pi_heightUpdated = true;
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
				return pi_heightUpdated;
			}

			void GuiRepeatFreeHeightItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
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

			void GuiRepeatFreeHeightItemComposition::OnInstallItems()
			{
				heights.Resize(itemSource->GetCount());
				Layout_InvalidateItemSizeCache();

				offsets.Resize(itemSource->GetCount());
				EnsureOffsetForItem(heights.Count() - 1);

				GuiVirtualRepeatCompositionBase::OnInstallItems();
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

/***********************************************************************
GuiRepeatFixedHeightItemComposition
***********************************************************************/

			vint GuiRepeatFixedHeightItemComposition::GetWidth()
			{
				return -1;
			}

			vint GuiRepeatFixedHeightItemComposition::GetYOffset()
			{
				return 0;
			}

			void GuiRepeatFixedHeightItemComposition::Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
			{
				pi_width = GetWidth();
				if (forMoving)
				{
					pi_rowHeight = rowHeight;
					newStartIndex = (newBounds.Top() - GetYOffset()) / pi_rowHeight;
				}
			}

			void GuiRepeatFixedHeightItemComposition::Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				if (forMoving)
				{
					vint styleHeight = Layout_GetStylePreferredSize(style).y;
					if (pi_rowHeight < styleHeight)
					{
						pi_rowHeight = styleHeight;
					}
				}

				vint top = GetYOffset() + index * pi_rowHeight;
				if (pi_width == -1)
				{
					alignmentToParent = Margin(0, -1, 0, -1);
					bounds = Rect(Point(0, top), Size(0, pi_rowHeight));
				}
				else
				{
					alignmentToParent = Margin(-1, -1, -1, -1);
					bounds = Rect(Point(0, top), Size(pi_width, pi_rowHeight));
				}
			}

			bool GuiRepeatFixedHeightItemComposition::Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
			{
				return bounds.Bottom() >= viewBounds.Bottom();
			}

			bool GuiRepeatFixedHeightItemComposition::Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
			{
				if (forMoving)
				{
					if (pi_rowHeight != rowHeight)
					{
						rowHeight = pi_rowHeight;
						return true;
					}
				}
				return false;
			}

			void GuiRepeatFixedHeightItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedHeightItemComposition::Layout_InvalidateItemSizeCache()
			{
				rowHeight = 1;
			}

			Size GuiRepeatFixedHeightItemComposition::Layout_CalculateTotalSize()
			{
				if (!itemSource || itemSource->GetCount() == 0) return Size(0, 0);

				vint width = GetWidth();
				if (width == -1) width = viewBounds.Width();
				return Size(width, rowHeight * itemSource->GetCount() + GetYOffset());
			}

			vint GuiRepeatFixedHeightItemComposition::FindItem(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				if (count == 0) return -1;
				vint groupCount = viewBounds.Height() / rowHeight;
				if (groupCount == 0) groupCount = 1;
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
					itemIndex -= groupCount;
					break;
				case KeyDirection::PageDown:
					itemIndex += groupCount;
					break;
				default:
					return -1;
				}

				if (itemIndex < 0) return 0;
				else if (itemIndex >= count) return count - 1;
				else return itemIndex;
			}

			VirtualRepeatEnsureItemVisibleResult GuiRepeatFixedHeightItemComposition::EnsureItemVisible(vint itemIndex)
			{
				if (!itemSource) return VirtualRepeatEnsureItemVisibleResult::NotMoved;
				if (itemIndex < 0 || itemIndex >= itemSource->GetCount())
				{
					return VirtualRepeatEnsureItemVisibleResult::ItemNotExists;
				}

				vint yOffset = GetYOffset();
				vint viewY1 = viewBounds.y1 + yOffset;
				vint viewY2 = viewBounds.y2;
				vint itemY1 = itemIndex * rowHeight + yOffset;
				vint itemY2 = itemY1 + rowHeight;

				if (viewY2 - viewY1 < rowHeight)
				{
					if (itemY1 < viewY2 && itemY2 > viewY1)
					{
						return VirtualRepeatEnsureItemVisibleResult::NotMoved;
					}
				}
				else
				{
					if (itemY1 >= viewY1 && itemY2 <= viewY2)
					{
						return VirtualRepeatEnsureItemVisibleResult::NotMoved;
					}

					if (itemY1 < viewY1 && itemY2 > viewY1)
					{
						Layout_UpdateViewLocation({ viewBounds.x1,viewBounds.y1 + itemY1 - viewY1 });
						return VirtualRepeatEnsureItemVisibleResult::Moved;
					}

					if (itemY1 < viewY2 && itemY2 > viewY2)
					{
						Layout_UpdateViewLocation({ viewBounds.x1,viewBounds.y1 + itemY2 - viewY2 });
						return VirtualRepeatEnsureItemVisibleResult::Moved;
					}
				}

				bool up = itemY1 < viewY1;
				while (true)
				{
					if (up)
					{
						if (itemY1 >= viewY1) break;
						Layout_UpdateViewLocation({ viewBounds.x1,viewBounds.y1 + itemY1 - viewY1 });
					}
					else
					{
						if (itemY2 <= viewY2) break;
						Layout_UpdateViewLocation({ viewBounds.x1,viewBounds.y1 + itemY2 - viewY2 });
					}

					viewY1 = viewBounds.y1 + yOffset;
					viewY2 = viewBounds.y2;
					itemY1 = itemIndex * rowHeight + yOffset;
					itemY2 = itemY1 + rowHeight;
				}

				return VirtualRepeatEnsureItemVisibleResult::Moved;
			}

			Size GuiRepeatFixedHeightItemComposition::GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint yOffset = GetYOffset();
				vint y = expectedSize.y - yOffset;
				vint itemCount = itemSource->GetCount();
				return Size(expectedSize.x, yOffset + CalculateAdoptedSize(y, itemCount, rowHeight));
			}

/***********************************************************************
GuiRepeatFixedSizeMultiColumnItemComposition
***********************************************************************/

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
			{
				if (forMoving)
				{
					pi_itemSize = itemSize;
					vint rows = newBounds.Top() / pi_itemSize.y;
					if (rows < 0) rows = 0;
					vint cols = newBounds.Width() / pi_itemSize.x;
					if (cols < 1) cols = 1;
					newStartIndex = rows * cols;
				}
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				if (forMoving)
				{
					Size styleSize = Layout_GetStylePreferredSize(style);
					if (pi_itemSize.x < styleSize.x) pi_itemSize.x = styleSize.x;
					if (pi_itemSize.y < styleSize.y) pi_itemSize.y = styleSize.y;
				}

				vint rowItems = viewBounds.Width() / pi_itemSize.x;
				if (rowItems < 1) rowItems = 1;

				vint row = index / rowItems;
				vint col = index % rowItems;
				bounds = Rect(Point(col * pi_itemSize.x, row * pi_itemSize.y), pi_itemSize);
			}

			bool GuiRepeatFixedSizeMultiColumnItemComposition::Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
			{
				vint rowItems = viewBounds.Width() / pi_itemSize.x;
				vint col = index % rowItems;
				return col == rowItems - 1 && bounds.Bottom() >= viewBounds.Bottom();
			}

			bool GuiRepeatFixedSizeMultiColumnItemComposition::Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
			{
				if (forMoving)
				{
					if (pi_itemSize != itemSize)
					{
						itemSize = pi_itemSize;
						return true;
					}
				}
				return false;
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_InvalidateItemSizeCache()
			{
				itemSize = Size(1, 1);
			}

			Size GuiRepeatFixedSizeMultiColumnItemComposition::Layout_CalculateTotalSize()
			{
				if (!itemSource || itemSource->GetCount() == 0) return Size(0, 0);

				vint rowItems = viewBounds.Width() / itemSize.x;
				if (rowItems < 1) rowItems = 1;
				vint rows = itemSource->GetCount() / rowItems;
				if (itemSource->GetCount() % rowItems) rows++;

				return Size(itemSize.x * rowItems, itemSize.y * rows);
			}

			vint GuiRepeatFixedSizeMultiColumnItemComposition::FindItem(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				vint columnCount = viewBounds.Width() / itemSize.x;
				if (columnCount == 0) columnCount = 1;
				vint rowCount = viewBounds.Height() / itemSize.y;
				if (rowCount == 0) rowCount = 1;

				switch (key)
				{
				case KeyDirection::Up:
					itemIndex -= columnCount;
					break;
				case KeyDirection::Down:
					itemIndex += columnCount;
					break;
				case KeyDirection::Left:
					itemIndex--;
					break;
				case KeyDirection::Right:
					itemIndex++;
					break;
				case KeyDirection::Home:
					itemIndex = 0;
					break;
				case KeyDirection::End:
					itemIndex = count;
					break;
				case KeyDirection::PageUp:
					itemIndex -= columnCount * rowCount;
					break;
				case KeyDirection::PageDown:
					itemIndex += columnCount * rowCount;
					break;
				case KeyDirection::PageLeft:
					itemIndex -= itemIndex % columnCount;
					break;
				case KeyDirection::PageRight:
					itemIndex += columnCount - itemIndex % columnCount - 1;
					break;
				default:
					return -1;
				}

				if (itemIndex < 0) return 0;
				else if (itemIndex >= count) return count - 1;
				else return itemIndex;
			}

			VirtualRepeatEnsureItemVisibleResult GuiRepeatFixedSizeMultiColumnItemComposition::EnsureItemVisible(vint itemIndex)
			{
				if (!itemSource) return VirtualRepeatEnsureItemVisibleResult::NotMoved;
				if (itemIndex < 0 || itemIndex >= itemSource->GetCount())
				{
					return VirtualRepeatEnsureItemVisibleResult::ItemNotExists;
				}
				bool moved = false;
				while (true)
				{
					vint rowHeight = itemSize.y;
					vint columnCount = viewBounds.Width() / itemSize.x;
					if (columnCount == 0) columnCount = 1;
					vint rowIndex = itemIndex / columnCount;

					vint top = rowIndex * rowHeight;
					vint bottom = top + rowHeight;

					if (viewBounds.Height() < rowHeight)
					{
						if (viewBounds.Top() < bottom && top < viewBounds.Bottom())
						{
							break;
						}
					}

					Point location = viewBounds.LeftTop();
					if (top < viewBounds.Top())
					{
						location.y = top;
					}
					else if (viewBounds.Bottom() < bottom)
					{
						location.y = bottom - viewBounds.Height();
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

			Size GuiRepeatFixedSizeMultiColumnItemComposition::GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint count = itemSource->GetCount();
				vint columnCount = viewBounds.Width() / itemSize.x;
				vint rowCount = viewBounds.Height() / itemSize.y;
				return Size(
					CalculateAdoptedSize(expectedSize.x, columnCount, itemSize.x),
					CalculateAdoptedSize(expectedSize.y, rowCount, itemSize.y)
				);
			}

/***********************************************************************
GuiRepeatFixedHeightMultiColumnItemComposition
***********************************************************************/

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
			{
				pi_currentWidth = 0;
				pi_totalWidth = 0;
				if (forMoving)
				{
					vint w = newBounds.Width();
					vint h = newBounds.Height();
					if (w <= 0) w = 1;

					pi_itemHeight = itemHeight;
					vint rows = h / itemHeight;
					if (rows < 1) rows = 1;
					vint columns = newBounds.Left() / w;
					newStartIndex = rows * columns;
				}
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				vint rows = viewBounds.Height() / itemHeight;
				if (rows < 1) rows = 1;

				vint row = index % rows;
				if (row == 0)
				{
					pi_totalWidth += pi_currentWidth;
					pi_currentWidth = 0;
				}

				Size styleSize = Layout_GetStylePreferredSize(style);
				if (pi_currentWidth < styleSize.x) pi_currentWidth = styleSize.x;
				bounds = Rect(Point(pi_totalWidth + viewBounds.Left(), itemHeight * row), Size(0, 0));
				if (forMoving)
				{
					if (pi_itemHeight < styleSize.y) pi_itemHeight = styleSize.y;
				}
			}

			bool GuiRepeatFixedHeightMultiColumnItemComposition::Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
			{
				return bounds.Left() >= viewBounds.Right();
			}

			bool GuiRepeatFixedHeightMultiColumnItemComposition::Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
			{
				if (forMoving)
				{
					if (pi_itemHeight != itemHeight)
					{
						itemHeight = pi_itemHeight;
						return true;
					}
				}
				return false;
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_InvalidateItemSizeCache()
			{
				itemHeight = 1;
			}

			Size GuiRepeatFixedHeightMultiColumnItemComposition::Layout_CalculateTotalSize()
			{
				if (!itemSource || itemSource->GetCount() == 0) return Size(0, 0);

				vint rows = viewBounds.Height() / itemHeight;
				if (rows < 1) rows = 1;
				vint columns = (itemSource->GetCount() + rows - 1) / rows;

				return Size(viewBounds.Width() * (columns + 1), 0);
			}

			vint GuiRepeatFixedHeightMultiColumnItemComposition::FindItem(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				vint groupCount = viewBounds.Height() / itemHeight;
				if (groupCount == 0) groupCount = 1;
				switch (key)
				{
				case KeyDirection::Up:
					itemIndex--;
					break;
				case KeyDirection::Down:
					itemIndex++;
					break;
				case KeyDirection::Left:
					itemIndex -= groupCount;
					break;
				case KeyDirection::Right:
					itemIndex += groupCount;
					break;
				case KeyDirection::Home:
					itemIndex = 0;
					break;
				case KeyDirection::End:
					itemIndex = count;
					break;
				case KeyDirection::PageUp:
					itemIndex -= itemIndex % groupCount;
					break;
				case KeyDirection::PageDown:
					itemIndex += groupCount - itemIndex % groupCount - 1;
					break;
				default:
					return -1;
				}

				if (itemIndex < 0) return 0;
				else if (itemIndex >= count) return count - 1;
				else return itemIndex;
			}

			VirtualRepeatEnsureItemVisibleResult GuiRepeatFixedHeightMultiColumnItemComposition::EnsureItemVisible(vint itemIndex)
			{
				if (!itemSource) return VirtualRepeatEnsureItemVisibleResult::NotMoved;
				if (itemIndex < 0 || itemIndex >= itemSource->GetCount())
				{
					return VirtualRepeatEnsureItemVisibleResult::ItemNotExists;
				}
				bool moved = false;
				while (true)
				{
					vint rowCount = viewBounds.Height() / itemHeight;
					if (rowCount == 0) rowCount = 1;
					vint columnIndex = itemIndex / rowCount;
					vint minIndex = startIndex;
					vint maxIndex = startIndex + visibleStyles.Count() - 1;

					Point location = viewBounds.LeftTop();
					if (minIndex <= itemIndex && itemIndex <= maxIndex)
					{
						Rect bounds = Layout_GetStyleBounds(visibleStyles[itemIndex - startIndex]);
						if (0 < bounds.Bottom() && bounds.Top() < viewBounds.Width() && bounds.Width() > viewBounds.Width())
						{
							break;
						}
						else if (bounds.Left() < 0)
						{
							location.x -= viewBounds.Width();
						}
						else if (bounds.Right() > viewBounds.Width())
						{
							location.x += viewBounds.Width();
						}
						else
						{
							break;
						}
					}
					else if (columnIndex < minIndex / rowCount)
					{
						location.x -= viewBounds.Width();
					}
					else if (columnIndex >= maxIndex / rowCount)
					{
						location.x += viewBounds.Width();
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

			Size GuiRepeatFixedHeightMultiColumnItemComposition::GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint count = itemSource->GetCount();
				return Size(expectedSize.x, CalculateAdoptedSize(expectedSize.y, count, itemHeight));
			}
		}
	}
}