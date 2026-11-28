#include "GuiRemoteController.h"
#include "../Hosted/GuiHostedController.h"
#include "../../Utilities/FakeServices/GuiFakeClipboardService.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::elements;

/***********************************************************************
SetupRemoteNativeController
***********************************************************************/

extern void GuiApplicationMain();

int SetupRemoteNativeControllerInternal(IGuiRemoteProtocol* protocol, const GuiRemoteControllerConfig& config, INativeClipboardService* clipboardService)
{
	FakeClipboardService fakeClipboard;
	GuiRemoteController remoteController(protocol, config, clipboardService ? clipboardService : &fakeClipboard);
	GuiHostedController hostedController(&remoteController);

	GuiRemoteGraphicsResourceManager remoteResourceManager(&remoteController, &hostedController);
	GuiHostedGraphicsResourceManager hostedResourceManager(&hostedController, &remoteResourceManager);

	SetNativeController(&hostedController);
	SetGuiGraphicsResourceManager(&hostedResourceManager);
	SetHostedApplication(hostedController.GetHostedApplication());

	remoteController.Initialize();
	remoteResourceManager.Initialize();
	hostedController.Initialize();
	GuiApplicationMain();
	hostedController.Finalize();
	remoteResourceManager.Finalize();
	remoteController.Finalize();

	SetHostedApplication(nullptr);
	SetGuiGraphicsResourceManager(nullptr);
	SetNativeController(nullptr);
	return 0;
}
