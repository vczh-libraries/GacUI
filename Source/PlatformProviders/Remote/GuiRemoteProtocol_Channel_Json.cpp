#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
GuiRemoteProtocolFromJsonChannel
***********************************************************************/

	void GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<glr::json::JsonNode>& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<JsonNode>&)#"
		auto jsonArray = package.Cast<glr::json::JsonArray>();
		CHECK_ERROR(jsonArray && jsonArray->items.Count() == 4, ERROR_MESSAGE_PREFIX L"A JSON array with 4 elements is expected.");

		auto jsonCategory = jsonArray->items[0].Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonCategory, ERROR_MESSAGE_PREFIX L"The first element should be a string.");

		auto jsonMessage = jsonArray->items[1].Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonMessage, ERROR_MESSAGE_PREFIX L"The second element should be a string.");

#define EVENT_NOREQ(NAME, REQUEST)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			events->On ## NAME();\
		} else\

#define EVENT_REQ(NAME, REQUEST)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArray->items[3], arguments);\
			events->On ## NAME(arguments);\
		} else\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		if (jsonCategory->content.value == L"Event")
		{
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized event name");
			}
		}
		else
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			auto jsonId = jsonArray->items[2].Cast<glr::json::JsonNumber>();\
			CHECK_ERROR(jsonId, ERROR_MESSAGE_PREFIX L"The third element should be a number.");\
			RESPONSE arguments;\
			ConvertJsonToCustomType(jsonArray->items[3], arguments);\
			events->Respond ## NAME(wtoi(jsonId->content.value), arguments);\
		} else\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		if (jsonCategory->content.value == L"Respond")
		{
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized response name");
			}
		} else
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized category name");
#undef ERROR_MESSAGE_PREFIX
	}

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME()\
	{\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonLiteral);\
		jsonId->value = glr::json::JsonLiteralValue::Null;\
		auto jsonRequest = Ptr(new glr::json::JsonLiteral);\
		jsonRequest->value = glr::json::JsonLiteralValue::Null;\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		channel->Write(jsonArray);\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id)\
	{\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonNumber);\
		jsonId->content.value = itow(id);\
		auto jsonRequest = Ptr(new glr::json::JsonLiteral);\
		jsonRequest->value = glr::json::JsonLiteralValue::Null;\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		channel->Write(jsonArray);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(const REQUEST& arguments)\
	{\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonLiteral);\
		jsonId->value = glr::json::JsonLiteralValue::Null;\
		auto jsonRequest = ConvertCustomTypeToJson(arguments);\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		channel->Write(jsonArray);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id, const REQUEST& arguments)\
	{\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonNumber);\
		jsonId->content.value = itow(id);\
		auto jsonRequest = ConvertCustomTypeToJson(arguments);\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		channel->Write(jsonArray);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	GuiRemoteProtocolFromJsonChannel::GuiRemoteProtocolFromJsonChannel(IJsonChannel* _channel)
		: channel(_channel)
	{
	}

	GuiRemoteProtocolFromJsonChannel::~GuiRemoteProtocolFromJsonChannel()
	{
	}

	void GuiRemoteProtocolFromJsonChannel::Initialize(IGuiRemoteProtocolEvents* _events)
	{
		events = _events;
		channel->Initialize(this);
	}

	WString GuiRemoteProtocolFromJsonChannel::GetExecutablePath()
	{
		return channel->GetExecutablePath();
	}
	
	void GuiRemoteProtocolFromJsonChannel::Submit()
	{
		channel->Submit();
	}

	void GuiRemoteProtocolFromJsonChannel::ProcessRemoteEvents()
	{
		channel->ProcessRemoteEvents();
	}

/***********************************************************************
GuiRemoteJsonChannelFromProtocol
***********************************************************************/

#define EVENT_NOREQ(NAME, REQUEST)\
	void GuiRemoteJsonChannelFromProtocol::On ## NAME()\
	{\
		auto jsonCategory = Ptr(new glr::json::JsonString);\
		jsonCategory->content.value = WString::Unmanaged(L"Event");\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonLiteral);\
		jsonId->value = glr::json::JsonLiteralValue::Null;\
		auto jsonRequest = Ptr(new glr::json::JsonLiteral);\
		jsonRequest->value = glr::json::JsonLiteralValue::Null;\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonCategory);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		receiver->OnReceive(jsonArray);\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteJsonChannelFromProtocol::On ## NAME(const REQUEST& arguments)\
	{\
		auto jsonCategory = Ptr(new glr::json::JsonString);\
		jsonCategory->content.value = WString::Unmanaged(L"Event");\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonLiteral);\
		jsonId->value = glr::json::JsonLiteralValue::Null;\
		auto jsonRequest = ConvertCustomTypeToJson(arguments);\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonCategory);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		receiver->OnReceive(jsonArray);\
	}\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)						EVENT_ ## REQTAG(NAME, REQUEST)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteJsonChannelFromProtocol::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		auto jsonCategory = Ptr(new glr::json::JsonString);\
		jsonCategory->content.value = WString::Unmanaged(L"Respond");\
		auto jsonName = Ptr(new glr::json::JsonString);\
		jsonName->content.value = WString::Unmanaged(L ## #NAME);\
		auto jsonId = Ptr(new glr::json::JsonNumber);\
		jsonId->content.value = itow(id);\
		auto jsonRequest = ConvertCustomTypeToJson(arguments);\
		auto jsonArray = Ptr(new glr::json::JsonArray);\
		jsonArray->items.Add(jsonCategory);\
		jsonArray->items.Add(jsonName);\
		jsonArray->items.Add(jsonId);\
		jsonArray->items.Add(jsonRequest);\
		receiver->OnReceive(jsonArray);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

	GuiRemoteJsonChannelFromProtocol::GuiRemoteJsonChannelFromProtocol(IGuiRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
	}

	GuiRemoteJsonChannelFromProtocol::~GuiRemoteJsonChannelFromProtocol()
	{
	}

	void GuiRemoteJsonChannelFromProtocol::Initialize(IJsonChannelReceiver* _receiver)
	{
		receiver = _receiver;
		protocol->Initialize(this);
	}

	IJsonChannelReceiver* GuiRemoteJsonChannelFromProtocol::GetReceiver()
	{
		return receiver;
	}

	void GuiRemoteJsonChannelFromProtocol::Write(const Ptr<glr::json::JsonNode>& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteJsonChannelFromProtocol::Write(const Ptr<JsonNode>&)#"
		auto jsonArray = package.Cast<glr::json::JsonArray>();
		CHECK_ERROR(jsonArray && jsonArray->items.Count() == 3, ERROR_MESSAGE_PREFIX L"A JSON array with 3 elements is expected.");

		auto jsonMessage = jsonArray->items[0].Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonMessage, ERROR_MESSAGE_PREFIX L"The first element should be a string.");

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			protocol->Request ## NAME();\
		} else\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			auto jsonId = jsonArray->items[1].Cast<glr::json::JsonNumber>();\
			CHECK_ERROR(jsonMessage, ERROR_MESSAGE_PREFIX L"The second element should be a number.");\
			protocol->Request ## NAME(wtoi(jsonId->content.value));\
		} else\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArray->items[2], arguments);\
			protocol->Request ## NAME(arguments);\
		} else\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
		if (jsonMessage->content.value == L ## #NAME)\
		{\
			auto jsonId = jsonArray->items[1].Cast<glr::json::JsonNumber>();\
			CHECK_ERROR(jsonMessage, ERROR_MESSAGE_PREFIX L"The second element should be a number.");\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArray->items[2], arguments);\
			protocol->Request ## NAME(wtoi(jsonId->content.value), arguments);\
		} else\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized request name");
		}
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

#undef ERROR_MESSAGE_PREFIX
	}

	WString GuiRemoteJsonChannelFromProtocol::GetExecutablePath()
	{
		return protocol->GetExecutablePath();
	}

	void GuiRemoteJsonChannelFromProtocol::Submit()
	{
		protocol->Submit();
	}

	void GuiRemoteJsonChannelFromProtocol::ProcessRemoteEvents()
	{
		protocol->ProcessRemoteEvents();
	}
}