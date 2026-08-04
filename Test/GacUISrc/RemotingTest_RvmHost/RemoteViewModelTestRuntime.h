#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "../../../Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h"
#include <cstdlib>

namespace vl::presentation::remote_view_model_test
{
	template<typename TServerBase>
	class RemoteViewModelChannelServer : public remoting::RemotingChannelServer<TServerBase>
	{
		using Base = remoting::RemotingChannelServer<TServerBase>;

	private:
		Ptr<remoting::RemotingRequesterSession>				session;

	protected:
		inter_process::WaitForClientResult OnRemoteClientConnected(
			vint clientId,
			const remoting::JsonChannelClient::ChannelNameList& availableChannels
			) override
		{
			if (session)
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
			}
			return Base::OnRemoteClientConnected(clientId, availableChannels);
		}

	public:
		template<typename... TArgs>
		RemoteViewModelChannelServer(
			Ptr<glr::json::Parser> parser,
			bool _acceptViewModel,
			bool _acceptRenderer,
			TArgs&&... args
			)
			: Base(parser, _acceptRenderer, std::forward<TArgs>(args)...)
		{
			if (_acceptViewModel)
			{
				session = Ptr(new remoting::RemotingRequesterSession(
					CreateConfiguration(),
					CreateDispatcherFactory(),
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
