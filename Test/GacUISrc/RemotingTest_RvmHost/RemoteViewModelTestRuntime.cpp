#include "RemoteViewModelTestRuntime.h"
#include <chrono>

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

	namespace
	{
		vint64_t GetRemoteViewModelMonotonicTime()
		{
			return (vint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch()
				).count();
		}

		bool ParseControlSequence(const WString& value, const wchar_t* prefixText, vint64_t& sequence)
		{
			auto prefix = WString::Unmanaged(prefixText);
			if (
				value.Length() <= prefix.Length() ||
				value.Sub(0, prefix.Length()) != prefix
				)
			{
				return false;
			}

			bool success = false;
			sequence = wtoi64_test(value.Sub(prefix.Length(), value.Length() - prefix.Length()), success);
			return success && sequence >= 0;
		}

		void RecordFirstError(WString& error, const WString& message)
		{
			if (error == L"")
			{
				error = message;
			}
		}
	}

	JsonPackage CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind kind, vint64_t sequence)
	{
		auto package = Ptr(new glr::json::JsonString);
		switch (kind)
		{
		case RemoteViewModelControlMessageKind::Ready:
			package->content.value = WString::Unmanaged(ViewModelReadyMessage);
			break;
		case RemoteViewModelControlMessageKind::Heartbeat:
			CHECK_ERROR(sequence >= 0, L"CreateRemoteViewModelControlMessage(...)#A heartbeat requires a non-negative sequence.");
			package->content.value = WString::Unmanaged(ViewModelHeartbeatMessagePrefix) + i64tow(sequence);
			break;
		case RemoteViewModelControlMessageKind::HeartbeatAck:
			CHECK_ERROR(sequence >= 0, L"CreateRemoteViewModelControlMessage(...)#A heartbeat acknowledgement requires a non-negative sequence.");
			package->content.value = WString::Unmanaged(ViewModelHeartbeatAckMessagePrefix) + i64tow(sequence);
			break;
		case RemoteViewModelControlMessageKind::RequesterStopping:
			package->content.value = WString::Unmanaged(ViewModelRequesterStoppingMessage);
			break;
		case RemoteViewModelControlMessageKind::RequesterStoppingAck:
			package->content.value = WString::Unmanaged(ViewModelRequesterStoppingAckMessage);
			break;
		default:
			CHECK_FAIL(L"CreateRemoteViewModelControlMessage(...)#Unknown control message kind.");
		}
		return package;
	}

	bool ParseRemoteViewModelControlMessage(
		const JsonPackage& package,
		RemoteViewModelControlMessageKind& kind,
		vint64_t& sequence
		)
	{
		kind = RemoteViewModelControlMessageKind::Unknown;
		sequence = -1;
		auto jsonString = package.Cast<glr::json::JsonString>();
		if (!jsonString)
		{
			return false;
		}

		auto&& value = jsonString->content.value;
		if (value == WString::Unmanaged(ViewModelReadyMessage))
		{
			kind = RemoteViewModelControlMessageKind::Ready;
			return true;
		}
		if (value == WString::Unmanaged(ViewModelRequesterStoppingMessage))
		{
			kind = RemoteViewModelControlMessageKind::RequesterStopping;
			return true;
		}
		if (value == WString::Unmanaged(ViewModelRequesterStoppingAckMessage))
		{
			kind = RemoteViewModelControlMessageKind::RequesterStoppingAck;
			return true;
		}
		if (ParseControlSequence(value, ViewModelHeartbeatMessagePrefix, sequence))
		{
			kind = RemoteViewModelControlMessageKind::Heartbeat;
			return true;
		}
		if (ParseControlSequence(value, ViewModelHeartbeatAckMessagePrefix, sequence))
		{
			kind = RemoteViewModelControlMessageKind::HeartbeatAck;
			return true;
		}
		return false;
	}

	void RemoteViewModelBackgroundThread::Run()
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

	RemoteViewModelBackgroundThread::RemoteViewModelBackgroundThread(const Func<void()>& _callback)
		: callback(_callback)
	{
		CHECK_ERROR(callback, L"RemoteViewModelBackgroundThread::RemoteViewModelBackgroundThread(...)#The callback is null.");
		CHECK_ERROR(eventWake.CreateAutoUnsignal(false), L"RemoteViewModelBackgroundThread::RemoteViewModelBackgroundThread(...)#Failed to create the wake event.");
	}

	void RemoteViewModelBackgroundThread::Notify()
	{
		eventWake.Signal();
	}

	void RemoteViewModelBackgroundThread::StopAndWait()
	{
		bool shouldWait = false;
		SPIN_LOCK(lockState)
		{
			stopping = true;
			shouldWait = GetState() == ThreadState::Running;
		}
		eventWake.Signal();
		if (shouldWait)
		{
			Wait();
		}
	}

	RemoteViewModelJsonDispatcherClient::RemoteViewModelJsonDispatcherClient(Ptr<TaskQueue> taskQueue)
		: RpcJsonDispatcherClientForTaskQueue(taskQueue)
	{
	}

	void RemoteViewModelJsonDispatcherClient::InitializeRpc(vint clientId)
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

	RemoteViewModelBroadcastingLocalClient::RemoteViewModelBroadcastingLocalClient(Ptr<glr::json::Parser> parser)
		: JsonLocalChannelClient(parser)
	{
		channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
	}

	const JsonChannelClient::ChannelNameList& RemoteViewModelBroadcastingLocalClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	vint RemoteViewModelBroadcastingLocalClient::Connect(
		JsonChannelServer* channelServer,
		Ptr<JsonChannelClient> self,
		Ptr<TaskQueue> taskQueue
		)
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelBroadcastingLocalClient::Connect(...)#"
		CHECK_ERROR(channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
		CHECK_ERROR(self, ERROR_MESSAGE_PREFIX L"The shared local client is null.");

		clientId = channelServer->ConnectLocalClient(self);
		CHECK_ERROR(clientId != -1, ERROR_MESSAGE_PREFIX L"Failed to connect the broadcasting client.");
		dispatcher = Ptr(new RpcJsonDispatcherServerForTaskQueue(this, GetChannels()[WString::Unmanaged(ViewModelChannelName)], taskQueue));
		return clientId;
#undef ERROR_MESSAGE_PREFIX
	}

	RpcJsonDispatcherServer* RemoteViewModelBroadcastingLocalClient::GetDispatcher()
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelBroadcastingLocalClient::GetDispatcher()#The broadcasting client has not been connected.");
		return dispatcher.Obj();
	}

	RemoteViewModelRequesterLocalClient::RemoteViewModelRequesterLocalClient(Ptr<glr::json::Parser> parser)
		: JsonLocalChannelClient(parser)
	{
		channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
	}

	const JsonChannelClient::ChannelNameList& RemoteViewModelRequesterLocalClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	void RemoteViewModelRequesterLocalClient::OnConnected(vint clientId)
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelRequesterLocalClient::OnConnected(vint)#The dispatcher is null.");
		dispatcher->InitializeRpc(clientId);
	}

	vint RemoteViewModelRequesterLocalClient::Connect(
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
		dispatcher->SetServerLocalClientId(serverClientId);
		return clientId;
#undef ERROR_MESSAGE_PREFIX
	}

	RemoteViewModelJsonDispatcherClient* RemoteViewModelRequesterLocalClient::GetDispatcher()
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelRequesterLocalClient::GetDispatcher()#The requester client has not been connected.");
		return dispatcher.Obj();
	}

	RemoteViewModelReadyLocalClient::RemoteViewModelReadyLocalClient(
		Ptr<glr::json::Parser> parser,
		const Func<void(vint, const JsonPackage&)>& _readyCallback
		)
		: JsonLocalChannelClient(parser)
		, readyCallback(_readyCallback)
	{
		CHECK_ERROR(readyCallback, L"RemoteViewModelReadyLocalClient::RemoteViewModelReadyLocalClient(...)#The ready callback is null.");
		channelNames.Add(WString::Unmanaged(ViewModelReadyChannelName), nullptr);
	}

	const JsonChannelClient::ChannelNameList& RemoteViewModelReadyLocalClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	vint RemoteViewModelReadyLocalClient::Connect(
		JsonChannelServer* channelServer,
		Ptr<JsonChannelClient> self
		)
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelReadyLocalClient::Connect(...)#"
		CHECK_ERROR(channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
		CHECK_ERROR(self, ERROR_MESSAGE_PREFIX L"The shared local client is null.");

		auto clientId = channelServer->ConnectLocalClient(self);
		CHECK_ERROR(clientId != -1, ERROR_MESSAGE_PREFIX L"Failed to connect the ready client.");
		auto channel = GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
		CHECK_ERROR(channel, ERROR_MESSAGE_PREFIX L"The ready channel is null.");
		channel->Initialize(this);
		return clientId;
#undef ERROR_MESSAGE_PREFIX
	}

	void RemoteViewModelReadyLocalClient::OnRead(vint senderClientId, const JsonPackage& package)
	{
		readyCallback(senderClientId, package);
	}

	bool RemoteViewModelReadyLocalClient::SendToClient(vint clientId, const JsonPackage& package)
	{
		auto channel = GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
		CHECK_ERROR(channel, L"RemoteViewModelReadyLocalClient::SendToClient(...)#The ready channel is null.");
		channel->SendToClient(clientId, package);
		bool disconnected = false;
		channel->BatchWrite(disconnected);
		return !disconnected;
	}

	void RemoteViewModelTaskQueueThread::Run()
	{
		while (true)
		{
			try
			{
				taskQueue->RunTaskQueue();
				break;
			}
			catch (const Exception& e)
			{
				ReportFailure(e.Message());
			}
			catch (const Error& e)
			{
				ReportFailure(WString::Unmanaged(e.Description()));
			}
			catch (...)
			{
				ReportFailure(WString::Unmanaged(L"Unknown RPC task queue failure."));
			}
		}
	}

	RemoteViewModelTaskQueueThread::RemoteViewModelTaskQueueThread(Ptr<TaskQueue> _taskQueue)
		: taskQueue(_taskQueue)
	{
		CHECK_ERROR(taskQueue, L"RemoteViewModelTaskQueueThread::RemoteViewModelTaskQueueThread(Ptr<TaskQueue>)#The task queue is null.");
	}

	void RemoteViewModelTaskQueueThread::ReportFailure(const WString& message)
	{
		Func<void(const WString&)> callback;
		bool claimed = false;
		SPIN_LOCK(lockFailure)
		{
			if (!failureReported)
			{
				failureReported = true;
				failureMessage = message;
				callback = failureCallback;
				claimed = true;
			}
		}
		if (claimed && callback)
		{
			WString callbackFailure;
			try
			{
				callback(message);
			}
			catch (const Exception& e)
			{
				callbackFailure = e.Message();
			}
			catch (const Error& e)
			{
				callbackFailure = WString::Unmanaged(e.Description());
			}
			catch (...)
			{
				callbackFailure = WString::Unmanaged(L"Unknown failure callback error.");
			}
			if (callbackFailure != L"")
			{
				SPIN_LOCK(lockFailure)
				{
					failureMessage += WString::Unmanaged(L" Failure callback failed: ") + callbackFailure;
				}
			}
		}
	}

	void RemoteViewModelTaskQueueThread::SetFailureCallback(const Func<void(const WString&)>& callback)
	{
		CHECK_ERROR(GetState() == ThreadState::NotStarted, L"RemoteViewModelTaskQueueThread::SetFailureCallback(...)#The task queue thread has already started.");
		SPIN_LOCK(lockFailure)
		{
			failureCallback = callback;
		}
	}

	Nullable<WString> RemoteViewModelTaskQueueThread::GetFailure()
	{
		Nullable<WString> result;
		SPIN_LOCK(lockFailure)
		{
			if (failureReported)
			{
				result = failureMessage;
			}
		}
		return result;
	}

	RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(
		Ptr<glr::json::Parser> _parser,
		const Func<void(const WString&)>& _terminalAction
		)
		: parser(_parser)
		, terminalAction(_terminalAction)
	{
		CHECK_ERROR(parser, L"RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(...)#The JSON parser is null.");
		CHECK_ERROR(terminalAction, L"RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(...)#The terminal action is null.");
		CHECK_ERROR(roleState.EnableRemoteViewModel(), L"RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(...)#Failed to enable remote view model mode.");
		CHECK_ERROR(stoppingAcknowledged.CreateAutoUnsignal(false), L"RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(...)#Failed to create the stopping acknowledgement event.");

		taskQueue = Ptr(new TaskQueue);
		taskQueueThread = Ptr(new RemoteViewModelTaskQueueThread(taskQueue));
		broadcastingClient = Ptr(new RemoteViewModelBroadcastingLocalClient(parser));
		readyClient = Ptr(new RemoteViewModelReadyLocalClient(
			parser,
			Func<void(vint, const JsonPackage&)>([this](vint senderClientId, const JsonPackage& package)
			{
				OnReadyMessage(senderClientId, package);
			})
			));
		requesterClient = Ptr(new RemoteViewModelRequesterLocalClient(parser));
		stateThread = Ptr(new RemoteViewModelBackgroundThread(Func<void()>([this]()
		{
			ProcessState();
		})));
	}

	RemoteViewModelRequesterSession::~RemoteViewModelRequesterSession()
	{
		bool stopStateThread = false;
		bool stopTaskQueue = false;
		SPIN_LOCK(lockState)
		{
			stopStateThread = stateThreadStarted;
			stopTaskQueue = taskQueueStarted;
		}
		if (stopStateThread)
		{
			stateThread->StopAndWait();
			SPIN_LOCK(lockState)
			{
				stateThreadStarted = false;
			}
		}
		if (stopTaskQueue)
		{
			taskQueue->QueueExitTask();
			taskQueueThread->Wait();
			SPIN_LOCK(lockState)
			{
				taskQueueStarted = false;
			}
		}
	}

	bool RemoteViewModelRequesterSession::CanAcceptLocalClient(JsonChannelClient* localClient)
	{
		if (
			dynamic_cast<RemoteViewModelBroadcastingLocalClient*>(localClient) ||
			dynamic_cast<RemoteViewModelReadyLocalClient*>(localClient)
			)
		{
			return true;
		}
		if (dynamic_cast<RemoteViewModelRequesterLocalClient*>(localClient))
		{
			bool accept = false;
			SPIN_LOCK(lockState)
			{
				accept = brokerDispatcher && brokerDispatcher->HasServerClientId();
			}
			return accept;
		}
		return false;
	}

	bool RemoteViewModelRequesterSession::TryAcceptViewModelHost(vint clientId)
	{
		bool accepted = false;
		bool notifyStateThread = false;
		SPIN_LOCK(lockState)
		{
			if (
				admissionReady &&
				brokerDispatcher &&
				brokerDispatcher->HasServerClientId() &&
				roleState.TryAcceptViewModelHost(clientId)
				)
			{
				hostLease.Start(GetRemoteViewModelMonotonicTime(), RemoteViewModelStartupGraceMilliseconds);
				accepted = true;
				notifyStateThread = stateThreadStarted;
			}
		}
		if (accepted && notifyStateThread)
		{
			stateThread->Notify();
		}
		return accepted;
	}

	void RemoteViewModelRequesterSession::RegisterViewModelHost(vint clientId)
	{
		SPIN_LOCK(lockState)
		{
			if (
				roleState.GetViewModelHostId() == clientId &&
				brokerViewModelHostId == InvalidRemoteViewModelClientId
				)
			{
				CHECK_ERROR(brokerDispatcher, L"RemoteViewModelRequesterSession::RegisterViewModelHost(...)#The broker dispatcher is null.");
				brokerDispatcher->RegisterClient(clientId);
				brokerViewModelHostId = clientId;
			}
		}
	}

	void RemoteViewModelRequesterSession::OnReadyMessage(vint senderClientId, const JsonPackage& package)
	{
		RemoteViewModelControlMessageKind kind;
		vint64_t sequence = -1;
		if (!ParseRemoteViewModelControlMessage(package, kind, sequence))
		{
			return;
		}

		if (kind == RemoteViewModelControlMessageKind::Ready)
		{
			RegisterViewModelHost(senderClientId);
			return;
		}

		if (kind == RemoteViewModelControlMessageKind::Heartbeat)
		{
			RegisterViewModelHost(senderClientId);
			bool acknowledged = false;
			SPIN_LOCK(lockState)
			{
				if (roleState.GetViewModelHostId() == senderClientId)
				{
					acknowledged = hostLease.Renew(
						sequence,
						GetRemoteViewModelMonotonicTime(),
						RemoteViewModelLeaseTimeoutMilliseconds
						);
				}
			}
			if (acknowledged)
			{
				try
				{
					if (!readyClient->SendToClient(
						senderClientId,
						CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind::HeartbeatAck, sequence)
						))
					{
						ReportFailure(WString::Unmanaged(RemoteViewModelHostDisconnectedError));
					}
				}
				catch (...)
				{
					ReportFailure(WString::Unmanaged(RemoteViewModelHostDisconnectedError));
				}
			}
		}
		else if (kind == RemoteViewModelControlMessageKind::RequesterStoppingAck)
		{
			bool acknowledged = false;
			SPIN_LOCK(lockState)
			{
				acknowledged = roleState.GetViewModelHostId() == senderClientId;
			}
			if (acknowledged)
			{
				stoppingAcknowledged.Signal();
			}
		}
	}

	void RemoteViewModelRequesterSession::OnClientDisconnected(vint clientId)
	{
		RpcJsonDispatcherServer* targetBroker = nullptr;
		bool brokerClient = false;
		bool notifyStateThread = false;
		SPIN_LOCK(lockState)
		{
			targetBroker = brokerDispatcher;
			brokerClient = roleState.GetViewModelHostId() == clientId;
			if (roleState.FailViewModelHost(clientId))
			{
				hostLease.Stop();
				if (terminalState.TryClaimFailure(WString::Unmanaged(RemoteViewModelHostDisconnectedError)))
				{
					notifyStateThread = stateThreadStarted;
				}
			}
			if (brokerViewModelHostId == clientId)
			{
				brokerViewModelHostId = InvalidRemoteViewModelClientId;
			}
			if (roleState.DisconnectRequester(clientId))
			{
				brokerClient = true;
			}
		}

		if (brokerClient && targetBroker)
		{
			targetBroker->DisconnectClient(clientId);
		}
		if (notifyStateThread)
		{
			stateThread->Notify();
		}
	}

	void RemoteViewModelRequesterSession::ReportFailure(const WString& message)
	{
		bool claimed = false;
		bool notifyStateThread = false;
		SPIN_LOCK(lockState)
		{
			if (terminalState.TryClaimFailure(message))
			{
				roleState.FailRemoteViewModel();
				hostLease.Stop();
				claimed = true;
				notifyStateThread = stateThreadStarted;
			}
		}
		if (claimed && notifyStateThread)
		{
			stateThread->Notify();
		}
	}

	void RemoteViewModelRequesterSession::ProcessState()
	{
		bool invokeTerminal = false;
		bool normal = false;
		WString message;
		SPIN_LOCK(lockState)
		{
			if (hostLease.IsExpired(GetRemoteViewModelMonotonicTime()))
			{
				auto hostId = roleState.GetViewModelHostId();
				if (hostId != InvalidRemoteViewModelClientId)
				{
					roleState.FailViewModelHost(hostId);
				}
				hostLease.Stop();
				terminalState.TryClaimFailure(WString::Unmanaged(RemoteViewModelHostDisconnectedError));
			}
			invokeTerminal = terminalState.TryTake(normal, message);
		}
		if (invokeTerminal)
		{
			terminalAction(message);
		}
	}

	void RemoteViewModelRequesterSession::Start(JsonChannelServer* _channelServer)
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelRequesterSession::Start(JsonChannelServer*)#"
		CHECK_ERROR(_channelServer, ERROR_MESSAGE_PREFIX L"The channel server is null.");
		CHECK_ERROR(!channelServer, ERROR_MESSAGE_PREFIX L"The requester session has already started.");
		channelServer = _channelServer;

		broadcastingClient->Connect(channelServer, broadcastingClient, taskQueue);
		SPIN_LOCK(lockState)
		{
			brokerDispatcher = broadcastingClient->GetDispatcher();
		}
		readyClient->Connect(channelServer, readyClient);

		auto requesterClientId = requesterClient->Connect(
			channelServer,
			requesterClient,
			taskQueue,
			brokerDispatcher->GetServerClientId()
			);
		requesterDispatcher = requesterClient->GetDispatcher();
		SPIN_LOCK(lockState)
		{
			CHECK_ERROR(roleState.RegisterRequester(requesterClientId), ERROR_MESSAGE_PREFIX L"A requester has already been registered.");
		}
		brokerDispatcher->RegisterClient(requesterClientId);

		taskQueueThread->SetFailureCallback(Func<void(const WString&)>([this](const WString& message)
		{
			ReportFailure(WString::Unmanaged(L"RPC task queue failed: ") + message);
		}));

		CHECK_ERROR(stateThread->Start(), ERROR_MESSAGE_PREFIX L"Failed to start the RVM state thread.");
		SPIN_LOCK(lockState)
		{
			stateThreadStarted = true;
		}
		CHECK_ERROR(taskQueueThread->Start(), ERROR_MESSAGE_PREFIX L"Failed to start the RPC task queue thread.");
		SPIN_LOCK(lockState)
		{
			taskQueueStarted = true;
			admissionReady = true;
		}
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<rvmt::IViewModel> RemoteViewModelRequesterSession::RequestViewModel()
	{
#define ERROR_MESSAGE_PREFIX L"RemoteViewModelRequesterSession::RequestViewModel()#"
		CHECK_ERROR(requesterDispatcher, ERROR_MESSAGE_PREFIX L"The requester session has not started.");
		CHECK_ERROR(!rpcInitialized, ERROR_MESSAGE_PREFIX L"The requester dispatcher has already been initialized.");
		rpcInitialized = true;
		requesterDispatcher->Initialize();
		viewModel = requesterDispatcher
			->GetRpcLifecycle()
			->RequestService(WString::Unmanaged(ViewModelServiceName))
			.Cast<rvmt::IViewModel>();
		CHECK_ERROR(viewModel, ERROR_MESSAGE_PREFIX L"Failed to request rvmt::IViewModel.");
		return viewModel;
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<rvmt::IViewModel> RemoteViewModelRequesterSession::GetViewModel()
	{
		return viewModel;
	}

	bool RemoteViewModelRequesterSession::BeginRunning()
	{
		bool succeeded = false;
		SPIN_LOCK(lockState)
		{
			succeeded = !terminalState.HasFailure() && roleState.BeginRunning();
		}
		return succeeded;
	}

	bool RemoteViewModelRequesterSession::CanAdmitRenderer()
	{
		bool accepted = false;
		SPIN_LOCK(lockState)
		{
			accepted = roleState.CanAdmitRenderer() && !terminalState.HasFailure();
		}
		return accepted;
	}

	void RemoteViewModelRequesterSession::BeginStopping()
	{
		vint hostId = InvalidRemoteViewModelClientId;
		bool sendStopping = false;
		SPIN_LOCK(lockState)
		{
			if (terminalState.BeginStopping())
			{
				hostId = roleState.GetViewModelHostId();
				roleState.BeginStopping();
				hostLease.Stop();
				sendStopping = hostId != InvalidRemoteViewModelClientId;
			}
		}
		if (sendStopping)
		{
			try
			{
				if (readyClient->SendToClient(
					hostId,
					CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind::RequesterStopping)
					))
				{
					stoppingAcknowledged.WaitForTime(RemoteViewModelTerminalDeliveryMilliseconds);
				}
			}
			catch (...)
			{
			}
		}
	}

	Nullable<WString> RemoteViewModelRequesterSession::GetFatalError()
	{
		Nullable<WString> result;
		SPIN_LOCK(lockState)
		{
			if (terminalState.HasFailure())
			{
				result = terminalState.GetMessage();
			}
		}
		return result;
	}

	bool RemoteViewModelRequesterSession::IsRemoteViewModelClientId(vint clientId)
	{
		bool result = false;
		SPIN_LOCK(lockState)
		{
			result =
				roleState.GetViewModelHostId() == clientId ||
				roleState.GetRequesterClientId() == clientId;
		}
		return result;
	}

	Nullable<WString> RemoteViewModelRequesterSession::Stop(const Func<void()>& stopServer)
	{
		WString error;
		if (stopped)
		{
			return {};
		}
		BeginStopping();

		if (requesterDispatcher)
		{
			try
			{
				if (taskQueueStarted)
				{
					FinalizeRpcOnTaskQueue(taskQueue, requesterDispatcher);
				}
				else
				{
					requesterDispatcher->FinalizeRpc();
				}
			}
			catch (const Exception& e)
			{
				RecordFirstError(error, WString::Unmanaged(L"RPC cleanup failed: ") + e.Message());
			}
			catch (const Error& e)
			{
				RecordFirstError(error, WString::Unmanaged(L"RPC cleanup failed: ") + WString::Unmanaged(e.Description()));
			}
			catch (...)
			{
				RecordFirstError(error, WString::Unmanaged(L"RPC cleanup failed with an unknown error."));
			}
		}

		try
		{
			if (stopServer)
			{
				stopServer();
			}
		}
		catch (const Exception& e)
		{
			RecordFirstError(error, WString::Unmanaged(L"Channel cleanup failed: ") + e.Message());
		}
		catch (const Error& e)
		{
			RecordFirstError(error, WString::Unmanaged(L"Channel cleanup failed: ") + WString::Unmanaged(e.Description()));
		}
		catch (...)
		{
			RecordFirstError(error, WString::Unmanaged(L"Channel cleanup failed with an unknown error."));
		}

		bool stopStateThread = false;
		bool stopTaskQueue = false;
		SPIN_LOCK(lockState)
		{
			admissionReady = false;
			stopStateThread = stateThreadStarted;
			stopTaskQueue = taskQueueStarted;
		}
		if (stopStateThread)
		{
			stateThread->StopAndWait();
			SPIN_LOCK(lockState)
			{
				stateThreadStarted = false;
			}
		}
		if (stopTaskQueue)
		{
			if (auto failure = StopRpcTaskQueue(taskQueue, taskQueueThread.Obj()))
			{
				RecordFirstError(error, WString::Unmanaged(L"RPC task queue cleanup failed: ") + failure.Value());
			}
			SPIN_LOCK(lockState)
			{
				taskQueueStarted = false;
			}
		}

		viewModel = nullptr;
		stopped = true;
		if (error != L"")
		{
			return error;
		}
		return {};
	}

	RemoteViewModelHostingClient::RemoteViewModelHostingClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue,
		const Func<void(const WString&, bool)>& _terminalAction
		)
		: JsonNetworkChannelClient(networkClient, parser)
		, terminalAction(_terminalAction)
	{
		CHECK_ERROR(terminalAction, L"RemoteViewModelHostingClient::RemoteViewModelHostingClient(...)#The terminal action is null.");
		channelNames.Add(WString::Unmanaged(ViewModelChannelName), nullptr);
		channelNames.Add(WString::Unmanaged(ViewModelReadyChannelName), nullptr);
		dispatcher = Ptr(new RemoteViewModelJsonDispatcherClient(taskQueue));
		stateThread = Ptr(new RemoteViewModelBackgroundThread(Func<void()>([this]()
		{
			ProcessState();
		})));
	}

	RemoteViewModelHostingClient::~RemoteViewModelHostingClient()
	{
		BeginStopping();
	}

	const JsonChannelClient::ChannelNameList& RemoteViewModelHostingClient::OnGetChannelNames()
	{
		return channelNames.Keys();
	}

	void RemoteViewModelHostingClient::OnConnected(vint clientId)
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelHostingClient::OnConnected(vint)#The dispatcher is null.");
		dispatcher->InitializeRpc(clientId);
		auto targetReadyChannel = GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
		CHECK_ERROR(targetReadyChannel, L"RemoteViewModelHostingClient::OnConnected(vint)#The ready channel is null.");
		targetReadyChannel->Initialize(this);
		SPIN_LOCK(lockState)
		{
			readyChannel = targetReadyChannel;
			requesterLease.Start(GetRemoteViewModelMonotonicTime(), RemoteViewModelStartupGraceMilliseconds);
		}
		CHECK_ERROR(stateThread->Start(), L"RemoteViewModelHostingClient::OnConnected(vint)#Failed to start the host state thread.");
		SPIN_LOCK(lockState)
		{
			stateThreadStarted = true;
		}
		stateThread->Notify();
	}

	void RemoteViewModelHostingClient::ReportFailure(const WString& message)
	{
		bool claimed = false;
		bool notifyStateThread = false;
		SPIN_LOCK(lockState)
		{
			if (terminalState.TryClaimFailure(message))
			{
				requesterLease.Stop();
				claimed = true;
				notifyStateThread = stateThreadStarted;
			}
		}
		if (claimed && notifyStateThread)
		{
			stateThread->Notify();
		}
	}

	void RemoteViewModelHostingClient::OnDisconnected()
	{
		ReportFailure(WString::Unmanaged(RemoteViewModelRequesterDisconnectedError));
	}

	void RemoteViewModelHostingClient::OnReadError(const WString& errorMessage)
	{
		ReportFailure(errorMessage);
	}

	void RemoteViewModelHostingClient::OnLocalError(const WString& errorMessage, bool fatal)
	{
		if (fatal)
		{
			ReportFailure(errorMessage);
		}
	}

	void RemoteViewModelHostingClient::OnRead(vint senderClientId, const JsonPackage& package)
	{
		RemoteViewModelControlMessageKind kind;
		vint64_t sequence = -1;
		if (!ParseRemoteViewModelControlMessage(package, kind, sequence))
		{
			return;
		}

		if (kind == RemoteViewModelControlMessageKind::HeartbeatAck)
		{
			SPIN_LOCK(lockState)
			{
				if (requesterReadyClientId == InvalidRemoteViewModelClientId)
				{
					requesterReadyClientId = senderClientId;
				}
				if (
					requesterReadyClientId == senderClientId &&
					sequence <= heartbeatSequence
					)
				{
					requesterLease.Renew(
						sequence,
						GetRemoteViewModelMonotonicTime(),
						RemoteViewModelLeaseTimeoutMilliseconds
						);
				}
			}
		}
		else if (kind == RemoteViewModelControlMessageKind::RequesterStopping)
		{
			bool accepted = false;
			JsonChannel* targetReadyChannel = nullptr;
			SPIN_LOCK(lockState)
			{
				if (requesterReadyClientId == InvalidRemoteViewModelClientId)
				{
					requesterReadyClientId = senderClientId;
				}
				accepted = requesterReadyClientId == senderClientId;
				targetReadyChannel = readyChannel;
			}
			if (accepted && targetReadyChannel)
			{
				try
				{
					targetReadyChannel->SendToClient(
						senderClientId,
						CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind::RequesterStoppingAck)
						);
					bool disconnected = false;
					targetReadyChannel->BatchWrite(disconnected);
				}
				catch (...)
				{
				}

				bool claimed = false;
				bool notifyStateThread = false;
				SPIN_LOCK(lockState)
				{
					if (terminalState.TryClaimNormal())
					{
						requesterLease.Stop();
						claimed = true;
						notifyStateThread = stateThreadStarted;
					}
				}
				if (claimed && notifyStateThread)
				{
					stateThread->Notify();
				}
			}
		}
	}

	void RemoteViewModelHostingClient::ProcessState()
	{
		bool invokeTerminal = false;
		bool normal = false;
		WString message;
		JsonChannel* targetChannel = nullptr;
		vint64_t sequence = -1;
		SPIN_LOCK(lockState)
		{
			if (requesterLease.IsExpired(GetRemoteViewModelMonotonicTime()))
			{
				requesterLease.Stop();
				terminalState.TryClaimFailure(WString::Unmanaged(RemoteViewModelRequesterDisconnectedError));
			}
			invokeTerminal = terminalState.TryTake(normal, message);
			if (!invokeTerminal && requesterLease.IsActive())
			{
				targetChannel = readyChannel;
				sequence = ++heartbeatSequence;
			}
		}

		if (invokeTerminal)
		{
			terminalAction(message, normal);
			return;
		}
		if (targetChannel)
		{
			try
			{
				targetChannel->BroadcastFromClient(
					CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind::Heartbeat, sequence)
					);
				bool disconnected = false;
				targetChannel->BatchWrite(disconnected);
				if (disconnected)
				{
					ReportFailure(WString::Unmanaged(RemoteViewModelRequesterDisconnectedError));
				}
			}
			catch (...)
			{
				ReportFailure(WString::Unmanaged(RemoteViewModelRequesterDisconnectedError));
			}
		}
	}

	void RemoteViewModelHostingClient::Connect()
	{
		List<WString> waitingForServices;
		dispatcher->WaitForServer(this, GetChannels()[WString::Unmanaged(ViewModelChannelName)], waitingForServices);

		auto readyChannel = GetChannels()[WString::Unmanaged(ViewModelReadyChannelName)];
		CHECK_ERROR(readyChannel, L"RemoteViewModelHostingClient::Connect()#The ready channel is null.");
		readyChannel->BroadcastFromClient(CreateRemoteViewModelControlMessage(RemoteViewModelControlMessageKind::Ready));
		bool disconnected = false;
		readyChannel->BatchWrite(disconnected);
		CHECK_ERROR(!disconnected, L"RemoteViewModelHostingClient::Connect()#The ready channel disconnected.");
	}

	void RemoteViewModelHostingClient::BeginStopping()
	{
		bool stopThread = false;
		SPIN_LOCK(lockState)
		{
			terminalState.BeginStopping();
			requesterLease.Stop();
			stopThread = stateThreadStarted;
		}
		if (stopThread)
		{
			stateThread->StopAndWait();
			SPIN_LOCK(lockState)
			{
				stateThreadStarted = false;
			}
		}
	}

	RemoteViewModelJsonDispatcherClient* RemoteViewModelHostingClient::GetDispatcher()
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelHostingClient::GetDispatcher()#The dispatcher is null.");
		return dispatcher.Obj();
	}

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
		WString errorMessage;
		taskQueue->QueueTask(Func<void()>([&]()
		{
			try
			{
				dispatcher->FinalizeRpc();
			}
			catch (const Exception& e)
			{
				errorMessage = e.Message();
			}
			catch (const Error& e)
			{
				errorMessage = WString::Unmanaged(e.Description());
			}
			catch (...)
			{
				errorMessage = WString::Unmanaged(L"Unknown RPC finalization failure.");
			}
			finalized.Signal();
		}));
		finalized.Wait();
		if (errorMessage != L"")
		{
			throw Exception(errorMessage);
		}
#undef ERROR_MESSAGE_PREFIX
	}

	Nullable<WString> StopRpcTaskQueue(Ptr<TaskQueue> taskQueue, RemoteViewModelTaskQueueThread* taskQueueThread)
	{
#define ERROR_MESSAGE_PREFIX L"StopRpcTaskQueue(...)#"
		CHECK_ERROR(taskQueue, ERROR_MESSAGE_PREFIX L"The task queue is null.");
		CHECK_ERROR(taskQueueThread, ERROR_MESSAGE_PREFIX L"The task queue thread is null.");
		taskQueue->QueueExitTask();
		taskQueueThread->Wait();
		return taskQueueThread->GetFailure();
#undef ERROR_MESSAGE_PREFIX
	}
}
