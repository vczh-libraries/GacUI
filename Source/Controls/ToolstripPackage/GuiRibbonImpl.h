/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONIMPL
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONIMPL

#include "../ListControlPackage/GuiListControlItemArrangers.h"
#include "../../GraphicsComposition/GuiGraphicsResponsiveComposition.h"

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
				class GalleryItemArrangerRepeatComposition : public compositions::GuiVirtualRepeatCompositionBase, public Description<GalleryItemArrangerRepeatComposition>
				{
				private:
					vint													pim_itemWidth = 0;
					bool													blockScrollUpdate = true;

				protected:
					GuiBindableRibbonGalleryList*							owner;
					vint													itemWidth = 1;
					vint													firstIndex = 0;

					void													Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)override;
					compositions::VirtualRepeatPlaceItemResult				Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					compositions::VirtualRepeatEndPlaceItemResult			Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)override;
					void													Layout_EndLayout(bool totalSizeUpdated) override;
					void													Layout_InvalidateItemSizeCache()override;
					void													Layout_CalculateTotalSize(Size& full, Size& minimum)override;
					Size													Layout_GetAdoptedSize(Size expectedSize)override;
				public:
					GalleryItemArrangerRepeatComposition(GuiBindableRibbonGalleryList* _owner);
					~GalleryItemArrangerRepeatComposition();

					vint													FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)override;
					compositions::VirtualRepeatEnsureItemVisibleResult		EnsureItemVisible(vint itemIndex)override;

					void													ScrollUp();
					void													ScrollDown();
					void													UnblockScrollUpdate();
				};

				class GalleryItemArranger : public list::VirtualRepeatRangedItemArrangerBase<GalleryItemArrangerRepeatComposition>, public Description<GalleryItemArranger>
				{
					using TBase = list::VirtualRepeatRangedItemArrangerBase<GalleryItemArrangerRepeatComposition>;
				public:
					GalleryItemArranger(GuiBindableRibbonGalleryList* _owner);
					~GalleryItemArranger();

					void										ScrollUp();
					void										ScrollDown();
					void										UnblockScrollUpdate();
				};

				class GalleryResponsiveLayout : public compositions::GuiResponsiveCompositionBase, public Description<GalleryResponsiveLayout>
				{
				protected:
					vint										minCount = 0;
					vint										maxCount = 0;
					Size										sizeOffset;
					vint										itemCount = 0;
					vint										itemWidth = 1;

					void										UpdateMinSize();
				public:
					GalleryResponsiveLayout();
					~GalleryResponsiveLayout();

					vint										GetMinCount();
					vint										GetMaxCount();
					vint										GetItemWidth();
					Size										GetSizeOffset();
					vint										GetVisibleItemCount();

					void										SetMinCount(vint value);
					void										SetMaxCount(vint value);
					void										SetItemWidth(vint value);
					void										SetSizeOffset(Size value);

					vint										GetLevelCount()override;
					vint										GetCurrentLevel()override;
					bool										LevelDown()override;
					bool										LevelUp()override;
				};
			}
		}
	}
}

#endif
