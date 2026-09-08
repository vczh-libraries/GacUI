#ifndef VCZH_PRESENTATION_CONTROLS_TUIITEMTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TUIITEMTEMPLATES

#include "../Templates/GuiControlTemplates.h"

namespace vl::presentation::controls::list
{
	extern templates::TuiListItemBackgroundTemplate*	TuiGetItemBackground(compositions::GuiGraphicsComposition* item);
	extern Color										TuiGetItemTextColor(compositions::GuiGraphicsComposition* item, Color fallback);
	extern void											TuiUpdateGridCellColors(templates::GuiGridVisualizerTemplate* cell);
	extern void											TuiInitializeItemBackground(templates::GuiListItemTemplate* item, GuiSelectableButton* background);
}

#endif
