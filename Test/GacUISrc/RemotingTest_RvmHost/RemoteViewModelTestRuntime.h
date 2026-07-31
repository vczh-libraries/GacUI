#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_RUNTIME

#include "RemoteViewModelTestShared.h"
#include "RemoteViewModelRoleState.h"
#include "RemoteViewModelTestRpc.h"
#include <VlppWorkflowLibrary.h>

namespace vl::presentation::remote_view_model_test
{
	enum class RemoteViewModelControlMessageKind
	{
		Unknown,
		Ready,
		Heartbeat,
		HeartbeatAck,
		RequesterStopping,
		RequesterStoppingAck,
	};

	extern rpc_controller::channeling::JsonPackage	CreateRemoteViewModelControlMessage(
															RemoteViewModelControlMessageKind kind,
															vint64_t sequence = -1
															);
	extern bool											ParseRemoteViewModelControlMessage(
															const rpc_controller::channeling::JsonPackage& package,
															RemoteViewModelControlMessageKind& kind,
															vint64_t& sequence
															);

	class RemoteViewModelBackgroundThread : public Thread
	{
	private:
		SpinLock										lockState;
		EventObject										eventWake;
		Func<void()>									callback;
		bool											stopping = false;

	protected:
		void											Run() override;

	public:
		RemoteViewModelBackgroundThread(const Func<void()>& _callback);

		void											Notify();
		void											StopAndWait();
	};

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
		Func<void(vint, const rpc_controller::channeling::JsonPackage&)>
														readyCallback;

	protected:
		void											OnRead(
															vint senderClientId,
															const rpc_controller::channeling::JsonPackage& package
															) override;

	public:
		RemoteViewModelReadyLocalClient(
			Ptr<glr::json::Parser> parser,
			const Func<void(vint, const rpc_controller::channeling::JsonPackage&)>& _readyCallback
			);

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		vint											Connect(
															rpc_controller::channeling::JsonChannelServer* channelServer,
															Ptr<rpc_controller::channeling::JsonChannelClient> self
															);
		bool											SendToClient(
															vint clientId,
															const rpc_controller::channeling::JsonPackage& package
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

	class RemoteViewModelRequesterSession : public Object
	{
	private:
		SpinLock										lockState;
		Ptr<glr::json::Parser>							parser;
		Func<void(const WString&)>						terminalAction;
		RemoteViewModelRoleState						roleState;
		RemoteViewModelTerminalState					terminalState;
		RemoteViewModelLeaseState						hostLease;

		rpc_controller::channeling::JsonChannelServer*	channelServer = nullptr;
		rpc_controller::channeling::RpcJsonDispatcherServer*
														brokerDispatcher = nullptr;
		vint											brokerViewModelHostId = InvalidRemoteViewModelClientId;

		Ptr<rpc_controller::channeling::TaskQueue>		taskQueue;
		Ptr<RemoteViewModelTaskQueueThread>				taskQueueThread;
		Ptr<RemoteViewModelBroadcastingLocalClient>		broadcastingClient;
		Ptr<RemoteViewModelReadyLocalClient>				readyClient;
		Ptr<RemoteViewModelRequesterLocalClient>			requesterClient;
		RemoteViewModelJsonDispatcherClient*			requesterDispatcher = nullptr;
		Ptr<rvmt::IViewModel>							viewModel;
		Ptr<RemoteViewModelBackgroundThread>			stateThread;
		EventObject										stoppingAcknowledged;

		bool											taskQueueStarted = false;
		bool											stateThreadStarted = false;
		bool											admissionReady = false;
		bool											rpcInitialized = false;
		bool											stopped = false;

		void											ProcessState();
		void											ReportFailure(const WString& message);
		void											OnReadyMessage(
															vint senderClientId,
															const rpc_controller::channeling::JsonPackage& package
															);
		void											RegisterViewModelHost(vint clientId);

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
		Ptr<rvmt::IViewModel>							GetViewModel();

		bool											BeginRunning();
		bool											CanAdmitRenderer();
		void											BeginStopping();
		Nullable<WString>								GetFatalError();
		bool											IsRemoteViewModelClientId(vint clientId);

		Nullable<WString>								Stop(const Func<void()>& stopServer);
	};

	class RemoteViewModelHostingClient
		: public rpc_controller::channeling::JsonNetworkChannelClient
		, protected inter_process::IChannelReader<rpc_controller::channeling::JsonPackage>
	{
	private:
		rpc_controller::channeling::JsonChannelClient::ChannelMap
														channelNames;
		Ptr<RemoteViewModelJsonDispatcherClient>		dispatcher;
		Func<void(const WString&, bool)>					terminalAction;
		SpinLock										lockState;
		RemoteViewModelTerminalState					terminalState;
		RemoteViewModelLeaseState						requesterLease;
		Ptr<RemoteViewModelBackgroundThread>			stateThread;
		rpc_controller::channeling::JsonChannel*		readyChannel = nullptr;
		vint											requesterReadyClientId = InvalidRemoteViewModelClientId;
		vint64_t										heartbeatSequence = 0;
		bool											stateThreadStarted = false;

		void											ProcessState();
		void											ReportFailure(const WString& message);
		void											OnRead(
															vint senderClientId,
															const rpc_controller::channeling::JsonPackage& package
															) override;

	public:
		RemoteViewModelHostingClient(
			Ptr<inter_process::INetworkProtocolClient> networkClient,
			Ptr<glr::json::Parser> parser,
			Ptr<rpc_controller::channeling::TaskQueue> taskQueue,
			const Func<void(const WString&, bool)>& _terminalAction
			);
		~RemoteViewModelHostingClient();

		const rpc_controller::channeling::JsonChannelClient::ChannelNameList&
														OnGetChannelNames() override;
		void											OnConnected(vint clientId) override;
		void											OnDisconnected() override;
		void											OnReadError(const WString& errorMessage) override;
		void											OnLocalError(const WString& errorMessage, bool fatal) override;

		void											Connect();
		void											BeginStopping();
		RemoteViewModelJsonDispatcherClient*			GetDispatcher();
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
