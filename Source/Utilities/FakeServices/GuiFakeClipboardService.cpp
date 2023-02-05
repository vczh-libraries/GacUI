#include "GuiFakeClipboardService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
FakeClipboardService
***********************************************************************/

		FakeClipboardService::FakeClipboardService()
		{
		}

		Ptr<INativeClipboardReader> FakeClipboardService::ReadClipboard()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		Ptr<INativeClipboardWriter> FakeClipboardService::WriteClipboard()
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}