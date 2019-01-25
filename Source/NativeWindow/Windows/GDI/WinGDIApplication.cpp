#include "WinGDIApplication.h"
#include "..\..\..\GraphicsElement\WindowsGDI\GuiGraphicsWindowsGDI.h"
#include <ShellScalingApi.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace vl::collections;

			class GdiWindowsNativeWindowListener : public Object, public INativeWindowListener
			{
			protected:
				Ptr<WinBitmap>					buffer;
				INativeWindow*					window;

				vint DetermineBufferLength(vint minSize, vint minBound, vint maxBound, vint currentSize)
				{
					if(currentSize<minSize || currentSize>maxBound)
					{
						return minBound;
					}
					else
					{
						return currentSize;
					}
				}

				NativeSize CalculateBufferSize()
				{
					NativeSize nativeWindowSize = window->GetClientSize();
					Size windowSize(nativeWindowSize.x.value, nativeWindowSize.y.value);
					Size minBounds(windowSize.x * 5 / 4, windowSize.y * 5 / 4);
					Size maxBounds(windowSize.x * 3 / 2, windowSize.y * 3 / 2);
					Size currentSize = buffer ? Size(buffer->GetWidth(), buffer->GetHeight()) : Size(0, 0);
					vint newWidth = DetermineBufferLength(windowSize.x, minBounds.x, maxBounds.x, currentSize.x);
					vint newHeight = DetermineBufferLength(windowSize.y, minBounds.y, maxBounds.y, currentSize.y);
					return NativeSize(newWidth, newHeight);
				}

				void RebuildCanvas(NativeSize size)
				{
					if (size.x < 256)size.x = 256;
					if (size.y < 256)size.y = 256;
					if (buffer)
					{
						if (buffer->GetWidth() != size.x.value || buffer->GetHeight() != size.y.value)
						{
							buffer = 0;
						}
					}
					if (!buffer)
					{
						buffer = new WinBitmap(size.x.value, size.y.value, WinBitmap::vbb32Bits, true);
						buffer->GetWinDC()->SetBackTransparent(true);
					}
				}
			public:
				GdiWindowsNativeWindowListener(INativeWindow* _window)
					:window(_window)
				{
				}

				void Moved()
				{
					RebuildCanvas(CalculateBufferSize());
				}

				void Paint()
				{
					IWindowsForm* form=GetWindowsForm(window);
					WinControlDC controlDC(form->GetWindowHandle());
					controlDC.Draw(0, 0, buffer);
				}

				WinDC* GetWinDC()
				{
					if(!buffer) Moved();
					return buffer->GetWinDC();
				}
			};

			class GdiWindowsNativeControllerListener : public Object, public INativeControllerListener
			{
			public:
				Dictionary<INativeWindow*, Ptr<GdiWindowsNativeWindowListener>>		nativeWindowListeners;

				void NativeWindowCreated(INativeWindow* window)
				{
					Ptr<GdiWindowsNativeWindowListener> listener=new GdiWindowsNativeWindowListener(window);
					window->InstallListener(listener.Obj());
					nativeWindowListeners.Add(window, listener);
				}

				void NativeWindowDestroying(INativeWindow* window)
				{
					Ptr<GdiWindowsNativeWindowListener> listener=nativeWindowListeners[window];
					nativeWindowListeners.Remove(window);
					window->UninstallListener(listener.Obj());
				}
			};

			GdiWindowsNativeControllerListener* gdiListener=0;

			WinDC* GetNativeWindowDC(INativeWindow* window)
			{
				vint index=gdiListener->nativeWindowListeners.Keys().IndexOf(window);
				return index==-1?0:gdiListener->nativeWindowListeners.Values().Get(index)->GetWinDC();
			}

			HDC GetNativeWindowHDC(INativeWindow* window)
			{
				WinDC* dc=GetNativeWindowDC(window);
				return dc?dc->GetHandle():NULL;
			}
		}

		namespace elements_windows_gdi
		{
/***********************************************************************
OS Supporting
***********************************************************************/

			class WinGDIApplicationGDIObjectProvider : public IWindowsGDIObjectProvider
			{
			protected:
				IMLangFontLink2*				mLangFontLink;

			public:
				WinGDIApplicationGDIObjectProvider()
					:mLangFontLink(0)
				{
					CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMLangFontLink2, (void**)&mLangFontLink);
				}

				~WinGDIApplicationGDIObjectProvider()
				{
					mLangFontLink->Release();
				}

				windows::WinDC* GetNativeWindowDC(INativeWindow* window)override
				{
					return vl::presentation::windows::GetNativeWindowDC(window);
				}

				IWindowsGDIRenderTarget* GetBindedRenderTarget(INativeWindow* window)override
				{
					return dynamic_cast<IWindowsGDIRenderTarget*>(vl::presentation::windows::GetWindowsForm(window)->GetGraphicsHandler());
				}

				void SetBindedRenderTarget(INativeWindow* window, IWindowsGDIRenderTarget* renderTarget)override
				{
					vl::presentation::windows::GetWindowsForm(window)->SetGraphicsHandler(renderTarget);
				}

				IWICImagingFactory* GetWICImagingFactory()override
				{
					return vl::presentation::windows::GetWICImagingFactory();
				}

				IWICBitmap* GetWICBitmap(INativeImageFrame* frame)override
				{
					return vl::presentation::windows::GetWICBitmap(frame);
				}

				IMLangFontLink2* GetMLangFontLink()override
				{
					return mLangFontLink;
				}
			};
		}
	}
}

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::windows;
using namespace vl::presentation::elements_windows_gdi;

int WinMainGDI(HINSTANCE hInstance, void(*RendererMain)())
{
	EnableCrossKernelCrashing();
	// create controller
	INativeController* controller=CreateWindowsNativeController(hInstance);
	SetCurrentController(controller);
	{
		// install listener
		GdiWindowsNativeControllerListener listener;
		controller->CallbackService()->InstallListener(&listener);
		gdiListener=&listener;
		// main
		RendererMain();
		// uninstall listener
		gdiListener=0;
		controller->CallbackService()->UninstallListener(&listener);
	}
	// destroy controller
	DestroyWindowsNativeController(controller);
	return 0;
}

int SetupWindowsGDIRenderer()
{
	InitDpiAwareness(false);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	HINSTANCE hInstance=(HINSTANCE)GetModuleHandle(NULL);
	WinGDIApplicationGDIObjectProvider objectProvider;
	SetWindowsGDIObjectProvider(&objectProvider);
	return WinMainGDI(hInstance, &RendererMainGDI);
}