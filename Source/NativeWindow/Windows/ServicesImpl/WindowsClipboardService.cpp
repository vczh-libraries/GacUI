#include "WindowsClipboardService.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{

/***********************************************************************
WindowsClipboardService
***********************************************************************/

			WindowsClipboardService::WindowsClipboardService()
				:ownerHandle(NULL)
			{
			}

			void WindowsClipboardService::SetOwnerHandle(HWND handle)
			{
				HWND oldHandle=ownerHandle;
				ownerHandle=handle;
				if(handle==NULL)
				{
					RemoveClipboardFormatListener(oldHandle);
				}
				else
				{
					AddClipboardFormatListener(ownerHandle);
				}
			}

			bool WindowsClipboardService::ContainsText()
			{
				if(OpenClipboard(ownerHandle))
				{
					UINT format=0;
					bool contains=false;
					while(format=EnumClipboardFormats(format))
					{
						if(format==CF_TEXT || format==CF_UNICODETEXT)
						{
							contains=true;
							break;
						}
					}
					CloseClipboard();
					return contains;
				}
				return false;
			}

			WString WindowsClipboardService::GetText()
			{
				if(OpenClipboard(ownerHandle))
				{
					WString result;
					HANDLE handle=GetClipboardData(CF_UNICODETEXT);
					if(handle!=0)
					{
						wchar_t* buffer=(wchar_t*)GlobalLock(handle);
						result=buffer;
						GlobalUnlock(handle);
					}
					CloseClipboard();
					return result;
				}
				return L"";
			}

			bool WindowsClipboardService::SetText(const WString& value)
			{
				if(OpenClipboard(ownerHandle))
				{
					EmptyClipboard();
					vint size=(value.Length()+1)*sizeof(wchar_t);
					HGLOBAL data=GlobalAlloc(GMEM_MOVEABLE, size);
					wchar_t* buffer=(wchar_t*)GlobalLock(data);
					memcpy(buffer, value.Buffer(), size);
					GlobalUnlock(data);
					SetClipboardData(CF_UNICODETEXT, data);
					CloseClipboard();
					return true;
				}
				return false;
			}
		}
	}
}