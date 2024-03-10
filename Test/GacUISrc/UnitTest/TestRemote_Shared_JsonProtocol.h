#ifndef GACUISRC_REMOTE_BATCHEDPROTOCOL
#define GACUISRC_REMOTE_BATCHEDPROTOCOL

#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

namespace remote_protocol_tests
{
	class JsonProtocolShared
	{
	protected:
		glr::json::Parser		jsonParser;

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
	};

	class JsonEvent : public JsonProtocolShared, public GuiRemoteEventCombinator
	{
	public:
		// events
	
#define EVENT_NOREQ(NAME, REQUEST)\
		void On ## NAME() override\
		{\
			targetEvents->On ## NAME();\
		}\
	
#define EVENT_REQ(NAME, REQUEST)\
		void On ## NAME(const REQUEST& arguments) override\
		{\
			REQUEST deserialized;\
			auto json = ToJson<REQUEST>(arguments);\
			FromJson<REQUEST>(json, deserialized);\
			targetEvents->On ## NAME(deserialized);\
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
			RESPONSE deserialized;\
			auto json = ToJson<RESPONSE>(arguments);\
			FromJson<RESPONSE>(json, deserialized);\
			targetEvents->Respond ## NAME(id, deserialized);\
		}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
	};

	class JsonProtocol : public JsonProtocolShared, public GuiRemoteProtocolCombinator<JsonEvent>
	{
	public:
		JsonProtocol(IGuiRemoteProtocol* _protocol)
			: GuiRemoteProtocolCombinator<JsonEvent>(_protocol)
		{
		}
	
		// messages
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
		void Request ## NAME() override\
		{\
			targetProtocol->Request ## NAME();\
		}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
		void Request ## NAME(vint id) override\
		{\
			targetProtocol->Request ## NAME(id);\
		}\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
		void Request ## NAME(const REQUEST& arguments) override\
		{\
			REQUEST deserialized;\
			auto json = ToJson<REQUEST>(arguments);\
			FromJson<REQUEST>(json, deserialized);\
			targetProtocol->Request ## NAME(deserialized);\
		}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
		void Request ## NAME(vint id, const REQUEST& arguments) override\
		{\
			REQUEST deserialized;\
			auto json = ToJson<REQUEST>(arguments);\
			FromJson<REQUEST>(json, deserialized);\
			targetProtocol->Request ## NAME(id, deserialized);\
		}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
	};
}

#endif