/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSPECIALIZEDCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSPECIALIZEDCOMPOSITION

#include "IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Specialized Compositions
***********************************************************************/

			/// <summary>
			/// Represents a composition that is aligned to one border of the parent composition.
			/// </summary>
			class GuiSideAlignedComposition : public GuiGraphicsComposition_Specialized, public Description<GuiSideAlignedComposition>
			{
			public:
				/// <summary>The border to align.</summary>
				enum Direction
				{
					/// <summary>The left border.</summary>
					Left,
					/// <summary>The top border.</summary>
					Top,
					/// <summary>The right border.</summary>
					Right,
					/// <summary>The bottom border.</summary>
					Bottom,
				};
			protected:
				Direction							direction = Top;
				vint								maxLength = 10;
				double								maxRatio = 1.0;

				Rect								Layout_CalculateBounds(Size parentSize) override;

			public:
				GuiSideAlignedComposition() = default;
				~GuiSideAlignedComposition() = default;
				
				/// <summary>Get the border to align.</summary>
				/// <returns>The border to align.</returns>
				Direction							GetDirection();
				/// <summary>Set the border to align.</summary>
				/// <param name="value">The border to align.</param>
				void								SetDirection(Direction value);
				/// <summary>Get the maximum length of this composition.</summary>
				/// <returns>The maximum length of this composition.</returns>
				vint								GetMaxLength();
				/// <summary>Set the maximum length of this composition.</summary>
				/// <param name="value">The maximum length of this composition.</param>
				void								SetMaxLength(vint value);
				/// <summary>Get the maximum ratio to limit the size according to the size of the parent.</summary>
				/// <returns>The maximum ratio to limit the size according to the size of the parent.</returns>
				double								GetMaxRatio();
				/// <summary>Set the maximum ratio to limit the size according to the size of the parent.</summary>
				/// <param name="value">The maximum ratio to limit the size according to the size of the parent.</param>
				void								SetMaxRatio(double value);
			};

			/// <summary>
			/// Represents a composition that its location and size are decided by the client area of the parent composition by setting ratios.
			/// </summary>
			class GuiPartialViewComposition : public GuiGraphicsComposition_Specialized, public Description<GuiPartialViewComposition>
			{
			protected:
				double								wRatio = 0.0;
				double								wPageSize = 1.0;
				double								hRatio = 0.0;
				double								hPageSize = 1.0;

				Rect								Layout_CalculateBounds(Size parentSize) override;

			public:
				GuiPartialViewComposition() = default;
				~GuiPartialViewComposition() = default;
				
				/// <summary>Get the width ratio to decided the horizontal location. Value in [0, 1-pageSize].</summary>
				/// <returns>The width ratio to decided the horizontal location.</returns>
				double								GetWidthRatio();
				/// <summary>Get the page size to decide the horizontal size. Value in [0, 1].</summary>
				/// <returns>The page size to decide the horizontal size.</returns>
				double								GetWidthPageSize();
				/// <summary>Get the height ratio to decided the vertical location. Value in [0, 1-pageSize].</summary>
				/// <returns>The height ratio to decided the vertical location.</returns>
				double								GetHeightRatio();
				/// <summary>Get the page size to decide the vertical size. Value in [0, 1].</summary>
				/// <returns>The page size to decide the vertical size.</returns>
				double								GetHeightPageSize();
				/// <summary>Set the width ratio to decided the horizontal location. Value in [0, 1-pageSize].</summary>
				/// <param name="value">The width ratio to decided the horizontal location.</param>
				void								SetWidthRatio(double value);
				/// <summary>Set the page size to decide the horizontal size. Value in [0, 1].</summary>
				/// <param name="value">The page size to decide the horizontal size.</param>
				void								SetWidthPageSize(double value);
				/// <summary>Set the height ratio to decided the vertical location. Value in [0, 1-pageSize].</summary>
				/// <param name="value">The height ratio to decided the vertical location.</param>
				void								SetHeightRatio(double value);
				/// <summary>Set the page size to decide the vertical size. Value in [0, 1].</summary>
				/// <param name="value">The page size to decide the vertical size.</param>
				void								SetHeightPageSize(double value);
			};
		}
	}
}

#endif