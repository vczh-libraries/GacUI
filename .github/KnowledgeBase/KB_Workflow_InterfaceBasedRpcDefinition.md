# Workflow Interface-Based RPC Definition

## Attributes

All user-authored attributes here do not have arguments unless explicitly specified.

- `@rpc:Interface` can be used on an interface.
  - `interface` in this document does not include generic interface: `$interface IDerivbed:IBased<TYPE>;`.
- `@rpc:Ctor` can be used only when `@rpc:Interface` is on that type.
- `@rpc:Byval` and `@rpc:Byref` on a property, method, parameter.
- `@rpc:Cached` and `@rpc:Dynamic` on a property.
- `@rpc:IdString(string)` and `@rpc:IdNumber(int)` on interfaces, events and methods.
  - These attributes are generated into RPC Workflow metadata.
  - User-authored `@rpc:IdString` and `@rpc:IdNumber` on RPC interfaces, methods and events are ignored when metadata is regenerated.
  - `IdString` stores the generated full text id, and `IdNumber` stores the generated numeric id from the sorted RPC id list.

## Serializable Types

All types below count as serializable types, but serialization itself is optional:
- Predefined primitive types.
- Struct types.
  - If it is reflection serializable, use the serialization.
  - Otherwise, trigger the fallback serialization (generated).
- Enum types, including flags.
  - Serialized to its interger value.
- Nullable types.
  - Nullable could be applied to primitive types, struct types and enum types.
  - Also all nullable types should just be serializable.
- Strong typed collection types, its element type, key type, value type should all be serializable.
  - Strong typed collection types here mean the type uses Workflow's collection type syntax.
    - `T{}`, `T[]`, `V[K]` and `observe T[]`.
  - Weak typed collection types, like `system::Enumerable` and all others, are just trivial interfaces, they are not serializable because they do not have `@rpc:Interface`.
- Interface with `@rpc:Interface`. Only shared pointer `T^` is acceptable, `T*` is not.

`system::RpcObjectReference` and `system::RpcException` are internal RPC transport structs. Generated infrastructure serializes them when needed, but user-authored RPC interfaces cannot use them in function return values, function arguments, or event arguments.

## Compile Errors:

- There are multiple types of property definitions, all count as properties.
- `FULL-NAME` means a full name of a type.
  - If the target is a member, it becomes `type.member`.
  - If the target is a parameter, it becomes `type.method(parameter)`.
- A method is serializable only when all argument types and the return type are serializable.
  - If the return type is `void`, it is also serializable.
- When generating helper functions for an error, multiple error messages with the same format can share one helper function. Only text in `xxx` becomes a parameter (or if `xxx` has only one choice in this helper function, it should not be a parameter).
  - The error code should be `H\d+`.
  - Each helper function will be assigned with a unique error code.
- (AST) does not belong to the error message, it is a hint that this error should be checked by traversing the AST, otherwise it should be checked against `ITypeDescriptor`.

## @rpc:Interface

- (AST) `@rpc:Interface` can only apply to an interface definition, but not `FULL-NAME`.
  - Triggered when it is applied to anything else.
- `@rpc:Interface` cannot be applied to interface `FULL-NAME` because its base type `FULL-NAME` is not serializable.
  - Triggered when any base type does not apply with `@rpc:Interface`.
  - Triggered for each inqualify base type.
- `@rpc:Interface` cannot be applied to interface `FULL-NAME` because its member `MEMBER-NAME` is not serializable.
  - Triggered when the interface type has unserializable members.
  - Triggered for each inqualify member.
- `@rpc:Interface` cannot be applied to interface `FULL-NAME` because its member `MEMBER-NAME` uses reserved RPC type `FULL-NAME` in a return value.
  - Triggered when a function return value or property value contains `system::RpcObjectReference` or `system::RpcException`.
- `@rpc:Interface` cannot be applied to interface `FULL-NAME` because its function argument `MEMBER-NAME` uses reserved RPC type `FULL-NAME`.
  - Triggered when a function argument contains `system::RpcObjectReference` or `system::RpcException`.
- `@rpc:Interface` cannot be applied to interface `FULL-NAME` because its event argument `MEMBER-NAME` uses reserved RPC type `FULL-NAME`.
  - Triggered when an event argument contains `system::RpcObjectReference` or `system::RpcException`.

## @rpc:Ctor

- (AST) `@rpc:Ctor` can only apply to an interface definition with `@rpc:Interface`.
  - Triggered when it is applied to anything else.
  - Triggered when it is applied to an interface without `@rpc:Interface`.

## @rpc:Byval

- (AST) `@rpc:Byval` can only apply to a property, a method or a parameter.
  - Triggered when it is applied to anything else.
- (AST) `@rpc:Byval` can only be used inside an interface type with `@rpc:Interface`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member it applies to is not in an interface with `@rpc:Interface`.
- `@rpc:Byval` cannot be used on member `MEMBER-NAME` because it does not have a strong typed collection types.
  - For property or parameter, it means the type.
  - For method, it means the return type.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member is inqualify.

## @rpc:Byref

- (AST) `@rpc:Byref` can only apply to a property, a method or a parameter.
  - Triggered when it is applied to anything else.
- (AST) `@rpc:Byref` cannot be used on member `MEMBER-NAME` because it already has `@rpc:Byval`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when it is applied to a member with `@rpc:Byval`.
- (AST) `@rpc:Byref` can only be used inside an interface type with `@rpc:Interface`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member it applies to is not in an interface with `@rpc:Interface`.
- `@rpc:Byref` cannot be used on member `MEMBER-NAME` because it does not have a strong typed collection types.
  - For property or parameter, it means the type.
  - For method, it means the return type.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member is inqualify.

## @rpc:Cached

- (AST) `@rpc:Cached` can only apply to a property.
  - Triggered when it is applied to anything else.
- `@rpc:Cached` can only be used inside an interface type with `@rpc:Interface`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member it applies to is not in an interface with `@rpc:Interface`.

## @rpc:Dynamic

- (AST) `@rpc:Dynamic` can only apply to a property.
  - Triggered when it is applied to anything else.
- (AST) `@rpc:Dynamic` cannot be used on member `MEMBER-NAME` because it already has `@rpc:Cached`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when it is applied to a member with `@rpc:Cached`.
- `@rpc:Dynamic` can only be used inside an interface type with `@rpc:Interface`.
  - Not triggered any of the above error is triggered on the same attribute.
  - Triggered when the member it applies to is not in an interface with `@rpc:Interface`.

## Semantic

## @rpc:Interface

## @rpc:Ctor

- Interface with this attribute could have implementation exposed as a singleton.
- RPC interface only allow registering singleton implementing `@rpc:Ctor` marked interfaces.
- RPC interface has dedicated methods for acquiring such singleton.

## Default options between @rpc:Byval and @rpc:Byref

- `Compile Errors` already limit these attributes on properties / return values / parameters of strong typed collection.
- If none is offered, the following collection types will by default `@rpc:Byref`, others will by default `@rpc:Byval`:
  - `observe T[]`.
  - Strong typed collections whose element or value is an interface, directly or through nested strong typed collections.
- During generating RPC Workflow metadata, `@rpc:Byval` and `@rpc:Byref` from the property will be copied to getter's return value and setter's parameter.
- Event arguments will pick the default value, and no customization could apply (because there is no syntax for putting attributes on event arguments).

## @rpc:Byval

- The whole collection is sent to a client, and it doesn't keep track on changes from the other side.
- All nested collections in this collection share the same behavior.

## @rpc:Byref

- The collection is treated as remote object.
- All nested collections in this collection share the same behavior.

## @rpc:Cached

- This is the default option, for any property.
- When property value is not cached, trigger actual RPC action and cache the result.
- When property value is cached, return the result immediately.
- Cached will be cleared if the associate event (if exists) is triggered.
- Client could proactivaly send property values to cache.

## @rpc:Dynamic

- The getter will trigger actual RPC action immediately.

## Message Dispatching and Event Suppression

The RPC runtime has three local concepts and one cross-client concept:

- `IRpcLifecycle` owns object/reference conversion for one client.
- `IRpcController` owns local objects, local wrappers, local ops, and local suppression state for one client.
- `IRpcOperations` exposes the local client's list, object, list-event, and object-event operation objects. List operations and list-event operations are local runtime adapters; cross-client list traffic is transported through object operations and object-event operations with predefined negative ids.
- `IRpcDispatcher` is the only object that knows how to send a message from one client to another client.

Lifecycle and controller implementations should not use another lifecycle's objects directly. When a message needs to leave the current client, it goes through `IRpcDispatcher`.

### Service Registration and Lookup

`IRpcLifecycle` owns service lookup state. A local service is stored in the service-owning lifecycle as a map from RPC type id to the local service object. A remote service is stored in each receiving lifecycle as a map from RPC type id to the full `RpcObjectReference` declared by the service owner.

`IRpcLifecycle::RegisterLocalService(typeId, service)` records a local service before lifecycle initialization. It converts the service object to a local `RpcObjectReference`, adds the owner hold, stores the local service object, and calls `IRpcDispatcher::DeclareLocalService(ref)` so the dispatcher implementation can transmit the declaration. Registering the same local type id twice is a recoverable service-registration error, and registering after `IRpcLifecycle::Initialize()` is also an error.

`IRpcDispatcher::DeclareLocalService(ref)` is a data-transmission hook, not service storage. The receiving lifecycle handles the declaration through `IRpcLifecycle::DeclareRemoteService(ref)`. Declaring a remote service with the same type id overwrites the earlier remote declaration and stores the full reference, including `objectId`.

`IRpcObjectOps::RequestService` should not exist. `IRpcLifecycle::GetTypeIdFromName(typeName)` resolves the type name through the lifecycle RPC id map and returns `RpcTypeId_NotFound` when the name is unknown. `IRpcLifecycle::RequestService(typeName)` uses this function, returns the local registered service when that type id exists locally, or finds the stored remote `RpcObjectReference` and calls `RefToPtr(ref)`.

`RefToPtr(ref)` uses `ref.clientId` to decide whether the ref belongs to the current client:

- If the ref is local, it returns the local object.
- If the ref is remote, it returns or creates a wrapper for the remote object.

### Refcounting and Service Hold Semantics

Refcounting tracks interested clients for local objects. It is not the same concept as `Ptr<T>` or `std::shared_ptr` ownership. A hold means a client has a remote wrapper or an equivalent durable interest in the local object.

- `IRpcLifecycle::LocalObjectHold(ref, remoteClientId)` records that `remoteClientId` is interested in the local object identified by `ref`.
- `IRpcLifecycle::LocalObjectUnhold(ref, remoteClientId)` removes that interest.
- These functions only track local objects. If `ref` does not belong to the current lifecycle, this is an implementation error.
- The counter should track interested clients, not wrapper instances. Wrapper construction and destruction should happen once for each object/client pair, so duplicate holds or unholds for the same `(ref, remoteClientId)` should not silently create a different ownership meaning.

`PtrToRef(ptr)` only converts a local object to a `RpcObjectReference`. It should allocate a new local object id when needed and attach the object-to-ref internal property, but it must not increase the interested-client counter.

For a normal non-service object, the counter becomes 1 only after the first remote client actually receives the ref and creates its wrapper:

- The remote wrapper constructor calls `IRpcDispatcher::SendToClient_ObjectOps(ref.clientId)->ObjectHold(ref, currentClientId, true)`.
- Generated interface wrappers and predefined container wrappers follow the same rule.
- The owner lifecycle receives this through `IRpcObjectOps::ObjectHold` and calls `IRpcLifecycle::LocalObjectHold(ref, remoteClientId)`.
- The wrapper destructor calls `ObjectHold(ref, currentClientId, false)` if it has not been disconnected from the lifecycle, and the owner lifecycle calls `IRpcLifecycle::LocalObjectUnhold(ref, remoteClientId)`.

When the interested-client counter decreases to 0, the lifecycle should remove all resources and tracking for that local object reference. After this removal, a later `PtrToRef(ptr)` on the same object should allocate a new object id.

There is a special case when `PtrToRef(ptr)` is called but no remote client ever receives the returned ref, so no wrapper constructor sends `ObjectHold(..., true)`. The object-to-ref internal property is the final fallback for this case: if the local object is deleted while the lifecycle is still alive, the property cleanup removes the tracking resource. During lifecycle finalization, implementations should remove these internal properties from all tracked local objects, so objects deleted after the lifecycle is gone do not call back into a finalized lifecycle.

Services use the same local-object tracking, but `RegisterLocalService` adds an owner hold:

- An object registered by `IRpcLifecycle::RegisterLocalService` has an initial interested-client count of 1.
- This hold represents the owner client being in the interested-client list. It is not created by a remote wrapper.
- Remote clients still send normal hold and unhold messages when they create or destroy wrappers for the service object.
- When all remote clients unhold the service object, the owner hold remains, so the service is not unregistered by ordinary refcounting.
- Ordinary service unregistration is not part of the RPC interface. Finalization clears lifecycle local-object tracking and local/remote service registration state.

### Point-to-Point Operations

When a wrapper performs a method call or list operation, the message is sent to the lifecycle that owns `ref.clientId`.

- Object method calls use `IRpcDispatcher::SendToClient_ObjectOps(ref.clientId)`.
- List operations use `IRpcDispatcher::SendToClient_ObjectOps(ref.clientId)->InvokeMethod(...)` with predefined list method ids through `RpcCallerListOps`.
- Array resize uses its own predefined method id through `IRpcListOps::ArrayResize`; list-only mutation ids such as clear and remove-at should not be accepted as array resize shortcuts.

The returned ops object is the target client's local operation object. The caller should not know or store the target lifecycle directly.

### Event Broadcasts

Events are broadcast from the client that observed or raised the event. The broadcast excludes that caller client and sends to all other clients.

- Object events use generated strong typed caller-side ops, which call `IRpcDispatcher::BroadcastFromClient_ObjectEventOps(selfClientId)->InvokeEvent(...)`.
- List events use `IRpcDispatcher::BroadcastFromClient_ObjectEventOps(selfClientId)->InvokeEvent(...)` with the predefined observable-list event id through `RpcCallerListEventOps`.

This rule applies whether the event is raised from a local object or from a wrapper. If a wrapper raises an event, the owner client is still just another client in the broadcast target set unless it is the caller client.

Event broadcasts return `object` so the same ops interface can carry either direct reflected values or JSON serialized values. After deserialization, the value is `system::RpcException[int]` or `null`, keyed by the lifecycle client id that caught an event handler exception. A broadcast should attempt every target lifecycle even when earlier targets report exceptions, then aggregate all returned maps. The lifecycle that triggered the event receives the aggregate map internally; generated object-event send-side code and predefined container-event send-side code deserialize the returned value and call `system::IRpcLifecycle::ReadEventException`, which raises a normal Workflow exception message when the aggregate is non-empty.

In the dual-client test implementation, broadcasting can be implemented by returning event ops from the lifecycle whose client id is not `selfClientId`.

### Event Suppression

Event suppression is local controller state. It is not dispatcher state.

Suppression prevents an event replayed from a remote message from being forwarded again by the normal locally attached event handlers.

`IRpcController` exposes these suppression APIs:

- `SetEventSuppressedFlag(ref, eventId, bool)`
- `GetEventSuppressedFlag(ref, eventId)`
- `SetItemChangedSuppressedFlag(ref, bool)`
- `GetItemChangedSuppressedFlag(ref)`

Although the setter takes a `bool`, implementations should store a counter for each suppression key. Setting `true` increments the counter. Setting `false` decrements it. The getter returns true when the counter is greater than zero. Decrementing below zero should be treated as an implementation error.

Object event suppression is keyed by `(RpcObjectReference, eventId)`. List `ItemChanged` suppression is keyed by `RpcObjectReference`.

When `IRpcObjectEventOps::InvokeEvent(ref, eventId, arguments)` receives a remote event, generated `rpc_IRpcObjectEventOps` should:

- Call `lc.Controller.SetEventSuppressedFlag(ref, eventId, true)`.
- Unbox arguments and raise the local event.
- Catch exceptions into a returned `system::RpcException[int]` map keyed by `lc.ClientId`.
- Call `lc.Controller.SetEventSuppressedFlag(ref, eventId, false)` in a `finally` block.
- Treat unknown event ids as local dispatch errors that are raised directly and are not inserted into the returned exception map.

Generated `rpclistener_*` handlers should check `lc.Controller.GetEventSuppressedFlag(ref, eventId)` before forwarding the event. If the flag is set, the handler returns immediately. Otherwise it calls the generated strong typed caller-side ops event method, letting those ops handle boxing, optional JSON serialization, broadcasting, deserialization, and exception raising.

List events use the same shape with the predefined observable-list event id. When receive-side `OnItemChanged(ref, index, oldCount, newCount)` replays a remote list notification locally, it should set `SetItemChangedSuppressedFlag(ref, true)`, raise the local list notification, catch exceptions into a `system::RpcException[int]` map keyed by `lc.ClientId`, return that map as `object`, and clear the flag in a `finally` block. The locally attached native list event handler should check `GetItemChangedSuppressedFlag(ref)` before broadcasting through the object-event dispatcher path, deserialize the returned value when a serializer exists, and call `system::IRpcLifecycle::ReadEventException` on the resulting map.

## Byval Return Collection Lifecycle

When a method return value is marked with `@rpc:Byval`, the generated `IRpcObjectOps::InvokeMethod` result is not the boxed collection itself. It is a `system::RpcByvalReturnValue` object:

- `value` stores the boxed returned collection, or the JSON serialized `JsonNode` when JSON object ops are used.
- `slot` stores an incremental index allocated by the callee-side object ops object.

The generated object ops object owns `_slot : int` and `_byvalReturnValues : object[int]`. For each byval collection return, it first calls `system::IRpcLifecycle::RpcCopyByval` on the real returned collection. This recursively copies every nested collection layer, so later mutation of the original collection cannot affect the value being transported. The copied collection is stored in `_byvalReturnValues[slot]`, and then the copied collection is boxed or serialized for `RpcByvalReturnValue.value`.

The callee caches the copied collection instead of the boxed or JSON value. This keeps interface elements alive for both non-JSON and JSON transport: boxed byval collections may contain only `RpcObjectReference` values, and JSON values contain no interface objects at all. Holding the recursive copy keeps the actual interface objects alive until the caller has received the result and created the needed wrappers.

Generated caller-side ops know from metadata whether a return value uses this path. They cast the `InvokeMethod` result directly to `system::RpcByvalReturnValue^`, deserialize and unbox `value` into a local return variable, call `EndInvokeMethod(slot)` on the same object ops object, and then return the local variable. `EndInvokeMethod` removes the cached copied collection from `_byvalReturnValues`.

Non-byval returns do not use `RpcByvalReturnValue`, `_slot`, `_byvalReturnValues`, or `EndInvokeMethod` for result cleanup.
