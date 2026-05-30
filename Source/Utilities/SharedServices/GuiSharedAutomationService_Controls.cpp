#include "GuiSharedAutomationService_Controls.h"
#include "../../PlatformProviders/Hosted/GuiHostedApplication.h"
#include "../../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
AutomationService
***********************************************************************/

		WString AutomationService::DumpControlTreeInternal()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		AutomationService::AutomationService()
		{
		}

		AutomationService::~AutomationService()
		{
		}

		bool AutomationService::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
AutomationServiceHosted
***********************************************************************/

		WString AutomationServiceHosted::DumpControlTreeInternal()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		AutomationServiceHosted::AutomationServiceHosted()
		{
		}

		AutomationServiceHosted::~AutomationServiceHosted()
		{
		}

		bool AutomationServiceHosted::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
RemoteProtocolAutomationService
***********************************************************************/

		Nullable<WString> RemoteProtocolAutomationService::GetNativeWindowId(INativeWindow* window)
		{
			return {};
		}

		INativeWindow* RemoteProtocolAutomationService::GetNativeWindow(Nullable<WString> windowId)
		{
			return GetHostedApplication()->GetNativeWindowHost();
		}

		WString RemoteProtocolAutomationService::RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand)
		{
			auto window = dynamic_cast<GuiRemoteWindow*>(this->GetNativeWindow(windowId));
			if (!window)
			{
				return L"!Invalid window.";
			}

			return RunIOCommandOnNativeWindow(GetHostedApplication()->GetNativeController(), window->listeners, ioCommand);
		}

		RemoteProtocolAutomationService::RemoteProtocolAutomationService()
		{
			windowManagement = WString::Unmanaged(L"HostedRemoteProtocol");
		}

		RemoteProtocolAutomationService::~RemoteProtocolAutomationService()
		{
		}

		bool RemoteProtocolAutomationService::CanRunIOCommands()
		{
			return true;
		}

/***********************************************************************
DumpWindowClientArea
***********************************************************************/

		Ptr<glr::json::JsonNode> DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}