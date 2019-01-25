#include "WinDirect2DApplication.h"
#include "..\..\..\GraphicsElement\WindowsDirect2D\GuiGraphicsWindowsDirect2D.h"
#include <ShellScalingApi.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")

namespace vl
{
	namespace presentation
	{
		using namespace elements;

		namespace windows
		{
			using namespace vl::collections;

/***********************************************************************
WindowListener
***********************************************************************/

			class Direct2DWindowsNativeWindowListener : public Object, public INativeWindowListener
			{
			protected:
				ID2D1Factory*					d2dFactory;
				INativeWindow*					window;
				bool							rendering = false;
				bool							movedWhileRendering = false;

				virtual void					RebuildCanvas(NativeSize size) = 0;
			public:
				Direct2DWindowsNativeWindowListener(INativeWindow* _window, ID2D1Factory* _d2dFactory)
					:window(_window)
					,d2dFactory(_d2dFactory)
				{
				}

				void Moved()override
				{
					if (rendering)
					{
						movedWhileRendering = true;
					}
					else
					{
						ResizeRenderTarget();
					}
				}

				void ResizeRenderTarget()
				{
					RebuildCanvas(window->GetClientSize());
				}

				void StartRendering()
				{
					rendering = true;
				}

				void StopRendering()
				{
					rendering = false;
				}

				bool RetrieveAndResetMovedWhileRendering()
				{
					bool result = movedWhileRendering;
					movedWhileRendering = false;
					return result;
				}

				virtual ID2D1RenderTarget*		GetDirect2DRenderTarget() = 0;
				virtual void					RecreateRenderTarget() = 0;
				virtual bool					PresentRenderTarget() = 0;
			};

/***********************************************************************
WindowListener 1.0
***********************************************************************/

			class Direct2DWindowsNativeWindowListener_1_0 : public Direct2DWindowsNativeWindowListener
			{
			protected:
				ComPtr<ID2D1HwndRenderTarget>	d2dRenderTarget;
				NativeSize						previousSize;

				void RebuildCanvas(NativeSize size)override
				{
					if (size.x <= 1) size.x = 1;
					if (size.y <= 1) size.y = 1;
					if (!d2dRenderTarget)
					{
						ID2D1HwndRenderTarget* renderTarget = 0;
						IWindowsForm* form = GetWindowsForm(window);
						D2D1_RENDER_TARGET_PROPERTIES tp = D2D1::RenderTargetProperties();
						{
							UINT dpiX = 0;
							UINT dpiY = 0;
							DpiAwared_GetDpiForWindow(form->GetWindowHandle(), &dpiX, &dpiY);
							tp.dpiX = (FLOAT)dpiX;
							tp.dpiY = (FLOAT)dpiY;
						}
						HRESULT hr = d2dFactory->CreateHwndRenderTarget(
							tp,
							D2D1::HwndRenderTargetProperties(
								form->GetWindowHandle(),
								D2D1::SizeU((int)size.x.value, (int)size.y.value)
							),
							&renderTarget
						);
						if (!FAILED(hr))
						{
							d2dRenderTarget = renderTarget;
							d2dRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
						}
					}
					else if (previousSize != size)
					{
						d2dRenderTarget->Resize(D2D1::SizeU((int)size.x.value, (int)size.y.value));
					}
					previousSize = size;
				}
			public:
				Direct2DWindowsNativeWindowListener_1_0(INativeWindow* _window, ID2D1Factory* _d2dFactory)
					:Direct2DWindowsNativeWindowListener(_window, _d2dFactory)
				{
				}

				ID2D1RenderTarget* GetDirect2DRenderTarget()override
				{
					if(!d2dRenderTarget) Moved();
					return d2dRenderTarget.Obj();
				}

				void RecreateRenderTarget()override
				{
					if (d2dRenderTarget)
					{
						d2dRenderTarget = 0;
					}
				}

				bool PresentRenderTarget()override
				{
					return true;
				}
			};

/***********************************************************************
WindowListener 1.1
***********************************************************************/

			class Direct2DWindowsNativeWindowListener_1_1 : public Direct2DWindowsNativeWindowListener
			{
			protected:
				ComPtr<ID2D1Factory1>			d2dFactory1;
				ID3D11Device*					d3d11Device;
				ComPtr<IDXGIDevice>				dxgiDevice;
				ComPtr<IDXGISwapChain1>			dxgiSwapChain;
				ComPtr<ID2D1DeviceContext>		d2dDeviceContext;
				NativeSize						previousSize;

				ComPtr<IDXGIDevice> GetDXGIDevice()
				{
					IDXGIDevice* device = nullptr;
					HRESULT hr = d3d11Device->QueryInterface(&device);
					if (!SUCCEEDED(hr)) return 0;
					return device;
				}

				ComPtr<IDXGISwapChain1> CreateSwapChain(IDXGIDevice* dxgiDevice)
				{
					ComPtr<IDXGIAdapter> dxgiAdapter;
					{
						IDXGIAdapter* adapter = nullptr;
						HRESULT hr = dxgiDevice->GetAdapter(&adapter);
						if (!SUCCEEDED(hr)) return 0;
						dxgiAdapter = adapter;
					}

					ComPtr<IDXGIFactory2> dxgiFactory;
					{
						IDXGIFactory2* factory = nullptr;
						HRESULT hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);
						if (!SUCCEEDED(hr)) return 0;
						dxgiFactory = factory;
					}

					IWindowsForm* form = GetWindowsForm(window);
					ComPtr<IDXGISwapChain1> dxgiSwapChain;
					{
						DXGI_SWAP_CHAIN_DESC1 props = {};
						props.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
						props.SampleDesc.Count = 1;
						props.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
						props.BufferCount = 2;

						IDXGISwapChain1* swapChain = nullptr;
						HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, form->GetWindowHandle(), &props, nullptr, nullptr, &swapChain);
						if (!SUCCEEDED(hr)) return 0;
						dxgiSwapChain = swapChain;
					}

					return dxgiSwapChain;
				}

				ComPtr<ID2D1DeviceContext> CreateDeviceContext(IDXGIDevice* dxgiDevice)
				{
					ComPtr<ID2D1Device> d2d1Device;
					{
						ID2D1Device* device = nullptr;
						HRESULT hr = d2dFactory1->CreateDevice(dxgiDevice, &device);
						if (!SUCCEEDED(hr)) return 0;
						d2d1Device = device;
					}

					ComPtr<ID2D1DeviceContext> d2dDeviceContext;
					{
						ID2D1DeviceContext* deviceContext = nullptr;
						HRESULT hr = d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &deviceContext);
						if (!SUCCEEDED(hr)) return 0;
						d2dDeviceContext = deviceContext;
					}

					return d2dDeviceContext;
				}

				ComPtr<ID2D1Bitmap1> CreateBitmap(IDXGISwapChain1* swapChain, ID2D1DeviceContext* deviceContext)
				{
					ComPtr<IDXGISurface> dxgiSurface;
					{
						IDXGISurface* surface = nullptr;
						HRESULT hr = swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&surface);
						if (!SUCCEEDED(hr))return 0;
						dxgiSurface = surface;
					}

					ComPtr<ID2D1Bitmap1> d2dBitmap;
					{
						auto props = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));

						ID2D1Bitmap1* bitmap = nullptr;
						HRESULT hr = deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Obj(), props, &bitmap);
						if (!SUCCEEDED(hr)) return 0;
						d2dBitmap = bitmap;
					}

					return d2dBitmap;
				}

				void RebuildCanvas(NativeSize size)override
				{
					if (size.x <= 1) size.x = 1;
					if (size.y <= 1) size.y = 1;

					if(!d2dDeviceContext)
					{
						if (!dxgiDevice)
						{
							dxgiDevice = GetDXGIDevice();
						}

						if (!dxgiSwapChain)
						{
							dxgiSwapChain = CreateSwapChain(dxgiDevice.Obj());
						}

						d2dDeviceContext = CreateDeviceContext(dxgiDevice.Obj());
						auto d2dBitmap = CreateBitmap(dxgiSwapChain.Obj(), d2dDeviceContext.Obj());
						d2dDeviceContext->SetTarget(d2dBitmap.Obj());
						IWindowsForm* form = GetWindowsForm(window);
						{
							UINT dpiX = 0;
							UINT dpiY = 0;
							DpiAwared_GetDpiForWindow(form->GetWindowHandle(), &dpiX, &dpiY);
							d2dDeviceContext->SetDpi((FLOAT)dpiX, (FLOAT)dpiY);
						}
					}
					else if(previousSize!=size)
					{
						d2dDeviceContext->SetTarget(nullptr);
						HRESULT hr = dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
						if (SUCCEEDED(hr))
						{
							auto d2dBitmap = CreateBitmap(dxgiSwapChain.Obj(), d2dDeviceContext.Obj());
							d2dDeviceContext->SetTarget(d2dBitmap.Obj());
						}
					}
					previousSize=size;
				}
			public:
				Direct2DWindowsNativeWindowListener_1_1(INativeWindow* _window, ComPtr<ID2D1Factory1> _d2dFactory1, ID3D11Device* _d3d11Device)
					:Direct2DWindowsNativeWindowListener(_window, _d2dFactory1.Obj())
					,d2dFactory1(_d2dFactory1)
					,d3d11Device(_d3d11Device)
				{
				}

				ID2D1RenderTarget* GetDirect2DRenderTarget()override
				{
					if(!d2dDeviceContext) Moved();
					return d2dDeviceContext.Obj();
				}

				void RecreateRenderTarget()override
				{
					if (d2dDeviceContext)
					{
						d2dDeviceContext = 0;
						dxgiSwapChain = 0;
					}
				}

				bool PresentRenderTarget()override
				{
					if (d2dDeviceContext)
					{
						if (dxgiSwapChain)
						{
							DXGI_PRESENT_PARAMETERS parameters = {0};
							HRESULT hr = dxgiSwapChain->Present1(1, 0, &parameters);
							return hr == S_OK || hr == DXGI_STATUS_OCCLUDED;
						}
					}
					return false;
				}
			};

/***********************************************************************
ControllerListener
***********************************************************************/

			class Direct2DWindowsNativeControllerListener : public Object, public INativeControllerListener
			{
			public:
				Dictionary<INativeWindow*, Ptr<Direct2DWindowsNativeWindowListener>>		nativeWindowListeners;
				ComPtr<ID2D1Factory>														d2dFactory;
				ComPtr<IDWriteFactory>														dwrite;
				ComPtr<ID3D11Device>														d3d11Device;

				Direct2DWindowsNativeControllerListener()
				{
					{
						D2D1_FACTORY_OPTIONS fo = {};
						#ifdef _DEBUG
						fo.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
						#endif

						ID2D1Factory* factory=0;
						HRESULT hr=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, fo, &factory);
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

				ComPtr<ID3D11Device> CreateD3D11Device(D3D_DRIVER_TYPE driverType)
				{
					UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
					#ifdef _DEBUG
					flags |= D3D11_CREATE_DEVICE_DEBUG;
					#endif

					D3D_FEATURE_LEVEL featureLevels[] =
					{
						D3D_FEATURE_LEVEL_11_1,
						D3D_FEATURE_LEVEL_11_0,
						D3D_FEATURE_LEVEL_10_1,
						D3D_FEATURE_LEVEL_10_0,
						D3D_FEATURE_LEVEL_9_3,
						D3D_FEATURE_LEVEL_9_2,
						D3D_FEATURE_LEVEL_9_1
					};


					ID3D11Device* device = nullptr;
					HRESULT hr = D3D11CreateDevice(
						nullptr,
						driverType,
						nullptr,
						flags,
						featureLevels,
						sizeof(featureLevels) / sizeof(*featureLevels),
						D3D11_SDK_VERSION,
						&device,
						nullptr,
						nullptr);
					if (SUCCEEDED(hr))
					{
						return device;
					}
					else if (device)
					{
						device->Release();
					}
					return 0;
				}

				void NativeWindowCreated(INativeWindow* window)
				{
					ComPtr<ID2D1Factory1> d2dfactory1;
					{
						ID2D1Factory1* factory = nullptr;
						HRESULT hr = windows::GetDirect2DFactory()->QueryInterface(&factory);
						if (SUCCEEDED(hr))
						{
							d2dfactory1 = factory;
						}
					}

					Ptr<Direct2DWindowsNativeWindowListener> listener;
					if (d2dfactory1)
					{
						if (!d3d11Device)
						{
							d3d11Device = CreateD3D11Device(D3D_DRIVER_TYPE_HARDWARE);
							if (!d3d11Device)
							{
								d3d11Device = CreateD3D11Device(D3D_DRIVER_TYPE_WARP);
							}
						}
#if _DEBUG
						CHECK_ERROR(d3d11Device,
							L"Direct2DWindowsNativeControllerListener::NativeWindowCreated(INativeWindow*)#"
							L"Failed to create Direct3D 11 Device. "
							L"If you are running in Debug mode on Windows 10, please ensure the optional feature [Graphics Tools] is correctly installed. "
							L"This error will be skipped in Release mode and GacUI will fallback to use Direct2D 1.0, but you still need to check your Windows SDK Installation."
							);
#endif
					}

					if (d2dfactory1 && d3d11Device)
					{
						listener = new Direct2DWindowsNativeWindowListener_1_1(window, d2dfactory1, d3d11Device.Obj());
					}
					else
					{
						listener = new Direct2DWindowsNativeWindowListener_1_0(window, d2dFactory.Obj());
					}
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

			ID2D1Factory* GetDirect2DFactory()
			{
				return direct2DListener->d2dFactory.Obj();
			}

			IDWriteFactory* GetDirectWriteFactory()
			{
				return direct2DListener->dwrite.Obj();
			}

			ID3D11Device* GetD3D11Device()
			{
				return direct2DListener->d3d11Device.Obj();
			}
		}

		namespace elements_windows_d2d
		{
/***********************************************************************
OS Supporting
***********************************************************************/

			class WinDirect2DApplicationDirect2DObjectProvider : public IWindowsDirect2DObjectProvider
			{
			protected:

				windows::Direct2DWindowsNativeWindowListener* GetNativeWindowListener(INativeWindow* window)
				{
					vint index = windows::direct2DListener->nativeWindowListeners.Keys().IndexOf(window);
					return index == -1
						? nullptr
						: windows::direct2DListener->nativeWindowListeners.Values().Get(index).Obj();
				}

			public:
				void RecreateRenderTarget(INativeWindow* window)override
				{
					if (auto listener = GetNativeWindowListener(window))
					{
						return listener->RecreateRenderTarget();
					}
				}

				void ResizeRenderTarget(INativeWindow* window)override
				{
					if (auto listener = GetNativeWindowListener(window))
					{
						return listener->ResizeRenderTarget();
					}
				}

				ID2D1RenderTarget* GetNativeWindowDirect2DRenderTarget(INativeWindow* window)override
				{
					if (auto listener = GetNativeWindowListener(window))
					{
						return listener->GetDirect2DRenderTarget();
					}
					return nullptr;
				}

				void StartRendering(INativeWindow* window)override
				{
					if (auto listener = GetNativeWindowListener(window))
					{
						listener->StartRendering();
						if (auto renderTarget = listener->GetDirect2DRenderTarget())
						{
							renderTarget->BeginDraw();
							renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
						}
					}
				}

				RenderTargetFailure StopRenderingAndPresent(INativeWindow* window)override
				{
					if (auto listener = GetNativeWindowListener(window))
					{
						listener->StopRendering();
						bool moved = listener->RetrieveAndResetMovedWhileRendering();

						if (auto renderTarget = listener->GetDirect2DRenderTarget())
						{
							HRESULT hr = renderTarget->EndDraw();
							if (hr == S_OK)
							{
								if (moved)
								{
									return RenderTargetFailure::ResizeWhileRendering;
								}
								else if (listener->PresentRenderTarget())
								{
									return RenderTargetFailure::None;
								}
							}
						}
					}
					return RenderTargetFailure::LostDevice;
				}

				ID2D1Factory* GetDirect2DFactory()override
				{
					return vl::presentation::windows::GetDirect2DFactory();
				}

				IDWriteFactory* GetDirectWriteFactory()override
				{
					return vl::presentation::windows::GetDirectWriteFactory();
				}

				IWindowsDirect2DRenderTarget* GetBindedRenderTarget(INativeWindow* window)override
				{
					return dynamic_cast<IWindowsDirect2DRenderTarget*>(vl::presentation::windows::GetWindowsForm(window)->GetGraphicsHandler());
				}

				void SetBindedRenderTarget(INativeWindow* window, IWindowsDirect2DRenderTarget* renderTarget)override
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
	InitDpiAwareness(true);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	HINSTANCE hInstance=(HINSTANCE)GetModuleHandle(NULL);
	WinDirect2DApplicationDirect2DObjectProvider objectProvider;
	SetWindowsDirect2DObjectProvider(&objectProvider);
	return WinMainDirect2D(hInstance, &RendererMainDirect2D);
}