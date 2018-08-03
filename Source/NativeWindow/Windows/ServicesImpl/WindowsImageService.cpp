#include "WindowsImageService.h"
#include "../GDI/WinGDI.h"
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

			void WindowsImageFrame::SaveBitmapToStream(stream::IStream& stream)
			{
				UINT width = 0;
				UINT height = 0;
				frameBitmap->GetSize(&width, &height);
				auto bitmap = MakePtr<WinBitmap>((vint)width, (vint)height, WinBitmap::vbb32Bits, true);

				WICRect rect;
				rect.X = 0;
				rect.Y = 0;
				rect.Width = (INT)width;
				rect.Height = (INT)height;
				frameBitmap->CopyPixels(&rect, (UINT)bitmap->GetLineBytes(), (UINT)(bitmap->GetLineBytes()*height), (BYTE*)bitmap->GetScanLines()[0]);

				bitmap->SaveToStream(stream, false);
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

			void CopyMetadataBody(IWICMetadataQueryReader* reader, IWICMetadataQueryWriter* writer, const WString& prefix)
			{
				IEnumString* enumString = nullptr;
				HRESULT hr = reader->GetEnumerator(&enumString);
				if (enumString)
				{
					while (true)
					{
						LPOLESTR metadataName = nullptr;
						ULONG fetched = 0;
						hr = enumString->Next(1, &metadataName, &fetched);
						if (hr != S_OK) break;
						if (fetched == 0) break;

						PROPVARIANT metadataValue;
						PropVariantInit(&metadataValue);
						hr = reader->GetMetadataByName(metadataName, &metadataValue);
						if (hr == S_OK)
						{
							if (metadataValue.vt == VT_UNKNOWN && metadataValue.punkVal)
							{
								IWICMetadataQueryReader* embeddedReader = nullptr;
								hr = metadataValue.punkVal->QueryInterface<IWICMetadataQueryReader>(&embeddedReader);
								if (embeddedReader)
								{
									CopyMetadataBody(embeddedReader, writer, prefix + metadataName);
									embeddedReader->Release();
								}
							}
							else
							{
								hr = writer->SetMetadataByName((prefix + metadataName).Buffer(), &metadataValue);
							}
							hr = PropVariantClear(&metadataValue);
						}

						CoTaskMemFree(metadataName);
					}
					enumString->Release();
				}
			}

			template<typename TDecoder, typename TEncoder>
			void CopyMetadata(TDecoder* decoder, TEncoder* encoder)
			{
				IWICMetadataQueryReader* reader = nullptr;
				IWICMetadataQueryWriter* writer = nullptr;
				HRESULT hr = decoder->GetMetadataQueryReader(&reader);
				hr = encoder->GetMetadataQueryWriter(&writer);
				if (reader && writer)
				{
					CopyMetadataBody(reader, writer, WString::Empty);
				}
				if (reader) reader->Release();
				if (writer) writer->Release();
			}

			GUID GetGuidFromFormat(INativeImage::FormatType formatType)
			{
				switch (formatType)
				{
				case INativeImage::Bmp: return GUID_ContainerFormatBmp;
				case INativeImage::Gif: return GUID_ContainerFormatGif;
				case INativeImage::Icon: return GUID_ContainerFormatIco;
				case INativeImage::Jpeg: return GUID_ContainerFormatJpeg;
				case INativeImage::Png: return GUID_ContainerFormatPng;
				case INativeImage::Tiff: return GUID_ContainerFormatTiff;
				case INativeImage::Wmp: return GUID_ContainerFormatWmp;
				default: CHECK_FAIL(L"GetGuidFromFormat(INativeImage::FormatType)#Unexpected format type.");
				}
			}

			void MoveIStreamToStream(IStream* pIStream, stream::IStream& stream)
			{
				LARGE_INTEGER dlibMove;
				dlibMove.QuadPart = 0;
				HRESULT hr = pIStream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
				Array<char> buffer(65536);
				while (true)
				{
					ULONG count = (ULONG)buffer.Count();
					ULONG read = 0;
					hr = pIStream->Read(&buffer[0], count, &read);
					if (read > 0)
					{
						stream.Write(&buffer[0], (vint)read);
					}
					if (read != count)
					{
						break;
					}
				}
			}

			void WindowsImage::SaveToStream(stream::IStream& stream, FormatType formatType)
			{
				auto factory = GetWICImagingFactory();
				GUID formatGUID;
				HRESULT hr;

				bool sameFormat = formatType == INativeImage::Unknown || formatType == GetFormat();
				if (formatType == INativeImage::Unknown)
				{
					hr = bitmapDecoder->GetContainerFormat(&formatGUID);
					if (hr != S_OK) goto FAILED;
				}
				else
				{
					formatGUID = GetGuidFromFormat(formatType);
				}

				{
					IWICBitmapEncoder* bitmapEncoder = nullptr;
					hr = factory->CreateEncoder(formatGUID, NULL, &bitmapEncoder);
					if (!bitmapEncoder) goto FAILED;

					IStream* pIStream = nullptr;
					hr = CreateStreamOnHGlobal(NULL, TRUE, &pIStream);
					if (!pIStream)
					{
						bitmapEncoder->Release();
						goto FAILED;
					}

					hr = bitmapEncoder->Initialize(pIStream, WICBitmapEncoderNoCache);
					if (hr != S_OK)
					{
						pIStream->Release();
						bitmapEncoder->Release();
						goto FAILED;
					}

					{
						UINT actualCount = 0;
						Array<IWICColorContext*> colorContexts(16);
						hr = bitmapDecoder->GetColorContexts((UINT)colorContexts.Count(), &colorContexts[0], &actualCount);
						if (hr == S_OK)
						{
							if ((vint)actualCount > colorContexts.Count())
							{
								for (vint i = 0; i < colorContexts.Count(); i++) colorContexts[i]->Release();
								colorContexts.Resize((vint)actualCount);
								bitmapDecoder->GetColorContexts(actualCount, &colorContexts[0], &actualCount);
							}
							if (actualCount > 0)
							{
								bitmapEncoder->SetColorContexts(actualCount, &colorContexts[0]);
								for (vint i = 0; i < (vint)actualCount; i++) colorContexts[i]->Release();
							}
						}
					}
					{
						IWICPalette* palette = nullptr;
						hr = factory->CreatePalette(&palette);
						if (palette)
						{
							hr = bitmapDecoder->CopyPalette(palette);
							if (hr == S_OK)
							{
								bitmapEncoder->SetPalette(palette);
							}
							palette->Release();
						}
					}
					{
						IWICBitmapSource* source = nullptr;
						hr = bitmapDecoder->GetPreview(&source);
						if (source)
						{
							bitmapEncoder->SetPreview(source);
							source->Release();
						}
					}
					{
						IWICBitmapSource* source = nullptr;
						hr = bitmapDecoder->GetThumbnail(&source);
						if (source)
						{
							bitmapEncoder->SetThumbnail(source);
							source->Release();
						}
					}

					if (sameFormat)
					{
						CopyMetadata(bitmapDecoder.Obj(), bitmapEncoder);
					}

					UINT frameCount = 0;
					bitmapDecoder->GetFrameCount(&frameCount);
					for (UINT i = 0; i < frameCount; i++)
					{
						IWICBitmapFrameDecode* frameDecode = nullptr;
						IWICBitmapFrameEncode* frameEncode = nullptr;
						hr = bitmapDecoder->GetFrame(i, &frameDecode);
						hr = bitmapEncoder->CreateNewFrame(&frameEncode, NULL);
						if (frameDecode && frameEncode)
						{
							hr = frameEncode->Initialize(NULL);
							if (sameFormat)
							{
								CopyMetadata(frameDecode, frameEncode);
							}
							hr = frameEncode->WriteSource(frameDecode, NULL);
							hr = frameEncode->Commit();
						}
						if (frameDecode) frameDecode->Release();
						if (frameEncode) frameEncode->Release();
					}

					hr = bitmapEncoder->Commit();
					bitmapEncoder->Release();
					MoveIStreamToStream(pIStream, stream);
					pIStream->Release();
				}
			FAILED:;
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

			void WindowsBitmapImage::SaveToStream(stream::IStream& stream, FormatType formatType)
			{
				auto factory = GetWICImagingFactory();
				if (formatType == INativeImage::Unknown)
				{
					formatType = INativeImage::Bmp;
				}
				GUID formatGUID = GetGuidFromFormat(formatType);

				IWICBitmapEncoder* bitmapEncoder = nullptr;
				HRESULT hr = factory->CreateEncoder(formatGUID, NULL, &bitmapEncoder);
				if (!bitmapEncoder) goto FAILED;

				{
					IStream* pIStream = nullptr;
					hr = CreateStreamOnHGlobal(NULL, TRUE, &pIStream);
					if (!pIStream)
					{
						bitmapEncoder->Release();
						goto FAILED;
					}

					hr = bitmapEncoder->Initialize(pIStream, WICBitmapEncoderNoCache);
					if (hr != S_OK)
					{
						pIStream->Release();
						bitmapEncoder->Release();
						goto FAILED;
					}

					{
						IWICBitmapFrameEncode* frameEncode = nullptr;
						hr = bitmapEncoder->CreateNewFrame(&frameEncode, NULL);
						if (frameEncode)
						{
							hr = frameEncode->Initialize(NULL);
							hr = frameEncode->WriteSource(frame->GetFrameBitmap(), NULL);
							hr = frameEncode->Commit();
							frameEncode->Release();
						}
					}

					hr = bitmapEncoder->Commit();
					bitmapEncoder->Release();
					MoveIStreamToStream(pIStream, stream);
					pIStream->Release();
				}
			FAILED:;
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