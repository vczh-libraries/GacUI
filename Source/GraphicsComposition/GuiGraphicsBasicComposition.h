/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSBASICCOMPOSITION
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSBASICCOMPOSITION

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
			protected:
				INativeWindow*						attachedWindow;
			public:
				GuiWindowComposition();
				~GuiWindowComposition();
				
				/// <summary>Get the attached native window object.</summary>
				/// <returns>The attached native window object.</returns>
				INativeWindow*						GetAttachedWindow();
				/// <summary>Attached a native window object.</summary>
				/// <param name="window">The native window object to attach.</param>
				void								SetAttachedWindow(INativeWindow* window);

				Rect								GetBounds()override;
				void								SetMargin(Margin value)override;
			};

			/// <summary>
			/// Represents a composition that is free to change the expected bounds.
			/// </summary>
			class GuiBoundsComposition : public GuiGraphicsSite, public Description<GuiBoundsComposition>
			{
			protected:
				Rect								compositionBounds;
				Margin								alignmentToParent;
				
			public:
				GuiBoundsComposition();
				~GuiBoundsComposition();
				
				Rect								GetPreferredBounds()override;
				Rect								GetBounds()override;
				/// <summary>Set the expected bounds.</summary>
				/// <param name="value">The expected bounds.</param>
				void								SetBounds(Rect value);

				/// <summary>Make the composition not aligned to its parent.</summary>
				void								ClearAlignmentToParent();
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