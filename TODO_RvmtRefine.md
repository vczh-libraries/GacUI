# Refine Remote View-Model Test-App Lifetime Handling

This document is a static code review of the work centered on commit
`96d3f3a08cc0eef8d401d582ccd70a35843f0c7c`. No application was built or run
while preparing it.

## My Original Requirement

- `CppTest_Rvm` with `RemotingTest_RvmHost`:
  - Closing any process doesn't automatically close others.
- `RemotingTest_*` with or without `/RVMT`:
  - Closing any process doesn't render a fatal error on `RemotingTest_Rendering_Win32`.
  - Closing `RemotingTest_Rendering_Win32` only closes `RemotingTest_Core`.
- `RemotingTest_RvmHost` might just close itself when the connection between `RemotingTest_Core` is lost. `CppTest_Rvm` should do the same. `RemotingTest_Core` should do the same thing except that a fatal error should be sent to `RemotingTest_Rendering_Win32` first.
- Try to simplify code or remove duplicated code in `Test/GacUISrc`:
  - `CppTest_Rvm/*.cpp`
  - `RemotingTest_Core/GuiMain.cpp`
  - `RemotingTest_RvmHost/*.cpp`
- Verify `RemotingTest_Core /RVMT` + `RemotingTest_RvmHost` + `GacJS` to make sure terminating any process renders a fatal error on `GacJS`.

## What Need to Fix

### 1. Make the required process outcomes explicit

The phrases "closing" and "terminating any process" mix normal application
shutdown, an explicit renderer exit request, abrupt process loss, and RVM-host
loss. They do not have the same required result. The implementation and tests
must use the following matrix, which follows `Project.md` and
`DebugRemoteProtocolSop.md`.

| Topology and event | Required result |
| --- | --- |
| `CppTest_Rvm` exits normally or is killed | `RemotingTest_RvmHost` detects that its requester is gone and exits without remaining blocked. |
| `RemotingTest_RvmHost` disconnects before the `CppTest_Rvm` window has closed | `CppTest_Rvm` terminates with an error. This must work during startup, while idle, and during an RPC call. |
| Core exits normally, including an exit requested through the active renderer | Core sends exactly one `ControllerConnectionStopped` when possible. The renderer enters its normal terminal state without a fatal error. Under `/RVMT`, the host detects Core loss and exits. |
| Core is killed or its transport disappears | The renderer treats the resulting 404/transport failure as a normal disconnection, not a locally synthesized fatal error. Under `/RVMT`, the host detects Core loss and exits. |
| A renderer is killed or a browser page disappears without an exit request | Core remains running. Under `/RVMT`, the host also remains running. A replacement renderer can connect and take over the preserved application state. |
| A renderer explicitly requests application exit | Core performs its normal close path. Under `/RVMT`, Core shutdown then causes the host to exit. No fatal error is rendered. |
| A new renderer replaces an old renderer | The old renderer settles without a fatal error or retry loop; Core and the new renderer remain live. |
| `RemotingTest_RvmHost` disconnects before the `/RVMT` Core window has closed | Core sends exactly one fatal `!Error` package to the active renderer, does **not** send `ControllerConnectionStopped` first, and then terminates with an error. Native and GacJS render the Core-authored error. |

Consequently, the final GacJS bullet can only mean terminating
`RemotingTest_RvmHost` while Core and GacJS are alive. Killing Core cannot
produce a Core-authored fatal package, and killing GacJS cannot make that same
GacJS process render anything. `Project.md` explicitly requires a post-Core
HTTP failure to remain nonfatal.

### 2. Repair the requester/host lifetime propagation

- `RemotingTest_RvmHost/Main.cpp` has no `OnDisconnected`, `OnReadError`, or
  fatal `OnLocalError` handling. Its owner thread can remain blocked forever in
  `taskQueue->RunTaskQueue()` after Core or `CppTest_Rvm` disappears.
- Add a thread-safe, one-shot terminal transition to
  `RemoteViewModelHostingClient`. Losing the requester must terminate the host
  during connection/login, dispatcher initialization, idle task-queue
  processing, and an active RPC call. Local callbacks caused by the host's own
  `Stop()` must be suppressed.
- Follow the test-app policy in `Project.md`: direct fail-fast process
  termination is preferable to a large graceful-recovery state machine. Merely
  queuing `QueueExitTask()` is insufficient because the host can still be
  blocked before it reaches `RunTaskQueue()`.
- If implementation nevertheless chooses clean unwinding instead of direct
  process termination, the Workflow dispatcher needs an upstream,
  idempotent terminal-failure operation that wakes every broker-ID,
  service-acquisition, task-queue, and direct-response wait. Do not patch
  generated `Import` files directly.

### 3. Deliver Core's fatal error before terminating

- `RemotingTest_Core/GuiMain.cpp` currently identifies the RVM host in
  `RemotingChannelServerBase::OnClientDisconnected`, logs the loss, and calls
  `std::_Exit(1)`.
- That exit bypasses the only existing renderer-fatal path: the `GuiMain`
  exception handlers call `protocolServer->BroadcastError(...)`, which emits
  `!Error` before stopping the transports. Native rendering therefore receives
  only `OnDisconnected`, and GacJS receives only an HTTP disconnect.
- Replace the divergent host-loss and `FailRpcTaskQueue` branches with one
  latched Core-fatal operation. It must retain the exact error, broadcast it
  while the protocol server and renderer connection are still live, and then
  terminate Core with a nonzero result even if broadcasting fails.
- Run the broadcast from a context that is safe with respect to transport
  callbacks and `Stop()`. Do not depend on posting `window->Close()`: the UI
  thread can be blocked in `IViewModel::Translate`, and a window close can be
  rejected.
- The fatal path must never run normal finalization first. In particular, it
  must not send `ControllerConnectionStopped`, because a renderer is required
  to ignore later fatal errors after receiving that message.
- `CppTest_Rvm` already calls `_Exit(1)` when a physical host disconnect is
  detected. Keep its simple nonzero fail-fast behavior, but route host loss and
  RPC task-queue failure through one one-shot decision so the two failure paths
  cannot diverge.

### 4. Detect abrupt host loss for every supported transport

- Current server-side disconnect callbacks are enough for a detectable named
  pipe loss, but not for `/Http` or `/MiniHttp`.
- Those transports keep a logical connection token behind `/Connect`,
  `/Request`, and `/Response`. A client process disappearing does not
  necessarily remove the server token or invoke `OnClientDisconnected`.
- A graceful logout alone cannot cover a killed process. Add bounded,
  host-specific liveness, such as an opt-in heartbeat/lease over the existing
  RVM host logical channels. A controlled shutdown signal may shorten the
  graceful case, but the lease must cover abrupt termination.
- Do not add a reverse HTTP `/Disconnect` endpoint, require a
  renderer-to-Core shutdown handshake, or change ordinary renderer
  disconnection behavior. The liveness policy is for the single RVM host role,
  not for `GacUIRemoteProtocol`.
- Define the heartbeat interval, lease timeout, startup grace period, and
  stopping behavior in one shared location. The requester/Core's own teardown
  after its main window is closing, scheduler shutdown, or renderer replacement
  must not be classified as host failure. In contrast, an RVM host-initiated
  `Stop()` while the requester is still live is host loss and must trigger the
  required requester failure.

Without this work, the requested `/Http`, `/MiniHttp`, and GacJS termination
cases can still leave Core or `CppTest_Rvm` running indefinitely.

### 5. Make native renderer terminal delivery ordered and startup-safe

- `RemotingTest_Rendering_Win32::StartClient` waits for the server before
  `SetupRawWindowsDirect2DRenderer()` creates the native controller and window.
  If Core sends `!Error` in that interval, `QueueFatalPrompt()` dereferences
  `GetCurrentController()` before it exists. A disconnect in the same interval
  can be lost and allow a permanently "Connecting ..." window to open.
- Cache fatal and terminal notifications until the renderer, automation
  service, and main-thread invoker are installed. Apply the cached terminal
  state before exposing a live window.
- `OnDisconnected()` currently detaches the async renderer channel
  immediately. Detaching can clear an already-received but not-yet-dispatched
  `ControllerConnectionStopped` or `!Error` package. Marshal protocol messages
  and transport terminal callbacks through one ordered main-thread queue, and
  classify the terminal state before detaching.
- `QueueFatalPrompt()` captures raw renderer and stack-owned automation-service
  pointers. A queued callback can run after cleanup. Re-read guarded state when
  the callback executes, or use a generation/cancellation token; do not retain
  stale callback targets across shutdown.
- Do not fix these races by converting an ordinary Core transport disconnect
  into a fatal prompt. `ForceExitByFatelError()` currently hides the native
  renderer; despite its name, that silent terminal result is correct for Core
  loss. The fatal UI in this task must originate from Core's `!Error` after RVM
  host loss.

### 6. Remove the duplicate normal-stop message

`RemotingTest_Core/GuiMain.cpp` explicitly submits
`ControllerConnectionStopped`, and `GuiRemoteController::Finalize()` submits
the same message again. Make framework finalization the single owner of normal
Core shutdown and remove the Core-specific duplicate and any global state that
only supports it. The RVM-host fatal path must bypass this normal finalization.

### 7. Correct role-state admission

`RemoteViewModelRoleState::RegisterRequester()` checks that a new client ID is
distinct from the other roles, but it does not check that
`requesterClientId` is invalid. A second distinct requester can overwrite the
live requester ID, leaving the previous broker registration behind and making
later disconnect cleanup inconsistent. Reject a requester unless the slot is
empty and add coverage for the rejection.

### 8. Consolidate duplicated RVM test-app code

Put the shared RVM requester lifetime in
`RemotingTest_RvmHost/RemoteViewModelTestRuntime.{h,cpp}` (or an equivalently
small shared test-runtime pair):

- Own the task queue/thread, broker local client, ready client, requester
  client, dispatcher, proxy, terminal state, and ordered shutdown in one
  session object.
- Share host reservation, broker registration, requester admission, stopping,
  and one-shot failure bookkeeping between `CppTest_Rvm` and
  `RemotingTest_Core`.
- Expose only the process-specific terminal action:
  `CppTest_Rvm` exits with an error; Core broadcasts a fatal error and exits.
- Keep Core's renderer admission, renderer replacement, and
  `GacUIRemoteProtocol` routing in Core. They do not belong in the generic RVM
  session.
- Replace the identical `IRemoteViewModelUiServer` and
  `IRemoteViewModelCoreState` interfaces with one shared contract, or remove
  the interface if the owned session makes it unnecessary.
- Collapse the duplicated fatal boolean, fatal string, callback, and
  application-phase state into one authoritative terminal state. Simplify
  `RemoteViewModelRoleState` to the role IDs and admission facts that remain
  necessary.
- Use owned cleanup/RAII to replace the repeated try/catch cleanup blocks in
  `CppTest_Rvm/GuiMain.cpp`, `RemotingTest_Core/GuiMain.cpp`, and
  `RemotingTest_RvmHost/Main.cpp`. Ensure the server is stopped while all
  callback targets are still alive.
- Once that shutdown owner exists, remove the redundant MiniHTTP derived-server
  destructor that calls `Stop()` in addition to the explicit startup cleanup
  and base shutdown.
- Share transport argument parsing and Pipe/HTTP/MiniHTTP construction only
  where it removes repeated policy. Do not introduce tiny helpers that merely
  move trivial constructor calls.

The goal is to remove duplicated lifetime policy, which has already produced
different Cpp/Core failure behavior, rather than to turn these demo apps into a
production recovery framework.

### 9. Correct the SOP terminal-contract typos

`DebugRemoteProtocolSop.md` contains two relevant typos:

- It calls the remote view-model mode `/RVMP` in its Workflow RPC test-app
  list. The implemented and documented switch is `/RVMT`.
- Its deterministic-shutdown rule says Core sends
  `ControllerConnectionEstablished`; the surrounding rules and implementation
  require `ControllerConnectionStopped`.

Correct both so the acceptance procedure names one canonical mode and one
unambiguous normal-terminal message.

## My Analysis

### Commit scope

- `96d3f3a08cc0eef8d401d582ccd70a35843f0c7c` introduced the relevant RVM
  server/host runtime, Core integration, lifetime state, and tests.
- The adjacent generated/test-UI and release commits do not repair these
  runtime paths.
- The three later commits only update the release-note review bullets. The
  relevant source remains unchanged after `96d3f3a08...`.
- The new unit tests cover channel classification and basic
  `RemoteViewModelRoleState` transitions only. They do not exercise a process
  lifecycle, dispatcher terminal wait, in-flight call, fatal delivery, HTTP
  liveness, or renderer ordering race.

### Why the current process cascade is incomplete

There are two independent missing directions:

1. Host as client does not respond to requester loss, so closing Core or
   `CppTest_Rvm` can strand `RemotingTest_RvmHost`.
2. Requester as server only responds when the transport reports host loss.
   Named pipes can do that, while the current HTTP logical-token protocol
   cannot reliably infer a dead client.

This is why adding only a host `OnDisconnected()` override or only a Core
fatal broadcast would still leave supported cases broken.

### Why Core currently cannot render the requested fatal error

The RVM-host callback terminates Core inside the channel server. It never
reaches `GuiMain`'s catch blocks, which are the code that calls
`BroadcastError`. The native renderer then executes its transport-disconnect
path and GacJS creates a `RemoteProtocolHttpDisconnectError`; neither is the
Core-authored fatal path.

The correct order is:

1. Claim RVM-host loss exactly once.
2. Send `!Error` to the active renderer.
3. Allow the channel implementation's fatal-send barrier to complete.
4. Stop/terminate Core with an error.

Normal finalization and `ControllerConnectionStopped` are deliberately absent
from this sequence.

### Native and GacJS fatal rendering

- Native `OnReadError()` queues the Core fatal prompt. `OnDisconnected()`
  alone only detaches and exits/hides the renderer.
- GacJS converts `!Error` into a normal JavaScript `Error`, which selects the
  error mask. `RemoteProtocolHttpDisconnectError` and the normal renderer-exit
  exception select the success mask.
- Therefore a fetch failure, 404, hidden native window, green success mask, or
  process exit alone does not prove the requested host-loss fatal delivery.
  Verification must observe the actual Core error message in the fatal UI.

### Contract constraints

- Renderer loss is intentionally survivable and renderer replacement is
  supported.
- A Core transport failure after Core has exited is intentionally nonfatal to
  the renderer.
- A normal renderer-requested shutdown is identified by
  `ControllerConnectionStopped`.
- RVM-host loss while a requester window is alive is the exceptional case:
  it is fatal to `CppTest_Rvm` and Core, and Core must report it to its renderer
  first.

These constraints resolve the apparently broader wording in the original
review bullets. No local renderer-generated fatal should be added for a killed
Core, and no reverse HTTP endpoint should be introduced.

## What Need to Do to Verify If All Items Are Addressed

No verification below was performed during this review. It is the acceptance
plan for the implementation.

### 1. Static and build verification

- Review the final diff and confirm that generated folders, `Import`, and
  `Release` were not edited directly. If a Workflow or VlppOS change is truly
  needed, change its source repository and update the imported release through
  the documented dependency process.
- Confirm there is no new HTTP `/Disconnect`, renderer heartbeat requirement,
  renderer-to-Core shutdown acknowledgement, or local fatal synthesis for
  ordinary Core loss.
- Confirm there is one owner for normal `ControllerConnectionStopped` and one
  owner for the Core RVM fatal transition.
- Confirm all terminal callbacks are idempotent, ignore callbacks caused by
  local stopping, and cannot access destroyed callback targets.
- Run `git diff --check`.
- Build every affected Windows project in the configurations required by
  `Project.md`, including `CppTest_Rvm`, `RemotingTest_Core`,
  `RemotingTest_Rendering_Win32`, `RemotingTest_RvmHost`, and `UnitTest`.
- Build the affected Linux `debug x64` projects from their documented
  directories, including the build-only `CppTest_Rvm` target and the runnable
  Core/RVM-host targets.
- Run `UnitTest` after any C++ source/header change.

### 2. Automated focused coverage

- Reject a second requester without changing the original requester ID or
  broker registration.
- Exercise host terminal notification before connection completion, during
  dispatcher initialization, immediately before the task loop, while idle,
  and during one or more outstanding direct RPC calls.
- Race response, heartbeat renewal, lease expiry, local `Stop()`, and repeated
  terminal callbacks; require exactly one terminal action.
- Use a controllable clock for heartbeat/lease tests. Cover startup grace,
  renewal, expiry, and no false expiry during normal teardown.
- Deliver `!Error` and `ControllerConnectionStopped` immediately before and
  after renderer UI/invoker registration and immediately before transport
  disconnect. Require FIFO classification with no null controller, stale
  pointer, discarded terminal package, duplicate prompt, or stuck window.
- If graceful Workflow dispatcher cancellation is implemented, cover every
  broker/service/response wait, future calls after failure, repeated failure,
  response-versus-failure races, and finalization. This coverage is unnecessary
  if the test apps use the preferred direct process termination.

### 3. `CppTest_Rvm` + `RemotingTest_RvmHost`

Run every case with matching `/Pipe`, `/Http`, and `/MiniHttp` arguments:

1. Start `CppTest_Rvm`, then the host, and prove the live UI by performing a
   translation.
2. Close `CppTest_Rvm` normally. Require the host to exit within the defined
   bound.
3. Kill `CppTest_Rvm`. Require the host to exit within the same bound.
4. Terminate the host during login, while the UI is idle, and during
   `Translate`. Require `CppTest_Rvm` to exit nonzero without hanging.
5. Repeat normal and forced termination at least five times. Immediately
   restart on the same pipe/port and require no orphan process, occupied
   endpoint, duplicate fatal transition, or blocked task queue.

### 4. Core + native renderer without `/RVMT`

Run `/FCT` and `/RPT` with `/Pipe`, `/Http`, and `/MiniHttp`:

1. Request application exit through the renderer UI. Require normal Core and
   renderer termination with no fatal prompt.
2. Kill the renderer without sending an exit request. Require Core to stay
   alive, then connect a replacement renderer and verify state continuity.
3. Replace a still-running renderer. Require the old renderer to settle without
   fatal UI and the new renderer to take over.
4. Close and kill Core. Require the renderer to enter a normal terminal state
   without a fatal transport prompt or retry loop.
5. Repeat shutdown/replacement cases to expose ordering races and confirm the
   normal-stop message is not duplicated.

### 5. Core `/RVMT` + host + native renderer

Run with `/Pipe`, `/Http`, and `/MiniHttp`:

1. Start Core, start the host, wait for
   `rvmt::IViewModel acquired; renderer admission is open.`, then start the
   renderer and prove live translation.
2. Terminate the host during Core startup before any renderer is admitted.
   Require Core to exit nonzero without a hang; there is no renderer to receive
   the fatal package in this case.
3. Terminate the host while the connected renderer is idle and during
   translation. Require exactly one Core-authored fatal message in the
   renderer, no prior `ControllerConnectionStopped`, and a nonzero Core exit
   without a hang.
4. Terminate the host after renderer transport connection but before the
   native controller/window is ready. Require the fatal to be retained and
   shown after initialization, with no null-controller crash or stuck
   "Connecting ..." window.
5. In the native fatal prompt, choose the retain option. Require the stopped
   fatal overlay and the exact error in renderer automation DOM data.
6. Exit Core normally through the renderer. Require Core and host to exit and
   the renderer to settle without a fatal.
7. Kill Core. Require the host to exit and the renderer to settle without a
   fatal transport error.
8. Kill the renderer. Require Core and host to remain live, then attach a new
   renderer and verify preserved state.
9. Replace a live renderer and require the old renderer to settle without a
   fatal.
10. Repeat the fatal and normal cascades at least five times and immediately
   reuse the same endpoint.

### 6. Core `/RVMT` + host + GacJS

Run GacJS with `/Http` and `/MiniHttp`; named pipes are not a browser transport:

1. Start Core, then the host, wait until renderer admission opens, load GacJS,
   and prove the session is live by translating text.
2. Gracefully stop and forcibly kill the host, both while idle and during a
   translation. Within the documented lease bound, require:
   - the browser to receive Core's exact `!Error` message;
   - the GacJS fatal/error mask to be visible;
   - the success mask to remain hidden; and
   - Core to exit nonzero.
3. Treat a fetch failure, 404, generic disconnected page, or success mask as a
   failure of host-loss fatal propagation.
4. Request normal application exit through GacJS. Require
   `ControllerConnectionStopped`, the normal/success terminal UI, Core and host
   exit, and no fatal mask.
5. Kill Core. Require host exit and GacJS's documented normal-disconnection UI,
   not a fabricated fatal.
6. Close/kill the browser page without requesting application exit. Require
   Core and host to remain alive, then reconnect GacJS and verify state
   continuity.
7. Repeat each fatal and normal case and confirm immediate port reuse with no
   orphan process or stale logical connection.

### 7. Cross-platform regression

- On Linux, run the documented `/MiniHttp` Core `/RVMT` + RVM-host demo and
  repeat the host-loss, Core-loss, renderer-loss, and replacement cases that
  the platform supports.
- Confirm normal renderer replacement and normal application exit still settle
  without an uncontrolled fatal alert or retry loop on every supported
  renderer.
- Preserve logs proving the process exit codes, fatal package/message, visible
  terminal UI, lease bound, and absence of orphan processes for each topology.
