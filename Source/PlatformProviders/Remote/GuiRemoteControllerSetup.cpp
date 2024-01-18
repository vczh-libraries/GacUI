#include "GuiRemoteController.h"
#include "../Hosted/GuiHostedController.h"

using namespace vl;
using namespace vl::presentation;

/***********************************************************************
SetupRemoteNativeController
***********************************************************************/

extern void GuiApplicationMain();

int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol)
{
	auto remoteController = new GuiRemoteController(protocol);
	auto hostedController = new GuiHostedController(remoteController);
	SetNativeController(hostedController);
	{
		// TODO: register element renderers;
		hostedController->Initialize();
		GuiApplicationMain();
		hostedController->Finalize();
	}
	SetNativeController(nullptr);
	delete hostedController;
	delete remoteController;
	return 0;
}