#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "RemoteViewModelTestRpc.h"
#include "../../../Source/RemotingHelpers/RemotingClient/RemotingClient.h"
#include "../../../Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h"
#include <cstdlib>

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelRequesterSession : public remoting::RemotingRequesterSession
	{
	public:
		RemoteViewModelRequesterSession(
			Ptr<glr::json::Parser> parser,
			const Func<void(const WString&)>& terminalAction
			);

		Ptr<rvmt::IViewModel>								RequestViewModel();
	};

	template<typename TServerBase>
	class RemoteViewModelChannelServer : public remoting::RemotingChannelServer<TServerBase>
	{
		using Base = remoting::RemotingChannelServer<TServerBase>;

	private:
		RemoteViewModelRequesterSession						session;

	protected:
		inter_process::WaitForClientResult OnRemoteClientConnected(
			vint clientId,
			const remoting::JsonChannelClient::ChannelNameList& availableChannels
			) override
		{
			using WaitForClientResult = inter_process::WaitForClientResult;
			if (IsRemoteViewModelHostChannel(availableChannels))
			{
				return session.TryAcceptHost(clientId)
					? WaitForClientResult::Accept
					: WaitForClientResult::Reject;
			}
			if (remoting::IsRendererChannel(availableChannels) && !session.CanAdmitRenderer())
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
			, session(
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
				)
		{
		}

		RemoteViewModelRequesterSession* GetSession()
		{
			return &session;
		}

		void OnClientDisconnected(vint clientId) override
		{
			Base::OnClientDisconnected(clientId);
			session.OnClientDisconnected(clientId);
		}
	};

	class RemoteViewModelHostingClient : public remoting::RemotingHostingClient
	{
	public:
		RemoteViewModelHostingClient(
			Ptr<inter_process::INetworkProtocolClient> networkClient,
			Ptr<glr::json::Parser> parser,
			Ptr<rpc_controller::channeling::TaskQueue> taskQueue
			);
	};
}

#endif
