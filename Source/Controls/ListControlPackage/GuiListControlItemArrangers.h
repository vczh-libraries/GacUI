/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLITEMARRANGERS
#define VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLITEMARRANGERS

#include "GuiListControls.h"
#include "../../GraphicsComposition/GuiGraphicsRepeatComposition.h"

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

					GuiListControl*									listControl = nullptr;
					GuiListControl::IItemArrangerCallback*			callback = nullptr;
					list::IItemProvider*							itemProvider = nullptr;
					Ptr<description::IValueObservableList>			itemSource;
					compositions::GuiVirtualRepeatCompositionBase*	repeat = nullptr;

					void											OnViewLocationChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);
					void											OnTotalSizeChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);
					void											OnAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);
				public:
					/// <summary>Create the arranger.</summary>
					/// <param name="_repeat">A repeat composition to implement the item layout. It will be deleted when the item arranger is deleted.</param>
					RangedItemArrangerBase(compositions::GuiVirtualRepeatCompositionBase* _repeat);
					~RangedItemArrangerBase();

					void											OnAttached(list::IItemProvider* provider)override;
					void											OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
					void											AttachListControl(GuiListControl* value)override;
					void											DetachListControl()override;
					GuiListControl::IItemArrangerCallback*			GetCallback()override;
					void											SetCallback(GuiListControl::IItemArrangerCallback* value)override;
					Size											GetTotalSize()override;
					GuiListControl::ItemStyle*						GetVisibleStyle(vint itemIndex)override;
					vint											GetVisibleIndex(GuiListControl::ItemStyle* style)override;
					void											ReloadVisibleStyles()override;
					void											OnViewChanged(Rect bounds)override;
					vint											FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key) override;
					GuiListControl::EnsureItemVisibleResult			EnsureItemVisible(vint itemIndex) override;
					Size											GetAdoptedSize(Size expectedSize) override;
				};

				template<typename TVirtualRepeatComposition>
				class VirtualRepeatRangedItemArrangerBase : public RangedItemArrangerBase
				{
					using TArranger = VirtualRepeatRangedItemArrangerBase<TVirtualRepeatComposition>;
				protected:
					class ArrangerRepeatComposition : public TVirtualRepeatComposition
					{
					protected:
						TArranger*								arranger = nullptr;

						void Layout_UpdateIndex(templates::GuiTemplate* style, vint index) override
						{
							auto itemStyle = arranger->callback->GetItem(style);
							itemStyle->SetIndex(index);
						}

						templates::GuiTemplate* CreateStyleInternal(vint index) override
						{
							auto itemStyle = arranger->callback->CreateItem(index);
							return arranger->callback->GetItemBounds(itemStyle);
						}

						void DeleteStyleInternal(templates::GuiTemplate* style) override
						{
							auto itemStyle = arranger->callback->GetItem(style);
							arranger->callback->ReleaseItem(itemStyle);
						}
					public:
						template<typename ...TArgs>
						ArrangerRepeatComposition(TArranger* _arranger, TArgs&& ...args)
							: TVirtualRepeatComposition(std::forward<TArgs&&>(args)...)
							, arranger(_arranger)
						{
							this->SetUseMinimumTotalSize(true);
						}
					};

					TVirtualRepeatComposition* GetRepeatComposition()
					{
						return dynamic_cast<TVirtualRepeatComposition*>(repeat);
					}

				protected:
					VirtualRepeatRangedItemArrangerBase()
						: RangedItemArrangerBase(new ArrangerRepeatComposition(this))
					{
					}

					VirtualRepeatRangedItemArrangerBase(ArrangerRepeatComposition* _repeat)
						: RangedItemArrangerBase(_repeat)
					{
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
