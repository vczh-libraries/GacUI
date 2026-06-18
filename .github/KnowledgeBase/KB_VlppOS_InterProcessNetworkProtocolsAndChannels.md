# Inter-Process Network Protocols and Channels

## Intended Scope and Extension Strategy

The `vl::inter_process` namespace is designed for inter-process communication. Some abstractions are general enough to carry messages inside one process or across a custom transport, but the lifecycle, error handling, connection model and naming are intended for communication between processes.

The built-in `INetworkProtocol*` transports should be treated as reference implementations, validation targets and demo-friendly options. The current concrete `NamedPipeServer` / `NamedPipeClient` and `HttpServer` / `HttpClient` implementations are Windows-only, and they are not meant to be the only production transport choice for every application.

This distinction is especially important for HTTP. `HttpServer` and `HttpClient` are raw `INetworkProtocol*` reference/demo transports, while `HttpServerApi` and `HttpClientApi` are lower-level Windows HTTP helper utilities. When a Windows feature needs HTTP request/response behavior directly, the helper utilities can still be used without adopting the inter-process raw transport as the feature contract.

When the built-in raw protocol implementation does not fit the platform, security model, deployment shape, performance target or reconnection behavior of a feature, implement a custom `INetworkProtocolServer`, `INetworkProtocolClient` and `INetworkProtocolConnection`. The default channel bridge can still be reused as long as the custom raw transport follows the `INetworkProtocol*` contract.

Feature code should usually depend on `IChannelServer<TPackage>`, `IChannelClient<TPackage>` and `IChannel<TPackage>` instead of a concrete raw transport. Keeping the feature boundary at `IChannel*` decouples package delivery from the underlying `INetworkProtocol*`, so the transport can be replaced later without rewriting the feature logic.

The inter-process communication APIs in `vl::inter_process` are layered:

- The raw protocol layer, represented by `INetworkProtocolServer`, `INetworkProtocolClient`, `INetworkProtocolConnection` and `INetworkProtocolCallback`, exchanges asynchronous `WString` messages between a server and clients.
- The channel layer, represented by `IChannelServer<TPackage>`, `IChannelClient<TPackage>`, `IChannel<TPackage>` and `IChannelReader<TPackage>`, builds typed named channels on top of a connected client id model.
- The default bridge, represented by `NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>`, `NetworkProtocolChannelClient<TPackage, TSerialization>` and `NetworkProtocolLocalChannelClient<TPackage, TSerialization>`, serializes channel batches into raw `WString` messages over an `INetworkProtocol*` transport.

The interfaces are transport-agnostic. Use the abstract interfaces in portable feature code, and bind them to concrete transports at application composition boundaries.

## Choosing the Layer

- Use `INetworkProtocolServer`, `INetworkProtocolClient`, `INetworkProtocolConnection` and `INetworkProtocolCallback` when the application only needs raw asynchronous text messages.
- Use `IChannelServer<TPackage>`, `IChannelClient<TPackage>`, `IChannel<TPackage>` and `IChannelReader<TPackage>` when the application needs named logical channels, typed packages, client ids, direct delivery, broadcast delivery or batched writes.
- Use `NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>` and `NetworkProtocolChannelClient<TPackage, TSerialization>` when the channel layer should run over an existing `INetworkProtocolServer` / `INetworkProtocolClient` transport.
- Use `NetworkProtocolLocalChannelClient<TPackage, TSerialization>` when server-side logic needs to participate in a channel as a normal speaker with a real positive client id.
- Use `ChannelSerializer<TSerialization>` when an existing `IChannel<DestType>` should be adapted to another package type by serializing and deserializing packages.
- Choose or implement the concrete `INetworkProtocol*` transport at the application composition boundary; keep feature code on `IChannel*` when transport replacement should remain possible.

## Raw Network Protocol Contract

`INetworkProtocolConnection` represents one text-message connection between two peers.

- `InstallCallback` installs one `INetworkProtocolCallback`; passing `nullptr` uninstalls it.
- `BeginReadingLoopUnsafe` starts asynchronous reading. Some implementations may already be receiving data before this call, so user callbacks must tolerate early delivery after callback installation.
- `SendString` sends one `WString` message to the other side.
- `Stop` closes the connection. Implementations should treat `Stop` as the shutdown boundary and drain asynchronous callbacks before returning.

`INetworkProtocolCallback` receives connection events and must be thread-safe, because callbacks may run on any thread.

- `OnInstalled` gives the callback its `INetworkProtocolConnection`.
- `OnConnected` reports that the connection became available.
- `OnReadString` receives a normal text message.
- `OnReadError` receives an error reported by the remote side.
- `OnLocalError` receives a local transport failure; the `fatal` argument indicates whether the connection should be considered disconnected.
- `OnDisconnected` reports that the connection is lost.

`INetworkProtocolClient` owns one connection to a server.

- `GetConnection` always returns an object, but using it before `WaitForServer` finishes is undefined by contract.
- `WaitForServer` blocks until the connection is established or the implementation gives up.
- `GetStatus` reports `ClientStatus::Ready`, `ClientStatus::WaitingForServer`, `ClientStatus::Connected` or `ClientStatus::Disconnected`.

`INetworkProtocolServer` owns the listening side.

- `Start` begins accepting clients.
- `OnClientConnected` is called for each accepted transport connection and returns `WaitForClientResult::Accept` or `WaitForClientResult::Reject`.
- `Stop` ends listening and disconnects owned connections.
- `IsStopped` reports either explicit shutdown or an underlying transport failure.
- No client-connected callback should happen before `Start` or after `Stop`.

The normal raw-protocol usage pattern is:

1. Derive a server from a concrete `INetworkProtocolServer` implementation and override `OnClientConnected`.
2. In `OnClientConnected`, call `InstallCallback` on the new `INetworkProtocolConnection`, then call `BeginReadingLoopUnsafe`, and return `WaitForClientResult::Accept`.
3. Create each client, call `GetConnection()->InstallCallback`, call `WaitForServer`, then call `GetConnection()->BeginReadingLoopUnsafe`.
4. Exchange messages through `INetworkProtocolConnection::SendString`.
5. Call `Stop` and uninstall callbacks during shutdown.

## Channel API Contract

There is no separate `IChannelConnection` type. The channel layer models a connected participant with `IChannelClient<TPackage>`, its assigned positive client id, and its named `IChannel<TPackage>` objects.

`IChannelReader<TPackage>` receives typed channel packages.

- `OnRead` receives a positive `senderClientId` and one `TPackage`.

`IChannel<TPackage>` represents one named logical channel.

- `GetChannelName` returns the channel name.
- `GetReader` returns the installed reader.
- `Initialize` installs one reader and replays unread messages that arrived before the reader was installed.
- `Initialize` can only install a reader once; no reader uninstallation is supported.
- `SendToClient` queues a direct message to one receiver client id.
- `BroadcastFromClient` queues a broadcast to all other participants on the same channel.
- The `BroadcastFromClient` overload with `blockedReceivers` excludes specific receiver ids from a broadcast.
- `BatchWrite` flushes all queued messages and reports disconnection through its `disconnected` output argument.

Channel names:

- User channel names must be non-empty.
- User channel names must not contain `!`.
- Names beginning with `!` are reserved for system channels.
- `ErrorChannel` is used by the default implementation for fatal channel errors.
- `SystemChannel` is declared as reserved, but the default implementation currently does not use it.

`IChannelClient<TPackage>` represents one channel participant.

- `OnGetChannelNames` tells the implementation which channel names this client supports.
- `GetChannels` returns implementation-created channel objects. The channel map is expected to be empty before connection and populated by the implementation.
- `GetClientId` returns `-1` before connection and the server-assigned positive id after connection.
- `WaitForServer` blocks until connected for network clients. Local clients return immediately.
- `GetStatus` reports `Ready`, `WaitingForServer`, `Connected` or `Disconnected`.
- `OnConnected` and `OnDisconnected` report participant lifecycle.
- `OnReadError` receives fatal errors broadcast by the channel server.
- `OnLocalError` receives local transport errors.
- `BroadcastError` raises a fatal channel error.

`IChannelServer<TPackage>` manages connected channel participants.

- `Start` begins accepting channel clients.
- `OnClientConnected` receives the assigned client id, the client's channel names and a `localClient` pointer.
- The `localClient` argument is non-null only when the connection is created by `ConnectLocalClient`.
- `ConnectLocalClient` connects an in-process server-side participant and returns its assigned positive client id, or `-1` if it cannot connect.
- `IsLocalClient` reports whether a client id belongs to a local client.
- `DisconnectClient` disconnects either a network client or a local client.
- `GetClientIds` returns all known client ids.
- `GetClientChannels` returns the client-id to channel-name membership map.
- `BroadcastError` broadcasts a fatal channel error and stops the server.
- `Stop` disconnects clients and stops accepting.
- `IsStopped` reports explicit stop or underlying transport stop.

`IChannelServer<TPackage>` is a delivery and bookkeeping API. It is not itself a channel speaker. When server-side behavior needs to send channel messages, create a `NetworkProtocolLocalChannelClient<TPackage, TSerialization>`, connect it with `ConnectLocalClient`, and send messages through that local client's channels.

## Default Channel Implementation Over Raw Protocols

The default channel bridge serializes batches of typed packages to raw `WString` messages.

`TSerialization` for `NetworkProtocolChannel<TPackage, TSerialization>` must satisfy:

- `TSerialization::SourceType` is `collections::List<TPackage>`.
- `TSerialization::DestType` is `WString`.
- `TSerialization::ContextType` stores optional serializer context.
- `TSerialization::Serialize` converts a package list to `WString`.
- `TSerialization::Deserialize` converts a `WString` back to a package list.

`NetworkPackage` is the raw wire envelope.

- Its string format is `clientId,extraClientId1,...;channelName;messageBody`.
- `clientId` may be null.
- `extraClientIds` may be null.
- A null `clientId` with extra ids is preserved as a leading comma, for example `,1,2;Chat;Message`.
- Missing or empty `extraClientIds` normalize to null after parsing.

Connection handshake:

1. A channel client waits for the raw `INetworkProtocolClient`.
2. The client sends a `NetworkPackage` with null `clientId`, empty `channelName`, and `messageBody` equal to all supported channel names joined by `!`.
3. The server validates the first package as a connection request.
4. The server allocates a positive client id.
5. The server calls `OnClientConnected(clientId, channelNames, nullptr)` for network clients.
6. The server records channel membership.
7. The server sends a response package with the assigned `clientId`, empty `channelName`, and empty `messageBody`.
8. The client records the assigned id, changes status to connected, replays any queued channel packages, and calls `OnConnected`.

Channel message semantics:

- Client-to-server direct messages set `NetworkPackage::clientId` to the receiver client id.
- Client-to-server broadcasts use a null `clientId`.
- For broadcasts, `extraClientIds` means blocked receivers.
- For direct messages, `extraClientIds` are ignored by the delivery semantics.
- Server-to-client channel messages always set `NetworkPackage::clientId` to the positive sender client id.
- A received channel message without a positive sender id is invalid.

`NetworkProtocolChannel<TPackage, TSerialization>` handles per-channel queues.

- `ReadBatch` stores unread packages until `Initialize` installs a reader.
- `Initialize` replays unread packages to the reader.
- `SendToClient` and `BroadcastFromClient` validate ids and queue packages.
- `BatchWrite` moves queued packages out, groups them by receiver id and blocked-receiver list, serializes each group, and calls `WriteBatch`.
- If `WriteBatch` reports disconnection, `BatchWrite` sets `disconnected` and stops flushing.

`NetworkProtocolChannelClientBase<TPackage, TSerialization>` owns the shared client-side channel state.

- It owns serializer context, client id, client status, connection notification state and generated channels.
- `GetChannels` lazily creates channel objects from `OnGetChannelNames`.
- The generated channel validates that a client is connected before sending.
- Direct sends require a positive receiver id.
- Direct sends cannot also specify blocked receivers.
- Blocked receiver ids must be positive and must not be the sender's own id.

`NetworkProtocolChannelClient<TPackage, TSerialization>` adapts a real `INetworkProtocolClient`.

- The constructor installs an `INetworkProtocolCallback` on the raw connection.
- `WaitForServer` waits for the raw transport, sends the channel-name handshake, starts raw reading, waits for the assigned client id, and notifies `OnConnected`.
- Incoming `ErrorChannel` packages call `OnReadError` and disconnect.
- Incoming empty-channel packages are treated as connection responses.
- Incoming channel packages that arrive before the assigned id are queued and replayed after connection.
- `SendBatch` serializes one package list into one `NetworkPackage` and calls `SendString`.
- `BroadcastError` sends an `ErrorChannel` package and disconnects locally.
- The destructor stops the raw connection if still connected and uninstalls the callback.

`NetworkProtocolLocalChannelClient<TPackage, TSerialization>` adapts no transport.

- It connects only through `NetworkProtocolChannelServer::ConnectLocalClient`.
- `WaitForServer` is a no-op.
- `SendBatch` calls the server's local-client send hook.
- `BroadcastError` routes through the server when connected.
- If already disconnected, `BroadcastError` reports a fatal local error and disconnects locally.

`NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>` combines the channel server with a raw protocol server.

- `TServerBase` is a concrete `INetworkProtocolServer` implementation, such as `NamedPipeServer` or `HttpServer`.
- The class inherits `TServerBase`, `IChannelServer<TPackage>` and the private local-client server interface.
- `Start` records the channel server as started, then calls `TServerBase::Start`.
- Raw `OnClientConnected(INetworkProtocolConnection*)` rejects connections before start or after stop, creates a pending connection context, installs a raw callback, starts raw reading, and waits for the channel handshake.
- Before a connection has a client id, `OnReadString` requires the channel-name handshake.
- After a connection has a client id, `OnReadString` validates sender membership, receiver membership or blocked-receiver membership, deserializes the batch and forwards it through `SendBatch`.
- `SendBatch` handles direct delivery and broadcast delivery for both network clients and local clients.
- Network recipients receive serialized `NetworkPackage` strings.
- Local recipients receive packages through their local channel objects.
- `ConnectLocalClient` requires a started, non-stopped server and a `NetworkProtocolLocalChannelClient<TPackage, TSerialization>`.
- `ConnectLocalClient` validates local channels, allocates a positive id, calls `OnClientConnected(clientId, channels, localClient)`, records membership and notifies the local client.
- `DisconnectClient` removes either a network client or a local client, stops or notifies it, and calls `OnClientDisconnected`.
- `Stop` clears network, pending and local client state, notifies local clients, delegates transport shutdown to `TServerBase::Stop`, and reports network disconnections.
- `BroadcastError` sends `ErrorChannel` packages to network clients, calls `OnReadError` on local clients, gives transport clients a short chance to consume the fatal package, and stops the server.

## Channel Usage Pattern

To build a typed channel application over a raw transport:

1. Define a serializer whose `SourceType` is `collections::List<TPackage>` and whose `DestType` is `WString`.
2. Define a channel server by using `NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>`, where `TServerBase` is the chosen raw transport server.
3. Override `OnClientConnected(vint, const ChannelNameList&, IChannelClient<TPackage>*)` to accept or reject clients after checking their channel names.
4. Define channel clients by deriving from `NetworkProtocolChannelClient<TPackage, TSerialization>` for network clients.
5. For each client, return supported channel names from `OnGetChannelNames`.
6. Obtain channels through `GetChannels`, call `Initialize` with an `IChannelReader<TPackage>`, then call `WaitForServer`.
7. Queue channel messages with `SendToClient` or `BroadcastFromClient`.
8. Call `BatchWrite` to flush queued channel messages.
9. If the server needs to speak on a channel, derive that participant from `NetworkProtocolLocalChannelClient<TPackage, TSerialization>` and connect it with `ConnectLocalClient`.

## Error and Shutdown Rules

- Keep remote channel errors and local transport errors separate.
- `IChannelClient<TPackage>::OnReadError` is for fatal errors broadcast through the channel server.
- `IChannelClient<TPackage>::OnLocalError` is for local raw-transport failures, such as connection, request or response failures.
- `INetworkProtocolCallback::OnReadError` is for remote errors at the raw transport contract level.
- `INetworkProtocolCallback::OnLocalError` is for local transport failures at the raw transport contract level.
- A fatal local error should lead to disconnection.
- `Stop` should be treated as a hard shutdown boundary: after it returns, transport callbacks should no longer touch the stopped object.
- `BatchWrite` may report disconnection; callers should stop assuming queued messages were delivered once `disconnected` becomes true.
- Server-originated normal channel messages should use a connected local channel client so all normal messages have a real positive sender id.

## Current Windows-Only Raw Transports

The built-in `NamedPipeServer` / `NamedPipeClient` and `HttpServer` / `HttpClient` classes are currently Windows-only implementations of the raw protocol interfaces. They should not be treated as cross-platform transport classes.

### `NamedPipeServer` and `NamedPipeClient`

`NamedPipeConnection` implements `INetworkProtocolConnection`. `NamedPipeClient` derives from `NamedPipeConnection` and implements `INetworkProtocolClient`. `NamedPipeServer` implements `INetworkProtocolServer`.

- `NamedPipeServer::Start` begins overlapped named-pipe accepting.
- `NamedPipeServer` keeps both accepted `NamedPipeConnection` objects and pending `ConnectNamedPipe` operations.
- `NamedPipeServer::OnClientConnected` is the user-overridable accept hook.
- `NamedPipeClient::WaitForServer` completes the client-side setup, switches the pipe to message-read mode, updates status to `ClientStatus::Connected`, and calls callback `OnConnected`.
- `NamedPipeConnection::SendString` frames one `WString` with byte count and string length data.
- `NamedPipeConnection` chunks writes by `MaxMessageSize`, because the named-pipe implementation does not support one message larger than 64K.
- `NamedPipeConnection::BeginReadingLoopUnsafe` accumulates overlapped read chunks until one complete framed message is available, then calls `OnReadString`.
- Broken-pipe cases become `OnDisconnected`; invalid handle or aborted I/O cases become fatal `OnLocalError` followed by disconnect.
- `NamedPipeConnection::Stop` cancels pending overlapped pipe I/O, unregisters pending waits, waits for pending callbacks, and closes handles.
- `NamedPipeServer::Stop` drains both pending accepts and accepted connections.

### `HttpServer` and `HttpClient`

`HttpClient` implements both `INetworkProtocolConnection` and `INetworkProtocolClient`. `HttpServer` derives from `HttpServerApi` and implements `INetworkProtocolServer`. This is also a Windows-only reference/demo implementation.

The HTTP protocol uses three routes:

- `GET /VlppInterProcess/Connect` creates a logical connection and returns a pair of per-connection URLs.
- `POST /VlppInterProcess/Request/GUID` is the client-maintained long-poll request for server-to-client messages.
- `POST /VlppInterProcess/Response/GUID` sends client-to-server messages and may also receive one server-to-client message in the response.

`HttpClient` behavior:

- `HttpClient::WaitForServer` sends `/Connect`, waits for the response, validates the `requestUrl;responseUrl` body, stores both URLs, changes status to connected, and calls `OnConnected`.
- `HttpClient::BeginReadingLoopUnsafe` starts the long-poll `/Request` loop.
- After each successful `/Request`, `HttpClient` starts the next `/Request` before delivering a non-empty response body to `OnReadString`.
- `HttpClient::SendString` sends a `/Response` request with the string body.
- `/Connect` and `/Response` failures retry up to `HttpRequestMaxAttempts`; the last failure is fatal.
- `/Request` failures are retried while the client is still running.
- `HttpClient::Stop` stops the underlying `HttpClientApi`, signals any waiting `WaitForServer`, and reports `OnDisconnected`.

`HttpServer` and `HttpServerConnection` behavior:

- On `/Connect`, `HttpServer` creates a `HttpServerConnection`, assigns a GUID, calls `OnClientConnected`, and returns the per-connection request and response URLs, or rejects with an HTTP error response.
- `HttpServerConnection::BeginReadingLoopUnsafe` is a no-op because `HttpServerApi` owns the receive loop.
- `HttpServerConnection::InstallCallback` stores the callback and replays queued inbound strings after releasing its queue lock.
- `HttpServerConnection::SendString` responds to a pending `/Request` immediately when possible; otherwise it queues outbound messages.
- If a server message is produced while handling `/Response`, `HttpServerConnection` may return one outbound message in that `/Response` response and queue the rest for future `/Request` calls.
- `HttpServerConnection::OnNewHttpRequestForPendingRequest` cancels an old pending `/Request`, stores the new one, and replies immediately if a message is already queued.
- `HttpServerConnection::SubmitResponse` reads the client body, dispatches or queues it as inbound text, then returns one queued outbound message if available.
- `HttpServer::OnHttpServerStopping` clears connections, cancels pending long-poll requests, disconnects connection objects from the server, and calls `OnDisconnected`.

## Windows HTTP Helper Layer

`HttpClientApi` and `HttpServerApi` are reusable Windows helper classes used by `HttpClient` and `HttpServer`. Use `HttpClient` and `HttpServer` only when the reference/demo raw `INetworkProtocol*` transport is the desired shape; use the helper APIs directly when a Windows-specific feature needs lower-level HTTP request/response behavior.

`HttpClientApi` owns one WinHTTP session and connection for one host and port.

- `HttpClientApi::HttpQuery` sends one asynchronous HTTP request.
- `HttpRequest` describes method, query, body, content type, accept types, credentials, cookies, headers, timeouts and `keepAliveOnStop`.
- `HttpRequest::SetBodyUtf8` converts a `WString` request body to UTF-8 bytes.
- `HttpResponse` carries HTTP status code, response body bytes, cookie and content type.
- `HttpResponse::GetBodyUtf8` converts a UTF-8 response body back to `WString`.
- `HttpError` represents Windows or WinHTTP transport failures.
- HTTP status codes such as 404 are represented as successful `HttpResponse` values, not `HttpError`.
- `HttpClientApi::Stop` marks the helper as stopping, closes active non-keep-alive requests, waits for request handle-closing callbacks, then closes WinHTTP handles.

`HttpServerApi` owns one HTTP.sys URL prefix.

- `HttpServerApi::Start` begins the receive loop.
- `HttpServerApi::Stop` unregisters the pending receive wait, waits for callbacks, calls `OnHttpServerStopping`, and closes HTTP.sys handles.
- `OnHttpRequestReceived` is the virtual request-dispatch hook.
- `OnHttpServerStopping` is the virtual shutdown hook.
- `GetUtf8Body` validates `application/json; charset=utf8`, reads the complete request body and converts it to `WString`.
- `SendResponse` sends a structured `HttpServerResponse`.
- `SendResponseUtf8` sends a UTF-8 body with JSON content type.
- Optional `OPTIONS` handling exists for cross-origin HTTP clients.
