# Workflow JSON Request Routing

This document describes the runtime meaning of the generic JSON dispatcher envelopes declared in `Release/Rpc.d.ts`. It is independent of any particular generated application or transport. Generated interface and serialization rules are covered by [Workflow Interface-Based RPC Definition](./KB_Workflow_InterfaceBasedRpcDefinition.md), [Workflow Generated RPC Wrappers](./KB_Workflow_Design_GeneratedRpcWrappers.md), and [Workflow JSON Serialization Schema](./KB_Workflow_Design_JsonSerializationSchema.md).

## Setup

The reusable JSON RPC setup lives in `Source/Library/RpcJson`. User code normally touches only these pieces:

- Include `WfLibraryRpcJson.h` when building a custom transport adapter around `vl::rpc_controller::IRpcJsonMessageDispatcher`, `vl::rpc_controller::RpcJsonDispatcher`, and `vl::rpc_controller::RpcJsonLifecycle`.
- Include `WfLibraryRpcJsonDispatcherClient.h` and `WfLibraryRpcJsonDispatcherServer.h` when using the default channel-backed setup in `vl::rpc_controller::channeling`.
- Create one `RpcJsonDispatcher(clientId, messageDispatcher)` and one `RpcJsonLifecycle(clientId, dispatcher)` per RPC endpoint. The `clientId` must be the endpoint id assigned by the transport.
- Configure the lifecycle from generated RPC code before calling `Initialize()`: set the generated id map, pass the generated JSON serializer, JSON object ops, JSON object event ops, type-id callback, and event-attacher callback to `RpcJsonLifecycle::Register`, and register the generated wrapper factory.
- Register local services with `IRpcLifecycle::RegisterLocalService(typeId, service)` before lifecycle initialization. Remote services are discovered through service declaration messages and can be requested by type name after initialization.
- Use `RpcJsonDispatcherClientForTaskQueue` for endpoint-side channel IO when a single `TaskQueue` should process incoming RPC requests. A small generated-app wrapper should derive from it, call `SetRpcObjects`, and expose an app-specific `InitializeRpc(clientId)` function.
- Use `RpcJsonDispatcherServerForTaskQueue` for the transport coordinator. It is not a service owner; it tracks connected client ids, forwards broadcast requests, caches service declarations, and consolidates broadcast responses.
- Pass required remote service type names to `WaitForServer` or `ConnectLocalServer` so client initialization waits until those services have been declared.
- Call `FinalizeRpc()` on endpoint dispatchers before shutting down the transport or unloading generated Workflow context.

An application-specific dispatcher wrapper normally derives from `RpcJsonDispatcherClientForTaskQueue`, creates the dispatcher/lifecycle pair after the transport assigns a client id, wires generated JSON serializer and ops into the lifecycle, registers the generated wrapper factory, and leaves transport behavior to `Source/Library/RpcJson`.

### Setup from Workflow Generated Code

Generated Workflow code is the only source of RPC-specific setup data. After the transport assigns `clientId`, the application-specific dispatcher wrapper creates `RpcJsonDispatcher(clientId, messageDispatcher)` and `RpcJsonLifecycle(clientId, rpcDispatcher.Obj())`, then stores them with `SetRpcObjects`.

The lifecycle id map comes from generated Workflow code. Setup calls `rpc_GetIds()`, converts the result to `Dictionary<WString, vint>` when crossing the C++ reflection boundary, and passes it to `RpcJsonLifecycle::SetIdMap`. Service-owning code resolves a full RPC interface name through `GetTypeIdFromName(fullName)` or reads the same id from `rpc_GetIds()[fullName]`, then calls `RegisterLocalService(typeId, service)` before lifecycle initialization.

Generated JSON operations are created once per lifecycle:

- `app.rpcops_IRpcSerializer()` creates the generated serializer object.
- `app.rpcops_IRpcObjectOpsJson(lifecycle.Obj())` creates the generated receive-side object ops.
- `app.rpcops_IRpcObjectEventOpsJson(lifecycle.Obj())` creates the generated receive-side object-event ops.
- `app.rpcops_IOps_CreateJson(lifecycle.Obj())` creates the generated caller-side ops that wrappers and listener attachers reuse.

`RpcJsonLifecycle::Register` receives those generated ops plus two callbacks. The type-id callback calls generated `rpcwrapper_GetTypeId(BoxValue<IDescriptable*>(obj))`, so local object references can be assigned generated RPC type ids. The event-attacher callback calls generated `rpclistener_Attach(ref.typeId, lifecycle, ref, obj, ops)`, so generated listeners attach to local object events and forward unsuppressed events through the generated caller-side ops.

The wrapper factory is also generated-code based. `RegisterWrapperFactory` installs a callback that calls generated `rpcwrapper_Create(ref, lc, ops)`. `rpcwrapper_Create` returns a generated wrapper for a remote `RpcObjectReference`, and the same generated wrapper class is used for JSON transport because the JSON caller-side ops object is the `ops` argument passed into the wrapper.

Endpoint startup then happens through the reusable dispatcher client. A network endpoint calls `WaitForServer(channelClient, rpcChannel, waitingForServices)`, or a local endpoint calls `ConnectLocalServer(channelServer, localClient, rpcChannel, waitingForServices)`. When the channel reports `OnConnected(clientId)`, the application-specific wrapper performs the generated setup described above and then calls `Initialize()`. Client code requests a service with `GetRpcLifecycle()->RequestService(fullName)`; when the service is remote, the lifecycle returns the generated wrapper created by the registered factory, and ordinary interface method calls on that wrapper go through the generated JSON caller-side ops.

### Implementation

`IRpcJsonMessageDispatcher` is the transport boundary. `AllocateRequestId()` provides request ids for JSON envelopes, and `OnJsonRequest(message, requestType)` sends a direct, broadcast, or broadcast-and-drop request through the transport. `IRpcJsonMessageDispatcher::DefaultTranslate` is the receiver-side helper that routes JSON envelopes to local object ops, object event ops, or lifecycle service declaration handling.

`RpcJsonObjectOps` and `RpcJsonObjectEventOps` adapt generated JSON ops to the shared envelopes declared by `Release/Rpc.d.ts`. They build request objects on the caller side, validate matching responses, and translate received requests back to `IRpcObjectOps` or `IRpcObjectEventOps`.

`RpcJsonDispatcher` is the `IRpcDispatcher` implementation for JSON transport. It creates per-target object ops, one broadcast object-event ops adapter, and sends local service declarations as broadcast-and-drop messages.

`RpcJsonLifecycle` derives from `RpcLifecycleBase` and installs the generated JSON serializer, object ops, event ops, type-id callback, and event-attacher callback. It also wraps predefined byref collection operations through the reusable list/object ops adapters.

`WfLibraryRpcJsonDispatcherClient` owns endpoint-side channel details that are not part of the generic RPC lifecycle: nested request processing while waiting for a response, response buffering by request id, pre-initialization service declaration caching, required-service waiting, and server-coordinator login/logout messages.

`WfLibraryRpcJsonDispatcherServer` owns coordinator-side channel details: connected client tracking, broadcast request redirection, expected response tracking, response consolidation, service declaration replay to future clients, and client disconnect cleanup. The task-queue subclasses keep scheduling policy outside the core translation helpers.

Every JSON RPC envelope has:

- `rpcMethod`: a string beginning with `Request:` or `Response:`.
- `rpcRequestId`: the caller-allocated id for matching a response to a request.
- `sourceClientId`: the lifecycle client id that created the envelope.

Direct requests and ordinary responses also have `targetClientId`. Broadcast requests omit `targetClientId` because the transport coordinator expands the request to multiple clients.

## Request Kinds

There are three request kinds at the JSON message dispatcher boundary.

1. `Direct`

   The request is sent to exactly one `targetClientId`. The receiver translates the request to its local ops object and sends exactly one response with the same `rpcRequestId`. Direct requests are synchronous from the caller's point of view.

2. `Broadcast`

   The request is sent to the transport coordinator, which sends it to all relevant clients except the originating client. Each receiver translates the request locally and sends a response to the coordinator. The coordinator waits for all expected responses, merges the returned data, and sends one response to the original caller with the original `rpcRequestId`.

3. `BroadcastAndDrop`

   The request is sent to the transport coordinator, which sends it to all relevant clients except the originating client and stores enough state to replay the declaration to future clients. Receivers translate the request locally but do not send a response. The original caller receives no response; the local `JsonRequest` result is `null`.

## Requests and Responses

### `Request:IObjectOps_InvokeMethod`

Kind: `Direct`.

The caller sends the request to `targetClientId`, normally the same client id as `ref.clientId`. The receiver calls its local object ops `InvokeMethod(ref, methodId, arguments)` and returns `Response:IObjectOps_InvokeMethod` to the caller.

The response carries:

- the same `rpcRequestId`,
- `sourceClientId` equal to the receiver,
- `targetClientId` equal to the original caller,
- `response` containing the serialized method result, serialized `system_RpcException`, or `system_RpcByvalReturnValue<T>` for byval collection returns.

Unknown method ids and malformed references are local dispatch errors. User-code exceptions are transported as `system_RpcException` according to the generated ops rules.

### `Request:IObjectOps_EndInvokeMethod`

Kind: `Direct`.

The caller sends this to the same client that returned a byval collection slot. The receiver calls local object ops `EndInvokeMethod(slot)` and returns `Response:IObjectOps_EndInvokeMethod` with the same `rpcRequestId`. There is no `response` field.

### `Request:IObjectOps_ObjectHold`

Kind: `Direct`.

The caller sends this to the owner of `ref.clientId` when creating or releasing a wrapper interest. The receiver calls local object ops `ObjectHold(ref, remoteClientId, hold)` and returns `Response:IObjectOps_ObjectHold` with the same `rpcRequestId`. There is no `response` field.

`remoteClientId` is the lifecycle whose interest changes. The receiver should validate that `ref` belongs to the receiving lifecycle before mutating local-object hold state.

### `Request:IObjectEventOps_InvokeEvent`

Kind: `Broadcast`.

The originating lifecycle has already observed or raised the event locally, so the coordinator must not send the broadcast back to that originating client. Each receiving lifecycle calls local object event ops `InvokeEvent(ref, eventId, arguments)`, which replays the event under event-suppression rules and returns the serialized form of `null | [number, system_RpcException][]`.

Each receiver sends `Response:Broadcast_Response` to the coordinator with:

- the redirected broadcast `rpcRequestId` chosen by the coordinator,
- `sourceClientId` equal to that receiver,
- `targetClientId` equal to the coordinator,
- `response` equal to the event exception map or `null`.

The coordinator sends one `Response:Broadcast_Response` to the original caller with:

- the original caller's `rpcRequestId`,
- `sourceClientId` equal to the coordinator client id,
- `targetClientId` equal to the original caller,
- `response` equal to `null` if every receiver returned `null`, otherwise a merged event exception map.

The coordinator must attempt every receiver even when earlier receivers report exceptions. When clients disconnect during an active broadcast, the coordinator removes them from the expected response set and completes the broadcast if all remaining receivers have responded.

### `Request:IRpcDispatcher_DeclareRemoteService`

Kind: `BroadcastAndDrop`.

This request declares that `sourceClientId` owns a service reference. It carries `ref: system_RpcObjectReference`, and `ref.clientId` must equal `sourceClientId`. The receiver calls `IRpcLifecycle::DeclareRemoteService(ref)` and stores the full reference by `ref.typeId`.

No `Response:*` envelope is created for this request. A caller-side dispatcher returns `null` immediately after sending or caching the message. A receiver-side translator also returns `null`.

The coordinator caches every service declaration and replays cached declarations to newly connected clients after the new client has learned the coordinator client id. The replayed request keeps the original `sourceClientId` and the original `ref`; it is not rewritten to the coordinator client id.

## Lifecycle Handling

`IRpcLifecycle::RegisterLocalService(typeId, service)` is a pre-initialization operation. It creates a full `RpcObjectReference` for the service object, stores the local service by type id, adds the owner hold, and calls `IRpcDispatcher::DeclareLocalService(ref)`.

For JSON transport, `IRpcDispatcher::DeclareLocalService(ref)` creates `Request:IRpcDispatcher_DeclareRemoteService` and sends it as `BroadcastAndDrop`. A lifecycle receiving this request stores `ref` through `IRpcLifecycle::DeclareRemoteService(ref)`.

`IRpcLifecycle::GetTypeIdFromName(typeName)` resolves names through the lifecycle id map and returns `RpcTypeId_NotFound` when the name is unknown. `IRpcLifecycle::RequestService(typeName)` uses this function, returns a local registered service first, otherwise looks up the stored remote `RpcObjectReference` by type id and calls `RefToPtr(ref)`.

Before a lifecycle is initialized, a client may receive service declaration requests. It should cache `Request:IRpcDispatcher_DeclareRemoteService` and reject all other RPC messages. During initialization it processes cached declarations before requesting required remote services. After initialization, later service declarations are processed immediately.

If a client is waiting for required service type names, it should compare each incoming declaration's `ref.typeId` with `GetTypeIdFromName(typeName)`. When all required services have been declared, the wait completes.

## Expected Sequences

### Transport Coordinator Startup

1. Start the transport layer enough for local endpoints to connect.
2. Connect the coordinator endpoint first and record its client id.
3. Report the coordinator client id to clients as an out-of-band transport login message before replaying RPC declarations.
4. Connect any local service-owning clients.
5. Let local service-owning clients register services and send `Request:IRpcDispatcher_DeclareRemoteService`.
6. Start accepting remote clients only after the coordinator client id is known and local service declarations have been sent.

The coordinator endpoint is not a service owner. It routes broadcasts, caches service declarations, and consolidates broadcast responses.

### New Client Startup

1. Connect to the transport and learn the client's own client id.
2. Register the JSON channel reader before waiting for the coordinator login message.
3. Learn the coordinator client id.
4. Cache any `Request:IRpcDispatcher_DeclareRemoteService` messages that arrive before lifecycle initialization.
5. Initialize the lifecycle, process cached declarations, send local declarations if this client owns services, and wait for required remote service names if needed.
6. Begin ordinary direct method calls and event broadcasts.

### Direct Method Call

1. Caller allocates `rpcRequestId` and sends a direct request to `targetClientId`.
2. Receiver translates and executes the local operation.
3. Receiver sends the matching direct response.
4. Caller matches by `rpcRequestId`; while waiting, it may process nested incoming requests and buffer unrelated responses.

### Event Broadcast

1. Originating lifecycle raises or observes the event locally.
2. Originating caller sends `Request:IObjectEventOps_InvokeEvent` to the coordinator.
3. Coordinator chooses a redirected request id, sends the request to every expected receiver except the originator, and records the original `(sourceClientId, rpcRequestId)`.
4. Receivers replay the event under suppression and respond to the coordinator.
5. Coordinator merges responses and sends one `Response:Broadcast_Response` to the originator using the original request id.

### Service Declaration

1. Service owner registers a local service before lifecycle initialization.
2. Its dispatcher sends or caches `Request:IRpcDispatcher_DeclareRemoteService`.
3. Coordinator caches the declaration and broadcasts it without waiting for responses.
4. Receivers store `ref.typeId -> ref`.
5. Future clients receive the cached declaration during startup.

## Error Handling Rules

Unknown `rpcMethod` values are transport or implementation errors. Do not silently drop them.

A request kind mismatch is an error: object ops are direct, object events are broadcast, and service declarations are broadcast-and-drop.

Only `Request:IRpcDispatcher_DeclareRemoteService` may be accepted before lifecycle initialization. Other RPC messages before initialization indicate a startup-order violation.

Broadcast-and-drop requests must not produce responses. A client waiting for a response to this request will deadlock a correct implementation.
