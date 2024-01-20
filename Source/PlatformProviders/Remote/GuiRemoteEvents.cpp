#include "GuiRemoteController.h"

namespace vl::presentation
{

/***********************************************************************
GuiRemoteMessages
***********************************************************************/

	GuiRemoteMessages::GuiRemoteMessages(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	GuiRemoteMessages::~GuiRemoteMessages()
	{
	}

	void GuiRemoteMessages::Submit()
	{
		remote->remoteProtocol->Submit();
	}

	void GuiRemoteMessages::ClearResponses()
	{
#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			response ## NAME.Clear();
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	MESSAGE_NOREQ_RES(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES
	}

/***********************************************************************
GuiRemoteMessages (messages)
***********************************************************************/

#define MESSAGE_NOREQ_NORES(NAME)\
	void GuiRemoteMessages::Request ## NAME()\
	{\
		remote->remoteProtocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
	vint GuiRemoteMessages::Request ## NAME()\
	{\
		vint requestId = ++id;\
		remote->remoteProtocol->Request ## NAME(requestId);\
		return requestId;\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST)\
	void GuiRemoteMessages::Request ## NAME(const REQUEST& arguments)\
	{\
		remote->remoteProtocol->Request ## NAME(arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	vint GuiRemoteMessages::Request ## NAME(const REQUEST& arguments)\
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
	void GuiRemoteMessages::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		response ## NAME.Add(id, arguments);\
	}\
	const RESPONSE& GuiRemoteMessages::Retrieve ## NAME(vint id)\
	{\
		return response ## NAME[id];\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

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
GuiRemoteEvents (messages)
***********************************************************************/

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
	void GuiRemoteEvents::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		remote->remoteMessages.Respond ## NAME(id, arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

/***********************************************************************
GuiRemoteEvents (events)
***********************************************************************/

	void GuiRemoteEvents::OnControllerConnect()
	{
		remote->OnControllerConnect();
		remote->remoteMessages.RequestControllerConnectionEstablished();
		remote->remoteMessages.Submit();
	}

	void GuiRemoteEvents::OnControllerDisconnect()
	{
		remote->OnControllerDisconnect();
	}

	void GuiRemoteEvents::OnControllerExit()
	{
		remote->OnControllerExit();
	}

	void GuiRemoteEvents::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		remote->OnControllerScreenUpdated(arguments);
	}

	void GuiRemoteEvents::OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments)
	{
		remote->remoteWindow.OnWindowBoundsUpdated(arguments);
	}
}