/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocolChannel<T>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_JSON
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_JSON

#include "GuiRemoteProtocol_Channel_Async.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using IJsonChannelReceiver = IGuiRemoteProtocolChannelReceiver<Ptr<glr::json::JsonObject>>;
	using IJsonChannel = IGuiRemoteProtocolChannel<Ptr<glr::json::JsonObject>>;

/***********************************************************************
ChannelPackageSemantic
***********************************************************************/

	extern void				ChannelPackageSemanticUnpack(Ptr<glr::json::JsonObject> package, ChannelPackageInfo& info);

/***********************************************************************
GuiRemoteProtocolFromJsonChannel
***********************************************************************/
	
	class GuiRemoteProtocolFromJsonChannel
		: public Object
		, public virtual IGuiRemoteProtocol
		, protected IJsonChannelReceiver
	{
	protected:
		IJsonChannel*					channel = nullptr;
		IGuiRemoteProtocolEvents*		events = nullptr;

		using OnReceiveEventHandler = void (GuiRemoteProtocolFromJsonChannel::*)(Ptr<glr::json::JsonNode>);
		using OnReceiveEventHandlerMap = collections::Dictionary<WString, OnReceiveEventHandler>;
		OnReceiveEventHandlerMap		onReceiveEventHandlers;

		using OnReceiveResponseHandler = void (GuiRemoteProtocolFromJsonChannel::*)(vint, Ptr<glr::json::JsonNode>);
		using OnReceiveResponseHandlerMap = collections::Dictionary<WString, OnReceiveResponseHandler>;
		OnReceiveResponseHandlerMap		onReceiveResponseHandlers;

#define EVENT_NOREQ(NAME, REQUEST)					void OnReceive_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments);
#define EVENT_REQ(NAME, REQUEST)					void OnReceive_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments);
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)										void OnReceive_Response_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		void						OnReceive(const Ptr<glr::json::JsonObject>& package) override;

	public:

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)	void Request ## NAME() override;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)		void Request ## NAME(vint id) override;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)		void Request ## NAME(const REQUEST& arguments) override;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)		void Request ## NAME(vint id, const REQUEST& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

		GuiRemoteProtocolFromJsonChannel(IJsonChannel* _channel);
		~GuiRemoteProtocolFromJsonChannel();

		vl::Event<void(const ChannelPackageInfo&)>		BeforeWrite;
		vl::Event<void(const ChannelPackageInfo&)>		BeforeOnReceive;

		void											Initialize(IGuiRemoteProtocolEvents* _events) override;
		WString											GetExecutablePath() override;
		void											Submit(bool& disconnected) override;
		void											ProcessRemoteEvents() override;
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
		IJsonChannelReceiver*							receiver = nullptr;
		IGuiRemoteProtocol*								protocol = nullptr;

#define EVENT_NOREQ(NAME, REQUEST)						void On ## NAME() override;
#define EVENT_REQ(NAME, REQUEST)						void On ## NAME(const REQUEST& arguments) override;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)		EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)						void Respond ## NAME(vint id, const RESPONSE& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

	protected:

		using WriteHandler = void (GuiRemoteJsonChannelFromProtocol::*)(vint, Ptr<glr::json::JsonNode>);
		using WriteHandlerMap = collections::Dictionary<WString, WriteHandler>;
		WriteHandlerMap			writeHandlers;

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Write_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Write_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Write_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Write_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

	public:

		GuiRemoteJsonChannelFromProtocol(IGuiRemoteProtocol* _protocol);
		~GuiRemoteJsonChannelFromProtocol();

		vl::Event<void(const ChannelPackageInfo&)>		BeforeWrite;
		vl::Event<void(const ChannelPackageInfo&)>		BeforeOnReceive;

		void											Initialize(IJsonChannelReceiver* _receiver) override;
		IJsonChannelReceiver*							GetReceiver() override;
		void											Write(const Ptr<glr::json::JsonObject>& package) override;
		WString											GetExecutablePath() override;
		void											Submit(bool& disconnected) override;
		void											ProcessRemoteEvents() override;
	};

/***********************************************************************
JsonToStringSerializer
***********************************************************************/

	struct JsonToStringSerializer
	{
		using SourceType = Ptr<glr::json::JsonObject>;
		using DestType = WString;
		using ContextType = Ptr<glr::json::Parser>;

		static void										Serialize(Ptr<glr::json::Parser> parser, const SourceType& source, DestType& dest);
		static void										Deserialize(Ptr<glr::json::Parser> parser, const DestType& source, SourceType& dest);
	};

	using GuiRemoteJsonChannelStringSerializer = GuiRemoteProtocolChannelSerializer<JsonToStringSerializer>;
	using GuiRemoteJsonChannelStringDeserializer = GuiRemoteProtocolChannelDeserializer<JsonToStringSerializer>;
}

#endif