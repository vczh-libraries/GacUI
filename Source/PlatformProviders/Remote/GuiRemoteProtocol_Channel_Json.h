/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  vl::inter_process::IChannel<Ptr<JsonNode>>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_JSON
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_JSON

#include "GuiRemoteProtocol_Shared.h"
#include <VlppOS.h>

namespace vl::presentation::remoteprotocol::channeling
{
	constexpr const wchar_t* GacUIRemoteProtocolChannelName = L"GacUIRemoteProtocol";
	constexpr vint GacUIRemoteProtocolCoreClientId = 1;

	using JsonPackage = Ptr<glr::json::JsonNode>;
	using IJsonChannelReader = inter_process::IChannelReader<JsonPackage>;
	using IJsonChannel = inter_process::IChannel<JsonPackage>;
	using IJsonChannelClient = inter_process::IChannelClient<JsonPackage>;
	using IJsonChannelServer = inter_process::IChannelServer<JsonPackage>;

/***********************************************************************
ChannelPackageSemantic
***********************************************************************/

	enum class ChannelPackageSemantic
	{
		Message,
		Request,
		Response,
		Event,
		Unknown,
	};

	struct ChannelPackageInfo
	{
		ChannelPackageSemantic		semantic = ChannelPackageSemantic::Unknown;
		vint						id = -1;
		WString						name;
	};

	extern void						JsonChannelPack(const ChannelPackageInfo& info, Ptr<glr::json::JsonNode> arguments, Ptr<glr::json::JsonObject>& package);
	extern void						JsonChannelUnpack(Ptr<glr::json::JsonObject> package, ChannelPackageInfo& info, Ptr<glr::json::JsonNode>& arguments);
	extern void						JsonChannelUnpack(Ptr<glr::json::JsonNode> package, ChannelPackageInfo& info, Ptr<glr::json::JsonNode>& arguments);

/***********************************************************************
JsonNodeListSerializer
***********************************************************************/

	struct JsonNodeListSerializer
	{
		using SourceType = collections::List<JsonPackage>;
		using DestType = WString;
		using ContextType = Ptr<glr::json::Parser>;

		static void									Serialize(Ptr<glr::json::Parser> parser, const SourceType& source, DestType& dest);
		static void									Deserialize(Ptr<glr::json::Parser> parser, const DestType& source, SourceType& dest);
	};

	using GuiRemoteProtocolChannelServer = inter_process::NetworkProtocolChannelServer<JsonPackage, JsonNodeListSerializer>;

	class GuiRemoteProtocolChannelClient
		: public inter_process::NetworkProtocolChannelClient<JsonPackage, JsonNodeListSerializer>
	{
		using Base = inter_process::NetworkProtocolChannelClient<JsonPackage, JsonNodeListSerializer>;
	protected:
		IJsonChannelClient::ChannelMap				channelNames;

	public:
		GuiRemoteProtocolChannelClient(Ptr<inter_process::INetworkProtocolClient> client, Ptr<glr::json::Parser> parser);

		const IJsonChannelClient::ChannelNameList&	OnGetChannelNames() override;
		IJsonChannel*								GetProtocolChannel();
	};

	class GuiRemoteProtocolLocalChannelClient
		: public inter_process::NetworkProtocolLocalChannelClient<JsonPackage, JsonNodeListSerializer>
	{
		using Base = inter_process::NetworkProtocolLocalChannelClient<JsonPackage, JsonNodeListSerializer>;
	protected:
		IJsonChannelClient::ChannelMap				channelNames;

	public:
		GuiRemoteProtocolLocalChannelClient(Ptr<glr::json::Parser> parser);

		const IJsonChannelClient::ChannelNameList&
													OnGetChannelNames() override;
		IJsonChannel*								GetProtocolChannel();
	};

/***********************************************************************
GuiRemoteProtocolCoreChannel
***********************************************************************/
	
	class GuiRemoteProtocolCoreChannel
		: public Object
		, public virtual IGuiRemoteProtocol
		, protected IJsonChannelReader
	{
	protected:
		IJsonChannelClient*							client = nullptr;
		IJsonChannel*								channel = nullptr;
		IGuiRemoteProtocolEvents*					events = nullptr;
		IGuiRemoteEventProcessor*					eventProcessor = nullptr;
		WString										executablePath;
		SpinLock									lockRendererClientId;
		vint										rendererClientId = -1;

		using OnReadEventHandler = void (GuiRemoteProtocolCoreChannel::*)(Ptr<glr::json::JsonNode>);
		using OnReadEventHandlerMap = collections::Dictionary<WString, OnReadEventHandler>;
		OnReadEventHandlerMap						onReadEventHandlers;

		using OnReadResponseHandler = void (GuiRemoteProtocolCoreChannel::*)(vint, Ptr<glr::json::JsonNode>);
		using OnReadResponseHandlerMap = collections::Dictionary<WString, OnReadResponseHandler>;
		OnReadResponseHandlerMap					onReadResponseHandlers;

#define EVENT_NOREQ(NAME, REQUEST)					void OnRead_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments);
#define EVENT_REQ(NAME, REQUEST)					void OnRead_Event_ ## NAME (Ptr<glr::json::JsonNode> jsonArguments);
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)										void OnRead_Response_ ## NAME (vint id, Ptr<glr::json::JsonNode> jsonArguments);
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		void						Write(Ptr<glr::json::JsonObject> package);
		void						SetRendererClientId(vint clientId);
		vint						GetRendererClientId();
		void						OnRead(vint senderClientId, const JsonPackage& package) override;

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

		GuiRemoteProtocolCoreChannel(IJsonChannelClient* _client, IJsonChannel* _channel, const WString& _executablePath, IGuiRemoteEventProcessor* _eventProcessor = nullptr);
		~GuiRemoteProtocolCoreChannel();

		vl::Event<void(const ChannelPackageInfo&)>		BeforeWrite;
		vl::Event<void(const ChannelPackageInfo&)>		BeforeOnRead;

		void											Initialize(IGuiRemoteProtocolEvents* _events) override;
		WString											GetExecutablePath() override;
		void											Submit(bool& disconnected) override;
		IGuiRemoteEventProcessor*						GetRemoteEventProcessor() override;
	};

/***********************************************************************
GuiRemoteProtocolRendererChannel
***********************************************************************/

	class GuiRemoteProtocolRendererChannel
		: public Object
		, protected virtual IJsonChannelReader
		, protected virtual IGuiRemoteProtocolEvents
	{
	protected:
		IJsonChannelClient*								client = nullptr;
		IJsonChannel*									channel = nullptr;
		IGuiRemoteProtocol*								protocol = nullptr;
		bool											receiving = false;

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

		using WriteHandler = void (GuiRemoteProtocolRendererChannel::*)(vint, Ptr<glr::json::JsonNode>);
		using WriteHandlerMap = collections::Dictionary<WString, WriteHandler>;
		WriteHandlerMap									writeHandlers;

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

		void											Write(Ptr<glr::json::JsonObject> package);
		void											Flush(bool& disconnected);
		void											OnRead(vint senderClientId, const JsonPackage& package) override;

	public:

		GuiRemoteProtocolRendererChannel(IJsonChannelClient* _client, IJsonChannel* _channel, IGuiRemoteProtocol* _protocol);
		~GuiRemoteProtocolRendererChannel();

		vl::Event<void(const ChannelPackageInfo&)>		BeforeWrite;
		vl::Event<void(const ChannelPackageInfo&)>		BeforeOnRead;
	};
}

#endif
