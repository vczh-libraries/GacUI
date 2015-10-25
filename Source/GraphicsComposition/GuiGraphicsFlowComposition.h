/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSFLOWCOMPOSITION
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSFLOWCOMPOSITION

#include "GuiGraphicsBasicComposition.h"
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

			enum class FlowAlignment
			{
				Left,
				Center,
				Extend,
			};

			class GuiFlowComposition : public GuiBoundsComposition, public Description<GuiFlowComposition>
			{
				friend class GuiFlowItemComposition;

				typedef collections::List<GuiFlowItemComposition*>				ItemCompositionList;
			protected:
				Margin								extraMargin;
				vint								rowPadding = 0;
				vint								columnPadding = 0;
				FlowAlignment						alignment = FlowAlignment::Left;
				Ptr<IGuiAxis>						axis;

				ItemCompositionList					flowItems;
				collections::Array<Rect>			flowItemBounds;
				Rect								bounds;

				void								UpdateFlowItemBounds();
				void								OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments);
				void								OnChildInserted(GuiGraphicsComposition* child)override;
				void								OnChildRemoved(GuiGraphicsComposition* child)override;
			public:
				GuiFlowComposition();
				~GuiFlowComposition();

				Margin								GetExtraMargin();
				void								SetExtraMargin(Margin value);
				vint								GetRowPadding();
				void								SetRowPadding(vint value);
				vint								GetColumnPadding();
				void								SetColumnPadding(vint value);
				Ptr<IGuiAxis>						GetAxis();
				void								SetAxis(Ptr<IGuiAxis> value);
				FlowAlignment						GetAlignment();
				void								SetAlignment(FlowAlignment value);

				Size								GetMinPreferredClientSize()override;
				Rect								GetBounds()override;
			};

			struct GuiFlowOption
			{
				enum BaselineType
				{
					Percentage,
					FromTop,
					FromBottom,
				};

				BaselineType						baseline = FromBottom;
				double								percentage = 0.0;
				vint								distance = 0;
			};

			class GuiFlowItemComposition : public GuiGraphicsSite, public Description<GuiFlowItemComposition>
			{
				friend class GuiFlowComposition;
			protected:
				GuiFlowComposition*					flowParent;
				Rect								bounds;
				Margin								extraMargin;
				GuiFlowOption						option;

				void								OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)override;
				Size								GetMinSize();
			public:
				GuiFlowItemComposition();
				~GuiFlowItemComposition();
				
				bool								IsSizeAffectParent()override;
				Rect								GetBounds()override;
				void								SetBounds(Rect value);

				Margin								GetExtraMargin();
				void								SetExtraMargin(Margin value);

				GuiFlowOption						GetFlowOption();
				void								SetFlowOption(GuiFlowOption value);
			};
		}
	}
}

#endif