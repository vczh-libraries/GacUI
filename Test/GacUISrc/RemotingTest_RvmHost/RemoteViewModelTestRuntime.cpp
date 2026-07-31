#include "RemoteViewModelTestRuntime.h"
#include <chrono>
#include <cstdlib>

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

	namespace
	{
		enum class RequesterPhase
		{
			Starting,
			Running,
			Stopping,
		};

		vint64_t GetRemoteViewModelMonotonicTime()
		{
			return (vint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch()
				).count();
		}

		[[noreturn]] void ExitRemoteViewModelProcess(vint exitCode)
		{
			std::_Exit((int)exitCode);
		}

		JsonPackage CreateControlMessage(const wchar_t* message)
		{
			auto package = Ptr(new glr::json::JsonString);
			package->content.value = WString::Unmanaged(message);
			return package;
		}

		bool IsControlMessage(const JsonPackage& package, const wchar_t* message)
		{
			auto jsonString = package.Cast<glr::json::JsonString>();
			return
				jsonString &&
				jsonString->content.value == WString::Unmanaged(message);
		}

		class RemoteViewModelRepeatingThread : public Thread
		{
		private:
			SpinLock										lockState;
			EventObject										eventWake;
			Func<void()>									callback;
			bool											stopping = false;

		protected:
			void Run() override
			{
				try
				{
					while (true)
					{
						eventWake.WaitForTime(RemoteViewModelHeartbeatIntervalMilliseconds);
						bool shouldStop = false;
						SPIN_LOCK(lockState)
						{
							shouldStop = stopping;
						}
						if (shouldStop)
						{
							return;
						}
						callback();
					}
				}
				catch (...)
				{
					ExitRemoteViewModelProcess(1);
				}
			}

		public:
			RemoteViewModelRepeatingThread(const Func<void()>& _callback)
				: callback(_callback)
			{
				CHECK_ERROR(callback, L"RemoteViewModelRepeatingThread::RemoteViewModelRepeatingThread(...)#The callback is null.");
				CHECK_ERROR(eventWake.CreateAutoUnsignal(false), L"RemoteViewModelRepeatingThread::RemoteViewModelRepeatingThread(...)#Failed to create the wake event.");
			}

			void Notify()
			{
				eventWake.Signal();
			}

			void StopAndWait()
			{
				SPIN_LOCK(lockState)
				{
					stopping = true;
				}
				eventWake.Signal();
				Wait();
			}
		};

		class RemoteViewModelTaskQueueThread : public Thread
		{
		private:
			Ptr<TaskQueue>									taskQueue;

		protected:
			void Run() override
			{
				try
				{
					taskQueue->RunTaskQueue();
				}
				catch (...)
				{
					ExitRemoteViewModelProcess(1);
				}
			}

		public:
			RemoteViewModelTaskQueueThread(Ptr<TaskQueue> _taskQueue)
				: taskQueue(_taskQueue)
			{
				CHECK_ERROR(taskQueue, L"RemoteViewModelTaskQueueThread::RemoteViewModelTaskQueueThread(...)#The task queue is null.");
			}
		};

		class RemoteViewModelJsonDispatcherClient
			: public RpcJsonDispatcherClientForTaskQueue
		{
		public:
			RemoteViewModelJsonDispatcherClient(Ptr<TaskQueue> taskQueue)
				: RpcJsonDispatcherClientForTaskQueue(taskQueue)
			{
			}

			void InitializeRpc(vint clientId)
			{
				auto& app = vl_workflow_global::RemoteViewModelTestRpc::Instance();
				auto rpcDispatcher = Ptr(new RpcJsonDispatcher(clientId, this));
				auto lifecycle = Ptr(new RpcJsonLifecycle(clientId, rpcDispatcher.Obj()));
				SetRpcObjects(rpcDispatcher, lifecycle);

				auto idMap = UnboxParameter<Dictionary<WString, vint>>(BoxParameter(app.rpc_GetIds()));
				lifecycle->SetIdMap(idMap.Ref());

				auto serializer = app.rpcops_IRpcSerializer();
				auto objectOps = app.rpcops_IRpcObjectOpsJson(lifecycle.Obj());
				auto objectEventOps = app.rpcops_IRpcObjectEventOpsJson(lifecycle.Obj());
				auto ops = app.rpcops_IOps_CreateJson(lifecycle.Obj());

				lifecycle->Register(
					serializer,
					objectOps,
					objectEventOps,
					Func<vint(IDescriptable*)>([](IDescriptable* obj)
					{
						return vl_workflow_global::RemoteViewModelTestRpc::Instance().rpcwrapper_GetTypeId(BoxValue<IDescriptable*>(obj));
					}),
					Func<void(RpcObjectReference, IDescriptable*)>()
					);

				lifecycle->RegisterWrapperFactory(Func<Ptr<IRpcWrapperBase>(RpcObjectReference, IRpcLifecycle*)>(
					[ops](RpcObjectReference ref, IRpcLifecycle* lifecycle)
					{
						return vl_workflow_global::RemoteViewModelTestRpc::Instance().rpcwrapper_Create(ref, lifecycle, ops);
					}));
			}
		};

		class RemoteViewModelBroadcastingLocalClient
			: public JsonLocalChannelClient
			, protected inter_process::IChannelReader<JsonPackage>
		{
		private:
			JsonChannelClient::ChannelMap					channelNames;
			Ptr<RpcJsonDispatcherServerForTaskQueue>		dispatcher;
			Func<void(vint, const JsonPackage&)>			controlCallback;
			JsonChannel*									controlChannel = nullptr;

		protected:
			void OnRead(vint senderClientId, const JsonPackage& package) override
			{
				controlCallback(senderClientId, package);
			}

		public:
			RemoteViewModelBroadcastingLocalClient(
				Ptr<glr::json::Parser> parser,
				const Func<void(vint, const JsonPackage&)>& _controlCallback
				)
				: JsonLocalChannelClient(parser)
				, controlCallback(_controlCallback)
			{
				CHECK_ERROR(controlCallback, L"RemoteViewModelBroadcastingLocalClient::RemoteViewModelBroadcastingLocalClient(...)#The control callback is null.");
				channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
				channelNames.Add(WString::Unmanaged(ViewModelReadyChannelName), nullptr);
			}

			const JsonChannelClient::ChannelNameList& OnGetChannelNames() override
			{
				return channelNames.Keys();
			}

			vint Connect(
				JsonChannelServer* channelServer,
				Ptr<JsonChannelClient> self,
				Ptr<TaskQueue> taskQueue
				)
			{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelBroadcastingLocalClient::Connect(...)#"
				CHECK_ERROR(channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
				CHECK_ERROR(self, ERROR_MESSAGE_PREFIX L"The shared local client is null.");

				auto clientId = channelServer->ConnectLocalClient(self);
				CHECK_ERROR(clientId != -1, ERROR_MESSAGE_PREFIX L"Failed to connect the broadcasting client.");
				controlChannel = GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
				CHECK_ERROR(controlChannel, ERROR_MESSAGE_PREFIX L"The control channel is null.");
				controlChannel->Initialize(this);
				dispatcher = Ptr(new RpcJsonDispatcherServerForTaskQueue(
					this,
					GetChannels()[WString::Unmanaged(ViewModelChannelName)],
					taskQueue
					));
				return clientId;
#undef ERROR_MESSAGE_PREFIX
			}

			RpcJsonDispatcherServer* GetDispatcher()
			{
				CHECK_ERROR(dispatcher, L"RemoteViewModelBroadcastingLocalClient::GetDispatcher()#The broadcasting client has not been connected.");
				return dispatcher.Obj();
			}

			void SendToClient(vint clientId, const JsonPackage& package)
			{
				CHECK_ERROR(controlChannel, L"RemoteViewModelBroadcastingLocalClient::SendToClient(...)#The control channel is null.");
				controlChannel->SendToClient(clientId, package);
				bool disconnected = false;
				controlChannel->BatchWrite(disconnected);
			}
		};

		class RemoteViewModelRequesterLocalClient : public JsonLocalChannelClient
		{
		private:
			JsonChannelClient::ChannelMap					channelNames;
			Ptr<RemoteViewModelJsonDispatcherClient>		dispatcher;

		public:
			RemoteViewModelRequesterLocalClient(Ptr<glr::json::Parser> parser)
				: JsonLocalChannelClient(parser)
			{
				channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
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
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelRequesterLocalClient::Connect(...)#"
				CHECK_ERROR(channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
				CHECK_ERROR(self, ERROR_MESSAGE_PREFIX L"The shared local client is null.");

				List<WString> waitingForServices;
				waitingForServices.Add(WString::Unmanaged(ViewModelServiceName));
				dispatcher = Ptr(new RemoteViewModelJsonDispatcherClient(taskQueue));
				auto clientId = dispatcher->ConnectLocalServer(
					channelServer,
					self,
					GetChannels()[WString::Unmanaged(ViewModelChannelName)],
					waitingForServices
					);
				CHECK_ERROR(clientId != -1, ERROR_MESSAGE_PREFIX L"Failed to connect the requester client.");
				dispatcher->InitializeRpc(clientId);
				dispatcher->SetServerLocalClientId(serverClientId);
				return clientId;
#undef ERROR_MESSAGE_PREFIX
			}

			RemoteViewModelJsonDispatcherClient* GetDispatcher()
			{
				CHECK_ERROR(dispatcher, L"RemoteViewModelRequesterLocalClient::GetDispatcher()#The requester client has not been connected.");
				return dispatcher.Obj();
			}
		};

		void FinalizeRpcOnTaskQueue(
			Ptr<TaskQueue> taskQueue,
			RemoteViewModelJsonDispatcherClient* dispatcher
			)
		{
#define ERROR_MESSAGE_PREFIX L"FinalizeRpcOnTaskQueue(...)#"
			CHECK_ERROR(taskQueue, ERROR_MESSAGE_PREFIX L"The task queue is null.");
			CHECK_ERROR(dispatcher, ERROR_MESSAGE_PREFIX L"The dispatcher is null.");

			EventObject finalized;
			CHECK_ERROR(finalized.CreateAutoUnsignal(false), ERROR_MESSAGE_PREFIX L"Failed to create the finalization event.");
			taskQueue->QueueTask(Func<void()>([&]()
			{
				dispatcher->FinalizeRpc();
				finalized.Signal();
			}));
			finalized.Wait();
#undef ERROR_MESSAGE_PREFIX
		}
	}

	class RemoteViewModelRequesterSession::Impl : public Object
	{
	public:
		SpinLock										lockState;
		CriticalSection									lockBroker;
		Func<void(const WString&)>						terminalAction;
		RequesterPhase									phase = RequesterPhase::Starting;
		vint											viewModelHostId = InvalidRemoteViewModelClientId;
		bool											viewModelHostEverAccepted = false;
		bool											brokerRegistrationClaimed = false;
		bool											hostLeaseActive = false;
		vint64_t										hostLeaseExpiration = -1;
		bool											fatalClaimed = false;
		bool											fatalTaken = false;
		WString											fatalMessage;
		bool											admissionReady = false;
		bool											rpcInitialized = false;

		RpcJsonDispatcherServer*						brokerDispatcher = nullptr;
		Ptr<TaskQueue>									taskQueue;
		Ptr<RemoteViewModelTaskQueueThread>				taskQueueThread;
		Ptr<RemoteViewModelBroadcastingLocalClient>		broadcastingClient;
		Ptr<RemoteViewModelRequesterLocalClient>			requesterClient;
		RemoteViewModelJsonDispatcherClient*			requesterDispatcher = nullptr;
		Ptr<rvmt::IViewModel>							viewModel;
		Ptr<RemoteViewModelRepeatingThread>				stateThread;

		Impl(
			Ptr<glr::json::Parser> _parser,
			const Func<void(const WString&)>& _terminalAction
			)
			: terminalAction(_terminalAction)
		{
			CHECK_ERROR(_parser, L"RemoteViewModelRequesterSession::Impl::Impl(...)#The JSON parser is null.");
			CHECK_ERROR(terminalAction, L"RemoteViewModelRequesterSession::Impl::Impl(...)#The terminal action is null.");

			taskQueue = Ptr(new TaskQueue);
			taskQueueThread = Ptr(new RemoteViewModelTaskQueueThread(taskQueue));
			broadcastingClient = Ptr(new RemoteViewModelBroadcastingLocalClient(
				_parser,
				Func<void(vint, const JsonPackage&)>([this](vint senderClientId, const JsonPackage& package)
				{
					OnControlMessage(senderClientId, package);
				})
				));
			requesterClient = Ptr(new RemoteViewModelRequesterLocalClient(_parser));
			stateThread = Ptr(new RemoteViewModelRepeatingThread(Func<void()>([this]()
			{
				ProcessState();
			})));
		}

		bool TryAcceptViewModelHost(vint clientId)
		{
			bool accepted = false;
			SPIN_LOCK(lockState)
			{
				if (
					admissionReady &&
					phase == RequesterPhase::Starting &&
					!viewModelHostEverAccepted &&
					!fatalClaimed &&
					clientId != InvalidRemoteViewModelClientId
					)
				{
					viewModelHostId = clientId;
					viewModelHostEverAccepted = true;
					hostLeaseActive = true;
					hostLeaseExpiration =
						GetRemoteViewModelMonotonicTime() +
						RemoteViewModelStartupGraceMilliseconds;
					accepted = true;
				}
			}
			if (accepted)
			{
				stateThread->Notify();
			}
			return accepted;
		}

		void RegisterViewModelHost(vint clientId)
		{
			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(
					viewModelHostId == clientId &&
					!brokerRegistrationClaimed,
					L"RemoteViewModelRequesterSession received Ready from an unexpected client."
					);
				brokerRegistrationClaimed = true;
			}

			CS_LOCK(lockBroker)
			{
				bool shouldRegister = false;
				SPIN_LOCK(lockState)
				{
					shouldRegister =
						viewModelHostId == clientId &&
						brokerRegistrationClaimed;
				}
				if (shouldRegister)
				{
					brokerDispatcher->RegisterClient(clientId);
				}
			}
		}

		void OnControlMessage(vint senderClientId, const JsonPackage& package)
		{
			if (IsControlMessage(package, ViewModelReadyMessage))
			{
				RegisterViewModelHost(senderClientId);
				return;
			}

			CHECK_ERROR(
				IsControlMessage(package, ViewModelHeartbeatMessage),
				L"RemoteViewModelRequesterSession received an unexpected control message."
				);

			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(
					viewModelHostId == senderClientId,
					L"RemoteViewModelRequesterSession received a heartbeat from an unexpected client."
					);
				if (hostLeaseActive)
				{
					hostLeaseExpiration =
						GetRemoteViewModelMonotonicTime() +
						RemoteViewModelLeaseTimeoutMilliseconds;
				}
			}
		}

		void ClaimHostLoss()
		{
			bool notify = false;
			SPIN_LOCK(lockState)
			{
				if (
					phase != RequesterPhase::Stopping &&
					!fatalClaimed
					)
				{
					fatalClaimed = true;
					fatalMessage = WString::Unmanaged(RemoteViewModelHostDisconnectedError);
					hostLeaseActive = false;
					notify = true;
				}
			}
			if (notify)
			{
				stateThread->Notify();
			}
		}

		void OnClientDisconnected(vint clientId)
		{
			bool hostDisconnected = false;
			bool disconnectBroker = false;
			SPIN_LOCK(lockState)
			{
				if (viewModelHostId == clientId)
				{
					viewModelHostId = InvalidRemoteViewModelClientId;
					hostLeaseActive = false;
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
				ClaimHostLoss();
			}
		}

		void ProcessState()
		{
			bool invokeTerminal = false;
			WString message;
			SPIN_LOCK(lockState)
			{
				if (
					hostLeaseActive &&
					GetRemoteViewModelMonotonicTime() >= hostLeaseExpiration
					)
				{
					viewModelHostId = InvalidRemoteViewModelClientId;
					hostLeaseActive = false;
					if (phase != RequesterPhase::Stopping && !fatalClaimed)
					{
						fatalClaimed = true;
						fatalMessage = WString::Unmanaged(RemoteViewModelHostDisconnectedError);
					}
				}
				if (fatalClaimed && !fatalTaken)
				{
					fatalTaken = true;
					message = fatalMessage;
					invokeTerminal = true;
				}
			}
			if (invokeTerminal)
			{
				terminalAction(message);
			}
		}

		void BeginStopping()
		{
			vint hostId = InvalidRemoteViewModelClientId;
			SPIN_LOCK(lockState)
			{
				if (phase == RequesterPhase::Stopping || fatalClaimed)
				{
					return;
				}
				phase = RequesterPhase::Stopping;
				hostLeaseActive = false;
				hostId = viewModelHostId;
			}
			if (hostId != InvalidRemoteViewModelClientId)
			{
				broadcastingClient->SendToClient(
					hostId,
					CreateControlMessage(ViewModelRequesterStoppingMessage)
					);
			}
		}
	};

	RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(
		Ptr<glr::json::Parser> parser,
		const Func<void(const WString&)>& terminalAction
		)
		: impl(Ptr(new Impl(parser, terminalAction)))
	{
	}

	RemoteViewModelRequesterSession::~RemoteViewModelRequesterSession()
	{
	}

	bool RemoteViewModelRequesterSession::CanAcceptLocalClient(JsonChannelClient* localClient)
	{
		if (dynamic_cast<RemoteViewModelBroadcastingLocalClient*>(localClient))
		{
			return true;
		}
		if (dynamic_cast<RemoteViewModelRequesterLocalClient*>(localClient))
		{
			bool accept = false;
			SPIN_LOCK(impl->lockState)
			{
				accept = impl->brokerDispatcher && impl->brokerDispatcher->HasServerClientId();
			}
			return accept;
		}
		return false;
	}

	bool RemoteViewModelRequesterSession::TryAcceptViewModelHost(vint clientId)
	{
		return impl->TryAcceptViewModelHost(clientId);
	}

	void RemoteViewModelRequesterSession::OnClientDisconnected(vint clientId)
	{
		impl->OnClientDisconnected(clientId);
	}

	void RemoteViewModelRequesterSession::Start(JsonChannelServer* channelServer)
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelRequesterSession::Start(JsonChannelServer*)#"
		CHECK_ERROR(channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
		CHECK_ERROR(!impl->brokerDispatcher, ERROR_MESSAGE_PREFIX L"The requester session has already started.");

		impl->broadcastingClient->Connect(channelServer, impl->broadcastingClient, impl->taskQueue);
		SPIN_LOCK(impl->lockState)
		{
			impl->brokerDispatcher = impl->broadcastingClient->GetDispatcher();
		}

		auto requesterClientId = impl->requesterClient->Connect(
			channelServer,
			impl->requesterClient,
			impl->taskQueue,
			impl->brokerDispatcher->GetServerClientId()
			);
		impl->requesterDispatcher = impl->requesterClient->GetDispatcher();
		impl->brokerDispatcher->RegisterClient(requesterClientId);

		CHECK_ERROR(impl->stateThread->Start(), ERROR_MESSAGE_PREFIX L"Failed to start the RVM state thread.");
		CHECK_ERROR(impl->taskQueueThread->Start(), ERROR_MESSAGE_PREFIX L"Failed to start the RPC task queue thread.");
		SPIN_LOCK(impl->lockState)
		{
			impl->admissionReady = true;
		}
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<rvmt::IViewModel> RemoteViewModelRequesterSession::RequestViewModel()
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelRequesterSession::RequestViewModel()#"
		CHECK_ERROR(impl->requesterDispatcher, ERROR_MESSAGE_PREFIX L"The requester session has not started.");
		CHECK_ERROR(!impl->rpcInitialized, ERROR_MESSAGE_PREFIX L"The requester dispatcher has already been initialized.");
		impl->rpcInitialized = true;
		impl->requesterDispatcher->Initialize();
		impl->viewModel = impl->requesterDispatcher
			->GetRpcLifecycle()
			->RequestService(WString::Unmanaged(ViewModelServiceName))
			.Cast<rvmt::IViewModel>();
		CHECK_ERROR(impl->viewModel, ERROR_MESSAGE_PREFIX L"Failed to request rvmt::IViewModel.");
		return impl->viewModel;
#undef ERROR_MESSAGE_PREFIX
	}

	bool RemoteViewModelRequesterSession::BeginRunning()
	{
		bool succeeded = false;
		SPIN_LOCK(impl->lockState)
		{
			if (
				impl->phase == RequesterPhase::Starting &&
				!impl->fatalClaimed &&
				impl->viewModelHostId != InvalidRemoteViewModelClientId
				)
			{
				impl->phase = RequesterPhase::Running;
				succeeded = true;
			}
		}
		return succeeded;
	}

	bool RemoteViewModelRequesterSession::CanAdmitRenderer()
	{
		bool accepted = false;
		SPIN_LOCK(impl->lockState)
		{
			accepted =
				impl->phase == RequesterPhase::Running &&
				!impl->fatalClaimed;
		}
		return accepted;
	}

	void RemoteViewModelRequesterSession::BeginStopping()
	{
		impl->BeginStopping();
	}

	Nullable<WString> RemoteViewModelRequesterSession::GetFatalError()
	{
		Nullable<WString> result;
		SPIN_LOCK(impl->lockState)
		{
			if (impl->fatalClaimed)
			{
				result = impl->fatalMessage;
			}
		}
		return result;
	}

	void RemoteViewModelRequesterSession::Stop(const Func<void()>& stopServer)
	{
		impl->BeginStopping();
		if (impl->requesterDispatcher)
		{
			FinalizeRpcOnTaskQueue(impl->taskQueue, impl->requesterDispatcher);
		}
		if (stopServer)
		{
			stopServer();
		}
		impl->stateThread->StopAndWait();
		impl->taskQueue->QueueExitTask();
		impl->taskQueueThread->Wait();
		impl->viewModel = nullptr;
	}

	class RemoteViewModelHostingClient::Impl : public Object
	{
	public:
		JsonChannelClient::ChannelMap					channelNames;
		Ptr<RemoteViewModelJsonDispatcherClient>		dispatcher;
		JsonChannel*									controlChannel = nullptr;
		Ptr<RemoteViewModelRepeatingThread>				heartbeatThread;

		Impl(Ptr<TaskQueue> taskQueue)
		{
			channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
			channelNames.Add(WString::Unmanaged(ViewModelReadyChannelName), nullptr);
			dispatcher = Ptr(new RemoteViewModelJsonDispatcherClient(taskQueue));
			heartbeatThread = Ptr(new RemoteViewModelRepeatingThread(Func<void()>([this]()
			{
				SendHeartbeat();
			})));
		}

		~Impl()
		{
			if (heartbeatThread->GetState() == Thread::Running)
			{
				heartbeatThread->StopAndWait();
			}
		}

		void Connect(
			JsonChannelClient* channelClient,
			inter_process::IChannelReader<JsonPackage>* controlReader
			)
		{
			List<WString> waitingForServices;
			dispatcher->WaitForServer(
				channelClient,
				channelClient->GetChannels()[WString::Unmanaged(ViewModelChannelName)],
				waitingForServices
				);
			dispatcher->InitializeRpc(channelClient->GetClientId());
			controlChannel = channelClient->GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
			CHECK_ERROR(controlChannel, L"RemoteViewModelHostingClient::Connect()#The control channel is null.");
			controlChannel->Initialize(controlReader);
		}

		void FlushControlMessage(const wchar_t* message)
		{
			CHECK_ERROR(controlChannel, L"RemoteViewModelHostingClient control channel is null.");
			controlChannel->BroadcastFromClient(CreateControlMessage(message));
			bool disconnected = false;
			controlChannel->BatchWrite(disconnected);
			if (disconnected)
			{
				ExitRemoteViewModelProcess(1);
			}
		}

		void SendHeartbeat()
		{
			FlushControlMessage(ViewModelHeartbeatMessage);
		}

		void SendReady()
		{
			CHECK_ERROR(heartbeatThread->GetState() == Thread::NotStarted, L"RemoteViewModelHostingClient::SendReady()#The heartbeat has already started.");
			FlushControlMessage(ViewModelReadyMessage);
		}

		void StartHeartbeat()
		{
			CHECK_ERROR(heartbeatThread->GetState() == Thread::NotStarted, L"RemoteViewModelHostingClient::StartHeartbeat()#The heartbeat has already started.");
			CHECK_ERROR(heartbeatThread->Start(), L"RemoteViewModelHostingClient::StartHeartbeat()#Failed to start the heartbeat thread.");
		}
	};

	RemoteViewModelHostingClient::RemoteViewModelHostingClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue
		)
		: JsonNetworkChannelClient(networkClient, parser)
		, impl(Ptr(new Impl(taskQueue)))
	{
	}

	RemoteViewModelHostingClient::~RemoteViewModelHostingClient()
	{
	}

	const JsonChannelClient::ChannelNameList& RemoteViewModelHostingClient::OnGetChannelNames()
	{
		return impl->channelNames.Keys();
	}

	void RemoteViewModelHostingClient::OnConnected(vint)
	{
	}

	void RemoteViewModelHostingClient::OnDisconnected()
	{
		ExitRemoteViewModelProcess(1);
	}

	void RemoteViewModelHostingClient::OnReadError(const WString&)
	{
		ExitRemoteViewModelProcess(1);
	}

	void RemoteViewModelHostingClient::OnLocalError(const WString&, bool fatal)
	{
		if (fatal)
		{
			ExitRemoteViewModelProcess(1);
		}
	}

	void RemoteViewModelHostingClient::OnRead(vint, const JsonPackage& package)
	{
		if (IsControlMessage(package, ViewModelRequesterStoppingMessage))
		{
			ExitRemoteViewModelProcess(0);
		}
		ExitRemoteViewModelProcess(1);
	}

	void RemoteViewModelHostingClient::Connect()
	{
		impl->Connect(this, this);
	}

	void RemoteViewModelHostingClient::StartHeartbeat()
	{
		impl->StartHeartbeat();
	}

	void RemoteViewModelHostingClient::SendReady()
	{
		impl->SendReady();
	}

	RpcJsonDispatcherClient* RemoteViewModelHostingClient::GetDispatcher()
	{
		return impl->dispatcher.Obj();
	}
}
