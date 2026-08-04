#ifndef VCZH_PRESENTATION_REMOTING_MINIHTTPAUTOMATIONSERVICE
#define VCZH_PRESENTATION_REMOTING_MINIHTTPAUTOMATIONSERVICE

#include "../../../Source/GacUI.h"
#include <VlppOS.h>

namespace vl::presentation::remoting
{
	extern void StartMiniHttpAutomationService(
		Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer,
		const WString& applicationName
		);
	extern void StopMiniHttpAutomationService();
}

#endif
