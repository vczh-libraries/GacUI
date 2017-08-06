#include "GuiReflectionPlugin.h"

/***********************************************************************
Plugin
***********************************************************************/

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			GUIREFLECTIONBASIC_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONELEMENT_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONCOMPOSITION_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONEVENT_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONTEMPLATES_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONCONTROLS_TYPELIST(IMPL_VL_TYPE_INFO)

			extern bool LoadGuiBasicTypes();
			extern bool LoadGuiElementTypes();
			extern bool LoadGuiCompositionTypes();
			extern bool LoadGuiEventTypes();
			extern bool LoadGuiTemplateTypes();
			extern bool LoadGuiControlTypes();

			using namespace presentation::controls;

			class GuiReflectionPlugin : public Object, public IGuiPlugin
			{
			public:

				GUI_PLUGIN_NAME(GacUI_Reflection)
				{
				}

				void Load()override
				{
					LoadPredefinedTypes();
					LoadParsingTypes();
					XmlLoadTypes();
					JsonLoadTypes();
					LoadGuiBasicTypes();
					LoadGuiElementTypes();
					LoadGuiCompositionTypes();
					LoadGuiEventTypes();
					LoadGuiTemplateTypes();
					LoadGuiControlTypes();
				}

				void Unload()override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiReflectionPlugin)
		}
	}
}