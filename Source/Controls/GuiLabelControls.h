/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILABELCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUILABELCONTROLS

#include "GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Label
***********************************************************************/

			/// <summary>A control to display a text.</summary>
			class GuiLabel : public GuiControl, public Description<GuiLabel>
			{
			public:
				/// <summary>Style controller interface for <see cref="GuiLabel"/>.</summary>
				class IStyleController : virtual public GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Get the default text color.</summary>
					/// <returns>The default text color.</returns>
					virtual Color						GetDefaultTextColor() = 0;
					/// <summary>Called when the text color changed.</summary>
					/// <param name="value">The new text color.</param>
					virtual void						SetTextColor(Color value) = 0;
				};
			protected:
				Color									textColor;
				IStyleController*						styleController;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiLabel(IStyleController* _styleController);
				~GuiLabel();

				/// <summary>Get the text color.</summary>
				/// <returns>The text color.</returns>
				Color									GetTextColor();
				/// <summary>Set the text color.</summary>
				/// <param name="value">The text color.</param>
				void									SetTextColor(Color value);
			};
		}
	}
}

#endif
