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

### Injected RPC failure revision

This revision supersedes the callback-side process-exit design and the no-generated-or-imported-files expectation above. Keep the earlier text as review history, but do not implement `std::_Exit` for host loss.

- Add `RpcInjectedException : vl::Exception` and `IRpcJsonMessageDispatcher::InjectException(const WString& message)` in the Workflow repository. The method must be safe to call concurrently with `OnJsonRequest`.
- Injection is persistent and last-write-wins. An empty string is a valid injected message, so absence must be represented separately. Throwing does not consume or clear the state; every later `OnJsonRequest`, including `BroadcastAndDrop`, must throw `RpcInjectedException` before transmitting anything. A later injection replaces the message only for calls that have not observed and thrown an earlier value.
- For an `OnJsonRequest` blocked in the dispatcher's own response wait, `InjectException` must wake the wait and make that call throw on its caller thread. Linearize injection, response selection, and response commitment under the same lock: if injection commits first, it wins; if a matching response commits first, that invocation may return normally while the persistent injection poisons all remaining and future calls.
- "Immediately" means at the next dispatcher-controlled checkpoint. The API cannot asynchronously throw through a transport send, arbitrary user code, or an implementation that is blocked outside a dispatcher-owned wait. Preserve the existing supported `OnJsonRequest` execution model, including same-thread nested RPC; this task does not add support for concurrent top-level callers.
- In `RpcJsonDispatcherClient`, replace the receive queue's `SpinLock` plus message `Semaphore` with a `CriticalSection` plus `ConditionVariable` guarding received messages, buffered responses, and the injected state. Wait in a predicate loop, test injection before messages, and call `WakeAllPendings` after storing a new injected value. Check the state at request entry, throughout the response loop, after processing a nested incoming request, and immediately before committing a matching response. Never synthesize a receive message or release the old semaphore as a terminal signal.
- The persistent state must also release and be checked by dispatcher-owned startup waits such as `WaitForExpectedServices` and `WaitForServerClientId`; otherwise accepted-host loss during `RequesterPhase::Starting` can still hang before the first ordinary `OnJsonRequest`. If GacUI observes host loss before `requesterDispatcher` is installed, latch the message in `RpcServerHelpers` and inject it as soon as the dispatcher becomes available.
- Implement the pure virtual method in every Workflow implementation. The channel client owns the wakeable wait. A synchronous/direct bridge checks the persistent state before dispatch and again at its next dispatcher-controlled checkpoint; it is not required to interrupt arbitrary code already executing. Update the reflected interface proxy and method registration. `RpcInjectedException` itself does not need reflection exposure.
- After `RpcServerHelpers::OnClientDisconnected` has atomically claimed the accepted host and completed broker-disconnect bookkeeping, invoke `requesterDispatcher->InjectException(RemoteViewModelHostDisconnectedError)` outside `lockState` and `lockBroker`. Normal shutdown, renderer loss, and duplicate host notifications must not inject. `CppTest_Rvm` adds no catch, so the exception remains unhandled on the UI thread.
- `RemotingTest_Core` does not currently convert arbitrary UI-thread exceptions into a remote fatal error. Add a catch boundary around `SetupRemoteNativeController` while `channelServer` and its Core channels are still alive. Catch `RpcInjectedException`, attempt exactly one `BroadcastError(ex.Message())`, and then propagate the failure or return nonzero without recovery even if delivery fails. Do not broadcast from the disconnect callback; the UI-thread catch is the conversion point.
- Persistent injection deliberately does not terminate an idle requester immediately. After the transport reports host loss, an idle requester fails on its next `OnJsonRequest`; an in-flight response wait fails as soon as it observes the injected state. This applies to every transport, while idle `/Http` and `/MiniHttp` can additionally defer injection until a real operation exposes the peer loss.
- Treat this as an intentional source/ABI-breaking cross-repository public API change for custom `IRpcJsonMessageDispatcher` implementations. Update Workflow source, tests, reflection proxy/registration, reflection baselines, RPC knowledge, and public custom-dispatcher documentation; regenerate `Workflow/Release/VlppWorkflowLibrary.{h,cpp}` with CodePack. Then run the GacUI import workflow to copy the released pair into `GacUI/Import` without hand-editing it, and regenerate affected GacUI reflection metadata. GacUI `Release` still does not acquire a second Workflow copy.

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

### Injected-exception revision verification

The checks in this subsection supersede the callback-side termination, fake-semaphore, no-code-generation, and no-import assertions above.

- In Workflow `LibraryTest`, use deterministic events/barriers rather than sleeps to verify:
  - Injection before a request throws the exact `RpcInjectedException::Message()`, sends no request, and remains effective for repeated calls and `BroadcastAndDrop`.
  - Injecting `A` and then `B` makes calls that have not yet observed the state throw `B`; include an empty-string injection to prove that presence is tracked separately from the message value.
  - One request blocked after its send wakes within a bound and throws on its original caller thread. Same-thread nested request processing rechecks the persistent state so an injected failure is not converted into an ordinary serialized RPC failure.
  - Response-versus-injection races follow the documented lock ordering, never assert on an empty queue, never lose a wake-up, and leave all future requests poisoned.
  - Dispatcher startup waits wake and throw when injection occurs before expected services or the server client id arrives. The direct/synchronous bridge observes injection at its documented checkpoints.
- Regenerate and verify Workflow's `Reflection32.txt`, `Reflection64.txt`, `ReflectionCppTypes32.txt`, and `ReflectionCppTypes64.txt` baselines for the new `IRpcJsonMessageDispatcher` method, then run the complete Debug Win32/x64 build, library/compiler/runtime/Cpp RPC tests, TypeScript build, and final Workflow CodePack/build sequence required by `Workflow/Project.md`. Exercise the multi-client ChatBot RPC scenario because dispatcher waiting semantics changed.
- Confirm the Workflow release output contains the API and implementation, import it through the normal GacUI build/import workflow, and regenerate the affected `ReflectionCore32`, `ReflectionCore64`, `Reflection32`, and `Reflection64` binary and text metadata groups. Do not hand-edit `Import/VlppWorkflowLibrary.h`, `Import/VlppWorkflowLibrary.cpp`, release output, or generated metadata.
- In GacUI static review, confirm accepted-host loss is claimed once, startup loss is latched when necessary, injection occurs outside helper locks after broker bookkeeping, and no host-loss path calls `std::_Exit` or fabricates an RPC message. Confirm Core catches the injected exception while Core channels are alive, attempts the exact fatal broadcast once, and still finishes nonzero if broadcasting throws; confirm `CppTest_Rvm` does not catch it.
- Run the existing GacUI Debug x64/Win32 builds, metadata generation/tests, and `UnitTest`, followed by the full normal and forced-loss application matrix above. For every transport, cover both an in-flight `Translate` and an idle requester followed by the next real `Translate`. The former must unblock within a bound; the latter is the specified trigger for persistent injection. Core must deliver exactly one Core-authored `RemotingTest_RvmHost disconnected.` fatal error before stopping, while `CppTest_Rvm` must terminate nonzero without recovery.
- In one representative transport, terminate the accepted host while the requester is still waiting for dispatcher initialization and require bounded failure rather than a startup hang. Continue to require no false fatal error during normal stop or renderer replacement and no surviving process, listener, or native crash dialog.

## REVIEW COMMENTS

### QUEUED UI FAILURE CANNOT INTERRUPT A BLOCKED RPC [CLOSED]

**review comment**: `RpcJsonDispatcherClient::OnJsonRequest` waits synchronously for a matching response. If host loss is discovered while `Translate` is blocked, an exception queued through `InvokeInMainThread` cannot execute because the same UI thread is waiting. Idle `/Http` and `/MiniHttp` loss is intentionally not detected through heartbeat or polling, so the next real RPC operation is also the most likely place to expose the loss. The proposed queued exception alone therefore does not guarantee the requested no-hang behavior.

**suggested solution**: Decide whether the requirement covers only a host-loss callback observed while the UI loop is free. If host loss during an in-flight RPC must terminate reliably, add an upstream `RpcJsonDispatcherClient` terminal/cancellation notification that records the failure, wakes pending message waits, and makes the blocked `OnJsonRequest` throw; implement and release that change from the owning Workflow repository before importing it here. Preserve the immediate callback-side terminal action until an equivalent unblock mechanism exists.

Good catch, is it possible to just crash the blocking `OnJsonRequest` in this case?

**review comment**: Yes, if "crash" means terminating the requester process. The accepted-host disconnect callback runs independently of the UI thread and can terminate the process while that thread is blocked. Portable C++ cannot instead inject an exception into the blocked `OnJsonRequest` call stack, and releasing its shared message semaphore without enqueuing a real message would only break the receive-queue invariant for an arbitrary waiter.

**suggested solution**: Keep the fatal action in `RpcServerHelpers::OnClientDisconnected`. For Core, attempt the single Core-authored `BroadcastError(RemoteViewModelHostDisconnectedError)` and then call `std::_Exit(1)`; for `CppTest_Rvm`, call `std::_Exit(1)` directly. Do not require an upstream dispatcher change unless unwinding the blocked RPC stack, rather than terminating the process, becomes an explicit requirement. Verify this with host loss during an in-flight `Translate`.

I don't like the `Exit` solution and still want `OnJsonRequest` to crash directly. My idea is that, `OnJsonRequest` is just waiting on a response, if we add a `InjectExceptionToJsonRequest` function to this class (actually the interface `IRpcJsonMessageDispatcher` and all implementations) in `Workflow` repo, do you think it is doable? To make the API well defined, we can call `InjectException(WString message)`, and then `OnJsonRequest` just thrown an `RpcInjectedException` with the message. And if `CppTest_Rvm` doesn't catch it, it just crashes in the UI thread. And for `RemotingTest_Core`, any UI thread exception is already converted to a fatal error, so nothing additional needs to do.

The behavior of `InjectExceptionToJsonRequest` will be, if `OnJsonRequest` is already blocking, it throws an exception immediateky. If no `OnJsonRequest` is blocking, the next call will just crash, in this case, `InjectExceptionToJsonRequest` calls will override already injected messages. And the message will stay forever, that's said, crashing `OnJsonRequest` does not remove the injected exception, all following `OnJsonRequest` calls will keep crashing.

### INJECTED RPC FAILURE IS FEASIBLE, BUT CORE NEEDS A FATAL-ERROR BOUNDARY

**review comment**: Yes, the persistent `InjectException` design is doable for `RpcJsonDispatcherClient` and is preferable to `std::_Exit`. It needs a predicate-based terminal wake rather than an unmatched semaphore release, and the interface-level promise must be limited to dispatcher-controlled checkpoints because a C++ API cannot asynchronously throw through arbitrary code in every implementation. The reply's idle behavior is also clear: with no active request, failure occurs on the next `OnJsonRequest`, not at disconnect time. One assertion does not match the current tree, however: `RemotingTest_Core` has no general UI-exception-to-fatal-error conversion between `SetupRemoteNativeController`, `GuiApplicationMain`, the remote event loop, and `main`. An unhandled `RpcInjectedException` would stop Core, but the renderer would see transport closure rather than the required Core-authored fatal message. The dispatcher also has startup waits outside `OnJsonRequest`, which must observe injection to avoid a loss-before-initialization hang.

**suggested solution**: Implement the persistent upstream API, synchronization, startup-wait checks, reflection propagation, tests, release, and GacUI import described above. In GacUI, inject only after the accepted-host guard claims loss. Leave `CppTest_Rvm` uncaught, but add an explicit `RpcInjectedException` catch around Core's `SetupRemoteNativeController` call while the Core channels are alive; broadcast the exception message exactly once and then fail nonzero without recovery. This preserves the requested UI-thread unwinding and gives the renderer the exact fatal error without callback-side process exit.

For `RemotingTest_Core`, if you take a look at `/RPT` you will find a fatal error button just throw an exception. That's verified to be able to convert to a fatal error. `OnJsonRequest` is called on the UI thread, so when `RemotingTest_RvmHost` is closed, typing anything to the text box triggers a call to `IViewModel` and at this moment an exception should raise in the UI thread, converting to fatal error using the same path, isn't it?