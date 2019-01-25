/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCALLBACKSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCALLBACKSERVICE

#include "..\..\GuiNativeWindow.h"
#include "..\WinNativeDpiAwareness.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsCallbackService : public Object, public INativeCallbackService
			{
			protected:
				collections::List<INativeControllerListener*>	listeners;

			public:
				WindowsCallbackService();

				bool											InstallListener(INativeControllerListener* listener)override;
				bool											UninstallListener(INativeControllerListener* listener)override;

				void											InvokeMouseHook(WPARAM message, NativePoint location);
				void											InvokeGlobalTimer();
				void											InvokeClipboardUpdated();
				void											InvokeNativeWindowCreated(INativeWindow* window);
				void											InvokeNativeWindowDestroyed(INativeWindow* window);
			};
		}
	}
}

#endif