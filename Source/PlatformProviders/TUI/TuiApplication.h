#ifndef VCZH_PRESENTATION_TUIAPPLICATION
#define VCZH_PRESENTATION_TUIAPPLICATION

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation
{
	class ITuiApplication : public virtual Interface, public Description<ITuiApplication>
	{
	public:
		virtual void	Stop() = 0;
	};

	extern ITuiApplication*			GetTuiApplication();
	extern void						SetTuiApplication(ITuiApplication* application);
}

#endif
