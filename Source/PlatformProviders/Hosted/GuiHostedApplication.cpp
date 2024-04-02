#include "GuiHostedApplication.h"

namespace vl::presentation
{
	IGuiHostedApplication* hostedApplication = nullptr;

	IGuiHostedApplication* GetHostedApplication()
	{
		return hostedApplication;
	}

	void SetHostedApplication(IGuiHostedApplication* _hostedApp)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::SetHostedApplication(IGuiHostedApplication*)#"
		if (_hostedApp)
		{
			CHECK_ERROR(!hostedApplication, ERROR_MESSAGE_PREFIX L"IGuiHostedApplication instance already exists during initializing.");
		}
		else
		{
			CHECK_ERROR(hostedApplication, ERROR_MESSAGE_PREFIX L"IGuiHostedApplication instance does not exist during finalizing.");
		}
		hostedApplication = _hostedApp;
#undef ERROR_MESSAGE_PREFIX
	}
}