#include "WindowsInputService.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			bool WinIsKeyPressing(VKEY code)
			{
				return (GetKeyState((int)code)&0xF0)!=0;
			}

			bool WinIsKeyToggled(VKEY code)
			{
				return (GetKeyState((int)code)&0x0F)!=0;
			}

/***********************************************************************
WindowsInputService
***********************************************************************/

			WString WindowsInputService::GetKeyNameInternal(VKEY code)
			{
				if ((vint)code < 8) return L"?";
				wchar_t name[256]={0};
				vint scanCode=MapVirtualKey((int)code, MAPVK_VK_TO_VSC)<<16;
				switch((vint)code)
				{
				case VK_INSERT:
				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
					scanCode|=1<<24;
					break;
				case VK_CLEAR:
				case VK_LSHIFT:
				case VK_RSHIFT: 
				case VK_LCONTROL:
				case VK_RCONTROL:
				case VK_LMENU:
				case VK_RMENU:
					return L"?";
				}
				GetKeyNameText((int)scanCode, name, sizeof(name)/sizeof(*name));
				return name[0]?name:L"?";
			}

			void WindowsInputService::InitializeKeyNames()
			{
				for (vint i = 0; i < keyNames.Count(); i++)
				{
					keyNames[i] = GetKeyNameInternal((VKEY)i);
					if (keyNames[i] != L"?")
					{
						keys.Set(keyNames[i], (VKEY)i);
					}
				}
			}

			WindowsInputService::WindowsInputService(HOOKPROC _mouseProc)
				:ownerHandle(NULL)
				,mouseHook(NULL)
				,isTimerEnabled(false)
				,mouseProc(_mouseProc)
				,keyNames(146)
			{
				InitializeKeyNames();
			}

			void WindowsInputService::SetOwnerHandle(HWND handle)
			{
				ownerHandle=handle;
			}

			void WindowsInputService::StartHookMouse()
			{
				if(!IsHookingMouse())
				{
					mouseHook=SetWindowsHookEx(WH_MOUSE_LL, mouseProc, NULL, NULL);
				}
			}

			void WindowsInputService::StopHookMouse()
			{
				if(IsHookingMouse())
				{
					UnhookWindowsHookEx(mouseHook);
					mouseHook=NULL;
				}
			}

			bool WindowsInputService::IsHookingMouse()
			{
				return mouseHook!=NULL;
			}

			void WindowsInputService::StartTimer()
			{
				if(!IsTimerEnabled())
				{
					SetTimer(ownerHandle, 1, 16, NULL);
					isTimerEnabled=true;
				}
			}

			void WindowsInputService::StopTimer()
			{
				if(IsTimerEnabled())
				{
					KillTimer(ownerHandle, 1);
					isTimerEnabled=false;
				}
			}

			bool WindowsInputService::IsTimerEnabled()
			{
				return isTimerEnabled;
			}
				
			bool WindowsInputService::IsKeyPressing(VKEY code)
			{
				return WinIsKeyPressing(code);
			}

			bool WindowsInputService::IsKeyToggled(VKEY code)
			{
				return WinIsKeyToggled(code);
			}

			WString WindowsInputService::GetKeyName(VKEY code)
			{
				if (0 <= (vint)code && (vint)code < keyNames.Count())
				{
					return keyNames[(vint)code];
				}
				else
				{
					return L"?";
				}
			}

			VKEY WindowsInputService::GetKey(const WString& name)
			{
				vint index = keys.Keys().IndexOf(name);
				return index == -1 ? VKEY::_UNKNOWN : keys.Values()[index];
			}
		}
	}
}