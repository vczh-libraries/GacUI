#include "GuiRemoteController.h"
#include "../Hosted/GuiHostedController.h"

/***********************************************************************
SetupRemoteNativeController
***********************************************************************/

int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol)
{
	vl::presentation::GuiRemoteController();
	return 0;
}