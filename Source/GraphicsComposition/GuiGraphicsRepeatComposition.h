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
				Ptr<IValueEnumerable>								GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="value">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(Ptr<IValueEnumerable> value);
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