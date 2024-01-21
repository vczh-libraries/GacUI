/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL

#include "Protocol/Generated/GuiRemoteProtocolSchema.h"

namespace vl::presentation
{
/***********************************************************************
IGuiRemoteProtocolEvents
***********************************************************************/

	class IGuiRemoteProtocolEvents : public virtual Interface
	{
	public:
#define EVENT_NOREQ(NAME, REQUEST)					virtual void On ## NAME() = 0;
#define EVENT_REQ(NAME, REQUEST)					virtual void On ## NAME(const REQUEST& arguments) = 0;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)										virtual void Respond ## NAME(vint id, const RESPONSE& arguments) = 0;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
	};

/***********************************************************************
IGuiRemoteProtocolMessages
***********************************************************************/

	class IGuiRemoteProtocolMessages : public virtual Interface
	{
	public:
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					virtual void Request ## NAME() = 0;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						virtual void Request ## NAME(vint id) = 0;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						virtual void Request ## NAME(const REQUEST& arguments) = 0;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						virtual void Request ## NAME(vint id, const REQUEST& arguments) = 0;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
	};

/***********************************************************************
IGuiRemoteProtocolConfig
***********************************************************************/

	class IGuiRemoteProtocolConfig : public virtual Interface
	{
	public:
		virtual WString			GetExecutablePath() = 0;
	};

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

	class IGuiRemoteProtocol
		: public virtual IGuiRemoteProtocolConfig
		, public virtual IGuiRemoteProtocolMessages
	{
	public:
		virtual void			Initialize(IGuiRemoteProtocolEvents* events) = 0;
		virtual void			Submit() = 0;
	};
}

#endif