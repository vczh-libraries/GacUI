/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCLIPBOARDSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSCLIPBOARDSERVICE

#include "..\..\GuiNativeWindow.h"
#include <windows.h>

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsClipboardService;

			class WindowsClipboardReader : public Object, public INativeClipboardReader
			{
				friend class WindowsClipboardService;
			protected:
				WindowsClipboardService*		service;

			public:
				WindowsClipboardReader(WindowsClipboardService* _service);
				~WindowsClipboardReader();

				bool							ContainsText()override;
				WString							GetText()override;

				void							CloseClipboard();
			};

			class WindowsClipboardWriter : public Object, public INativeClipboardWriter
			{
				friend class WindowsClipboardService;
			protected:
				WindowsClipboardService*		service;
				Nullable<WString>				textData;

			public:
				WindowsClipboardWriter(WindowsClipboardService* _service);
				~WindowsClipboardWriter();

				void							SetText(const WString& value)override;
				void							Submit()override;
			};

			class WindowsClipboardService : public Object, public INativeClipboardService
			{
				friend class WindowsClipboardReader;
				friend class WindowsClipboardWriter;
			protected:
				HWND							ownerHandle;
				WindowsClipboardReader*			reader = nullptr;

			public:
				WindowsClipboardService();

				Ptr<INativeClipboardReader>		ReadClipboard()override;
				Ptr<INativeClipboardWriter>		WriteClipboard()override;

				void							SetOwnerHandle(HWND handle);
			};
		}
	}
}

#endif