/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSRESOURCESERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSRESOURCESERVICE

#include "..\..\GuiNativeWindow.h"
#include "..\WinNativeDpiAwareness.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsCursor : public Object, public INativeCursor
			{
			protected:
				HCURSOR										handle;
				bool										isSystemCursor;
				SystemCursorType							systemCursorType;
			public:
				WindowsCursor(HCURSOR _handle);
				WindowsCursor(SystemCursorType type);

				bool										IsSystemCursor()override;
				SystemCursorType							GetSystemCursorType()override;
				HCURSOR										GetCursorHandle();
			};

			class WindowsResourceService : public Object, public INativeResourceService
			{
			protected:
				collections::Array<Ptr<WindowsCursor>>		systemCursors;
				FontProperties								defaultFont;
			public:
				WindowsResourceService();

				INativeCursor*								GetSystemCursor(INativeCursor::SystemCursorType type)override;
				INativeCursor*								GetDefaultSystemCursor()override;
				FontProperties								GetDefaultFont()override;
				void										SetDefaultFont(const FontProperties& value)override;
			};
		}
	}
}

#endif