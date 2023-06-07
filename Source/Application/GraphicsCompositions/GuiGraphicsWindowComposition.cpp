#include "GuiGraphicsWindowComposition.h"
#include "../GraphicsHost/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiWindowComposition
***********************************************************************/

			GuiWindowComposition::GuiWindowComposition()
				: GuiGraphicsComposition(true)
			{
			}

			GuiWindowComposition::~GuiWindowComposition()
			{
			}

			Rect GuiWindowComposition::GetBounds()
			{
				Rect bounds;
				if (relatedHostRecord)
				{
					if (auto window = relatedHostRecord->host->GetNativeWindow())
					{
						bounds = Rect(Point(0, 0), window->Convert(window->GetClientSize()));
					}
				}
				UpdatePreviousBounds(bounds);
				return bounds;
			}
		}
	}
}