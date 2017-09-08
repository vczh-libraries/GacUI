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

			GuiLabel::GuiLabel(ControlTemplateType* _controlTemplate)
				:GuiControl(_styleController)
				,controlTemplate(_controlTemplate)
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