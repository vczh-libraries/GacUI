# Inter-Process Async-Socket-Based Mini HTTP API

## Testing-Only Protocol Implementations

The bundled async-socket, HTTP request and Mini HTTP APIs described here, including the concrete `INetworkProtocol*` implementations built on them, are testing, validation and demonstration infrastructure. Do not use these implementations as production networking infrastructure. This restriction does not apply to the transport-agnostic `INetworkProtocol*` and `IChannel*` interfaces or the channel implementation built on them, which can be used in production with a production-ready protocol. Applications that need production inter-process communication should implement their own `INetworkProtocolServer`, `INetworkProtocolClient` and `INetworkProtocolConnection` to meet their security, deployment, performance, reconnection and delivery requirements.

## Purpose and Layered Organization

The types in `vl::inter_process::async_tcp_socket` form a portable, loopback-only stack for asynchronous TCP bytes and HTTP/1.1 request/response work. Applications can enter at the layer matching the abstraction they need:

| Layer | Main types | Responsibility |
| --- | --- | --- |
| Socket HTTP protocol and channels | `SocketHttpServer`, `SocketHttpClient`, `NetworkProtocolChannelServer`, `NetworkProtocolChannelClient` | Optional `WString` protocol over Mini HTTP and typed named channels over that protocol |
| Mini HTTP API | `SocketHttpServerApi`, `SocketHttpRequestContext`, `SocketHttpClientApi` | URL-prefix dispatch, parsed request conveniences, normalized response builders and asynchronous queries |
| HTTP/1.1 messages | `HttpRequestServer`, `HttpRequestClient`, `HttpRequestConnection`, `IHttpRequestConnection` | Parse, serialize and sequence complete HTTP messages; provide canonical framing, field, body and strict UTF-8 primitives |
| Asynchronous bytes | `IAsyncSocketServer`, `IAsyncSocketClient`, `IAsyncSocketConnection`, `AsyncSocketBuffer` | Ordered full-duplex byte streams |
| Native backend | Platform `AsyncSocketServer` and `AsyncSocketClient` | Winsock/IOCP, `io_uring`, or Network.framework |

The lower layers do not perform URL routing or application policy. The Mini HTTP layer does not expose raw socket framing. The `SocketHttpServer` / `SocketHttpClient` protocol adapter and its channel composition are optional consumers above Mini HTTP; their usage is documented in [Inter-Process Network Protocols and Channels](./KB_VlppOS_InterProcessNetworkProtocolsAndChannels.md).

## Choosing an Entry Point

- Use `IAsyncSocket*` when the application owns its byte framing and protocol state machine.
- Use `HttpRequestServer`, `HttpRequestClient` and `IHttpRequestConnection` when the application needs complete, binary-safe HTTP/1.1 message objects and controls each connection directly.
- Use `SocketHttpServerApi` and `SocketHttpClientApi` when the application needs a small loopback HTTP service with prefix routing, normalized responses and queued asynchronous queries.
- Use `SocketHttpServer` / `SocketHttpClient` or the channel bridge when the application needs asynchronous `WString` messages or typed named channels instead of request/response semantics.

## Platform Backends and Startup

All native implementations bind and connect to numeric IPv4 loopback. The public port constructor does not accept a remote host.

| Platform | Native types | Definition | Build dependency |
| --- | --- | --- | --- |
| Windows | `windows_socket::AsyncSocketServer`, `windows_socket::AsyncSocketClient` | `Source/InterProcess/AsyncSocket/AsyncSocket.Windows.h` | Winsock; the implementation links `Ws2_32.lib` and manages `WSAStartup` internally |
| Linux | `linux_socket::AsyncSocketServer`, `linux_socket::AsyncSocketClient` | `Source/InterProcess/AsyncSocket/AsyncSocket.Linux.h` | `liburing` and `-luring` |
| macOS | `macos_socket::AsyncSocketServer`, `macos_socket::AsyncSocketClient` | `Source/InterProcess/AsyncSocket/AsyncSocket.macOS.h` | Clang Blocks, CoreFoundation and Network.framework |

The repository build scripts already supply these dependencies. A custom build must preserve the same link and compile options.

Platform-neutral code selects the compiled native backend through the common factories declared in `AsyncSocket.h`:

```C++
auto socketServer = CreateDefaultAsyncSocketServer(port);
auto socketClient = CreateDefaultAsyncSocketClient(port);
```

The released platform umbrella is `VlppOS.Windows.h` on Windows and `VlppOS.Linux.h` on Linux and macOS. Each compiled platform translation unit defines the same two factories. Higher HTTP layers never select a backend internally: inject the returned server or client at the composition boundary. `SocketHttpClient` takes only one client pointer, uses that exact object for its first physical lane, and obtains the additional independent lanes required by the logical protocol through `IAsyncSocketClient::CreateSameEndpointClient()`.

## Async Socket API

The async socket interfaces are defined in `Source/InterProcess/AsyncSocket/AsyncSocket.h`. `IAsyncSocketServer::GetPort()` and `IAsyncSocketClient::GetPort()` return the immutable loopback port selected during construction, so higher adapters do not accept a duplicate port argument. `IAsyncSocketClient::CreateSameEndpointClient()` returns a distinct fresh `Ready` client with the same transport configuration and port; it is the transport-preserving way for a multi-lane adapter to acquire another physical connection without accepting a separate factory.

### Connections and Callbacks

`IAsyncSocketConnection` is an ordered, full-duplex byte stream.

- `InstallCallback` installs one non-owning `IAsyncSocketCallback`. `OnInstalled` runs synchronously with the connection.
- `BeginReadingLoopUnsafe` starts asynchronous reads after the callback is ready.
- `WriteAsync` accepts a `Ptr<AsyncSocketBuffer>`. The connection retains the whole buffer until the write finishes and calls `OnWriteCompleted` after all bytes have been sent.
- Only one write should be outstanding at this layer. Maintain a queue and submit its next buffer from completion handling when multiple writes are needed.
- `OnRead` receives a positive borrowed byte block. Copy any bytes that must survive the callback.
- `OnError`, `OnConnected` and `OnDisconnected` report lifecycle changes. Callbacks may run on arbitrary threads and must be thread-safe.
- From outside callbacks, `Stop` cancels and drains the connection. A callback-reentrant `Stop` prevents further work but cannot unwind the current callback; keep callback-visible state alive until that callback returns and perform final teardown outside it.

### Server and Client Sequence

An `IAsyncSocketServerCallback` accepts or rejects each physical connection. For an accepted connection, install its `IAsyncSocketCallback`, start reading, and then return `WaitForClientResult::Accept`.

```C++
auto server = CreateDefaultAsyncSocketServer(port);
server->Start(&serverCallback);

auto client = CreateDefaultAsyncSocketClient(port);
auto connection = client->GetConnection();
connection->InstallCallback(&clientCallback);
client->WaitForServer();
if (client->GetStatus() == ClientStatus::Connected)
{
    connection->BeginReadingLoopUnsafe();

    auto buffer = Ptr(new AsyncSocketBuffer);
    buffer->data.Resize(size);
    // Fill buffer->data before submitting it.
    connection->WriteAsync(buffer);
    writeCompleted.Wait(); // Signaled by the callback's OnWriteCompleted.
}

connection->Stop();
connection->InstallCallback(nullptr);
server->Stop();
```

Keep both callback objects alive until their corresponding stop calls finish. `IAsyncSocketClient::WaitForServer` blocks its caller while native connection attempts and retry delays remain asynchronous; the common policy uses `AsyncSocketClientRetryCount` attempts separated by `AsyncSocketClientRetryDelay` milliseconds. `IAsyncSocketServer::Start` can throw `AsyncSocketServerStartException`, whose failure is `AddressInUse` or `Other`.

## HTTP/1.1 Request Layer

The request layer adapts each `IAsyncSocketConnection` to an `IHttpRequestConnection`. It parses arbitrary socket blocks into complete HTTP/1.1 messages and serializes application-created messages back to bytes.

### Message Values

The binary-safe message values are defined in `Source/InterProcess/AsyncSocket/HttpRequest.h`:

- `HttpRequest` contains `HttpVersion`, method, exact request target, ordered headers and `HttpBody`.
- `HttpResponse` contains version, status code, reason, ordered headers and `HttpBody`.
- `HttpField` stores a `WString` name and byte-array value. Parsed field names are normalized to lowercase, and leading/trailing optional whitespace is removed from parsed values; the remaining values are not text-decoded.
- `HttpBody` stores binary chunks and ordered trailers. Chunk boundaries and trailers are preserved for chunked messages.
- `ParseHttpRequestBodyToChunks` is a framing helper used by the parser; application code normally consumes the completed body instead.

### Canonical Analysis and Conversion Helpers

`HttpRequest.h` also exposes the protocol-neutral helpers used by the parser, serializer and higher layers. `AnalyzeHttpFraming(fields, framing)` is the single canonical analysis of `Content-Length`, `Transfer-Encoding` and `Connection: close` fields. It resets `framing` on entry; the output is authoritative only when the result is `HttpFramingAnalysisResult::Succeeded`.

`HttpFraming` reports:

- `kind` as `None`, `ContentLength` or `Chunked`;
- the agreed numeric `contentLength`;
- `contentLengthFieldCount` separately from the number of comma-list values in `contentLengthValueCount`;
- whether every physical length value is one unadorned digit sequence through `contentLengthValuesPlainDecimal`; and
- whether a `Connection` field contains `close`.

The analyzer expects already validated, lowercase-normalized field names and compares them exactly. Equal duplicate or comma-list `Content-Length` values can be valid ordinary HTTP framing, while conflicts, malformed values and `Content-Length` combined with `Transfer-Encoding` are invalid. Anything other than exactly one parameter-free `chunked` transfer coding is reported as `UnsupportedTransferCoding`. Successful framing analysis therefore describes the wire framing; a higher layer can still impose stricter field-count, plain-decimal or transfer-coding policy.

The remaining public helpers avoid reimplementing byte parsing in consumers:

| Helpers | Contract |
| --- | --- |
| `FindHttpField`, `CountHttpFields` | Find the first or count all exact matches for a caller-supplied lowercase normalized name; no case folding is performed. |
| `CreateAsciiHttpField` | Validate an ASCII token name, lowercase it and validate the ASCII field value. Invalid input raises `CHECK_ERROR`. |
| `DecodeAsciiHttpFieldValue`, `HttpFieldValueEqualsAscii` | Decode or compare explicit ASCII bytes without treating a non-ASCII value as text. |
| `TryGetHttpBodySize`, `FlattenHttpBody` | Count or flatten chunk data up to `HttpBodySizeLimit`. Trailers are not included, and a failed output operation leaves its output unchanged. |
| `SetHttpBodyBytes` | Replace chunks and trailers with an empty body or one flat chunk. It checks the body limit but does not reconcile enclosing framing fields. |
| `EncodeStrictUtf8`, `DecodeStrictUtf8` | Convert explicit-length Unicode/UTF-8 strictly, rejecting malformed Unicode and malformed UTF-8 while permitting empty text and embedded NUL. A failed conversion leaves its output unchanged. |
| `ValidateHttpRequestLine` | Validate the ASCII token method, printable-ASCII target and configured request-line size, returning `Succeeded`, `InvalidMethod`, `InvalidRequestTarget` or `TooLong`. |

These helpers deliberately do not decide routes, media types, logical-message validity or whether embedded NUL is acceptable. Those are policies for their consuming layer. Likewise, body-container helpers do not claim that a message is serializable when its headers and body disagree; serialization performs that complete validation.

The parser and serializer support HTTP/1.1 fixed-length and chunked framing and reject ambiguous or unsupported framing. The configured limits are:

| Item | Limit |
| --- | ---: |
| Request/status line | 8 KiB |
| Header block | 64 KiB |
| Body | 16 MiB |
| Chunk-size line | 4 KiB |
| Trailer block | 64 KiB |
| Default/server incomplete-message timeout | 30 seconds |

`HttpRequestConnection` is directional. A client connection calls `SendRequest` and receives `OnReadResponse`; a server connection receives `OnReadRequest` and calls `SendResponse`. One client exchange is active at a time. A server sends exactly one response for each delivered request before processing the next exchange on that connection.

`IHttpRequestCallback::OnReadRequestFailure` reports a request-side HTTP status such as 400, 408, 413, 414, 417, 431, 501 or 505. `OnError` reports connection or framing failure. `OnWriteCompleted` reports completion of a complete serialized message.

When directly constructing lower-layer `HttpRequest` or `HttpResponse` values, header and trailer names must already be lowercase ASCII token names. The serializer rejects uppercase names. This differs from `SocketHttpServerApi`, which folds application response field names to lowercase while normalizing a response.

### Starting `HttpRequestServer` and `HttpRequestClient`

Derive from `HttpRequestServer`, inject `Ptr<IAsyncSocketServer>`, and override `OnClientConnected(IHttpRequestConnection*)`. The override retains and installs one thread-safe `IHttpRequestCallback`, calls `BeginReadingLoopUnsafe`, and accepts or rejects the connection. A most-derived destructor must call `HttpRequestServer::Stop` before destroying callback-visible state.

```C++
auto nativeServer = CreateDefaultAsyncSocketServer(port);
RequestServer server(nativeServer); // Derives from HttpRequestServer.
server.Start();

auto nativeClient = CreateDefaultAsyncSocketClient(port);
HttpRequestClient client(nativeClient);
auto connection = client.GetConnection();
connection->InstallCallback(&clientCallback);
client.WaitForServer();
if (client.GetStatus() == ClientStatus::Connected)
{
    connection->BeginReadingLoopUnsafe();

    auto request = Ptr(new HttpRequest);
    request->method = L"GET";
    request->requestTarget = L"/status";
    connection->SendRequest(request);

    responseReceived.Wait(); // Signaled by OnReadResponse.
}

connection->Stop();
connection->InstallCallback(nullptr);
server.Stop();
```

The lower request layer does not synthesize `Host`, route a target, decode a query, or add application response headers. Applications that want those behaviors should use the Mini HTTP API.

## Mini HTTP Server API

`SocketHttpServerApi`, defined in `Source/InterProcess/AsyncSocket/AsyncSocket_HttpServerApi.h`, owns a URL-prefix registration over a caller-injected `IAsyncSocketServer`. Construction parses and stores the prefix and reads the port from the server. `Start` registers it; the first active API receiving a particular server pointer starts that listener, while later APIs receiving the same pointer join it. The API never creates or replaces the socket server.

### Prefixes and Dispatch

Construct a derived server with `SocketHttpServerApi(socketServer, urlPrefix, respondToOptions)`. `respondToOptions` defaults to `true`.

- The injected server supplies a port from 1 through 65535. The prefix is an empty origin path or begins with `/`; it does not contain a scheme, host or port.
- The path may contain percent-encoded UTF-8. Raw non-ASCII characters and backslashes are rejected; non-ASCII text must use percent-encoded UTF-8. Query and fragment components, NUL, invalid UTF-8, encoded separators and malformed escapes are also rejected.
- Trailing slashes are removed. `GetUrlPrefix` returns the absolute normalized value `http://localhost:PORT{urlPrefix}`.
- A prefix matches its exact path and slash-delimited descendants. For example, `/ABC/def` matches `/ABC/def` and `/ABC/def/item`, but not `/ABC/defghi`.
- Multiple API objects with different prefixes share one listener only when they receive the exact same `Ptr<IAsyncSocketServer>`. Different server objects that report the same port do not join each other.
- Every request is dispatched to the longest matching active prefix, including later requests on a persistent connection.
- A duplicate normalized prefix sharing the same socket server is rejected.

Before routing, the dispatcher requires HTTP/1.1 and exactly one valid `Host` whose name is `localhost` or `127.0.0.1` and whose port matches the injected listener. `SocketHttpClientApi` supplies this field automatically; a manually constructed lower-layer request must supply it. The dispatcher accepts `GET`, `HEAD`, `POST` and `OPTIONS`. Unsupported methods and malformed requests receive automatic responses. When `respondToOptions` is true, supported browser preflight requests are answered before application dispatch.

### Handling a Request

Override `OnHttpRequestReceived(Ptr<SocketHttpRequestContext>)`:

- `GetRequest` returns the exact parsed `async_tcp_socket::HttpRequest`.
- `GetRelativePath` returns the decoded path relative to the selected prefix. An exact prefix match is `/`.
- `GetQuery` returns the raw query without the leading `?`.
- `TryGetBodyUtf8` flattens the complete body and strictly decodes UTF-8. It returns `false` for an oversized or malformed body without changing its output. Empty text and embedded NUL are valid at this layer; the caller owns any application-message policy.
- `Respond` wins at most once. Its optional completion callback receives `true` only after the physical response write completes.
- `RespondStatus` builds an empty response, `RespondBytes` builds a binary response, and `RespondUtf8` strictly encodes a text response. All three delegate to `Respond`, so they retain the same normalization, completion and context-race behavior.
- `Cancel` wins at most once, abandons the response, and closes that physical connection.
- A context can be retained and completed from another thread.

The response conveniences accept status codes from 200 through 599, a printable-ASCII reason and an optional ASCII content type. An empty reason uses the normal default reason during normalization, and an empty content type omits that field. Byte and UTF-8 bodies are limited by `HttpBodySizeLimit`; `RespondUtf8` rejects invalid Unicode. Arguments are validated before the context lifecycle is claimed: invalid input raises `CHECK_ERROR` even for an already consumed context, while a valid call on a consumed context returns `false`. Keep `Respond(Ptr<HttpResponse>)` for custom headers, chunk containers or other binary-oriented construction.

```C++
class StatusApi : public SocketHttpServerApi
{
protected:
    void OnHttpRequestReceived(Ptr<SocketHttpRequestContext> context) override
    {
        if (context->GetRelativePath() != L"/status")
        {
            context->RespondStatus(404, L"Not Found");
            return;
        }

        context->RespondUtf8(
            200,
            L"OK",
            L"application/json; charset=utf-8",
            L"{\"status\":\"ok\"}"
            );
    }

public:
    StatusApi(Ptr<IAsyncSocketServer> socketServer)
        : SocketHttpServerApi(socketServer, L"/api")
    {
    }

    ~StatusApi()
    {
        Stop();
    }
};

auto socketServer = CreateDefaultAsyncSocketServer(8888);
StatusApi api(socketServer);
api.Start();
shutdownRequested.Wait(); // Application/test coordination.
api.Stop();
```

`Respond` normalizes the response. It validates the status, reason, headers and body; rejects response transfer coding and trailers; supplies missing `Date`, `Cache-Control: no-store` and `Access-Control-Allow-Origin: *` policy fields; and validates then rebuilds `Content-Length` framing. It also enforces `HEAD`, 204 and 304 body rules.

Always call `Stop` in the most-derived destructor before destroying fields used by `OnHttpRequestReceived`, response completions or `OnHttpServerStopping`. From outside callbacks, `Stop` unregisters the prefix, cancels its pending contexts and drains callbacks. A callback-reentrant call cannot unwind its current frame, so that frame's visible state must remain alive until it returns. The final active API sharing an injected server also stops that listener.

The portable file-serving example is `Test/UnitTest/MiniHttpServer/Main.cpp`. It demonstrates multiple prefixes on different ports, binary response bodies, content types and explicit start/stop ordering.

## Mini HTTP Client API

`SocketHttpClientApi`, defined in `Source/InterProcess/AsyncSocket/AsyncSocket_HttpClientApi.h`, owns one `HttpRequestClient` and one physical persistent connection. Inject an `IAsyncSocketClient` and the loopback server name; the client supplies its locked-in port. This keeps socket selection and endpoint ownership outside the HTTP API.

```C++
auto nativeClient = CreateDefaultAsyncSocketClient(8888);
auto client = Ptr(new SocketHttpClientApi(nativeClient, L"localhost"));
client->WaitForServer();

vl::EventObject queryCompleted;
queryCompleted.CreateManualUnsignal(false);
if (client->GetStatus() == ClientStatus::Connected)
{
    windows_http::HttpRequest request;
    request.method = L"POST";
    request.query = L"/api/status?detail=full";
    request.contentType = L"text/plain; charset=utf-8";
    request.SetBodyUtf8(L"ping");
    request.receiveTimeout = 30000;

    client->HttpQuery(
        request,
        [&queryCompleted](Variant<windows_http::HttpResponse, windows_http::HttpError> result)
        {
            if (auto error = result.TryGet<windows_http::HttpError>())
            {
                // Transport, framing, validation, timeout or stop failure.
                queryCompleted.Signal();
                return;
            }

            auto&& response = result.Get<windows_http::HttpResponse>();
            WString text;
            if (!response.TryGetBodyUtf8(text))
            {
                // The flat response body is not strict UTF-8.
                queryCompleted.Signal();
                return;
            }
            // Inspect response.statusCode, contentType, cookie and body.
            queryCompleted.Signal();
        }
        );

    queryCompleted.Wait();
}

client->Stop();
```

`WaitForServer` must finish and `GetStatus` must report `ClientStatus::Connected` before normal `HttpQuery` use. Accepted queries run in FIFO order; one response callback may submit the next query reentrantly. A callback can run asynchronously on an arbitrary thread, but request validation, not-ready, stopped or immediate-send failures can also invoke it synchronously before `HttpQuery` returns. Callback state must support both cases and must remain alive for asynchronous delivery.

### Portable Compatibility Values

`SocketHttpClientApi::HttpQuery` uses `windows_http::HttpRequest`, `windows_http::HttpResponse` and `windows_http::HttpError`. Despite the namespace, these value types are declared in the platform-neutral `Source/InterProcess/NetworkProtocolHttp.h`; using them here does not invoke WinHTTP.

These convenient values differ from the lower binary-oriented message types:

- `windows_http::HttpRequest` has method, target query, flat body bytes, content type, accept values, cookie, extra headers and timeout fields.
- `SocketHttpClientApi` combines the constructor server and `client->GetPort()` into the `Host` authority, adds `Accept-Encoding: identity`, then maps the flat request to `async_tcp_socket::HttpRequest`.
- TLS, credentials and `keepAliveOnStop` are rejected. A caller-provided `Host` must match the authority formed from the constructor server and injected-client port, and response compression other than identity is unsupported.
- The injected socket owns connection and send timing. Only `receiveTimeout` controls the response deadline for an accepted HTTP exchange.
- `windows_http::HttpResponse` contains status, a flattened body, the first content type and the first returned cookie.
- `windows_http::HttpResponse::TryGetBodyUtf8` strictly decodes that flat body and distinguishes malformed UTF-8 from a valid empty body without changing its output on failure. `GetBodyUtf8` remains available for compatibility but does not provide this failure signal.
- Status codes other than 404 remain successful `HttpResponse` values. `HttpRequestClient` classifies 404 as the structured fatal failure `HttpResponseFailure::NotFound`, stops its injected socket connection, and `SocketHttpClientApi` reports `HttpError` with `SocketHttpClientErrorCode::ResponseNotFound`.

A 404, transport, framing, unsupported-coding or response-timeout failure makes that `SocketHttpClientApi` terminal and completes its accepted queue with errors. The API never creates or replaces the injected client; create a new API with a fresh native client when a higher layer requires physical reconnection. From outside callbacks, `Stop` cancels current and queued work and drains callbacks. A callback-reentrant `Stop` is supported, but the current callback must return before its captured state can be destroyed.

## Lifecycle Rules Across the Stack

- Install callbacks before starting reads or waiting for client connection events.
- Keep callback objects and captured state alive until the owning connection, client or server has stopped.
- Assume callbacks are concurrent unless a higher layer explicitly serializes them.
- Start servers only after the most-derived object is fully constructed.
- Call `Stop` in a most-derived asynchronous server destructor before destroying callback-visible fields.
- Treat an external `Stop` as the hard drain boundary. A callback-reentrant `Stop` prevents new work but does not unwind the current callback; retain its visible state until it returns.
- Stop client connections before stack-owned callback or channel wrappers leave scope.

## Source and Example Map

- Async socket contracts: [AsyncSocket.h](../../Source/InterProcess/AsyncSocket/AsyncSocket.h)
- HTTP message contracts: [HttpRequest.h](../../Source/InterProcess/AsyncSocket/HttpRequest.h)
- HTTP connection implementation surface: [AsyncSocket_HttpRequest.h](../../Source/InterProcess/AsyncSocket/AsyncSocket_HttpRequest.h)
- HTTP request server wrapper: [AsyncSocket_HttpRequestServer.h](../../Source/InterProcess/AsyncSocket/AsyncSocket_HttpRequestServer.h)
- HTTP request client wrapper: [AsyncSocket_HttpRequestClient.h](../../Source/InterProcess/AsyncSocket/AsyncSocket_HttpRequestClient.h)
- Mini HTTP server API: [AsyncSocket_HttpServerApi.h](../../Source/InterProcess/AsyncSocket/AsyncSocket_HttpServerApi.h)
- Mini HTTP client API: [AsyncSocket_HttpClientApi.h](../../Source/InterProcess/AsyncSocket/AsyncSocket_HttpClientApi.h)
- Portable HTTP compatibility values: [NetworkProtocolHttp.h](../../Source/InterProcess/NetworkProtocolHttp.h)
- Portable Mini HTTP example server: [MiniHttpServer Main.cpp](../../Test/UnitTest/MiniHttpServer/Main.cpp)
