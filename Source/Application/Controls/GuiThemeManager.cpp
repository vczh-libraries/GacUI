#include "GuiThemeManager.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{

/***********************************************************************
GuiTemplate
***********************************************************************/

			GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			controls::GuiControlHost* GuiTemplate::GetControlHostForInstance()
			{
				return GetRelatedControlHost();
			}

			void GuiTemplate::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				OnControlHostForInstanceChanged();
			}

			GuiTemplate::GuiTemplate()
			{
				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTemplate::~GuiTemplate()
			{
				FinalizeInstanceRecursively(this);
			}

/***********************************************************************
Template Declarations
***********************************************************************/

			GUI_CORE_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_IMPL)
		}
	}
}