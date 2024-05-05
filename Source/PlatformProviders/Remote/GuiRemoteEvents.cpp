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
	RESPONSE GuiRemoteMessages::Retrieve ## NAME(vint id)\
	{\
		RESPONSE response = response ## NAME[id];\
		response ## NAME.Remove(id);\
		return response;\
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
		remote->remoteMessages.RequestControllerConnectionEstablished();
		remote->OnControllerConnect();
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

	void GuiRemoteEvents::OnIOGlobalShortcutKey(const vint& arguments)
	{
		remote->callbackService.InvokeGlobalShortcutKeyActivated(arguments);
	}

	void GuiRemoteEvents::OnIOButtonDown(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		switch (arguments.button)
		{
		case remoteprotocol::IOMouseButton::Left:
			for (auto l : remote->remoteWindow.listeners) l->LeftButtonDown(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Middle:
			for (auto l : remote->remoteWindow.listeners) l->MiddleButtonDown(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Right:
			for (auto l : remote->remoteWindow.listeners) l->RightButtonDown(arguments.info);
			break;
		default:
			CHECK_FAIL(L"vl::presentation::GuiRemoteEvents::OnIOButtonDown(const IOMouseInfoWithButton&)#Unrecognized button.");
		}
	}

	void GuiRemoteEvents::OnIOButtonDoubleClick(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		switch (arguments.button)
		{
		case remoteprotocol::IOMouseButton::Left:
			for (auto l : remote->remoteWindow.listeners) l->LeftButtonDoubleClick(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Middle:
			for (auto l : remote->remoteWindow.listeners) l->MiddleButtonDoubleClick(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Right:
			for (auto l : remote->remoteWindow.listeners) l->RightButtonDoubleClick(arguments.info);
			break;
		default:
			CHECK_FAIL(L"vl::presentation::GuiRemoteEvents::OnIOButtonDoubleClick(const IOMouseInfoWithButton&)#Unrecognized button.");
		}
	}

	void GuiRemoteEvents::OnIOButtonUp(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		switch (arguments.button)
		{
		case remoteprotocol::IOMouseButton::Left:
			for (auto l : remote->remoteWindow.listeners) l->LeftButtonUp(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Middle:
			for (auto l : remote->remoteWindow.listeners) l->MiddleButtonUp(arguments.info);
			break;
		case remoteprotocol::IOMouseButton::Right:
			for (auto l : remote->remoteWindow.listeners) l->RightButtonUp(arguments.info);
			break;
		default:
			CHECK_FAIL(L"vl::presentation::GuiRemoteEvents::OnIOButtonUp(const IOMouseInfoWithButton&)#Unrecognized button.");
		}
	}

	void GuiRemoteEvents::OnIOHWheel(const NativeWindowMouseInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->HorizontalWheel(arguments);
	}

	void GuiRemoteEvents::OnIOVWheel(const NativeWindowMouseInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->VerticalWheel(arguments);
	}

	void GuiRemoteEvents::OnIOMouseMoving(const NativeWindowMouseInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseMoving(arguments);
	}

	void GuiRemoteEvents::OnIOMouseEntered()
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseEntered();
	}

	void GuiRemoteEvents::OnIOMouseLeaved()
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseLeaved();
	}

	void GuiRemoteEvents::OnIOKeyDown(const NativeWindowKeyInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->KeyDown(arguments);
	}

	void GuiRemoteEvents::OnIOKeyUp(const NativeWindowKeyInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->KeyUp(arguments);
	}

	void GuiRemoteEvents::OnIOChar(const NativeWindowCharInfo& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->Char(arguments);
	}
}