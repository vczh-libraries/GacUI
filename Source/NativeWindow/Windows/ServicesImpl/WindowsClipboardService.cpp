#include "WindowsClipboardService.h"
#include "../../../Resources/GuiDocumentClipboard.h"
#include "../../../Resources/GuiParserManager.h"

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
					if (size % sizeof(wchar_t) != 0)
					{
						::GlobalUnlock(handle);
						return nullptr;
					}
					Array<wchar_t> textBuffer((vint)size / sizeof(wchar_t) + 1);
					memcpy(&textBuffer[0], buffer, size);
					textBuffer[textBuffer.Count() - 1] = 0;
					::GlobalUnlock(handle);

					WString text = &textBuffer[0];
					auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
					List<GuiResourceError> errors;
					auto xml = parser->Parse({}, text, errors);
					if (errors.Count() > 0) return nullptr;

					auto tempResource = MakePtr<GuiResource>();
					auto tempResourceItem = MakePtr<GuiResourceItem>();
					tempResource->AddItem(L"Document", tempResourceItem);
					auto tempResolver = MakePtr<GuiResourcePathResolver>(tempResource, L"");

					auto document = DocumentModel::LoadFromXml(tempResourceItem, xml, tempResolver, errors);
					return document;
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

				documentData = value;
				ModifyDocumentForClipboard(documentData);
			}

			void WindowsClipboardWriter::Submit()
			{
				if (service->reader)
				{
					service->reader->CloseClipboard();
				}

				CHECK_ERROR(::OpenClipboard(service->ownerHandle), L"WindowsClipboardWriter::Submit()#Failed to open the clipboard.");
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
					stream::MemoryStream memoryStream;
					{
						stream::StreamWriter streamWriter(memoryStream);
						auto xml = documentData->SaveToXml();
						XmlPrint(xml, streamWriter);
					}
					memoryStream.SeekFromBegin(0);

					HGLOBAL data = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)memoryStream.Size());
					auto buffer = ::GlobalLock(data);
					memoryStream.Read(buffer, (vint)memoryStream.Size());
					::GlobalUnlock(data);
					::SetClipboardData(service->WCF_Document, data);
				}

				::CloseClipboard();
			}

/***********************************************************************
WindowsClipboardService
***********************************************************************/

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
					CHECK_ERROR(::OpenClipboard(ownerHandle), L"WindowsClipboardWriter::Submit()#Failed to open the clipboard.");
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