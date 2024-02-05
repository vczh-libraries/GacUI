#include "TestRemote_Shared_FilterProtocol.h"
#include "TestRemote_Shared_JsonProtocol.h"
#include "TestRemote_Shared_NoRenderingProtocol.h"
#include "TestRemote_Shared_LoggingWindowListener.h"

#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

using namespace remote_protocol_tests;

extern void SetGuiMainProxy(const Func<void()>& proxy);