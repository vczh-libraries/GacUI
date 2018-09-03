/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::GDI Provider for Windows Implementation

Interfaces:
***********************************************************************/
#ifndef VCZH_PRESENTATION_WINDOWS_GDI_WINGDIAPPLICATION
#define VCZH_PRESENTATION_WINDOWS_GDI_WINGDIAPPLICATION

#include "..\WinNativeWindow.h"
#include "WinGDI.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			extern WinDC*									GetNativeWindowDC(INativeWindow* window);
			extern HDC										GetNativeWindowHDC(INativeWindow* window);
		}
	}
}

extern int WinMainGDI(HINSTANCE hInstance, void(*RendererMain)());

#endif