/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_WINNATIVEDPIAWARENESS
#define VCZH_PRESENTATION_WINDOWS_WINNATIVEDPIAWARENESS

#include <Windows.h>
#include <ShellScalingApi.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
/***********************************************************************
DPI Awareness Functions
***********************************************************************/

			extern void				InitDpiAwareness(bool dpiAware);
			extern void				DpiAwared_GetDpiForMonitor(HMONITOR monitor, UINT* x, UINT* y);
			extern void				DpiAwared_GetDpiForWindow(HWND handle, UINT* x, UINT* y);
			extern void				DpiAwared_AdjustWindowRect(LPRECT rect, HWND handle, UINT dpi);
			extern int				DpiAwared_GetSystemMetrics(int index, UINT dpi);
		}
	}
}

#endif