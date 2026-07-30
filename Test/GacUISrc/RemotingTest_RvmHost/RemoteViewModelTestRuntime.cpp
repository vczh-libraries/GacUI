#include "RemoteViewModelTestRuntime.h"

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

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
		const Func<void(vint)>& _readyCallback
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

	void RemoteViewModelReadyLocalClient::OnRead(vint senderClientId, const JsonPackage&)
	{
		readyCallback(senderClientId);
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
