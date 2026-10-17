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

	void GuiRemoteMessages::Submit(bool& disconnected)
	{
		remote->remoteProtocol->Submit(disconnected);
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

	void GuiRemoteEvents::OnControllerConnect(const remoteprotocol::ControllerGlobalConfig& arguments)
	{
		remote->remoteMessages.RequestControllerConnectionEstablished();
		remote->OnControllerConnect(arguments);
		bool disconnected = false;
		remote->remoteMessages.Submit(disconnected);
		// there is no result from this request, assuming succeeded
		// if disconnected, OnControllerDisconnect will be called
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


	void GuiRemoteEvents::OnIOPointStarted(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointStarted(arguments);
	}

	void GuiRemoteEvents::OnIOPointDown(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointAction(NativePointEvent::Down, arguments);
	}

	void GuiRemoteEvents::OnIOPointUpdate(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointAction(NativePointEvent::Update, arguments);
	}

	void GuiRemoteEvents::OnIOPointUp(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointAction(NativePointEvent::Up, arguments);
	}

	void GuiRemoteEvents::OnIOPointEnter(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointAction(NativePointEvent::Entered, arguments);
	}

	void GuiRemoteEvents::OnIOPointLeave(const remoteprotocol::IOPointAction& arguments)
	{
		remote->remoteWindow.OnPointAction(NativePointEvent::Leaved, arguments);
	}

	void GuiRemoteEvents::OnIOPointEnded(const remoteprotocol::IOPointEnd& arguments)
	{
		remote->remoteWindow.OnPointEnded(arguments);
	}

	void GuiRemoteEvents::OnIOButtonDown(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseDown(arguments.button, arguments.info);
	}

	void GuiRemoteEvents::OnIOButtonDoubleClick(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseDoubleClick(arguments.button, arguments.info);
	}

	void GuiRemoteEvents::OnIOButtonUp(const remoteprotocol::IOMouseInfoWithButton& arguments)
	{
		for (auto l : remote->remoteWindow.listeners) l->MouseUp(arguments.button, arguments.info);
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

namespace vl::presentation::remoteprotocol
{
	vuint64_t DecodePointUnsigned(const WString& text)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::DecodePointUnsigned#"
		CHECK_ERROR(text.Length() > 0 && (text.Length() == 1 || text[0] != L'0'), ERROR_MESSAGE_PREFIX L"Expected canonical unsigned decimal.");
		vuint64_t value = 0;
		for (vint i = 0; i < text.Length(); i++)
		{
			auto c = text[i];
			CHECK_ERROR(c >= L'0' && c <= L'9', ERROR_MESSAGE_PREFIX L"Expected canonical unsigned decimal.");
			auto digit = (vuint64_t)(c - L'0');
			CHECK_ERROR(value <= ((vuint64_t)-1 - digit) / 10, ERROR_MESSAGE_PREFIX L"Unsigned decimal overflow.");
			value = value * 10 + digit;
		}
		return value;
#undef ERROR_MESSAGE_PREFIX
	}

	IOPointInfo EncodePointInfo(const NativeWindowPointInfo& info)
	{
		IOPointInfo result;
		result.sequenceId = u64tow(info.sequenceId);
		result.pointId = info.pointId;
		result.timestamp = u64tow(info.timestamp);
		result.activePointCount = info.activePointCount;
		result.position = info.position;
		result.primary = info.primary;
		result.ctrl = info.ctrl;
		result.shift = info.shift;
		result.alt = info.alt;
		result.osSuper = info.osSuper;
		result.contactBounds = info.contactBounds;
		result.pressure = info.pressure;
		result.orientation = info.orientation;
		if (info.platformFrameId) result.platformFrameId = u64tow(info.platformFrameId.Value());
		return result;
	}

	NativeWindowPointInfo DecodePointInfo(const IOPointInfo& info)
	{
		NativeWindowPointInfo result;
		result.sequenceId = DecodePointUnsigned(info.sequenceId);
		result.pointId = info.pointId;
		result.timestamp = DecodePointUnsigned(info.timestamp);
		result.activePointCount = info.activePointCount;
		result.position = info.position;
		result.primary = info.primary;
		result.ctrl = info.ctrl;
		result.shift = info.shift;
		result.alt = info.alt;
		result.osSuper = info.osSuper;
		result.contactBounds = info.contactBounds;
		result.pressure = info.pressure;
		result.orientation = info.orientation;
		if (info.platformFrameId) result.platformFrameId = DecodePointUnsigned(info.platformFrameId.Value());
		return result;
	}
}
