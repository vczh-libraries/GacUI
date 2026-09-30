# Remote Protocol Channel Layer

GacUI remote protocol channeling uses the vl::inter_process channel bridge and the INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer interfaces underneath. The GacUI layer remains a rendering protocol between a headless core and a renderer.

The layers are:
- IGuiRemoteProtocol and IGuiRemoteEventProcessor: the strongly typed GacUI remote protocol.
- GuiRemoteProtocolCoreChannel and GuiRemoteProtocolRendererChannel: conversion between the strongly typed protocol and JSON channel packages.
- GuiRemoteProtocolNetworkChannelServer, GuiRemoteProtocolLocalChannelClient and GuiRemoteProtocolChannelClient: the vl::inter_process JSON channel bridge.
- **vl::inter_process::async_tcp_socket::NetworkProtocolServer\<TAsyncSocketServer\>** and **NetworkProtocolClient\<TAsyncSocketClient\>**, **vl::inter_process::async_tcp_socket::SocketHttpServer** and **SocketHttpClient**, the Windows-only named-pipe and **windows_http** implementations, or a custom **INetworkProtocol** implementation: the underlying data-transmission mechanism.

The **INetworkProtocol** and **IChannel** interface families and channel bridge can be reused directly, but VlppOS provides them only for test-purpose local cross-process communication. Product code is strongly recommended to supply its own implementation instead of treating the bundled transports and channel stack as production networking. The GacUI JSON channel layer remains unchanged above that implementation.

The GacUI **/MiniHttp** tests create platform-default async TCP sockets and wrap them in **SocketHttpServer** and **SocketHttpClient**. The core can register its remote-protocol prefix and automation prefix on the exact same **IAsyncSocketServer**, while each prefix remains independently stoppable.

[GacJS](https://github.com/vczh-libraries/GacJS)'s primary feature is WebAssembly execution with HTML5 rendering. Its `remote-protocol-wasm` adapter loads the GacUI core's `app.mjs` and `app.wasm` in a browser worker. The exposed `StartApplication` and `SendDataToWasmCore` functions and callbacks carry the existing channel packages between JavaScript and the core. Both the HTML5 renderer and the TypeScript view-model host can use this transport, through the remote protocol and Workflow RPC channels respectively. It is an alternative to the HTTP transport, which is used mainly to test HTML5 rendering without WebAssembly; WebAssembly channel traffic does not pass through an HTTP protocol server. See [GacJS Network Protocol](https://github.com/vczh-libraries/GacJS/blob/master/doc/NetworkProtocol.md) for the bridge contract.

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

Add a new underlying data-transmission implementation below GacUI by implementing the vl::inter_process INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer interfaces. The names say network protocol, but the contract is only asynchronous WString message exchange. The implementation can be a pipe, socket, stdio stream, shared memory, DLL function calls, a WebAssembly/JavaScript bridge, or any other mechanism that can satisfy INetworkProtocolConnection, INetworkProtocolCallback, INetworkProtocolClient and INetworkProtocolServer. Then reuse the GacUI JSON channel classes.

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

