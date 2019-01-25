#include "WindowsCallbackService.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{

/***********************************************************************
WindowsCallbackService
***********************************************************************/

			WindowsCallbackService::WindowsCallbackService()
			{
			}

			bool WindowsCallbackService::InstallListener(INativeControllerListener* listener)
			{
				if(listeners.Contains(listener))
				{
					return false;
				}
				else
				{
					listeners.Add(listener);
					return true;
				}
			}

			bool WindowsCallbackService::UninstallListener(INativeControllerListener* listener)
			{
				if(listeners.Contains(listener))
				{
					listeners.Remove(listener);
					return true;
				}
				else
				{
					return false;
				}
			}

			void WindowsCallbackService::InvokeMouseHook(WPARAM message, NativePoint location)
			{
				switch(message)
				{
				case WM_LBUTTONDOWN:
					{
						for(vint i=0;i<listeners.Count();i++)
						{
							listeners[i]->LeftButtonDown(location);
						}
					}
					break;
				case WM_LBUTTONUP:
					{
						for(vint i=0;i<listeners.Count();i++)
						{
							listeners[i]->LeftButtonUp(location);
						}
					}
					break;
				case WM_RBUTTONDOWN:
					{
						for(vint i=0;i<listeners.Count();i++)
						{
							listeners[i]->RightButtonDown(location);
						}
					}
					break;
				case WM_RBUTTONUP:
					{
						for(vint i=0;i<listeners.Count();i++)
						{
							listeners[i]->RightButtonUp(location);
						}
					}
					break;
				case WM_MOUSEMOVE:
					{
						for(vint i=0;i<listeners.Count();i++)
						{
							listeners[i]->MouseMoving(location);
						}
					}
					break;
				}
			}

			void WindowsCallbackService::InvokeGlobalTimer()
			{
				for(vint i=0;i<listeners.Count();i++)
				{
					listeners[i]->GlobalTimer();
				}
			}

			void WindowsCallbackService::InvokeClipboardUpdated()
			{
				for(vint i=0;i<listeners.Count();i++)
				{
					listeners[i]->ClipboardUpdated();
				}
			}

			void WindowsCallbackService::InvokeNativeWindowCreated(INativeWindow* window)
			{
				for(vint i=0;i<listeners.Count();i++)
				{
					listeners[i]->NativeWindowCreated(window);
				}
			}

			void WindowsCallbackService::InvokeNativeWindowDestroyed(INativeWindow* window)
			{
				for(vint i=0;i<listeners.Count();i++)
				{
					listeners[i]->NativeWindowDestroying(window);
				}
			}
		}
	}
}