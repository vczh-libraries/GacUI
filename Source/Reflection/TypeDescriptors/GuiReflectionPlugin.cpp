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
			GUIREFLECTIONTEMPLATES_EXTRA_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONCONTROLS_TYPELIST(IMPL_VL_TYPE_INFO)

#define GUIREFLECTIONTEMPLATE_IMPL_VL_TYPE_INFO(CLASS, BASE) IMPL_VL_TYPE_INFO(presentation::templates::##CLASS)
			GUI_CONTROL_TEMPLATE_DECL(GUIREFLECTIONTEMPLATE_IMPL_VL_TYPE_INFO)
			GUI_ITEM_TEMPLATE_DECL(GUIREFLECTIONTEMPLATE_IMPL_VL_TYPE_INFO)
#undef GUIREFLECTIONTEMPLATE_IMPL_VL_TYPE_INFO

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

				GUI_PLUGIN_NAME(GacUI_Instance_Reflection)
				{
				}

				void Load()override
				{
					LoadPredefinedTypes();
					LoadParsingTypes();
					XmlLoadTypes();
					JsonLoadTypes();
					WfLoadLibraryTypes();
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