Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Terminating `RemotingTest_RvmHost` process manually, should crash `CppTest_Rvm`, and create a fatal error in `RemotingTest_Core`, no recovery needed.
  - The current behavior is the UI hangs when `RemotingTest_RvmHost` is forced terminated.
  - This should generates a disconnection signal in `CppTest_Rvm` or `RemotingTest_Core`, where is a good place to trigger.
  - A task to throw an exception could be injected using `InvokeInMainThread`, to make it thrown in UI thread, to make fatal error generating much easier. In `CppTest_Rvm` the exception doesn't need to catch, it just crashes the app as expected.
- Remove the dependency to `TaskQueue`:
  - Do not use `RpcJsonDispatcherClientForTaskQueue`, use `RpcJsonDispatcherClient` instead, as `RemoteViewModelJsonDispatcherClient`'s base class.
  - For `CppTest_Rvm` and `RemotingTest_Core`:
    - DO not use `TaskQueue`, delete `TaskQueueThread`.
    - The `ScheduleTask` function will be override in `GuiMain.cpp` files, twice.
    - `GetCurrentController()->AsyncService()->InvokeInMainThread` could be modeled as a task queue, just inherit from `RpcJsonDispatcherClient` and use this function instead, therefore no `TaskQueue` will be needed.
  - For `RemotingTest_RvmHost`:
    - The `ScheduleTask` function will be override in `Main.cpp` to use `TaskQueue`.
    - Unfortunately the `RpcJsonDispatcherClientForTaskQueue` class cannot be used here.
  - Since new sub classes of `RemoteViewModelJsonDispatcherClient` is created in each test app, they need to be passed to `RemoteViewModelChannelServer` and `ViewModelHostClient`, instead of being created in these two classes internally.
- No `ViewModelHostClient::Impl` is needed, just merge the class into `ViewModelHostClient` directly.

## DETAILS

### Dispatcher ownership

- Change `RemoteViewModelJsonDispatcherClient` to derive directly from `rpc_controller::channeling::RpcJsonDispatcherClient`. It continues to own only the generated `RemoteViewModelTest` RPC initialization and remains abstract until an application supplies `ScheduleTask`.
- Remove the `TaskQueue` and `RpcJsonDispatcherClientForTaskQueue` aliases from `ViewModelShared.h`; the shared RVM helpers must not depend on either task-queue adapter.
- Define one concrete `RemoteViewModelJsonDispatcherClient` subclass at each application composition boundary:
  - `CppTest_Rvm/GuiMain.cpp` and `RemotingTest_Core/GuiMain.cpp` override `ScheduleTask` with `GetCurrentController()->AsyncService()->InvokeInMainThread(nullptr, task)`.
  - `RemotingTest_RvmHost/Main.cpp` owns the only remaining `TaskQueue` and its override calls `QueueTask`.
- Construct the concrete dispatcher before the corresponding channel helper and pass it as an owning `Ptr<RemoteViewModelJsonDispatcherClient>`:
  - `RemoteViewModelChannelServer` / `RpcServerHelpers` retain the requester dispatcher instead of creating one in `RpcServiceAccessLocalClient`.
  - `ViewModelHostClient` retains the host dispatcher instead of creating one internally.
- Flatten `ViewModelHostClient::Impl` into `ViewModelHostClient`. Move its channel-name map, injected dispatcher, and ready control-channel pointer directly into the class, and update `Connect`, `SendReady`, and dispatcher access accordingly.

### Complete `TaskQueue` removal on the requester side

- Removing only `RpcJsonDispatcherClientForTaskQueue` is insufficient. `RpcBroadcastingLocalClient` currently owns `RpcJsonDispatcherServerForTaskQueue`, and `RpcJsonDispatcherServer::RegisterClient` also calls `ScheduleTask` to deliver login messages.
- Replace the broker task-queue adapter with an explicit non-`TaskQueue` scheduling policy. It must be usable during the exact `Ready` / service-acquisition startup sequence, before a GacUI controller exists, and it must preserve registration-versus-disconnection ordering without holding `lockBroker` across a potentially reentrant send.
- Delete `TaskQueueThread`, requester-side `TaskQueue` storage, `FinalizeRpcOnTaskQueue`, queue exit/join logic, and all `RpcJsonDispatcher*ForTaskQueue` references from `Test/RemotingHelpers/Rvmt`. `TaskQueue` remains application-owned only in `RemotingTest_RvmHost/Main.cpp`.
- Preserve the existing startup invariants: the Core local renderer-protocol client remains client ID 0; the host is reserved only after its exact two-channel admission and exact `Ready` message; the requester and broker identities are initialized before service acquisition completes; renderer admission remains closed until `BeginRunning` succeeds.

### Host-loss behavior

- `RpcServerHelpers::OnClientDisconnected` remains the accepted-host ownership boundary. Trigger the terminal action only for that host while the requester is not in `RequesterPhase::Stopping`; normal requester shutdown must not become a fatal host-loss report.
- Treat a host termination as fatal when the channel/transport actually observes it. Do not add heartbeat, polling, retry, recovery, a reverse disconnect endpoint, or a shutdown acknowledgement. `/Pipe` can report broken-pipe loss immediately; an idle `/Http` or `/MiniHttp` session is not required to detect peer loss proactively.
- `CppTest_Rvm` must terminate nonzero without recovery. `RemotingTest_Core` must first send exactly one Core-authored `BroadcastError(RemoteViewModelHostDisconnectedError)` and then terminate nonzero, so the active renderer receives `RemotingTest_RvmHost disconnected.` instead of seeing only a transport closure.
- A queued `InvokeInMainThread` exception must not be the only terminal mechanism. It cannot run while the UI thread is blocked in a synchronous RPC wait, and a bare exception from `SetupRemoteNativeController` does not create a Core-authored `!Error`.

### Scheduling lifetime and shutdown

- The two GacUI dispatcher overrides may queue work only while `GetCurrentController()` and its async service are alive. Establish an explicit RVM finalization/detach-and-drain boundary before destroying either injected dispatcher; no queued callback may retain a dead dispatcher.
- Do not call `InvokeInMainThreadAndWait` from the UI thread.
- In `RemotingTest_Core`, do not stop the entire shared channel server prematurely: `GuiRemoteController::Finalize` still needs it to deliver normal `ControllerConnectionStopped`. Split RVM dispatcher finalization from the later base transport stop, or provide an equivalent drain boundary that preserves normal renderer shutdown.
- Keep fixed RVM constants and generated-RPC initialization in `ViewModelShared.*`; keep all helper code test-only and outside `Release` CodePack.
- Reconcile `Project.md` and `.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md` with the injected dispatcher ownership and removal of requester-side task queues. The generic Workflow documentation may continue to describe `RpcJsonDispatcherClientForTaskQueue` as an available API.
- No new source files, project inventories, generated files, reflection registrations, protocol schemas, `Import` files, or `Release` files are expected.

## VERIFICATION

### Static and build verification

- Run `git diff --check`.
- Confirm there is no `RpcJsonDispatcherClientForTaskQueue`, `RpcJsonDispatcherServerForTaskQueue`, `TaskQueueThread`, helper-owned `TaskQueue`, or `ViewModelHostClient::Impl` in `Test/RemotingHelpers/Rvmt`.
- Confirm exactly one application-specific dispatcher subclass exists in each of the two `GuiMain.cpp` files and `RemotingTest_RvmHost/Main.cpp`; both helper entry points retain the injected dispatcher and do not construct it internally.
- Confirm `TaskQueue` remains only in `RemotingTest_RvmHost/Main.cpp`, and both GacUI overrides use `InvokeInMainThread`, not `InvokeInMainThreadAndWait`.
- Confirm the exact `Ready` barrier, singleton-host admission, requester/broker separation, renderer-admission gate, stopping suppression, and exact disconnect error text remain present.
- Build `Test/GacUISrc/GacUISrc.sln` through `.github/Scripts/copilotBuild.ps1` in `Debug|x64` and `Debug|Win32`; require `0 Warning(s)` and `0 Error(s)` in `Build.log` for each build.
- Run the existing `UnitTest` project in `Debug|x64` through `.github/Scripts/copilotExecute.ps1`; require all selected tests to pass and no memory-leak dump. No new unit-test file or code generation is required for this test-helper refactor.

### Windows application matrix

Use fresh processes and matching transport arguments for every cell:

| Topology | `/Pipe` | `/Http` | `/MiniHttp` |
| --- | --- | --- | --- |
| `CppTest_Rvm` + `RemotingTest_RvmHost` | Required | Required | Required |
| `RemotingTest_Core /RVMT` + `RemotingTest_RvmHost` + `RemotingTest_Rendering_Win32` | Required | Required | Required |

For each topology and transport, run one normal session and one forced-host-loss session.

- Normal session:
  - Start the requester before the host. For Core, start the renderer only after Core `Controls` exposes `Remote View Model Test`.
  - Require the exact initial greeting `Hello, !`; perform a real `Translate` and require exactly `Hello, <marker>!`. For Core, require Core `Controls` and renderer `Dom` to converge.
  - Close through the application UI/automation path. Require no false host-loss fatal error, no surviving process or listener, and an immediate clean restart.
  - In at least one Core session per transport, replace the renderer and require the Core and host to remain usable; renderer loss is nonfatal and must not be confused with host loss.
- Forced-host-loss session:
  - First complete one successful `Translate`, then force-terminate only the accepted host.
  - `/Pipe` should expose the loss directly. `/Http` and `/MiniHttp` need not detect an idle peer proactively; exercise the next real RPC operation when required by the resolved host-loss design. Bound every wait and treat a frozen UI as failure.
  - Require `CppTest_Rvm` to terminate nonzero without retry or recovery.
  - Require Core to deliver exactly one Core-authored `RemotingTest_RvmHost disconnected.` error to the renderer before Core terminates nonzero. A local renderer transport error alone is not sufficient. Retain the renderer once to verify `Dom.fatalError`, then close it with exact `!Exit`.
  - Confirm no requester, host, renderer, listener, or native crash dialog is left behind.

- A successful `Translate` exercises the host-side `TaskQueue` scheduler but does not normally invoke the new requester-side `ScheduleTask` overrides, because `rvmt::IViewModel` has no unsolicited callback. Keep the structural checks for both GacUI overrides instead of claiming runtime thread-affinity coverage from the happy path.
- Run the portable Core + host `/MiniHttp` flow on Linux and macOS before claiming those platforms verified; otherwise record them explicitly as unverified.

## REVIEW COMMENTS

### QUEUED UI FAILURE CANNOT INTERRUPT A BLOCKED RPC

**review comment**: `RpcJsonDispatcherClient::OnJsonRequest` waits synchronously for a matching response. If host loss is discovered while `Translate` is blocked, an exception queued through `InvokeInMainThread` cannot execute because the same UI thread is waiting. Idle `/Http` and `/MiniHttp` loss is intentionally not detected through heartbeat or polling, so the next real RPC operation is also the most likely place to expose the loss. The proposed queued exception alone therefore does not guarantee the requested no-hang behavior.

**suggested solution**: Decide whether the requirement covers only a host-loss callback observed while the UI loop is free. If host loss during an in-flight RPC must terminate reliably, add an upstream `RpcJsonDispatcherClient` terminal/cancellation notification that records the failure, wakes pending message waits, and makes the blocked `OnJsonRequest` throw; implement and release that change from the owning Workflow repository before importing it here. Preserve the immediate callback-side terminal action until an equivalent unblock mechanism exists.

### BROKER SCHEDULING AND UI SCHEDULING HAVE DIFFERENT LIFETIMES

**review comment**: The requester broker must schedule login delivery before GacUI controller setup, while the new requester dispatcher is intended to use the controller's async service. Simply moving the broker to `InvokeInMainThread` cannot satisfy startup. Simply executing broker tasks inline while holding the current `lockBroker` can reenter `OnClientDisconnected` during a failed send and deadlock. The deleted task queue also used to drain work before dispatcher destruction, while Core must keep the shared transport alive long enough to send normal `ControllerConnectionStopped`.

**suggested solution**: Give the broker a separate pre-GUI scheduler. If its `ScheduleTask` executes inline, move `RegisterClient` outside `lockBroker`, use a post-registration host-state recheck with idempotent `DisconnectClient` to close the registration/disconnection race, and never hold the broker lock across channel sends. Separately add an RVM-only finalization/detach-and-drain boundary while the GacUI scheduler is still alive; let the base channel transport stop only after controller finalization.
