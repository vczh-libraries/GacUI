# Remote Protocol Channel Layer

GacUI remote protocol channeling uses the vl::inter_process channel bridge and the INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer interfaces underneath. The GacUI layer remains a rendering protocol between a headless core and a renderer.

The layers are:
- IGuiRemoteProtocol and IGuiRemoteEventProcessor: the strongly typed GacUI remote protocol.
- GuiRemoteProtocolCoreChannel and GuiRemoteProtocolRendererChannel: conversion between the strongly typed protocol and JSON channel packages.
- GuiRemoteProtocolNetworkChannelServer, GuiRemoteProtocolLocalChannelClient and GuiRemoteProtocolChannelClient: the vl::inter_process JSON channel bridge.
- NamedPipeServer, NamedPipeClient, HttpServer, HttpClient, or custom implementations of the INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer interfaces: the underlying data-transmission implementation.

The JSON channel package type is Ptr\<glr::json::JsonNode\>. The remote protocol channel name is GacUIRemoteProtocolChannelName, whose value is GacUIRemoteProtocol. The in-process core client should be assigned GacUIRemoteProtocolCoreClientId. Renderer-side packages are sent to that core client id, and core-side packages are sent to the renderer client id learned from ControllerConnect.

ChannelPackageInfo describes the remote protocol envelope:
- semantic: Message, Request, Response or Event.
- id: the request or response id, or -1 when the package does not need one.
- name: the generated remote protocol message, request, response or event name.

JsonChannelPack and JsonChannelUnpack convert this envelope and the optional arguments node to and from a JSON object. Application code normally does not call these functions unless it is logging, debugging or building a non-C++ renderer.

## What User Code Owns

A normal C++ application configures the remote protocol channel at startup. After GuiRemoteProtocolCoreChannel and GuiRemoteProtocolRendererChannel are connected, the library routes individual remote protocol messages between the strongly typed protocol and JSON channel packages.

User code usually owns only:
- A small server policy around GuiRemoteProtocolNetworkChannelServer, if the core needs to wait for a renderer, reject invalid clients, or replace a previous renderer.
- An underlying data-transmission implementation, represented by INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer.
- Optional renderer-side error handling by overriding GuiRemoteProtocolChannelClient::OnReadError, OnLocalError or OnDisconnected.

GuiRemoteProtocolCoreChannel exposes BeforeWrite and BeforeOnRead. GuiRemoteProtocolRendererChannel exposes the same events. Use these events for tracing package flow or diagnosing mismatched renderer/core behavior.

## Adding a Transport

Add a new underlying data-transmission implementation below GacUI by implementing the vl::inter_process INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer interfaces. The names say network protocol, but the contract is only asynchronous WString message exchange. The implementation can be a pipe, socket, stdio stream, shared memory, DLL function calls, or any other mechanism that can satisfy INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer. Then reuse the GacUI JSON channel classes.

The core side selects the INetworkProtocolServer implementation as the template argument:
```c++
using MyRemoteCoreServer =
    GuiRemoteProtocolNetworkChannelServer<MyTransportServer>;

auto jsonParser = Ptr(new glr::json::Parser);
MyRemoteCoreServer server(jsonParser, /* transport server constructor arguments */);
server.Start();

auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
server.ConnectLocalClient(coreClient);
```

The renderer side wraps an INetworkProtocolClient object:
```c++
auto jsonParser = Ptr(new glr::json::Parser);
auto networkClient = Ptr(new MyTransportClient(/* connection arguments */));

GuiRemoteProtocolChannelClient channelClient(networkClient, jsonParser);
channelClient.WaitForServer();
```

The same channel setup shown in [Remote Protocol Core Application](../.././gacui/modes/remote_core.md) and [Remote Protocol Client Application](../.././gacui/modes/remote_client.md) works after the transport object is connected. See [Using Inter-Process Channels](../.././vlppos/using-inter-process.md) for the inter-process protocol and channel interface contracts.

