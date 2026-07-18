# Using Inter-Process Channels

The **vl::inter_process** namespace is organized in layers. Feature code normally uses the channel interfaces, because they describe typed named message delivery between connected participants. Transport code implements the raw network protocol interfaces, because they describe how one server and many clients exchange asynchronous **WString** messages. Portable async-socket transports and Windows-specific transports are composition choices that connect these two layers.

The transport-agnostic raw-protocol and channel interfaces, together with the default channel bridge, can be used in production. Every concrete raw-protocol implementation supplied by VlppOS is testing, validation and demonstration infrastructure and should not be used as a production transport. A production application should implement its own **INetworkProtocolServer**, **INetworkProtocolClient** and **INetworkProtocolConnection** for its security, deployment, performance, reconnection and delivery requirements, then reuse the channel layer over that implementation.

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

The raw protocol layer is the extension point for a new underlying implementation. It intentionally knows nothing about channel names or package types. One **INetworkProtocolConnection** exchanges asynchronous **WString** messages with the other side.

Implement these interfaces for a new transport:
- **INetworkProtocolConnection**: install or uninstall one callback, start the asynchronous read loop, send one string message and stop the connection.
- **INetworkProtocolCallback**: receive strings, remote errors, local errors, connected and disconnected events. Callback methods may run on any thread, so implementations and users must be thread-safe.
- **INetworkProtocolClient**: own one connection, block in **WaitForServer** until the connection is established or fails, and report **ClientStatus**.
- **INetworkProtocolServer**: start accepting, call **OnClientConnected** for accepted transport connections, stop all owned connections and report stopped state.

**InstallCallback** should call **OnInstalled** with the connection. Passing **nullptr** uninstalls the callback. **BeginReadingLoopUnsafe** starts receiving messages asynchronously, but callers must tolerate implementations that have already received data after the callback was installed. **Stop** is the shutdown boundary; after it returns, pending transport callbacks should no longer touch the stopped object.

A raw client must return a connection object from **GetConnection**, but using that connection before **WaitForServer** finishes is outside the contract. A raw server should not report client-connected callbacks before **Start** or after **Stop**. Once a logical connection reports disconnection, that object does not reconnect. A transport may replace internal physical connections before logical disconnection; the portable Socket HTTP client uses this distinction for its two physical lanes.

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
    // Accept MyConnection objects and pass them to OnClientConnected.
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
        IChannelClient<WString>* localClient) override
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

VlppOS offers two portable raw protocol choices over native async sockets. Both bind to **windows_socket::AsyncSocketServer/Client**, **linux_socket::AsyncSocketServer/Client**, or **macos_socket::AsyncSocketServer/Client** at the application composition boundary.

### Direct Length-Framed Transport

Use **async_tcp_socket::NetworkProtocolServer\<TAsyncSocketServer\>** and **async_tcp_socket::NetworkProtocolClient\<TAsyncSocketClient\>** when both peers use VlppOS and HTTP compatibility is unnecessary. One logical connection is one physical socket. Each **WString** is framed with its character count and native **wchar_t** bytes. The implementation owns callback and write draining; **Stop** is a hard boundary and supports reentrant calls from a current callback.

### HTTP-Compatible Socket Transport

Use **async_tcp_socket::SocketHttpServer** and **async_tcp_socket::SocketHttpClient** when the transport must use the legacy VlppOS HTTP wire protocol or interoperate with the Windows HTTP implementation.

The public construction surface is:

```C++
SocketHttpServer(const WString& baseUrl, vint port);

using NativeClientFactory = Func<Ptr<IAsyncSocketClient>(vint)>;
SocketHttpClient(const WString& baseUrl, vint port);
SocketHttpClient(const WString& baseUrl, vint port, NativeClientFactory clientFactory);
```

The default client factory selects the current platform's native async-socket client. The factory overload is per logical client and must return a fresh non-null native client for every initial or replacement physical connection.

The base URL is empty for the origin root or an ASCII origin-form prefix beginning with **/** and having no trailing slash, query, fragment, backslash, NUL, malformed escape, or encoded separator. The server listens at **http://localhost:PORT{baseUrl}** and the client sends the exact **localhost:PORT** HTTP authority.

The HTTP adapter has one logical token and two physical client lanes:

```text
logical connection {token}
|- receive lane: one infinite POST /VlppInterProcess/Request/{token}
`- send lane: GET /VlppInterProcess/Connect, then FIFO POST /VlppInterProcess/Response/{token}
```

**/Connect** creates one server logical connection and returns the two token-bearing paths. The receive lane submits a replacement poll before delivering a nonempty response to **OnReadString**. The send lane accepts one nonempty NUL-free **WString** per request, encodes it as direct UTF-8 bytes, and keeps one active FIFO head so retries cannot be overtaken. A server message generated synchronously while handling the same **/Response** can be piggybacked in that HTTP response; otherwise server messages complete the pending long poll in FIFO order.

HTTP status, exact content type, body size, UTF-8, NUL, and returned-path validation belong to the adapter. A non-200 or malformed response retries on the healthy physical API. A terminal transport failure replaces only the affected physical lane through **NativeClientFactory**, retaining the logical token. **/Connect** and **/Response** stop after three failed attempts with two nonfatal local errors followed by one fatal local error; **/Request** retries silently while running.

**Stop** rejects new sends, gives accepted sends a bounded drain opportunity, cancels the infinite poll, drains replacement workers and lower callbacks, and reports **OnDisconnected** once. It can be called repeatedly or from an adapter callback; a reentrant call waits for other callback frames but not the current frame.

The protocol has no acknowledgement, deduplication, heartbeat, or disconnect route. A response lost after the server processes **/Response** can cause the client message to be delivered twice. Do not treat this transport as exactly-once.

### Portable HTTP Request Helpers

**async_tcp_socket::SocketHttpServerApi** and **async_tcp_socket::SocketHttpClientApi** are lower request/response helpers, not raw protocol transports. The server API owns prefix dispatch, response framing, CORS and callback draining. **SocketHttpRequestContext::Respond** is one-shot and reports whether its physical response completed; **Cancel** abandons a pending context. One client API serializes HTTP exchanges on one physical connection and becomes terminal after transport/framing/timeout failure. **HttpRequestServer**, **HttpRequestClient**, and **HttpRequestConnection** are the still-lower HTTP/1.1 layer.

## Windows Implementations

Named pipes and the legacy HTTP.sys/WinHTTP transport are Windows-only. Treat them as ready-to-use Windows transports and reference implementations, but keep portable feature code on interfaces or the portable async-socket choices.

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

