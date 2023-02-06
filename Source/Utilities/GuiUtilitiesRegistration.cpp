#include "FakeServices/GuiFakeClipboardService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Utilities Registration
***********************************************************************/

		FakeClipboardService* fakeClipboardService = nullptr;

		void GuiInitializeUtilities()
		{
			if (!fakeClipboardService)
			{
				fakeClipboardService = new FakeClipboardService;
				GetNativeServiceSubstitution()->Substitute(fakeClipboardService, true);
			}
		}

		void GuiFinalizeUtilities()
		{
			if (fakeClipboardService)
			{
				GetNativeServiceSubstitution()->Unsubstitute(fakeClipboardService);
				delete fakeClipboardService;
				fakeClipboardService = nullptr;
			}
		}
	}
}