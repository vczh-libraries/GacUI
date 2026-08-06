#include "ViewModelHostServer.h"

namespace vl::presentation::remoting
{
	using namespace collections;
	using namespace rpc_controller::channeling;

	enum class RemotingRequesterSession::RequesterPhase
	{
		Starting,
		Running,
		Stopping,
	};

/***********************************************************************
TaskQueueThread
***********************************************************************/

	class RemotingRequesterSession::TaskQueueThread : public Thread
	{
	private:
		Ptr<TaskQueue>										taskQueue;

	protected:
		void Run() override
		{
			try
			{
				taskQueue->RunTaskQueue();
			}
			catch (...)
			{
				std::_Exit(1);
			}
		}

	public:
		TaskQueueThread(Ptr<TaskQueue> _taskQueue)
			: taskQueue(_taskQueue)
		{
			CHECK_ERROR(taskQueue, L"TaskQueueThread::TaskQueueThread(...)#The task queue is null.");
		}
	};

/***********************************************************************
BroadcastingLocalClient
***********************************************************************/

	class RemotingRequesterSession::BroadcastingLocalClient
		: public JsonLocalChannelClient
		, protected inter_process::IChannelReader<JsonPackage>
	{
	private:
		JsonChannelClient::ChannelMap						channelNames;
		Ptr<RpcJsonDispatcherServerForTaskQueue>			dispatcher;
		Func<void(vint, const JsonPackage&)>				controlCallback;
		JsonChannel*										controlChannel = nullptr;

	protected:
		void OnRead(vint senderClientId, const JsonPackage& package) override
		{
			controlCallback(senderClientId, package);
		}

	public:
		BroadcastingLocalClient(
			Ptr<glr::json::Parser> parser,
			const Func<void(vint, const JsonPackage&)>& _controlCallback
			)
			: JsonLocalChannelClient(parser)
			, controlCallback(_controlCallback)
		{
			CHECK_ERROR(controlCallback, L"BroadcastingLocalClient::BroadcastingLocalClient(...)#The control callback is null.");
			channelNames.Add(ViewModelChannelName, nullptr);
			channelNames.Add(ViewModelReadyChannelName, nullptr);
		}

		const JsonChannelClient::ChannelNameList& OnGetChannelNames() override
		{
			return channelNames.Keys();
		}

		vint Connect(JsonChannelServer* channelServer, Ptr<JsonChannelClient> self, Ptr<TaskQueue> taskQueue)
		{
			CHECK_ERROR(channelServer, L"BroadcastingLocalClient::Connect(...)#The channel server is null.");
			CHECK_ERROR(self, L"BroadcastingLocalClient::Connect(...)#The shared local client is null.");
			auto clientId = channelServer->ConnectLocalClient(self);
			CHECK_ERROR(clientId != -1, L"BroadcastingLocalClient::Connect(...)#Failed to connect the broadcasting client.");
			controlChannel = GetChannels()[ViewModelReadyChannelName];
			CHECK_ERROR(controlChannel, L"BroadcastingLocalClient::Connect(...)#The control channel is null.");
			controlChannel->Initialize(this);
			dispatcher = Ptr(new RpcJsonDispatcherServerForTaskQueue(
				this,
				GetChannels()[ViewModelChannelName],
				taskQueue
				));
			return clientId;
		}

		RpcJsonDispatcherServer* GetDispatcher()
		{
			CHECK_ERROR(dispatcher, L"BroadcastingLocalClient::GetDispatcher()#The client is not connected.");
			return dispatcher.Obj();
		}
	};

/***********************************************************************
RequesterLocalClient
***********************************************************************/

	class RemotingRequesterSession::RequesterLocalClient : public JsonLocalChannelClient
	{
	private:
		JsonChannelClient::ChannelMap						channelNames;
		Ptr<remote_view_model_test::RemoteViewModelJsonDispatcherClient> dispatcher;

	public:
		RequesterLocalClient(Ptr<glr::json::Parser> parser)
			: JsonLocalChannelClient(parser)
		{
			channelNames.Add(ViewModelChannelName, nullptr);
		}

		const JsonChannelClient::ChannelNameList& OnGetChannelNames() override
		{
			return channelNames.Keys();
		}

		vint Connect(
			JsonChannelServer* channelServer,
			Ptr<JsonChannelClient> self,
			Ptr<TaskQueue> taskQueue,
			vint serverClientId
			)
		{
			List<WString> waitingForServices;
			waitingForServices.Add(ViewModelServiceName);
			dispatcher = Ptr(new remote_view_model_test::RemoteViewModelJsonDispatcherClient(taskQueue));
			auto clientId = dispatcher->ConnectLocalServer(
				channelServer,
				self,
				GetChannels()[ViewModelChannelName],
				waitingForServices
				);
			CHECK_ERROR(clientId != -1, L"RequesterLocalClient::Connect(...)#Failed to connect the requester client.");
			dispatcher->InitializeRpc(clientId);
			dispatcher->SetServerLocalClientId(serverClientId);
			return clientId;
		}

		remote_view_model_test::RemoteViewModelJsonDispatcherClient* GetDispatcher()
		{
			CHECK_ERROR(dispatcher, L"RequesterLocalClient::GetDispatcher()#The client is not connected.");
			return dispatcher.Obj();
		}
	};

/***********************************************************************
RemotingRequesterSession
***********************************************************************/

	void FinalizeRpcOnTaskQueue(
		Ptr<TaskQueue> taskQueue,
		remote_view_model_test::RemoteViewModelJsonDispatcherClient* dispatcher
		)
	{
		CHECK_ERROR(taskQueue, L"FinalizeRpcOnTaskQueue(...)#The task queue is null.");
		CHECK_ERROR(dispatcher, L"FinalizeRpcOnTaskQueue(...)#The dispatcher is null.");
		EventObject finalized;
		CHECK_ERROR(finalized.CreateAutoUnsignal(false), L"FinalizeRpcOnTaskQueue(...)#Failed to create the finalization event.");
		taskQueue->QueueTask(Func<void()>([&]()
		{
			dispatcher->FinalizeRpc();
			finalized.Signal();
		}));
		finalized.Wait();
	}

	RemotingRequesterSession::RemotingRequesterSession(
		Ptr<glr::json::Parser> parser,
		const Func<void(const WString&)>& _terminalAction
		)
		: terminalAction(_terminalAction)
		, phase(RequesterPhase::Starting)
		, taskQueue(Ptr(new TaskQueue))
	{
		CHECK_ERROR(parser, L"RemotingRequesterSession::RemotingRequesterSession(...)#The JSON parser is null.");
		CHECK_ERROR(terminalAction, L"RemotingRequesterSession::RemotingRequesterSession(...)#The terminal action is null.");

		taskQueueThread = Ptr(new TaskQueueThread(taskQueue));
		broadcastingClient = Ptr(new BroadcastingLocalClient(
			parser,
			Func<void(vint, const JsonPackage&)>([this](vint senderClientId, const JsonPackage& package)
			{
				OnControlMessage(senderClientId, package);
			})
			));
		requesterClient = Ptr(new RequesterLocalClient(parser));
	}

	RemotingRequesterSession::~RemotingRequesterSession() = default;

	bool RemotingRequesterSession::TryAcceptHost(vint clientId)
	{
		bool accepted = false;
		SPIN_LOCK(lockState)
		{
			if (
				admissionReady &&
				phase == RequesterPhase::Starting &&
				!hostEverAccepted &&
				clientId != InvalidRemoteViewModelClientId
				)
			{
				hostId = clientId;
				hostEverAccepted = true;
				accepted = true;
			}
		}
		return accepted;
	}

	void RemotingRequesterSession::RegisterHost(vint clientId)
	{
		SPIN_LOCK(lockState)
		{
			CHECK_ERROR(
				hostId == clientId && !brokerRegistrationClaimed,
				L"RemotingRequesterSession received Ready from an unexpected client."
				);
			brokerRegistrationClaimed = true;
		}

		CS_LOCK(lockBroker)
		{
			bool shouldRegister = false;
			SPIN_LOCK(lockState)
			{
				shouldRegister = hostId == clientId && brokerRegistrationClaimed;
			}
			if (shouldRegister)
			{
				brokerDispatcher->RegisterClient(clientId);
			}
		}
	}

	void RemotingRequesterSession::OnControlMessage(vint senderClientId, const JsonPackage& package)
	{
		if (IsViewModelReadyMessage(package))
		{
			RegisterHost(senderClientId);
			return;
		}
		CHECK_ERROR(false, L"RemotingRequesterSession received an unexpected control message.");
	}

	void RemotingRequesterSession::OnClientDisconnected(vint clientId)
	{
		bool hostDisconnected = false;
		bool disconnectBroker = false;
		SPIN_LOCK(lockState)
		{
			if (hostId == clientId)
			{
				hostId = InvalidRemoteViewModelClientId;
				hostDisconnected = phase != RequesterPhase::Stopping;
				disconnectBroker = brokerRegistrationClaimed;
			}
		}
		if (disconnectBroker)
		{
			CS_LOCK(lockBroker)
			{
				brokerDispatcher->DisconnectClient(clientId);
			}
		}
		if (hostDisconnected)
		{
			terminalAction(RemoteViewModelHostDisconnectedError);
		}
	}

	void RemotingRequesterSession::Start(JsonChannelServer* channelServer)
	{
		CHECK_ERROR(channelServer, L"RemotingRequesterSession::Start(...)#The channel server is null.");
		CHECK_ERROR(!brokerDispatcher, L"RemotingRequesterSession::Start(...)#The session has already started.");
		broadcastingClient->Connect(channelServer, broadcastingClient, taskQueue);
		SPIN_LOCK(lockState)
		{
			brokerDispatcher = broadcastingClient->GetDispatcher();
		}
		auto requesterClientId = requesterClient->Connect(
			channelServer,
			requesterClient,
			taskQueue,
			brokerDispatcher->GetServerClientId()
			);
		requesterDispatcher = requesterClient->GetDispatcher();
		brokerDispatcher->RegisterClient(requesterClientId);
		CHECK_ERROR(taskQueueThread->Start(), L"RemotingRequesterSession::Start(...)#Failed to start the task queue thread.");
		SPIN_LOCK(lockState)
		{
			admissionReady = true;
		}
	}

	Ptr<IDescriptable> RemotingRequesterSession::RequestService()
	{
		CHECK_ERROR(requesterDispatcher, L"RemotingRequesterSession::RequestService()#The session has not started.");
		CHECK_ERROR(!rpcInitialized, L"RemotingRequesterSession::RequestService()#The dispatcher is already initialized.");
		rpcInitialized = true;
		requesterDispatcher->Initialize();
		service = requesterDispatcher
			->GetRpcLifecycle()
			->RequestService(ViewModelServiceName);
		CHECK_ERROR(service, L"RemotingRequesterSession::RequestService()#Failed to request the service.");
		return service;
	}

	bool RemotingRequesterSession::BeginRunning()
	{
		SPIN_LOCK(lockState)
		{
			if (
				phase == RequesterPhase::Starting &&
				hostId != InvalidRemoteViewModelClientId
				)
			{
				phase = RequesterPhase::Running;
				return true;
			}
		}
		return false;
	}

	bool RemotingRequesterSession::CanAdmitRenderer()
	{
		bool result = false;
		SPIN_LOCK(lockState)
		{
			result = phase == RequesterPhase::Running;
		}
		return result;
	}

	void RemotingRequesterSession::BeginStopping()
	{
		SPIN_LOCK(lockState)
		{
			if (phase == RequesterPhase::Stopping)
			{
				return;
			}
			phase = RequesterPhase::Stopping;
		}
	}

	void RemotingRequesterSession::Stop(const Func<void()>& stopServer)
	{
		BeginStopping();
		if (requesterDispatcher)
		{
			FinalizeRpcOnTaskQueue(taskQueue, requesterDispatcher);
		}
		if (stopServer)
		{
			stopServer();
		}
		taskQueue->QueueExitTask();
		taskQueueThread->Wait();
		service = nullptr;
	}
}
