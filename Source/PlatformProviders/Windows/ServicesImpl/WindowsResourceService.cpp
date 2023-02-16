#include "WindowsResourceService.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{

/***********************************************************************
WindowsCursor
***********************************************************************/

			WindowsCursor::WindowsCursor(HCURSOR _handle)
				: handle(_handle)
				, isSystemCursor(false)
				, systemCursorType(INativeCursor::Arrow)
			{
			}

			WindowsCursor::WindowsCursor(SystemCursorType type)
				:handle(NULL)
				, isSystemCursor(true)
				, systemCursorType(type)
			{
				LPWSTR id = NULL;
				switch (type)
				{
				case SmallWaiting:
					id = IDC_APPSTARTING;
					break;
				case LargeWaiting:
					id = IDC_WAIT;
					break;
				case Arrow:
					id = IDC_ARROW;
					break;
				case Cross:
					id = IDC_CROSS;
					break;
				case Hand:
					id = IDC_HAND;
					break;
				case Help:
					id = IDC_HELP;
					break;
				case IBeam:
					id = IDC_IBEAM;
					break;
				case SizeAll:
					id = IDC_SIZEALL;
					break;
				case SizeNESW:
					id = IDC_SIZENESW;
					break;
				case SizeNS:
					id = IDC_SIZENS;
					break;
				case SizeNWSE:
					id = IDC_SIZENWSE;
					break;
				case SizeWE:
					id = IDC_SIZEWE;
					break;
				}
				handle = (HCURSOR)LoadImage(NULL, id, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
			}
				
			bool WindowsCursor::IsSystemCursor()
			{
				return isSystemCursor;
			}

			INativeCursor::SystemCursorType WindowsCursor::GetSystemCursorType()
			{
				return systemCursorType;
			}

			HCURSOR WindowsCursor::GetCursorHandle()
			{
				return handle;
			}

/***********************************************************************
WindowsResourceService
***********************************************************************/

			WindowsResourceService::WindowsResourceService()
			{
				{
					systemCursors.Resize(INativeCursor::SystemCursorCount);
					for (vint i = 0; i < systemCursors.Count(); i++)
					{
						systemCursors[i] = Ptr(new WindowsCursor((INativeCursor::SystemCursorType)i));
					}
				}
				{
					NONCLIENTMETRICS metrics;
					metrics.cbSize = sizeof(NONCLIENTMETRICS);
					SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
					if (!*metrics.lfMessageFont.lfFaceName)
					{
						metrics.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(metrics.iPaddedBorderWidth);
						SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
					}
					defaultFont.fontFamily = metrics.lfMessageFont.lfFaceName;
					defaultFont.size = metrics.lfMessageFont.lfHeight;
					if (defaultFont.size < 0)
					{
						defaultFont.size = -defaultFont.size;
					}
				}
			}

			INativeCursor* WindowsResourceService::GetSystemCursor(INativeCursor::SystemCursorType type)
			{
				vint index = (vint)type;
				if (0 <= index && index < systemCursors.Count())
				{
					return systemCursors[index].Obj();
				}
				else
				{
					return 0;
				}
			}

			INativeCursor* WindowsResourceService::GetDefaultSystemCursor()
			{
				return GetSystemCursor(INativeCursor::Arrow);
			}

			FontProperties WindowsResourceService::GetDefaultFont()
			{
				return defaultFont;
			}

			void WindowsResourceService::SetDefaultFont(const FontProperties& value)
			{
				defaultFont = value;
			}

			void WindowsResourceService::EnumerateFonts(collections::List<WString>& fonts)
			{
				auto proc = [](const LOGFONTW* lpelf, const TEXTMETRICW* lpntm, DWORD fontType, LPARAM lParam) -> int
				{
					auto&& fonts = *(collections::List<WString>*)lParam;
					fonts.Add(lpelf->lfFaceName);
					return 1;
				};

				HDC refHdc = GetDC(NULL);
				EnumFontFamilies(refHdc, NULL, proc, (LPARAM)&fonts);
				ReleaseDC(NULL, refHdc);
			}
		}
	}
}