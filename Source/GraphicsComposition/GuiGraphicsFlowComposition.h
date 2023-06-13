/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSFLOWCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSFLOWCOMPOSITION

#include "IncludeForward.h"
#include "GuiGraphicsAxis.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
/***********************************************************************
Flow Compositions
***********************************************************************/

			/// <summary>
			/// Alignment for a row in a flow layout
			/// </summary>
			enum class FlowAlignment
			{
				/// <summary>Align to the left.</summary>
				Left,
				/// <summary>Align to the center.</summary>
				Center,
				/// <summary>Align to the Right.</summary>
				Right,
				/// <summary>Extend to the entire row.</summary>
				Extend,
			};
			
			/// <summary>
			/// Represents a flow composition.
			/// </summary>
			class GuiFlowComposition : public GuiBoundsComposition, public Description<GuiFlowComposition>
			{
				friend class GuiFlowItemComposition;

				typedef collections::List<GuiFlowItemComposition*>				ItemCompositionList;
			private:
				bool								layout_invalid = true;
				vint								layout_lastVirtualWidth = 0;
				ItemCompositionList					layout_flowItems;
				vint								layout_minVirtualHeight = 0;

				void								Layout_UpdateFlowItemLayout(vint maxVirtualWidth);
				Size								Layout_UpdateFlowItemLayoutByConstraint(Size constraintSize);

			protected:
				Margin								extraMargin;
				vint								rowPadding = 0;
				vint								columnPadding = 0;
				FlowAlignment						alignment = FlowAlignment::Left;
				Ptr<IGuiAxis>						axis = Ptr(new GuiDefaultAxis);

				void								OnChildInserted(GuiGraphicsComposition* child) override;
				void								OnChildRemoved(GuiGraphicsComposition* child) override;
				void								OnCompositionStateChanged() override;
				Size								Layout_CalculateMinSize() override;
				Rect								Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiFlowComposition() = default;
				~GuiFlowComposition() = default;
				
				/// <summary>Get all flow items inside the flow composition.</summary>
				/// <returns>All flow items inside the flow composition.</returns>
				const ItemCompositionList&			GetFlowItems();
				/// <summary>Insert a flow item at a specified position.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="index">The position.</param>
				/// <param name="item">The flow item to insert.</param>
				bool								InsertFlowItem(vint index, GuiFlowItemComposition* item);
				
				/// <summary>Get the extra margin inside the flow composition.</summary>
				/// <returns>The extra margin inside the flow composition.</returns>
				Margin								GetExtraMargin();
				/// <summary>Set the extra margin inside the flow composition.</summary>
				/// <param name="value">The extra margin inside the flow composition.</param>
				void								SetExtraMargin(Margin value);
				
				/// <summary>Get the distance between rows.</summary>
				/// <returns>The distance between rows.</returns>
				vint								GetRowPadding();
				/// <summary>Set the distance between rows.</summary>
				/// <param name="value">The distance between rows.</param>
				void								SetRowPadding(vint value);
				
				/// <summary>Get the distance between columns.</summary>
				/// <returns>The distance between columns.</returns>
				vint								GetColumnPadding();
				/// <summary>Set the distance between columns.</summary>
				/// <param name="value">The distance between columns.</param>
				void								SetColumnPadding(vint value);
				
				/// <summary>Get the axis of the layout.</summary>
				/// <returns>The axis.</returns>
				Ptr<IGuiAxis>						GetAxis();
				/// <summary>Set the axis of the layout.</summary>
				/// <param name="value">The axis.</param>
				void								SetAxis(Ptr<IGuiAxis> value);
				
				/// <summary>Get the alignment for rows.</summary>
				/// <returns>The alignment.</returns>
				FlowAlignment						GetAlignment();
				/// <summary>Set the alignment for rows.</summary>
				/// <param name="value">The alignment.</param>
				void								SetAlignment(FlowAlignment value);
			};
			
			/// <summary>
			/// Represnets a base line configuration for a flow item.
			/// </summary>
			struct GuiFlowOption
			{
				/// <summary>
				/// Specify the the relationship between this item and the baseline of the row that this item currently belongs to.
				/// The height of a row is the maximum value of minimum heights of all items in it.
				/// </summary>
				enum BaselineType
				{
					/// <summary>Top of this item is "percentage" times of the item height above the baseline.</summary>
					Percentage,
					/// <summary>Top of this item is "distance" above the baseline.</summary>
					FromTop,
					/// <summary>Bottom of this item is "distance" below the baseline.</summary>
					FromBottom,
				};
				
				/// <summary>The base line calculation algorithm.</summary>
				BaselineType						baseline = FromBottom;
				/// <summary>The percentage value.</summary>
				double								percentage = 0.0;
				/// <summary>The distance value.</summary>
				vint								distance = 0;

				GUI_DEFINE_COMPARE_OPERATORS(GuiFlowOption)
			};
			
			/// <summary>
			/// Represents a flow item composition of a <see cref="GuiFlowComposition"/>.
			/// </summary>
			class GuiFlowItemComposition : public GuiGraphicsComposition_Controlled, public Description<GuiFlowItemComposition>
			{
				friend class GuiFlowComposition;
			private:
				GuiFlowComposition*					layout_flowParent = nullptr;
				Rect								layout_virtualBounds;

				void								Layout_SetFlowItemBounds(Size contentSize, Rect virtualBounds);
			protected:
				Margin								extraMargin;
				GuiFlowOption						option;

				void								OnParentLineChanged() override;

			public:
				GuiFlowItemComposition();
				~GuiFlowItemComposition() = default;
				
				/// <summary>Get the extra margin for this flow item. An extra margin is used to enlarge the bounds of the flow item, but only the non-extra part will be used for deciding the flow item layout.</summary>
				/// <returns>The extra margin for this flow item.</returns>
				Margin								GetExtraMargin();
				/// <summary>Set the extra margin for this flow item. An extra margin is used to enlarge the bounds of the flow item, but only the non-extra part will be used for deciding the flow item layout.</summary>
				/// <param name="value">The extra margin for this flow item.</param>
				void								SetExtraMargin(Margin value);

				/// <summary>Get the base line option for this flow item.</summary>
				/// <returns>The base line option.</returns>
				GuiFlowOption						GetFlowOption();
				/// <summary>Set the base line option for this flow item.</summary>
				/// <param name="value">The base line option.</param>
				void								SetFlowOption(GuiFlowOption value);
			};
		}
	}
}

#endif
