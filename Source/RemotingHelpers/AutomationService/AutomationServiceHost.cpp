#include "AutomationServiceHost.h"

namespace vl::presentation::remoting
{
	NativeAutomationServiceScope::NativeAutomationServiceScope(INativeAutomationService* service)
		: automationService(service)
	{
		CHECK_ERROR(automationService, L"vl::presentation::remoting::NativeAutomationServiceScope::NativeAutomationServiceScope(INativeAutomationService*)#The automation service is null.");
		GetNativeServiceSubstitution()->Substitute(automationService, false);
	}

	NativeAutomationServiceScope::~NativeAutomationServiceScope()
	{
		automationService->Stop();
		GetNativeServiceSubstitution()->Unsubstitute(automationService);
	}
}
