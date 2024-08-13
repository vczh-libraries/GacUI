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
				if (size != viewBounds.GetSize() || itemSourceUpdated)
				{
					itemSourceUpdated = false;
					Layout_UpdateViewBounds(Rect(viewBounds.LeftTop(), size), true);
				}
				return bounds;
			}

			void GuiVirtualRepeatCompositionBase::Layout_ResetLayout()
			{
				viewBounds = Rect({ 0,0 }, { 0,0 });
				ViewLocationChanged.Execute(GuiEventArgs(this));
				OnResetViewLocation();
				itemSourceUpdated = true;
				Layout_InvalidateItemSizeCache();
				AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
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

			void GuiVirtualRepeatCompositionBase::OnStyleCachedMinSizeChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				InvalidateLayout();
			}

			void GuiVirtualRepeatCompositionBase::AttachEventHandler(GuiGraphicsComposition* itemStyle)
			{
				eventHandlers.Add(itemStyle, itemStyle->CachedMinSizeChanged.AttachMethod(this, &GuiVirtualRepeatCompositionBase::OnStyleCachedMinSizeChanged));
			}

			void GuiVirtualRepeatCompositionBase::DetachEventHandler(GuiGraphicsComposition* itemStyle)
			{
				vint index = eventHandlers.Keys().IndexOf(itemStyle);
				if (index != -1)
				{
					auto eventHandler = eventHandlers.Values()[index];
					itemStyle->CachedBoundsChanged.Detach(eventHandler);
					eventHandlers.Remove(itemStyle);
				}
			}

			void GuiVirtualRepeatCompositionBase::OnChildRemoved(GuiGraphicsComposition* child)
			{
				DetachEventHandler(child);
				GuiBoundsComposition::OnChildRemoved(child);
			}

			void GuiVirtualRepeatCompositionBase::OnItemChanged(vint start, vint oldCount, vint newCount)
			{
				itemSourceUpdated = true;
				InvokeOnCompositionStateChanged();

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
					Layout_UpdateIndex(style, startIndex + i);
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
				Layout_ResetLayout();
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

			GuiVirtualRepeatCompositionBase::ItemStyleRecord GuiVirtualRepeatCompositionBase::CreateStyleInternal(vint index)
			{
				auto source = itemSource->Get(index);
				auto itemStyle = itemTemplate(source);
				itemStyle->SetContext(itemContext);
				return itemStyle;
			}

			void GuiVirtualRepeatCompositionBase::DeleteStyleInternal(ItemStyleRecord style)
			{
				SafeDeleteComposition(style);
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
					vint deltaA = expectedSize - visibleCount * itemSize;
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
				auto itemStyle = CreateStyleInternal(index);
				AddChild(itemStyle);
				itemStyle->ForceCalculateSizeImmediately();
				AttachEventHandler(itemStyle);
				return itemStyle;
			}

			void GuiVirtualRepeatCompositionBase::DeleteStyle(ItemStyleRecord style)
			{
				DetachEventHandler(style);
				DeleteStyleInternal(style);
			}

			void GuiVirtualRepeatCompositionBase::UpdateFullSize()
			{
				Size fullSize, minimumSize;
				Layout_CalculateTotalSize(fullSize, minimumSize);
				realFullSize = axis->VirtualSizeToRealSize(fullSize);
				realMinimumFullSize = axis->VirtualSizeToRealSize(minimumSize);
			}

			void GuiVirtualRepeatCompositionBase::OnViewChangedInternal(Rect oldBounds, Rect newBounds, bool forceUpdateTotalSize)
			{
				bool needToUpdateTotalSize = forceUpdateTotalSize;

				if (itemTemplate && itemSource)
				{
					while (true)
					{
						bool needRestart = false;

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
							auto placeItemResult = Layout_PlaceItem(true, !reuseOldStyle, i, style, newBounds, bounds, alignmentToParent);

							if (placeItemResult != VirtualRepeatPlaceItemResult::None)
							{
								needRestart = placeItemResult == VirtualRepeatPlaceItemResult::Restart;
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

						needToUpdateTotalSize = (Layout_EndPlaceItem(true, newBounds, newStartIndex) == VirtualRepeatEndPlaceItemResult::TotalSizeUpdated) || needToUpdateTotalSize;
						startIndex = newStartIndex;

						if (!needRestart) break;
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

						needToUpdateTotalSize = (Layout_EndPlaceItem(false, viewBounds, startIndex) == VirtualRepeatEndPlaceItemResult::TotalSizeUpdated) || needToUpdateTotalSize;
					}
				}
				else if (realFullSize != Size(0, 0))
				{
					needToUpdateTotalSize = true;
				}

				if (needToUpdateTotalSize)
				{
					UpdateFullSize();
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
				for (auto [style, eventHandler] : eventHandlers)
				{
					style->CachedMinSizeChanged.Detach(eventHandler);
				}
				eventHandlers.Clear();
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
				return useMinimumFullSize ? realMinimumFullSize : realFullSize;
			}

			bool GuiVirtualRepeatCompositionBase::GetUseMinimumTotalSize()
			{
				return useMinimumFullSize;
			}

			void GuiVirtualRepeatCompositionBase::SetUseMinimumTotalSize(bool value)
			{
				if (useMinimumFullSize != value)
				{
					useMinimumFullSize = value;
					UpdateFullSize();
					TotalSizeChanged.Execute(GuiEventArgs(this));
				}
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

			void GuiVirtualRepeatCompositionBase::ResetLayout(bool recreateVisibleStyles)
			{
				if (recreateVisibleStyles)
				{
					OnClearItems();
				}
				else
				{
					Layout_ResetLayout();
				}
			}

			void GuiVirtualRepeatCompositionBase::InvalidateLayout()
			{
				itemSourceUpdated = true;
			}

			Size GuiVirtualRepeatCompositionBase::GetAdoptedSize(Size expectedSize)
			{
				Size expectedViewSize = axis->RealSizeToVirtualSize(expectedSize);
				Size adoptedViewSize = Layout_GetAdoptedSize(expectedViewSize);
				Size adoptedSize = axis->VirtualSizeToRealSize(adoptedViewSize);
				return adoptedSize;
			}

			vint GuiVirtualRepeatCompositionBase::FindItemByRealKeyDirection(vint itemIndex, compositions::KeyDirection key)
			{
				return FindItemByVirtualKeyDirection(itemIndex, axis->RealKeyDirectionToVirtualKeyDirection(key));
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

			void GuiRepeatFreeHeightItemComposition::Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)
			{
				pi_heightUpdated = false;
				EnsureOffsetForItem(heights.Count() - 1);
				if (firstPhase)
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

			VirtualRepeatPlaceItemResult GuiRepeatFreeHeightItemComposition::Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
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

				if (bounds.Bottom() >= viewBounds.Bottom())
				{
					return VirtualRepeatPlaceItemResult::HitLastItem;
				}
				else
				{
					return VirtualRepeatPlaceItemResult::None;
				}
			}

			VirtualRepeatEndPlaceItemResult GuiRepeatFreeHeightItemComposition::Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)
			{
				return pi_heightUpdated ? VirtualRepeatEndPlaceItemResult::TotalSizeUpdated : VirtualRepeatEndPlaceItemResult::None;
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

			void GuiRepeatFreeHeightItemComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
			{
				if (heights.Count() == 0)
				{
					full = minimum = Size(0, 0);
					return;
				}

				EnsureOffsetForItem(heights.Count());
				vint w = viewBounds.Width();
				vint h = offsets[heights.Count() - 1] + heights[heights.Count() - 1];
				full = Size(w, h);
				minimum = Size(0, h);
			}

			Size GuiRepeatFreeHeightItemComposition::Layout_GetAdoptedSize(Size expectedSize)
			{
				vint h = expectedSize.x * 2;
				if (expectedSize.y < h) expectedSize.y = h;
				return expectedSize;
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

			vint GuiRepeatFreeHeightItemComposition::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
			{
				if (!itemSource) return -1;
				vint count = itemSource->GetCount();
				if (itemIndex < 0 || itemIndex >= count) return -1;

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
					itemIndex = count - 1;
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

/***********************************************************************
GuiRepeatFixedHeightItemComposition
***********************************************************************/

			void GuiRepeatFixedHeightItemComposition::Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)
			{
				pi_width = itemWidth;
				pi_yoffset = itemYOffset;
				if (firstPhase)
				{
					pi_rowHeight = rowHeight;
					newStartIndex = newBounds.Top() / pi_rowHeight;
				}
			}

			VirtualRepeatPlaceItemResult GuiRepeatFixedHeightItemComposition::Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				if (firstPhase)
				{
					vint styleHeight = Layout_GetStylePreferredSize(style).y;
					if (pi_rowHeight < styleHeight)
					{
						pi_rowHeight = styleHeight;
					}
				}

				vint top = pi_yoffset + index * pi_rowHeight;
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

				if (bounds.Bottom() >= viewBounds.Bottom())
				{
					return VirtualRepeatPlaceItemResult::HitLastItem;
				}
				else
				{
					return VirtualRepeatPlaceItemResult::None;
				}
			}

			VirtualRepeatEndPlaceItemResult GuiRepeatFixedHeightItemComposition::Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)
			{
				if (firstPhase)
				{
					if (pi_rowHeight != rowHeight)
					{
						rowHeight = pi_rowHeight;
						return VirtualRepeatEndPlaceItemResult::TotalSizeUpdated;
					}
				}
				return VirtualRepeatEndPlaceItemResult::None;
			}

			void GuiRepeatFixedHeightItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedHeightItemComposition::Layout_InvalidateItemSizeCache()
			{
				rowHeight = 1;
			}

			void GuiRepeatFixedHeightItemComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
			{
				if (!itemSource || itemSource->GetCount() == 0)
				{
					full = minimum = Size(0, 0);
					return;
				}

				vint w = itemWidth;
				vint w1 = w == -1 ? viewBounds.Width() : w;
				vint w2 = w == -1 ? 0 : w;
				vint h = rowHeight * itemSource->GetCount() + itemYOffset;
				full = Size(w1, h);
				minimum = Size(w2, h);
			}

			Size GuiRepeatFixedHeightItemComposition::Layout_GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint y = expectedSize.y - itemYOffset;
				vint itemCount = itemSource->GetCount();
				return Size(expectedSize.x, itemYOffset + CalculateAdoptedSize(y, itemCount, rowHeight));
			}

			vint GuiRepeatFixedHeightItemComposition::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				if (itemIndex < 0 || itemIndex >= count) return -1;
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

				vint viewY1 = viewBounds.y1 + itemYOffset;
				vint viewY2 = viewBounds.y2;
				vint itemY1 = itemIndex * rowHeight + itemYOffset;
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

					viewY1 = viewBounds.y1 + itemYOffset;
					viewY2 = viewBounds.y2;
					itemY1 = itemIndex * rowHeight + itemYOffset;
					itemY2 = itemY1 + rowHeight;
				}

				return VirtualRepeatEnsureItemVisibleResult::Moved;
			}

			vint GuiRepeatFixedHeightItemComposition::GetItemWidth()
			{
				return itemWidth;
			}

			void GuiRepeatFixedHeightItemComposition::SetItemWidth(vint value)
			{
				if (value < -1) value = -1;
				if (itemWidth != value)
				{
					itemWidth = value;
					InvalidateLayout();
				}
			}

			vint GuiRepeatFixedHeightItemComposition::GetItemYOffset()
			{
				return itemYOffset;
			}

			void GuiRepeatFixedHeightItemComposition::SetItemYOffset(vint value)
			{
				if (value < 0) value = 0;
				if (itemYOffset != value)
				{
					itemYOffset = value;
					InvalidateLayout();
				}
			}

/***********************************************************************
GuiRepeatFixedSizeMultiColumnItemComposition
***********************************************************************/

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)
			{
				if (firstPhase)
				{
					pi_itemSize = itemSize;
					vint rows = newBounds.Top() / pi_itemSize.y;
					if (rows < 0) rows = 0;
					vint cols = newBounds.Width() / pi_itemSize.x;
					if (cols < 1) cols = 1;
					newStartIndex = rows * cols;
				}
			}

			VirtualRepeatPlaceItemResult GuiRepeatFixedSizeMultiColumnItemComposition::Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
				if (firstPhase)
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

				if (col == rowItems - 1 && bounds.Bottom() >= viewBounds.Bottom())
				{
					return VirtualRepeatPlaceItemResult::HitLastItem;
				}
				else
				{
					return VirtualRepeatPlaceItemResult::None;
				}
			}

			VirtualRepeatEndPlaceItemResult GuiRepeatFixedSizeMultiColumnItemComposition::Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)
			{
				if (firstPhase)
				{
					if (pi_itemSize != itemSize)
					{
						itemSize = pi_itemSize;
						return VirtualRepeatEndPlaceItemResult::TotalSizeUpdated;
					}
				}
				return VirtualRepeatEndPlaceItemResult::None;
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_InvalidateItemSizeCache()
			{
				itemSize = Size(1, 1);
			}

			void GuiRepeatFixedSizeMultiColumnItemComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
			{
				if (!itemSource || itemSource->GetCount() == 0)
				{
					full = minimum = Size(0, 0);
					return;
				}

				vint rowItems = viewBounds.Width() / itemSize.x;
				if (rowItems < 1) rowItems = 1;
				vint rows = itemSource->GetCount() / rowItems;
				if (itemSource->GetCount() % rowItems) rows++;

				vint h = itemSize.y * rows;
				full = Size(itemSize.x * rowItems, h);
				minimum = Size(itemSize.x, h);
			}

			Size GuiRepeatFixedSizeMultiColumnItemComposition::Layout_GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint count = itemSource->GetCount();
				vint columnCount = viewBounds.Width() / itemSize.x;
				vint rowCount = count / columnCount;
				if (count % columnCount != 0) rowCount++;

				if (columnCount == 0) columnCount = 1;
				if (rowCount == 0) rowCount = 1;
				return Size(
					CalculateAdoptedSize(expectedSize.x, columnCount, itemSize.x),
					CalculateAdoptedSize(expectedSize.y, rowCount, itemSize.y)
				);
			}

			vint GuiRepeatFixedSizeMultiColumnItemComposition::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				if (itemIndex < 0 || itemIndex >= count) return -1;
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

/***********************************************************************
GuiRepeatFixedHeightMultiColumnItemComposition
***********************************************************************/

			void GuiRepeatFixedHeightMultiColumnItemComposition::FixColumnWidth(vint index)
			{
				vint c = index / pi_rows - pi_firstColumn;
				vint r = index % pi_rows;
				vint w = pi_visibleItemWidths[index - pi_firstColumn * pi_rows];

				if (r == 0)
				{
					while (pi_visibleColumnWidths.Count() <= c) pi_visibleColumnWidths.Add(0);
					while (pi_visibleColumnOffsets.Count() <= c) pi_visibleColumnOffsets.Add(0);

					pi_visibleColumnWidths[c] = w;
					if (c == 0)
					{
						pi_visibleColumnOffsets[c] = 0;
					}
					else
					{
						pi_visibleColumnOffsets[c] = pi_visibleColumnOffsets[c - 1] + pi_visibleColumnWidths[c - 1];
					}
				}
				else
				{
					if (pi_visibleColumnWidths[c] < w)
					{
						pi_visibleColumnWidths[c] = w;
					}
				}
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)
			{
				if (firstPhase)
				{
					pi_firstColumn = newBounds.Width() == 0 ? 0 : newBounds.x1 / newBounds.Width();
					pi_itemHeight = itemHeight;

					pi_visibleItemWidths.Clear();
					pi_visibleColumnWidths.Clear();
					pi_visibleColumnOffsets.Clear();
					pi_rows = newBounds.Height() / itemHeight;
					if (pi_rows < 1) pi_rows = 1;

					if (pi_firstColumn < 0)
					{
						pi_firstColumn = 0;
					}
					else if (pi_firstColumn * pi_rows >= itemSource->GetCount())
					{
						pi_firstColumn = (itemSource->GetCount() + pi_rows - 1) / pi_rows - 1;
					}

					newStartIndex = pi_firstColumn * pi_rows;
				}
			}

			VirtualRepeatPlaceItemResult GuiRepeatFixedHeightMultiColumnItemComposition::Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
			{
#define ERROR_MESSAGE_INTERNAL_ERROR L"vl::presentation::compositions::GuiRepeatFixedHeightMultiColumnItemComposition::Layout_PlaceItem(...)#Internal error."

				vint visibleColumn = index / pi_rows - pi_firstColumn;
				vint visibleRow = index % pi_rows;

				if (firstPhase)
				{
					Size styleSize = Layout_GetStylePreferredSize(style);
					if (pi_itemHeight < styleSize.y)
					{
						pi_itemHeight = styleSize.y;
						vint newRows = viewBounds.Height() / pi_itemHeight;
						if (newRows != pi_rows)
						{
							CHECK_ERROR(newRows < pi_rows, ERROR_MESSAGE_INTERNAL_ERROR);
							vint oldFirstIndex = pi_firstColumn * pi_rows;
							pi_rows = newRows > 0 ? newRows : 1;
							vint newFirstIndex = pi_firstColumn * pi_rows;

							if (oldFirstIndex == newFirstIndex)
							{
								for (vint i = newFirstIndex; i < index; i++)
								{
									FixColumnWidth(i + newFirstIndex);
								}
							}
							else
							{
								CHECK_ERROR(oldFirstIndex > newFirstIndex, ERROR_MESSAGE_INTERNAL_ERROR);
								return VirtualRepeatPlaceItemResult::Restart;
							}

							visibleColumn = index / pi_rows - pi_firstColumn;
							visibleRow = index % pi_rows;
						}
					}

					pi_visibleItemWidths.Add(styleSize.x);
					FixColumnWidth(index);
				}

				vint x = viewBounds.x1 + pi_visibleColumnOffsets[visibleColumn];
				vint y = pi_itemHeight * visibleRow;
				vint w = pi_visibleItemWidths[index - pi_firstColumn * pi_rows];
				bounds = Rect({ x,y }, { w,pi_itemHeight });

				if (visibleRow == pi_rows - 1 && pi_visibleColumnOffsets[visibleColumn] + pi_visibleColumnWidths[visibleColumn] >= viewBounds.Width())
				{
					return VirtualRepeatPlaceItemResult::HitLastItem;
				}
				else
				{
					return VirtualRepeatPlaceItemResult::None;
				}
#undef ERROR_MESSAGE_INTERNAL_ERROR
			}

			VirtualRepeatEndPlaceItemResult GuiRepeatFixedHeightMultiColumnItemComposition::Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)
			{
				if (firstPhase)
				{
					bool itemHeightUpdated = pi_itemHeight != itemHeight;

					firstColumn = pi_firstColumn;
					itemHeight = pi_itemHeight;
					if (pi_visibleColumnOffsets.Count() <= 1)
					{
						fullVisibleColumns = pi_visibleColumnOffsets.Count();
					}
					else
					{
						vint c = pi_visibleColumnOffsets.Count() - 1;
						vint x = pi_visibleColumnOffsets[c] + pi_visibleColumnWidths[c];
						if (x <= viewBounds.Width())
						{
							fullVisibleColumns = c + 1;
						}
						else
						{
							fullVisibleColumns = c;
						}
					}

					if (itemHeightUpdated)
					{
						return VirtualRepeatEndPlaceItemResult::TotalSizeUpdated;
					}
				}
				return VirtualRepeatEndPlaceItemResult::None;
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_EndLayout(bool totalSizeUpdated)
			{
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_InvalidateItemSizeCache()
			{
				itemHeight = 1;
			}

			void GuiRepeatFixedHeightMultiColumnItemComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
			{
				if (!itemSource || itemSource->GetCount() == 0)
				{
					full = minimum = Size(0, 0);
					return;
				}

				vint rows = viewBounds.Height() / itemHeight;
				if (rows < 1) rows = 1;
				vint columns = (itemSource->GetCount() + rows - 1) / rows;

				vint w = viewBounds.Width() * (columns + 1);
				vint h = rows * itemHeight;
				full = Size(w, h);
				minimum = Size(w, 0);
			}

			Size GuiRepeatFixedHeightMultiColumnItemComposition::Layout_GetAdoptedSize(Size expectedSize)
			{
				if (!itemSource) return expectedSize;
				vint count = itemSource->GetCount();
				vint rowCount = viewBounds.Height() / itemHeight;
				if (rowCount > count) rowCount = count;
				if (rowCount == 0) rowCount = 1;
				return Size(expectedSize.x, CalculateAdoptedSize(expectedSize.y, rowCount, itemHeight));
			}

			vint GuiRepeatFixedHeightMultiColumnItemComposition::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
			{
				vint count = itemSource->GetCount();
				if (itemIndex < 0 || itemIndex >= count) return -1;
				vint rowCount = viewBounds.Height() / itemHeight;
				if (rowCount == 0) rowCount = 1;

				switch (key)
				{
				case KeyDirection::Up:
					itemIndex--;
					break;
				case KeyDirection::Down:
					itemIndex++;
					break;
				case KeyDirection::Left:
					itemIndex -= rowCount;
					break;
				case KeyDirection::Right:
					itemIndex += rowCount;
					break;
				case KeyDirection::Home:
					itemIndex = 0;
					break;
				case KeyDirection::End:
					itemIndex = count;
					break;
				case KeyDirection::PageUp:
					itemIndex -= itemIndex % rowCount;
					break;
				case KeyDirection::PageDown:
					itemIndex += rowCount - itemIndex % rowCount - 1;
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
					vint column = itemIndex / rowCount;
					Point location = viewBounds.LeftTop();

					if (column < firstColumn)
					{
						location.x = viewBounds.Width() * column;
					}
					else if (column >= firstColumn + fullVisibleColumns)
					{
						location.x = viewBounds.Width() * (column - fullVisibleColumns + 1);
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
		}
	}
}