/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSSCREENSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSSCREENSERVICE

#include "..\..\GuiNativeWindow.h"
#include "..\WinNativeDpiAwareness.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsScreen : public Object, public INativeScreen
			{
				friend class WindowsScreenService;
			protected:
				HMONITOR										monitor;

			public:
				WindowsScreen();

				NativeRect										GetBounds()override;
				NativeRect										GetClientBounds()override;
				WString											GetName()override;
				bool											IsPrimary()override;
				double											GetScalingX()override;
				double											GetScalingY()override;
			};

			class WindowsScreenService : public Object, public INativeScreenService
			{
				typedef HWND (*HandleRetriver)(INativeWindow*);
			protected:
				collections::List<Ptr<WindowsScreen>>			screens;
				HandleRetriver									handleRetriver;

			public:

				struct MonitorEnumProcData
				{
					WindowsScreenService*						screenService;
					vint										currentScreen;
				};

				WindowsScreenService(HandleRetriver _handleRetriver);

				static BOOL CALLBACK							MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
				void											RefreshScreenInformation();
				vint											GetScreenCount()override;
				INativeScreen*									GetScreen(vint index)override;
				INativeScreen*									GetScreen(INativeWindow* window)override;
			};
		}
	}
}

#endif