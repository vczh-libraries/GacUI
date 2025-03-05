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

		GuiRemoteEventFilterVerifier();
		~GuiRemoteEventFilterVerifier();

		void													ClearDropRepeatMasks();
		void													ClearDropConsecutiveMasks();

		// responses

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)								void Respond ## NAME(vint id, const RESPONSE& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		// events
#define EVENT_NOREQ(NAME, REQUEST, DROPTAG)						void On ## NAME() override;
#define EVENT_REQ(NAME, REQUEST, DROPTAG)						void On ## NAME(const REQUEST& arguments) override;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST, DROPTAG)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
	};

/***********************************************************************
GuiRemoteProtocolFilterVerifier
***********************************************************************/

	class GuiRemoteProtocolFilter;
	
	class GuiRemoteProtocolFilterVerifier : public GuiRemoteProtocolCombinator<GuiRemoteEventFilterVerifier>
	{
		friend class GuiRemoteProtocolFilter;
	protected:
		vint																lastRequestId = -1;
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												bool lastDropRepeatRequest ## NAME = false;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
		void																ClearDropRepeatMasks();

	public:
		GuiRemoteProtocolFilterVerifier(IGuiRemoteProtocol* _protocol);
		~GuiRemoteProtocolFilterVerifier();
	
	public:
	
		// messages
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)				void Request ## NAME() override;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(vint id) override;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(const REQUEST& arguments) override;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(vint id, const REQUEST& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE, DROPTAG)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
	
		// protocol
	
		void																Submit(bool& disconnected) override;
	};
}

#endif