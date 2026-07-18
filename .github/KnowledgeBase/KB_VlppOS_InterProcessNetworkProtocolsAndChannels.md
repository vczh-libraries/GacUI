# Inter-Process Network Protocols and Channels

## Production Interfaces and Testing-Only Protocol Implementations

The transport-agnostic `INetworkProtocol*` and `IChannel*` interfaces, together with the channel implementation built on them, can be used in production. However, every concrete `INetworkProtocol*` implementation supplied by this repository is testing, validation and demonstration infrastructure and should not be used as a production transport. Applications that need production inter-process communication should implement their own `INetworkProtocolServer`, `INetworkProtocolClient` and `INetworkProtocolConnection` to meet their security, deployment, performance, reconnection and delivery requirements; the `IChannel*` layer can be reused over that implementation.

## Scope

This document describes how the async-socket-based Mini HTTP transport supplies the `INetworkProtocol*` interfaces and how the default channel bridge builds `IChannel*` communication on that transport. It intentionally treats HTTP and sockets as implementation details of the text-message transport.

Use [Inter-Process Async-Socket-Based Mini HTTP API](./KB_VlppOS_InterProcessAsyncSocketBasedMiniHttpApi.md) when an application needs raw asynchronous bytes, parsed HTTP/1.1 messages, or direct Mini HTTP request/response APIs.

All types in this document are in `vl::inter_process` unless a more specific namespace is shown.

## Layered Organization

The channel and protocol stack is:

| Layer | Main types | Responsibility |
| --- | --- | --- |
| Typed channels | `IChannelServer<TPackage>`, `IChannelClient<TPackage>`, `IChannel<TPackage>`, `IChannelReader<TPackage>` | Named typed-package delivery, client ids, direct sends, broadcasts and batching |
| Channel bridge | `NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>`, `NetworkProtocolChannelClient<TPackage, TSerialization>`, `NetworkProtocolLocalChannelClient<TPackage, TSerialization>` | Serializes channel batches to `WString` protocol messages |
| Text protocol | `INetworkProtocolServer`, `INetworkProtocolClient`, `INetworkProtocolConnection`, `INetworkProtocolCallback` | Asynchronous `WString` messages and connection lifecycle |
| Portable transport | `async_tcp_socket::SocketHttpServer`, `async_tcp_socket::SocketHttpClient` | Implements the text protocol with the async-socket-based Mini HTTP stack |

Feature code should normally depend on `IChannel*` or `INetworkProtocol*`. Bind it to `async_tcp_socket::SocketHttpServer` and `async_tcp_socket::SocketHttpClient` at the application composition boundary.

## Using the Network Protocol

`INetworkProtocolServer` owns the listening side. Derive a server from `async_tcp_socket::SocketHttpServer`, override `OnClientConnected`, and keep a thread-safe `INetworkProtocolCallback` alive for every accepted connection.

For an accepted connection:

1. Call `INetworkProtocolConnection::InstallCallback`.
2. Call `BeginReadingLoopUnsafe` after the callback is installed.
3. Return `WaitForClientResult::Accept`, or return `Reject` without using the connection.
4. Exchange nonempty `WString` messages with `SendString`.

The most-derived server destructor must call `Stop` before destroying fields that connection callbacks can access. From outside callbacks, `Stop` is the callback-draining shutdown boundary. A callback-reentrant `Stop` prevents further work but does not unwind the current callback, whose visible state must survive until it returns.

`INetworkProtocolClient` owns one logical connection. The normal client sequence is:

```C++
using namespace vl;
using namespace vl::inter_process;

auto client = Ptr<INetworkProtocolClient>(
    new async_tcp_socket::SocketHttpClient(L"/example", 8888)
    );
auto connection = client->GetConnection();

connection->InstallCallback(&callback);
client->WaitForServer();
if (client->GetStatus() == ClientStatus::Connected)
{
    connection->BeginReadingLoopUnsafe();
    connection->SendString(L"Hello");
    exchangeCompleted.Wait(); // Application/test event signaled by the callback.
}

connection->Stop();
connection->InstallCallback(nullptr);
```

Install the callback before `WaitForServer`, because connection events may be delivered while that call establishes the logical connection. Callback methods may run on arbitrary threads and must be thread-safe.

## Using Named Channels

The default bridge turns typed channel batches into `WString` messages and runs them over an `INetworkProtocol*` transport.

`TSerialization` must provide:

- `SourceType` equal to `collections::List<TPackage>`.
- `DestType` equal to `WString`.
- `ContextType` for optional serializer state.
- `Serialize` and `Deserialize` between a package list and one `WString`.

Use the portable Mini HTTP transport as the channel server base:

```C++
template<typename TPackage, typename TSerialization>
class ChannelServer
    : public NetworkProtocolChannelServer<
        TPackage,
        TSerialization,
        async_tcp_socket::SocketHttpServer
        >
{
    using Base = NetworkProtocolChannelServer<
        TPackage,
        TSerialization,
        async_tcp_socket::SocketHttpServer
        >;
    using SerializationContext = typename TSerialization::ContextType;
    using ChannelNameList = typename IChannelClient<TPackage>::ChannelNameList;

public:
    using Base::OnClientConnected;

    ChannelServer(
        const SerializationContext& serializationContext,
        const WString& baseUrl,
        vint port
        )
        : Base(serializationContext, baseUrl, port)
    {
    }

    ~ChannelServer()
    {
        Stop();
    }

    WaitForClientResult OnClientConnected(
        vint clientId,
        const ChannelNameList& channelNames,
        IChannelClient<TPackage>* localClient
        ) override
    {
        // Validate channelNames and accept or reject the participant.
        return WaitForClientResult::Accept;
    }
};
```

The server bridge performs the protocol callback setup and channel handshake. It assigns each accepted participant a positive client id and records which channel names that participant supports.

A network channel client derives from `NetworkProtocolChannelClient<TPackage, TSerialization>`, declares its supported names, and wraps a Socket HTTP protocol client:

```C++
template<typename TPackage, typename TSerialization>
class ChannelClient
    : public NetworkProtocolChannelClient<TPackage, TSerialization>
{
    using Base = NetworkProtocolChannelClient<TPackage, TSerialization>;
    using SerializationContext = typename TSerialization::ContextType;
    using ChannelNameList = typename IChannelClient<TPackage>::ChannelNameList;
    ChannelNameList channelNames;

public:
    ChannelClient(
        Ptr<INetworkProtocolClient> protocolClient,
        const SerializationContext& serializationContext
        )
        : Base(protocolClient, serializationContext)
    {
        channelNames.Add(L"Events");
    }

    const ChannelNameList& OnGetChannelNames() override
    {
        return channelNames;
    }
};

auto protocolClient = Ptr<INetworkProtocolClient>(
    new async_tcp_socket::SocketHttpClient(L"/example", 8888)
    );
auto channelClient = Ptr(
    new ChannelClient<TPackage, TSerialization>(
        protocolClient,
        serializationContext
        )
    );

auto&& channels = channelClient->GetChannels();
channels[L"Events"]->Initialize(&reader);
channelClient->WaitForServer();

bool disconnected = false;
channels[L"Events"]->BroadcastFromClient(package);
channels[L"Events"]->BatchWrite(disconnected);
```

The derived client can also handle `OnConnected`, `OnDisconnected`, `OnReadError` and `OnLocalError`.

`IChannelServer<TPackage>` routes messages but is not itself a channel participant. When server-side code must send ordinary channel messages, connect a `NetworkProtocolLocalChannelClient<TPackage, TSerialization>` with `ConnectLocalClient`; this gives the local participant a normal positive client id.

## How Socket HTTP Implements the Protocol

`async_tcp_socket::SocketHttpServer`, defined in `Source/InterProcess/AsyncSocket/AsyncSocket_HttpServer.h`, derives from `async_tcp_socket::SocketHttpServerApi` and implements `INetworkProtocolServer`. `async_tcp_socket::SocketHttpClient`, defined in `Source/InterProcess/AsyncSocket/AsyncSocket_HttpClient.h`, implements both `INetworkProtocolClient` and its single logical `INetworkProtocolConnection`.

The transport keeps a logical connection above short-lived or replaceable physical HTTP connections. These routes are relative to the configured `baseUrl`:

- `GET /VlppInterProcess/Connect` creates one logical connection token.
- `POST /VlppInterProcess/Request/{token}` is the client-maintained long poll for server-to-client messages.
- `POST /VlppInterProcess/Response/{token}` sends one client-to-server message and may return one server message in the same response.

Each normal body is the direct UTF-8 encoding of one `WString`. The media type is `application/json; charset=utf8`, but the body is not JSON syntax.

The client uses two physical `async_tcp_socket::SocketHttpClientApi` lanes for one logical token. One lane keeps the receive poll alive; the other serializes connection control and client sends. Replacing a failed physical lane does not replace the logical `INetworkProtocolConnection` unless the transport reports final disconnection.

The server maps the token to its logical connection, queues server messages when no receive request is available, and dispatches client messages through `INetworkProtocolCallback::OnReadString`. The channel bridge above this layer is unaware of the HTTP routes and physical lanes.

## Starting on Windows, Linux and macOS

High-level Socket HTTP construction and startup are identical on all supported platforms:

```C++
typename TSerialization::ContextType serializationContext{};
ChannelServer<TPackage, TSerialization> server(
    serializationContext,
    L"/example",
    8888
    );
server.Start();

{
    auto protocolClient = Ptr<INetworkProtocolClient>(
        new async_tcp_socket::SocketHttpClient(L"/example", 8888)
        );
    auto channelClient = Ptr(
        new ChannelClient<TPackage, TSerialization>(
            protocolClient,
            serializationContext
            )
        );
    auto&& channels = channelClient->GetChannels();
    channels[L"Events"]->Initialize(&reader);
    channelClient->WaitForServer();

    // Exchange channel packages, then stop before callback owners are destroyed.
    protocolClient->GetConnection()->Stop();
}

server.Stop();
```

The common implementation selects the native loopback backend at build time:

| Platform guard | Native server | Native client |
| --- | --- | --- |
| `VCZH_MSVC` | `async_tcp_socket::windows_socket::AsyncSocketServer` | `async_tcp_socket::windows_socket::AsyncSocketClient` |
| `VCZH_GCC && VCZH_APPLE` | `async_tcp_socket::macos_socket::AsyncSocketServer` | `async_tcp_socket::macos_socket::AsyncSocketClient` |
| `VCZH_GCC && !VCZH_APPLE` | `async_tcp_socket::linux_socket::AsyncSocketServer` | `async_tcp_socket::linux_socket::AsyncSocketClient` |

Use `VlppOS.Windows.h` on Windows; Winsock initialization and `Ws2_32.lib` linkage are internal. Use `VlppOS.Linux.h` on Linux and link `liburing`. Use the same `VlppOS.Linux.h` umbrella on macOS; custom builds must enable Clang Blocks and link CoreFoundation and Network.framework. The repository projects and build scripts already supply these platform settings.

`SocketHttpServer::Start` creates the platform listener internally. The default `SocketHttpClient` constructor creates platform clients internally. `WaitForServer` blocks while establishing the logical connection and returns after connection or after the client reaches a terminal stopped state, so call it on a thread that may block. Both sides use IPv4 loopback; they do not expose a remote-host option.

Use the `SocketHttpClient(const WString&, vint, NativeClientFactory)` overload only when the composition root must select the native client explicitly. The factory receives the port and must return a fresh non-null `IAsyncSocketClient` for every initial or replacement physical connection.

`baseUrl` is empty for the origin root or an ASCII origin-form prefix such as `/example`. A nonempty `baseUrl` must start with `/`, must not end with `/`, and must not contain a query, fragment, backslash, NUL, malformed escape, or encoded path separator. Server and client must use the same `baseUrl` and port.

For the lower socket, HTTP request and Mini HTTP API startup rules on each platform, see [Inter-Process Async-Socket-Based Mini HTTP API](./KB_VlppOS_InterProcessAsyncSocketBasedMiniHttpApi.md).
