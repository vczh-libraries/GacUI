#include "GuiGraphicsResponsiveComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiResponsiveCompositionBase
***********************************************************************/

			GuiResponsiveCompositionBase::GuiResponsiveCompositionBase()
			{
			}

			GuiResponsiveCompositionBase::~GuiResponsiveCompositionBase()
			{
			}

/***********************************************************************
GuiResponsiveViewComposition
***********************************************************************/

			GuiResponsiveViewComposition::GuiResponsiveViewComposition()
			{
			}

			GuiResponsiveViewComposition::~GuiResponsiveViewComposition()
			{
			}

/***********************************************************************
GuiResponsiveFixedComposition
***********************************************************************/

			GuiResponsiveFixedComposition::GuiResponsiveFixedComposition()
			{
			}

			GuiResponsiveFixedComposition::~GuiResponsiveFixedComposition()
			{
			}

			vint GuiResponsiveFixedComposition::GetLevelCount()
			{
				return 1;
			}

			vint GuiResponsiveFixedComposition::GetCurrentLevel()
			{
				return 0;
			}

			bool GuiResponsiveFixedComposition::LevelDown()
			{
				return false;
			}

			bool GuiResponsiveFixedComposition::LevelUp()
			{
				return false;
			}

/***********************************************************************
GuiResponsiveStackComposition
***********************************************************************/

			GuiResponsiveStackComposition::GuiResponsiveStackComposition()
			{
			}

			GuiResponsiveStackComposition::~GuiResponsiveStackComposition()
			{
			}

/***********************************************************************
GuiResponsiveGroupComposition
***********************************************************************/

			GuiResponsiveGroupComposition::GuiResponsiveGroupComposition()
			{
			}

			GuiResponsiveGroupComposition::~GuiResponsiveGroupComposition()
			{
			}
		}
	}
}
