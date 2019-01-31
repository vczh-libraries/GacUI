#include "WinNativeDpiAwareness.h"

#pragma comment(lib, "Shcore.lib")

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
#define USE_API(NAME, API) static auto proc_##API = (decltype(&API))(GetProcAddress(GetModuleHandle(L#NAME), #API))

			void InitDpiAwareness(bool dpiAware)
			{
				USE_API(user32, SetProcessDpiAwarenessContext);
				USE_API(shcore, SetProcessDpiAwareness);

				if(proc_SetProcessDpiAwarenessContext)
				{
					proc_SetProcessDpiAwarenessContext(dpiAware ? DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:  DPI_AWARENESS_CONTEXT_UNAWARE);
					return;
				}

				if(proc_SetProcessDpiAwareness)
				{
					proc_SetProcessDpiAwareness(dpiAware ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE);
					return;
				}
			}

			void DpiAwared_GetDpiForMonitor(HMONITOR monitor, UINT* x, UINT* y)
			{
				USE_API(shcore, GetDpiForMonitor);

				if (proc_GetDpiForMonitor)
				{
					if (proc_GetDpiForMonitor(monitor, MDT_DEFAULT, x, y) == S_OK)
					{
						return;
					}
				}
				*x = 96;
				*y = 96;
			}

			void DpiAwared_GetDpiForWindow(HWND handle, UINT* x, UINT* y)
			{
				USE_API(user32, GetDpiForWindow);

				if (proc_GetDpiForWindow)
				{
					*x = *y = proc_GetDpiForWindow(handle);
				}
				else
				{
					HMONITOR monitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONULL);
					if (monitor == NULL)
					{
						*x = *y = 96;
					}
					else
					{
						DpiAwared_GetDpiForMonitor(monitor, x, y);
					}
				}
			}

			void DpiAwared_AdjustWindowRect(LPRECT rect, HWND handle, UINT dpi)
			{
				USE_API(user32, AdjustWindowRectExForDpi);

				if (proc_AdjustWindowRectExForDpi)
				{
					proc_AdjustWindowRectExForDpi(rect, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtr(handle, GWL_EXSTYLE), dpi);
				}
				else
				{
					AdjustWindowRect(rect, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
				}
			}

			int DpiAwared_GetSystemMetrics(int index, UINT dpi)
			{
				USE_API(user32, GetSystemMetricsForDpi);

				if (proc_GetSystemMetricsForDpi)
				{
					return proc_GetSystemMetricsForDpi(index, dpi);
				}
				else
				{
					return GetSystemMetrics(index);
				}
			}

#undef USE_API
		}
	}
}