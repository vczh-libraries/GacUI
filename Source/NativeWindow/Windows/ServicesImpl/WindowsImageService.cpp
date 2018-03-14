#include "WindowsImageService.h"

#include <Shlwapi.h>

#pragma comment(lib, "WindowsCodecs.lib")

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace collections;

/***********************************************************************
WindowsImageFrame
***********************************************************************/

			void WindowsImageFrame::Initialize(IWICBitmapSource* bitmapSource)
			{
				IWICImagingFactory* factory=GetWICImagingFactory();
				ComPtr<IWICFormatConverter> converter;
				{
					IWICFormatConverter* formatConverter=0;
					HRESULT hr=factory->CreateFormatConverter(&formatConverter);
					if(SUCCEEDED(hr))
					{
						converter=formatConverter;
						converter->Initialize(
							bitmapSource,
							GUID_WICPixelFormat32bppPBGRA,
							WICBitmapDitherTypeNone,
							NULL,
							0.0f,
							WICBitmapPaletteTypeCustom);
					}
				}

				IWICBitmap* bitmap=0;
				IWICBitmapSource* convertedBitmapSource=0;
				if(converter)
				{
					convertedBitmapSource=converter.Obj();
				}
				else
				{
					convertedBitmapSource=bitmapSource;
				}
				HRESULT hr=factory->CreateBitmapFromSource(convertedBitmapSource, WICBitmapCacheOnLoad, &bitmap);
				if(SUCCEEDED(hr))
				{
					frameBitmap=bitmap;
				}
			}

			WindowsImageFrame::WindowsImageFrame(INativeImage* _image, IWICBitmapFrameDecode* frameDecode)
				:image(_image)
			{
				Initialize(frameDecode);
			}

			WindowsImageFrame::WindowsImageFrame(INativeImage* _image, IWICBitmap* sourceBitmap)
				:image(_image)
			{
				Initialize(sourceBitmap);
			}

			WindowsImageFrame::~WindowsImageFrame()
			{
				for(vint i=0;i<caches.Count();i++)
				{
					caches.Values().Get(i)->OnDetach(this);
				}
			}

			INativeImage* WindowsImageFrame::GetImage()
			{
				return image;
			}

			Size WindowsImageFrame::GetSize()
			{
				UINT width=0;
				UINT height=0;
				frameBitmap->GetSize(&width, &height);
				return Size(width, height);
			}

			bool WindowsImageFrame::SetCache(void* key, Ptr<INativeImageFrameCache> cache)
			{
				vint index=caches.Keys().IndexOf(key);
				if(index!=-1)
				{
					return false;
				}
				caches.Add(key, cache);
				cache->OnAttach(this);
				return true;
			}

			Ptr<INativeImageFrameCache> WindowsImageFrame::GetCache(void* key)
			{
				vint index=caches.Keys().IndexOf(key);
				return index==-1?nullptr:caches.Values().Get(index);
			}

			Ptr<INativeImageFrameCache> WindowsImageFrame::RemoveCache(void* key)
			{
				vint index=caches.Keys().IndexOf(key);
				if(index==-1)
				{
					return 0;
				}
				Ptr<INativeImageFrameCache> cache=caches.Values().Get(index);
				cache->OnDetach(this);
				caches.Remove(key);
				return cache;
			}

			IWICBitmap* WindowsImageFrame::GetFrameBitmap()
			{
				return frameBitmap.Obj();
			}

/***********************************************************************
WindowsImage
***********************************************************************/

			WindowsImage::WindowsImage(INativeImageService* _imageService, IWICBitmapDecoder* _bitmapDecoder)
				:imageService(_imageService)
				,bitmapDecoder(_bitmapDecoder)
			{
				UINT count=0;
				bitmapDecoder->GetFrameCount(&count);
				frames.Resize(count);
			}

			WindowsImage::~WindowsImage()
			{
			}

			INativeImageService* WindowsImage::GetImageService()
			{
				return imageService;
			}

			INativeImage::FormatType WindowsImage::GetFormat()
			{
				GUID formatGUID;
				HRESULT hr=bitmapDecoder->GetContainerFormat(&formatGUID);
				if(SUCCEEDED(hr))
				{
					if(formatGUID==GUID_ContainerFormatBmp)
					{
						return INativeImage::Bmp;
					}
					else if(formatGUID==GUID_ContainerFormatPng)
					{
						return INativeImage::Png;
					}
					else if(formatGUID==GUID_ContainerFormatGif)
					{
						return INativeImage::Gif;
					}
					else if(formatGUID==GUID_ContainerFormatJpeg)
					{
						return INativeImage::Jpeg;
					}
					else if(formatGUID==GUID_ContainerFormatIco)
					{
						return INativeImage::Icon;
					}
					else if(formatGUID==GUID_ContainerFormatTiff)
					{
						return INativeImage::Tiff;
					}
					else if(formatGUID==GUID_ContainerFormatWmp)
					{
						return INativeImage::Wmp;
					}
				}
				return INativeImage::Unknown;
			}

			vint WindowsImage::GetFrameCount()
			{
				return frames.Count();
			}

			INativeImageFrame* WindowsImage::GetFrame(vint index)
			{
				if(0<=index && index<GetFrameCount())
				{
					Ptr<WindowsImageFrame>& frame=frames[index];
					if(!frame)
					{
						IWICBitmapFrameDecode* frameDecode=0;
						HRESULT hr=bitmapDecoder->GetFrame((int)index, &frameDecode);
						if(SUCCEEDED(hr))
						{
							frame=new WindowsImageFrame(this, frameDecode);
							frameDecode->Release();
						}
					}
					return frame.Obj();
				}
				else
				{
					return 0;
				}
			}

/***********************************************************************
WindowsBitmapImage
***********************************************************************/

			WindowsBitmapImage::WindowsBitmapImage(INativeImageService* _imageService, IWICBitmap* sourceBitmap, FormatType _formatType)
				:imageService(_imageService)
				,formatType(_formatType)
			{
				frame = new WindowsImageFrame(this, sourceBitmap);
			}

			WindowsBitmapImage::~WindowsBitmapImage()
			{
			}

			INativeImageService* WindowsBitmapImage::GetImageService()
			{
				return imageService;
			}

			INativeImage::FormatType WindowsBitmapImage::GetFormat()
			{
				return formatType;
			}

			vint WindowsBitmapImage::GetFrameCount()
			{
				return 1;
			}

			INativeImageFrame* WindowsBitmapImage::GetFrame(vint index)
			{
				return index==0?frame.Obj():0;
			}

/***********************************************************************
WindowsImageService
***********************************************************************/

			WindowsImageService::WindowsImageService()
			{
				IWICImagingFactory* factory=0;
				HRESULT hr = CoCreateInstance(
#if defined(WINCODEC_SDK_VERSION2)
					CLSID_WICImagingFactory1,
#else
					CLSID_WICImagingFactory,
#endif
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_IWICImagingFactory,
					(LPVOID*)&factory
					);
				if(SUCCEEDED(hr))
				{
					imagingFactory=factory;
				}
			}

			WindowsImageService::~WindowsImageService()
			{
			}

			Ptr<INativeImage> WindowsImageService::CreateImageFromFile(const WString& path)
			{
				IWICBitmapDecoder* bitmapDecoder=0;
				HRESULT hr=imagingFactory->CreateDecoderFromFilename(
					path.Buffer(),
					NULL,
					GENERIC_READ,
					WICDecodeMetadataCacheOnDemand,
					&bitmapDecoder);
				if(SUCCEEDED(hr))
				{
					return new WindowsImage(this, bitmapDecoder);
				}
				else
				{
					return 0;
				}
			}

			Ptr<INativeImage> WindowsImageService::CreateImageFromMemory(void* buffer, vint length)
			{
				Ptr<INativeImage> result;
				::IStream* stream=SHCreateMemStream((const BYTE*)buffer, (int)length);
				if(stream)
				{
					IWICBitmapDecoder* bitmapDecoder=0;
					HRESULT hr=imagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);
					if(SUCCEEDED(hr))
					{
						result=new WindowsImage(this, bitmapDecoder);
					}
					stream->Release();
				}
				return result;
			}

			Ptr<INativeImage> WindowsImageService::CreateImageFromStream(stream::IStream& stream)
			{
				stream::MemoryStream memoryStream;
				char buffer[65536];
				while(true)
				{
					vint length=stream.Read(buffer, sizeof(buffer));
					memoryStream.Write(buffer, length);
					if(length!=sizeof(buffer))
					{
						break;
					}
				}
				return CreateImageFromMemory(memoryStream.GetInternalBuffer(), (vint)memoryStream.Size());
			}

			Ptr<INativeImage> WindowsImageService::CreateImageFromHBITMAP(HBITMAP handle)
			{
				IWICBitmap* bitmap=0;
				HRESULT hr=imagingFactory->CreateBitmapFromHBITMAP(handle, NULL, WICBitmapUseAlpha, &bitmap);
				if(SUCCEEDED(hr))
				{
					Ptr<INativeImage> image=new WindowsBitmapImage(this, bitmap, INativeImage::Bmp);
					bitmap->Release();
					return image;
				}
				else
				{
					return 0;
				}
			}

			Ptr<INativeImage> WindowsImageService::CreateImageFromHICON(HICON handle)
			{
				IWICBitmap* bitmap=0;
				HRESULT hr=imagingFactory->CreateBitmapFromHICON(handle, &bitmap);
				if(SUCCEEDED(hr))
				{
					Ptr<INativeImage> image=new WindowsBitmapImage(this, bitmap, INativeImage::Icon);
					bitmap->Release();
					return image;
				}
				else
				{
					return 0;
				}
			}

			IWICImagingFactory* WindowsImageService::GetImagingFactory()
			{
				return imagingFactory.Obj();
			}

/***********************************************************************
Helper Functions
***********************************************************************/

			IWICImagingFactory* GetWICImagingFactory()
			{
				return dynamic_cast<WindowsImageService*>(GetCurrentController()->ImageService())->GetImagingFactory();
			}

			IWICBitmap* GetWICBitmap(INativeImageFrame* frame)
			{
				return dynamic_cast<WindowsImageFrame*>(frame)->GetFrameBitmap();
			}

			Ptr<INativeImage> CreateImageFromHBITMAP(HBITMAP handle)
			{
				return dynamic_cast<WindowsImageService*>(GetCurrentController()->ImageService())->CreateImageFromHBITMAP(handle);
			}

			Ptr<INativeImage> CreateImageFromHICON(HICON handle)
			{
				return dynamic_cast<WindowsImageService*>(GetCurrentController()->ImageService())->CreateImageFromHICON(handle);
			}
		}
	}
}