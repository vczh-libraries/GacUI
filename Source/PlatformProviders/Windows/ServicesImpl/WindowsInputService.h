/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSINPUTSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSINPUTSERVICE

#include "..\..\..\NativeWindow\GuiNativeWindow.h"
#include "..\WinNativeDpiAwareness.h"

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
				bool									isTimerEnabled;
				vint									usedHotKeys = (vint)NativeGlobalShortcutKeyResult::ValidIdBegins;

				collections::Array<WString>				keyNames;
				collections::Dictionary<WString, VKEY>	keys;

				WString									GetKeyNameInternal(VKEY code);
				void									InitializeKeyNames();
			public:
				WindowsInputService();

				void									SetOwnerHandle(HWND handle);
				void									StartTimer()override;
				void									StopTimer()override;
				bool									IsTimerEnabled()override;
				bool									IsKeyPressing(VKEY code)override;
				bool									IsKeyToggled(VKEY code)override;
				WString									GetKeyName(VKEY code)override;
				VKEY									GetKey(const WString& name)override;
				vint									RegisterGlobalShortcutKey(bool ctrl, bool shift, bool alt, VKEY key)override;
				bool									UnregisterGlobalShortcutKey(vint id)override;
			};

			extern bool									WinIsKeyPressing(VKEY code);
			extern bool									WinIsKeyToggled(VKEY code);
		}
	}
}

#endif