#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "RemoteViewModelTestRpc.h"
#include <VlppWorkflowLibrary.h>

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelRequesterSession : public Object
	{
	private:
		class Impl;
		Ptr<Impl>										impl;

	public:
		RemoteViewModelRequesterSession(
			Ptr<glr::json::Parser> _parser,
			const Func<void(const WString&)>& _terminalAction
			);
		~RemoteViewModelRequesterSession();

		bool											CanAcceptLocalClient(
															rpc_controller::channeling::JsonChannelClient* localClient
															);
		bool											TryAcceptViewModelHost(vint clientId);
		void											OnClientDisconnected(vint clientId);

		void											Start(rpc_controller::channeling::JsonChannelServer* _channelServer);
		Ptr<rvmt::IViewModel>							RequestViewModel();

		bool											BeginRunning();
		bool											CanAdmitRenderer();
		void											BeginStopping();
		Nullable<WString>								GetFatalError();

		void											Stop(const Func<void()>& stopServer);
	};

	class RemoteViewModelHostingClient
		: public rpc_controller::channeling::JsonNetworkChannelClient
		, protected inter_process::IChannelReader<rpc_controller::channeling::JsonPackage>
	{
	private:
		class Impl;
		Ptr<Impl>										impl;

		void											OnRead(
															vint senderClientId,
															const rpc_controller::channeling::JsonPackage& package
															) override;

	public:
		RemoteViewModelHostingClient(
			Ptr<inter_process::INetworkProtocolClient> networkClient,
			Ptr<glr::json::Parser> parser,
			Ptr<rpc_controller::channeling::TaskQueue> taskQueue
			);
		~RemoteViewModelHostingClient();

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		void											OnConnected(vint clientId) override;
		void											OnDisconnected() override;
		void											OnReadError(const WString& errorMessage) override;
		void											OnLocalError(const WString& errorMessage, bool fatal) override;

		void											Connect();
		void											SendReady();
		void											StartHeartbeat();
		rpc_controller::channeling::RpcJsonDispatcherClient*
														GetDispatcher();
	};
}

#endif
