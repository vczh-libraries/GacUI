#include "GuiRibbonGalleryList.h"
#include "GuiRibbonImpl.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace compositions;

/***********************************************************************
GalleryItemArranger
***********************************************************************/

			namespace ribbon_impl
			{
				void GalleryItemArranger::BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
				{
					if (forMoving)
					{
						pim_itemWidth = itemWidth;
						newStartIndex = firstIndex;
					}
				}

				void GalleryItemArranger::PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
				{
					alignmentToParent = Margin(-1, 0, -1, 0);
					bounds = Rect(Point((index - firstIndex) * itemWidth, 0), Size(itemWidth, 0));

					if (forMoving)
					{
						vint styleWidth = callback->GetStylePreferredSize(GetStyleBounds(style)).x;
						if (pim_itemWidth < styleWidth)
						{
							pim_itemWidth = styleWidth;
						}
					}
				}

				bool GalleryItemArranger::IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
				{
					return bounds.Right() + pim_itemWidth > viewBounds.Right();
				}

				bool GalleryItemArranger::EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
				{
					bool result = false;
					if (forMoving)
					{
						if (pim_itemWidth != itemWidth)
						{
							itemWidth = pim_itemWidth;
							result = true;
						}
					}

					if (!blockScrollUpdate)
					{
						UnblockScrollUpdate();
					}

					return result;
				}

				void GalleryItemArranger::InvalidateItemSizeCache()
				{
					itemWidth = 1;
				}

				Size GalleryItemArranger::OnCalculateTotalSize()
				{
					return Size(1, 1);
				}

				GalleryItemArranger::GalleryItemArranger(GuiBindableRibbonGalleryList* _owner)
					:owner(_owner)
				{
				}

				GalleryItemArranger::~GalleryItemArranger()
				{
				}

				vint GalleryItemArranger::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Width() / itemWidth;

					switch (key)
					{
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

				GuiListControl::EnsureItemVisibleResult GalleryItemArranger::EnsureItemVisible(vint itemIndex)
				{
					if (callback)
					{
						if (0 <= itemIndex && itemIndex < itemProvider->Count())
						{
							vint groupCount = viewBounds.Width() / itemWidth;
							if (itemIndex < firstIndex)
							{
								firstIndex = itemIndex;
								callback->OnTotalSizeChanged();
							}
							else if (itemIndex >= firstIndex + groupCount)
							{
								firstIndex = itemIndex - groupCount + 1;
								callback->OnTotalSizeChanged();
							}
							return GuiListControl::EnsureItemVisibleResult::NotMoved;
						}
						else
						{
							return GuiListControl::EnsureItemVisibleResult::ItemNotExists;
						}
					}
					return GuiListControl::EnsureItemVisibleResult::NotMoved;
				}

				Size GalleryItemArranger::GetAdoptedSize(Size expectedSize)
				{
					return Size(1, 1);
				}

				void GalleryItemArranger::ScrollUp()
				{
					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Width() / itemWidth;
					if (count > groupCount)
					{
						firstIndex -= groupCount;
						if (firstIndex < 0)
						{
							firstIndex = 0;
						}

						if (callback)
						{
							callback->OnTotalSizeChanged();
						}
					}
				}

				void GalleryItemArranger::ScrollDown()
				{
					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Width() / itemWidth;
					if (count > groupCount)
					{
						firstIndex += groupCount;
						if (firstIndex > count - groupCount)
						{
							firstIndex = count - groupCount;
						}

						if (callback)
						{
							callback->OnTotalSizeChanged();
						}
					}
				}

				void GalleryItemArranger::UnblockScrollUpdate()
				{
					blockScrollUpdate = false;

					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Width() / pim_itemWidth;
					owner->SetScrollUpEnabled(firstIndex > 0);
					owner->SetScrollDownEnabled(firstIndex + groupCount < count);
					if (owner->layout->GetItemWidth() != pim_itemWidth)
					{
						owner->layout->SetItemWidth(pim_itemWidth);
						owner->UpdateLayoutSizeOffset();
					}
				}

/***********************************************************************
GalleryResponsiveLayout
***********************************************************************/

				void GalleryResponsiveLayout::UpdateMinSize()
				{
					SetPreferredMinSize(Size(itemCount * itemWidth + sizeOffset.x, sizeOffset.y));
				}

				GalleryResponsiveLayout::GalleryResponsiveLayout()
				{
					SetDirection(ResponsiveDirection::Horizontal);
				}

				GalleryResponsiveLayout::~GalleryResponsiveLayout()
				{
				}

				vint GalleryResponsiveLayout::GetMinCount()
				{
					return minCount;
				}

				vint GalleryResponsiveLayout::GetMaxCount()
				{
					return maxCount;
				}

				vint GalleryResponsiveLayout::GetItemWidth()
				{
					return itemWidth;
				}

				Size GalleryResponsiveLayout::GetSizeOffset()
				{
					return sizeOffset;
				}

				vint GalleryResponsiveLayout::GetVisibleItemCount()
				{
					return itemCount;
				}

				void GalleryResponsiveLayout::SetMinCount(vint value)
				{
					vint oldCount = GetLevelCount();
					vint oldLevel = GetCurrentLevel();

					if (minCount != value)
					{
						if (value < 0) value = 0;
						minCount = value;
						if (maxCount < minCount) maxCount = minCount;
						if (itemCount < minCount) itemCount = minCount;
						UpdateMinSize();
					}

					bool countChanged = oldCount != GetLevelCount();
					bool levelChanged = oldLevel != GetCurrentLevel();
					if (countChanged) LevelCountChanged.Execute(GuiEventArgs(this));
					if (levelChanged) CurrentLevelChanged.Execute(GuiEventArgs(this));
					if (countChanged || levelChanged) OnResponsiveChildLevelUpdated();
				}

				void GalleryResponsiveLayout::SetMaxCount(vint value)
				{
					vint oldCount = GetLevelCount();
					vint oldLevel = GetCurrentLevel();

					if (maxCount != value)
					{
						if (value < 0) value = 0;
						maxCount = value;
						if (minCount > maxCount) minCount = maxCount;
						if (itemCount > maxCount) itemCount = maxCount;
						UpdateMinSize();
					}

					if (oldCount != GetLevelCount()) LevelCountChanged.Execute(GuiEventArgs(this));
					if (oldLevel != GetCurrentLevel()) CurrentLevelChanged.Execute(GuiEventArgs(this));
				}

				void GalleryResponsiveLayout::SetItemWidth(vint value)
				{
					if (itemWidth != value)
					{
						itemWidth = value;
						UpdateMinSize();
					}
				}

				void GalleryResponsiveLayout::SetSizeOffset(Size value)
				{
					if (sizeOffset != value)
					{
						sizeOffset = value;
						UpdateMinSize();
					}
				}

				vint GalleryResponsiveLayout::GetLevelCount()
				{
					return maxCount - minCount + 1;
				}

				vint GalleryResponsiveLayout::GetCurrentLevel()
				{
					return itemCount - minCount;
				}

				bool GalleryResponsiveLayout::LevelDown()
				{
					if (itemCount > minCount)
					{
						itemCount--;
						UpdateMinSize();
						CurrentLevelChanged.Execute(GuiEventArgs(this));
						return true;
					}
					return false;
				}

				bool GalleryResponsiveLayout::LevelUp()
				{
					if (itemCount < maxCount)
					{
						itemCount++;
						UpdateMinSize();
						CurrentLevelChanged.Execute(GuiEventArgs(this));
						return true;
					}
					return false;
				}
			}
		}
	}
}