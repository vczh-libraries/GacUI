#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "RemoteViewModelTestRpc.h"
#include <VlppWorkflowLibrary.h>

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelJsonDispatcherClient
		: public rpc_controller::channeling::RpcJsonDispatcherClientForTaskQueue
	{
	public:
		RemoteViewModelJsonDispatcherClient(Ptr<rpc_controller::channeling::TaskQueue> taskQueue);

		void											InitializeRpc(vint clientId);
	};

	class RemoteViewModelBroadcastingLocalClient
		: public rpc_controller::channeling::JsonLocalChannelClient
	{
	private:
		rpc_controller::channeling::JsonChannelClient::ChannelMap
														channelNames;
		Ptr<rpc_controller::channeling::RpcJsonDispatcherServerForTaskQueue>
														dispatcher;
		vint											clientId = -1;

	public:
		RemoteViewModelBroadcastingLocalClient(Ptr<glr::json::Parser> parser);

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		vint											Connect(
															rpc_controller::channeling::JsonChannelServer* channelServer,
															Ptr<rpc_controller::channeling::JsonChannelClient> self,
															Ptr<rpc_controller::channeling::TaskQueue> taskQueue
															);
		rpc_controller::channeling::RpcJsonDispatcherServer*
														GetDispatcher();
	};

	class RemoteViewModelRequesterLocalClient
		: public rpc_controller::channeling::JsonLocalChannelClient
	{
	private:
		rpc_controller::channeling::JsonChannelClient::ChannelMap
														channelNames;
		Ptr<RemoteViewModelJsonDispatcherClient>		dispatcher;

	public:
		RemoteViewModelRequesterLocalClient(Ptr<glr::json::Parser> parser);

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		void											OnConnected(vint clientId) override;
		vint											Connect(
															rpc_controller::channeling::JsonChannelServer* channelServer,
															Ptr<rpc_controller::channeling::JsonChannelClient> self,
															Ptr<rpc_controller::channeling::TaskQueue> taskQueue,
															vint serverClientId
															);
		RemoteViewModelJsonDispatcherClient*			GetDispatcher();
	};

	class RemoteViewModelReadyLocalClient
		: public rpc_controller::channeling::JsonLocalChannelClient
		, protected inter_process::IChannelReader<rpc_controller::channeling::JsonPackage>
	{
	private:
		rpc_controller::channeling::JsonChannelClient::ChannelMap
														channelNames;
		Func<void(vint)>								readyCallback;

	protected:
		void											OnRead(
															vint senderClientId,
															const rpc_controller::channeling::JsonPackage& package
															) override;

	public:
		RemoteViewModelReadyLocalClient(
			Ptr<glr::json::Parser> parser,
			const Func<void(vint)>& _readyCallback
			);

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		vint											Connect(
															rpc_controller::channeling::JsonChannelServer* channelServer,
															Ptr<rpc_controller::channeling::JsonChannelClient> self
															);
	};

	class RemoteViewModelTaskQueueThread : public Thread
	{
	private:
		Ptr<rpc_controller::channeling::TaskQueue>		taskQueue;
		SpinLock										lockFailure;
		Func<void(const WString&)>						failureCallback;
		bool											failureReported = false;
		WString											failureMessage;

		void											ReportFailure(const WString& message);

	protected:
		void											Run() override;

	public:
		RemoteViewModelTaskQueueThread(Ptr<rpc_controller::channeling::TaskQueue> _taskQueue);

		void											SetFailureCallback(const Func<void(const WString&)>& callback);
		Nullable<WString>								GetFailure();
	};

	extern void											FinalizeRpcOnTaskQueue(
															Ptr<rpc_controller::channeling::TaskQueue> taskQueue,
															RemoteViewModelJsonDispatcherClient* dispatcher
															);
	extern Nullable<WString>							StopRpcTaskQueue(
															Ptr<rpc_controller::channeling::TaskQueue> taskQueue,
															RemoteViewModelTaskQueueThread* taskQueueThread
															);
}

#endif
