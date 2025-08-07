#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{
/***********************************************************************
ChannelPackageSemantic
***********************************************************************/

	void JsonChannelPack(const ChannelPackageInfo& info, Ptr<glr::json::JsonNode> arguments, Ptr<glr::json::JsonObject>& package)
	{
		package = Ptr(new glr::json::JsonObject);

		{
			auto value = Ptr(new glr::json::JsonString);
			switch (info.semantic)
			{
			case ChannelPackageSemantic::Message:
				value->content.value = WString::Unmanaged(L"Message");
				break;
			case ChannelPackageSemantic::Request:
				value->content.value = WString::Unmanaged(L"Request");
				break;
			case ChannelPackageSemantic::Response:
				value->content.value = WString::Unmanaged(L"Response");
				break;
			case ChannelPackageSemantic::Event:
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

		if (info.id != -1)
		{
			auto value = Ptr(new glr::json::JsonNumber);
			value->content.value = itow(info.id);

			auto field = Ptr(new glr::json::JsonObjectField);
			field->name.value = WString::Unmanaged(L"id");
			field->value = value;
			package->fields.Add(field);
		}

		{
			auto value = Ptr(new glr::json::JsonString);
			value->content.value = info.name;

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

	void JsonChannelUnpack(Ptr<glr::json::JsonObject> package, ChannelPackageInfo& info, Ptr<glr::json::JsonNode>& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::JsonChannelPack(Ptr<JsonObject>, ProtocolSemantic&, vint&, WString&, Ptr<JsonNode>&)#"

		for (auto&& field : package->fields)
		{
			if (field->name.value == L"semantic")
			{
				auto value = field->value.Cast<glr::json::JsonString>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The semantic field should be a string.");

				if (value->content.value == L"Message")
				{
					info.semantic = ChannelPackageSemantic::Message;
				}
				else if (value->content.value == L"Request")
				{
					info.semantic = ChannelPackageSemantic::Request;
				}
				else if (value->content.value == L"Response")
				{
					info.semantic = ChannelPackageSemantic::Response;
				}
				else if (value->content.value == L"Event")
				{
					info.semantic = ChannelPackageSemantic::Event;
				}
			}
			else if (field->name.value == L"id")
			{
				auto value = field->value.Cast<glr::json::JsonNumber>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The id field should be a number.");

				info.id = wtoi(value->content.value);
			}
			else if (field->name.value == L"name")
			{
				auto value = field->value.Cast<glr::json::JsonString>();
				CHECK_ERROR(value, ERROR_MESSAGE_PREFIX L"The name field should be a string.");

				info.name = value->content.value;
			}
			else if (field->name.value == L"arguments")
			{
				arguments = field->value;
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}

	void ChannelPackageSemanticUnpack(Ptr<glr::json::JsonObject> package, ChannelPackageInfo& info)
	{
		Ptr<glr::json::JsonNode> arguments;
		JsonChannelUnpack(package, info, arguments);
	}

/***********************************************************************
GuiRemoteProtocolFromJsonChannel
***********************************************************************/

#define EVENT_NOREQ(NAME, REQUEST)\
	void GuiRemoteProtocolFromJsonChannel::OnReceive_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		events->On ## NAME();\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteProtocolFromJsonChannel::OnReceive_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		REQUEST arguments;\
		ConvertJsonToCustomType(jsonArguments, arguments);\
		events->On ## NAME(arguments);\
	}\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)

#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::OnReceive_Response_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		RESPONSE arguments;\
		ConvertJsonToCustomType(jsonArguments, arguments);\
		events->Respond ## NAME(id, arguments);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

	void GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<glr::json::JsonObject>& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolFromJsonChannel::OnReceive(const Ptr<JsonNode>&)#"

		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);
		BeforeOnReceive(info);

		if (info.semantic == ChannelPackageSemantic::Event)
		{
			vint index = onReceiveEventHandlers.Keys().IndexOf(info.name);
			if (index == -1)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized event name");
			}
			else
			{
				(this->*onReceiveEventHandlers.Values()[index])(jsonArguments);
			}
		}
		else if (info.semantic == ChannelPackageSemantic::Response)
		{
			vint index = onReceiveResponseHandlers.Keys().IndexOf(info.name);
			if (index == -1)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized response name");
			}
			else
			{
				(this->*onReceiveResponseHandlers.Values()[index])(info.id, jsonArguments);
			}
		}
		else
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized category name");
		}

#undef ERROR_MESSAGE_PREFIX
	}

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME()\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Message, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeWrite(info);\
		channel->Write(package);\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Request, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeWrite(info);\
		channel->Write(package);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Message, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
		channel->Write(package);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolFromJsonChannel::Request ## NAME(vint id, const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Request, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
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
#define EVENT_NOREQ(NAME, REQUEST)					onReceiveEventHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolFromJsonChannel::OnReceive_Event_ ## NAME);
#define EVENT_REQ(NAME, REQUEST)					onReceiveEventHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolFromJsonChannel::OnReceive_Event_ ## NAME);
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)										onReceiveResponseHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolFromJsonChannel::OnReceive_Response_ ## NAME);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
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
	
	void GuiRemoteProtocolFromJsonChannel::Submit(bool& disconnected)
	{
		channel->Submit(disconnected);
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
		ChannelPackageInfo info{ChannelPackageSemantic::Event, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeOnReceive(info);\
		receiver->OnReceive(package);\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteJsonChannelFromProtocol::On ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Event, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeOnReceive(info);\
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
		ChannelPackageInfo info{ChannelPackageSemantic::Response, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeOnReceive(info);\
		receiver->OnReceive(package);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteJsonChannelFromProtocol::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		protocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteJsonChannelFromProtocol::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		protocol->Request ## NAME(id);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteJsonChannelFromProtocol::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		REQUEST arguments;\
		ConvertJsonToCustomType(jsonArguments, arguments);\
		protocol->Request ## NAME(arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteJsonChannelFromProtocol::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		REQUEST arguments;\
		ConvertJsonToCustomType(jsonArguments, arguments);\
		protocol->Request ## NAME(id, arguments);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	GuiRemoteJsonChannelFromProtocol::GuiRemoteJsonChannelFromProtocol(IGuiRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteJsonChannelFromProtocol::Write_ ## NAME);;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteJsonChannelFromProtocol::Write_ ## NAME);;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteJsonChannelFromProtocol::Write_ ## NAME);;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteJsonChannelFromProtocol::Write_ ## NAME);;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
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

		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);
		BeforeWrite(info);

		vint index = writeHandlers.Keys().IndexOf(info.name);
		if (index == -1)
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized request name");
		}
		else
		{
			(this->*writeHandlers.Values()[index])(info.id, jsonArguments);
		}

#undef ERROR_MESSAGE_PREFIX
	}

	WString GuiRemoteJsonChannelFromProtocol::GetExecutablePath()
	{
		return protocol->GetExecutablePath();
	}

	void GuiRemoteJsonChannelFromProtocol::Submit(bool& disconnected)
	{
		protocol->Submit(disconnected);
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