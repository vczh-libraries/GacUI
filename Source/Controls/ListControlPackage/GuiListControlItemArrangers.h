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

				template<typename TVirtualRepeatComposition>
				class VirtualRepeatRangedItemArrangerBase : public RangedItemArrangerBase
				{
				public:
					vint FindItem(vint itemIndex, compositions::KeyDirection key) override
					{
						CHECK_FAIL(L"Not Implemented!");
					}

					GuiListControl::EnsureItemVisibleResult EnsureItemVisible(vint itemIndex) override
					{
						CHECK_FAIL(L"Not Implemented!");
					}

					Size GetAdoptedSize(Size expectedSize) override
					{
						CHECK_FAIL(L"Not Implemented!");
					}
				};

				/// <summary>Free height item arranger. This arranger will cache heights of all items.</summary>
				class FreeHeightItemArranger : public VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFreeHeightItemComposition>, public Description<FreeHeightItemArranger>
				{
				public:
					/// <summary>Create the arranger.</summary>
					FreeHeightItemArranger();
					~FreeHeightItemArranger();
				};
				
				/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
				class FixedHeightItemArranger : public VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFixedHeightItemComposition>, public Description<FixedHeightItemArranger>
				{
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightItemArranger();
					~FixedHeightItemArranger();
				};

				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple lines with the same size. The width is the maximum width of all minimum widths of displayed items. The same to height.</summary>
				class FixedSizeMultiColumnItemArranger : public VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFixedSizeMultiColumnItemComposition>, public Description<FixedSizeMultiColumnItemArranger>
				{
				public:
					/// <summary>Create the arranger.</summary>
					FixedSizeMultiColumnItemArranger();
					~FixedSizeMultiColumnItemArranger();
				};
				
				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple columns with the same height. The height is the maximum width of all minimum height of displayed items. Each item will displayed using its minimum width.</summary>
				class FixedHeightMultiColumnItemArranger : public VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFixedHeightMultiColumnItemComposition>, public Description<FixedHeightMultiColumnItemArranger>
				{
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightMultiColumnItemArranger();
					~FixedHeightMultiColumnItemArranger();
				};
			}
		}
	}
}

#endif
