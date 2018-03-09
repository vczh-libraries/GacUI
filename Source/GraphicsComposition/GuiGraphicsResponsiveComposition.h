/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESPONSIVECOMPOSITION
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESPONSIVECOMPOSITION

#include "GuiGraphicsBasicComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			/// <summary>Base class for responsive layout compositions.</summary>
			class GuiResponsiveCompositionBase abstract : public GuiBoundsComposition, public Description<GuiResponsiveCompositionBase>
			{
			public:
				GuiResponsiveCompositionBase();
				~GuiResponsiveCompositionBase();
			};

			/// <summary>A responsive layout composition defined by views of different sizes.</summary>
			class GuiResponsiveViewComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveViewComposition>
			{
			public:
				GuiResponsiveViewComposition();
				~GuiResponsiveViewComposition();
			};

			/// <summary>A responsive layout composition which stop parent responsive composition to search its children.</summary>
			class GuiFixedLayoutComposition : public GuiResponsiveCompositionBase, public Description<GuiFixedLayoutComposition>
			{
			public:
				GuiFixedLayoutComposition();
				~GuiFixedLayoutComposition();
			};

			/// <summary>A responsive layout composition which change its size by changing children's views one by one in one direction.</summary>
			class GuiResponsiveStackComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveStackComposition>
			{
			public:
				GuiResponsiveStackComposition();
				~GuiResponsiveStackComposition();
			};

			/// <summary>A responsive layout composition which change its size by changing children's views at the same time.</summary>
			class GuiResponsiveGroupComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveGroupComposition>
			{
			public:
				GuiResponsiveGroupComposition();
				~GuiResponsiveGroupComposition();
			};
		}
	}
}

#endif