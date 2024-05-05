/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_FILTERVERIFIER
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_FILTERVERIFIER

#include "GuiRemoteProtocol_Shared.h"

namespace vl::presentation::remoteprotocol::repeatfiltering
{
/***********************************************************************
GuiRemoteEventFilterVerifier
***********************************************************************/

	class GuiRemoteEventFilterVerifier : public GuiRemoteEventCombinator
	{
	protected:
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)										bool lastDropRepeatEvent ## NAME = false;
#define EVENT_DROPCON(NAME)										bool lastDropConsecutiveEvent ## NAME = false;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)		EVENT_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP

	public:
		bool													submitting = false;

		void ClearDropRepeatMasks()
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

		void ClearDropConsecutiveMasks()
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
		void Respond ## NAME(vint id, const RESPONSE& arguments) override\
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
		void On ## NAME() override\
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
		void On ## NAME(const REQUEST& arguments) override\
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
	};

/***********************************************************************
GuiRemoteProtocolFilterVerifier
***********************************************************************/

	class GuiRemoteProtocolFilter;
	
	class GuiRemoteProtocolFilterVerifier : public GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>
	{
		friend class GuiRemoteProtocolFilter;
	protected:
		vint													lastRequestId = -1;
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												bool lastDropRepeatRequest ## NAME = false;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
		void ClearDropRepeatMasks()
		{
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												lastDropRepeatRequest ## NAME = false;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
		}
	public:
		GuiRemoteProtocolFilterVerifier(IGuiRemoteProtocol* _protocol)
			: GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>(_protocol)
		{
		}
	
	protected:
	
	public:
	
		// messages
	
#define MESSAGE_NODROP(NAME)
	
#define MESSAGE_DROPREP(NAME)\
			CHECK_ERROR(!lastDropRepeatRequest ## NAME, L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilterVerifier::Request" L ## #NAME L"(...)#[@DropRepeat] message repeated.");\
			lastDropRepeatRequest ## NAME = true;\
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME() override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			targetProtocol->Request ## NAME();\
		}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			targetProtocol->Request ## NAME(id);\
		}\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(const REQUEST& arguments) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			targetProtocol->Request ## NAME(arguments);\
		}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id, const REQUEST& arguments) override\
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
	
		void Submit() override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::repeatfiltering::GuiRemoteProtocolFilterVerifier::Submit()#"
			CHECK_ERROR(!eventCombinator.submitting, ERROR_MESSAGE_PREFIX L"This function is not allowed to be called recursively.");
			eventCombinator.submitting = true;
			GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>::Submit();
			ClearDropRepeatMasks();
			eventCombinator.ClearDropRepeatMasks();
			eventCombinator.ClearDropConsecutiveMasks();
			eventCombinator.submitting = false;
#undef ERROR_MESSAGE_PREFIX
		}
	};
}

#endif