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
ribbon_impl::GalleryItemArranger
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

				void GalleryItemArranger::PlaceItem(bool forMoving, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
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
					return bounds.Right() + pim_itemWidth >= viewBounds.Right();
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

				bool GalleryItemArranger::EnsureItemVisible(vint itemIndex)
				{
					return false;
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
						callback->OnTotalSizeChanged();
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
						callback->OnTotalSizeChanged();
					}
				}

				void GalleryItemArranger::UnblockScrollUpdate()
				{
					blockScrollUpdate = false;

					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Width() / pim_itemWidth;
					owner->SetScrollUpEnabled(firstIndex > 0);
					owner->SetScrollDownEnabled(firstIndex + groupCount < count);
				}
			}
		}
	}
}