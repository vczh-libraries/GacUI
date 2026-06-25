# JSON RPC Channel Setup

The JSON RPC layer separates RPC object semantics from data transmission. RpcJsonDispatcher, RpcJsonLifecycle, RpcJsonObjectOps, and RpcJsonObjectEventOps know how to turn RPC operations into JSON nodes. IRpcJsonMessageDispatcher is the boundary that sends those JSON nodes to another endpoint and receives JSON nodes back.

Use this page when wiring a JSON RPC endpoint to a transport. Implement IRpcJsonMessageDispatcher for a custom transport. Use the channeling classes when vl::inter_process should provide the transport.

## IRpcJsonMessageDispatcher Contract

IRpcJsonMessageDispatcher has two runtime responsibilities:
- AllocateRequestId returns a unique request id for outgoing JSON RPC messages from this endpoint.
- OnJsonRequest sends one JSON request according to the RequestType and returns the response JSON node when a response is expected.

RequestType describes the routing behavior:
- Direct sends a request to the target client and waits for the matching response.
- Broadcast sends a request through the server-side broker and waits for the broker response.
- BroadcastAndDrop sends a broadcast message that does not produce a response. Service declarations use this mode.

When a transport receives a JSON RPC request for the local endpoint, call IRpcJsonMessageDispatcher::DefaultTranslate with the local object operations, event operations, dispatcher and lifecycle. DefaultTranslate invokes the local RPC operation and returns the response JSON node when the request needs one.

A custom transport usually follows this shape:
```C++
using namespace vl;
using namespace vl::glr::json;
using namespace vl::rpc_controller;

class MyJsonMessageDispatcher : public Object, public virtual IRpcJsonMessageDispatcher
{
private:
	atomic_vint nextRequestId = 0;
	IRpcObjectOps* objectOps = nullptr;
	IRpcObjectEventOps* objectEventOps = nullptr;
	IRpcDispatcher* rpcDispatcher = nullptr;
	IRpcLifecycle* lifecycle = nullptr;

public:
	vint AllocateRequestId() override
	{
		return ++nextRequestId;
	}

	Ptr<JsonNode> OnJsonRequest(Ptr<JsonNode> message, RequestType requestType) override
	{
		switch (requestType)
		{
		case RequestType::Direct:
			return SendDirectAndWaitForResponse(message);
		case RequestType::Broadcast:
			return SendBroadcastAndWaitForResponse(message);
		case RequestType::BroadcastAndDrop:
			SendBroadcastAndDrop(message);
			return nullptr;
		default:
			CHECK_FAIL(L"Unexpected JSON RPC request type.");
		}
	}

	void OnIncomingJsonRequest(Ptr<JsonNode> message)
	{
		auto response = IRpcJsonMessageDispatcher::DefaultTranslate(
			message,
			ClassifyIncomingRequest(message),
			objectOps,
			objectEventOps,
			rpcDispatcher,
			lifecycle);

		if (response)
		{
			SendJsonResponse(response);
		}
	}
};
```

The functions named SendDirectAndWaitForResponse, SendBroadcastAndWaitForResponse, SendBroadcastAndDrop, ClassifyIncomingRequest and SendJsonResponse are transport-specific. They may use sockets, pipes, HTTP, shared memory or another message bus. The RPC layer only requires that request ids and response routing stay consistent.

## Default vl::inter_process Channeling

The default implementation is in namespace vl::rpc_controller::channeling. It implements IRpcJsonMessageDispatcher on top of vl::inter_process channels:
- JsonPackage is Ptr\<vl::glr::json::JsonNode\>.
- JsonChannel is vl::inter_process::IChannel\<JsonPackage\>.
- JsonNetworkChannelClient and JsonNetworkChannelServer connect JSON packages to an INetworkProtocol transport.
- JsonLocalChannelClient connects an in-process endpoint to the same channel server.
- RpcJsonDispatcherClient implements IRpcJsonMessageDispatcher and IChannelReader\<JsonPackage\> for one endpoint.
- RpcJsonDispatcherServer implements IChannelReader\<JsonPackage\> for the server-side broadcast broker.

The channel server is a broker. It must have one local channel client owned by RpcJsonDispatcherServer. That local client receives broadcast requests, service declarations and broadcast responses. Normal clients, whether local or remote, are registered with the broker after they connect.

## Shared Channel Client Glue

A JSON channel client must declare at least one channel name. The RPC dispatcher only needs the channel object for that name:
```C++
#include <VlppWorkflowLibrary.h>
#include <VlppOS.Windows.h>
#include <utility>

using namespace vl;
using namespace vl::collections;
using namespace vl::glr::json;
using namespace vl::inter_process;
using namespace vl::reflection;
using namespace vl::rpc_controller;
using namespace vl::rpc_controller::channeling;

constexpr const wchar_t* RpcChannelName = L"WorkflowRpc";

template<typename TBase>
class JsonRpcChannelClient : public TBase
{
private:
	JsonChannelClient::ChannelMap channelNames;
	JsonChannel* rpcChannel = nullptr;

protected:
	void InitializeRpcChannel()
	{
		channelNames.Add(WString::Unmanaged(RpcChannelName), nullptr);
		auto&& channels = this->GetChannels();
		auto index = channels.Keys().IndexOf(WString::Unmanaged(RpcChannelName));
		CHECK_ERROR(index != -1, L"Missing Workflow RPC channel.");
		rpcChannel = channels.Values()[index];
	}

public:
	template<typename... TArgs>
	JsonRpcChannelClient(TArgs&&... args)
		: TBase(std::forward<TArgs>(args)...)
	{
		InitializeRpcChannel();
	}

	const JsonChannelClient::ChannelNameList& OnGetChannelNames() override
	{
		return channelNames.Keys();
	}

	JsonChannel* GetRpcChannel()
	{
		CHECK_ERROR(rpcChannel, L"Missing Workflow RPC channel.");
		return rpcChannel;
	}
};

using JsonRpcNetworkClient = JsonRpcChannelClient<JsonNetworkChannelClient>;
using JsonRpcLocalClient = JsonRpcChannelClient<JsonLocalChannelClient>;
```

## Endpoint Dispatcher Client

RpcJsonDispatcherClientForTaskQueue already implements IRpcJsonMessageDispatcher by sending JSON packages through a JsonChannel. A project still needs a small subclass to create RpcJsonDispatcher and RpcJsonLifecycle, then register the RPC metadata and helper operations for the loaded module.

The module-specific registration is represented below by ConfigureLifecycleForRpcModule. It should set the id map, serializer, object ops, event ops, event attachers and wrapper factory for the module being used.
```C++
void ConfigureLifecycleForRpcModule(RpcJsonLifecycle* lifecycle);

class MyRpcDispatcherClient : public RpcJsonDispatcherClientForTaskQueue
{
public:
	MyRpcDispatcherClient(Ptr<TaskQueue> taskQueue)
		: RpcJsonDispatcherClientForTaskQueue(taskQueue)
	{
	}

	void InitializeRpc(vint clientId)
	{
		auto rpcDispatcher = Ptr(new RpcJsonDispatcher(clientId, this));
		auto lifecycle = Ptr(new RpcJsonLifecycle(clientId, rpcDispatcher.Obj()));

		SetRpcObjects(rpcDispatcher, lifecycle);
		ConfigureLifecycleForRpcModule(lifecycle.Obj());
	}
};
```

## Server Channel Setup

The server channel accepts only clients that declare the RPC channel name. After the broker local client is connected, every accepted normal client is registered with RpcJsonDispatcherServer.
```C++
class JsonRpcChannelServer : public JsonNetworkChannelServer<NamedPipeServer>
{
private:
	RpcJsonDispatcherServer* dispatcher = nullptr;

public:
	JsonRpcChannelServer(Ptr<Parser> parser, const WString& pipeName)
		: JsonNetworkChannelServer<NamedPipeServer>(parser, pipeName)
	{
	}

	WaitForClientResult OnClientConnected(
		vint clientId,
		const JsonChannelClient::ChannelNameList& availableChannels,
		JsonChannelClient* localClient) override
	{
		if (!availableChannels.Contains(WString::Unmanaged(RpcChannelName)))
		{
			return WaitForClientResult::Reject;
		}

		if (localClient)
		{
			return WaitForClientResult::Accept;
		}

		if (!dispatcher || !dispatcher->HasServerClientId())
		{
			return WaitForClientResult::Reject;
		}

		dispatcher->RegisterClient(clientId);
		return WaitForClientResult::Accept;
	}

	void OnClientDisconnected(vint clientId) override
	{
		if (dispatcher && dispatcher->HasServerClientId())
		{
			dispatcher->DisconnectClient(clientId);
		}
	}

	void SetDispatcher(RpcJsonDispatcherServer* value)
	{
		dispatcher = value;
	}
};

class JsonRpcBroadcastingLocalClient : public JsonRpcLocalClient
{
private:
	Ptr<RpcJsonDispatcherServerForTaskQueue> dispatcher;

public:
	JsonRpcBroadcastingLocalClient(Ptr<Parser> parser)
		: JsonRpcLocalClient(parser)
	{
	}

	void Connect(JsonChannelServer* channelServer, Ptr<JsonChannelClient> self, Ptr<TaskQueue> taskQueue)
	{
		auto clientId = channelServer->ConnectLocalClient(self);
		CHECK_ERROR(clientId != -1, L"Failed to connect the RPC broker local client.");

		dispatcher = Ptr(new RpcJsonDispatcherServerForTaskQueue(
			this,
			GetRpcChannel(),
			taskQueue));
	}

	RpcJsonDispatcherServer* GetDispatcher()
	{
		CHECK_ERROR(dispatcher, L"The RPC broker local client is not connected.");
		return dispatcher.Obj();
	}
};

void StartRpcChannelServer(
	Ptr<Parser> parser,
	Ptr<TaskQueue> taskQueue,
	Ptr<JsonRpcChannelServer>& channelServer,
	Ptr<JsonRpcBroadcastingLocalClient>& brokerClient)
{
	channelServer = Ptr(new JsonRpcChannelServer(parser, L"WorkflowRpcPipe"));
	channelServer->Start();

	brokerClient = Ptr(new JsonRpcBroadcastingLocalClient(parser));
	brokerClient->Connect(channelServer.Obj(), brokerClient, taskQueue);
	channelServer->SetDispatcher(brokerClient->GetDispatcher());
}
```

## Hosting a Local Service Endpoint

A process can host services through a local channel client connected to the same channel server. This is useful when the process that owns the channel server also owns local services.
```C++
class JsonRpcServiceLocalClient : public JsonRpcLocalClient
{
private:
	Ptr<MyRpcDispatcherClient> dispatcher;

public:
	JsonRpcServiceLocalClient(Ptr<Parser> parser)
		: JsonRpcLocalClient(parser)
	{
	}

	void OnConnected(vint clientId) override
	{
		CHECK_ERROR(dispatcher, L"The RPC dispatcher client is missing.");
		dispatcher->InitializeRpc(clientId);
	}

	vint Connect(
		JsonChannelServer* channelServer,
		Ptr<JsonChannelClient> self,
		Ptr<TaskQueue> taskQueue,
		vint serverClientId,
		const List<WString>& waitingForServices)
	{
		dispatcher = Ptr(new MyRpcDispatcherClient(taskQueue));
		auto clientId = dispatcher->ConnectLocalServer(
			channelServer,
			self,
			GetRpcChannel(),
			waitingForServices);

		dispatcher->SetServerLocalClientId(serverClientId);
		return clientId;
	}

	MyRpcDispatcherClient* GetDispatcher()
	{
		CHECK_ERROR(dispatcher, L"The RPC dispatcher client is not connected.");
		return dispatcher.Obj();
	}
};

void HostLocalService(
	JsonChannelServer* channelServer,
	RpcJsonDispatcherServer* broker,
	Ptr<Parser> parser,
	Ptr<TaskQueue> taskQueue,
	Ptr<IDescriptable> service)
{
	auto serviceClient = Ptr(new JsonRpcServiceLocalClient(parser));
	List<WString> waitingForServices;
	serviceClient->Connect(
		channelServer,
		serviceClient,
		taskQueue,
		broker->GetServerClientId(),
		waitingForServices);

	auto rpcClient = serviceClient->GetDispatcher();
	auto lifecycle = rpcClient->GetRpcLifecycle();
	auto typeId = lifecycle->GetTypeIdFromName(L"example::IExampleService");
	CHECK_ERROR(typeId != RpcTypeId_NotFound, L"Unknown RPC service type.");

	lifecycle->RegisterLocalService(typeId, service);
	rpcClient->Initialize();
}
```

## Connecting a Remote Client

A remote client uses JsonNetworkChannelClient over a raw transport such as NamedPipeClient or HttpClient. The channel connection supplies the client id; OnConnected initializes the RPC lifecycle for that id.
```C++
class JsonRpcNetworkEndpoint : public JsonRpcNetworkClient
{
private:
	Ptr<MyRpcDispatcherClient> dispatcher;

public:
	JsonRpcNetworkEndpoint(
		Ptr<MyRpcDispatcherClient> _dispatcher,
		Ptr<INetworkProtocolClient> transport,
		Ptr<Parser> parser)
		: JsonRpcNetworkClient(transport, parser)
		, dispatcher(_dispatcher)
	{
	}

	void OnConnected(vint clientId) override
	{
		CHECK_ERROR(dispatcher, L"The RPC dispatcher client is missing.");
		dispatcher->InitializeRpc(clientId);
	}
};

Ptr<MyRpcDispatcherClient> ConnectRemoteRpcClient(
	Ptr<Parser> parser,
	Ptr<TaskQueue> taskQueue,
	const List<WString>& waitingForServices)
{
	auto dispatcher = Ptr(new MyRpcDispatcherClient(taskQueue));
	auto transport = Ptr(new NamedPipeClient(L"WorkflowRpcPipe"));
	auto channelClient = Ptr(new JsonRpcNetworkEndpoint(dispatcher, transport, parser));

	dispatcher->WaitForServer(
		channelClient.Obj(),
		channelClient->GetRpcChannel(),
		waitingForServices);

	dispatcher->Initialize();
	return dispatcher;
}
```

After ConnectRemoteRpcClient returns, use dispatcher-\>GetRpcLifecycle() to request remote services. The server sends a login message through the broker, so a network client does not call SetServerLocalClientId.

