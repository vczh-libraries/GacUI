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

			/// <summary>This composition implements most of the common functionality that display a continuing subset of items at a time.</summary>
			class GuiVirtualRepeatCompositionBase : public GuiBoundsComposition, public GuiRepeatCompositionBase, public Description<GuiVirtualRepeatCompositionBase>
			{
			protected:
				using ItemStyleRecord = templates::GuiTemplate*;
				using StyleList = collections::List<ItemStyleRecord>;

			private:
				Ptr<IGuiAxis>										axis = Ptr(new GuiDefaultAxis);
				Size												realFullSize;
				Rect												viewBounds;
				vint												startIndex = 0;
				StyleList											visibleStyles;

				virtual void										Callback_UpdateViewLocation(Point location) = 0;
				virtual void										Callback_UpdateIndex(ItemStyleRecord style, vint index) = 0;

				virtual void										Layout_BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex) = 0;
				virtual void										Layout_PlaceItem(bool forMoving, bool newCreatedStyle, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent) = 0;
				virtual bool										Layout_IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds) = 0;
				virtual bool										Layout_EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex) = 0;
				virtual void										Layout_InvalidateItemSizeCache() = 0;
				virtual Size										Layout_CalculateTotalSize() = 0;

			protected:

				void												Layout_UpdateViewBounds(Rect value);
				Rect												Layout_CalculateBounds(Size parentSize) override;

				void												OnItemChanged(vint start, vint oldCount, vint newCount) override;
				void												ClearItems() override;
				void												InstallItems() override;
				void												UpdateContext() override;

				vint												CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);
				ItemStyleRecord										CreateStyle(vint index);
				void												DeleteStyle(ItemStyleRecord style);
				void												OnViewChangedInternal(Rect oldBounds, Rect newBounds);
				virtual void										RearrangeItemBounds();

			public:
				/// <summary>Create the arranger.</summary>
				GuiVirtualRepeatCompositionBase();
				~GuiVirtualRepeatCompositionBase();

				/// <summary>Total size changed event. This event raises when the total size of the content is changed.</summary>
				GuiNotifyEvent										TotalSizeChanged;

				/// <summary>View location changed event. This event raises when the view location of the content is changed.</summary>
				GuiNotifyEvent										ViewLocationChanged;

				/// <summary>This event raises when the adopted size of the content is potentially changed.</summary>
				GuiNotifyEvent										AdoptedSizeInvalidated;

				Size												GetTotalSize();
				Point												GetViewLocation();
				void												SetViewLocation(Point value);

				ItemStyleRecord										GetVisibleStyle(vint itemIndex);
				vint												GetVisibleIndex(ItemStyleRecord style);
				void												ReloadVisibleStyles();
			};
		}
	}
}

#endif