#include "WinNativeDpiAwareness.h"

#pragma comment(lib, "Shcore.lib")

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			void InitDpiAwareness(bool dpiAware)
			{
				{
					HMODULE moduleHandle = LoadLibrary(L"user32");
					bool available = GetProcAddress(moduleHandle, "SetProcessDpiAwarenessContext") != NULL;
					FreeLibrary(moduleHandle);
					if (available)
					{
						SetProcessDpiAwarenessContext(dpiAware ? DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:  DPI_AWARENESS_CONTEXT_UNAWARE);
						return;
					}
				}
				{
					HMODULE moduleHandle = LoadLibrary(L"Shcore");
					bool available = GetProcAddress(moduleHandle, "SetProcessDpiAwareness") != NULL;
					FreeLibrary(moduleHandle);
					if (available)
					{
						SetProcessDpiAwareness(dpiAware ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE);
						return;
					}
				}
			}

			void DpiAwared_GetDpiForMonitor(HMONITOR monitor, UINT* x, UINT* y)
			{
				static bool initialized = false;
				static bool available_GetDpiForMonitor = false;
				if (!initialized)
				{
					initialized = true;
					HMODULE moduleHandle = LoadLibrary(L"Shcore");
					available_GetDpiForMonitor = GetProcAddress(moduleHandle, "GetDpiForMonitor") != NULL;
					FreeLibrary(moduleHandle);
				}

				if (!available_GetDpiForMonitor)
				{
					*x = 96;
					*y = 96;
				}

				if (GetDpiForMonitor(monitor, MDT_DEFAULT, x, y) != S_OK)
				{
					*x = 96;
					*y = 96;
				}
			}

			void DpiAwared_AdjustWindowRect(LPRECT rect, HWND handle)
			{
				static bool initialized = false;
				static bool available_GetDpiForWindow_AdjustWindowRectExForDpi = false;
				if (!initialized)
				{
					initialized = true;
					HMODULE moduleHandle = LoadLibrary(L"user32");
					available_GetDpiForWindow_AdjustWindowRectExForDpi = GetProcAddress(moduleHandle, "GetDpiForWindow") != NULL && GetProcAddress(moduleHandle, "AdjustWindowRectExForDpi") != NULL;
					FreeLibrary(moduleHandle);
				}

				if (available_GetDpiForWindow_AdjustWindowRectExForDpi)
				{
					AdjustWindowRectExForDpi(rect, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtr(handle, GWL_EXSTYLE), GetDpiForWindow(handle));
				}
				else
				{
					AdjustWindowRect(rect, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
				}
			}
		}
	}
}