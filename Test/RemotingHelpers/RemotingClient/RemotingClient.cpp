#include "RemotingClient.h"
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
		bool												admissionReady = false;
		bool												rpcInitialized = false;

		RpcJsonDispatcherServer*							brokerDispatcher = nullptr;
		Ptr<TaskQueue>										taskQueue;
		Ptr<TaskQueueThread>								taskQueueThread;
		Ptr<BroadcastingLocalClient>						broadcastingClient;
		Ptr<RequesterLocalClient>							requesterClient;
		RemotingJsonDispatcherClient*						requesterDispatcher = nullptr;
		Ptr<IDescriptable>									service;

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
					clientId != configuration.invalidClientId
					)
				{
					hostId = clientId;
					hostEverAccepted = true;
					accepted = true;
				}
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
			CHECK_ERROR(false, L"RemotingRequesterSession received an unexpected control message.");
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
				terminalAction(configuration.hostDisconnectedError);
			}
		}

		void BeginStopping()
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
			result = impl->phase == RequesterPhase::Running;
		}
		return result;
	}

	void RemotingRequesterSession::BeginStopping()
	{
		impl->BeginStopping();
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
		}

		void Connect(JsonChannelClient* channelClient)
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

		void SendReady()
		{
			FlushControlMessage(configuration.readyMessage);
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

	void RemotingHostingClient::Connect()
	{
		impl->Connect(this);
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
