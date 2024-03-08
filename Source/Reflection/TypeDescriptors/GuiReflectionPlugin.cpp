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
#define GUIREFLECTIONTEMPLATES_IMPL_VL_TYPE_INFO(NAME, BASE) IMPL_VL_TYPE_INFO(presentation::templates::NAME)

			GUIREFLECTIONBASIC_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONELEMENT_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONCOMPOSITION_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONEVENT_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONTEMPLATES_TYPELIST(IMPL_VL_TYPE_INFO)
			GUIREFLECTIONCONTROLS_TYPELIST(IMPL_VL_TYPE_INFO)

#undef GUIREFLECTIONTEMPLATES_IMPL_VL_TYPE_INFO

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			extern bool LoadGuiBasicTypes();
			extern bool LoadGuiElementTypes();
			extern bool LoadGuiCompositionTypes();
			extern bool LoadGuiEventTypes();
			extern bool LoadGuiTemplateTypes();
			extern bool LoadGuiControlTypes();

			using namespace presentation::controls;

			class GuiReflectionPlugin : public Object, public presentation::IGuiPlugin
			{
			public:

				GUI_PLUGIN_NAME(GacUI_Instance_Reflection)
				{
				}

				void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
				{
					if (controllerUnrelatedPlugins)
					{
						LoadPredefinedTypes();
						LoadParsing2Types();
						XmlAstLoadTypes();
						JsonAstLoadTypes();
						WfLoadLibraryTypes();
						LoadGuiBasicTypes();
						LoadGuiElementTypes();
						LoadGuiCompositionTypes();
						LoadGuiEventTypes();
						LoadGuiTemplateTypes();
						LoadGuiControlTypes();
					}
				}

				void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiReflectionPlugin)
#endif
		}
	}
}