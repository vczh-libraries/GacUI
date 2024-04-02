#include "WinGDIApplication.h"
#include "..\..\Hosted\GuiHostedController.h"
#include "Renderers\GuiGraphicsWindowsGDI.h"
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
				bool							needPaintAfterResize = false;

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
							needPaintAfterResize = true;
						}
					}
					if (!buffer)
					{
						buffer = Ptr(new WinBitmap(size.x.value, size.y.value, WinBitmap::vbb32Bits, true));
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
					// for the first Paint() call after Moved()
					// trigger the global timer so that all GuiGraphicsHost with needToRender=true will call Render(false)
					// so that to fill the buffer to prevent from seeing a black frame
					// but it still leave some black color since the layout may needs more than 1 frame to finish
					if (needPaintAfterResize)
					{
						needPaintAfterResize = false;
						GetWindowsNativeController()->CallbackService()->Invoker()->InvokeGlobalTimer();
					}
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

				~GdiWindowsNativeControllerListener()
				{
					// TODO: (enumerable) foreach on dictionary
					for (vint i = 0; i < nativeWindowListeners.Count(); i++)
					{
						auto window = nativeWindowListeners.Keys()[i];
						auto listener = nativeWindowListeners.Values()[i];
						window->UninstallListener(listener.Obj());
					}
				}

				void NativeWindowCreated(INativeWindow* window)
				{
					auto listener=Ptr(new GdiWindowsNativeWindowListener(window));
					window->InstallListener(listener.Obj());
					nativeWindowListeners.Add(window, listener);
				}

				void NativeWindowDestroying(INativeWindow* window)
				{
					auto listener=nativeWindowListeners[window];
					nativeWindowListeners.Remove(window);
					window->UninstallListener(listener.Obj());
				}
			};

			GdiWindowsNativeControllerListener* gdiListener = nullptr;

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

int SetupWindowsGDIRendererInternal(bool hosted)
{
	InitDpiAwareness(false);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	WinGDIApplicationGDIObjectProvider objectProvider;
	SetWindowsGDIObjectProvider(&objectProvider);
	EnableCrossKernelCrashing();

	// create controller
	GuiHostedController* hostedController = nullptr;
	StartWindowsNativeController(hInstance);
	auto nativeController = GetWindowsNativeController();
	if (hosted)
	{
		hostedController = new GuiHostedController(nativeController);
		SetNativeController(hostedController);
		SetHostedApplication(hostedController->GetHostedApplication());
	}
	else
	{
		SetNativeController(nativeController);
	}

	{
		// install listener
		GdiWindowsNativeControllerListener listener;
		nativeController->CallbackService()->InstallListener(&listener);
		gdiListener = &listener;
		// main
		RendererMainGDI(hostedController);
		// uninstall listener
		gdiListener = nullptr;
		nativeController->CallbackService()->UninstallListener(&listener);
	}

	// destroy controller
	SetNativeController(nullptr);
	if (hostedController)
	{
		SetHostedApplication(nullptr);
		delete hostedController;
	}
	StopWindowsNativeController();
	return 0;
}

int SetupWindowsGDIRenderer()
{
	return SetupWindowsGDIRendererInternal(false);
}

int SetupHostedWindowsGDIRenderer()
{
	return SetupWindowsGDIRendererInternal(true);
}