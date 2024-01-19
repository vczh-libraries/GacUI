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

#define GACUI_REMOTEPROTOCOL_MESSAGE_EMPTY_NOREQ_NORES(NAME)
#define GACUI_REMOTEPROTOCOL_MESSAGE_EMPTY_REQ_NORES(NAME, RESPONSE)
#define GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)\
			GACUI_REMOTEPROTOCOL_MESSAGES(\
				GACUI_REMOTEPROTOCOL_MESSAGE_EMPTY_NOREQ_NORES,\
				MESSAGE_NOREQ_RES,\
				GACUI_REMOTEPROTOCOL_MESSAGE_EMPTY_REQ_NORES,\
				MESSAGE_REQ_RES)\

namespace vl::presentation
{
/***********************************************************************
IGuiRemoteProtocolEvents
***********************************************************************/

	class IGuiRemoteProtocolEvents : public virtual Interface
	{
	public:
#define EVENT_NOREQ(NAME)			virtual void On ## NAME() = 0;
#define EVENT_REQ(NAME, REQUEST)	virtual void On ## NAME(const REQUEST& arguments) = 0;
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_NOREQ, EVENT_REQ)
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			virtual void Respond ## NAME(vint id, const RESPONSE& arguments) = 0;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	MESSAGE_NOREQ_RES(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES
	};

/***********************************************************************
IGuiRemoteProtocolMessages
***********************************************************************/

	class IGuiRemoteProtocolMessages : public virtual Interface
	{
	public:
#define MESSAGE_NOREQ_NORES(NAME)					virtual void Request ## NAME() = 0;
#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			virtual void Request ## NAME(vint id) = 0;
#define MESSAGE_REQ_NORES(NAME, REQUEST)			virtual void Request ## NAME(const REQUEST& arguments) = 0;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	virtual void Request ## NAME(vint id, const REQUEST& arguments) = 0;
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_NOREQ_NORES, MESSAGE_NOREQ_RES, MESSAGE_REQ_NORES, MESSAGE_REQ_RES)
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