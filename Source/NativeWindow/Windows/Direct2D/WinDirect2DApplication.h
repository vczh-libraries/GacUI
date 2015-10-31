/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Direct2D Provider for Windows Implementation

Interfaces:
***********************************************************************/
#ifndef VCZH_PRESENTATION_WINDOWS_GDI_WINDIRECT2DAPPLICATION
#define VCZH_PRESENTATION_WINDOWS_GDI_WINDIRECT2DAPPLICATION

#include <d2d1_1.h>
#include <dwrite_1.h>
#include "..\WinNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			extern ID2D1RenderTarget*					GetNativeWindowDirect2DRenderTarget(INativeWindow* window);
			extern void									RecreateNativeWindowDirect2DRenderTarget(INativeWindow* window);
			extern ID2D1Factory*						GetDirect2DFactory();
			extern IDWriteFactory*						GetDirectWriteFactory();
		}
	}
}

extern int WinMainDirect2D(HINSTANCE hInstance, void(*RendererMain)());

#endif