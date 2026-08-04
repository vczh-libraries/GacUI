# Remoting requester analysis

## Short answers

1. `vl::presentation::remoting::RemotingRequesterSession` owns the requester-side runtime for acquiring `rvmt::IViewModel` from `RemotingTest_RvmHost`. `RemoteViewModelChannelServer` is its only owner/creator, but it is not its only caller: `CppTest_Rvm` and `RemotingTest_Core` obtain it through `GetSession()` and drive its startup, service acquisition, running phase, and shutdown.
2. `RequesterLocalClient` is correctly a local channel client even though it lives in the server process. At the channel layer, the server only routes packages; code that sends or receives ordinary channel packages must participate as an `IChannelClient` and have a positive sender ID. “Local” describes an in-process connection to the channel server, not the client/server role of the containing process.
3. The two local clients are necessary, but the public `RemotingRequesterSession` orchestration is twisted in much the same way as the callback wiring removed by `TODO_Task.md`. The right inheritance boundary is `RemoteViewModelChannelServer : RemotingChannelServer`, not `RemotingRequesterSession : RequesterLocalClient`. The RVM server specialization should directly own the requester runtime and override server lifecycle/admission hooks.

# What `RemotingRequesterSession` does

The implementation is in `Test/RemotingHelpers/RemotingClient/ViewModelHostClient.cpp`, with its public API in `ViewModelHostClient.h`. It combines four responsibilities:

- It creates a `TaskQueue` and a dedicated `TaskQueueThread` for Workflow RPC dispatch.
- It connects `BroadcastingLocalClient` to the channel server. This participant owns `RpcJsonDispatcherServerForTaskQueue`, acts as the RPC broker on `ViewModelChannel`, and reads the `Ready` message on `ViewModelReadyChannel`.
- It connects `RequesterLocalClient` as a second local participant. This participant owns `RemotingJsonDispatcherClient`, initializes the generated RPC types, waits for `rvmt::IViewModel`, and produces the proxy returned by `RequestService()`.
- It implements the RVM-host state machine: admit exactly one matching remote host, register it with the broker only after its `Ready` message, prevent renderer admission until the UI enters the running phase, treat an unexpected host disconnection as terminal, and order RPC/server/task-queue shutdown.

The actual use sites are split across layers:

- `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModelTestRuntime.h` is the only place that constructs and owns a `RemotingRequesterSession`. `RemoteViewModelChannelServer` delegates `TryAcceptHost`, `CanAdmitRenderer`, and `OnClientDisconnected` to it.
- `Test/GacUISrc/CppTest_Rvm/GuiMain.cpp` calls `Start`, `RequestService`, `BeginRunning`, `BeginStopping`, and `Stop` through the pointer returned by `RemoteViewModelChannelServer::GetSession()`.
- `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp` calls the same lifecycle methods for `/RVMT`; its other modes receive a null session.

Therefore, “only `RemoteViewModelChannelServer` uses it” is true only for ownership. The test apps still know about and operate the session directly.

# Why `RequesterLocalClient` is a client on the server side

The repository knowledge base in `.github/KnowledgeBase/manual/vlppos/using-inter-process.md` states that an `IChannelServer<TPackage>` manages routing and bookkeeping but is not itself a channel speaker. Server-side feature logic must connect a `NetworkProtocolLocalChannelClient` when it needs to send or receive normal channel messages. This gives every message a real positive sender client ID.

That is exactly what happens here:

- `RemotingTest_RvmHost` is a remote/network channel client that hosts `rvmt::IViewModel`.
- `BroadcastingLocalClient` is an in-process channel client in the requester/server process that brokers RPC traffic.
- `RequesterLocalClient` is another in-process channel client in the same process that requests and calls the remote service.

The broker and requester cannot simply be one local client. Both need a reader on `ViewModelChannel`, while `IChannel::Initialize` permits exactly one reader and no reader replacement. They also need distinct client IDs so `RpcJsonDispatcherServer` can register the requester and remote host as separate RPC participants and route declarations, requests, and responses between them.

The file placement is more confusing than the abstraction. The old `RemotingClient.h/.cpp` was renamed to `ViewModelHostClient.h/.cpp` by the current HEAD, and `RequesterLocalClient` is private to the anonymous namespace in the `.cpp`; it is not declared in the header. Historically, the file grouped channel-client implementations, including local clients. With the specialized name `ViewModelHostClient`, keeping the requester-server implementation in the same file is now misleading and should be corrected as part of any refactor.

# Relationship to the `TODO_Task.md` refactoring

`TODO_Task.md` identified callbacks that simulated ordinary virtual dispatch. The completed refactoring correctly made `RemoteViewModelChannelServer` inherit `RemotingChannelServer` and override `OnRemoteClientConnected` and `OnClientDisconnected`.

The remaining session relationship still bounces control across an artificial boundary:

- The server owns the session.
- The server callbacks delegate admission and disconnection back to the session.
- The app asks the server for the session.
- The app starts the session by passing its owning server back into `RemotingRequesterSession::Start`.
- The app stops the session by passing a callback that calls the owning server's `Stop`.

This is the same general design smell: one logical RVM requester server is represented by two public objects that repeatedly call back into each other. `RemotingRequesterSession::Stop(const Func<void()>& stopServer)` is the clearest symptom because a child owned by the server receives a callback solely to stop that server at the required point in the child's shutdown sequence.

There is also a concrete problem caused by the optional-session design. `RemotingTest_Core` constructs `RemoteViewModelChannelServer` with `_acceptViewModel == false` for non-RVM modes, leaving `session` null, but `RemoteViewModelChannelServer::OnClientDisconnected` unconditionally calls `session->OnClientDisconnected(clientId)`. Any disconnection reaching this override in those modes can dereference a null pointer. Selecting a plain `RemotingChannelServer` for non-RVM modes removes this invalid state instead of adding another runtime null check.

# Recommended shape

Use inheritance and overriding at the server-specialization boundary:

- Use `RemotingChannelServer<TServerBase>` directly for Core modes that do not request a remote view model.
- Use `RemoteViewModelChannelServer<TServerBase> : RemotingChannelServer<TServerBase>` only when an RVM requester is required.
- Let `RemoteViewModelChannelServer` directly override `Start`, `Stop`, `OnRemoteClientConnected`, and `OnClientDisconnected` as needed. Keep `acceptRenderer` as construction policy; do not add callbacks for fixed state checks.
- Move the phase, host ID, RPC broker/requester objects, task queue/thread, and acquired service under that server specialization, either as direct private fields or as a private implementation detail.
- Expose only the RVM operations the app genuinely needs, such as requesting the typed view model and marking the GUI running/stopping phases. Remove `GetSession()`, the separate public `RemotingRequesterSession`, and the `stopServer` callback.
- Keep two private local-client classes. Renaming them to `RpcBrokerLocalClient` and `RpcRequesterLocalClient` would make their roles clearer. Do not merge them or turn the session/server into one local client, because their separate readers and client IDs are part of the RPC topology.

In short, the local-client implementation is structurally correct. The avoidable complexity is the public session facade and its circular lifecycle wiring with `RemoteViewModelChannelServer`.
