/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_FAKECLIPBOARDSERVICE
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_FAKECLIPBOARDSERVICE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		class FakeClipboardService
			: public Object
			, public INativeClipboardService
		{
		public:
			FakeClipboardService();


			Ptr<INativeClipboardReader>		ReadClipboard() override;
			Ptr<INativeClipboardWriter>		WriteClipboard() override;
		};
	}
}

#endif