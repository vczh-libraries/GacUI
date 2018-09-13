/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSBASICCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSBASICCOMPOSITION

#include "GuiGraphicsCompositionBase.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Basic Compositions
***********************************************************************/
			
			/// <summary>
			/// Represents a composition for the client area in an <see cref="INativeWindow"/>.
			/// </summary>
			class GuiWindowComposition : public GuiGraphicsSite, public Description<GuiWindowComposition>
			{
			public:
				GuiWindowComposition();
				~GuiWindowComposition();

				Rect								GetBounds()override;
				void								SetMargin(Margin value)override;
			};

			/// <summary>
			/// Represents a composition that is free to change the expected bounds.
			/// </summary>
			class GuiBoundsComposition : public GuiGraphicsSite, public Description<GuiBoundsComposition>
			{
			protected:
				bool								sizeAffectParent = true;
				Rect								compositionBounds;
				Margin								alignmentToParent{ -1,-1,-1,-1 };
				
			public:
				GuiBoundsComposition();
				~GuiBoundsComposition();

				/// <summary>Get if the parent composition's size calculation is aware of the configuration of this composition. If you want to bind Bounds, PreferredMinSize, AlignmentToParent or other similar properties to some properties of parent compositions, this property should be set to false to prevent from infinite size glowing.</summary>
				/// <returns>Returns true if it is awared.</returns>
				bool								GetSizeAffectParent();
				/// <summary>Set if the parent composition's size calculation is aware of the configuration of this composition.</summary>
				/// <param name="value">Set to true to be awared.</param>
				void								SetSizeAffectParent(bool value);
				
				bool								IsSizeAffectParent()override;
				Rect								GetPreferredBounds()override;
				Rect								GetBounds()override;
				/// <summary>Set the expected bounds.</summary>
				/// <param name="value">The expected bounds.</param>
				void								SetBounds(Rect value);

				/// <summary>Get the alignment to its parent. -1 in each alignment component means that the corressponding side is not aligned to its parent.</summary>
				/// <returns>The alignment to its parent.</returns>
				Margin								GetAlignmentToParent();
				/// <summary>Set the alignment to its parent. -1 in each alignment component means that the corressponding side is not aligned to its parent.</summary>
				/// <param name="value">The alignment to its parent.</param>
				void								SetAlignmentToParent(Margin value);
				/// <summary>Test is the composition aligned to its parent.</summary>
				/// <returns>Returns true if the composition is aligned to its parent.</returns>
				bool								IsAlignedToParent();
			};
		}
	}
}

#endif