/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCLIPBOARDSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCLIPBOARDSERVICE

#include "..\..\GuiNativeWindow.h"
#include <windows.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsClipboardService : public Object, public INativeClipboardService
			{
			protected:
				HWND					ownerHandle;
			public:
				WindowsClipboardService();

				void					SetOwnerHandle(HWND handle);
				bool					ContainsText()override;
				WString					GetText()override;
				bool					SetText(const WString& value)override;
			};
		}
	}
}

#endif