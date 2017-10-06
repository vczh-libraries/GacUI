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

			void GuiLabel::BeforeControlTemplateUninstalled_()
			{
				textColorConsisted = (textColor == GetControlTemplateObject()->GetDefaultTextColor());
			}

			void GuiLabel::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				if (initialize || textColorConsisted)
				{
					SetTextColor(ct->GetDefaultTextColor());
				}
				else
				{
					ct->SetTextColor(textColor);
				}
			}

			GuiLabel::GuiLabel(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
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
				if (textColor != value)
				{
					textColor = value;
					GetControlTemplateObject()->SetTextColor(textColor);
				}
			}
		}
	}
}