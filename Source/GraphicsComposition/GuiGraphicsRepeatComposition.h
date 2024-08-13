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
				virtual void										OnClearItems() = 0;
				virtual void										OnInstallItems() = 0;
				virtual void										OnUpdateContext() = 0;
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
				void												OnClearItems() override;
				void												OnInstallItems() override;
				void												OnUpdateContext() override;

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

			/// <summary>Result for <see cref="GuiVirtualRepeatCompositionBase::EnsureItemVisible"/>.</summary>
			enum class VirtualRepeatEnsureItemVisibleResult
			{
				/// <summary>The requested item does not exist.</summary>
				ItemNotExists,
				/// <summary>The view location is moved.</summary>
				Moved,
				/// <summary>The view location is not moved.</summary>
				NotMoved,
			};

			enum class VirtualRepeatPlaceItemResult
			{
				None,
				HitLastItem,
				Restart,
			};

			enum class VirtualRepeatEndPlaceItemResult
			{
				None,
				TotalSizeUpdated,
			};

			/// <summary>This composition implements most of the common functionality that display a continuing subset of items at a time.</summary>
			class GuiVirtualRepeatCompositionBase : public GuiBoundsComposition, public GuiRepeatCompositionBase, public Description<GuiVirtualRepeatCompositionBase>
			{
			protected:
				using ItemStyleRecord = templates::GuiTemplate*;
				using StyleList = collections::List<ItemStyleRecord>;
				using StyleEventHandlerMap = collections::Dictionary<GuiGraphicsComposition*, Ptr<IGuiGraphicsEventHandler>>;

				Ptr<IGuiAxis>										axis = Ptr(new GuiDefaultAxis);
				bool												itemSourceUpdated = false;
				bool												useMinimumFullSize = false;
				Size												realFullSize;
				Size												realMinimumFullSize;
				Rect												viewBounds;
				vint												startIndex = 0;
				StyleList											visibleStyles;
				StyleEventHandlerMap								eventHandlers;

				virtual void										Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex) = 0;
				virtual VirtualRepeatPlaceItemResult				Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) = 0;
				virtual VirtualRepeatEndPlaceItemResult				Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex) = 0;
				virtual void										Layout_EndLayout(bool totalSizeUpdated) = 0;
				virtual void										Layout_InvalidateItemSizeCache() = 0;
				virtual void										Layout_CalculateTotalSize(Size& full, Size& minimum) = 0;
				virtual Size										Layout_GetAdoptedSize(Size expected) = 0;

				virtual void										Layout_UpdateIndex(ItemStyleRecord style, vint index);
				void												Layout_UpdateViewBounds(Rect value, bool forceUpdateTotalSize);
				void												Layout_UpdateViewLocation(Point value);
				Rect												Layout_CalculateBounds(Size parentSize) override;
				void												Layout_ResetLayout();

				void												Layout_SetStyleAlignmentToParent(ItemStyleRecord style, Margin value);
				Size												Layout_GetStylePreferredSize(ItemStyleRecord style);
				Rect												Layout_GetStyleBounds(ItemStyleRecord style);
				void												Layout_SetStyleBounds(ItemStyleRecord style, Rect value);

				void												OnStyleCachedMinSizeChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments);
				void												AttachEventHandler(GuiGraphicsComposition* itemStyle);
				void												DetachEventHandler(GuiGraphicsComposition* itemStyle);
				void												OnChildRemoved(GuiGraphicsComposition* child)override;

				void												OnItemChanged(vint start, vint oldCount, vint newCount) override;
				void												OnClearItems() override;
				void												OnInstallItems() override;
				void												OnUpdateContext() override;
				virtual void										OnResetViewLocation();
				virtual ItemStyleRecord								CreateStyleInternal(vint index);
				virtual void										DeleteStyleInternal(ItemStyleRecord style);

				vint												CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);
				ItemStyleRecord										CreateStyle(vint index);
				void												DeleteStyle(ItemStyleRecord style);
				void												UpdateFullSize();
				void												OnViewChangedInternal(Rect oldBounds, Rect newBounds, bool forceUpdateTotalSize);

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

				bool												GetUseMinimumTotalSize();
				void												SetUseMinimumTotalSize(bool value);
				Size												GetTotalSize();
				Point												GetViewLocation();
				void												SetViewLocation(Point value);

				ItemStyleRecord										GetVisibleStyle(vint itemIndex);
				vint												GetVisibleIndex(ItemStyleRecord style);
				void												ResetLayout(bool recreateVisibleStyles);
				void												InvalidateLayout();

				Size												GetAdoptedSize(Size expectedSize);
				vint												FindItemByRealKeyDirection(vint itemIndex, compositions::KeyDirection key);
				virtual vint										FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key) = 0;
				virtual VirtualRepeatEnsureItemVisibleResult		EnsureItemVisible(vint itemIndex) = 0;
			};

			/// <summary>Free height repeat composition. This arranger will cache heights of all items.</summary>
			class GuiRepeatFreeHeightItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFreeHeightItemComposition>
			{
			private:
				bool												pi_heightUpdated = false;

			protected:
				collections::Array<vint>							heights;
				collections::Array<vint>							offsets;
				vint												availableOffsetCount = 0;

				void												EnsureOffsetForItem(vint itemIndex);

				void												Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex) override;
				VirtualRepeatPlaceItemResult						Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) override;
				VirtualRepeatEndPlaceItemResult						Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex) override;
				void												Layout_EndLayout(bool totalSizeUpdated) override;
				void												Layout_InvalidateItemSizeCache() override;
				void												Layout_CalculateTotalSize(Size& full, Size& minimum) override;
				Size												Layout_GetAdoptedSize(Size expectedSize) override;

				void												OnItemChanged(vint start, vint oldCount, vint newCount) override;
				void												OnInstallItems() override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFreeHeightItemComposition() = default;
				~GuiRepeatFreeHeightItemComposition() = default;

				vint												FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key) override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex) override;
			};
				
			/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
			class GuiRepeatFixedHeightItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFixedHeightItemComposition>
			{
			private:
				vint												pi_width = 0;
				vint												pi_yoffset = 0;
				vint												pi_rowHeight = 0;

			protected:
				vint												rowHeight = 1;
				vint												itemWidth = -1;
				vint												itemYOffset = 0;

				void												Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)override;
				VirtualRepeatPlaceItemResult						Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
				VirtualRepeatEndPlaceItemResult						Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)override;
				void												Layout_EndLayout(bool totalSizeUpdated) override;
				void												Layout_InvalidateItemSizeCache()override;
				void												Layout_CalculateTotalSize(Size& full, Size& minimum)override;
				Size												Layout_GetAdoptedSize(Size expectedSize) override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFixedHeightItemComposition() = default;
				~GuiRepeatFixedHeightItemComposition() = default;

				vint												FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex)override;

				vint												GetItemWidth();
				void												SetItemWidth(vint value);
				vint												GetItemYOffset();
				void												SetItemYOffset(vint value);
			};

			/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple lines with the same size. The width is the maximum width of all minimum widths of displayed items. The same to height.</summary>
			class GuiRepeatFixedSizeMultiColumnItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFixedSizeMultiColumnItemComposition>
			{
			private:
				Size												pi_itemSize;

			protected:
				Size												itemSize{ 1,1 };

				void												Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)override;
				VirtualRepeatPlaceItemResult						Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
				VirtualRepeatEndPlaceItemResult						Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)override;
				void												Layout_EndLayout(bool totalSizeUpdated) override;
				void												Layout_InvalidateItemSizeCache()override;
				void												Layout_CalculateTotalSize(Size& full, Size& minimum)override;
				Size												Layout_GetAdoptedSize(Size expectedSize) override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFixedSizeMultiColumnItemComposition() = default;
				~GuiRepeatFixedSizeMultiColumnItemComposition() = default;

				vint												FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex)override;
			};
			
			/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple columns with the same height. The height is the maximum width of all minimum height of displayed items. Each item will displayed using its minimum width.</summary>
			class GuiRepeatFixedHeightMultiColumnItemComposition : public GuiVirtualRepeatCompositionBase, public Description<GuiRepeatFixedHeightMultiColumnItemComposition>
			{
			private:
				collections::List<vint>								pi_visibleItemWidths;
				collections::List<vint>								pi_visibleColumnWidths;
				collections::List<vint>								pi_visibleColumnOffsets;
				vint												pi_rows = 0;
				vint												pi_firstColumn = 0;
				vint												pi_itemHeight = 0;

			protected:
				vint												firstColumn = 0;
				vint												fullVisibleColumns = 0;
				vint												itemHeight = 1;

				void												FixColumnWidth(vint index);

				void												Layout_BeginPlaceItem(bool firstPhase, Rect newBounds, vint& newStartIndex)override;
				VirtualRepeatPlaceItemResult						Layout_PlaceItem(bool firstPhase, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
				VirtualRepeatEndPlaceItemResult						Layout_EndPlaceItem(bool firstPhase, Rect newBounds, vint newStartIndex)override;
				void												Layout_EndLayout(bool totalSizeUpdated) override;
				void												Layout_InvalidateItemSizeCache()override;
				void												Layout_CalculateTotalSize(Size& full, Size& minimum)override;
				Size												Layout_GetAdoptedSize(Size expectedSize) override;
			public:
				/// <summary>Create the arranger.</summary>
				GuiRepeatFixedHeightMultiColumnItemComposition() = default;
				~GuiRepeatFixedHeightMultiColumnItemComposition() = default;

				vint												FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)override;
				VirtualRepeatEnsureItemVisibleResult				EnsureItemVisible(vint itemIndex)override;
			};
		}
	}
}

#endif