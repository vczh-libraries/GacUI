#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "RemoteViewModelTestRpc.h"
#include "../../../Source/RemotingHelpers/RemotingClient/RemotingClient.h"

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelRequesterSession : public Object
	{
	private:
		Ptr<remoting::RemotingRequesterSession>				session;

	public:
		RemoteViewModelRequesterSession(
			Ptr<glr::json::Parser> parser,
			const Func<void(const WString&)>& terminalAction
			);

		bool												CanAcceptLocalClient(rpc_controller::channeling::JsonChannelClient* localClient);
		bool												TryAcceptViewModelHost(vint clientId);
		void												OnClientDisconnected(vint clientId);
		void												Start(rpc_controller::channeling::JsonChannelServer* channelServer);
		Ptr<rvmt::IViewModel>								RequestViewModel();
		bool												BeginRunning();
		bool												CanAdmitRenderer();
		void												BeginStopping();
		void												Stop(const Func<void()>& stopServer);
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
