#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;

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
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::JsonChannelUnpack(Ptr<JsonObject>, ChannelPackageInfo&, Ptr<JsonNode>&)#"
		CHECK_ERROR(package, ERROR_MESSAGE_PREFIX L"The package should be a JsonObject.");

		info = {};
		arguments = {};

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

		CHECK_ERROR(info.semantic != ChannelPackageSemantic::Unknown, ERROR_MESSAGE_PREFIX L"The semantic field is missing or invalid.");
		CHECK_ERROR(info.name.Length() > 0, ERROR_MESSAGE_PREFIX L"The name field is missing.");
#undef ERROR_MESSAGE_PREFIX
	}

	void JsonChannelUnpack(Ptr<glr::json::JsonNode> package, ChannelPackageInfo& info, Ptr<glr::json::JsonNode>& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::JsonChannelUnpack(Ptr<JsonNode>, ChannelPackageInfo&, Ptr<JsonNode>&)#"
		auto object = package.Cast<glr::json::JsonObject>();
		CHECK_ERROR(object, ERROR_MESSAGE_PREFIX L"The package should be a JsonObject.");
		JsonChannelUnpack(object, info, arguments);
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
GuiRemoteProtocolChannelClient
***********************************************************************/

	GuiRemoteProtocolChannelClient::GuiRemoteProtocolChannelClient(Ptr<inter_process::INetworkProtocolClient> client, Ptr<glr::json::Parser> parser)
		: Base(client, parser)
	{
		channelNames.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName), nullptr);
	}

	const IJsonChannelClient::ChannelNameList& GuiRemoteProtocolChannelClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	IJsonChannel* GuiRemoteProtocolChannelClient::GetProtocolChannel()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolChannelClient::GetProtocolChannel()#"
		auto&& channels = GetChannels();
		auto index = channels.Keys().IndexOf(WString::Unmanaged(GacUIRemoteProtocolChannelName));
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"The protocol channel is missing.");
		return channels.Values()[index];
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
GuiRemoteProtocolLocalChannelClient
***********************************************************************/

	GuiRemoteProtocolLocalChannelClient::GuiRemoteProtocolLocalChannelClient(Ptr<glr::json::Parser> parser)
		: Base(parser)
	{
		channelNames.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName), nullptr);
	}

	const IJsonChannelClient::ChannelNameList& GuiRemoteProtocolLocalChannelClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	IJsonChannel* GuiRemoteProtocolLocalChannelClient::GetProtocolChannel()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolLocalChannelClient::GetProtocolChannel()#"
		auto&& channels = GetChannels();
		auto index = channels.Keys().IndexOf(WString::Unmanaged(GacUIRemoteProtocolChannelName));
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"The protocol channel is missing.");
		return channels.Values()[index];
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
GuiRemoteProtocolCoreChannel
***********************************************************************/

#define EVENT_NOREQ(NAME, REQUEST)\
	void GuiRemoteProtocolCoreChannel::OnRead_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		events->On ## NAME();\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteProtocolCoreChannel::OnRead_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments)\
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
	void GuiRemoteProtocolCoreChannel::OnRead_Response_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments)\
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

	void GuiRemoteProtocolCoreChannel::Write(Ptr<glr::json::JsonObject> package)
	{
		auto receiverClientId = GetRendererClientId();
		if (receiverClientId == -1)
		{
			SPIN_LOCK(lockPackagesBeforeRenderer)
			{
				packagesBeforeRenderer.Add(package);
			}
		}
		else
		{
			List<JsonPackage> packages;
			SPIN_LOCK(lockPackagesBeforeRenderer)
			{
				packages = std::move(packagesBeforeRenderer);
			}

			for (auto&& pendingPackage : packages)
			{
				channel->SendToClient(receiverClientId, pendingPackage);
			}
			channel->SendToClient(receiverClientId, package);
		}
	}

	void GuiRemoteProtocolCoreChannel::SetRendererClientId(vint clientId)
	{
		rendererClientId.store(clientId);
	}

	vint GuiRemoteProtocolCoreChannel::GetRendererClientId()
	{
		return rendererClientId.load();
	}

	void GuiRemoteProtocolCoreChannel::OnRead(vint senderClientId, const JsonPackage& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolCoreChannel::OnRead(vint, const JsonPackage&)#"
		CHECK_ERROR(events, ERROR_MESSAGE_PREFIX L"The protocol event receiver is not initialized.");

		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);

		if (info.semantic == ChannelPackageSemantic::Event)
		{
			if (info.name == L"ControllerConnect")
			{
				SetRendererClientId(senderClientId);
			}
			else if (info.name == L"ControllerDisconnect" && GetRendererClientId() == senderClientId)
			{
				SetRendererClientId(-1);
			}
			else if (GetRendererClientId() != senderClientId)
			{
				return;
			}

			BeforeOnRead(info);
			vint index = onReadEventHandlers.Keys().IndexOf(info.name);
			if (index == -1)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized event name.");
			}
			else
			{
				(this->*onReadEventHandlers.Values()[index])(jsonArguments);
			}
		}
		else if (info.semantic == ChannelPackageSemantic::Response)
		{
			if (GetRendererClientId() != senderClientId)
			{
				return;
			}

			BeforeOnRead(info);
			vint index = onReadResponseHandlers.Keys().IndexOf(info.name);
			if (index == -1)
			{
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized response name.");
			}
			else
			{
				(this->*onReadResponseHandlers.Values()[index])(info.id, jsonArguments);
			}
		}
		else
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Only events and responses are expected.");
		}
#undef ERROR_MESSAGE_PREFIX
	}

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolCoreChannel::Request ## NAME()\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Message, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolCoreChannel::Request ## NAME(vint id)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Request, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolCoreChannel::Request ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Message, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolCoreChannel::Request ## NAME(vint id, const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Request, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	GuiRemoteProtocolCoreChannel::GuiRemoteProtocolCoreChannel(IJsonChannelClient* _client, IJsonChannel* _channel, const WString& _executablePath, IGuiRemoteEventProcessor* _eventProcessor)
		: client(_client)
		, channel(_channel)
		, eventProcessor(_eventProcessor)
		, executablePath(_executablePath)
	{
#define EVENT_NOREQ(NAME, REQUEST)					onReadEventHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolCoreChannel::OnRead_Event_ ## NAME);
#define EVENT_REQ(NAME, REQUEST)					onReadEventHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolCoreChannel::OnRead_Event_ ## NAME);
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)										onReadResponseHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolCoreChannel::OnRead_Response_ ## NAME);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		channel->Initialize(this);
	}

	GuiRemoteProtocolCoreChannel::~GuiRemoteProtocolCoreChannel()
	{
	}

	void GuiRemoteProtocolCoreChannel::Initialize(IGuiRemoteProtocolEvents* _events)
	{
		events = _events;
	}

	WString GuiRemoteProtocolCoreChannel::GetExecutablePath()
	{
		return executablePath;
	}
	
	void GuiRemoteProtocolCoreChannel::Submit(bool& disconnected)
	{
		auto receiverClientId = GetRendererClientId();
		List<JsonPackage> packages;
		SPIN_LOCK(lockPackagesBeforeRenderer)
		{
			packages = std::move(packagesBeforeRenderer);
		}

		if (receiverClientId == -1)
		{
			disconnected = false;
			return;
		}

		for (auto&& package : packages)
		{
			channel->SendToClient(receiverClientId, package);
		}
		channel->BatchWrite(disconnected);
	}

	IGuiRemoteEventProcessor* GuiRemoteProtocolCoreChannel::GetRemoteEventProcessor()
	{
		return eventProcessor;
	}

	void GuiRemoteProtocolCoreChannel::DetachRenderer(vint clientId)
	{
		if (GetRendererClientId() == clientId)
		{
			SetRendererClientId(-1);
		}
	}

/***********************************************************************
GuiRemoteProtocolRendererChannel
***********************************************************************/

#define EVENT_NOREQ(NAME, REQUEST)\
	void GuiRemoteProtocolRendererChannel::On ## NAME()\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Event, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, {}, package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define EVENT_REQ(NAME, REQUEST)\
	void GuiRemoteProtocolRendererChannel::On ## NAME(const REQUEST& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Event, -1, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)						EVENT_ ## REQTAG(NAME, REQUEST)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
	void GuiRemoteProtocolRendererChannel::Respond ## NAME(vint id, const RESPONSE& arguments)\
	{\
		Ptr<glr::json::JsonObject> package;\
		ChannelPackageInfo info{ChannelPackageSemantic::Response, id, WString::Unmanaged(L ## #NAME)};\
		JsonChannelPack(info, ConvertCustomTypeToJson(arguments), package);\
		BeforeWrite(info);\
		Write(package);\
	}\

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolRendererChannel::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		protocol->Request ## NAME();\
	}\

#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolRendererChannel::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		protocol->Request ## NAME(id);\
	}\

#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolRendererChannel::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
	{\
		REQUEST arguments;\
		ConvertJsonToCustomType(jsonArguments, arguments);\
		protocol->Request ## NAME(arguments);\
	}\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
	void GuiRemoteProtocolRendererChannel::Write_ ## NAME(vint id, Ptr<glr::json::JsonNode> jsonArguments)\
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

	void GuiRemoteProtocolRendererChannel::Write(Ptr<glr::json::JsonObject> package)
	{
		channel->SendToClient(GacUIRemoteProtocolCoreClientId, package);

		if (!receiving)
		{
			bool disconnected = false;
			Flush(disconnected);
		}
	}

	void GuiRemoteProtocolRendererChannel::Flush(bool& disconnected)
	{
		channel->BatchWrite(disconnected);
	}

	void GuiRemoteProtocolRendererChannel::OnRead(vint senderClientId, const JsonPackage& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolRendererChannel::OnRead(vint, const JsonPackage&)#"
		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);
		BeforeOnRead(info);

		if (info.semantic != ChannelPackageSemantic::Message && info.semantic != ChannelPackageSemantic::Request)
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Only messages and requests are expected.");
		}

		vint index = writeHandlers.Keys().IndexOf(info.name);
		if (index == -1)
		{
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized request name.");
		}
		else
		{
			receiving = true;
			(this->*writeHandlers.Values()[index])(info.id, jsonArguments);
			receiving = false;

			bool disconnected = false;
			Flush(disconnected);
		}
#undef ERROR_MESSAGE_PREFIX
	}

	GuiRemoteProtocolRendererChannel::GuiRemoteProtocolRendererChannel(IJsonChannel* _channel, IGuiRemoteProtocol* _protocol)
		: channel(_channel)
		, protocol(_protocol)
	{
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolRendererChannel::Write_ ## NAME);
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolRendererChannel::Write_ ## NAME);
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolRendererChannel::Write_ ## NAME);
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						writeHandlers.Add(WString::Unmanaged(L ## #NAME), &GuiRemoteProtocolRendererChannel::Write_ ## NAME);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

		channel->Initialize(this);
		protocol->Initialize(this);
	}

	GuiRemoteProtocolRendererChannel::~GuiRemoteProtocolRendererChannel()
	{
	}
}
