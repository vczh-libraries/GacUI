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

struct BatchedRequest
{
	vint			id = -1;
	const wchar_t*	name = nullptr;
	WString			arguments;
};

class BatchedProtocol
	: public Object
	, public virtual IGuiRemoteProtocol
	, protected virtual IGuiRemoteProtocolEvents
{
protected:
	glr::json::Parser						jsonParser;
	IGuiRemoteProtocol*						protocol = nullptr;
	IGuiRemoteProtocolEvents*				events = nullptr;
	vint									lastRequestId = -1;
	collections::List<BatchedRequest>		batchedRequests;
	collections::Dictionary<vint, WString>	batchedRequestIds;

	template<typename T>
	WString ToJson(const T& value)
	{
		return stream::GenerateToStream([&](stream::TextWriter& writer)
		{
			auto node = Ptr(new glr::json::JsonArray);
			node->items.Add(ConvertCustomTypeToJson<T>(value));
			JsonPrint(node, writer);
		});
	}

	template<typename T>
	void FromJson(const WString& json, T& value)
	{
		auto node = JsonParse(json, jsonParser).Cast<glr::json::JsonArray>();
		ConvertJsonToCustomType<T>(node->items[0], value);
	}
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
		REQUEST deserialized;\
		FromJson<REQUEST>(ToJson<REQUEST>(arguments), deserialized);\
		events->On ## NAME(deserialized);\
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
		CHECK_ERROR(batchedRequestIds[id] == L ## #NAME, L"Messages sending to IGuiRemoteProtocol should be responded by calling the correct function.");\
		batchedRequestIds.Remove(id);\
		RESPONSE deserialized;\
		FromJson<RESPONSE>(ToJson<RESPONSE>(arguments), deserialized);\
		events->Respond ## NAME(id, deserialized);\
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
		BatchedRequest request;\
		request.name = L ## #NAME;\
		batchedRequests.Add(request);\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(vint id) override\
	{\
		CHECK_ERROR(lastRequestId < id, L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
		lastRequestId = id;\
		BatchedRequest request;\
		request.id = id;\
		request.name = L ## #NAME;\
		batchedRequests.Add(request);\
		batchedRequestIds.Add(id, request.name);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(const REQUEST& arguments) override\
	{\
		BatchedRequest request;\
		request.name = L ## #NAME;\
		request.arguments = ToJson<REQUEST>(arguments);\
		batchedRequests.Add(request);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void Request ## NAME(vint id, const REQUEST& arguments) override\
	{\
		CHECK_ERROR(lastRequestId < id, L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
		lastRequestId = id;\
		BatchedRequest request;\
		request.id = id;\
		request.name = L ## #NAME;\
		request.arguments = ToJson<REQUEST>(arguments);\
		batchedRequests.Add(request);\
		batchedRequestIds.Add(id, request.name);\
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
		for (auto&& request : batchedRequests)
		{
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
			if (request.name == L ## #NAME)\
			{\
				protocol->Request ## NAME();\
			} else\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
			if (request.name == L ## #NAME)\
			{\
				protocol->Request ## NAME(request.id);\
			} else\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
			if (request.name == L ## #NAME)\
			{\
				REQUEST arguments;\
				FromJson<REQUEST>(request.arguments, arguments);\
				protocol->Request ## NAME(arguments);\
			} else\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
			if (request.name == L ## #NAME)\
			{\
				REQUEST arguments;\
				FromJson<REQUEST>(request.arguments, arguments);\
				protocol->Request ## NAME(request.id, arguments);\
			} else\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
			CHECK_FAIL(L"Unrecognized request!");
		}

		CHECK_ERROR(batchedRequestIds.Count() == 0, L"Messages sending to IGuiRemoteProtocol should be all responded.");
		batchedRequests.Clear();
	}

	void ProcessRemoteEvents() override
	{
		protocol->ProcessRemoteEvents();
	}
};