/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSBOUNDSCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSBOUNDSCOMPOSITION

#include "../GraphicsCompositions/GuiGraphicsComposition.h"

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
			/// Represents a composition that is free to change the expected bounds.
			/// </summary>
			class GuiBoundsComposition : public GuiGraphicsComposition_Trivial, public Description<GuiBoundsComposition>
			{
			protected:
				Rect								expectedBounds;
				Margin								alignmentToParent{ -1,-1,-1,-1 };

				Size								Layout_CalculateMinSize() override;
				Size								Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin) override;
				Rect								Layout_CalculateBounds(Size parentSize) override;
			public:
				GuiBoundsComposition() = default;
				~GuiBoundsComposition() = default;

				/// <summary>Get the expected bounds.</summary>
				/// <returns>The expected bounds.</returns>
				Rect								GetExpectedBounds();
				/// <summary>Set the expected bounds.</summary>
				/// <param name="value">The expected bounds.</param>
				void								SetExpectedBounds(Rect value);

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