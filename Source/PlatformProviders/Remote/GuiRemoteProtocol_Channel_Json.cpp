#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{
/***********************************************************************
Metadata
***********************************************************************/

	void JsonChannelPack(ProtocolSemantic semantic, vint id, const WString& name, Ptr<glr::json::JsonNode> arguments, Ptr<glr::json::JsonObject>& package)
	{
		package = Ptr(new glr::json::JsonObject);

		{
			auto value = Ptr(new glr::json::JsonString);
			switch (semantic)
			{
			case ProtocolSemantic::Request:
				value->content.value = WString::Unmanaged(L"Request");
				break;
			case ProtocolSemantic::Response:
				value->content.value = WString::Unmanaged(L"Response");
				break;
			case ProtocolSemantic::Event:
				value->content.value = WString::Unmanaged(L"Event");
				break;
			default:
				value->content.value = WString::Unmanaged(L"Unknown");
			}

			auto field = Ptr(new glr::json::JsonObjectField);
			field->name.value = WString::Unmanaged(L"semantic");
			field->value = value;
			package->fields.Add(field);
		}

		if (id != -1)
		{
			auto value = Ptr(new glr::json::JsonNumber);
			value->content.value = itow(id);

			auto field = Ptr(new glr::json::JsonObjectField);
			field->name.value = WString::Unmanaged(L"id");
			field->value = value;
			package->fields.Add(field);
		}

		{
			auto value = Ptr(new glr::json::JsonString);
			value->content.value = name;

			auto field = Ptr(new glr::json::JsonObjectField);
			field->name.value = WString::Unmanaged(L"name");
			field->value = value;
			package->fields.Add(field);
		}

		if (arguments)
		{
			auto field = Ptr(new glr::json::JsonObjectField);
			field->name.value = WString::Unmanaged(L"arguments");
			field->value = arguments;
			package->fields.Add(field);
		}
	}

	void JsonChannelUnpack(Ptr<glr::json::JsonObject> package, ProtocolSemantic& semantic, vint& id, WString& name, Ptr<glr::json::JsonNode>& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::JsonChannelPack(Ptr<JsonObject>, ProtocolSemantic&, vint&, WString&, Ptr<JsonNode>&)#"

		for (auto&& field : package->fields)
		{
			if (field->name.value == L"semantic")
			{
				auto value = field->value.Cast<glr::json::JsonString>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The semantic field should be a string.");

				if (value->content.value == L"Request")
				{
					semantic = ProtocolSemantic::Request;
				}
				else if (value->content.value == L"Response")
				{
					semantic = ProtocolSemantic::Response;
				}
				else if (value->content.value == L"Event")
				{
					semantic = ProtocolSemantic::Event;
				}
			}
			else if (field->name.value == L"id")
			{
				auto value = field->value.Cast<glr::json::JsonNumber>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The id field should be a number.");

				id = wtoi(value->content.value);
			}
			else if (field->name.value == L"name")
			{
				auto value = field->value.Cast<glr::json::JsonString>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The name field should be a string.");

				name = value->content.value;
			}
			else if (field->name.value == L"arguments")
			{
				arguments = field->value;
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
GuiRemoteProtocolFromJsonChannel
***********************************************************************/

	void GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<glr::json::JsonObject>& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<JsonNode>&)#"

		ProtocolSemantic semantic = ProtocolSemantic::Unknown;
		vint id = -1;
		WString name;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, semantic, id, name, jsonArguments);

#define EVENT_NOREQ(NAME, REQUEST)\
		if (name == L ## #NAME)\
		{\
			events->On ## NAME();\
		} else\

#define EVENT_REQ(NAME, REQUEST)\
		if (name == L ## #NAME)\
		{\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArguments, arguments);\
			events->On ## NAME(arguments);\
		} else\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		if (semantic == ProtocolSemantic::Event)
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
		if (name == L ## #NAME)\
		{\
			RESPONSE arguments;\
			ConvertJsonToCustomType(jsonArguments, arguments);\
			events->Respond ## NAME(id, arguments);\
		} else\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		if (semantic == ProtocolSemantic::Response)
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
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Request, -1, WString::Unmanaged(L ## #NAME), {}, package);\
		channel->Write(package);\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id)\
	{\
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Request, id, WString::Unmanaged(L ## #NAME), {}, package);\
		channel->Write(package);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Request, -1, WString::Unmanaged(L ## #NAME), ConvertCustomTypeToJson(arguments), package);\
		channel->Write(package);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id, const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Request, id, WString::Unmanaged(L ## #NAME), ConvertCustomTypeToJson(arguments), package);\
		channel->Write(package);\
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
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Event, -1, WString::Unmanaged(L ## #NAME), {}, package);\
		receiver->OnReceive(package);\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteJsonChannelFromProtocol::On ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Event, -1, WString::Unmanaged(L ## #NAME), ConvertCustomTypeToJson(arguments), package);\
		receiver->OnReceive(package);\
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
		Ptr<glr::json::JsonObject> package;\
		JsonChannelPack(ProtocolSemantic::Response, id, WString::Unmanaged(L ## #NAME), ConvertCustomTypeToJson(arguments), package);\
		receiver->OnReceive(package);\
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

	void GuiRemoteJsonChannelFromProtocol::Write(const Ptr<glr::json::JsonObject>& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteJsonChannelFromProtocol::Write(const Ptr<JsonNode>&)#"

		ProtocolSemantic semantic = ProtocolSemantic::Unknown;
		vint id = -1;
		WString name;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, semantic, id, name, jsonArguments);

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
		if (name == L ## #NAME)\
		{\
			protocol->Request ## NAME();\
		} else\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
		if (name == L ## #NAME)\
		{\
			protocol->Request ## NAME(id);\
		} else\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
		if (name == L ## #NAME)\
		{\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArguments, arguments);\
			protocol->Request ## NAME(arguments);\
		} else\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
		if (name == L ## #NAME)\
		{\
			REQUEST arguments;\
			ConvertJsonToCustomType(jsonArguments, arguments);\
			protocol->Request ## NAME(id, arguments);\
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

/***********************************************************************
JsonToStringSerializer
***********************************************************************/

	void JsonToStringSerializer::Serialize(Ptr<glr::json::Parser> parser, const SourceType& source, DestType& dest)
	{
		glr::json::JsonFormatting formatting;
		formatting.spaceAfterColon = false;
		formatting.spaceAfterComma = false;
		formatting.crlf = false;
		formatting.compact = true;
		dest = glr::json::JsonToString(source, formatting);
	}

	void JsonToStringSerializer::Deserialize(Ptr<glr::json::Parser> parser, const DestType& source, SourceType& dest)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteJsonChannelFromProtocol::Write(const Ptr<JsonNode>&)#"
		auto value = glr::json::JsonParse(source, *parser.Obj());
		dest = value.Cast<glr::json::JsonObject>();
		CHECK_ERROR(dest, ERROR_MESSAGE_PREFIX L"JSON parssing between the channel should be JsonObject.");
#undef ERROR_MESSAGE_PREFIX
	}
}