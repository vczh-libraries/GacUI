#include "WindowsClipboardService.h"
#include "../../../Resources/GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{

/***********************************************************************
WindowsClipboardReader
***********************************************************************/

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
				UINT format = 0;
				while (format = ::EnumClipboardFormats(format))
				{
					if (format == CF_UNICODETEXT)
					{
						return true;
					}
				}
				return false;
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
				return false;
			}

			Ptr<DocumentModel> WindowsClipboardReader::GetDocument()
			{
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

				::CloseClipboard();
			}

/***********************************************************************
WindowsClipboardService
***********************************************************************/

			WindowsClipboardService::WindowsClipboardService()
				:ownerHandle(NULL)
			{
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