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

			Rect GuiWindowComposition::Layout_CalculateBounds(Size parentSize)
			{
				Rect bounds;
				if (relatedHostRecord)
				{
					if (auto window = relatedHostRecord->host->GetNativeWindow())
					{
						bounds = Rect(Point(0, 0), window->Convert(window->GetClientSize()));
					}
				}
				return bounds;
			}
		}
	}
}