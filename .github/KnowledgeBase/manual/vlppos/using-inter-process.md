# Using Inter-Process Channels

The **vl::inter_process** namespace is organized in layers. Feature code normally uses the channel interfaces, because they describe typed named message delivery between connected participants. Transport code implements the raw network protocol interfaces, because they describe how one server and many clients exchange asynchronous **WString** messages. Portable async-socket transports and Windows-specific transports are composition choices that connect these two layers.

VlppOS provides the **INetworkProtocol** and **IChannel** interface families only for test-purpose local cross-process communication. Applications may use these public interfaces as integration contracts, but product code is strongly recommended to provide its own implementations instead of treating the bundled transports and channel stack as production networking.

Despite the names, these contracts are not tied to TCP/IP. Any mechanism that provides the required asynchronous message and lifecycle behavior can fit behind them, including stdio, DLL calls, shared memory, WebAssembly host calls, pipes, sockets, HTTP or a custom network protocol.

Keep this boundary in application code:
- Use **IChannelServer\<TPackage\>**, **IChannelClient\<TPackage\>**, **IChannel\<TPackage\>** and **IChannelReader\<TPackage\>** when the feature sends and receives packages.
- Use **INetworkProtocolServer**, **INetworkProtocolClient**, **INetworkProtocolConnection** and **INetworkProtocolCallback** when adding a new pipe, socket, HTTP, shared-memory or other underlying transport.
- Use **NetworkProtocolChannelServer\<TPackage, TSerialization, TServerBase\>** and **NetworkProtocolChannelClient\<TPackage, TSerialization\>** when a channel should run on top of a raw protocol implementation.

## Channel Interfaces for Features

A channel client is a participant with a server-assigned positive client id and a set of named channels. A channel is not a socket and it is not owned by the server alone. It is a logical lane shared by every connected participant that declares the same channel name.

The channel interfaces are the stable API for feature code:
- **IChannelReader\<TPackage\>::OnRead** receives the sender client id and one typed package.
- **IChannel\<TPackage\>** exposes **Initialize**, **SendToClient**, **BroadcastFromClient** and **BatchWrite**.
- **IChannelClient\<TPackage\>** exposes channel names, generated channel objects, client status, **WaitForServer** and lifecycle callbacks.
- **IChannelServer\<TPackage\>** accepts or rejects clients, records client ids, supports in-process local clients and broadcasts fatal channel errors.

Channel names are user-defined strings. A user channel name must not be empty and must not contain **!**. Names beginning with **!**, including **!Error** and **!System**, are reserved for the implementation.

**Initialize** installs one reader and replays messages that arrived before the reader was installed. There is no reader uninstallation. **SendToClient** queues a direct message to one client id. **BroadcastFromClient** queues a message for all other participants on the same channel, optionally excluding selected receiver ids. **BatchWrite** flushes queued messages and reports disconnection through its output argument.

**IChannelServer\<TPackage\>** manages delivery and bookkeeping. It is not itself a channel speaker. When server-side logic must send normal channel messages, create a **NetworkProtocolLocalChannelClient\<TPackage, TSerialization\>**, connect it with **ConnectLocalClient**, and send through that local client's channel objects. This keeps every normal channel message tied to a real positive sender client id.

A small channel client follows this shape:
```C++
using namespace vl;
using namespace vl::collections;
using namespace vl::inter_process;

constexpr const wchar_t* ChatChannelName = L"Chat";

struct WStringListSerializer
{
    using SourceType = List<WString>;
    using DestType = WString;
    using ContextType = std::nullptr_t;

    static void Serialize(const ContextType&, const SourceType& source, DestType& dest)
    {
        dest = WString::Empty;
        for (auto&& item : source)
        {
            dest += itow(item.Length()) + L":" + item;
        }
    }

    static void Deserialize(const ContextType&, const DestType& dest, SourceType& source)
    {
        source.Clear();
        const wchar_t* reading = dest.Buffer();
        const wchar_t* end = reading + dest.Length();
        while (reading < end)
        {
            auto delimiter = wcschr(reading, L':');
            CHECK_ERROR(delimiter && delimiter < end, L"Invalid package.");

            auto length = wtoi(WString::CopyFrom(reading, (vint)(delimiter - reading)));
            CHECK_ERROR(length >= 0 && delimiter + 1 + length <= end, L"Invalid package length.");

            source.Add(WString::CopyFrom(delimiter + 1, length));
            reading = delimiter + 1 + length;
        }
    }
};

using ChatChannelClientBase =
    NetworkProtocolChannelClient<WString, WStringListSerializer>;

class ChatClient
    : public ChatChannelClientBase
    , public virtual IChannelReader<WString>
{
private:
    IChannelClient<WString>::ChannelMap channelNames;
    IChannel<WString>* chat = nullptr;

public:
    ChatClient(Ptr<INetworkProtocolClient> transport)
        : ChatChannelClientBase(transport)
    {
        channelNames.Add(ChatChannelName, nullptr);
        auto&& channels = GetChannels();
        auto index = channels.Keys().IndexOf(ChatChannelName);
        CHECK_ERROR(index != -1, L"Missing chat channel.");
        chat = channels.Values()[index];
        chat->Initialize(this);
    }

    const IChannelClient<WString>::ChannelNameList& OnGetChannelNames() override
    {
        return channelNames.Keys();
    }

    void OnConnected(vint clientId) override
    {
        // Record the assigned positive client id if the feature needs it.
    }

    void OnRead(vint senderClientId, const WString& package) override
    {
        // Handle one typed package from senderClientId.
    }

    void Send(vint receiverClientId, const WString& package)
    {
        bool disconnected = false;
        chat->SendToClient(receiverClientId, package);
        chat->BatchWrite(disconnected);
        if (disconnected)
        {
            // Stop using this client.
        }
    }
};
```

## Network Protocol Interfaces for Transports

The raw protocol layer is the extension point for a new underlying implementation. It intentionally knows nothing about channel names or package types. One **INetworkProtocolConnection** exchanges asynchronous **WString** messages with the other side. This message contract can be implemented by a local pipe or socket, stdio, DLL calls, shared memory, a WebAssembly host bridge or another mechanism; the name **INetworkProtocol** does not require an IP network.

Implement these interfaces for a new transport:
- **INetworkProtocolConnection**: install or uninstall one callback, start the asynchronous read loop, send one string message and stop the connection.
- **INetworkProtocolCallback**: receive strings, remote errors, local errors, connected and disconnected events. Callback methods may run on any thread, so implementations and users must be thread-safe.
- **INetworkProtocolClient**: own one connection, block in **WaitForServer** until the connection is established or fails, and report **ClientStatus**.
- **INetworkProtocolServer**: start accepting, call **OnClientConnected** with **Ptr\<INetworkProtocolConnection\>** for accepted transport connections, stop all owned connections and report stopped state.

Connection-admission callbacks carry the producer's existing owning **Ptr** through the asynchronous socket, HTTP request, network protocol and channel layers. An override should accept that **Ptr** and forward or store it without constructing another **Ptr** from the callback argument or from **.Obj()**. Use **.Obj()** only when an existing operation requires a temporary raw pointer; doing so does not transfer ownership.

**InstallCallback** should call **OnInstalled** with the connection. Passing **nullptr** uninstalls the callback. **BeginReadingLoopUnsafe** starts receiving messages asynchronously, but callers must tolerate implementations that have already received data after the callback was installed. **Stop** is the shutdown boundary; after it returns, pending transport callbacks should no longer touch the stopped object.

A raw client must return a connection object from **GetConnection**, but using that connection before **WaitForServer** finishes is outside the contract. A raw server should not report client-connected callbacks before **Start** or after **Stop**. If a connection is lost, the same connection object does not reconnect; a reconnecting transport should create a new connection object.

After a raw transport exists, reuse the channel bridge instead of reimplementing channel routing:
```C++
class MyConnection : public Object, public virtual INetworkProtocolConnection
{
    // Store one callback, frame transport bytes into WString messages,
    // call OnReadString / OnLocalError / OnDisconnected, and drain callbacks in Stop.
};

class MyClient : public Object, public virtual INetworkProtocolClient
{
    // Own one MyConnection and implement WaitForServer / GetStatus.
};

class MyServer : public Object, public virtual INetworkProtocolServer
{
    // Accept MyConnection objects and pass their existing owning Ptr
    // unchanged to OnClientConnected.
};

using MyChannelServer =
    NetworkProtocolChannelServer<MyPackage, MyPackageListSerializer, MyServer>;

using MyChannelClient =
    NetworkProtocolChannelClient<MyPackage, MyPackageListSerializer>;
```

## Default Channel Bridge

**NetworkProtocolChannelServer**, **NetworkProtocolChannelClient** and **NetworkProtocolLocalChannelClient** translate channel batches into raw **WString** protocol messages. The bridge requires a serializer where:
- **TSerialization::SourceType** is **collections::List\<TPackage\>**.
- **TSerialization::DestType** is **WString**.
- **TSerialization::ContextType** stores optional serializer state, or **std::nullptr_t** when no state is required.
- **Serialize** converts a package list to one **WString**.
- **Deserialize** converts one **WString** back to a package list.

The bridge uses **NetworkPackage** as the raw text envelope. Its string shape is **clientId,extraClientId1,...;channelName;messageBody**. For client-to-server messages, **clientId** is the direct receiver id; an empty id means broadcast, and extra ids are blocked broadcast receivers. For server-to-client messages, **clientId** is the positive sender id.

A network channel client connects by sending one handshake package with an empty client id, an empty channel name and all supported channel names joined by **!**. The server validates the names, assigns a positive client id, records channel membership and sends the id back in an empty-channel response. After that, normal channel packages are delivered only when sender and receiver membership matches the channel name.

A channel server over a Windows named pipe can be declared like this:
```C++
#include <vlppos.h>
#include <VlppOS.Windows.h>

using namespace vl;
using namespace vl::inter_process;

class ChatServer
    : public NetworkProtocolChannelServer<WString, WStringListSerializer, named_pipe::NamedPipeServer>
{
public:
    ChatServer(const WString& pipeName)
        : NetworkProtocolChannelServer<WString, WStringListSerializer, named_pipe::NamedPipeServer>(pipeName)
    {
    }

    WaitForClientResult OnClientConnected(
        vint clientId,
        const IChannelClient<WString>::ChannelNameList& availableChannels,
        Ptr<IChannelClient<WString>> localClient) override
    {
        return availableChannels.Contains(ChatChannelName)
            ? WaitForClientResult::Accept
            : WaitForClientResult::Reject;
    }
};

auto server = Ptr(new ChatServer(L"ExamplePipe"));
server->Start();
```

## Portable Async-Socket Implementations

The types in **vl::inter_process::async_tcp_socket** form a portable, loopback-only stack for asynchronous TCP bytes, HTTP/1.1 requests and Mini HTTP services. Platform-neutral factories select **windows_socket::AsyncSocketServer** and **AsyncSocketClient**, **linux_socket::AsyncSocketServer** and **AsyncSocketClient**, or **macos_socket::AsyncSocketServer** and **AsyncSocketClient** at the application composition boundary.

### Direct Length-Framed Transport

Use **async_tcp_socket::NetworkProtocolServer\<TAsyncSocketServer\>** and **async_tcp_socket::NetworkProtocolClient\<TAsyncSocketClient\>** when both peers use VlppOS and HTTP compatibility is unnecessary. One logical connection is one physical TCP socket. Each **WString** is framed with its character count and native **wchar_t** bytes, so this is not a portable wire encoding between peers with different **wchar_t** representations.

These templates construct their socket backend from forwarded constructor arguments, so choose the concrete type for the target: **windows_socket::AsyncSocketServer** and **AsyncSocketClient**, **linux_socket::AsyncSocketServer** and **AsyncSocketClient**, or **macos_socket::AsyncSocketServer** and **AsyncSocketClient**, and pass the port to the protocol adapter. They do not accept an injected **Ptr\<IAsyncSocketServer\>** or **Ptr\<IAsyncSocketClient\>**.

**IAsyncSocketServer**, **IAsyncSocketClient**, **IAsyncSocketConnection** and **IAsyncSocketCallback** expose the underlying ordered full-duplex byte streams. The interface-returning **CreateDefaultAsyncSocketServer(port)** and **CreateDefaultAsyncSocketClient(port)** factories are for code that consumes these socket interfaces, including the injected **SocketHttpServer** and **SocketHttpClient** dependencies below. Each socket exposes its immutable loopback port through **GetPort**, and **IAsyncSocketClient::CreateSameEndpointClient** creates a fresh client for another physical lane on the same endpoint.

### HTTP-Compatible Socket Transport

Use **async_tcp_socket::SocketHttpServer** and **async_tcp_socket::SocketHttpClient** when the transport must use the VlppOS HTTP wire protocol or interoperate with the Windows HTTP implementation. They implement **INetworkProtocolServer**, **INetworkProtocolClient** and **INetworkProtocolConnection** over the portable Mini HTTP stack.

The public construction surface keeps native socket composition explicit:
```C++
SocketHttpServer(
    Ptr<IAsyncSocketServer> socketServer,
    const WString& urlPrefix
    );

SocketHttpClient(
    Ptr<IAsyncSocketClient> socketClient,
    const WString& server,
    const WString& urlPrefix
    );
```

The server adapter uses the injected listener and never creates another one. The client uses the injected client for its first physical lane and calls **CreateSameEndpointClient** for the additional lanes required by full-duplex polling and recovery. The server name must select loopback, and the URL prefix is empty for the origin root or begins with **/**.

The adapter uses the same **/VlppInterProcess/Connect**, **/VlppInterProcess/Request/{token}** and **/VlppInterProcess/Response/{token}** routes as **windows_http::HttpServer** and **windows_http::HttpClient**. The protocol has no message-level delivery acknowledgement, deduplication, heartbeat or disconnect route, so it must not be treated as exactly-once delivery.

### Portable Mini HTTP Request Helpers

**async_tcp_socket::HttpRequest**, **HttpResponse**, **HttpRequestServer**, **HttpRequestClient** and **HttpRequestConnection** provide the lower HTTP/1.1 message layer. Use them when code needs complete binary-safe request and response objects and controls each connection directly.

**async_tcp_socket::SocketHttpServerApi**, **SocketHttpRequestContext** and **SocketHttpClientApi** add prefix dispatch and request/response conveniences. A server API receives **(socketServer, urlPrefix)**, and multiple APIs share one listener only when they receive the exact same server object. Override **OnHttpRequestReceived**, inspect the relative path, query or UTF-8 body, and complete the one-shot context with **Respond**, **RespondStatus**, **RespondBytes**, **RespondUtf8** or **Cancel**. A client API receives **(socketClient, server)** and queues asynchronous **HttpQuery** calls.

**SocketHttpClientApi** uses the **windows_http::HttpRequest**, **HttpResponse** and **HttpError** value types, but those types are platform-neutral and using them here does not invoke WinHTTP. The TCP, HTTP and Mini HTTP implementations in this section are local test, validation and demonstration infrastructure rather than a production web stack.

## Windows Implementations

Named pipes and the legacy HTTP.sys/WinHTTP transport are Windows-only. They are available for local tests and as reference implementations for the raw protocol contract, but do not make portable feature code depend on them or treat them as production communication stacks.

### Named Pipe Transport

**vl::inter_process::named_pipe::NamedPipeServer** implements **INetworkProtocolServer**. **vl::inter_process::named_pipe::NamedPipeClient** derives from **vl::inter_process::named_pipe::NamedPipeConnection** and implements **INetworkProtocolClient**. **vl::inter_process::named_pipe::NamedPipeConnection** implements **INetworkProtocolConnection**.

The server starts overlapped named-pipe accepting in **Start** and owns both accepted connections and pending accepts. The client opens **\\.\pipe\NAME**, waits for the server and switches the pipe into message-read mode. **SendString** frames one **WString** with length data. The implementation chunks writes because a Windows named pipe message is limited to 64K bytes. Broken-pipe cases become disconnection events, and fatal local pipe failures become **OnLocalError** followed by disconnection.

### HTTP Transport

**vl::inter_process::windows_http::HttpServer** derives from **vl::inter_process::windows_http::HttpServerApi** and implements **INetworkProtocolServer**. **vl::inter_process::windows_http::HttpClient** implements both **INetworkProtocolClient** and **INetworkProtocolConnection**. This transport is a raw protocol implementation over Windows HTTP APIs, not a general web framework.

The raw HTTP protocol uses these routes under the configured base URL:
- **GET /VlppInterProcess/Connect** creates a logical connection and returns per-connection request and response URLs.
- **POST /VlppInterProcess/Request/GUID** is the client-maintained long-poll request for server-to-client messages.
- **POST /VlppInterProcess/Response/GUID** sends client-to-server messages and may also receive one queued server-to-client message.

**vl::inter_process::windows_http::HttpClient::WaitForServer** sends the connect request, validates the returned URLs, records them and reports connection. **BeginReadingLoopUnsafe** starts the long-poll request loop. **SendString** posts to the response URL. Connect and response failures retry a limited number of times; request failures retry while the client is still running.

**vl::inter_process::windows_http::HttpServer** creates a **vl::inter_process::windows_http::HttpServerConnection** for each connect request. Server-to-client messages are returned through a pending long-poll request when possible, or queued until the next request. Client-to-server request bodies are dispatched as inbound strings. When the server stops, pending long-poll requests are cancelled and connection callbacks receive disconnection.

### Windows HTTP Helper APIs

**vl::inter_process::windows_http::HttpClientApi** and **vl::inter_process::windows_http::HttpServerApi** are lower-level Windows helpers used by **vl::inter_process::windows_http::HttpClient** and **vl::inter_process::windows_http::HttpServer**. Use them directly when a Windows feature needs asynchronous HTTP request and response behavior without adopting the raw **INetworkProtocol** transport shape.

**vl::inter_process::windows_http::HttpClientApi** owns one WinHTTP session and connection for a host and port. **HttpQuery** sends one asynchronous request described by **vl::inter_process::windows_http::HttpRequest**, whose fields include method, query, body, content type, accept types, credentials, cookies, extra headers and timeouts. Results are either **vl::inter_process::windows_http::HttpResponse** or **vl::inter_process::windows_http::HttpError**; HTTP status codes such as 404 are represented as **HttpResponse** values.

**vl::inter_process::windows_http::HttpServerApi** owns one HTTP.sys URL prefix. Override **OnHttpRequestReceived** to dispatch requests and **OnHttpServerStopping** to clean up feature state. Helpers such as **GetUtf8Body**, **SendResponse** and **SendResponseUtf8** handle common request-body and response tasks.

