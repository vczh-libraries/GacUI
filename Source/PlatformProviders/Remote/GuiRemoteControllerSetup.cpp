#include "GuiRemoteController.h"
#include "../Hosted/GuiHostedController.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::elements;

/***********************************************************************
SetupRemoteNativeController
***********************************************************************/

extern void GuiApplicationMain();

int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol)
{
	GuiRemoteController remoteController(protocol);
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