#include "GuiRemoteController.h"

namespace vl::presentation
{
/***********************************************************************
GuiRemoteEvents
***********************************************************************/

	GuiRemoteEvents::GuiRemoteEvents(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	GuiRemoteEvents::~GuiRemoteEvents()
	{
	}

/***********************************************************************
GuiRemoteEvents (message)
***********************************************************************/

	void GuiRemoteEvents::RespondGetFontConfig(vint id, const remoteprotocol::FontConfig& arguments)
	{
	}

	void GuiRemoteEvents::RespondGetScreenConfig(vint id, const remoteprotocol::ScreenConfig& arguments)
	{
	}

/***********************************************************************
GuiRemoteEvents (controlling)
***********************************************************************/

	void GuiRemoteEvents::OnConnect()
	{
	}

	void GuiRemoteEvents::OnDisconnect()
	{
	}

	void GuiRemoteEvents::OnExit()
	{
	}
/***********************************************************************
GuiRemoteEvents (system)
***********************************************************************/

	void GuiRemoteEvents::OnScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
	}
}