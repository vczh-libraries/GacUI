#include "../../../Resources/GuiDocumentClipboard.h"
#include "WindowsClipboardService.h"
#include "WindowsImageService.h"
#include "../GDI/WinGDI.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace collections;

/***********************************************************************
WindowsClipboardReader
***********************************************************************/

			bool WindowsClipboardReader::ContainsFormat(UINT format)
			{
				UINT currentFormat = 0;
				while (currentFormat = ::EnumClipboardFormats(currentFormat))
				{
					if (currentFormat == format)
					{
						return true;
					}
				}
				return false;
			}

			WindowsClipboardReader::WindowsClipboardReader(WindowsClipboardService* _service)
				:service(_service)
			{
			}

			WindowsClipboardReader::~WindowsClipboardReader()
			{
				CloseClipboard();
			}

			bool WindowsClipboardReader::ContainsText()
			{
				return ContainsFormat(CF_UNICODETEXT);
			}

			WString WindowsClipboardReader::GetText()
			{
				WString result;
				HANDLE handle = ::GetClipboardData(CF_UNICODETEXT);
				if (handle != 0)
				{
					wchar_t* buffer = (wchar_t*)::GlobalLock(handle);
					result = buffer;
					::GlobalUnlock(handle);
				}
				return result;
			}

			bool WindowsClipboardReader::ContainsDocument()
			{
				return ContainsFormat(service->WCF_Document);
			}

			Ptr<DocumentModel> WindowsClipboardReader::GetDocument()
			{
				HANDLE handle = ::GetClipboardData(service->WCF_Document);
				if (handle != 0)
				{
					auto buffer = ::GlobalLock(handle);
					auto size = ::GlobalSize(handle);
					stream::MemoryWrapperStream memoryStream(buffer, (vint)size);
					auto document = LoadDocumentFromClipboardStream(memoryStream);
					::GlobalUnlock(handle);
					return document;
				}
				return nullptr;
			}

			bool WindowsClipboardReader::ContainsImage()
			{
				return ContainsFormat(service->WCF_PNG) || ContainsFormat(CF_BITMAP);
			}

			Ptr<INativeImage> WindowsClipboardReader::GetImage()
			{
				if (auto png = ::GetClipboardData(service->WCF_PNG))
				{
					auto size = ::GlobalSize(png);
					if (size > 0 && size <= INT_MAX)
					{
						if (auto data = ::GlobalLock(png))
						{
							auto image = GetCurrentController()->ImageService()->CreateImageFromMemory(data, (vint)size);
							::GlobalUnlock(png);
							if (image) return image;
						}
					}
				}

				HBITMAP handle = (HBITMAP)::GetClipboardData(CF_BITMAP);
				if (handle != 0)
				{
					if (!service->nativeImageService) return CreateImageFromHBITMAP(handle);
					auto nativeImage = service->nativeImageService->CreateImageFromHBITMAP(handle);
					if (!nativeImage) return nullptr;
					stream::MemoryStream png;
					nativeImage->SaveToStream(png, INativeImage::Png);
					png.SeekFromBegin(0);
					return GetCurrentController()->ImageService()->CreateImageFromStream(png);
				}
				return nullptr;
			}

			void WindowsClipboardReader::CloseClipboard()
			{
				if (service->reader == this)
				{
					::CloseClipboard();
					service->reader = nullptr;
				}
			}

/***********************************************************************
WindowsClipboardWriter
***********************************************************************/

			bool WindowsClipboardWriter::SetClipboardData(UINT format, stream::MemoryStream& bytes)
			{
				auto size = (SIZE_T)bytes.Size();
				HGLOBAL data = ::GlobalAlloc(GMEM_MOVEABLE, size);
				if (!data) return false;
				auto buffer = ::GlobalLock(data);
				if (buffer)
				{
					memcpy(buffer, bytes.GetInternalBuffer(), size);
					::GlobalUnlock(data);
					if (::SetClipboardData(format, data)) return true;
				}
				::GlobalFree(data);
				return false;
			}

			WindowsClipboardWriter::WindowsClipboardWriter(WindowsClipboardService* _service)
				:service(_service)
			{
			}

			WindowsClipboardWriter::~WindowsClipboardWriter()
			{
			}

			void WindowsClipboardWriter::SetText(const WString& value)
			{
				textData = value;
			}

			void WindowsClipboardWriter::SetDocument(Ptr<DocumentModel> value)
			{
				if (!textData)
				{
					textData = value->GetTextForReading(WString::Unmanaged(L"\r\n\r\n"));
				}

				if (!imageData)
				{
					imageData = GetImageFromSingleImageDocument(value);
				}

				documentData = value;
				ModifyDocumentForClipboard(documentData);
			}

			void WindowsClipboardWriter::SetImage(Ptr<INativeImage> value)
			{
				imageData = value;
			}

			bool WindowsClipboardWriter::Submit()
			{
				if (service->reader) service->reader->CloseClipboard();

				List<Pair<UINT, Ptr<stream::MemoryStream>>> formats;
				auto addFormat = [&](UINT type)
				{
					auto bytes = Ptr(new stream::MemoryStream);
					formats.Add({ type, bytes });
					return bytes;
				};
				if (textData)
				{
					auto bytes = addFormat(CF_UNICODETEXT);
					bytes->Write((void*)textData.Value().Buffer(), (textData.Value().Length() + 1) * sizeof(wchar_t));
				}
				if (documentData)
				{
					SaveDocumentToClipboardStream(documentData, *addFormat(service->WCF_Document).Obj());
					SaveDocumentToRtfStream(documentData, *addFormat(service->WCF_RTF).Obj());
					SaveDocumentToHtmlClipboardStream(documentData, *addFormat(service->WCF_HTML).Obj());
				}
				if (imageData)
				{
					auto png = Ptr(new stream::MemoryStream);
					imageData->SaveToStream(*png.Obj(), INativeImage::Png);
					if (png->Size() > 0) formats.Add({ service->WCF_PNG, png });
					if (service->nativeImageService && imageData->GetImageService() != service->nativeImageService)
					{
						png->SeekFromBegin(0);
						imageData = service->nativeImageService->CreateImageFromStream(*png.Obj());
					}
					if (!imageData || imageData->GetFrameCount() == 0) return false;
					auto wicBitmap = GetWICBitmap(imageData->GetFrame(0));
					UINT width = 0, height = 0;
					if (!wicBitmap || FAILED(wicBitmap->GetSize(&width, &height))) return false;
					auto bitmap = Ptr(new WinBitmap((vint)width, (vint)height, WinBitmap::vbb32Bits, true));
					WICRect rect{ 0, 0, (INT)width, (INT)height };
					if (FAILED(wicBitmap->CopyPixels(&rect, (UINT)bitmap->GetLineBytes(),
						(UINT)(bitmap->GetLineBytes() * height), (BYTE*)bitmap->GetScanLines()[0]))) return false;
					bitmap->SaveToStream(*addFormat(CF_DIBV5).Obj(), true);
				}

				if (!::OpenClipboard(service->ownerHandle)) return false;
				bool success = ::EmptyClipboard() != FALSE;
				for (const auto& [type, bytes] : formats)
				{
					if (!success) break;
					success = SetClipboardData(type, *bytes.Obj());
				}
				::CloseClipboard();
				return success;
			}

/***********************************************************************
WindowsClipboardService
***********************************************************************/

			class WindowsFakeClipboardReader : public Object, public INativeClipboardReader
			{
			public:
				bool							ContainsText()override { return false; }
				WString							GetText()override { return L""; }
				bool							ContainsDocument()override { return false; }
				Ptr<DocumentModel>				GetDocument()override { return nullptr; }
				bool							ContainsImage()override { return false; }
				Ptr<INativeImage>				GetImage()override { return nullptr; }
			};

			WindowsClipboardService::WindowsClipboardService(WindowsImageService* _nativeImageService)
				: ownerHandle(NULL)
				, nativeImageService(_nativeImageService)
			{
				WCF_Document = ::RegisterClipboardFormat(L"GacUI Document Format");
				WCF_RTF = ::RegisterClipboardFormat(L"Rich Text Format");
				WCF_HTML = ::RegisterClipboardFormat(L"HTML Format");
				WCF_PNG = ::RegisterClipboardFormat(L"PNG");
			}

			Ptr<INativeClipboardReader> WindowsClipboardService::ReadClipboard()
			{
				if (!reader)
				{
					if (!::OpenClipboard(ownerHandle))
					{
						readPending = true;
						return Ptr(new WindowsFakeClipboardReader);
					}
					reader = new WindowsClipboardReader(this);
				}
				return Ptr(reader);
			}

			Ptr<INativeClipboardWriter> WindowsClipboardService::WriteClipboard()
			{
				return Ptr(new WindowsClipboardWriter(this));
			}

			void WindowsClipboardService::MarkClipboardUpdated()
			{
				readPending = true;
			}

			Ptr<INativeClipboardReader> WindowsClipboardService::AcquirePendingRead()
			{
				if (!readPending) return nullptr;
				readPending = false;
				// Keep the OS clipboard open until the listeners finish reading it.
				auto nativeReader = ReadClipboard();
				return readPending ? nullptr : nativeReader;
			}

			void WindowsClipboardService::SetOwnerHandle(HWND handle)
			{
				HWND oldHandle=ownerHandle;
				readPending = false;
				ownerHandle=handle;
				if(handle==NULL)
				{
					::RemoveClipboardFormatListener(oldHandle);
				}
				else
				{
					::AddClipboardFormatListener(ownerHandle);
				}
			}
		}
	}
}
