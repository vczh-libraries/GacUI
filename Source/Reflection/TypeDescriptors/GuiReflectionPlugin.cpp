#include "GuiReflectionBasic.h"
#include "GuiReflectionElements.h"
#include "GuiReflectionCompositions.h"
#include "GuiReflectionControls.h"
#include "GuiReflectionTemplates.h"
#include "GuiReflectionEvents.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			class GuiReflectionPlugin : public Object, public IGuiPlugin
			{
			public:
				void Load()override
				{
					LoadPredefinedTypes();
					LoadParsingTypes();
					XmlLoadTypes();
					JsonLoadTypes();
					LoadGuiBasicTypes();
					LoadGuiElementTypes();
					LoadGuiCompositionTypes();
					LoadGuiControlTypes();
					LoadGuiTemplateTypes();
					LoadGuiEventTypes();
				}
				
				void AfterLoad()override
				{
				}

				void Unload()override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiReflectionPlugin)
		}
	}
}