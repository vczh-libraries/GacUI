/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSREPEATCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSREPEATCOMPOSITION

#include "GuiGraphicsStackComposition.h"
#include "GuiGraphicsFlowComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			/// <summary>A base class for all bindable repeat compositions.</summary>
			class GuiRepeatCompositionBase : public Object, public Description<GuiRepeatCompositionBase>
			{
				using ItemStyleProperty = TemplateProperty<templates::GuiTemplate>;
				using IValueEnumerable = reflection::description::IValueEnumerable;
				using IValueList = reflection::description::IValueList;
			protected:
				ItemStyleProperty									itemTemplate;
				Ptr<IValueList>										itemSource;
				description::Value									itemContext;
				Ptr<EventHandler>									itemChangedHandler;

				virtual void										OnItemChanged(vint index, vint oldCount, vint newCount) = 0;
				virtual void										ClearItems() = 0;
				virtual void										InstallItems() = 0;
				virtual void										UpdateContext() = 0;
			public:
				GuiRepeatCompositionBase();
				~GuiRepeatCompositionBase();

				/// <summary>Context changed event. This event raises when the context of the composition is changed.</summary>
				GuiNotifyEvent										ContextChanged;

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				ItemStyleProperty									GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				void												SetItemTemplate(ItemStyleProperty value);

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				Ptr<IValueEnumerable>								GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="value">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(Ptr<IValueEnumerable> value);

				/// <summary>Get the context of this composition. The all item templates (if it has) will see this context property.</summary>
				/// <returns>The context of this composition.</returns>
				description::Value									GetContext();
				/// <summary>Set the context of this composition.</summary>
				/// <param name="value">The context of this composition.</param>
				void												SetContext(const description::Value& value);
			};

/***********************************************************************
GuiNonVirtialRepeatCompositionBase
***********************************************************************/

			/// <summary>A base class for all bindable repeat compositions.</summary>
			class GuiNonVirtialRepeatCompositionBase : public GuiRepeatCompositionBase, public Description<GuiNonVirtialRepeatCompositionBase>
			{
			protected:

				virtual vint										GetRepeatCompositionCount() = 0;
				virtual GuiGraphicsComposition*						GetRepeatComposition(vint index) = 0;
				virtual GuiGraphicsComposition*						InsertRepeatComposition(vint index) = 0;
				virtual GuiGraphicsComposition*						RemoveRepeatComposition(vint index) = 0;

				void												OnItemChanged(vint index, vint oldCount, vint newCount) override;
				void												ClearItems() override;
				void												InstallItems() override;
				void												UpdateContext() override;

				void												RemoveItem(vint index);
				void												InstallItem(vint index);
			public:
				GuiNonVirtialRepeatCompositionBase();
				~GuiNonVirtialRepeatCompositionBase();

				/// <summary>An event called after a new item is inserted.</summary>
				GuiItemNotifyEvent									ItemInserted;
				/// <summary>An event called before a new item is removed.</summary>
				GuiItemNotifyEvent									ItemRemoved;
			};

			/// <summary>Bindable stack composition.</summary>
			class GuiRepeatStackComposition : public GuiStackComposition, public GuiNonVirtialRepeatCompositionBase, public Description<GuiRepeatStackComposition>
			{
			protected:
				vint												GetRepeatCompositionCount()override;
				GuiGraphicsComposition*								GetRepeatComposition(vint index)override;
				GuiGraphicsComposition*								InsertRepeatComposition(vint index)override;
				GuiGraphicsComposition*								RemoveRepeatComposition(vint index)override;

			public:
				GuiRepeatStackComposition();
				~GuiRepeatStackComposition();
			};

			/// <summary>Bindable flow composition.</summary>
			class GuiRepeatFlowComposition : public GuiFlowComposition, public GuiNonVirtialRepeatCompositionBase, public Description<GuiRepeatFlowComposition>
			{
			protected:
				vint												GetRepeatCompositionCount()override;
				GuiGraphicsComposition*								GetRepeatComposition(vint index)override;
				GuiGraphicsComposition*								InsertRepeatComposition(vint index)override;
				GuiGraphicsComposition*								RemoveRepeatComposition(vint index)override;

			public:
				GuiRepeatFlowComposition();
				~GuiRepeatFlowComposition();
			};

/***********************************************************************
GuiVirtualRepeatCompositionBase
***********************************************************************/

			/// <summary>EnsureItemVisible result for item arranger.</summary>
			enum class VirtualRepeatEnsureItemVisibleResult
			{
				/// <summary>The requested item does not exist.</summary>
				ItemNotExists,
				/// <summary>The view location is moved.</summary>
				Moved,
				/// <summary>The view location is not moved.</summary>
				NotMoved,
			};

			/// <summary>This composition implements most of the common functionality that display a continuing subset of items at a time.</summary>
			class GuiVirtualRepeatCompositionBase : public GuiBoundsComposition, public GuiRepeatCompositionBase, public Description<GuiVirtualRepeatCompositionBase>
			{
			protected:
				using ItemStyleRecord = templates::GuiTemplate*;
				using StyleList = collections::List<ItemStyleRecord>;

				Ptr<IGuiAxis>										axis = Ptr(new GuiDefaultAxis);
				bool												itemSourceUpdated = false;
				Size												realFullSize;
				Rect												viewBounds;
				vint												startIndex = 0;
				StyleList											visibleStyles;

				virtual void										Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex) = 0;
				virtual void										Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) = 0;
				virtual bool										Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds) = 0;
				virtual bool										Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex) = 0;
				virtual void										Layout_InvalidateItemSizeCache() = 0;
				virtual Size										Layout_CalculateTotalSize() = 0;

				virtual void										Layout_UpdateIndex(ItemStyleRecord style, vint index);
				void												Layout_UpdateViewBounds(Rect value);
				void												Layout_UpdateViewLocation(Point value);
				Rect												Layout_CalculateBounds(Size parentSize) override;

				void												Layout_SetStyleAlignmentToParent(ItemStyleRecord style, Margin value);
				Size												Layout_GetStylePreferredSize(ItemStyleRecord style);
				Rect												Layout_GetStyleBounds(ItemStyleRecord style);
				void												Layout_SetStyleBounds(ItemStyleRecord style, Rect value);

				void												OnItemChanged(vint start, vint oldCount, vint newCount) override;
				void												ClearItems() override;
				void												InstallItems() override;
				void												UpdateContext() override;

				vint												CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);
				ItemStyleRecord										CreateStyle(vint index);
				void												DeleteStyle(ItemStyleRecord style);
				void												OnViewChangedInternal(Rect oldBounds, Rect newBounds);

			public:
				/// <summary>Create the arranger.</summary>
				GuiVirtualRepeatCompositionBase();
				~GuiVirtualRepeatCompositionBase();

				/// <summary>Axis changed event.</summary>
				GuiNotifyEvent										AxisChanged;

				/// <summary>Total size changed event. This event raises when the total size of the content is changed.</summary>
				GuiNotifyEvent										TotalSizeChanged;

				/// <summary>View location changed event. This event raises when the view location of the content is changed.</summary>
				GuiNotifyEvent										ViewLocationChanged;

				/// <summary>This event raises when the adopted size of the content is potentially changed.</summary>
				GuiNotifyEvent										AdoptedSizeInvalidated;

				Ptr<IGuiAxis>										GetAxis();
				void												SetAxis(Ptr<IGuiAxis> value);

				Size												GetTotalSize();
				Point												GetViewLocation();
				void												SetViewLocation(Point value);

				ItemStyleRecord										GetVisibleStyle(vint itemIndex);
				vint												GetVisibleIndex(ItemStyleRecord style);
				void												ReloadVisibleStyles();

				virtual vint										FindItem(vint itemIndex, compositions::KeyDirection key) = 0;
				virtual VirtualRepeatEnsureItemVisibleResult		EnsureItemVisible(vint itemIndex) = 0;
				virtual Size										GetAdoptedSize(Size expectedSize) = 0;
			};

			/// <summary>Free height repeat composition. This arranger will cache heights of all items.</summary>
			class GuiRepeatFreeHeightItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFreeHeightItemComposition>
			{
			private:
				bool												pim_heightUpdated = false;

			protected:
				collections::Array<vint>							heights;
				collections::Array<vint>							offsets;
				vint												availableOffsetCount = 0;

				void												EnsureOffsetForItem(vint itemIndex);

				void												Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex) override;
				void												Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) override;
				bool												Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds) override;
				bool												Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex) override;
				void												Layout_InvalidateItemSizeCache() override;
				Size												Layout_CalculateTotalSize() override;

				void												OnItemChanged(vint start, vint oldCount, vint newCount) override;
				void												InstallItems() override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFreeHeightItemComposition() = default;
				~GuiRepeatFreeHeightItemComposition() = default;

				vint												FindItem(vint itemIndex, compositions::KeyDirection key) override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex) override;
				Size												GetAdoptedSize(Size expectedSize) override;
			};
				
			/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
			class GuiRepeatFixedHeightItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFixedHeightItemComposition>
			{
			private:
				vint												pi_width = 0;
				vint												pim_rowHeight = 0;

			protected:
				vint												rowHeight = 1;

				virtual vint										GetWidth();
				virtual vint										GetYOffset();

				void												Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
				void												Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
				bool												Layout_IsItemCouldBeTheLastVisibleInBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
				bool												Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
				void												Layout_InvalidateItemSizeCache()override;
				Size												Layout_CalculateTotalSize()override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFixedHeightItemComposition();
				~GuiRepeatFixedHeightItemComposition();

				vint												FindItem(vint itemIndex, compositions::KeyDirection key)override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex)override;
				Size												GetAdoptedSize(Size expectedSize)override;
			};
		}
	}
}

#endif