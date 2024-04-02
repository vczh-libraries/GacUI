/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Application

Interfaces:
  GuiHostedController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDAPPLICATION
#define VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDAPPLICATION

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation
{
/***********************************************************************
IGuiHostedApplication
***********************************************************************/

	/// <summary>
	/// 
	/// </summary>
	class IGuiHostedApplication : public virtual Interface
	{
	public:

		virtual INativeWindow*				GetNativeWindowHost() = 0;
	};

	extern IGuiHostedApplication*			GetHostedApplication();
	extern void								SetHostedApplication(IGuiHostedApplication* _hostedApp);
}

#endif