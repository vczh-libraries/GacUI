#include "GuiLabelControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiLabel
***********************************************************************/

			GuiLabel::GuiLabel(IStyleController* _styleController)
				:GuiControl(_styleController)
				,styleController(_styleController)
			{
				textColor=styleController->GetDefaultTextColor();
				styleController->SetTextColor(textColor);
			}

			GuiLabel::~GuiLabel()
			{
			}

			Color GuiLabel::GetTextColor()
			{
				return textColor;
			}

			void GuiLabel::SetTextColor(Color value)
			{
				if(textColor!=value)
				{
					textColor=value;
					styleController->SetTextColor(textColor);
				}
			}
		}
	}
}