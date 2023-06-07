/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSCOMPOSITIONBASE
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSCOMPOSITIONBASE

#include "GuiGraphicsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			/// <summary>
			/// Represents a composition for the client area in an <see cref="INativeWindow"/>.
			/// </summary>
			class GuiWindowComposition : public GuiGraphicsComposition, public Description<GuiWindowComposition>
			{
			public:
				GuiWindowComposition();
				~GuiWindowComposition();

				Rect								GetBounds()override;
			};
		}
	}
}

#endif