/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSIMAGESERIVCE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSIMAGESERIVCE

#include "..\..\GuiNativeWindow.h"
#include "..\WinNativeDpiAwareness.h"
#include <wincodec.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsImageFrame : public Object, public INativeImageFrame
			{
			protected:
				INativeImage*													image;
				ComPtr<IWICBitmap>												frameBitmap;
				collections::Dictionary<void*, Ptr<INativeImageFrameCache>>		caches;

				void										Initialize(IWICBitmapSource* bitmapSource);
			public:
				WindowsImageFrame(INativeImage* _image, IWICBitmapFrameDecode* frameDecode);
				WindowsImageFrame(INativeImage* _image, IWICBitmap* sourceBitmap);
				~WindowsImageFrame();

				INativeImage*								GetImage()override;
				Size										GetSize()override;
				bool										SetCache(void* key, Ptr<INativeImageFrameCache> cache)override;
				Ptr<INativeImageFrameCache>					GetCache(void* key)override;
				Ptr<INativeImageFrameCache>					RemoveCache(void* key)override;
				IWICBitmap*									GetFrameBitmap();
				void										SaveBitmapToStream(stream::IStream& stream);
			};

			class WindowsImage : public Object, public INativeImage
			{
			protected:
				INativeImageService*						imageService;
				ComPtr<IWICBitmapDecoder>					bitmapDecoder;
				collections::Array<Ptr<WindowsImageFrame>>	frames;
			public:
				WindowsImage(INativeImageService* _imageService, IWICBitmapDecoder* _bitmapDecoder);
				~WindowsImage();

				INativeImageService*						GetImageService()override;
				FormatType									GetFormat()override;
				vint										GetFrameCount()override;
				INativeImageFrame*							GetFrame(vint index)override;
				void										SaveToStream(stream::IStream& stream, FormatType formatType)override;
			};

			class WindowsBitmapImage : public Object, public INativeImage
			{
			protected:
				INativeImageService*						imageService;
				Ptr<WindowsImageFrame>						frame;
				FormatType									formatType;
			public:
				WindowsBitmapImage(INativeImageService* _imageService, IWICBitmap* sourceBitmap, FormatType _formatType);
				~WindowsBitmapImage();

				INativeImageService*						GetImageService()override;
				FormatType									GetFormat()override;
				vint										GetFrameCount()override;
				INativeImageFrame*							GetFrame(vint index)override;
				void										SaveToStream(stream::IStream& stream, FormatType formatType)override;
			};

			class WindowsImageService : public Object, public INativeImageService
			{
			protected:
				ComPtr<IWICImagingFactory>					imagingFactory;
			public:
				WindowsImageService();
				~WindowsImageService();

				Ptr<INativeImage>							CreateImageFromFile(const WString& path);
				Ptr<INativeImage>							CreateImageFromMemory(void* buffer, vint length);
				Ptr<INativeImage>							CreateImageFromStream(stream::IStream& stream);
				Ptr<INativeImage>							CreateImageFromHBITMAP(HBITMAP handle);
				Ptr<INativeImage>							CreateImageFromHICON(HICON handle);
				IWICImagingFactory*							GetImagingFactory();
			};

			extern IWICImagingFactory*						GetWICImagingFactory();
			extern IWICBitmap*								GetWICBitmap(INativeImageFrame* frame);
			extern Ptr<INativeImage>						CreateImageFromHBITMAP(HBITMAP handle);
			extern Ptr<INativeImage>						CreateImageFromHICON(HICON handle);
		}
	}
}

#endif