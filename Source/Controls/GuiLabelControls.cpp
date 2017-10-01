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

			GuiLabel::GuiLabel(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				textColor=controlTemplate->GetDefaultTextColor();
				controlTemplate->SetTextColor(textColor);
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
					controlTemplate->SetTextColor(textColor);
				}
			}
		}
	}
}