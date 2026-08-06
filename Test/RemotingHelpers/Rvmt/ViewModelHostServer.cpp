#include "ViewModelHostServer.h"

namespace vl::presentation::remoting
{
	using namespace collections;
	using namespace rpc_controller::channeling;

/***********************************************************************
TaskQueueThread
***********************************************************************/

	class TaskQueueThread : public Thread
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
RpcBroadcastingLocalClient
***********************************************************************/

	class RpcBroadcastingLocalClient
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
		RpcBroadcastingLocalClient(
			Ptr<glr::json::Parser> parser,
			const Func<void(vint, const JsonPackage&)>& _controlCallback
			)
			: JsonLocalChannelClient(parser)
			, controlCallback(_controlCallback)
		{
			CHECK_ERROR(controlCallback, L"RpcBroadcastingLocalClient::RpcBroadcastingLocalClient(...)#The control callback is null.");
			channelNames.Add(ViewModelChannelName, nullptr);
			channelNames.Add(ViewModelReadyChannelName, nullptr);
		}

		const JsonChannelClient::ChannelNameList& OnGetChannelNames() override
		{
			return channelNames.Keys();
		}

		vint Connect(JsonChannelServer* channelServer, Ptr<JsonChannelClient> self, Ptr<TaskQueue> taskQueue)
		{
			CHECK_ERROR(channelServer, L"RpcBroadcastingLocalClient::Connect(...)#The channel server is null.");
			CHECK_ERROR(self, L"RpcBroadcastingLocalClient::Connect(...)#The shared local client is null.");
			auto clientId = channelServer->ConnectLocalClient(self);
			CHECK_ERROR(clientId != -1, L"RpcBroadcastingLocalClient::Connect(...)#Failed to connect the broadcasting client.");
			controlChannel = GetChannels()[ViewModelReadyChannelName];
			CHECK_ERROR(controlChannel, L"RpcBroadcastingLocalClient::Connect(...)#The control channel is null.");
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
			CHECK_ERROR(dispatcher, L"RpcBroadcastingLocalClient::GetDispatcher()#The client is not connected.");
			return dispatcher.Obj();
		}
	};

/***********************************************************************
RpcServiceAccessLocalClient
***********************************************************************/

	class RpcServiceAccessLocalClient : public JsonLocalChannelClient
	{
	private:
		JsonChannelClient::ChannelMap						channelNames;
		Ptr<remote_view_model_test::RemoteViewModelJsonDispatcherClient> dispatcher;

	public:
		RpcServiceAccessLocalClient(Ptr<glr::json::Parser> parser)
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
			vint serverClientId,
			const WString& typeName
			)
		{
			List<WString> waitingForServices;
			waitingForServices.Add(typeName);
			dispatcher = Ptr(new remote_view_model_test::RemoteViewModelJsonDispatcherClient(taskQueue));
			auto clientId = dispatcher->ConnectLocalServer(
				channelServer,
				self,
				GetChannels()[ViewModelChannelName],
				waitingForServices
				);
			CHECK_ERROR(clientId != -1, L"RpcServiceAccessLocalClient::Connect(...)#Failed to connect the requester client.");
			dispatcher->InitializeRpc(clientId);
			dispatcher->SetServerLocalClientId(serverClientId);
			return clientId;
		}

		remote_view_model_test::RemoteViewModelJsonDispatcherClient* GetDispatcher()
		{
			CHECK_ERROR(dispatcher, L"RpcServiceAccessLocalClient::GetDispatcher()#The client is not connected.");
			return dispatcher.Obj();
		}
	};

/***********************************************************************
RpcServerHelpers
***********************************************************************/

	void RpcServerHelpers::FinalizeRpcOnTaskQueue()
	{
		EventObject finalized;
		CHECK_ERROR(finalized.CreateAutoUnsignal(false), L"RpcServerHelpers::FinalizeRpcOnTaskQueue()#Failed to create the finalization event.");
		taskQueue->QueueTask(Func<void()>([&]()
		{
			requesterDispatcher->FinalizeRpc();
			finalized.Signal();
		}));
		finalized.Wait();
	}

	RpcServerHelpers::RpcServerHelpers(Ptr<glr::json::Parser> parser)
		: phase(RequesterPhase::Starting)
		, taskQueue(Ptr(new TaskQueue))
	{
		CHECK_ERROR(parser, L"RpcServerHelpers::RpcServerHelpers(...)#The JSON parser is null.");

		taskQueueThread = Ptr(new TaskQueueThread(taskQueue));
		broadcastingClient = Ptr(new RpcBroadcastingLocalClient(
			parser,
			Func<void(vint, const JsonPackage&)>([this](vint senderClientId, const JsonPackage& package)
			{
				OnControlMessage(senderClientId, package);
			})
			));
		requesterClient = Ptr(new RpcServiceAccessLocalClient(parser));
	}

	RpcServerHelpers::~RpcServerHelpers() = default;

	bool RpcServerHelpers::TryAcceptHost(vint clientId)
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

	void RpcServerHelpers::RegisterHost(vint clientId)
	{
		SPIN_LOCK(lockState)
		{
			CHECK_ERROR(
				hostId == clientId && !brokerRegistrationClaimed,
				L"RpcServerHelpers received Ready from an unexpected client."
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

	void RpcServerHelpers::OnControlMessage(vint senderClientId, const JsonPackage& package)
	{
		if (IsViewModelReadyMessage(package))
		{
			RegisterHost(senderClientId);
			return;
		}
		CHECK_ERROR(false, L"RpcServerHelpers received an unexpected control message.");
	}

	void RpcServerHelpers::OnClientDisconnected(vint clientId)
	{
		bool hostDisconnected = false;
		bool disconnectBroker = false;
		remote_view_model_test::RemoteViewModelJsonDispatcherClient* dispatcherToInject = nullptr;
		SPIN_LOCK(lockState)
		{
			if (hostId == clientId)
			{
				hostId = InvalidRemoteViewModelClientId;
				hostDisconnected = phase != RequesterPhase::Stopping;
				hostLossClaimed = hostDisconnected;
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
			SPIN_LOCK(lockState)
			{
				dispatcherToInject = requesterDispatcher;
				pendingHostLoss = !dispatcherToInject;
			}
			if (dispatcherToInject)
			{
				dispatcherToInject->InjectException(RemoteViewModelHostDisconnectedError);
			}
		}
	}

	void RpcServerHelpers::Start(JsonChannelServer* _channelServer)
	{
		CHECK_ERROR(_channelServer, L"RpcServerHelpers::Start(...)#The channel server is null.");
		CHECK_ERROR(!channelServer, L"RpcServerHelpers::Start(...)#The helpers have already started.");
		channelServer = _channelServer;
		broadcastingClient->Connect(channelServer, broadcastingClient, taskQueue);
		SPIN_LOCK(lockState)
		{
			brokerDispatcher = broadcastingClient->GetDispatcher();
		}
		SPIN_LOCK(lockState)
		{
			admissionReady = true;
		}
	}

	Ptr<IDescriptable> RpcServerHelpers::RequestService(const WString& typeName)
	{
		CHECK_ERROR(channelServer, L"RpcServerHelpers::RequestService(...)#The helpers have not started.");
		CHECK_ERROR(!rpcInitialized, L"RpcServerHelpers::RequestService(...)#The dispatcher is already initialized.");
		auto requesterClientId = requesterClient->Connect(
			channelServer,
			requesterClient,
			taskQueue,
			brokerDispatcher->GetServerClientId(),
			typeName
			);
		auto dispatcher = requesterClient->GetDispatcher();
		bool injectHostLoss = false;
		SPIN_LOCK(lockState)
		{
			requesterDispatcher = dispatcher;
			injectHostLoss = pendingHostLoss;
		}
		if (injectHostLoss)
		{
			dispatcher->InjectException(RemoteViewModelHostDisconnectedError);
		}
		brokerDispatcher->RegisterClient(requesterClientId);
		CHECK_ERROR(taskQueueThread->Start(), L"RpcServerHelpers::RequestService(...)#Failed to start the task queue thread.");
		rpcInitialized = true;
		dispatcher->Initialize();
		service = dispatcher
			->GetRpcLifecycle()
			->RequestService(typeName);
		CHECK_ERROR(service, L"RpcServerHelpers::RequestService(...)#Failed to request the service.");
		if (!BeginRunning())
		{
			throw rpc_controller::RpcInjectedException(RemoteViewModelHostDisconnectedError);
		}
		return service;
	}

	bool RpcServerHelpers::BeginRunning()
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

	bool RpcServerHelpers::CanAdmitRenderer()
	{
		bool result = false;
		SPIN_LOCK(lockState)
		{
			result = phase == RequesterPhase::Running;
		}
		return result;
	}

	void RpcServerHelpers::BeginStopping()
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

	void RpcServerHelpers::Stop(const Func<void()>& stopServer)
	{
		BeginStopping();
		bool finalizeRpc = false;
		SPIN_LOCK(lockState)
		{
			finalizeRpc = rpcInitialized && !hostLossClaimed;
		}
		if (finalizeRpc)
		{
			FinalizeRpcOnTaskQueue();
		}
		if (stopServer)
		{
			stopServer();
		}
		if (rpcInitialized)
		{
			taskQueue->QueueExitTask();
			taskQueueThread->Wait();
		}
		service = nullptr;
	}
}
