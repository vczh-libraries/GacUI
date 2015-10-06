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

			class GuiFlowComposition : public GuiBoundsComposition, public Description<GuiFlowComposition>
			{
			public:
				GuiFlowComposition();
				~GuiFlowComposition();
			};

			class GuiFlowItemComposition : public GuiGraphicsSite, public Description<GuiFlowItemComposition>
			{
			public:
				GuiFlowItemComposition();
				~GuiFlowItemComposition();

				Rect							GetBounds()override;
			};
		}
	}
}

#endif