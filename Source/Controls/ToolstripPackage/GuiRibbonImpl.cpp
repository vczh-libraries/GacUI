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
GalleryItemArrangerRepeatComposition
***********************************************************************/

			namespace ribbon_impl
			{
				void GalleryItemArrangerRepeatComposition::Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)
				{
					if (firstPhase)
					{
						pim_itemWidth = itemWidth;
						newStartIndex = firstIndex;
					}
				}

				compositions::VirtualRepeatPlaceItemResult GalleryItemArrangerRepeatComposition::Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
				{
					alignmentToParent = Margin(-1, 0, -1, 0);
					bounds = Rect(Point((index - firstIndex) * itemWidth, 0), Size(itemWidth, 0));

					if (firstPhase)
					{
						vint styleWidth = Layout_GetStylePreferredSize(style).x;
						if (pim_itemWidth < styleWidth)
						{
							pim_itemWidth = styleWidth;
						}
					}

					if (bounds.Right() + pim_itemWidth > viewBounds.Right())
					{
						return VirtualRepeatPlaceItemResult::HitLastItem;
					}
					else
					{
						return VirtualRepeatPlaceItemResult::None;
					}
				}

				compositions::VirtualRepeatEndPlaceItemResult GalleryItemArrangerRepeatComposition::Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)
				{
					bool result = false;
					if (firstPhase)
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

					return result ?
						VirtualRepeatEndPlaceItemResult::TotalSizeUpdated :
						VirtualRepeatEndPlaceItemResult::None;
				}

				void GalleryItemArrangerRepeatComposition::Layout_EndLayout(bool totalSizeUpdated)
				{
				}

				void GalleryItemArrangerRepeatComposition::Layout_InvalidateItemSizeCache()
				{
					itemWidth = 1;
				}

				void GalleryItemArrangerRepeatComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
				{
					full = Size(1, 1);
					minimum = Size(1, 1);
				}

				Size GalleryItemArrangerRepeatComposition::Layout_GetAdoptedSize(Size expectedSize)
				{
					return Size(1, 1);
				}

				GalleryItemArrangerRepeatComposition::GalleryItemArrangerRepeatComposition(GuiBindableRibbonGalleryList* _owner)
					:owner(_owner)
				{
				}

				GalleryItemArrangerRepeatComposition::~GalleryItemArrangerRepeatComposition()
				{
				}

				vint GalleryItemArrangerRepeatComposition::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
				{
					vint count = itemSource->GetCount();
					if (itemIndex < 0 || itemIndex >= count) return -1;
					vint groupCount = viewBounds.Width() / itemWidth;
					if (groupCount == 0) groupCount = 1;

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

				compositions::VirtualRepeatEnsureItemVisibleResult GalleryItemArrangerRepeatComposition::EnsureItemVisible(vint itemIndex)
				{
					if (!itemSource) return VirtualRepeatEnsureItemVisibleResult::NotMoved;
					if (itemIndex < 0 || itemIndex >= itemSource->GetCount())
					{
						return VirtualRepeatEnsureItemVisibleResult::ItemNotExists;
					}

					vint groupCount = viewBounds.Width() / itemWidth;
					if (groupCount == 0) groupCount = 1;

					if (itemIndex < firstIndex)
					{
						firstIndex = itemIndex;
						InvalidateLayout();
					}
					else if (itemIndex >= firstIndex + groupCount)
					{
						firstIndex = itemIndex - groupCount + 1;
						InvalidateLayout();
					}
					return VirtualRepeatEnsureItemVisibleResult::NotMoved;
				}

				void GalleryItemArrangerRepeatComposition::ScrollUp()
				{
					vint count = itemSource->GetCount();
					vint groupCount = viewBounds.Width() / itemWidth;
					if (groupCount == 0) groupCount = 1;

					if (count > groupCount)
					{
						firstIndex -= groupCount;
						if (firstIndex < 0)
						{
							firstIndex = 0;
						}
						InvalidateLayout();
					}
				}

				void GalleryItemArrangerRepeatComposition::ScrollDown()
				{
					vint count = itemSource->GetCount();
					vint groupCount = viewBounds.Width() / itemWidth;
					if (groupCount == 0) groupCount = 1;

					if (count > groupCount)
					{
						firstIndex += groupCount;
						if (firstIndex > count - groupCount)
						{
							firstIndex = count - groupCount;
						}
						InvalidateLayout();
					}
				}

				void GalleryItemArrangerRepeatComposition::UnblockScrollUpdate()
				{
					blockScrollUpdate = false;

					vint count = itemSource->GetCount();
					vint groupCount = viewBounds.Width() / pim_itemWidth;
					if (groupCount == 0) groupCount = 1;

					owner->SetScrollUpEnabled(firstIndex > 0);
					owner->SetScrollDownEnabled(firstIndex + groupCount < count);
					if (owner->layout->GetItemWidth() != pim_itemWidth)
					{
						owner->layout->SetItemWidth(pim_itemWidth);
						owner->UpdateLayoutSizeOffset();
					}
				}

/***********************************************************************
GalleryItemArranger
***********************************************************************/

				GalleryItemArranger::GalleryItemArranger(GuiBindableRibbonGalleryList* _owner)
					: TBase(new TBase::ArrangerRepeatComposition(this, _owner))
				{
				}

				GalleryItemArranger::~GalleryItemArranger()
				{
				}
				
				void GalleryItemArranger::ScrollUp()
				{
					GetRepeatComposition()->ScrollUp();
				}

				void GalleryItemArranger::ScrollDown()
				{
					GetRepeatComposition()->ScrollDown();
				}

				void GalleryItemArranger::UnblockScrollUpdate()
				{
					GetRepeatComposition()->UnblockScrollUpdate();
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