/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Common Style Helpers

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITHEMESTYLEFACTORY
#define VCZH_PRESENTATION_CONTROLS_GUITHEMESTYLEFACTORY

#include "GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{
			class Theme;

			/// <summary>Partial control template collections. [F:vl.presentation.theme.GetCurrentTheme] will returns an object, which walks through multiple registered [T:vl.presentation.theme.ThemeTemplates] to create a correct template object for a control.</summary>
			class ThemeTemplates : public controls::GuiInstanceRootObject, public AggregatableDescription<ThemeTemplates>
			{
				friend class Theme;
			protected:
				ThemeTemplates*					previous = nullptr;
				ThemeTemplates*					next = nullptr;

				controls::GuiControlHost*		GetControlHostForInstance()override;
			public:
				~ThemeTemplates();

				WString							Name;
				Nullable<bool>					PreferCustomFrameWindow;

#define GUI_DEFINE_ITEM_PROPERTY(TEMPLATE, CONTROL) TemplateProperty<templates::Gui##TEMPLATE> CONTROL;
				GUI_CONTROL_TEMPLATE_TYPES(GUI_DEFINE_ITEM_PROPERTY)
#undef GUI_DEFINE_ITEM_PROPERTY
			};

			/// <summary>Register a control template collection object.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="theme">The control template collection object.</param>
			extern bool							RegisterTheme(Ptr<ThemeTemplates> theme);
			/// <summary>Unregister a control template collection object.</summary>
			/// <returns>The registered object. Returns null if it does not exist.</returns>
			/// <param name="name">The name of the theme.</param>
			extern Ptr<ThemeTemplates>			UnregisterTheme(const WString& name);
		}
	}
}

#endif