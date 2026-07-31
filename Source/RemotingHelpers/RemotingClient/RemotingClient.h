#ifndef VCZH_PRESENTATION_REMOTING_REMOTINGCLIENT
#define VCZH_PRESENTATION_REMOTING_REMOTINGCLIENT

#include "../../GacUI.h"
#include <VlppOS.h>
#include <VlppWorkflowLibrary.h>

namespace vl::presentation::remoting
{
	using JsonPackage = rpc_controller::channeling::JsonPackage;
	using JsonChannelClient = rpc_controller::channeling::JsonChannelClient;
	using JsonChannelServer = rpc_controller::channeling::JsonChannelServer;
	using TaskQueue = rpc_controller::channeling::TaskQueue;
	using RpcDispatcherClient = rpc_controller::channeling::RpcJsonDispatcherClient;

	struct RemotingRpcConfiguration
	{
		WString												rpcChannelName;
		WString												controlChannelName;
		WString												serviceName;
		WString												readyMessage;
		WString												heartbeatMessage;
		WString												requesterStoppingMessage;
		WString												hostDisconnectedError;
		vint												invalidClientId = -1;
		vint												heartbeatIntervalMilliseconds = 1000;
		vint												startupGraceMilliseconds = 10000;
		vint												leaseTimeoutMilliseconds = 5000;
	};

	class RemotingJsonDispatcherClient
		: public rpc_controller::channeling::RpcJsonDispatcherClientForTaskQueue
	{
	public:
		RemotingJsonDispatcherClient(Ptr<TaskQueue> taskQueue);
		virtual void										InitializeRpc(vint clientId) = 0;
	};

	using RemotingDispatcherFactory = Func<Ptr<RemotingJsonDispatcherClient>(Ptr<TaskQueue>)>;

	class RemotingRequesterSession : public Object
	{
	private:
		class Impl;
		Ptr<Impl>											impl;

	public:
		RemotingRequesterSession(
			const RemotingRpcConfiguration& configuration,
			const RemotingDispatcherFactory& dispatcherFactory,
			Ptr<glr::json::Parser> parser,
			const Func<void(const WString&)>& terminalAction
			);
		~RemotingRequesterSession();

		bool												CanAcceptLocalClient(JsonChannelClient* localClient);
		bool												TryAcceptHost(vint clientId);
		void												OnClientDisconnected(vint clientId);
		void												Start(JsonChannelServer* channelServer);
		Ptr<IDescriptable>									RequestService();
		bool												BeginRunning();
		bool												CanAdmitRenderer();
		void												BeginStopping();
		Nullable<WString>									GetFatalError();
		void												Stop(const Func<void()>& stopServer);
	};

	class RemotingHostingClient
		: public rpc_controller::channeling::JsonNetworkChannelClient
		, protected inter_process::IChannelReader<JsonPackage>
	{
	private:
		class Impl;
		Ptr<Impl>											impl;

		void												OnRead(vint senderClientId, const JsonPackage& package) override;

	public:
		RemotingHostingClient(
			Ptr<inter_process::INetworkProtocolClient> networkClient,
			const RemotingRpcConfiguration& configuration,
			const RemotingDispatcherFactory& dispatcherFactory,
			Ptr<glr::json::Parser> parser,
			Ptr<TaskQueue> taskQueue
			);
		~RemotingHostingClient();

		const JsonChannelClient::ChannelNameList&			OnGetChannelNames() override;
		void												OnConnected(vint clientId) override;
		void												OnDisconnected() override;
		void												OnReadError(const WString& errorMessage) override;
		void												OnLocalError(const WString& errorMessage, bool fatal) override;

		void												Connect();
		void												SendReady();
		void												StartHeartbeat();
		RpcDispatcherClient*								GetDispatcher();
	};
}

#endif
