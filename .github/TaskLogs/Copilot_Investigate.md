# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

[TODO_RvmtRefine2.md](TODO_RvmtRefine2.md) . I would like you to limit the editing scope to `GacUI-repo/Test` with your best effort, unless explicitly instructed in the task description.

# UPDATES

# TEST [CONFIRMED]

The problem is architectural excess rather than a missing end-user behavior.
The required requester/host behavior was established by the predecessor
investigation and remains the regression contract for this refinement:

- The requester starts first, admits exactly one host for the process run,
  waits for the post-route `Ready` barrier, acquires `rvmt::IViewModel`, and
  completes one `Translate` call.
- A host heartbeat renews only the requester-owned lease. Host loss before
  normal stopping remains fatal under `/Pipe`, `/Http`, and `/MiniHttp`.
- Normal requester stopping claims the stopping phase before sending one
  `RequesterStopping` message, so the resulting host disconnect is not
  reclassified as fatal.
- Core retains ownership of renderer admission/replacement and of the one-shot
  fatal `!Error` delivery. Renderer loss and replacement remain nonfatal.
- The host registers its RPC service, sends the post-route `Ready` barrier,
  initializes its dispatcher after the broker ID arrives, and only then
  starts periodic one-way `Heartbeat` messages. Exact `RequesterStopping`
  terminates it normally; asynchronous transport failure terminates it
  abnormally.

The untouched implementation confirms the refinement target statically:

- `RemoteViewModelTestShared.h` declares heartbeat and stopping
  acknowledgements plus a terminal-delivery wait even though the clarified
  protocol needs only `Ready`, `Heartbeat`, and `RequesterStopping`.
- `RemoteViewModelHostingClient` owns a requester lease, sequence numbers, a
  terminal state machine, a failure callback, and cleanup entry points.
- `RemoteViewModelTaskQueueThread` catches task failures, reports them through
  stored state and a callback, then restarts `RunTaskQueue`.
- `RemoteViewModelRequesterSession::Stop` aggregates cleanup errors and
  catches failures around RPC and channel shutdown.
- `RemoteViewModelRoleState` duplicates terminal facts and owns renderer state
  that production Core already owns independently.
- The broker/control lane and RPC broker are split across two local clients,
  and RPC setup is hidden in `OnConnected` callbacks.

Verification for the retained implementation requires:

- Focused unit coverage for exact RVM-host channel classification without a
  renderer-protocol dependency.
- A Debug x64 solution build and the complete `UnitTest` run with no leak
  report.
- Live requester/host smoke coverage for `/Pipe`, `/Http`, and `/MiniHttp`,
  including successful startup/translation, abrupt host loss while idle, and
  normal requester stopping.
- `git diff --check`, no direct edits to generated/import/release folders, and
  a final source audit proving the deleted acknowledgements, reverse lease,
  retry/error-recording queue code, renderer state, dead APIs, and cleanup
  ladders are absent.

Success means the existing externally observable lifetime contract is
unchanged while the runtime exposes only the requester session and small host
client surface, and all source edits remain under `Test`.

# PROPOSALS

- No.1 Replace recoverable supervision with the minimal one-way control protocol [CONFIRMED]

## No.1 Replace recoverable supervision with the minimal one-way control protocol

Keep the current Workflow RPC topology and scheduling because they implement
real behavior: the requester still needs a broker local participant, a
separate requester RPC endpoint, and a task queue that can progress while
service acquisition blocks; the host still needs its main-thread task queue
for incoming `Translate` calls. Remove the recovery and symmetry layered
around that topology.

The broker local participant will also advertise and read
`ViewModelReadyChannel`, folding the third local client into the broker without
mixing the RPC and control readers. The control protocol becomes three exact
JSON strings:

- host to requester: `Ready`;
- host to requester: periodic `Heartbeat`;
- requester to host: `RequesterStopping`.

The requester alone owns a startup/heartbeat lease. It claims host broker
registration under its state lock, performs `RegisterClient` after releasing
the lock, and retains the existing post-route `Ready` barrier. Its state is
reduced to starting/running/stopping, the accepted host and `everAccepted`
fact, broker-registration ownership, lease expiration, and the first
host-loss message/action. Normal stopping is claimed before the one-way stop
send. There is no acknowledgement, sequence, terminal-delivery wait, cleanup
error aggregation, or renderer state.

The host explicitly initializes RpcJson and its control reader after
connection. After the caller registers `rvmt::IViewModel`, the host sends
`Ready`, initializes the dispatcher, and starts a one-purpose heartbeat
thread. Exact requester stopping calls `std::_Exit(0)`. Post-connection
disconnect, read error, fatal local error, a failed heartbeat flush, or an
exception at the heartbeat thread boundary calls `std::_Exit(1)`. Nonfatal
pre-connection local retry notifications remain ignored. Synchronous
startup/RPC failures are uncaught.

The order differs deliberately from the static P1 wording. Workflow
`RpcJsonDispatcherClient::Initialize()` sends cached service declarations and
waits for the broker client ID. The requester cannot send that broker ID until
it receives the post-route `Ready` proof and registers the host. Initializing
before `Ready` therefore deadlocks. Registering the service caches its
declaration, so the working order is connect/lifecycle setup, local service
registration, `Ready`, dispatcher initialization, heartbeat start, and task
queue run.

The requester task-queue thread runs the queue once. An exception at that
thread boundary terminates the test process instead of restarting the queue or
converting the exception into host-disconnection state. Requester cleanup is
straight-line while preserving the required lifetime order: claim stopping
and send the host signal, finalize RPC on the queue, stop the channel server
while callback targets are alive, stop the lease thread, then exit and join
the queue.

Core will classify renderer channels in its own source and retain all renderer
IDs and replacement policy. Shared RVM code will classify only the exact pair
of host channels and will no longer include renderer protocol headers.

P4 upstream Workflow/VlppOS generator and endpoint helpers are not part of
this proposal: they require changes outside the requested `Test` editing
boundary and are not prerequisites for removing GacUI test-app supervision
code. The generic requester scheduler is retained because the generated RPC
surface is not contractually forbidden from gaining callbacks, events, nested
RPC, or requester-owned objects.

### CODE CHANGE

- Simplify `RemoteViewModelTestShared.h`,
  `RemoteViewModelTestRuntime.{h,cpp}`, and `RemotingTest_RvmHost/Main.cpp` to
  the exact one-way control protocol and fail-fast host/queue boundaries.
- Remove the obsolete role-state header and its project references; move the
  minimal requester state and helper implementations into the runtime `.cpp`.
- Update Cpp/Core callers for straight-line requester shutdown and keep
  renderer classification/ownership exclusively in Core.
- Replace obsolete state-machine unit tests with exact host-channel
  classification coverage, then run the required build, unit, source, and live
  process verification.

### CONFIRMED

The Test-scoped implementation applies the proposal:

- `RemoteViewModelTestShared.h` now contains only the exact `Ready`,
  `Heartbeat`, and `RequesterStopping` control strings plus requester-owned
  liveness timings and exact two-channel host classification.
- The broker local participant owns both the RPC and control channels. The
  separate ready local client, acknowledgement protocol, heartbeat sequence,
  reverse lease, terminal state machine, cleanup error aggregation, and
  retrying task-queue worker are gone.
- `RemoteViewModelRoleState.h` and its project entries are deleted. Minimal
  requester state is private to `RemoteViewModelTestRuntime.cpp`; renderer
  classification and replacement state remain exclusively in Core.
- RpcJson and control readers are initialized explicitly in connection paths.
  Broker registration is claimed while holding the state lock and performed
  after releasing it.
- The host and requester queue boundaries are fail-fast, requester stopping is
  claimed before the one-way stop send, and shutdown keeps callback targets
  alive until the channel server stops.
- The public runtime header exposes only the requester session and small
  hosting client surface. The final diff removes 1,913 lines and adds 648
  lines across 12 `Test` files, a net reduction of 1,265 lines.

Verification succeeded:

- Debug x64 `GacUISrc.sln`: build succeeded with 0 warnings and 0 errors.
- Complete Debug x64 `UnitTest`: 89/89 test files and 1,714/1,714 test cases
  passed; no memory-leak dump followed the summary.
- `/Pipe`, `/Http`, and `/MiniHttp` live requester/host runs each acquired the
  service and exposed `Hello, !`, proving a successful `Translate` call.
- Normal automation `!Exit` returned `Queued` and both requester and host
  wrappers exited with code 0 for all three transports.
- For each transport, forcibly terminating an idle host caused
  `CppTest_Rvm` to report `RemotingTest_RvmHost disconnected.` and terminate
  nonzero; no requester or host process remained.
- `git diff --check` passed. A focused source/project audit found no remaining
  role-state header reference, acknowledgement constant, reverse-lease state,
  terminal-state helper, heartbeat sequence, or recovery callback.

All product source edits are under `Test`. The investigation log itself is the
workflow-required exception. The upstream P4 Workflow/VlppOS work was not
attempted because it is outside the requested scope. The larger P0/P5 matrix
for pre-`Ready` and mid-call kills, an active Core renderer's exact fatal
package ordering/replacement behavior, Linux, and GacJS remains outside this
confirmed Windows requester/host refinement run.
