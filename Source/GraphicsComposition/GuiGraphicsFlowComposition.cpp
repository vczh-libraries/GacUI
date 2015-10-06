#include "GuiGraphicsFlowComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiFlowComposition
***********************************************************************/

			GuiFlowComposition::GuiFlowComposition()
			{
			}

			GuiFlowComposition::~GuiFlowComposition()
			{
			}

/***********************************************************************
GuiFlowItemComposition
***********************************************************************/

			GuiFlowItemComposition::GuiFlowItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiFlowItemComposition::~GuiFlowItemComposition()
			{
			}

			Rect GuiFlowItemComposition::GetBounds()
			{
				throw 0;
			}
		}
	}
}