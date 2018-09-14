/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_INCLUDEFORWARD
#define VCZH_PRESENTATION_COMPOSITION_INCLUDEFORWARD

#include "GuiGraphicsBasicComposition.h"
#include "GuiGraphicsAxis.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiTableComposition;
			class GuiCellComposition;
			class GuiTableSplitterCompositionBase;
			class GuiRowSplitterComposition;
			class GuiColumnSplitterComposition;

			class GuiStackComposition;
			class GuiStackItemComposition;
			class GuiFlowComposition;
			class GuiFlowItemComposition;

			class GuiSideAlignedComposition;
			class GuiPartialViewComposition;

			class GuiResponsiveCompositionBase;
			class GuiResponsiveViewComposition;
			class GuiResponsiveSharedComposition;
			class GuiResponsiveFixedComposition;
			class GuiResponsiveStackComposition;
			class GuiResponsiveGroupComposition;
			class GuiResponsiveContainerComposition;

			class GuiSharedSizeItemComposition;
			class GuiSharedSizeRootComposition;

			class GuiRepeatCompositionBase;
			class GuiRepeatStackComposition;
			class GuiRepeatFlowComposition;
		}
	}
}

#endif