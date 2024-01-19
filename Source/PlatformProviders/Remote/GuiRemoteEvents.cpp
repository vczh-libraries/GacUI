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

#define MESSAGE_NOREQ_NORES(NAME)\
	void GuiRemoteEvents::Request ## NAME()\
	{\
		remote->remoteProtocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
	vint GuiRemoteEvents::Request ## NAME()\
	{\
		vint requestId = ++id;\
		remote->remoteProtocol->Request ## NAME(requestId);\
		return requestId;\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST)\
	void GuiRemoteEvents::Request ## NAME(const REQUEST& arguments)\
	{\
		remote->remoteProtocol->Request ## NAME(arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	vint GuiRemoteEvents::Request ## NAME(, const REQUEST& arguments)\
	{\
		vint requestId = ++id;\
		remote->remoteProtocol->Request ## NAME(requestId, arguments);\
		return requestId;\
	}\

	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_NOREQ_NORES, MESSAGE_NOREQ_RES, MESSAGE_REQ_NORES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
	void GuiRemoteEvents::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		response ## NAME.Add(id, arguments);\
	}\
	const RESPONSE& GuiRemoteEvents::Retrive ## NAME(vint id)\
	{\
		return response ## NAME[id];\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

	void GuiRemoteEvents::ClearResponses()
	{
#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			response ## NAME.Clear();
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	MESSAGE_NOREQ_RES(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES
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
		remote->remoteScreenConfig = arguments;
	}
}