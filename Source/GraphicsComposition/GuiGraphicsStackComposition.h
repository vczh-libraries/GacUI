/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSTACKCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSTACKCOMPOSITION

#include "IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Stack Compositions
***********************************************************************/

			/// <summary>
			/// Represents a stack composition.
			/// </summary>
			class GuiStackComposition : public GuiBoundsComposition, public Description<GuiStackComposition>
			{
				friend class GuiStackItemComposition;

				typedef collections::List<GuiStackItemComposition*>				ItemCompositionList;
			public:
				/// <summary>Stack item layout direction.</summary>
				enum Direction
				{
					/// <summary>Stack items is layouted from left to right.</summary>
					Horizontal,
					/// <summary>Stack items is layouted from top to bottom.</summary>
					Vertical,
					/// <summary>Stack items is layouted from right to left.</summary>
					ReversedHorizontal,
					/// <summary>Stack items is layouted from bottom to top.</summary>
					ReversedVertical,
				};

			private:
				bool								layout_invalid = true;
				ItemCompositionList					layout_stackItems;
				GuiStackItemComposition*			layout_ensuringVisibleStackItem = nullptr;
				vint								layout_adjustment = 0;
				Size								layout_stackItemTotalSize;

				void								Layout_UpdateStackItemMinSizes();
				void								Layout_UpdateStackItemBounds(Rect contentBounds);
			protected:

				Direction							direction = Horizontal;
				vint								padding = 0;
				Margin								extraMargin;

				void								OnChildInserted(GuiGraphicsComposition* child) override;
				void								OnChildRemoved(GuiGraphicsComposition* child) override;
				void								OnCompositionStateChanged() override;
				Size								Layout_CalculateMinSize() override;
				Rect								Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiStackComposition() = default;
				~GuiStackComposition() = default;

				/// <summary>Get all stack items inside the stack composition.</summary>
				/// <returns>All stack items inside the stack composition.</returns>
				const ItemCompositionList&			GetStackItems();
				/// <summary>Insert a stack item at a specified position.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="index">The position.</param>
				/// <param name="item">The statck item to insert.</param>
				bool								InsertStackItem(vint index, GuiStackItemComposition* item);
				
				/// <summary>Get the stack item layout direction.</summary>
				/// <returns>The stack item layout direction.</returns>
				Direction							GetDirection();
				/// <summary>Set the stack item layout direction.</summary>
				/// <param name="value">The stack item layout direction.</param>
				void								SetDirection(Direction value);
				/// <summary>Get the stack item padding.</summary>
				/// <returns>The stack item padding.</returns>
				vint								GetPadding();
				/// <summary>Set the stack item padding.</summary>
				/// <param name="value">The stack item padding.</param>
				void								SetPadding(vint value);
				
				/// <summary>Get the extra margin inside the stack composition.</summary>
				/// <returns>The extra margin inside the stack composition.</returns>
				Margin								GetExtraMargin();
				/// <summary>Set the extra margin inside the stack composition.</summary>
				/// <param name="value">The extra margin inside the stack composition.</param>
				void								SetExtraMargin(Margin value);
				/// <summary>Test is any stack item clipped in the stack direction.</summary>
				/// <returns>Returns true if any stack item is clipped.</returns>
				bool								IsStackItemClipped();
				/// <summary>Make an item visible as complete as possible.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="index">The index of the item.</param>
				bool								EnsureVisible(vint index);
			};
			
			/// <summary>
			/// Represents a stack item composition of a <see cref="GuiStackComposition"/>.
			/// </summary>
			class GuiStackItemComposition : public GuiGraphicsComposition_Controlled, public Description<GuiStackItemComposition>
			{
				friend class GuiStackComposition;
			private:
				GuiStackComposition*				layout_stackParent = nullptr;
				Point								layout_virtualOffset;

				void								Layout_SetStackItemBounds(Rect contentBounds, Point virtualOffset);

			protected:
				Margin								extraMargin;

				void								OnParentLineChanged() override;
			public:
				GuiStackItemComposition();
				~GuiStackItemComposition() = default;
				
				/// <summary>Get the extra margin for this stack item. An extra margin is used to enlarge the bounds of the stack item, but only the non-extra part will be used for deciding the stack item layout.</summary>
				/// <returns>The extra margin for this stack item.</returns>
				Margin								GetExtraMargin();
				/// <summary>Set the extra margin for this stack item. An extra margin is used to enlarge the bounds of the stack item, but only the non-extra part will be used for deciding the stack item layout.</summary>
				/// <param name="value">The extra margin for this stack item.</param>
				void								SetExtraMargin(Margin value);
			};
		}
	}
}

#endif