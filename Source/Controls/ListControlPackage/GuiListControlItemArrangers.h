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
					GuiListControl*								listControl;
					GuiListControl::IItemArrangerCallback*		callback;
					GuiListControl::IItemProvider*				itemProvider;
					bool										suppressOnViewChanged = false;
					Rect										viewBounds;
					vint										startIndex;
					StyleList									visibleStyles;

					void										InvalidateAdoptedSize();
					vint										CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);

					virtual void								ClearStyles();
					virtual void								OnStylesCleared()=0;
					virtual Size								OnCalculateTotalSize()=0;
					virtual void								OnViewChangedInternal(Rect oldBounds, Rect newBounds)=0;
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
				protected:
					vint										rowHeight;

					virtual void								RearrangeItemBounds();
					virtual vint								GetWidth();
					virtual vint								GetYOffset();
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
				protected:
					Size										itemSize;

					virtual void								RearrangeItemBounds();
					void										CalculateRange(Size itemSize, Rect bounds, vint count, vint& start, vint& end);
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
				protected:
					vint										itemHeight;

					virtual void								RearrangeItemBounds();
					void										CalculateRange(vint itemHeight, Rect bounds, vint& rows, vint& startColumn);
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
