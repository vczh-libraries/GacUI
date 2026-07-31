#ifndef VCZH_PRESENTATION_REMOTING_MINIHTTPAUTOMATIONSERVICE
#define VCZH_PRESENTATION_REMOTING_MINIHTTPAUTOMATIONSERVICE

#include "AutomationServiceHost.h"

namespace vl::presentation::remoting
{
	extern void StartMiniHttpAutomationService(
		Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer,
		const WString& applicationName
		);
	extern void StartMiniHttpAutomationService(
		Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer
		);
	extern void StopMiniHttpAutomationService();

	class MiniHttpAutomationServiceScope
	{
	public:
		MiniHttpAutomationServiceScope(
			Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer,
			const WString& applicationName
			);
		~MiniHttpAutomationServiceScope();

		MiniHttpAutomationServiceScope(const MiniHttpAutomationServiceScope&) = delete;
		MiniHttpAutomationServiceScope& operator=(const MiniHttpAutomationServiceScope&) = delete;
	};
}

#endif
