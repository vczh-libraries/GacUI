/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNELING
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNELING

#include "GuiRemoteProtocol_FilterVerifier.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
IGuiRemoteProtocolChannel<T>
***********************************************************************/

	template<typename TPackage>
	class IGuiRemoteProtocolChannelReceiver : public virtual Interface
	{
	public:
		virtual void											OnReceive(const TPackage& package) = 0;
	};

	template<typename TPackage>
	class IGuiRemoteProtocolChannel : public virtual Interface
	{
	public:
		virtual void											Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* receiver) = 0;
		virtual IGuiRemoteProtocolChannelReceiver<TPackage>*	GetReceiver() = 0;
		virtual void											Write(const TPackage& package) = 0;
		virtual WString											GetExecutablePath() = 0;
		virtual void											Submit() = 0;
		virtual void											ProcessRemoteEvents() = 0;
	};

	using IJsonChannelReceiver = IGuiRemoteProtocolChannelReceiver<Ptr<glr::json::JsonNode>>;
	using IJsonChannel = IGuiRemoteProtocolChannel<Ptr<glr::json::JsonNode>>;

/***********************************************************************
GuiRemoteProtocolFromJsonChannel
***********************************************************************/
	
	class GuiRemoteProtocolFromJsonChannel
		: public Object
		, public virtual IGuiRemoteProtocol
		, protected IJsonChannelReceiver
	{
	protected:
		IJsonChannel*				channel = nullptr;
		IGuiRemoteProtocolEvents*	events = nullptr;

		void OnReceive(const Ptr<glr::json::JsonNode>& package) override
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
				CHECK_ERROR(jsonMessage, ERROR_MESSAGE_PREFIX L"The third element should be a number.");\
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
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												vint lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP

	public:

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
		void Request ## NAME() override\
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
		void Request ## NAME(vint id) override\
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
		void Request ## NAME(const REQUEST& arguments) override\
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
		void Request ## NAME(vint id, const REQUEST& arguments) override\
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

		GuiRemoteProtocolFromJsonChannel(IJsonChannel* _channel)
			: channel(_channel)
		{
		}

		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			events = _events;
			channel->Initialize(this);
		}

		WString GetExecutablePath() override
		{
			return channel->GetExecutablePath();
		}
	
		void Submit() override
		{
			channel->Submit();
		}

		void ProcessRemoteEvents() override
		{
			channel->ProcessRemoteEvents();
		}
	};

/***********************************************************************
GuiRemoteJsonChannelFromProtocol
***********************************************************************/

	class GuiRemoteJsonChannelFromProtocol
		: public Object
		, public virtual IJsonChannel
		, protected virtual IGuiRemoteProtocolEvents
	{
	protected:
		IJsonChannelReceiver*		receiver = nullptr;
		IGuiRemoteProtocol*			protocol = nullptr;

#define EVENT_NOREQ(NAME, REQUEST)\
		void On ## NAME() override\
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
		void On ## NAME(const REQUEST& arguments) override\
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
		void Respond ## NAME(vint id, const RESPONSE& arguments) override\
		{\
			auto jsonCategory = Ptr(new glr::json::JsonString);\
			jsonCategory->content.value = WString::Unmanaged(L"Respond");\
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

#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
	public:

		GuiRemoteJsonChannelFromProtocol(IGuiRemoteProtocol* _protocol)
			: protocol(_protocol)
		{
		}

		void Initialize(IJsonChannelReceiver* _receiver) override
		{
			receiver = receiver;
			protocol->Initialize(this);
		}

		IJsonChannelReceiver* GetReceiver() override
		{
			return receiver;
		}

		void Write(const Ptr<glr::json::JsonNode>& package) override
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

		WString GetExecutablePath() override
		{
			return protocol->GetExecutablePath();
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
}

#endif