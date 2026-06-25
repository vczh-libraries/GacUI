# Workflow RPC

Workflow RPC describes service contracts with Workflow interfaces and runs them through a transport-independent lifecycle, dispatcher, serializer, and channel layer. The contract is written in Workflow; the runtime decides how object references, method calls, property access, and events travel between processes or machines.

## Declaring RPC Interfaces

An RPC service interface is a non-generic Workflow interface marked with @rpc:Interface. Base interfaces must also be RPC interfaces.

Use @rpc:Ctor on an RPC interface when that interface represents a service type that can be requested by clients.
```
module Rpc;
using system::*;

namespace Chat
{
	@rpc:Interface
	@rpc:Ctor
	interface IChatService
	{
		func Send(text : string) : void;
		func LastMessage() : string;
	}
}
```

RPC members can use serializable values, shared pointers to RPC interfaces, and strong typed collections whose element types are serializable. void is valid only as a method return type.

## RPC Attributes

The RPC analyzer recognizes these attributes:
- @rpc:Interface: marks a Workflow interface as an RPC interface.
- @rpc:Ctor: marks an RPC interface as constructible through the RPC service model.
- @rpc:Cached: marks an RPC property as cached by the client-side wrapper.
- @rpc:Dynamic: marks an RPC property as read from the remote object every time.
- @rpc:Byval: transfers a strong typed collection by value.
- @rpc:Byref: transfers a strong typed collection by reference.

@rpc:Cached and @rpc:Dynamic can only be used on RPC properties and cannot appear together. A property is cached by default unless it is marked @rpc:Dynamic.

@rpc:Byval and @rpc:Byref can be used on RPC properties, method return values, and method parameters that are strong typed collections. They cannot appear together. When neither is written, observable collections and collections containing RPC interfaces default to reference transfer; other strong typed collections default to value transfer.

## Serializable Values

RPC payloads must be serializable. Supported payload shapes are:
- primitive reflected values such as booleans, integers, floating point values, and strings;
- reflected structs, normal enums, and flag enums;
- nullable values whose element type is serializable;
- shared pointers to interfaces marked with @rpc:Interface;
- strong typed collections whose type arguments are serializable.

Reserved framework types such as object references and RPC exceptions are part of the transport protocol and are not valid as ordinary RPC member payloads.

## Service and Client Flow

A server creates an implementation object and registers it with IRpcLifecycle. A client asks the lifecycle for the same service name and casts the returned object to the service interface.
```
func serviceMain(lc : IRpcLifecycle*) : void
{
	var service = new Chat::IChatService^
	{
		var last : string = "";

		override func Send(text : string) : void
		{
			last = text;
		}

		override func LastMessage() : string
		{
			return last;
		}
	};

	lc.RegisterLocalService(rpc_GetIds()["Chat::IChatService"], service);
}

func clientMain(lc : IRpcLifecycle*) : string
{
	var service = cast (Chat::IChatService^) lc.RequestService("Chat::IChatService");
	service.Send("hello");
	return service.LastMessage();
}
```

rpc_GetIds() exposes the RPC interface id table for the loaded RPC module. RegisterLocalService binds a local implementation to an interface id. RequestService asks the remote side for a service by name and returns a wrapper object.

## Runtime Objects

The Workflow RPC runtime separates protocol logic from transport:
- IRpcLifecycle owns the per-client or per-session state. It maps local objects to object references, creates wrappers for remote object references, registers services, requests services, and exposes the serializer, dispatcher, and controller.
- IRpcDispatcher routes object operations and event operations. It declares local services, sends object operations to clients, broadcasts object events, and has explicit initialize and finalize steps.
- IRpcController performs local object operations and event operations. It also controls whether property change events and collection item events are suppressed while an RPC operation is being applied.
- IRpcSerializer converts reflected values, object references, exceptions, and operation records to and from the transport representation.

## Implementing a Dispatcher

A dispatcher implementation owns the lifecycle of a transport connection. It should:
- create or receive an IRpcLifecycle for each connected client;
- call Initialize before sending or receiving RPC operations;
- translate incoming transport messages to object operations, event operations, service declarations, or lifecycle operations;
- call DeclareLocalService when local services must become visible to the other side;
- send object operations through SendToClient_ObjectOps and object events through BroadcastFromClient_ObjectEventOps;
- call Finalize when the transport is closed.

Dispatchers should schedule user callbacks through the host task queue or event loop that owns the service objects. RPC callbacks can re-enter user code, so a dispatcher must keep its threading and lifetime policy consistent.

## JSON RPC Channel Layer

The JSON implementation uses RpcJsonDispatcher, RpcJsonLifecycle, RpcJsonObjectOps, and RpcJsonObjectEventOps. IRpcJsonMessageDispatcher accepts JSON requests and classifies them as direct, broadcast, or broadcast-and-drop messages.

The shared channel implementation connects the JSON dispatcher to [vl::inter_process](.././vlppos/using-inter-process.md) channels:
- JsonPackage is a JSON node package.
- JsonChannel is an IChannel\<JsonPackage\>.
- JsonChannelClient and JsonChannelServer are the channel endpoints used by the dispatcher.
- JsonNetworkChannelClient and JsonNetworkChannelServer use the network channel implementation.
- JsonLocalChannelClient uses the local inter-process channel implementation.

RpcJsonDispatcherClient receives JSON packages from a channel, waits for the server handshake, connects to local servers when needed, and exposes its RPC lifecycle and dispatcher. RpcJsonDispatcherServer receives JSON packages from clients, registers and disconnects clients, broadcasts service declarations, and forwards RPC requests. Both classes require a concrete ScheduleTask policy; the task-queue variants provide the standard implementation.

See [JSON RPC Channel Setup](.././workflow/rpc/json-channel.md) for the IRpcJsonMessageDispatcher transport contract and concrete vl::inter_process setup code.

