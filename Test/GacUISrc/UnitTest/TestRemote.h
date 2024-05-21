#include "TestRemote_Shared_JsonProtocol.h"
#include "TestRemote_Shared_NoRenderingProtocol.h"
#include "TestRemote_Shared_RenderingProtocol.h"
#include "TestRemote_Shared_LoggingWindowListener.h"

using namespace remote_protocol_tests;

extern void SetGuiMainProxy(const Func<void()>& proxy);

template<typename TProtocol>
void StartRemoteControllerTest(TProtocol& protocol)
{
	JsonProtocol jsonProtocol(&protocol);
	repeatfiltering::GuiRemoteProtocolFilterVerifier verifierProtocol(&jsonProtocol);
	repeatfiltering::GuiRemoteProtocolFilter filteredProtocol(&verifierProtocol);
	SetupRemoteNativeController(&filteredProtocol);
	SetGuiMainProxy({});
}

template<typename TProtocol>
	requires(std::is_base_of_v<UnitTestRemoteProtocolBase, TProtocol>)
void StartRemoteControllerTest(TProtocol& protocol)
{
	StartRemoteControllerTest(*protocol.GetProtocol());
}