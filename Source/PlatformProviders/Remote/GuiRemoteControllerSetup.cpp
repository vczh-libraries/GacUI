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
	GuiRemoteGraphicsResourceManager remoteResourceManager(&remoteController);

	GuiHostedController hostedController(&remoteController);
	GuiHostedGraphicsResourceManager hostedResourceManager(&hostedController, &remoteResourceManager);

	SetNativeController(&hostedController);
	SetGuiGraphicsResourceManager(&hostedResourceManager);

	remoteController.Initialize();
	hostedController.Initialize();
	GuiApplicationMain();
	hostedController.Finalize();
	remoteController.Finalize();

	SetGuiGraphicsResourceManager(nullptr);
	SetNativeController(nullptr);
	return 0;
}