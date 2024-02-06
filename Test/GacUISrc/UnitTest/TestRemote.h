#include "TestRemote_Shared_FilterProtocol.h"
#include "TestRemote_Shared_JsonProtocol.h"
#include "TestRemote_Shared_NoRenderingProtocol.h"
#include "TestRemote_Shared_RenderingProtocol.h"
#include "TestRemote_Shared_LoggingWindowListener.h"

#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

using namespace remote_protocol_tests;

extern void SetGuiMainProxy(const Func<void()>& proxy);

template<typename TProtocol>
void StartRemoteControllerTest(TProtocol& protocol)
{
	JsonProtocol jsonProtocol(&protocol);
	GuiRemoteProtocolFilter filteredProtocol(&jsonProtocol);
	SetupRemoteNativeController(&filteredProtocol);
	SetGuiMainProxy({});
}