/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSINPUTSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSINPUTSERVICE

#include "..\..\GuiNativeWindow.h"
#include <windows.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsInputService : public Object, public INativeInputService
			{
			protected:
				HWND									ownerHandle;
				HHOOK									mouseHook;
				bool									isTimerEnabled;
				HOOKPROC								mouseProc;

				collections::Array<WString>				keyNames;
				collections::Dictionary<WString, vint>	keys;

				WString									GetKeyNameInternal(vint code);
				void									InitializeKeyNames();
			public:
				WindowsInputService(HOOKPROC _mouseProc);

				void									SetOwnerHandle(HWND handle);
				void									StartHookMouse()override;
				void									StopHookMouse()override;
				bool									IsHookingMouse()override;
				void									StartTimer()override;
				void									StopTimer()override;
				bool									IsTimerEnabled()override;
				bool									IsKeyPressing(vint code)override;
				bool									IsKeyToggled(vint code)override;
				WString									GetKeyName(vint code)override;
				vint									GetKey(const WString& name)override;
			};

			extern bool									WinIsKeyPressing(vint code);
			extern bool									WinIsKeyToggled(vint code);
		}
	}
}

#endif