#include "FakeServices/GuiFakeClipboardService.h"
#include "FakeServices/Dialogs/GuiFakeDialogService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Utilities Registration
***********************************************************************/

		FakeClipboardService* fakeClipboardService = nullptr;
		FakeDialogService* fakeDialogService = nullptr;

		void GuiInitializeUtilities()
		{
			if (!fakeClipboardService)
			{
				fakeClipboardService = new FakeClipboardService;
				GetNativeServiceSubstitution()->Substitute(fakeClipboardService, true);
			}

			if (!fakeDialogService)
			{
				fakeDialogService = new FakeDialogService;
				GetNativeServiceSubstitution()->Substitute(fakeDialogService, true);
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

			if (fakeDialogService)
			{
				GetNativeServiceSubstitution()->Unsubstitute(fakeDialogService);
				delete fakeDialogService;
				fakeDialogService = nullptr;
			}
		}
	}
}