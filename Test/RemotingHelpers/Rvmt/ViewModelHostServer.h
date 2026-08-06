#ifndef VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_SERVER
#define VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_SERVER

#include "ViewModelShared.h"
#include "../RemotingServer/RemotingChannelServer.h"
#include <cstdlib>

namespace vl::presentation::remoting
{
	class RemotingRequesterSession : public Object
	{
	private:
		enum class RequesterPhase;
		class TaskQueueThread;
		class BroadcastingLocalClient;
		class RequesterLocalClient;

		SpinLock									lockState;
		CriticalSection								lockBroker;
		Func<void(const WString&)>					terminalAction;
		RequesterPhase								phase;
		vint										hostId = InvalidRemoteViewModelClientId;
		bool										hostEverAccepted = false;
		bool										brokerRegistrationClaimed = false;
		bool										admissionReady = false;
		bool										rpcInitialized = false;

		rpc_controller::channeling::RpcJsonDispatcherServer*			brokerDispatcher = nullptr;
		Ptr<TaskQueue>													taskQueue;
		Ptr<TaskQueueThread>											taskQueueThread;
		Ptr<BroadcastingLocalClient>									broadcastingClient;
		Ptr<RequesterLocalClient>										requesterClient;
		remote_view_model_test::RemoteViewModelJsonDispatcherClient*	requesterDispatcher = nullptr;
		Ptr<IDescriptable>												service;

		void										RegisterHost(vint clientId);
		void										OnControlMessage(vint senderClientId, const JsonPackage& package);

	public:
		RemotingRequesterSession(
			Ptr<glr::json::Parser> parser,
			const Func<void(const WString&)>& terminalAction
			);
		~RemotingRequesterSession();

		bool										TryAcceptHost(vint clientId);
		void										OnClientDisconnected(vint clientId);
		void										Start(JsonChannelServer* channelServer);
		Ptr<IDescriptable>							RequestService();
		bool										BeginRunning();
		bool										CanAdmitRenderer();
		void										BeginStopping();
		void										Stop(const Func<void()>& stopServer);
	};
}

namespace vl::presentation::remote_view_model_test
{
	template<typename TServerBase>
	class RemoteViewModelChannelServer : public remoting::RemotingChannelServer<TServerBase>
	{
		using Base = remoting::RemotingChannelServer<TServerBase>;

	private:
		Ptr<remoting::RemotingRequesterSession>				session;

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
				return session->TryAcceptHost(clientId)
					? WaitForClientResult::Accept
					: WaitForClientResult::Reject;
			}
			if (remoting::IsRendererChannel(availableChannels) && !session->CanAdmitRenderer())
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
		{
			session = Ptr(new remoting::RemotingRequesterSession(
				parser,
				Func<void(const WString&)>([this, _acceptRenderer](const WString& message)
				{
					if (_acceptRenderer)
					{
						try
						{
							this->BroadcastError(message);
						}
						catch (...)
						{
						}
					}
					std::_Exit(1);
				})
				));
		}

		remoting::RemotingRequesterSession* GetSession()
		{
			return session.Obj();
		}

		void OnClientDisconnected(vint clientId) override
		{
			Base::OnClientDisconnected(clientId);
			session->OnClientDisconnected(clientId);
		}
	};
}

#endif
