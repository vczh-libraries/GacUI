#ifndef VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_SERVER
#define VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_SERVER

#include "ViewModelShared.h"
#include "../RemotingServer/RemotingChannelServer.h"

namespace vl::presentation::remoting
{
	class TaskQueueThread;
	class RpcBroadcastingLocalClient;
	class RpcServiceAccessLocalClient;

	class RpcServerHelpers
	{
	private:
		enum class RequesterPhase
		{
			Starting,
			Running,
			Stopping,
		};

		// covers phase, host state, admissionReady, taskQueueStarted, rpcInitialized, brokerDispatcher, requesterDispatcher and pending host-loss state
		SpinLock									lockState;
		CriticalSection								lockBroker;
		RequesterPhase								phase;
		vint										hostId = InvalidRemoteViewModelClientId;
		bool										hostEverAccepted = false;
		bool										brokerRegistrationClaimed = false;
		bool										admissionReady = false;
		bool										taskQueueStarted = false;
		bool										rpcInitialized = false;
		bool										pendingHostLoss = false;
		bool										hostLossClaimed = false;

		rpc_controller::channeling::RpcJsonDispatcherServer*			brokerDispatcher = nullptr;
		JsonChannelServer*											channelServer = nullptr;
		Ptr<TaskQueue>													taskQueue;
		Ptr<TaskQueueThread>											taskQueueThread;
		Ptr<RpcBroadcastingLocalClient>								broadcastingClient;
		Ptr<RpcServiceAccessLocalClient>							requesterClient;
		RpcDispatcherClient*										requesterDispatcher = nullptr;

		void										RegisterHost(vint clientId);
		void										OnControlMessage(vint senderClientId, const JsonPackage& package);
		void										FinalizeRpcOnTaskQueue();
		bool										BeginRunning();
		void										BeginStopping();

	protected:
		RpcServerHelpers(Ptr<glr::json::Parser> parser);
		~RpcServerHelpers();

		bool										TryAcceptHost(vint clientId);
		void										OnClientDisconnected(vint clientId);
		void										Start(JsonChannelServer* channelServer);
		vint										Connect(const collections::List<WString>& requiredServiceNames);
		RpcDispatcherClient*							GetDispatcher();
		Ptr<IDescriptable>							RequestService(const WString& typeName);
		bool										CanAdmitRenderer();
		void										Stop(const Func<void()>& stopServer);
	};
}

namespace vl::presentation::remote_view_model_test
{
	template<typename TServerBase>
	class RemoteViewModelChannelServer
		: public remoting::RemotingChannelServer<TServerBase>
		, protected remoting::RpcServerHelpers
	{
		using Base = remoting::RemotingChannelServer<TServerBase>;
		using Helpers = remoting::RpcServerHelpers;

	protected:
		bool IsRemoteViewModelHostChannel(
			const remoting::JsonChannelClient::ChannelNameList& availableChannels
			)
		{
			return
				availableChannels.Count() == 2 &&
				availableChannels.IndexOf(WString::Unmanaged(remoting::ViewModelChannelName)) != -1 &&
				availableChannels.IndexOf(WString::Unmanaged(remoting::ViewModelReadyChannelName)) != -1;
		}

		inter_process::WaitForClientResult OnRemoteClientConnected(
			vint clientId,
			const remoting::JsonChannelClient::ChannelNameList& availableChannels
			) override
		{
			using WaitForClientResult = inter_process::WaitForClientResult;
			if (IsRemoteViewModelHostChannel(availableChannels))
			{
				return TryAcceptHost(clientId)
					? WaitForClientResult::Accept
					: WaitForClientResult::Reject;
			}
			if (remoting::IsRendererChannel(availableChannels) && !Helpers::CanAdmitRenderer())
			{
				return WaitForClientResult::Reject;
			}
			return Base::OnRemoteClientConnected(clientId, availableChannels);
		}

	public:
		template<typename... TArgs>
		RemoteViewModelChannelServer(
			Ptr<glr::json::Parser> parser,
			bool _acceptRenderer,
			TArgs&&... args
			)
			: Base(parser, _acceptRenderer, std::forward<TArgs>(args)...)
			, Helpers(parser)
		{
		}

		void Start() override
		{
			Base::Start();
		}

		vint Connect(const collections::List<WString>& requiredServiceNames)
		{
			Helpers::Start(this);
			return Helpers::Connect(requiredServiceNames);
		}

		remoting::RpcDispatcherClient* GetDispatcher()
		{
			return Helpers::GetDispatcher();
		}

		Ptr<IDescriptable> RequestService(const WString& typeName)
		{
			return Helpers::RequestService(typeName);
		}

		bool CanAdmitRenderer()
		{
			return Helpers::CanAdmitRenderer();
		}

		void Stop() override
		{
			Helpers::Stop(Func<void()>([this]()
			{
				Base::Stop();
			}));
		}

		void OnClientDisconnected(vint clientId) override
		{
			Base::OnClientDisconnected(clientId);
			Helpers::OnClientDisconnected(clientId);
		}
	};

	template<typename TServerBase, typename TRvmChannelServer>
	class RemoteViewModelRendererChannelServer
		: public remoting::RemotingChannelServer<TServerBase>
	{
		using Base = remoting::RemotingChannelServer<TServerBase>;

	private:
		TRvmChannelServer*						rvmChannelServer = nullptr;

	protected:
		inter_process::WaitForClientResult OnRemoteClientConnected(
			vint clientId,
			const remoting::JsonChannelClient::ChannelNameList& availableChannels
			) override
		{
			if (remoting::IsRendererChannel(availableChannels) && (!rvmChannelServer || !rvmChannelServer->CanAdmitRenderer()))
			{
				return inter_process::WaitForClientResult::Reject;
			}
			return Base::OnRemoteClientConnected(clientId, availableChannels);
		}

	public:
		template<typename... TArgs>
		RemoteViewModelRendererChannelServer(
			Ptr<glr::json::Parser> parser,
			TArgs&&... args
			)
			: Base(parser, true, std::forward<TArgs>(args)...)
		{
		}

		void SetRvmChannelServer(TRvmChannelServer* value)
		{
			rvmChannelServer = value;
		}
	};
}

#endif
