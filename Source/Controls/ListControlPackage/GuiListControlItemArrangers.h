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
					typedef collections::List<GuiListControl::IItemStyleController*>		StyleList;
				protected:
					GuiListControl*								listControl = nullptr;
					GuiListControl::IItemArrangerCallback*		callback = nullptr;
					GuiListControl::IItemProvider*				itemProvider = nullptr;
					bool										suppressOnViewChanged = false;
					Rect										viewBounds;
					vint										startIndex = 0;
					StyleList									visibleStyles;

					void										InvalidateAdoptedSize();
					vint										CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);
					void										ClearStyles();
					virtual void								RearrangeItemBounds();

					virtual void								BeginPlaceItem() = 0;
					virtual void								PlaceItem(vint index, GuiListControl::IItemStyleController* style, Rect& bounds, Margin& alignmentToParent) = 0;
					virtual void								EndPlaceItem() = 0;

					virtual void								OnStylesCleared() = 0;
					virtual Size								OnCalculateTotalSize() = 0;
					virtual void								OnViewChangedInternal(Rect oldBounds, Rect newBounds) = 0;
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
					GuiListControl::IItemStyleController*		GetVisibleStyle(vint itemIndex)override;
					vint										GetVisibleIndex(GuiListControl::IItemStyleController* style)override;
					void										OnViewChanged(Rect bounds)override;
				};
				
				/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
				class FixedHeightItemArranger : public RangedItemArrangerBase, public Description<FixedHeightItemArranger>
				{
				private:
					vint										pi_x0 = 0;
					vint										pi_y0 = 0;
					vint										pi_width = 0;

				protected:
					vint										rowHeight;

					virtual vint								GetWidth();
					virtual vint								GetYOffset();

					void										BeginPlaceItem()override;
					void										PlaceItem(vint index, GuiListControl::IItemStyleController* style, Rect& bounds, Margin& alignmentToParent)override;
					void										EndPlaceItem()override;

					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightItemArranger();
					~FixedHeightItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};

				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple lines with the same size. The width is the maximum width of all minimum widths of displayed items. The same to height.</summary>
				class FixedSizeMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedSizeMultiColumnItemArranger>
				{
				private:
					vint										pi_y0 = 0;
					vint										pi_rowItems = 0;

				protected:
					Size										itemSize;

					void										CalculateRange(Size itemSize, Rect bounds, vint count, vint& start, vint& end);

					void										BeginPlaceItem()override;
					void										PlaceItem(vint index, GuiListControl::IItemStyleController* style, Rect& bounds, Margin& alignmentToParent)override;
					void										EndPlaceItem()override;

					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedSizeMultiColumnItemArranger();
					~FixedSizeMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
				
				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple columns with the same height. The height is the maximum width of all minimum height of displayed items. Each item will displayed using its minimum width.</summary>
				class FixedHeightMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedHeightMultiColumnItemArranger>
				{
				private:
					vint										pi_rows = 0;
					vint										pi_currentWidth = 0;
					vint										pi_totalWidth = 0;

				protected:
					vint										itemHeight;

					void										CalculateRange(vint itemHeight, Rect bounds, vint& rows, vint& startColumn);

					void										BeginPlaceItem()override;
					void										PlaceItem(vint index, GuiListControl::IItemStyleController* style, Rect& bounds, Margin& alignmentToParent)override;
					void										EndPlaceItem()override;

					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightMultiColumnItemArranger();
					~FixedHeightMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
			}
		}
	}
}

#endif
