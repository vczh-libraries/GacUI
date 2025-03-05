#include "GuiRemoteProtocol_FilterVerifier.h"

namespace vl::presentation::remoteprotocol::repeatfiltering
{
/***********************************************************************
GuiRemoteEventFilterVerifier
***********************************************************************/

	GuiRemoteEventFilterVerifier::GuiRemoteEventFilterVerifier()
	{
	}

	GuiRemoteEventFilterVerifier::~GuiRemoteEventFilterVerifier()
	{
	}

	void GuiRemoteEventFilterVerifier::ClearDropRepeatMasks()
	{
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)									lastDropRepeatEvent ## NAME = false;
#define EVENT_DROPCON(NAME)
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP
	}

	void GuiRemoteEventFilterVerifier::ClearDropConsecutiveMasks()
	{
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)
#define EVENT_DROPCON(NAME)									lastDropConsecutiveEvent ## NAME = false;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP
	}

	// responses

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteEventFilterVerifier::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		targetEvents->Respond ## NAME(id, arguments);\
	}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

	// events
	
#define EVENT_NODROP(NAME)
	
#define EVENT_DROPREP(NAME)\
		CHECK_ERROR(!lastDropRepeatEvent ## NAME, L"vl::presentation::remoteprotocol::GuiRemoteEventFilterVerifier::On" L ## #NAME L"(...)#[@DropRepeat] event repeated.");\
		lastDropRepeatEvent ## NAME = true;\
	
#define EVENT_DROPCON(NAME)\
		CHECK_ERROR(!lastDropConsecutiveEvent ## NAME, L"vl::presentation::remoteprotocol::GuiRemoteEventFilterVerifier::On" L ## #NAME L"(...)#[@DropConsecutive] event repeated.");\
		ClearDropConsecutiveMasks();\
		lastDropConsecutiveEvent ## NAME = true;\
	
#define EVENT_NOREQ(NAME, REQUEST, DROPTAG)\
	void GuiRemoteEventFilterVerifier::On ## NAME()\
	{\
		if (submitting)\
		{\
			EVENT_ ## DROPTAG(NAME);\
			targetEvents->On ## NAME();\
		}\
		else\
		{\
			targetEvents->On ## NAME();\
		}\
	}\
	
#define EVENT_REQ(NAME, REQUEST, DROPTAG)\
	void GuiRemoteEventFilterVerifier::On ## NAME(const REQUEST& arguments)\
	{\
		if (submitting)\
		{\
			EVENT_ ## DROPTAG(NAME);\
			targetEvents->On ## NAME(arguments);\
		}\
		else\
		{\
			targetEvents->On ## NAME(arguments);\
		}\
	}\
	
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST, DROPTAG)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NOREP

/***********************************************************************
GuiRemoteProtocolFilterVerifier
***********************************************************************/

	void GuiRemoteProtocolFilterVerifier::ClearDropRepeatMasks()
	{
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												lastDropRepeatRequest ## NAME = false;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	}

	GuiRemoteProtocolFilterVerifier::GuiRemoteProtocolFilterVerifier(IGuiRemoteProtocol* _protocol)
		: GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>(_protocol)
	{
	}

	GuiRemoteProtocolFilterVerifier::~GuiRemoteProtocolFilterVerifier()
	{
	}
	
	// messages
	
#define MESSAGE_NODROP(NAME)
	
#define MESSAGE_DROPREP(NAME)\
		CHECK_ERROR(!lastDropRepeatRequest ## NAME, L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilterVerifier::Request" L ## #NAME L"(...)#[@DropRepeat] message repeated.");\
		lastDropRepeatRequest ## NAME = true;\
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilterVerifier::Request ## NAME()\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		targetProtocol->Request ## NAME();\
	}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilterVerifier::Request ## NAME(vint id)\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		targetProtocol->Request ## NAME(id);\
	}\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilterVerifier::Request ## NAME(const REQUEST& arguments)\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		targetProtocol->Request ## NAME(arguments);\
	}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilterVerifier::Request ## NAME(vint id, const REQUEST& arguments)\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		targetProtocol->Request ## NAME(id, arguments);\
	}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE, DROPTAG)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
	// protocol
	
	void GuiRemoteProtocolFilterVerifier::Submit(bool& disconnected)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::repeatfiltering::GuiRemoteProtocolFilterVerifier::Submit()#"
		CHECK_ERROR(!eventCombinator.submitting, ERROR_MESSAGE_PREFIX L"This function is not allowed to be called recursively.");
		eventCombinator.submitting = true;
		GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>::Submit(disconnected);
		ClearDropRepeatMasks();
		eventCombinator.ClearDropRepeatMasks();
		eventCombinator.ClearDropConsecutiveMasks();
		eventCombinator.submitting = false;
#undef ERROR_MESSAGE_PREFIX
	}
}