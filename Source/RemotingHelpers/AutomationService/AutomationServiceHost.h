#ifndef VCZH_PRESENTATION_REMOTING_AUTOMATIONSERVICEHOST
#define VCZH_PRESENTATION_REMOTING_AUTOMATIONSERVICEHOST

#include "../../GacUI.h"
#include <VlppOS.h>

namespace vl::presentation::remoting
{
	enum class RemotingAutomationService
	{
		WindowsHttp,
		MiniHttp,
	};

	struct RemotingHostContext
	{
		RemotingAutomationService								automationService;
		Ptr<inter_process::async_tcp_socket::IAsyncSocketServer>	miniHttpSocketServer;
	};

	class NativeAutomationServiceScope
	{
	private:
		INativeAutomationService*								automationService = nullptr;

	public:
		NativeAutomationServiceScope(INativeAutomationService* service);
		~NativeAutomationServiceScope();

		NativeAutomationServiceScope(const NativeAutomationServiceScope&) = delete;
		NativeAutomationServiceScope& operator=(const NativeAutomationServiceScope&) = delete;
	};
}

#endif
