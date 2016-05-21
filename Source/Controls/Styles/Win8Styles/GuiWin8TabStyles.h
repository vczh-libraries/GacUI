/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8TABSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8TABSTYLES

#include "../Win7Styles/GuiWin7TabStyles.h"
#include "GuiWin8ButtonStyles.h"
#include "GuiWin8MenuStyles.h"
#include "GuiWin8ControlStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Tab
***********************************************************************/

			/// <summary>Tab page header style (Windows 8).</summary>
			class Win8TabPageHeaderStyle : public Win8ButtonStyleBase, public Description<Win8TabPageHeaderStyle>
			{
			protected:
				void														TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)override;
			public:
				/// <summary>Create the style.</summary>
				Win8TabPageHeaderStyle();
				~Win8TabPageHeaderStyle();

				void														SetFont(const FontProperties& value)override;
			};

			/// <summary>Tab control style (Windows 8).</summary>
			class Win8TabStyle : public win7::Win7TabStyle, public Description<Win8TabStyle>
			{
			protected:
				
				controls::GuiSelectableButton::IStyleController*			CreateHeaderStyleController()override;
				controls::GuiButton::IStyleController*						CreateMenuButtonStyleController()override;
				controls::GuiToolstripMenu::IStyleController*				CreateMenuStyleController()override;
				controls::GuiToolstripButton::IStyleController*				CreateMenuItemStyleController()override;
				Color														GetBorderColor()override;
				Color														GetBackgroundColor()override;
			public:
				/// <summary>Create the style.</summary>
				Win8TabStyle();
				~Win8TabStyle();
			};
		}
	}
}

#endif