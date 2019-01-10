/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLITEMARRANGERS
#define VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLITEMARRANGERS

#include "GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Predefined ItemArranger
***********************************************************************/

			namespace list
			{

				/// <summary>Ranged item arranger. This arranger implements most of the common functionality for those arrangers that display a continuing subset of item at a time.</summary>
				class RangedItemArrangerBase : public Object, virtual public GuiListControl::IItemArranger, public Description<RangedItemArrangerBase>
				{
				protected:
					using ItemStyleRecord = collections::Pair<GuiListControl::ItemStyle*, GuiSelectableButton*>;
					typedef collections::List<ItemStyleRecord>	StyleList;

					GuiListControl*								listControl = nullptr;
					GuiListControl::IItemArrangerCallback*		callback = nullptr;
					GuiListControl::IItemProvider*				itemProvider = nullptr;

					bool										suppressOnViewChanged = false;
					Rect										viewBounds;
					vint										startIndex = 0;
					StyleList									visibleStyles;

				protected:

					void										InvalidateAdoptedSize();
					vint										CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);
					ItemStyleRecord								CreateStyle(vint index);
					void										DeleteStyle(ItemStyleRecord style);
					compositions::GuiBoundsComposition*			GetStyleBounds(ItemStyleRecord style);
					void										ClearStyles();
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds);
					virtual void								RearrangeItemBounds();

					virtual void								BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex) = 0;
					virtual void								PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) = 0;
					virtual bool								IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds) = 0;
					virtual bool								EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex) = 0;
					virtual void								InvalidateItemSizeCache() = 0;
					virtual Size								OnCalculateTotalSize() = 0;
				public:
					/// <summary>Create the arranger.</summary>
					RangedItemArrangerBase();
					~RangedItemArrangerBase();

					void										OnAttached(GuiListControl::IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount)override;
					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;
					GuiListControl::IItemArrangerCallback*		GetCallback()override;
					void										SetCallback(GuiListControl::IItemArrangerCallback* value)override;
					Size										GetTotalSize()override;
					GuiListControl::ItemStyle*					GetVisibleStyle(vint itemIndex)override;
					vint										GetVisibleIndex(GuiListControl::ItemStyle* style)override;
					void										ReloadVisibleStyles()override;
					void										OnViewChanged(Rect bounds)override;
				};
				/// <summary>Free height item arranger. This arranger will cache heights of all items.</summary>
				class FreeHeightItemArranger : public RangedItemArrangerBase, public Description<FreeHeightItemArranger>
				{
				private:
					bool										pim_heightUpdated = false;

				protected:
					collections::Array<vint>					heights;
					collections::Array<vint>					offsets;
					vint										availableOffsetCount = 0;

					void										EnsureOffsetForItem(vint itemIndex);

					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					/// <summary>Create the arranger.</summary>
					FreeHeightItemArranger();
					~FreeHeightItemArranger();

					void										OnAttached(GuiListControl::IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount)override;
					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					GuiListControl::EnsureItemVisibleResult		EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
				
				/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
				class FixedHeightItemArranger : public RangedItemArrangerBase, public Description<FixedHeightItemArranger>
				{
				private:
					vint										pi_width = 0;
					vint										pim_rowHeight = 0;

				protected:
					vint										rowHeight = 1;

					virtual vint								GetWidth();
					virtual vint								GetYOffset();

					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightItemArranger();
					~FixedHeightItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					GuiListControl::EnsureItemVisibleResult		EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};

				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple lines with the same size. The width is the maximum width of all minimum widths of displayed items. The same to height.</summary>
				class FixedSizeMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedSizeMultiColumnItemArranger>
				{
				private:
					Size										pim_itemSize;

				protected:
					Size										itemSize{ 1,1 };

					void										CalculateRange(Size itemSize, Rect bounds, vint count, vint& start, vint& end);

					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedSizeMultiColumnItemArranger();
					~FixedSizeMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					GuiListControl::EnsureItemVisibleResult		EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
				
				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple columns with the same height. The height is the maximum width of all minimum height of displayed items. Each item will displayed using its minimum width.</summary>
				class FixedHeightMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedHeightMultiColumnItemArranger>
				{
				private:
					vint										pi_currentWidth = 0;
					vint										pi_totalWidth = 0;
					vint										pim_itemHeight = 0;

				protected:
					vint										itemHeight;

					void										CalculateRange(vint itemHeight, Rect bounds, vint& rows, vint& startColumn);

					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightMultiColumnItemArranger();
					~FixedHeightMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					GuiListControl::EnsureItemVisibleResult		EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
			}
		}
	}
}

#endif
