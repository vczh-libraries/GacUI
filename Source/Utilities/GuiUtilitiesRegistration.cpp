#include "FakeServices/GuiFakeClipboardService.h"
#include "FakeServices/Dialogs/GuiFakeDialogService.h"
#include "FakeServices/TuiDialogs/TuiFakeDialogService.h"
#include "../PlatformProviders/TUI/TuiApplication.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Utilities Registration
***********************************************************************/

		FakeClipboardService* fakeClipboardService = nullptr;
		FakeDialogServiceBase* fakeDialogService = nullptr;

		void GuiInitializeUtilities()
		{
			GetNativeServiceSubstitution()->Substitute(INativeAutomationService::UnavailableService(), true);

			if (!fakeClipboardService)
			{
				fakeClipboardService = new FakeClipboardService;
				GetNativeServiceSubstitution()->Substitute(fakeClipboardService, true);
			}

			if (!fakeDialogService)
			{
				if (GetTuiApplication())
				{
					fakeDialogService = new FakeTuiDialogService;
				}
				else
				{
					fakeDialogService = new FakeDialogService;
				}
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
