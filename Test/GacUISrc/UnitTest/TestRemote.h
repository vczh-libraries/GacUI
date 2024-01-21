#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

extern void SetGuiMainProxy(void(*proxy)());

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

class NotImplementedProtocolBase : public Object, public virtual IGuiRemoteProtocol
{
public:
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME()									override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id)							override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments)			override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments)	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	IGuiRemoteProtocolEvents* events = nullptr;

	void Initialize(IGuiRemoteProtocolEvents* _events)
	{
		events = _events;
	}

	void Submit() override
	{
	}

	void ProcessRemoteEvents() override
	{
		CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!");
	}
};

class BatchedProtocol
	: public Object
	, public virtual IGuiRemoteProtocol
	, protected virtual IGuiRemoteProtocolEvents
{
protected:
	IGuiRemoteProtocol*					protocol = nullptr;
	IGuiRemoteProtocolEvents*			events = nullptr;

public:
	BatchedProtocol(IGuiRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
	}

protected:

	// events

#define EVENT_NOREQ(NAME, REQUEST)\
	void On ## NAME() override\
	{\
		events->On ## NAME();\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void On ## NAME(const REQUEST& arguments) override\
	{\
		events->On ## NAME(arguments);\
	}\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void Respond ## NAME(vint id, const RESPONSE& arguments) override\
	{\
		events->Respond ## NAME(id, arguments);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

public:

	// messages

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME() override\
	{\
		protocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(vint id) override\
	{\
		protocol->Request ## NAME(id);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(const REQUEST& arguments) override\
	{\
		protocol->Request ## NAME(arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(vint id, const REQUEST& arguments) override\
	{\
		protocol->Request ## NAME(id, arguments);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	// protocol

	WString GetExecutablePath() override
	{
		return protocol->GetExecutablePath();
	}

	void Initialize(IGuiRemoteProtocolEvents* _events) override
	{
		events = _events;
		protocol->Initialize(this);
	}

	void Submit() override
	{
		protocol->Submit();
	}

	void ProcessRemoteEvents() override
	{
		protocol->ProcessRemoteEvents();
	}
};