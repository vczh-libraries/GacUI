#ifndef GACUISRC_REMOTE_BATCHEDPROTOCOL
#define GACUISRC_REMOTE_BATCHEDPROTOCOL

#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

namespace remote_protocol_tests
{
	struct BatchedRequest
	{
		bool			dropped = false;
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
		bool									submitting = false;
	
		vint									lastRequestId = -1;
		collections::Dictionary<vint, WString>	batchedRequestIds;
		collections::List<BatchedRequest>		batchedRequests;
		collections::List<BatchedRequest>		batchedResponses;
		collections::List<BatchedRequest>		batchedEvents;
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												vint lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)									vint lastDropRepeatEvent ## NAME = -1;
#define EVENT_DROPCON(NAME)									vint lastDropConsecutiveEvent ## NAME = -1;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP
	
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
	
		void ProcessRequests()
		{
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
			for (auto&& request : batchedRequests)
			{
				CHECK_ERROR(!request.dropped || request.id == -1, L"Messages with id cannot be dropped.");
				if (request.dropped)
				{
					continue;
				}
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					protocol->Request ## NAME();\
				} else\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					protocol->Request ## NAME(request.id);\
				} else\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					REQUEST arguments;\
					FromJson<REQUEST>(request.arguments, arguments);\
					protocol->Request ## NAME(arguments);\
				} else\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
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
	
		void ProcessResponses()
		{
			for (auto&& request : batchedResponses)
			{
#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					RESPONSE arguments;\
					FromJson<RESPONSE>(request.arguments, arguments);\
					events->Respond ## NAME(request.id, arguments);\
				} else\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
				GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
				CHECK_FAIL(L"Unrecognized response!");
			}
	
			batchedResponses.Clear();
		}
	
		void ProcessEvents()
		{
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)									lastDropRepeatEvent ## NAME = -1;
#define EVENT_DROPCON(NAME)									lastDropConsecutiveEvent ## NAME = -1;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP
	
			collections::List<BatchedRequest> requests;
			CopyFrom(requests, batchedEvents);
			batchedEvents.Clear();
	
			for (auto&& request : requests)
			{
				if (request.dropped)
				{
					continue;
				}
	
#define EVENT_NOREQ(NAME, REQUEST)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					events->On ## NAME();\
				} else\
	
#define EVENT_REQ(NAME, REQUEST)\
				if (wcscmp(request.name, L ## #NAME) == 0)\
				{\
					REQUEST arguments;\
					FromJson<REQUEST>(request.arguments, arguments);\
					events->On ## NAME(arguments);\
				} else\
	
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
				GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
				CHECK_FAIL(L"Unrecognized event!");
			}
		}
	public:
		BatchedProtocol(IGuiRemoteProtocol* _protocol)
			: protocol(_protocol)
		{
		}
	
	protected:
	
		// events
	
#define EVENT_NODROP(NAME)
	
#define EVENT_DROPREP(NAME)\
			if (lastDropRepeatEvent ## NAME != -1)\
			{\
				batchedEvents[lastDropRepeatEvent ## NAME].dropped = true;\
			}\
			lastDropRepeatEvent ## NAME = batchedEvents.Count() - 1\
	
#define EVENT_DROPCON(NAME)\
			if (lastDropConsecutiveEvent ## NAME != -1 && lastDropConsecutiveEvent ## NAME == batchedEvents.Count() - 1)\
			{\
				batchedEvents[lastDropConsecutiveEvent ## NAME].dropped = true;\
			}\
			lastDropConsecutiveEvent ## NAME = batchedEvents.Count() - 1\
	
#define EVENT_NOREQ(NAME, REQUEST, DROPTAG)\
		void On ## NAME() override\
		{\
			EVENT_ ## DROPTAG(NAME);\
			BatchedRequest request;\
			request.name = L ## #NAME;\
			batchedEvents.Add(request);\
			if (!submitting) ProcessEvents();\
		}\
	
#define EVENT_REQ(NAME, REQUEST, DROPTAG)\
		void On ## NAME(const REQUEST& arguments) override\
		{\
			EVENT_ ## DROPTAG(NAME);\
			BatchedRequest request;\
			request.name = L ## #NAME;\
			request.arguments = ToJson<REQUEST>(arguments);\
			batchedEvents.Add(request);\
			if (!submitting) ProcessEvents();\
		}\
	
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST, DROPTAG)
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
			BatchedRequest request;\
			request.id = id;\
			request.name = L ## #NAME;\
			request.arguments = ToJson<RESPONSE>(arguments);\
			batchedResponses.Add(request);\
		}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NOREP
	
	public:
	
		// messages
	
#define MESSAGE_NODROP(NAME)
	
#define MESSAGE_DROPREP(NAME)\
			if (lastDropRepeatRequest ## NAME != -1)\
			{\
				batchedRequests[lastDropRepeatRequest ## NAME].dropped = true;\
			}\
			lastDropRepeatRequest ## NAME = batchedRequests.Count()\
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME() override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			BatchedRequest request;\
			request.name = L ## #NAME;\
			batchedRequests.Add(request);\
		}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			CHECK_ERROR(lastRequestId < id, L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
			lastRequestId = id;\
			BatchedRequest request;\
			request.id = id;\
			request.name = L ## #NAME;\
			batchedRequests.Add(request);\
			batchedRequestIds.Add(id, request.name);\
		}\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(const REQUEST& arguments) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			BatchedRequest request;\
			request.name = L ## #NAME;\
			request.arguments = ToJson<REQUEST>(arguments);\
			batchedRequests.Add(request);\
		}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id, const REQUEST& arguments) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			CHECK_ERROR(lastRequestId < id, L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
			lastRequestId = id;\
			BatchedRequest request;\
			request.id = id;\
			request.name = L ## #NAME;\
			request.arguments = ToJson<REQUEST>(arguments);\
			batchedRequests.Add(request);\
			batchedRequestIds.Add(id, request.name);\
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
			submitting = true;
			protocol->Submit();
			ProcessRequests();
			ProcessResponses();
			submitting = false;
			ProcessEvents();
		}
	
		void ProcessRemoteEvents() override
		{
			protocol->ProcessRemoteEvents();
		}
	};
}

#endif