#include "TuiApplication.h"

namespace vl::presentation
{
	ITuiApplication* tuiApplication = nullptr;

	ITuiApplication* GetTuiApplication()
	{
		return tuiApplication;
	}

	void SetTuiApplication(ITuiApplication* application)
	{
		tuiApplication = application;
	}
}
