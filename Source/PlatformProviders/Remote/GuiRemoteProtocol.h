/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL

#include "GuiRemoteProtocol_Shared.h"
#include "GuiRemoteProtocol_FilterVerifier.h"
#include "GuiRemoteProtocol_Filter.h"
#include "GuiRemoteProtocol_DomDiff.h"
#include "GuiRemoteProtocol_Channel_Json.h"
#include "GuiRemoteProtocol_Channel_Async.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using GuiRemoteProtocolAsyncJsonChannelSerializer = GuiRemoteProtocolAsyncChannelSerializer<Ptr<glr::json::JsonObject>>;
}

#endif