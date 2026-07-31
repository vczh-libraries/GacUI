#include "RemotingClient.h"
#include <chrono>
#include <cstdlib>

namespace vl::presentation::remoting
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

		vint64_t GetMonotonicTime()
		{
			return (vint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch()
				).count();
		}

		[[noreturn]] void ExitProcess(vint exitCode)
		{
			std::_Exit((int)exitCode);
		}

		JsonPackage CreateControlMessage(const WString& message)
		{
			auto package = Ptr(new glr::json::JsonString);
			package->content.value = message;
			return package;
		}

		bool IsControlMessage(const JsonPackage& package, const WString& message)
		{
			auto jsonString = package.Cast<glr::json::JsonString>();
			return jsonString && jsonString->content.value == message;
		}

		class RepeatingThread : public Thread
		{
		private:
			SpinLock											lockState;
			EventObject											eventWake;
			Func<void()>										callback;
			vint												interval;
			bool												stopping = false;

		protected:
			void Run() override
			{
				try
				{
					while (true)
					{
						eventWake.WaitForTime(interval);
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
					ExitProcess(1);
				}
			}

		public:
			RepeatingThread(vint _interval, const Func<void()>& _callback)
				: callback(_callback)
				, interval(_interval)
			{
				CHECK_ERROR(interval > 0, L"RepeatingThread::RepeatingThread(...)#The interval should be positive.");
				CHECK_ERROR(callback, L"RepeatingThread::RepeatingThread(...)#The callback is null.");
				CHECK_ERROR(eventWake.CreateAutoUnsignal(false), L"RepeatingThread::RepeatingThread(...)#Failed to create the wake event.");
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
					ExitProcess(1);
				}
			}

		public:
			TaskQueueThread(Ptr<TaskQueue> _taskQueue)
				: taskQueue(_taskQueue)
			{
				CHECK_ERROR(taskQueue, L"TaskQueueThread::TaskQueueThread(...)#The task queue is null.");
			}
		};

		class BroadcastingLocalClient
			: public JsonLocalChannelClient
			, protected inter_process::IChannelReader<JsonPackage>
		{
		private:
			JsonChannelClient::ChannelMap						channelNames;
			WString												rpcChannelName;
			WString												controlChannelName;
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
				const RemotingRpcConfiguration& configuration,
				const Func<void(vint, const JsonPackage&)>& _controlCallback
				)
				: JsonLocalChannelClient(parser)
				, rpcChannelName(configuration.rpcChannelName)
				, controlChannelName(configuration.controlChannelName)
				, controlCallback(_controlCallback)
			{
				CHECK_ERROR(controlCallback, L"BroadcastingLocalClient::BroadcastingLocalClient(...)#The control callback is null.");
				channelNames.Add(rpcChannelName, nullptr);
				channelNames.Add(controlChannelName, nullptr);
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
				controlChannel = GetChannels()[controlChannelName];
				CHECK_ERROR(controlChannel, L"BroadcastingLocalClient::Connect(...)#The control channel is null.");
				controlChannel->Initialize(this);
				dispatcher = Ptr(new RpcJsonDispatcherServerForTaskQueue(
					this,
					GetChannels()[rpcChannelName],
					taskQueue
					));
				return clientId;
			}

			RpcJsonDispatcherServer* GetDispatcher()
			{
				CHECK_ERROR(dispatcher, L"BroadcastingLocalClient::GetDispatcher()#The client is not connected.");
				return dispatcher.Obj();
			}

			void SendToClient(vint clientId, const JsonPackage& package)
			{
				CHECK_ERROR(controlChannel, L"BroadcastingLocalClient::SendToClient(...)#The control channel is null.");
				controlChannel->SendToClient(clientId, package);
				bool disconnected = false;
				controlChannel->BatchWrite(disconnected);
			}
		};

		class RequesterLocalClient : public JsonLocalChannelClient
		{
		private:
			JsonChannelClient::ChannelMap						channelNames;
			WString												rpcChannelName;
			WString												serviceName;
			RemotingDispatcherFactory							dispatcherFactory;
			Ptr<RemotingJsonDispatcherClient>					dispatcher;

		public:
			RequesterLocalClient(
				Ptr<glr::json::Parser> parser,
				const RemotingRpcConfiguration& configuration,
				const RemotingDispatcherFactory& _dispatcherFactory
				)
				: JsonLocalChannelClient(parser)
				, rpcChannelName(configuration.rpcChannelName)
				, serviceName(configuration.serviceName)
				, dispatcherFactory(_dispatcherFactory)
			{
				CHECK_ERROR(dispatcherFactory, L"RequesterLocalClient::RequesterLocalClient(...)#The dispatcher factory is null.");
				channelNames.Add(rpcChannelName, nullptr);
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
				waitingForServices.Add(serviceName);
				dispatcher = dispatcherFactory(taskQueue);
				CHECK_ERROR(dispatcher, L"RequesterLocalClient::Connect(...)#The dispatcher factory returned null.");
				auto clientId = dispatcher->ConnectLocalServer(
					channelServer,
					self,
					GetChannels()[rpcChannelName],
					waitingForServices
					);
				CHECK_ERROR(clientId != -1, L"RequesterLocalClient::Connect(...)#Failed to connect the requester client.");
				dispatcher->InitializeRpc(clientId);
				dispatcher->SetServerLocalClientId(serverClientId);
				return clientId;
			}

			RemotingJsonDispatcherClient* GetDispatcher()
			{
				CHECK_ERROR(dispatcher, L"RequesterLocalClient::GetDispatcher()#The client is not connected.");
				return dispatcher.Obj();
			}
		};

		void FinalizeRpcOnTaskQueue(Ptr<TaskQueue> taskQueue, RemotingJsonDispatcherClient* dispatcher)
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
	}

	RemotingJsonDispatcherClient::RemotingJsonDispatcherClient(Ptr<TaskQueue> taskQueue)
		: RpcJsonDispatcherClientForTaskQueue(taskQueue)
	{
	}

	class RemotingRequesterSession::Impl : public Object
	{
	public:
		RemotingRpcConfiguration							configuration;
		SpinLock											lockState;
		CriticalSection										lockBroker;
		Func<void(const WString&)>							terminalAction;
		RequesterPhase										phase = RequesterPhase::Starting;
		vint												hostId;
		bool												hostEverAccepted = false;
		bool												brokerRegistrationClaimed = false;
		bool												hostLeaseActive = false;
		vint64_t											hostLeaseExpiration = -1;
		bool												fatalClaimed = false;
		bool												fatalTaken = false;
		WString												fatalMessage;
		bool												admissionReady = false;
		bool												rpcInitialized = false;

		RpcJsonDispatcherServer*							brokerDispatcher = nullptr;
		Ptr<TaskQueue>										taskQueue;
		Ptr<TaskQueueThread>								taskQueueThread;
		Ptr<BroadcastingLocalClient>						broadcastingClient;
		Ptr<RequesterLocalClient>							requesterClient;
		RemotingJsonDispatcherClient*						requesterDispatcher = nullptr;
		Ptr<IDescriptable>									service;
		Ptr<RepeatingThread>								stateThread;

		Impl(
			const RemotingRpcConfiguration& _configuration,
			const RemotingDispatcherFactory& dispatcherFactory,
			Ptr<glr::json::Parser> parser,
			const Func<void(const WString&)>& _terminalAction
			)
			: configuration(_configuration)
			, terminalAction(_terminalAction)
			, hostId(_configuration.invalidClientId)
		{
			CHECK_ERROR(parser, L"RemotingRequesterSession::Impl::Impl(...)#The JSON parser is null.");
			CHECK_ERROR(terminalAction, L"RemotingRequesterSession::Impl::Impl(...)#The terminal action is null.");
			CHECK_ERROR(configuration.rpcChannelName != L"", L"RemotingRequesterSession::Impl::Impl(...)#The RPC channel name is empty.");
			CHECK_ERROR(configuration.controlChannelName != L"", L"RemotingRequesterSession::Impl::Impl(...)#The control channel name is empty.");
			CHECK_ERROR(configuration.serviceName != L"", L"RemotingRequesterSession::Impl::Impl(...)#The service name is empty.");

			taskQueue = Ptr(new TaskQueue);
			taskQueueThread = Ptr(new TaskQueueThread(taskQueue));
			broadcastingClient = Ptr(new BroadcastingLocalClient(
				parser,
				configuration,
				Func<void(vint, const JsonPackage&)>([this](vint senderClientId, const JsonPackage& package)
				{
					OnControlMessage(senderClientId, package);
				})
				));
			requesterClient = Ptr(new RequesterLocalClient(parser, configuration, dispatcherFactory));
			stateThread = Ptr(new RepeatingThread(
				configuration.heartbeatIntervalMilliseconds,
				Func<void()>([this]() { ProcessState(); })
				));
		}

		bool TryAcceptHost(vint clientId)
		{
			bool accepted = false;
			SPIN_LOCK(lockState)
			{
				if (
					admissionReady &&
					phase == RequesterPhase::Starting &&
					!hostEverAccepted &&
					!fatalClaimed &&
					clientId != configuration.invalidClientId
					)
				{
					hostId = clientId;
					hostEverAccepted = true;
					hostLeaseActive = true;
					hostLeaseExpiration = GetMonotonicTime() + configuration.startupGraceMilliseconds;
					accepted = true;
				}
			}
			if (accepted)
			{
				stateThread->Notify();
			}
			return accepted;
		}

		void RegisterHost(vint clientId)
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

		void OnControlMessage(vint senderClientId, const JsonPackage& package)
		{
			if (IsControlMessage(package, configuration.readyMessage))
			{
				RegisterHost(senderClientId);
				return;
			}
			CHECK_ERROR(
				IsControlMessage(package, configuration.heartbeatMessage),
				L"RemotingRequesterSession received an unexpected control message."
				);
			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(hostId == senderClientId, L"RemotingRequesterSession received a heartbeat from an unexpected client.");
				if (hostLeaseActive)
				{
					hostLeaseExpiration = GetMonotonicTime() + configuration.leaseTimeoutMilliseconds;
				}
			}
		}

		void ClaimHostLoss()
		{
			bool notify = false;
			SPIN_LOCK(lockState)
			{
				if (phase != RequesterPhase::Stopping && !fatalClaimed)
				{
					fatalClaimed = true;
					fatalMessage = configuration.hostDisconnectedError;
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
				if (hostId == clientId)
				{
					hostId = configuration.invalidClientId;
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
				if (hostLeaseActive && GetMonotonicTime() >= hostLeaseExpiration)
				{
					hostId = configuration.invalidClientId;
					hostLeaseActive = false;
					if (phase != RequesterPhase::Stopping && !fatalClaimed)
					{
						fatalClaimed = true;
						fatalMessage = configuration.hostDisconnectedError;
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
			vint currentHostId = configuration.invalidClientId;
			SPIN_LOCK(lockState)
			{
				if (phase == RequesterPhase::Stopping || fatalClaimed)
				{
					return;
				}
				phase = RequesterPhase::Stopping;
				hostLeaseActive = false;
				currentHostId = hostId;
			}
			if (currentHostId != configuration.invalidClientId)
			{
				broadcastingClient->SendToClient(
					currentHostId,
					CreateControlMessage(configuration.requesterStoppingMessage)
					);
			}
		}
	};

	RemotingRequesterSession::RemotingRequesterSession(
		const RemotingRpcConfiguration& configuration,
		const RemotingDispatcherFactory& dispatcherFactory,
		Ptr<glr::json::Parser> parser,
		const Func<void(const WString&)>& terminalAction
		)
		: impl(Ptr(new Impl(configuration, dispatcherFactory, parser, terminalAction)))
	{
	}

	RemotingRequesterSession::~RemotingRequesterSession()
	{
	}

	bool RemotingRequesterSession::CanAcceptLocalClient(JsonChannelClient* localClient)
	{
		if (dynamic_cast<BroadcastingLocalClient*>(localClient))
		{
			return true;
		}
		if (dynamic_cast<RequesterLocalClient*>(localClient))
		{
			SPIN_LOCK(impl->lockState)
			{
				return impl->brokerDispatcher && impl->brokerDispatcher->HasServerClientId();
			}
		}
		return false;
	}

	bool RemotingRequesterSession::TryAcceptHost(vint clientId)
	{
		return impl->TryAcceptHost(clientId);
	}

	void RemotingRequesterSession::OnClientDisconnected(vint clientId)
	{
		impl->OnClientDisconnected(clientId);
	}

	void RemotingRequesterSession::Start(JsonChannelServer* channelServer)
	{
		CHECK_ERROR(channelServer, L"RemotingRequesterSession::Start(...)#The channel server is null.");
		CHECK_ERROR(!impl->brokerDispatcher, L"RemotingRequesterSession::Start(...)#The session has already started.");
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
		CHECK_ERROR(impl->stateThread->Start(), L"RemotingRequesterSession::Start(...)#Failed to start the state thread.");
		CHECK_ERROR(impl->taskQueueThread->Start(), L"RemotingRequesterSession::Start(...)#Failed to start the task queue thread.");
		SPIN_LOCK(impl->lockState)
		{
			impl->admissionReady = true;
		}
	}

	Ptr<IDescriptable> RemotingRequesterSession::RequestService()
	{
		CHECK_ERROR(impl->requesterDispatcher, L"RemotingRequesterSession::RequestService()#The session has not started.");
		CHECK_ERROR(!impl->rpcInitialized, L"RemotingRequesterSession::RequestService()#The dispatcher is already initialized.");
		impl->rpcInitialized = true;
		impl->requesterDispatcher->Initialize();
		impl->service = impl->requesterDispatcher
			->GetRpcLifecycle()
			->RequestService(impl->configuration.serviceName);
		CHECK_ERROR(impl->service, L"RemotingRequesterSession::RequestService()#Failed to request the service.");
		return impl->service;
	}

	bool RemotingRequesterSession::BeginRunning()
	{
		SPIN_LOCK(impl->lockState)
		{
			if (
				impl->phase == RequesterPhase::Starting &&
				!impl->fatalClaimed &&
				impl->hostId != impl->configuration.invalidClientId
				)
			{
				impl->phase = RequesterPhase::Running;
				return true;
			}
		}
		return false;
	}

	bool RemotingRequesterSession::CanAdmitRenderer()
	{
		bool result = false;
		SPIN_LOCK(impl->lockState)
		{
			result = impl->phase == RequesterPhase::Running && !impl->fatalClaimed;
		}
		return result;
	}

	void RemotingRequesterSession::BeginStopping()
	{
		impl->BeginStopping();
	}

	Nullable<WString> RemotingRequesterSession::GetFatalError()
	{
		SPIN_LOCK(impl->lockState)
		{
			if (impl->fatalClaimed)
			{
				return impl->fatalMessage;
			}
		}
		return {};
	}

	void RemotingRequesterSession::Stop(const Func<void()>& stopServer)
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
		impl->service = nullptr;
	}

	class RemotingHostingClient::Impl : public Object
	{
	public:
		RemotingRpcConfiguration							configuration;
		JsonChannelClient::ChannelMap						channelNames;
		Ptr<RemotingJsonDispatcherClient>					dispatcher;
		JsonChannel*										controlChannel = nullptr;
		Ptr<RepeatingThread>								heartbeatThread;

		Impl(
			const RemotingRpcConfiguration& _configuration,
			const RemotingDispatcherFactory& dispatcherFactory,
			Ptr<TaskQueue> taskQueue
			)
			: configuration(_configuration)
		{
			channelNames.Add(configuration.rpcChannelName, nullptr);
			channelNames.Add(configuration.controlChannelName, nullptr);
			dispatcher = dispatcherFactory(taskQueue);
			CHECK_ERROR(dispatcher, L"RemotingHostingClient::Impl::Impl(...)#The dispatcher factory returned null.");
			heartbeatThread = Ptr(new RepeatingThread(
				configuration.heartbeatIntervalMilliseconds,
				Func<void()>([this]() { SendHeartbeat(); })
				));
		}

		void Connect(JsonChannelClient* channelClient, inter_process::IChannelReader<JsonPackage>* controlReader)
		{
			List<WString> waitingForServices;
			dispatcher->WaitForServer(
				channelClient,
				channelClient->GetChannels()[configuration.rpcChannelName],
				waitingForServices
				);
			dispatcher->InitializeRpc(channelClient->GetClientId());
			controlChannel = channelClient->GetChannels()[configuration.controlChannelName];
			CHECK_ERROR(controlChannel, L"RemotingHostingClient::Impl::Connect(...)#The control channel is null.");
			controlChannel->Initialize(controlReader);
		}

		void FlushControlMessage(const WString& message)
		{
			CHECK_ERROR(controlChannel, L"RemotingHostingClient::Impl::FlushControlMessage(...)#The control channel is null.");
			controlChannel->BroadcastFromClient(CreateControlMessage(message));
			bool disconnected = false;
			controlChannel->BatchWrite(disconnected);
			if (disconnected)
			{
				ExitProcess(1);
			}
		}

		void SendHeartbeat()
		{
			FlushControlMessage(configuration.heartbeatMessage);
		}

		void SendReady()
		{
			CHECK_ERROR(heartbeatThread->GetState() == Thread::NotStarted, L"RemotingHostingClient::SendReady()#The heartbeat has already started.");
			FlushControlMessage(configuration.readyMessage);
		}

		void StartHeartbeat()
		{
			CHECK_ERROR(heartbeatThread->GetState() == Thread::NotStarted, L"RemotingHostingClient::StartHeartbeat()#The heartbeat has already started.");
			CHECK_ERROR(heartbeatThread->Start(), L"RemotingHostingClient::StartHeartbeat()#Failed to start the heartbeat thread.");
		}
	};

	RemotingHostingClient::RemotingHostingClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		const RemotingRpcConfiguration& configuration,
		const RemotingDispatcherFactory& dispatcherFactory,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue
		)
		: JsonNetworkChannelClient(networkClient, parser)
		, impl(Ptr(new Impl(configuration, dispatcherFactory, taskQueue)))
	{
	}

	RemotingHostingClient::~RemotingHostingClient()
	{
	}

	const JsonChannelClient::ChannelNameList& RemotingHostingClient::OnGetChannelNames()
	{
		return impl->channelNames.Keys();
	}

	void RemotingHostingClient::OnConnected(vint)
	{
	}

	void RemotingHostingClient::OnDisconnected()
	{
		ExitProcess(1);
	}

	void RemotingHostingClient::OnReadError(const WString&)
	{
		ExitProcess(1);
	}

	void RemotingHostingClient::OnLocalError(const WString&, bool fatal)
	{
		if (fatal)
		{
			ExitProcess(1);
		}
	}

	void RemotingHostingClient::OnRead(vint, const JsonPackage& package)
	{
		if (IsControlMessage(package, impl->configuration.requesterStoppingMessage))
		{
			ExitProcess(0);
		}
		ExitProcess(1);
	}

	void RemotingHostingClient::Connect()
	{
		impl->Connect(this, this);
	}

	void RemotingHostingClient::StartHeartbeat()
	{
		impl->StartHeartbeat();
	}

	void RemotingHostingClient::SendReady()
	{
		impl->SendReady();
	}

	RpcDispatcherClient* RemotingHostingClient::GetDispatcher()
	{
		return impl->dispatcher.Obj();
	}
}
