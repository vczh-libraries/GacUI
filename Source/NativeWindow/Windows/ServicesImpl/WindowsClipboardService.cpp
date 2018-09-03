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
			using namespace parsing::xml;
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
				return ContainsFormat(CF_BITMAP);
			}

			Ptr<INativeImage> WindowsClipboardReader::GetImage()
			{
				HBITMAP handle = (HBITMAP)::GetClipboardData(CF_BITMAP);
				if (handle != 0)
				{
					return CreateImageFromHBITMAP(handle);
				}
				return nullptr;
			}

			void WindowsClipboardReader::CloseClipboard()
			{
				if (service->reader)
				{
					::CloseClipboard();
					service->reader = nullptr;
				}
			}

/***********************************************************************
WindowsClipboardWriter
***********************************************************************/

			void WindowsClipboardWriter::SetClipboardData(UINT format, stream::MemoryStream& memoryStream)
			{
				memoryStream.SeekFromBegin(0);
				HGLOBAL data = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)memoryStream.Size());
				auto buffer = ::GlobalLock(data);
				memoryStream.Read(buffer, (vint)memoryStream.Size());
				::GlobalUnlock(data);
				::SetClipboardData(format, data);
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
				documentData = value;
				if (!textData)
				{
					textData = documentData->GetText(true);
				}

				if (!imageData && documentData->paragraphs.Count() == 1)
				{
					Ptr<DocumentContainerRun> container = documentData->paragraphs[0];
					while (container)
					{
						if (container->runs.Count() != 1) goto FAILED;
						if (auto imageRun = container->runs[0].Cast<DocumentImageRun>())
						{
							imageData = imageRun->image;
							break;
						}
						else
						{
							container = container->runs[0].Cast<DocumentContainerRun>();
						}
					}
				FAILED:;
				}

				ModifyDocumentForClipboard(documentData);
			}

			void WindowsClipboardWriter::SetImage(Ptr<INativeImage> value)
			{
				imageData = value;
			}

			bool WindowsClipboardWriter::Submit()
			{
				if (service->reader)
				{
					service->reader->CloseClipboard();
				}

				if (!::OpenClipboard(service->ownerHandle)) return false;
				::EmptyClipboard();

				if (textData)
				{
					vint size = (textData.Value().Length() + 1) * sizeof(wchar_t);
					HGLOBAL data = ::GlobalAlloc(GMEM_MOVEABLE, size);
					auto buffer = (wchar_t*)::GlobalLock(data);
					memcpy(buffer, textData.Value().Buffer(), size);
					::GlobalUnlock(data);
					::SetClipboardData(CF_UNICODETEXT, data);
				}

				if (documentData)
				{
					{
						stream::MemoryStream memoryStream;
						SaveDocumentToClipboardStream(documentData, memoryStream);
						SetClipboardData(service->WCF_Document, memoryStream);
					}
					{
						stream::MemoryStream memoryStream;
						SaveDocumentToRtfStream(documentData, memoryStream);
						SetClipboardData(service->WCF_RTF, memoryStream);
					}
					{
						stream::MemoryStream memoryStream;
						SaveDocumentToHtmlClipboardStream(documentData, memoryStream);
						SetClipboardData(service->WCF_HTML, memoryStream);
					}
				}

				if (imageData && imageData->GetFrameCount()>0)
				{
					if (auto wicBitmap = GetWICBitmap(imageData->GetFrame(0)))
					{
						UINT width = 0;
						UINT height = 0;
						wicBitmap->GetSize(&width, &height);
						auto bitmap = MakePtr<WinBitmap>((vint)width, (vint)height, WinBitmap::vbb32Bits, true);

						WICRect rect;
						rect.X = 0;
						rect.Y = 0;
						rect.Width = (INT)width;
						rect.Height = (INT)height;
						wicBitmap->CopyPixels(&rect, (UINT)bitmap->GetLineBytes(), (UINT)(bitmap->GetLineBytes()*height), (BYTE*)bitmap->GetScanLines()[0]);

						stream::MemoryStream memoryStream;
						bitmap->SaveToStream(memoryStream, true);
						SetClipboardData(CF_DIBV5, memoryStream);
					}
				}

				::CloseClipboard();
				return true;
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

			WindowsClipboardService::WindowsClipboardService()
				:ownerHandle(NULL)
			{
				WCF_Document = ::RegisterClipboardFormat(L"GacUI Document Format");
				WCF_RTF = ::RegisterClipboardFormat(L"Rich Text Format");
				WCF_HTML = ::RegisterClipboardFormat(L"HTML Format");
			}

			Ptr<INativeClipboardReader> WindowsClipboardService::ReadClipboard()
			{
				if (!reader)
				{
					if (!::OpenClipboard(ownerHandle)) return new WindowsFakeClipboardReader;
					reader = new WindowsClipboardReader(this);
				}
				return reader;
			}

			Ptr<INativeClipboardWriter> WindowsClipboardService::WriteClipboard()
			{
				return new WindowsClipboardWriter(this);
			}

			void WindowsClipboardService::SetOwnerHandle(HWND handle)
			{
				HWND oldHandle=ownerHandle;
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