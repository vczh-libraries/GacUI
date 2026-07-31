# Refine Remote View-Model RPC/Channel Integration

This document is a static review of:

- `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModel*.(h|cpp)`
- `Test/GacUISrc/RemotingTest_RvmHost/Main.cpp`
- their callers in `CppTest_Rvm` and `RemotingTest_Core`
- the required behavior in `DebugRemoteProtocolSop.md`

No application was built or run while preparing this review.

## Decision

Apply the test-application fail-fast policy:

- Unexpected synchronous exceptions are not recovered or translated into
  application state. Let them terminate the process.
- `RemotingTest_RvmHost` has one normal terminal event: receive a stop signal
  from its requester (`CppTest_Rvm` or `RemotingTest_Core`) and exit.
- An asynchronous transport failure is also terminal. It must terminate the
  host directly instead of starting a recovery or graceful-unwind protocol.

With this policy, the host side can be reduced substantially. It does not need
a requester lease, acknowledgement protocol, general terminal-state machine,
failure callback, cleanup-error aggregation, or retrying task-queue worker.

There is one important qualification: the host cannot be literally
receive-only. To preserve the SOP requirement that an abruptly killed host is
detected under `/Pipe`, `/Http`, and `/MiniHttp`, it still needs to send a
minimal one-way liveness message to the requester. The requester owns the
timeout and the fatal reaction. No acknowledgement or reverse lease is needed.

## Summary

The expectation that connecting generated `RpcJson*` objects to an
`IChannelClient` should take only a few application lines is reasonable.
However, only part of the current 1,920-line implementation is that connection:

| File | Lines | Main responsibility |
| --- | ---: | --- |
| `RemoteViewModelTestShared.h` | 31 | Names, messages, timing, endpoints |
| `RemoteViewModelRoleState.h` | 382 | Role, lease, phase, and terminal state |
| `RemoteViewModelTestRuntime.h` | 281 | All public runtime helper types |
| `RemoteViewModelTestRuntime.cpp` | 1,226 | RPC binding, three local clients, two-way liveness, failure and shutdown handling |

The generated-module binding in
`RemoteViewModelJsonDispatcherClient::InitializeRpc` is about 30 lines. The
basic channel adapters are also small. Most of the remaining code implements
symmetrical process supervision, acknowledgements, defensive cleanup, and
duplicated state.

The latest lifetime refinement added about 1,200 lines to these four files.
It fixed real lifetime problems, especially abrupt host loss over HTTP, but it
implemented a much stronger and more recoverable protocol than the written SOP
or the clarified host contract requires.

## Behavior That Must Remain

| Required outcome | Owner | Minimum mechanism |
| --- | --- | --- |
| The requester starts first and blocks until `rvmt::IViewModel` is available. | `RemoteViewModelRequesterSession` | RPC broker, requester RPC endpoint, and service wait |
| Exactly one host is accepted and it cannot be replaced during the process run. | Requester/Core | Accepted-host ID plus an `everAccepted` fact |
| Broker login is not scheduled before the remote channel route is usable. | Requester and host | A post-admission signal such as the current `Ready`, or the first heartbeat |
| Killing the host is fatal to `CppTest_Rvm` and Core before normal application stopping. | Requester/Core | Physical disconnect callback plus requester-side timeout for silent HTTP clients |
| Core sends its fatal `!Error` to an active renderer before terminating. | Core | The existing Core-owned one-shot fatal action; never host recovery |
| Renderer loss and renderer replacement remain nonfatal. | Core | Renderer policy stays outside the RVM runtime |
| Normal application stopping is not reclassified as host failure. | Requester/Core | Set `stopping` before sending the host stop signal or stopping transports |
| Normal requester/Core stopping causes the host to exit. | Host | One one-way stop message followed by immediate normal process exit |

The exact heartbeat text, acknowledgement, sequence number, terminal delivery
wait, and host exit code are not SOP behavior. They are implementation policy.

## What Is Intrinsically Necessary

### Two local RPC participants

The requester-side channel server still needs two distinct local clients:

1. A broker local client containing `RpcJsonDispatcherServer`.
2. A requester RPC endpoint containing its own `RpcJsonDispatcherClient` and
   lifecycle.

`IChannelServer` routes packages but is not itself a speaking channel client.
The broker therefore needs a positive local client ID. The requester endpoint
needs a different client ID because RPC object ownership, login, and service
discovery are endpoint-specific. Also, one `IChannel` has one reader, so these
roles cannot simply share the same `ViewModelChannel` reader.

This is the minimum topology under the current Workflow/VlppOS APIs:

```text
Requester process
  JsonChannelServer
    broker + control local client
      - ViewModelChannel: RpcJsonDispatcherServer
      - ViewModelReadyChannel: Ready/Heartbeat reader and Stop sender
    requester local RPC client
      - ViewModelChannel: RpcJsonDispatcherClient

Remote host process
  network RPC client
    - ViewModelChannel: RpcJsonDispatcherClient hosting rvmt::IViewModel
    - ViewModelReadyChannel: Ready/Heartbeat sender and Stop reader
```

The broker and requester clients should not be merged without redesigning
Workflow RPC server semantics.

### A post-admission barrier

The current `Ready` message is not merely ceremony.

The channel server calls its application admission callback before committing
the new client and its channel routes. In contrast,
`RpcJsonDispatcherServer::RegisterClient` immediately schedules the broker
login. Registering the host directly inside the existing
`OnClientConnected` callback can therefore run login before
`ClientHasChannel` becomes true and can fail or hang startup.

Receiving `Ready` or a first heartbeat proves that the host completed the
handshake and that the route can carry packages. Preserve one such barrier
until VlppOS provides a true post-commit callback.

### One-way host liveness

Named pipes normally reveal a killed host through physical disconnection.
`/Http` and `/MiniHttp` can retain a logical server-side client token after the
host process disappears. The requester therefore needs a bounded host lease.

Only this direction is required:

```text
host -- Heartbeat --> requester lease
```

The reverse `HeartbeatAck` and host-side requester lease do not help the
requester detect a dead host. The host already has an active client transport;
requester loss is reported through its channel callbacks. Normal requester
shutdown has an explicit stop message.

### A scheduling decision

The host still needs somewhere to execute incoming `Translate` calls. The
existing main-thread `TaskQueue::RunTaskQueue` is small and conventional. The
requester currently needs its RPC/broker work to continue while startup is
blocked in service acquisition, which explains its separate queue thread.

The error recovery around those queues is not required. The scheduling itself
is a separate concern and should not be deleted accidentally.

For the current request-only `rvmt::IViewModel` surface, an inline
`ScheduleTask` implementation could remove the requester task queue and its
thread. Treat that as an optional optimization only if this test interface is
declared not to gain events, callbacks, nested RPC, or requester-owned objects.
Those features can deadlock or introduce reentrancy if scheduling becomes
inline.

## Findings

### 1. There is no existing simple adapter to reuse

There is no hidden `RpcJson`/`IChannelClient` composition helper in GacUI,
Workflow, or VlppOS. The canonical Workflow ChatBot example duplicates the
same shape:

- `../Workflow/Test/UnitTest/ChatBotServer/Shared/ChatBotJsonDispatcherClient.*`
- `../Workflow/Test/UnitTest/ChatBotServer/Main.cpp`
- `../Workflow/Test/UnitTest/ChatBotClient/Main.cpp`

`RemoteViewModelJsonDispatcherClient` repeats the ChatBot lifecycle binding:
generated type IDs, serializer, object operations, event operations, listener,
and wrapper factory are manually assigned in application code.

This boilerplate is an upstream Workflow/generator abstraction gap. Moving it
to another GacUI helper without changing the Workflow API would only hide it.

### 2. The third local client is unnecessary

`RemoteViewModelReadyLocalClient` can be folded into
`RemoteViewModelBroadcastingLocalClient`.

The broker local client can advertise both logical channels:

- its RPC dispatcher remains the reader of `ViewModelChannel`;
- the broker client itself reads `ViewModelReadyChannel`;
- the same control channel sends the one-way stop message.

The remote host already demonstrates that one `IChannelClient` can own
separate RPC and control lanes. This removes one class, one channel map, one
local client ID, one connect path, `SendToClient`, and associated
`dynamic_cast` admission branches.

Keep the control lane separate from `ViewModelChannel`; combining readers
would require a new multiplexer and would route control traffic into RpcJson.

### 3. Host-side process supervision is outside the clarified contract

The following `RemoteViewModelHostingClient` state exists to supervise the
requester or negotiate terminal delivery:

- `RemoteViewModelTerminalState terminalState`
- `RemoteViewModelLeaseState requesterLease`
- `requesterReadyClientId`
- `heartbeatSequence`
- `ReportFailure`
- terminal selection in `ProcessState`
- `BeginStopping`
- the message/bool `terminalAction`

None is needed when the host has only two outcomes:

- exact requester stop message: exit normally;
- anything unexpected: terminate abnormally.

The host heartbeat worker should only send a heartbeat. It should not monitor
an acknowledgement, choose a terminal state, or attempt recovery.

### 4. The control protocol is symmetrical without a matching requirement

The current control protocol has:

- `Ready`
- `Heartbeat:<sequence>`
- `HeartbeatAck:<sequence>`
- `RequesterStopping`
- `RequesterStoppingAck`

It also has a general enum, encoder, parser, sequence parser, two leases, a
sender-binding ID, and a 200 ms acknowledgement wait.

The minimum protocol is:

- host to requester: `Ready`, then periodic `Heartbeat`;
- requester to host: `RequesterStopping`.

`Ready` can later be collapsed into the first heartbeat because either message
is a valid post-admission proof. Keeping an explicit one-shot `Ready` in the
first refactor is the lower-risk change and makes the handshake easy to test.
Start heartbeat emission only after `Ready` has been sent successfully.

With three exact messages, direct `JsonString` construction and strict
per-direction comparisons are clearer than a general message-kind parser.
The heartbeat sequence can also be removed if the channel's ordered,
single-sender delivery is retained. A stale duplicate could otherwise extend
the lease only if a transport is allowed to replay successful channel
packages; add a focused test before removing the sequence if that guarantee is
unclear.

### 5. Fail-fast does not mean throwing from every callback

Synchronous startup, RPC setup, service registration, and RPC execution can
remain uncaught.

Transport terminal conditions are different: `OnDisconnected`,
`OnReadError`, and `OnLocalError` report failures as callbacks. Throwing from
them is not a reliable cross-transport process policy:

- named-pipe and Windows HTTP callbacks cross operating-system C callback
  boundaries;
- MiniHTTP invokes some completion callbacks behind a `catch (...)` that
  discards callback exceptions;
- exceptions escaping a `vl::Thread` do not propagate to the owner thread.

Use one non-returning host exit boundary for these asynchronous events:

- exact `RequesterStopping`: `_Exit(0)`;
- fatal transport callback, disconnected heartbeat send, or exception caught
  at an OS/thread callback boundary: `_Exit(1)`, `abort`, or the repository's
  chosen fail-fast primitive.

This catch is an exception firewall, not recovery. It should terminate
immediately and own no error state. Do not perform fallible logging before the
guaranteed terminator.

If VlppOS adopts the same library-wide fail-fast policy, replace callback
exception swallowing with a defined terminate-at-callback-boundary policy.
Application code still must not attempt to propagate exceptions across an OS
callback ABI.

### 6. `Main.cpp` performs recovery that the host does not need

`RunHost` currently:

- catches `Exception` and `Error` around all host work;
- accumulates a result;
- begins an application stopping state;
- attempts RPC finalization and catches cleanup failures;
- stops the transport and catches cleanup failures.

The target host main path is straight-line:

1. Create parser, task queue, channel client, and service.
2. Connect and initialize the generated RPC lifecycle.
3. Register `rvmt::IViewModel` and initialize the dispatcher.
4. Send `Ready` and start one-way heartbeat emission.
5. Run the task queue.

The exact stop message exits from its control callback. Unexpected synchronous
exceptions escape. There is no subsequent graceful cleanup path to protect.

### 7. Requester state is duplicated and contains production-dead renderer state

`RemoteViewModelRoleState` contains renderer ownership even though Core owns
renderer admission and replacement:

- `rendererClientId`
- `GetRendererClientId`
- `TryAcceptRenderer`
- `DisconnectRenderer`

These members are referenced by unit tests but not by the production RVM
session. Delete them and their tests. Keep renderer policy in
`RemotingTest_Core`.

Other duplicated or unnecessary state includes:

- `remoteViewModelEnabled`, although constructing the session already enables
  the mode;
- a default `Running` phase followed by an artificial transition back to
  `Starting`;
- `RemoteViewModelRoleState::fatalError` plus
  `RemoteViewModelTerminalState`;
- host and requester IDs stored in multiple owners;
- `RemoteViewModelBroadcastingLocalClient::clientId`, which is written but not
  read.

`RemoteViewModelRoleState.h` also includes
`GuiRemoteProtocol_Channel_Json.h` solely to classify renderer channels. Split
host-role classification from Core's renderer classification so the shared
RVM requester code no longer depends on renderer protocol details.

The requester session needs only a small locked state:

- starting/running/stopping;
- accepted host ID and `everAccepted`;
- whether broker registration was claimed;
- host lease expiration;
- first host-loss message/action, because Core must order fatal renderer
  delivery.

Core's own renderer and fatal-delivery state must not move into this object.

### 8. Queue failure and cleanup code attempts recovery

`RemoteViewModelTaskQueueThread` records an exception, invokes a failure
callback, and restarts `RunTaskQueue` in a loop. This is contrary to the
fail-fast test-app policy. A dispatcher task failure is not recoverable.

Replace it with a minimal one-use queue thread. If a task throws, terminate at
the thread boundary. Do not restart the queue or convert arbitrary exceptions
into the special "host disconnected" behavior.

Likewise:

- remove `ReportFailure`, `SetFailureCallback`, `GetFailure`, and stored
  failure state;
- make requester `Stop` straight-line instead of returning a first cleanup
  error;
- remove cleanup `try`/`catch` ladders and `RecordFirstError`;
- inline or remove `StopRpcTaskQueue`;
- keep only the ordering needed to stop the server while callback targets are
  still alive.

`FinalizeRpcOnTaskQueue` may remain if dispatcher finalization must execute on
the queue thread, but it does not need exception transport or recovery.

### 9. The runtime header exposes implementation-only machinery

Most helper classes and free functions in `RemoteViewModelTestRuntime.h` are
used only by `RemoteViewModelTestRuntime.cpp`.

After simplification, expose only the requester session and the small hosting
client surface required by their callers. Move control parsing, background
workers, local-client implementations, and cleanup helpers into the `.cpp` or
a private implementation. This does not reduce executed code by itself, but it
removes misleading public complexity and coupling.

### 10. Broker registration currently calls outward while holding a spin lock

`RemoteViewModelRequesterSession::RegisterViewModelHost` calls
`brokerDispatcher->RegisterClient(clientId)` while holding `lockState`.

Claim the accepted registration under the lock, release the lock, and then
call the broker. This reduces lock scope and avoids scheduling RPC work while
holding session state.

### 11. Several runtime APIs and fields are unused

Remove after confirming the final call graph:

- `RemoteViewModelRequesterSession::GetViewModel`
- `RemoteViewModelRequesterSession::IsRemoteViewModelClientId`
- renderer-related `RemoteViewModelRoleState` APIs
- `RemoteViewModelBroadcastingLocalClient::clientId`
- query methods retained only for state-machine unit tests that are deleted
- partial-start and idempotent-stop flags made impossible by a linear,
  fail-fast lifecycle

Do not keep a state abstraction solely because its tests reference it. Tests
should protect required production behavior.

### 12. Connection callbacks are used for work that can be explicit

`RemoteViewModelRequesterLocalClient::OnConnected` and
`RemoteViewModelHostingClient::OnConnected` exist mainly to call
`InitializeRpc(clientId)`.

`WaitForServer` and `ConnectLocalServer` return only after assigning the client
ID, and the RpcJson reader is installed before the wait. Call
`InitializeRpc(GetClientId())` or `InitializeRpc(returnedClientId)` explicitly
from the owning `Connect` method instead. This removes callback-only setup and
makes the startup order visible in one straight-line function.

Initialize the control-channel reader in that same explicit connection path.
Packages queued before reader installation are replayed when `IChannel` is
initialized.

## Target Host Shape

`RemoteViewModelHostingClient` should have only four responsibilities:

1. Advertise `ViewModelChannel` and `ViewModelReadyChannel`.
2. Initialize its generated RPC dispatcher after channel connection.
3. Send one post-route `Ready` plus periodic one-way heartbeats.
4. On exact `RequesterStopping`, terminate normally; on asynchronous terminal
   failure, terminate abnormally.

It should not own:

- a requester lease;
- heartbeat or stop acknowledgements;
- a normal/failure/taken terminal-state machine;
- recovery callbacks;
- cleanup-result aggregation;
- reconnect or retry policy.

The requester remains more complex than the host because the SOP assigns it
the actual behavior: reserve one host, wait for the service, detect host loss
under all transports, distinguish normal stopping, and let Core deliver a fatal
renderer package.

## Actionable TODO

### P0: Lock the required behavior before deletion

- [ ] Add focused component/process coverage for requester startup, host
  admission, post-route broker registration, service acquisition, and one
  successful `Translate` call.
- [ ] Kill the host before `Ready`, while idle, and during `Translate`; require
  requester failure under `/Pipe`, `/Http`, and `/MiniHttp`.
- [ ] Verify Core sends exactly one host-loss `!Error`, with no earlier
  `ControllerConnectionStopped`, before its nonzero termination.
- [ ] Verify `CppTest_Rvm` terminates nonzero on host loss.
- [ ] Verify normal Core/Cpp stopping sets requester `stopping`, sends one host
  stop signal, and never reclassifies the resulting host disconnect as fatal.
- [ ] Verify renderer disappearance/replacement remains nonfatal and does not
  affect the RVM host.

### P1: Reduce `RemotingTest_RvmHost` to fail-fast behavior

- [ ] Replace the host's message/bool `terminalAction` with a small
  non-returning normal/fatal exit boundary, or make that policy direct in the
  host client because it has no other caller.
- [ ] Replace `RemoteViewModelHostingClient::OnRead` with strict handling of
  the one requester stop message followed by immediate normal exit.
- [ ] Replace host `OnDisconnected`, `OnReadError`, and fatal `OnLocalError`
  recovery with immediate abnormal termination. Ignore only documented
  nonfatal pre-connection retry notifications.
- [ ] Reduce the host worker to periodic one-way heartbeat send/flush. At its
  OS/thread exception boundary, terminate instead of reporting state.
- [ ] Delete host `terminalState`, `requesterLease`,
  `requesterReadyClientId`, heartbeat acknowledgement handling,
  `ReportFailure`, terminal `ProcessState`, and `BeginStopping`.
- [ ] Delete the host-side `HeartbeatAck` and `RequesterStoppingAck` protocol,
  constants, sequence/lease state, and acknowledgement wait.
- [ ] Make `RunHost` straight-line. Remove its catch/result/cleanup ladders and
  explicit RPC/transport recovery.
- [ ] Order startup as connect/lifecycle setup, service registration,
  dispatcher initialization, `Ready`, heartbeat start, and task-queue run.
  This makes `Ready` mean the host is both routable and prepared.

### P2: Reduce requester/control plumbing without changing behavior

- [ ] Fold `RemoteViewModelReadyLocalClient` into
  `RemoteViewModelBroadcastingLocalClient`; keep separate readers for the two
  logical channels.
- [ ] Retain only requester-side host lease/startup grace. Renew it from
  one-way host heartbeats; send no acknowledgement.
- [ ] In `BeginStopping`, claim stopping before sending the one-way stop
  message. Remove `RequesterStoppingAck`, `stoppingAcknowledged`, and the
  200 ms wait.
- [ ] Replace the general control enum/encoder/parser with strict
  direction-specific exact messages. Treat an unexpected or malformed control
  package as a test failure instead of silently ignoring it.
- [ ] Initially retain explicit `Ready` as the post-admission barrier. After
  focused startup-race coverage exists, consider using the first heartbeat as
  that barrier and delete the separate `Ready` message.
- [ ] Move `brokerDispatcher->RegisterClient` outside `lockState`.
- [ ] Move RpcJson and control-reader initialization from `OnConnected`
  overrides into the explicit local/network `Connect` paths.
- [ ] Replace `RemoteViewModelRoleState` and duplicate terminal state with the
  minimal requester facts listed above.
- [ ] Delete renderer fields and transitions from the shared RVM state; Core
  remains the only renderer owner. Remove the RVM runtime's renderer-protocol
  include and classification coupling.
- [ ] Remove dead fields/APIs and update tests to cover production behavior
  through the session rather than obsolete state helpers.
- [ ] Replace the retrying/error-recording task-queue thread with a one-use
  fail-fast thread.
- [ ] Make requester cleanup straight-line while preserving callback-target
  lifetime and server-stop ordering.
- [ ] Move implementation-only helper types out of
  `RemoteViewModelTestRuntime.h`.

### P3: Decide whether the requester scheduler is intentionally generic

- [ ] If the RVM test contract will remain request-only with no callbacks,
  events, nested RPC, or requester-owned objects, prototype direct scheduling
  and remove the requester task queue/thread.
- [ ] Otherwise retain scheduling and document that it supports future
  bidirectional RpcJson behavior; remove only its recovery machinery.
- [ ] Do not make host RPC execution inline merely to save lines without
  testing transport reentrancy and nested-call behavior.

### P4: Remove the real upstream boilerplate

- [ ] In Workflow's generator/source, generate one module-specific
  `ConfigureJsonRpcLifecycle` or lifecycle factory containing the generated
  IDs, serializer, operations, listener, and wrapper factory.
- [ ] Add a reusable Workflow network/local endpoint helper that accepts that
  generated lifecycle factory, connects, and initializes it with the assigned
  client ID.
- [ ] Add a reusable Workflow broker-local-client composition so applications
  do not repeat channel maps, local connection, dispatcher ownership, and
  broker access.
- [ ] In VlppOS, add distinct pre-admission and post-commit client callbacks,
  or another explicit route-ready hook. Then broker registration no longer
  needs an application-level `Ready` workaround.
- [ ] Consider a protected `EnsureChannel(name)`/constructor channel list in
  VlppOS to remove the repeated dummy `ChannelMap` plus
  `OnGetChannelNames` override.
- [ ] Define the library-wide exception-firewall rule: callback exceptions
  must terminate at OS/thread boundaries instead of being silently swallowed.
- [ ] Do not add graceful RpcJson cancellation merely for these test apps.
  Broker-ID, service, and response wait cancellation is needed only if a
  future application explicitly requires recovery.
- [ ] Change Workflow/VlppOS source repositories and regenerate their
  release/import artifacts; do not patch GacUI `Import` files directly.

### P5: Adopt and verify

- [ ] Refactor GacUI to use the upstream generated lifecycle and endpoint
  helpers when they are available.
- [ ] Update `Project.md`, Workflow RPC documentation/examples, and any channel
  protocol comments if `Ready` is collapsed or the exception boundary changes.
- [ ] Run `git diff --check`.
- [ ] Build affected Windows configurations and run `UnitTest`.
- [ ] Build the documented Linux debug/x64 targets.
- [ ] Run the full process matrix for `/Pipe`, `/Http`, and `/MiniHttp`,
  including native renderer and GacJS cases where supported.
- [ ] Repeat abrupt-host and normal-stop tests to catch post-admission,
  stopping, and callback-lifetime races.

## Do Not Remove

- The separate broker and requester RPC endpoint.
- A post-admission route-ready proof before broker registration.
- One-way host-to-requester liveness while the SOP promises abrupt host-loss
  detection for HTTP transports.
- Requester `stopping` suppression before it asks the host to exit.
- Core's independent, one-shot fatal renderer delivery; it must work even when
  the GUI/RPC thread is blocked.
- Ordered transport/server teardown while callbacks still reference requester
  objects.

These items implement actual behavior. The acknowledgements, reverse lease,
general state machines, recovery catches, and third local client do not.
