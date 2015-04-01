#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include "WinDirect2DApplication.h"
#include "..\..\..\GraphicsElement\WindowsDirect2D\GuiGraphicsWindowsDirect2D.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace vl::collections;

			class Direct2DWindowsNativeWindowListener : public Object, public INativeWindowListener
			{
			protected:
				ID2D1Factory*					d2dFactory;
				ComPtr<ID2D1HwndRenderTarget>	d2dRenderTarget;
				INativeWindow*					window;
				Size							previousSize;

				void RebuildCanvas(Size size)
				{
					if (size.x <= 1) size.x = 1;
					if (size.y <= 1) size.y = 1;
					if(!d2dRenderTarget)
					{
						ID2D1HwndRenderTarget* renderTarget=0;
						IWindowsForm* form=GetWindowsForm(window);
						D2D1_RENDER_TARGET_PROPERTIES tp=D2D1::RenderTargetProperties();
						tp.dpiX=96;
						tp.dpiY=96;
						HRESULT hr=d2dFactory->CreateHwndRenderTarget(
							tp,
							D2D1::HwndRenderTargetProperties(
								form->GetWindowHandle(),
								D2D1::SizeU((int)size.x, (int)size.y)
								),
							&renderTarget
							);
						if(!FAILED(hr))
						{
							d2dRenderTarget=renderTarget;
							d2dRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
						}
					}
					else if(previousSize!=size)
					{
						d2dRenderTarget->Resize(D2D1::SizeU((int)size.x, (int)size.y));
					}
					previousSize=size;
				}
			public:
				Direct2DWindowsNativeWindowListener(INativeWindow* _window, ID2D1Factory* _d2dFactory)
					:window(_window)
					,d2dFactory(_d2dFactory)
					,d2dRenderTarget(0)
				{
				}

				void Moved()
				{
					RebuildCanvas(window->GetClientSize());
				}

				void Paint()
				{
				}

				ID2D1RenderTarget* GetDirect2DRenderTarget()
				{
					if(!d2dRenderTarget) Moved();
					return d2dRenderTarget.Obj();
				}

				void RecreateRenderTarget()
				{
					if (d2dRenderTarget)
					{
						d2dRenderTarget = 0;
					}
				}
			};

			class Direct2DWindowsNativeControllerListener : public Object, public INativeControllerListener
			{
			public:
				Dictionary<INativeWindow*, Ptr<Direct2DWindowsNativeWindowListener>>		nativeWindowListeners;
				ComPtr<ID2D1Factory>														d2dFactory;
				ComPtr<IDWriteFactory>														dwrite;

				Direct2DWindowsNativeControllerListener()
				{
					{
						ID2D1Factory* factory=0;
						HRESULT hr=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
						if(!FAILED(hr))
						{
							d2dFactory=factory;
						}
					}
					{
						IDWriteFactory* factory=0;
						HRESULT hr=DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory));
						if(!FAILED(hr))
						{
							dwrite=factory;
						}
					}
				}

				void NativeWindowCreated(INativeWindow* window)
				{
					Ptr<Direct2DWindowsNativeWindowListener> listener=new Direct2DWindowsNativeWindowListener(window, d2dFactory.Obj());
					window->InstallListener(listener.Obj());
					nativeWindowListeners.Add(window, listener);
				}

				void NativeWindowDestroying(INativeWindow* window)
				{
					Ptr<Direct2DWindowsNativeWindowListener> listener=nativeWindowListeners[window];
					nativeWindowListeners.Remove(window);
					window->UninstallListener(listener.Obj());
				}
			};

			Direct2DWindowsNativeControllerListener* direct2DListener=0;

			ID2D1RenderTarget* GetNativeWindowDirect2DRenderTarget(INativeWindow* window)
			{
				vint index=direct2DListener->nativeWindowListeners.Keys().IndexOf(window);
				return index==-1?0:direct2DListener->nativeWindowListeners.Values().Get(index)->GetDirect2DRenderTarget();
			}

			void RecreateNativeWindowDirect2DRenderTarget(INativeWindow* window)
			{
				vint index=direct2DListener->nativeWindowListeners.Keys().IndexOf(window);
				if (index != -1)
				{
					direct2DListener->nativeWindowListeners.Values().Get(index)->RecreateRenderTarget();
				}
			}

			ID2D1Factory* GetDirect2DFactory()
			{
				return direct2DListener->d2dFactory.Obj();
			}

			IDWriteFactory* GetDirectWriteFactory()
			{
				return direct2DListener->dwrite.Obj();
			}
		}

		namespace elements_windows_d2d
		{
/***********************************************************************
OS Supporting
***********************************************************************/

			class WinDirect2DApplicationDirect2DObjectProvider : public IWindowsDirect2DObjectProvider
			{
			public:
				void RecreateRenderTarget(INativeWindow* window)
				{
					vl::presentation::windows::RecreateNativeWindowDirect2DRenderTarget(window);
				}

				ID2D1RenderTarget* GetNativeWindowDirect2DRenderTarget(INativeWindow* window)
				{
					return vl::presentation::windows::GetNativeWindowDirect2DRenderTarget(window);
				}

				ID2D1Factory* GetDirect2DFactory()
				{
					return vl::presentation::windows::GetDirect2DFactory();
				}

				IDWriteFactory* GetDirectWriteFactory()
				{
					return vl::presentation::windows::GetDirectWriteFactory();
				}

				IWindowsDirect2DRenderTarget* GetBindedRenderTarget(INativeWindow* window)
				{
					return dynamic_cast<IWindowsDirect2DRenderTarget*>(vl::presentation::windows::GetWindowsForm(window)->GetGraphicsHandler());
				}

				void SetBindedRenderTarget(INativeWindow* window, IWindowsDirect2DRenderTarget* renderTarget)
				{
					vl::presentation::windows::GetWindowsForm(window)->SetGraphicsHandler(renderTarget);
				}

				IWICImagingFactory* GetWICImagingFactory()
				{
					return vl::presentation::windows::GetWICImagingFactory();
				}

				IWICBitmap* GetWICBitmap(INativeImageFrame* frame)
				{
					return vl::presentation::windows::GetWICBitmap(frame);
				}
			};
		}
	}
}

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::windows;
using namespace vl::presentation::elements_windows_d2d;

int WinMainDirect2D(HINSTANCE hInstance, void(*RendererMain)())
{
	EnableCrossKernelCrashing();
	// create controller
	INativeController* controller=CreateWindowsNativeController(hInstance);
	SetCurrentController(controller);
	{
		// install listener
		Direct2DWindowsNativeControllerListener listener;
		controller->CallbackService()->InstallListener(&listener);
		direct2DListener=&listener;
		// main
		RendererMain();
		// uninstall listener
		direct2DListener=0;
		controller->CallbackService()->UninstallListener(&listener);
	}
	// destroy controller
	DestroyWindowsNativeController(controller);
	return 0;
}

int SetupWindowsDirect2DRenderer()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	HINSTANCE hInstance=(HINSTANCE)GetModuleHandle(NULL);
	WinDirect2DApplicationDirect2DObjectProvider objectProvider;
	SetWindowsDirect2DObjectProvider(&objectProvider);
	return WinMainDirect2D(hInstance, &RendererMainDirect2D);
}