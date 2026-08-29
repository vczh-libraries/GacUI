> This guide intentionally refers to the helpers by C++ symbol instead of by physical source or amalgamation file name. The same API is available as individual test sources in GacUI and as code-packed output, so the storage representation should not affect the instructions below.

# GacUI Remoting Test Helpers

These helpers shorten the application-specific setup needed to try GacUI remote protocol and Workflow remote view models. They are intended for tests, demos, and project bring-up only. They favor fixed roles and fail-fast behavior over recovery and production hardening.

The channel and remote-protocol layers can remain useful in a real application, but the bundled concrete implementations of `vl::inter_process::INetworkProtocolServer`, `vl::inter_process::INetworkProtocolClient`, and `vl::inter_process::INetworkProtocolConnection` are test implementations. Before shipping, provide your own implementation with the authentication, authorization, encryption, peer validation, input limits, deployment policy, reconnection behavior, and delivery guarantees required by your product.

When consuming the unpacked helpers, compile their complete implementation inventory. When consuming the code-packed helpers, compile the provided declaration and implementation pair instead. Choose one representation and do not compile both.

All core and renderer helpers are in `vl::presentation::remoting`. The application-facing remote-view-model server templates are in `vl::presentation::remote_view_model_test`.

# What Is Available

| Helper | Role |
| --- | --- |
| `IsRendererChannel` | Recognizes the exact channel advertisement of a GacUI remote renderer. |
| `RemotingChannelServer<TServerBase>` | Core-side server policy for local core clients, one current renderer, and renderer replacement. |
| `RemoteProtocolRendererClient` | Renderer-side channel client that marshals disconnect and Core-authored fatal errors onto the renderer UI thread. |
| `CreateViewModelReadyMessage` and `IsViewModelReadyMessage` | Create and recognize the post-route Ready signal used by a remote view-model host. |
| `ViewModelHostClient` | Host-process endpoint that exposes generated Workflow RPC services over a network-protocol client. |
| `RemoteViewModelChannelServer<TServerBase>` | Requester-side server that owns the RPC broker, local requester, task queue, host admission, service acquisition, and optional renderer admission. |
| `RemoteViewModelRendererChannelServer<TServerBase, TRvmChannelServer>` | Renderer-only server for the split topology where a separate server owns the remote view-model host. |
| `RpcServerHelpers` | Protected implementation base used by `RemoteViewModelChannelServer`; it is documented for understanding, not for direct application use. |

# Choosing a Bring-Up Transport

The templates accept any server implementing `vl::inter_process::INetworkProtocolServer`, and the clients accept any `vl::inter_process::INetworkProtocolClient`.

| Test transport | Typical use |
| --- | --- |
| `vl::inter_process::named_pipe::NamedPipeServer` and `NamedPipeClient` | Simple Windows-only local processes. |
| `vl::inter_process::windows_http::HttpServer` and `HttpClient` | Windows HTTP.sys/WinHTTP testing. |
| `vl::inter_process::async_tcp_socket::SocketHttpServer` and `SocketHttpClient` | Portable loopback Mini HTTP testing on Windows, Linux, and macOS. |
| `vl::inter_process::stdio_redirection::StdioRedirectionServer` and `StdioRedirectionClient` | A parent requester that launches one remote view-model host as a child and reserves stdin/stdout for framed protocol traffic. |
| Your own `INetworkProtocol*` implementation | The production replacement. The helper and JSON channel layers above it do not need to change. |

# Remote Protocol Core

## `IsRendererChannel`

```C++
bool IsRendererChannel(
    const JsonChannelClient::ChannelNameList& availableChannels
    );
```

Use this predicate in a server admission override when a remote client must be exactly a GacUI renderer. It returns `true` only when the peer advertises one channel and that channel is `GacUIRemoteProtocol`.

The exact check prevents an empty, mixed-purpose, or extra-channel client from being treated as a renderer. `RemotingChannelServer<TServerBase>` calls it automatically, so ordinary users normally do not call it themselves.

Reference: [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) uses the server template that owns this check. The focused channel-server test in [`UnitTest`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/UnitTest) constructs the same exact renderer advertisement.

## `RemotingChannelServer<TServerBase>`

Use this class as the core-side `GuiRemoteProtocolNetworkChannelServer` when a test application needs to accept a renderer and support replacing it without restarting the Core.

Construct it with:

```C++
RemotingChannelServer(
    Ptr<glr::json::Parser> parser,
    bool acceptRenderer,
    /* arguments forwarded to TServerBase */
    );
```

Pass `acceptRenderer == true` for a remote-protocol Core. Pass `false` when the same transport is being used only for remote-view-model traffic.

The normal setup and teardown order is:

1. Construct the server with the JSON parser and transport arguments, then call `Start()`.
2. Construct a `GuiRemoteProtocolLocalChannelClient` with the same parser and connect it with `ConnectLocalClient()`. The local core client is always accepted.
3. Build `GuiRemoteProtocolAsyncJsonChannel`, `GuiRemoteProtocolCoreChannel`, and any filter or DOM-diff layers used by the application.
4. Call `SetCoreChannels(coreJsonChannel, coreProtocolChannel)` after both channel objects exist.
5. Run the Core through `SetupRemoteNativeController()`.
6. Call `ClearCoreChannels()` before the stack-owned channel objects are destroyed.
7. Call `Stop()` on the server.

The application-facing operations are:

- `SetCoreChannels` installs non-owning pointers used to detach and notify an old renderer during replacement.
- `ClearCoreChannels` removes those pointers before their owners leave scope.
- `GetRendererClientId` returns the current renderer client ID, or `-1` when no renderer is current. A `GuiRemoteProtocolCoreChannel` subclass can use it to reject submissions from a stale renderer generation.
- `OnClientConnected` accepts every in-process local client and delegates every remote client to `OnRemoteClientConnected`.
- `OnRemoteClientConnected` is the protected extension point for a specialized admission policy.
- `OnClientDisconnected` clears the current renderer and detaches it from the Core protocol channel.

Internally, the server admits only the exact renderer channel signature. When a new renderer replaces an old one, it first records the new renderer, detaches the old renderer from `GuiRemoteProtocolCoreChannel`, and tries to send the old renderer a raw `ControllerConnectionStopped` message. If that notification cannot be delivered, it disconnects the old transport. A spin lock protects the current renderer and the non-owning Core channel pointers because network callbacks can arrive on other threads.

Reference: [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core), especially its `StartServer`, `StartServerHelper`, and `SwitchableRenderersCoreChannel` types, shows named-pipe, Windows HTTP, Mini HTTP, replacement-renderer validation, and shutdown composition.

# Remote Protocol Renderer

## `RemoteProtocolRendererClient`

This class derives from `GuiRemoteProtocolChannelClient`. It adds the renderer-host policy for Core-authored fatal errors, local transport failure, disconnection, UI-thread ordering, and retained-fatal automation state.

Construct the renderer stack in this order:

```C++
RemoteProtocolRendererClient channelClient(networkClient, parser, fatalDialogTitle);
GuiRemoteProtocolAsyncJsonChannelRenderer asyncChannel(channelClient.GetProtocolChannel());
GuiRemoteRendererSingle renderer(true);
GuiRemoteProtocolRendererChannel protocolChannel(&asyncChannel, &renderer);

channelClient.SetRenderer(&renderer);
channelClient.SetAsyncRendererChannel(&asyncChannel);
channelClient.WaitForServer();
```

After the native controller and renderer automation service exist:

1. Register the native main window with `GuiRemoteRendererSingle`.
2. Call `SetRendererAutomationService` with the active `AutomationServiceRenderer`.
3. Install an `IGuiRemoteProtocolAsyncRendererInvoker` on the async channel and process queued startup messages.
4. Run the native window service if the renderer is connected or `IsFatalErrorRetained()` reports that a Core error was retained for inspection.

Shut down in this order:

1. Clear the automation-service pointer while the service still exists.
2. Clear the async channel's main-thread invoker and unregister the native window.
3. Call `BeginStopping()` to reject new terminal work and detach the async channel.
4. Stop the underlying `INetworkProtocolConnection`.
5. Clear the async-channel and renderer pointers before their owners leave scope.

The setters store non-owning pointers. Their owners must outlive every callback and must be cleared during shutdown.

The terminal mechanism is intentionally simple:

- `OnReadError` means the Core broadcast a fatal `!Error`. The first such error wins and is queued through the async renderer's ordered main-thread FIFO. On platforms with the native prompt policy, choosing to close forces renderer exit; choosing to retain keeps the last rendered UI visible, adds the stopped/fatal overlay, and publishes the error through renderer automation. Platforms without that prompt retain the renderer directly.
- `OnLocalError(..., true)` treats a post-connection transport failure as an ordinary disconnect rather than a Core-authored fatal error.
- `OnDisconnected` calls the base implementation and queues the same idempotent disconnected transition.
- `BeginStopping` prevents a shutdown callback from reopening either terminal path.
- `IsFatalErrorRetained` is a thread-safe query used by the host application when deciding whether the native window loop should remain active.

Reference: [`RemotingTest_Rendering_Win32`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32), especially `StartClient` and `GuiMain`, shows the complete renderer stack on the supported native platforms and all three network transports.

# Remote View Models

Remote-view-model helpers add Workflow JSON RPC on two fixed logical channels:

- `ViewModelChannelName` is the ordinary RPC data channel.
- `ViewModelReadyChannelName` is the control channel used only for the post-route Ready barrier.
- `ViewModelReadyMessage` is the exact `Ready` payload.
- `RemoteViewModelHostDisconnectedError` is injected into the requester when its accepted host disappears outside normal stopping.
- `InvalidRemoteViewModelClientId` is the `-1` sentinel for no accepted host.

The convenience aliases `JsonPackage`, `JsonChannel`, `JsonChannelClient`, `JsonChannelServer`, `JsonLocalChannelClient`, `TaskQueue`, and `RpcDispatcherClient` name the Workflow RPC JSON channel types used by the helpers.

## `CreateViewModelReadyMessage`

This function returns a JSON string package containing the exact Ready payload. `ViewModelHostClient::SendReady()` calls it automatically. Call it directly only when implementing a compatible host client without `ViewModelHostClient`.

The Ready signal must be sent after the host transport route exists and after the host has registered its local generated RPC service. Channel admission happens before an accepted route is committed, so admission alone is too early for the requester-side broker to register the host.

Reference: [`RemotingTest_RvmHost`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_RvmHost) uses `ViewModelHostClient::SendReady`, which creates and broadcasts this package.

## `IsViewModelReadyMessage`

This function returns `true` only when a package is a JSON string whose value is the exact Ready payload. The requester-side broker uses it to validate the control channel. Any other control message is an invariant failure; there are no heartbeat, lease, retry, or shutdown-acknowledgement messages in this test protocol.

Reference: the remote-view-model server used by [`CppTest_Rvm`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/CppTest_Rvm) and [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) consumes this signal internally.

## `ViewModelHostClient`

Use this class in the process that owns and publishes a generated Workflow RPC service.

The required sequence is:

```C++
auto taskQueue = Ptr(new rpc_controller::channeling::TaskQueue);
auto host = Ptr(new remoting::ViewModelHostClient(networkClient, parser, taskQueue));
auto dispatcher = host->GetDispatcher();

host->Connect();
GeneratedRpcInitializer::InitializeRpc(dispatcher, host->GetClientId());

auto lifecycle = dispatcher->GetRpcLifecycle();
auto typeId = lifecycle->GetTypeIdFromName(fullServiceTypeName);
lifecycle->RegisterLocalService(typeId, service);

host->SendReady();
dispatcher->Initialize();
taskQueue->RunTaskQueue();
```

`Connect()` waits for the channel coordinator, installs the RPC data channel, and captures the Ready control channel. The list of services the host waits for is empty because this endpoint provides the service.

`GetDispatcher()` returns the generic `RpcJsonDispatcherClientForTaskQueue` that application-generated RPC initialization must configure. `SendReady()` broadcasts the Ready package and flushes the control channel after the local service is registered.

Disconnection, a remote error, or a fatal local channel error exits the host process immediately with a failure code. This is deliberate test-app behavior: stdin/stdout or another broken transport is not recovered, and exceptions from the task queue also terminate the process.

Reference: [`RemotingTest_RvmHost`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_RvmHost), especially `RunHost`, demonstrates named pipe, Windows HTTP, Mini HTTP, and stdio child modes as well as generated-RPC initialization and service registration.

## `RpcServerHelpers`

`RpcServerHelpers` is a protected implementation base. Applications should use `RemoteViewModelChannelServer<TServerBase>` instead of constructing or exposing this class.

Internally it creates:

- a local broadcasting client on both fixed channels, owning `RpcJsonDispatcherServerForTaskQueue` as the RPC coordinator and reading the Ready control message;
- a local requester client on the RPC data channel, owning `RpcJsonDispatcherClientForTaskQueue` for service access;
- one `TaskQueue` and a worker thread for broker and requester work.

It accepts at most one remote host, but does not register that host with the broker until the exact Ready message arrives. It lazily initializes requester RPC during the first `RequestService`, transitions from `Starting` to `Running` only after service acquisition, and exposes that phase as the renderer-admission gate. If the accepted host disappears, it disconnects the broker route and injects a persistent `RpcInjectedException` into the requester outside server locks. During normal shutdown it finalizes live RPC, stops the transport, exits the task queue, and joins the worker in that order.

References: [`CppTest_Rvm`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/CppTest_Rvm) exercises the hosted requester shape; [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) exercises combined and split renderer/requester shapes.

## `RemoteViewModelChannelServer<TServerBase>`

This is the application-facing requester/server lifetime. It derives from `RemotingChannelServer<TServerBase>` and uses `RpcServerHelpers` internally.

Construct it with a parser, an `acceptRenderer` flag, and the arguments for `TServerBase`. Use `false` for a hosted application that needs only a remote view model. Use `true` when the same server also accepts a GacUI renderer.

The required sequence is:

```C++
server.Start();

collections::List<WString> requiredServices;
requiredServices.Add(fullServiceTypeName);
auto requesterClientId = server.Connect(requiredServices);

// For stdio mode, launch the host child after Connect has prepared admission.
GeneratedRpcInitializer::InitializeRpc(server.GetDispatcher(), requesterClientId);
auto service = server.RequestService(fullServiceTypeName).Cast<IYourService>();

// Run the application and make ordinary calls through service.
server.Stop();
```

The public operations are:

- `Start()` starts the underlying channel server.
- `Connect(requiredServiceNames)` starts the local broker/requester composition, opens admission for one exact two-channel host, and returns the local requester client ID needed by generated RPC initialization. Call it once.
- `GetDispatcher()` returns the non-owning generic dispatcher after `Connect()`.
- `RequestService(typeName)` lazily initializes the requester dispatcher, returns the generated remote wrapper, and opens renderer admission after the first successful acquisition. It can be called repeatedly.
- `CanAdmitRenderer()` reports whether service acquisition moved the requester into its running phase.
- `Stop()` performs the RPC, server, and task-queue shutdown sequence.
- `OnClientDisconnected` preserves the base renderer cleanup and adds remote-host loss handling.

Remote admission accepts exactly one peer advertising both fixed remote-view-model channels. Renderer peers are rejected until the requester is running; when enabled, renderer replacement is inherited from `RemotingChannelServer<TServerBase>`.

Reference: [`CppTest_Rvm`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/CppTest_Rvm), especially its `StartServer` and transport-specific setup functions, is the smallest complete requester example. [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) shows the combined Core, renderer, and remote-view-model topology.

## `RemoteViewModelRendererChannelServer<TServerBase, TRvmChannelServer>`

Use this template when the renderer and remote-view-model host use different transports. A common bring-up case keeps the renderer on named pipe, Windows HTTP, or Mini HTTP while a separate `RemoteViewModelChannelServer<StdioRedirectionServer>` launches the host as a child.

Construct the renderer server with the parser and renderer transport arguments. Call `SetRvmChannelServer(rvmServer)` before starting it. The pointed object is non-owning and must outlive the renderer server callbacks. Clear it with `SetRvmChannelServer(nullptr)` before that object is destroyed.

The renderer server always enables renderer support, but its admission override asks the separate RVM server's `CanAdmitRenderer()` before accepting the exact renderer channel. Once admitted, renderer replacement and Core channel handling are inherited unchanged.

Reference: [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core), especially the `/RVMT` plus `/Cli` branch in `StartServerHelper`, shows the split server pair and child launch. The focused channel-server test in [`UnitTest`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/UnitTest) demonstrates the admission gate and replacement behavior with a minimal fake server.

# End-to-End Remote View-Model Order

The requester and host must preserve this ordering:

1. Requester starts `RemoteViewModelChannelServer`.
2. Requester calls `Connect(requiredServiceNames)`, which prepares the local broker and host admission.
3. The host connects with `ViewModelHostClient`.
4. Both applications initialize their generated RPC objects with their assigned client IDs.
5. The host registers its local service, then calls `SendReady()`.
6. Ready reaches the requester through the control channel after the route exists, so the broker registers the exact accepted host.
7. The host initializes its dispatcher and runs its task queue.
8. The requester calls `RequestService()`. Successful acquisition moves it to the running phase.
9. A combined or split renderer server may now admit the first renderer and later replacements.
10. On normal exit, stop the requester/server before destroying channel owners. A host loss outside normal stopping is fatal and is delivered as an injected requester exception.

# Reference Projects

- [`RemotingTest_Core`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core): remote Core server, renderer replacement, combined remote-view-model transport, and split stdio host transport.
- [`RemotingTest_Rendering_Win32`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32): native renderer client, main-thread invoker, fatal/disconnect policy, automation integration, and renderer shutdown.
- [`CppTest_Rvm`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/CppTest_Rvm): hosted GacUI requester using a remote view model without a remote renderer.
- [`RemotingTest_RvmHost`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_RvmHost): service-owning host client for named pipe, Windows HTTP, Mini HTTP, and stdio modes.
- [`UnitTest`](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/UnitTest): focused split-server admission and renderer-replacement coverage.
