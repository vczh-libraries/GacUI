/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONIMPL
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONIMPL

#include "../ListControlPackage/GuiListControlItemArrangers.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiBindableRibbonGalleryList;

/***********************************************************************
GalleryItemArranger
***********************************************************************/

			namespace ribbon_impl
			{
				class GalleryItemArranger : public list::RangedItemArrangerBase, public Description<GalleryItemArranger>
				{
				private:
					vint										pim_itemWidth = 0;
					bool										blockScrollUpdate = true;

				protected:
					GuiBindableRibbonGalleryList*				owner;
					vint										itemWidth = 1;
					vint										firstIndex = 0;

					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					GalleryItemArranger(GuiBindableRibbonGalleryList* _owner);
					~GalleryItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;

					void										ScrollUp();
					void										ScrollDown();
					void										UnblockScrollUpdate();
				};
			}
		}
	}
}

#endif
