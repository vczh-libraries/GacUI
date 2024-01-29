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
#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)			response ## NAME.Clear();
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
	}

/***********************************************************************
GuiRemoteMessages (messages)
***********************************************************************/

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteMessages::Request ## NAME()\
	{\
		remote->remoteProtocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	vint GuiRemoteMessages::Request ## NAME()\
	{\
		vint requestId = ++id;\
		remote->remoteProtocol->Request ## NAME(requestId);\
		return requestId;\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
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

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteMessages::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		response ## NAME.Add(id, arguments);\
	}\
	const RESPONSE& GuiRemoteMessages::Retrieve ## NAME(vint id)\
	{\
		return response ## NAME[id];\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

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

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteEvents::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		remote->remoteMessages.Respond ## NAME(id, arguments);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

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

	void GuiRemoteEvents::OnControllerRequestExit()
	{
		remote->OnControllerRequestExit();
	}

	void GuiRemoteEvents::OnControllerForceExit()
	{
		remote->OnControllerForceExit();
	}

	void GuiRemoteEvents::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		remote->OnControllerScreenUpdated(arguments);
	}

	void GuiRemoteEvents::OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments)
	{
		remote->remoteWindow.OnWindowBoundsUpdated(arguments);
	}

	void GuiRemoteEvents::OnWindowActivatedUpdated(const bool& arguments)
	{
		remote->remoteWindow.OnWindowActivatedUpdated(arguments);
	}

	void GuiRemoteEvents::OnIOButtonDown(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOButtonClick(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOButtonDoubleClick(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOButtonUp(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOHWheel(const NativeWindowMouseInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOVWheel(const NativeWindowMouseInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOMouseMoving(const NativeWindowMouseInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOMouseEnter(const NativeWindowMouseInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOMouseLeave(const NativeWindowMouseInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOKeyDown(const NativeWindowKeyInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOKeyUp(const NativeWindowKeyInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteEvents::OnIOChar(const NativeWindowCharInfo& arguments)
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}