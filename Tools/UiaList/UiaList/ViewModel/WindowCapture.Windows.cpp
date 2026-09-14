#include "WindowCapture.Windows.h"
#include "UiaCatalog.Windows.h"
#pragma push_macro("new")
#undef new
#include <windows.graphics.capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <windows.foundation.h>
#include <roapi.h>
#include <wrl.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#pragma pop_macro("new")

using namespace vl;
using namespace vl::collections;
namespace Capture = ABI::Windows::Graphics::Capture;
namespace Foundation = ABI::Windows::Foundation;
namespace Direct3D = ABI::Windows::Graphics::DirectX::Direct3D11;
template<typename T>
using AgileHandler = Microsoft::WRL::Implements<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, T, Microsoft::WRL::FtmBase>;

namespace uialist::native
{
	CaptureRequest::CaptureRequest(WindowIdentity window, vint epoch) : target(window), generation(epoch)
	{
		cancel = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!cancel) CheckUia(HRESULT_FROM_WIN32(GetLastError()), L"CreateEvent(capture cancellation)");
	}
	CaptureRequest::~CaptureRequest() { CloseHandle(cancel); }
	void CaptureRequest::Cancel() { SetEvent(cancel); }
	void CaptureWorker::Run()
	{
		CheckUia(RoInitialize(RO_INIT_MULTITHREADED), L"RoInitialize(capture)");
		queue.RunTaskQueue();
		RoUninitialize();
	}

	template<typename T>
	ComPtr<T> Factory(const wchar_t* name)
	{
		HSTRING value = nullptr;
		CheckUia(WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), &value), L"WindowsCreateString");
		T* result = nullptr;
		auto hr = RoGetActivationFactory(value, __uuidof(T), reinterpret_cast<void**>(&result));
		WindowsDeleteString(value);
		if (hr == REGDB_E_CLASSNOTREG || hr == E_NOINTERFACE) return nullptr;
		CheckUia(hr, L"RoGetActivationFactory(" + WString(name) + L")");
		return ComPtr<T>(result);
	}

	struct FrameCompletion : Object
	{
		std::atomic<int> outcome = 0;
		HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		ComPtr<Capture::IDirect3D11CaptureFrame> frame;
		HRESULT error = S_OK;
		~FrameCompletion() { CloseHandle(ready); }
		void Complete(int reason)
		{
			int expected = 0;
			if (outcome.compare_exchange_strong(expected, reason)) SetEvent(ready);
		}
	};

	void CloseCaptureObject(IUnknown* object)
	{
		if (auto close = QueryInterface<Foundation::IClosable>(object)) CheckUia(close->Close(), L"Close(capture object)");
	}

	Ptr<CaptureSnapshot> CaptureWindow(Ptr<CaptureRequest> request)
	{
		auto result = Ptr(new CaptureSnapshot);
		auto window = request->target.handle;
		if (!IsCurrentWindow(request->target) || !IsWindowVisible(window) || IsIconic(window)) return result;
		DWORD cloaked = 0;
		CheckUia(DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked)), L"Capture.DWMWA_CLOAKED");
		if (cloaked) return result;
		auto support = Factory<Capture::IGraphicsCaptureSessionStatics>(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureSession);
		if (!support) return result;
		boolean supported = false;
		CheckUia(support->IsSupported(&supported), L"GraphicsCaptureSession.IsSupported");
		if (!supported) return result;
		auto interop = Factory<IGraphicsCaptureItemInterop>(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem);
		auto factory = Factory<Capture::IDirect3D11CaptureFramePoolStatics2>(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool);
		if (!interop || !factory) return result;
		CheckUia(DwmGetWindowAttribute(window, DWMWA_EXTENDED_FRAME_BOUNDS, &result->bounds, sizeof(result->bounds)), L"Capture.extent before frame");
		result->dpi = GetDpiForWindow(window);
		Capture::IGraphicsCaptureItem* itemValue = nullptr;
		auto hr = interop->CreateForWindow(window, __uuidof(Capture::IGraphicsCaptureItem), reinterpret_cast<void**>(&itemValue));
		if (hr == E_ACCESSDENIED) return result;
		CheckUia(hr, L"GraphicsCaptureItem.CreateForWindow");
		ComPtr<Capture::IGraphicsCaptureItem> item(itemValue);
		ABI::Windows::Graphics::SizeInt32 size = {};
		CheckUia(item->get_Size(&size), L"GraphicsCaptureItem.Size");
		if (size.Width <= 0 || size.Height <= 0) return result;
		ID3D11Device* deviceValue = nullptr;
		ID3D11DeviceContext* contextValue = nullptr;
		CheckUia(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &deviceValue, nullptr, &contextValue), L"D3D11CreateDevice");
		ComPtr<ID3D11Device> device(deviceValue);
		ComPtr<ID3D11DeviceContext> context(contextValue);
		auto dxgi = QueryInterface<IDXGIDevice>(device.Obj());
		IInspectable* projectedValue = nullptr;
		CheckUia(CreateDirect3D11DeviceFromDXGIDevice(dxgi.Obj(), &projectedValue), L"CreateDirect3D11DeviceFromDXGIDevice");
		ComPtr<IInspectable> projected(projectedValue);
		auto directDevice = QueryInterface<Direct3D::IDirect3DDevice>(projected.Obj());
		Capture::IDirect3D11CaptureFramePool* poolValue = nullptr;
		CheckUia(factory->CreateFreeThreaded(directDevice.Obj(), ABI::Windows::Graphics::DirectX::DirectXPixelFormat_B8G8R8A8UIntNormalized, 1, size, &poolValue), L"FramePool.CreateFreeThreaded");
		ComPtr<Capture::IDirect3D11CaptureFramePool> pool(poolValue);
		Capture::IGraphicsCaptureSession* sessionValue = nullptr;
		CheckUia(pool->CreateCaptureSession(item.Obj(), &sessionValue), L"CreateCaptureSession");
		ComPtr<Capture::IGraphicsCaptureSession> session(sessionValue);
		if (auto cursor = QueryInterface<Capture::IGraphicsCaptureSession2>(session.Obj())) CheckUia(cursor->put_IsCursorCaptureEnabled(false), L"IsCursorCaptureEnabled");

		auto completion = Ptr(new FrameCompletion);
		if (!completion->ready) CheckUia(HRESULT_FROM_WIN32(GetLastError()), L"CreateEvent(capture completion)");
		auto arrived = Microsoft::WRL::Callback<AgileHandler<__FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable>>(
			[completion](Capture::IDirect3D11CaptureFramePool* sender, IInspectable*) -> HRESULT
			{
				if (completion->outcome.load()) return S_OK;
				Capture::IDirect3D11CaptureFrame* value = nullptr;
				auto error = sender->TryGetNextFrame(&value);
				ComPtr<Capture::IDirect3D11CaptureFrame> frame(value);
				ABI::Windows::Graphics::SizeInt32 content = {};
				if (SUCCEEDED(error) && frame) error = frame->get_ContentSize(&content);
				if (SUCCEEDED(error) && (!frame || content.Width <= 0 || content.Height <= 0)) return S_OK;
				int expected = 0;
				if (completion->outcome.compare_exchange_strong(expected, 1))
				{
					completion->error = error;
					completion->frame = frame;
					SetEvent(completion->ready);
				}
				return S_OK;
			});
		auto closed = Microsoft::WRL::Callback<AgileHandler<__FITypedEventHandler_2_Windows__CGraphics__CCapture__CGraphicsCaptureItem_IInspectable>>(
			[completion](Capture::IGraphicsCaptureItem*, IInspectable*) -> HRESULT { completion->Complete(2); return S_OK; });
		EventRegistrationToken frameToken = {}, closedToken = {};
		CheckUia(pool->add_FrameArrived(arrived.Get(), &frameToken), L"FrameArrived registration");
		CheckUia(item->add_Closed(closed.Get(), &closedToken), L"Closed registration");
		CheckUia(session->StartCapture(), L"StartCapture");
		HANDLE waits[] = { completion->ready, request->cancel };
		auto wait = WaitForMultipleObjects(2, waits, FALSE, 5000);
		if (wait == WAIT_FAILED) CheckUia(HRESULT_FROM_WIN32(GetLastError()), L"WaitForMultipleObjects(capture)");
		if (wait == WAIT_TIMEOUT) completion->Complete(3);
		if (wait == WAIT_OBJECT_0 + 1) completion->Complete(4);
		WaitForSingleObject(completion->ready, INFINITE);
		CheckUia(pool->remove_FrameArrived(frameToken), L"FrameArrived revoke");
		CheckUia(item->remove_Closed(closedToken), L"Closed revoke");
		CheckUia(completion->error, L"FrameArrived.TryGetNextFrame");

		if (completion->outcome == 1)
		{
			auto frame = completion->frame;
			ABI::Windows::Graphics::SizeInt32 content = {};
			CheckUia(frame->get_ContentSize(&content), L"Frame.ContentSize");
			Direct3D::IDirect3DSurface* surfaceValue = nullptr;
			CheckUia(frame->get_Surface(&surfaceValue), L"Frame.Surface");
			ComPtr<Direct3D::IDirect3DSurface> surface(surfaceValue);
			auto access = QueryInterface<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>(surface.Obj());
			ID3D11Texture2D* textureValue = nullptr;
			CheckUia(access->GetInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&textureValue)), L"GetInterface(ID3D11Texture2D)");
			ComPtr<ID3D11Texture2D> texture(textureValue);
			D3D11_TEXTURE2D_DESC desc = {};
			texture->GetDesc(&desc);
			if (content.Width > static_cast<int>(desc.Width) || content.Height > static_cast<int>(desc.Height)) throw Exception(L"Capture content exceeds texture.");
			desc.Width = content.Width; desc.Height = content.Height;
			desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = 0; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; desc.MiscFlags = 0;
			ID3D11Texture2D* stagingValue = nullptr;
			CheckUia(device->CreateTexture2D(&desc, nullptr, &stagingValue), L"CreateTexture2D(staging)");
			ComPtr<ID3D11Texture2D> staging(stagingValue);
			D3D11_BOX area = {0, 0, 0, static_cast<UINT>(content.Width), static_cast<UINT>(content.Height), 1};
			context->CopySubresourceRegion(staging.Obj(), 0, 0, 0, 0, texture.Obj(), 0, &area);
			D3D11_MAPPED_SUBRESOURCE mapping = {};
			CheckUia(context->Map(staging.Obj(), 0, D3D11_MAP_READ, 0, &mapping), L"Map(capture staging)");
			auto byteCount = static_cast<vuint64_t>(content.Width) * content.Height * 4;
			constexpr auto headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			if (byteCount > static_cast<vuint64_t>(INT_MAX) - headerSize) throw Exception(L"Capture bitmap exceeds addressable image size.");
			auto bytes = static_cast<vint>(byteCount);
			result->bitmap.Resize(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bytes);
			BITMAPFILEHEADER file = {0x4D42, static_cast<DWORD>(result->bitmap.Count()), 0, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)};
			BITMAPINFOHEADER info = {};
			info.biSize = sizeof(info); info.biWidth = content.Width; info.biHeight = -content.Height; info.biPlanes = 1; info.biBitCount = 32; info.biCompression = BI_RGB;
			memcpy(&result->bitmap[0], &file, sizeof(file));
			memcpy(&result->bitmap[sizeof(file)], &info, sizeof(info));
			for (vint row = 0; row < content.Height; row++)
				memcpy(&result->bitmap[file.bfOffBits + row * content.Width * 4], static_cast<const BYTE*>(mapping.pData) + row * mapping.RowPitch, content.Width * 4);
			context->Unmap(staging.Obj(), 0);
			CloseCaptureObject(frame.Obj());
			completion->frame = nullptr;
			RECT after = {};
			CheckUia(DwmGetWindowAttribute(window, DWMWA_EXTENDED_FRAME_BOUNDS, &after, sizeof(after)), L"Capture.extent after frame");
			result->status = EqualRect(&after, &result->bounds) && content.Width == after.right - after.left && content.Height == after.bottom - after.top ? CaptureStatus::Ready : CaptureStatus::GeometryChanged;
			SYSTEMTIME time;
			GetLocalTime(&time);
			wchar_t text[64];
			swprintf_s(text, L"%04u-%02u-%02u %02u:%02u:%02u.%03u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
			result->timestamp = text;
		}
		else if (completion->outcome == 4) result->status = CaptureStatus::Canceled;
		CloseCaptureObject(session.Obj());
		CloseCaptureObject(pool.Obj());
		return result;
	}
}
