# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Implement the current host-loss and `ViewModelHostClient` refactor specified in `TODO_Task.md`.

Plus check `DebugRemoteProtocolSop.md` to see if `Fatal Error` clicking and force terminating `RemotingTest_RvmHost` test is described in that SOP document or not and make sure it has. Commit and push all local changes on affected repos.

# UPDATES

# TEST [CONFIRMED]

- Confirm the current Workflow dispatcher cannot represent a persistent injected failure and can leave an `OnJsonRequest` or dispatcher startup wait blocked after the accepted RVM host disappears.
- Add deterministic Workflow `LibraryTest` coverage for persistent last-write-wins injection, empty messages, repeated ordinary and `BroadcastAndDrop` calls, wake-up of an in-flight response wait on its caller thread, response-versus-injection ordering, nested request checkpoints, startup waits, and the direct dispatcher implementation.
- Regenerate and verify Workflow reflection baselines, generated RPC code, TypeScript RPC output, and the released `VlppWorkflowLibrary` pair through the repository workflows.
- In GacUI, statically verify that only accepted-host loss outside normal stopping injects `RemotingTest_RvmHost disconnected.` outside server locks, startup loss is latched until the dispatcher exists, `CppTest_Rvm` leaves the exception uncaught, and Core converts `vl::Exception` and `vl::Error` through one shared fatal boundary while its channels are alive.
- Verify `ViewModelHostClient` has no nested `Impl` and preserves the existing connection, initialization, ready-barrier, and shutdown ordering with direct fields.
- Build GacUI in Debug x64 and Win32, regenerate affected reflection metadata through the prescribed projects, and run `UnitTest` with no failures or memory leaks.
- Exercise the two explicit SOP regressions through the native renderer over `/Pipe`: `/RPT` clicking `Fatal Error` must deliver the exact Core-authored `This is a fatel error!`; `/RVMT` must first complete a `Translate`, then force-terminate only `RemotingTest_RvmHost`, trigger a real RPC if idle, deliver exactly one Core-authored `RemotingTest_RvmHost disconnected.`, and terminate Core nonzero without leaving target processes.
- Ensure `DebugRemoteProtocolSop.md` describes both operations, exact observable results, bounded waits, renderer fatal state, and cleanup requirements.

The source baseline confirms the problem. Workflow's `IRpcJsonMessageDispatcher` has no injection API; `RpcJsonDispatcherClient` waits through a message semaphore and separate startup events with no persistent failure predicate. GacUI's accepted-host disconnect path still invokes a terminal callback that broadcasts conditionally and calls `std::_Exit`, and `ViewModelHostClient` still stores all operational state in a nested `Impl`. `RemotingTest_Core` currently has no generic `vl::Exception`/`vl::Error` fatal conversion around requester startup and UI execution. The SOP describes ordinary `/RPT` and `/RVMT` use but does not describe clicking the visible `Fatel Error` button or force-terminating `RemotingTest_RvmHost`.

# PROPOSALS

- No.1 Add persistent dispatcher failure injection and route RVM host loss through the shared Core fatal boundary

## No.1 Add persistent dispatcher failure injection and route RVM host loss through the shared Core fatal boundary

Add `RpcInjectedException` and `IRpcJsonMessageDispatcher::InjectException` in Workflow. Store injected state separately from its message, serialize it with response commitment under a `CriticalSection`, and use a `ConditionVariable` so dispatcher-owned waits can wake and throw on their original caller thread. Keep the state persistent and last-write-wins, and propagate the interface through all implementations, reflection, release output, and documentation.

Import the released Workflow pair through GacUI's normal dependency workflow. Make `RpcServerHelpers` inject accepted-host loss after broker bookkeeping and outside its locks, latching the message until the requester dispatcher exists. Leave `CppTest_Rvm` uncaught. Restore a generic Core `vl::Exception`/`vl::Error` boundary while Core channels remain available so `/RPT` and `/RVMT` use the same single `BroadcastError` conversion. Flatten `ViewModelHostClient::Impl` into its owner without changing protocol ordering. Extend the shared SOP with the two fatal operations and their exact results.

### CODE CHANGE

# RESULT [CONFIRMED WITH TRANSPORT LIMITATIONS]

Implemented the persistent failure path in Workflow. `RpcInjectedException` and
`IRpcJsonMessageDispatcher::InjectException` now provide a persistent,
last-write-wins failure state. `RpcJsonDispatcherClient` uses one
`CriticalSection` and `ConditionVariable` to linearize injection, response
selection, and response commitment; injection wakes response and startup waits,
and all dispatcher-controlled checkpoints rethrow on the original caller
thread. The direct bridge, reflection proxy and registration, generated
reflection baselines, public documentation, and released
`VlppWorkflowLibrary.{h,cpp}` were updated. Deterministic tests cover entry,
empty and replaced messages, repeated and `BroadcastAndDrop` calls, in-flight
wake-up, response races, nested requests, startup waits, and the direct bridge.

Imported the generated Workflow release through GacUI's dependency workflow and
regenerated all affected GacUI metadata. `ViewModelHostClient` now owns its
channel-name map, dispatcher, and ready channel directly; no nested `Impl`
remains. `RpcServerHelpers` claims only the accepted host once, completes broker
bookkeeping, and injects the exact `RemotingTest_RvmHost disconnected.` message
outside its locks. Startup loss is latched until the requester dispatcher is
installed. Normal stopping and renderer loss remain nonfatal. A claimed host
loss skips the otherwise graceful RPC finalization because the dispatcher is
permanently poisoned, preventing Core shutdown from waiting forever; the
existing task queue and its ownership model were not refactored.

Restored the generic Core `vl::Exception`/`vl::Error` boundary and kept one
attempted `BroadcastError` while the remote controller and channels are still
active. `CppTest_Rvm` has no corresponding catch. The renderer claims a received
Core-authored fatal package before its disconnect callback can win the UI-task
race, allowing the user to retain the stopped renderer and expose
`Dom.fatalError`.

Extended `DebugRemoteProtocolSop.md` with two explicit fresh-session tests:
clicking the intentionally spelled `Fatel Error` button in `/RPT`, and
force-terminating only the accepted `RemotingTest_RvmHost` in `/RVMT`. Both
sections specify exact messages, the native `ERROR from GacUI Core` prompt,
choosing No, retained DOM state, ordinary-command rejection, exact `!Exit`,
bounded waits, nonzero requester termination, and cleanup.

## Verification Evidence

- Workflow completed its full Debug Win32/x64 library, compiler, runtime, C++
  RPC, TypeScript, reflection, CodePack/release, and final build workflows.
  The multi-client ChatBot scenario completed nine messages across Tom, Jerry,
  and Spike, then propagated departure and shutdown normally.
- GacUI Debug x64 and Win32 builds completed through
  `.github/Scripts/copilotBuild.ps1` with `0 Warning(s)` and `0 Error(s)`.
- GacUI metadata generation and metadata tests passed for the regenerated x86
  and x64 groups.
- The final GacUI Debug x64 `UnitTest` run passed `88/88` files and
  `1713/1713` cases with no memory-leak dump.
- `/Pipe` `CppTest_Rvm` completed a normal `Translate`, and forced host loss
  followed by a real RPC terminated it through the uncaught injected exception.
- `/Pipe /RPT`, using the actual native renderer input path, produced the exact
  `ERROR from GacUI Core` prompt containing `This is a fatel error!`; choosing No
  retained that exact `Dom.fatalError`.
- `/Pipe /RVMT`, using the actual native renderer input path, converged Core and
  renderer at `Hello, OK!`; after force-terminating the exact accepted host PID,
  the next keystroke produced the exact Core-authored
  `RemotingTest_RvmHost disconnected.` prompt, Core stopped promptly, and
  choosing No retained the exact fatal DOM and `[STOPPED] Remote View Model Test`
  title.

## Explicitly Unverified or Blocked Matrix Cells

- `/MiniHttp /RVMT` completed startup and a real normal `Translate` with Core
  and renderer converged at `Hello, MINI!`. After a hard host termination, the
  next real RPC remained blocked because the underlying
  `SocketHttpServerConnection` did not report `OnDisconnected`: its documented
  no-ack transport rule requeues a failed long-poll response so a replacement
  poll can receive the same message. The VlppOS learning explicitly requires
  keeping that production rule because treating a locally failed response as a
  disconnect can duplicate messages. No heartbeat, polling, reverse disconnect
  endpoint, or transport semantic change was added.
- `/Http /RVMT` completed requester, host, and renderer startup. Synthetic
  native input used by this investigation blocked in the pre-existing remote
  protocol `BatchWrite` path before reaching the RVM call, so the forced-loss
  cell was not claimed as verified.
- The retained renderer's exact `!Exit` automation command is not currently
  implemented by `AutomationServiceRenderer`; the endpoint remains blocked or
  empty after fatal retention. This predates the current changes. Exact renderer
  PIDs were force-cleaned during testing, while the SOP records the intended
  `!Exit` contract.
- Linux and macOS `/MiniHttp` application flows were not run and are explicitly
  unverified.
