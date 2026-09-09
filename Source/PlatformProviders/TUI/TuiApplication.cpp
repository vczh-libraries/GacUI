#include "TuiApplication.h"

namespace vl::presentation
{
	TuiConfiguration::TuiConfiguration()
		: tabInterval(4)
	{
	}

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
