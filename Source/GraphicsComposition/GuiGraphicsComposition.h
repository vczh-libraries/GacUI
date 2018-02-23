/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSCOMPOSITION
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSCOMPOSITION

#include "GuiGraphicsCompositionBase.h"
#include "GuiGraphicsBasicComposition.h"
#include "GuiGraphicsTableComposition.h"
#include "GuiGraphicsStackComposition.h"
#include "GuiGraphicsFlowComposition.h"
#include "GuiGraphicsSpecializedComposition.h"

namespace vl
{
	namespace presentation
	{
		template<typename T>
		using ItemProperty = Func<T(const reflection::description::Value&)>;

		template<typename T>
		using WritableItemProperty = Func<T(const reflection::description::Value&, T, bool)>;

		template<typename T>
		using TemplateProperty = Func<T*(const reflection::description::Value&)>;

		namespace templates
		{
			class GuiTemplate;
		}

		namespace compositions
		{
			class GuiSharedSizeItemComposition;
			class GuiSharedSizeRootComposition;

			/// <summary>A shared size composition that shares the same size with all other <see cref="GuiSharedSizeItemComposition"/> that has a same group name.</summary>
			class GuiSharedSizeItemComposition : public GuiBoundsComposition, public Description<GuiSharedSizeItemComposition>
			{
			protected:
				GuiSharedSizeRootComposition*						parentRoot;
				WString												group;
				bool												sharedWidth;
				bool												sharedHeight;

				void												Update();
				void												OnParentLineChanged()override;
			public:
				GuiSharedSizeItemComposition();
				~GuiSharedSizeItemComposition();
				
				/// <summary>Get the group name of this item.</summary>
				/// <returns>The group name.</returns>
				const WString&										GetGroup();
				/// <summary>Set the group name of this item.</summary>
				/// <param name="value">The group name.</param>
				void												SetGroup(const WString& value);
				/// <summary>Test is the width of this item is shared.</summary>
				/// <returns>Returns true if the width of this item is shared.</returns>
				bool												GetSharedWidth();
				/// <summary>Enable or disable sharing the width of this item.</summary>
				/// <param name="value">Set to true to share the width of this item.</param>
				void												SetSharedWidth(bool value);
				/// <summary>Test is the height of this item is shared.</summary>
				/// <returns>Returns true if the height of this item is shared.</returns>
				bool												GetSharedHeight();
				/// <summary>Enable or disable sharing the height of this item.</summary>
				/// <param name="value">Set to true to share the height of this item.</param>
				void												SetSharedHeight(bool value);
			};

			/// <summary>A root composition that takes care of all direct or indirect <see cref="GuiSharedSizeItemComposition"/> to enable size sharing.</summary>
			class GuiSharedSizeRootComposition :public GuiBoundsComposition, public Description<GuiSharedSizeRootComposition>
			{
				friend class GuiSharedSizeItemComposition;
			protected:
				collections::Dictionary<WString, vint>				itemWidths;
				collections::Dictionary<WString, vint>				itemHeights;
				collections::List<GuiSharedSizeItemComposition*>	childItems;

				void												AddSizeComponent(collections::Dictionary<WString, vint>& sizes, const WString& group, vint sizeComponent);
				void												CollectSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights);
				void												AlignSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights);
				void												UpdateBounds();
			public:
				GuiSharedSizeRootComposition();
				~GuiSharedSizeRootComposition();

				void												ForceCalculateSizeImmediately()override;
				Rect												GetBounds()override;
			};

			/// <summary>A base class for all bindable repeat compositions.</summary>
			class GuiRepeatCompositionBase : public Object, public Description<GuiRepeatCompositionBase>
			{
				using ItemStyleProperty = TemplateProperty<templates::GuiTemplate>;
				using ItemSourceType = Ptr<reflection::description::IValueObservableList>;
			protected:
				ItemStyleProperty									itemTemplate;
				ItemSourceType										itemSource;
				Ptr<EventHandler>									itemChangedHandler;
				
				virtual vint										GetRepeatCompositionCount() = 0;
				virtual GuiGraphicsComposition*						GetRepeatComposition(vint index) = 0;
				virtual GuiGraphicsComposition*						InsertRepeatComposition(vint index) = 0;
				virtual GuiGraphicsComposition*						RemoveRepeatComposition(vint index) = 0;

				void												OnItemChanged(vint index, vint oldCount, vint newCount);
				void												RemoveItem(vint index);
				void												InstallItem(vint index);
				void												ClearItems();
				void												InstallItems();
			public:
				GuiRepeatCompositionBase();
				~GuiRepeatCompositionBase();

				/// <summary>An event called after a new item is inserted.</summary>
				GuiItemNotifyEvent									ItemInserted;
				/// <summary>An event called before a new item is removed.</summary>
				GuiItemNotifyEvent									ItemRemoved;

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				ItemStyleProperty									GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				void												SetItemTemplate(ItemStyleProperty value);

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				ItemSourceType										GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="_itemSource">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(ItemSourceType value);
			};

			/// <summary>Bindable stack composition.</summary>
			class GuiRepeatStackComposition : public GuiStackComposition, public GuiRepeatCompositionBase, public Description<GuiRepeatStackComposition>
			{
			protected:
				vint												GetRepeatCompositionCount()override;
				GuiGraphicsComposition*								GetRepeatComposition(vint index)override;
				GuiGraphicsComposition*								InsertRepeatComposition(vint index)override;
				GuiGraphicsComposition*								RemoveRepeatComposition(vint index)override;
			public:
			};

			/// <summary>Bindable flow composition.</summary>
			class GuiRepeatFlowComposition : public GuiFlowComposition, public GuiRepeatCompositionBase, public Description<GuiRepeatFlowComposition>
			{
			protected:
				vint												GetRepeatCompositionCount()override;
				GuiGraphicsComposition*								GetRepeatComposition(vint index)override;
				GuiGraphicsComposition*								InsertRepeatComposition(vint index)override;
				GuiGraphicsComposition*								RemoveRepeatComposition(vint index)override;
			public:
			};
		}
	}
}

#endif