# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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

# UPDATES

## Baseline audit

- `RemoteViewModelRoleState::RegisterRequester` validates the proposed ID
  against the other roles but never requires the existing requester ID to be
  invalid. A second requester therefore replaces the first ID, leaving the
  first broker participant impossible to disconnect through the stored role.
- `CppTest_Rvm` and `RemotingTest_Core /RVMT` duplicate requester ownership and
  split fatal authority between their role state, an application error string,
  and the task-queue thread. A physical host disconnect calls `std::_Exit`
  directly, while a task-queue failure takes a different callback path.
- Core's direct physical-disconnect exit bypasses `BroadcastError`, so a
  renderer cannot receive the required Core-authored `!Error`. Posting a
  window-close callback would not solve this because the GUI thread can be
  blocked in a synchronous RPC.
- `RemotingTest_RvmHost` overrides only `OnConnected`. Its inherited
  disconnect, read-error, and fatal-local-error callbacks are no-ops, so the
  process can remain blocked in broker login, dispatcher initialization, the
  task queue, or a synchronous RPC after requester loss.
- Pipe exposes abrupt loss through the existing transport callback, but HTTP
  and MiniHTTP can retain a logical client token indefinitely. Their existing
  send and batch APIs cannot prove that the peer is alive; an acknowledged,
  host-specific lease is required.
- Renderer protocol packages are queued until the UI invoker exists, but
  transport terminal callbacks are not. `OnDisconnected` immediately calls
  `Detach`, which clears a queued `ControllerConnectionStopped`; an early
  `!Error` dereferences `GetCurrentController` before raw renderer setup; and
  queued fatal UI captures stack-owned renderer-automation pointers.
- Core explicitly submits `ControllerConnectionStopped` after
  `GuiApplication::Run`, while `GuiRemoteController::Finalize` submits the same
  NODROP message again.
- The derived MiniHTTP server destructors in Cpp and Core redundantly call
  `Stop`; the owning startup functions already have the correct scope in which
  to stop the server while callback targets are alive.
- No VlppOS or Workflow source change is necessary. The existing JSON channel
  can carry a bidirectional lease, and fail-fast process actions avoid the need
  for a new Workflow cancellation API.

# TEST [CONFIRMED]

The investigation uses one focused baseline failure plus unit and end-to-end
coverage for the lifetime changes:

- `TestRemoteViewModelRuntime.cpp` registers requester `10`, attempts to
  register requester `11`, and requires the second registration to be rejected
  without changing requester `10`. The current implementation accepts `11` and
  overwrites the live requester ID, so this is the root reproducer.
- Shared terminal and lease-state tests use a controllable millisecond clock.
  They cover startup grace, heartbeat renewal, expiry, normal stopping,
  repeated terminal notifications, and response/expiry/stopping races. Exactly
  one process-specific terminal action may be claimed.
- Async renderer-channel tests queue protocol packages and a transport terminal
  notification before and after invoker installation. They require FIFO
  delivery, retention across startup, classification before detachment, and
  cancellation of callbacks after explicit shutdown.
- Existing remote-controller tests continue to require exactly one
  `ControllerConnectionStopped` during normal framework finalization.
- Static verification requires the Core-specific duplicate stop submission and
  MiniHTTP derived destructors to be absent, the two SOP typos to be corrected,
  generated/import/release folders to remain untouched, and
  `git diff --check` to pass.
- Build the Windows solution in every affected configuration and run the full
  Debug x64 `UnitTest` suite with no memory-leak report.
- Exercise `CppTest_Rvm` plus `RemotingTest_RvmHost` and Core `/RVMT` plus the
  host and native renderer with `/Pipe`, `/Http`, and `/MiniHttp`. Verify normal
  requester loss terminates the host, host loss during startup/idle/active RPC
  terminates requesters nonzero, Core delivers its exact fatal `!Error` before
  exiting, normal Core loss remains nonfatal to renderers, renderer replacement
  remains supported, endpoints can be reused, and no process is orphaned.
- Exercise Core `/RVMT` plus the host and GacJS with `/Http` and `/MiniHttp`.
  Host loss must show Core's exact error mask, not a fetch/404/success mask;
  normal application exit and Core loss must retain their existing nonfatal
  renderer behavior.

Success requires the focused reproducer to fail on the untouched baseline and
pass after the proposal, all automated regressions to pass, every exercised
process outcome to match the matrix in the problem description, all terminal
transitions to be one-shot and callback-safe, and every supported transport to
detect abrupt host loss within its documented lease bound.

The baseline was built with `copilotBuild.ps1 -Configuration Debug -Platform
x64`; MSBuild completed with zero warnings and zero errors. The full Debug x64
`UnitTest` run then reached
`TestRemoteViewModelRuntime.cpp / RemoteViewModelRoleState / A second requester
is rejected without replacing the first requester` and failed at exactly
`!state.RegisterRequester(11)`. This proves the first requester is overwritten
by the untouched implementation. The GUI tests regenerated six unrelated file
dialog snapshots during this run; those mechanical test artifacts were
restored and are not part of the reproducer.

# PROPOSALS

- No.1 Centralize requester lifetime and serialize renderer terminal delivery

## No.1 Centralize requester lifetime and serialize renderer terminal delivery

Implement one application-owned lifetime protocol rather than adding a
transport-specific workaround or changing Workflow's blocking RPC API. The
proposal has four coordinated parts.

First, make `RemoteViewModelTestRuntime.{h,cpp}` the owner of the RVM requester
session. A new `RemoteViewModelRequesterSession` owns the role state, exact
terminal result, task queue and its thread, broadcasting/ready/requester local
clients, broker and requester dispatchers, acquired proxy, liveness worker,
and ordered shutdown. Cpp and Core channel servers retain only their
transport-specific admission work and delegate every RVM admission,
registration, disconnect, phase, and cleanup operation to this session. Core
continues to own renderer IDs, replacement, and detach because those concerns
do not exist in Cpp. This removes the duplicate application interfaces and the
two copies of startup/finalization code without generalizing unrelated
renderer behavior.

The session has one first-terminal-wins state protected by its state lock.
Physical host loss, lease expiry, and RPC task-queue failure all claim that
same state with the exact error text. Claiming only signals an owned worker;
the transport callback never throws, manipulates a window, broadcasts, or
terminates directly. The worker runs the process-specific action outside every
session/server lock:

- Cpp logs the exact error and exits nonzero.
- Core calls `BroadcastError(error)` while the renderer transport is still
  alive, tolerates a delivery failure, and then unconditionally exits nonzero.

This fail-fast action is deliberate. It terminates startup waits, an idle task
loop, and a GUI thread blocked in a synchronous RPC without needing a new
Workflow cancellation primitive. A normal `BeginStopping` transition wins
against later callbacks, disables liveness expiry, and sends a requester-stop
control message before transport teardown. The host treats that message as a
successful terminal outcome; every disconnect/read/fatal-local error that
precedes it remains a one-shot failure.

Second, use only the host-specific `ViewModelReadyChannel` for an acknowledged
lease that works identically over Pipe, HTTP, and MiniHTTP. The shared header
defines the control-message names, canonical error, heartbeat interval, lease
timeout, startup grace, and terminal-delivery delay. The host begins sequenced
heartbeats as soon as the physical channel is connected, before broker login.
The requester binds the accepted host ID, validates increasing sequences,
renews its lease, and returns a targeted acknowledgement. The host validates
the acknowledgement sender and sequence and renews its requester lease.
Startup grace begins only after host admission, renderer traffic never touches
either lease, and stopping invalidates the lease before callbacks can classify
local teardown as remote failure. The state machine accepts a supplied
millisecond timestamp so expiry, renewal, stale packets, and stopping races can
be tested deterministically.

Third, put renderer transport terminal work into the same versioned FIFO as
remote JSON packages. `GuiRemoteProtocolAsyncJsonChannelRenderer` queues
either a package or a main-thread action and processes both in exact order.
The native renderer client translates Core `!Error` and transport disconnect
callbacks into these actions. Fatal claiming occurs only when the action is
executed, so an earlier queued `ControllerConnectionStopped` wins; disconnect
detaches only after all earlier work has been classified. Local transport
errors do not fabricate a Core fatal. Renderer, automation substitution,
automation target, and invoker are installed before cached work is drained
synchronously. Queued actions capture copied values and a stable client only;
they re-read guarded target pointers on the UI thread, and explicit stopping
invalidates/detaches the queue before stack-owned automation objects disappear.

Finally, apply the direct correctness cleanups discovered by the reproducer:

- reject a second requester without changing the first ID;
- remove Core's explicit normal-stop submission and leave
  `GuiRemoteController::Finalize` as the sole owner;
- remove the redundant Cpp/Core MiniHTTP derived destructors;
- correct `ControllerConnectionEstablished` to
  `ControllerConnectionStopped` and `/RVMP` to `/RVMT` in the SOP.

No `Import`, `Release`, generated source, VlppOS, or Workflow change is part of
this proposal.

### CODE CHANGE

- Add shared terminal, lease/control-message, background-worker,
  requester-session, and hosting-client implementation to
  `RemoteViewModelTestRuntime.{h,cpp}`, with constants in
  `RemoteViewModelTestShared.h`.
- Reduce the Cpp and Core channel-server/application code to delegation around
  the shared session; preserve Core-only renderer replacement and non-RVM
  behavior.
- Extend the async renderer channel with a versioned package/action FIFO and a
  synchronous pending-work drain; update the native renderer to use ordered,
  guarded terminal actions and startup-safe installation/cleanup.
- Fix requester admission, remove the duplicate Core stop and MiniHTTP
  destructors, and correct the two SOP terms.
- Extend unit coverage for second-requester preservation, terminal/lease state,
  and async package/action ordering, then run the complete build and lifecycle
  matrix recorded in `# TEST`.

## User clarifications and final transport ownership

- A fatal local error is independently terminal because the connection is no
  longer reliable. Callers must act on `OnLocalError(..., true)` immediately;
  correctness must not depend on a later `OnDisconnected`.
- The historical test-application behavior in which a post-connection HTTP 404
  is fatal at the channel boundary must be retained. This is a channel-lifetime
  decision, not a request made by the raw HTTP protocol.
- Ownership of promoting a post-connection 404, or any other local protocol
  error after connection, belongs to VlppOS's `IChannelClient`
  implementation. Raw VlppOS HTTP reports the failed exchange as nonfatal and
  keeps its transport retry policy; the channel client promotes the callback
  before retry because its logical delivery guarantee has already failed.

These clarifications supersede the earlier statements in this log that Core
transport loss must remain nonfatal at the channel boundary, that no VlppOS
change is required, that generated `Import` files are outside the change, and
that local transport errors remain nonfatal. GacUI imports the regenerated
VlppOS release. The native renderer consumes the promoted fatal callback
directly, queues its ordinary disconnected transition in protocol order, and
does not synthesize Core's fatal UI; only a Core-authored `!Error` package shows
that prompt or overlay.

## Verification

- VlppOS Debug x64 passed all 16 test files and 273 test cases with no
  memory-leak report. Debug/Release x64 and Win32 all built with zero warnings
  and zero errors. `CodePack` regenerated the release, and all six imported
  `VlppOS*` files in GacUI byte-match that release.
- GacUI Debug/Release x64 and Win32 all built with zero warnings and zero
  errors. The first post-import Debug x64 build had one diagnostic-free
  `CL.exe` process exit; the immediate incremental retry passed cleanly.
- The complete GacUI Debug x64 `UnitTest` run passed 89/89 test files and
  1728/1728 test cases with no memory-leak artifact.
- A live `/Http /RPT` native-renderer run changed `Click Me!` to
  `You have clicked!` through renderer `/IO`. A second live renderer took over
  and retained that state; the old renderer exited with code 0 after its stale
  HTTP exchange while Core remained alive. Forcibly terminating Core then made
  the active renderer exit with code 0. Neither channel-fatal transition opened
  a fatal dialog or entered a retry loop.
- A live `/Http /RVMT` Core/host/native-renderer run showed the exact
  `ERROR from GacUI Core` prompt with
  `RemotingTest_RvmHost disconnected.` after the host was killed. Choosing the
  retain option exposed the same `fatalError` in renderer DOM, ordinary IO
  returned `!Application stopped responding.`, Core exited with code 1, and
  exact `!Exit` closed the renderer with code 0.
- Linux verification was unavailable because WSL is not installed. The
  browser/GacJS acceptance path was not rerun in this Windows-native pass.
