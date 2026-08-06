Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Terminating `RemotingTest_RvmHost` process manually, should crash `CppTest_Rvm`, and create a fatal error in `RemotingTest_Core`, no recovery needed.
  - The current behavior is the UI hangs when `RemotingTest_RvmHost` is forced terminated.
  - This should generates a disconnection signal in `CppTest_Rvm` or `RemotingTest_Core`, where is a good place to trigger.
  - A task to throw an exception could be injected using `InvokeInMainThread`, to make it thrown in UI thread, to make fatal error generating much easier. In `CppTest_Rvm` the exception doesn't need to catch, it just crashes the app as expected.
- No `ViewModelHostClient::Impl` is needed, just merge the class into `ViewModelHostClient` directly.

## DETAILS

### Host-loss behavior

- `RpcServerHelpers::OnClientDisconnected` remains the accepted-host ownership boundary. Claim host loss exactly once and trigger the terminal action only for that host while the requester is not in `RequesterPhase::Stopping`; normal requester shutdown and renderer loss must not become fatal host-loss reports.
- Treat a host termination as fatal when the channel/transport actually observes it. Do not add heartbeat, polling, retry, recovery, a reverse disconnect endpoint, or a shutdown acknowledgement. `/Pipe` can report broken-pipe loss immediately; an idle `/Http` or `/MiniHttp` session is not required to detect peer loss proactively.
- Keep the accepted-host disconnect callback capable of terminating the requester while the UI thread is blocked in a synchronous `RpcJsonDispatcherClient::OnJsonRequest`. Portable C++ cannot inject an exception into that blocked stack, so the terminal path must not wait for an `InvokeInMainThread` task to execute.
- `CppTest_Rvm` must terminate nonzero directly from the terminal callback without recovery. `RemotingTest_Core` must first attempt exactly one Core-authored `BroadcastError(RemoteViewModelHostDisconnectedError)` and then terminate nonzero even if delivery throws, so the active renderer receives `RemotingTest_RvmHost disconnected.` when delivery succeeds instead of seeing only a transport closure.
- Do not repurpose the RPC message semaphore as a disconnection signal: waking it without a corresponding message violates the receive-queue invariant and is not a safe way to throw from `OnJsonRequest`.

### `ViewModelHostClient` flattening

- Remove the nested `ViewModelHostClient::Impl` declaration and the owning `Ptr<Impl>`.
- Move the channel-name map, `RemoteViewModelJsonDispatcherClient`, and ready control-channel pointer directly into `ViewModelHostClient`.
- Move the existing initialization, `Connect`, and `SendReady` logic into `ViewModelHostClient` itself. Update diagnostic prefixes accordingly and preserve the current channel registration, generated-RPC initialization, ready-message ordering, dispatcher access, and shutdown behavior.

### Scope

- Keep fixed RVM constants and generated-RPC initialization in `ViewModelShared.*`; keep all helper code test-only and outside `Release` CodePack.
- No new source files, project inventories, generated files, reflection registrations, protocol schemas, `Import` files, or `Release` files are expected.

## VERIFICATION

### Static and build verification

- Run `git diff --check`.
- Confirm there is no declaration, definition, pointer, or member access through `ViewModelHostClient::Impl`. Its channel-name map, dispatcher, and ready control-channel pointer must be direct `ViewModelHostClient` fields used directly by `Connect`, `SendReady`, and `GetDispatcher`.
- Confirm `RpcServerHelpers::OnClientDisconnected` claims only the accepted host, suppresses the terminal action during `RequesterPhase::Stopping`, and cannot report renderer or normal requester shutdown as host loss.
- Confirm the host-loss terminal path does not depend solely on a queued UI task. `CppTest_Rvm` must terminate nonzero directly; Core must attempt exactly one `BroadcastError(RemoteViewModelHostDisconnectedError)` before terminating nonzero even if broadcasting fails.
- Confirm no heartbeat, polling, retry, recovery, reverse disconnect endpoint, shutdown acknowledgement, or fake RPC receive message was introduced.
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
  - In one representative Core session, replace the renderer and require the Core and host to remain usable; renderer loss is nonfatal and must not be confused with host loss.
- Forced-host-loss session:
  - First complete one successful `Translate`, then force-terminate only the accepted host.
  - `/Pipe` should expose the loss directly. `/Http` and `/MiniHttp` need not detect an idle peer proactively; exercise the next real RPC operation when required by the resolved host-loss design. Bound every wait and treat a frozen UI as failure.
  - In at least one session per topology, force host loss while a second `Translate` is waiting for its response. Require bounded process termination rather than a permanently blocked `OnJsonRequest`.
  - Require `CppTest_Rvm` to terminate nonzero without retry or recovery.
  - Require Core to deliver exactly one Core-authored `RemotingTest_RvmHost disconnected.` error to the renderer before Core terminates nonzero. A local renderer transport error alone is not sufficient. Retain the renderer once to verify `Dom.fatalError`, then close it with exact `!Exit`.
  - Confirm no requester, host, renderer, listener, or native crash dialog is left behind.

- Run the portable Core + host `/MiniHttp` flow on Linux and macOS before claiming those platforms verified; otherwise record them explicitly as unverified.

## REVIEW COMMENTS

### QUEUED UI FAILURE CANNOT INTERRUPT A BLOCKED RPC [CLOSED]

**review comment**: `RpcJsonDispatcherClient::OnJsonRequest` waits synchronously for a matching response. If host loss is discovered while `Translate` is blocked, an exception queued through `InvokeInMainThread` cannot execute because the same UI thread is waiting. Idle `/Http` and `/MiniHttp` loss is intentionally not detected through heartbeat or polling, so the next real RPC operation is also the most likely place to expose the loss. The proposed queued exception alone therefore does not guarantee the requested no-hang behavior.

**suggested solution**: Decide whether the requirement covers only a host-loss callback observed while the UI loop is free. If host loss during an in-flight RPC must terminate reliably, add an upstream `RpcJsonDispatcherClient` terminal/cancellation notification that records the failure, wakes pending message waits, and makes the blocked `OnJsonRequest` throw; implement and release that change from the owning Workflow repository before importing it here. Preserve the immediate callback-side terminal action until an equivalent unblock mechanism exists.

Good catch, is it possible to just crash the blocking `OnJsonRequest` in this case?

**review comment**: Yes, if "crash" means terminating the requester process. The accepted-host disconnect callback runs independently of the UI thread and can terminate the process while that thread is blocked. Portable C++ cannot instead inject an exception into the blocked `OnJsonRequest` call stack, and releasing its shared message semaphore without enqueuing a real message would only break the receive-queue invariant for an arbitrary waiter.

**suggested solution**: Keep the fatal action in `RpcServerHelpers::OnClientDisconnected`. For Core, attempt the single Core-authored `BroadcastError(RemoteViewModelHostDisconnectedError)` and then call `std::_Exit(1)`; for `CppTest_Rvm`, call `std::_Exit(1)` directly. Do not require an upstream dispatcher change unless unwinding the blocked RPC stack, rather than terminating the process, becomes an explicit requirement. Verify this with host loss during an in-flight `Translate`.

I don't like the `Exit` solution and still want `OnJsonRequest` to crash directly. My idea is that, `OnJsonRequest` is just waiting on a response, if we add a `InjectExceptionToJsonRequest` function to this class (actually the interface `IRpcJsonMessageDispatcher` and all implementations) in `Workflow` repo, do you think it is doable? To make the API well defined, we can call `InjectException(WString message)`, and then `OnJsonRequest` just thrown an `RpcInjectedException` with the message. And if `CppTest_Rvm` doesn't catch it, it just crashes in the UI thread. And for `RemotingTest_Core`, any UI thread exception is already converted to a fatal error, so nothing additional needs to do.

The behavior of `InjectExceptionToJsonRequest` will be, if `OnJsonRequest` is already blocking, it throws an exception immediateky. If no `OnJsonRequest` is blocking, the next call will just crash, in this case, `InjectExceptionToJsonRequest` calls will override already injected messages. And the message will stay forever, that's said, crashing `OnJsonRequest` does not remove the injected exception, all following `OnJsonRequest` calls will keep crashing.