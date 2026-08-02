# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the following refactoring inside `Source/RemotingHelpers` and fix test apps accordingly:
- Add `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` to index the whole `Source/RemotingHelpers`:
  - Use them in all test apps under `GacUISrc` that need them, instead of adding actual source files to each project file.
  - No need to care if any source file is not used in any specific test app.
  - If `UnitTest` has any test cases testing against `Source/RemotingHelpers`, just delete them.
- Keep automation service API simple:
  - In `WindowsAutomationService.Windows.(h|cpp)` remove `WindowsAutomationServiceType` and `*Scope`.
  - In `MiniHttpAutomationService.(h|cpp)` remove `*Scope`.
  - In `RemotingTest_Core`, remove `NativeAutomationServiceScope`, we can just call `Unsubstitute` directly. Because when `GuiMain` crashes, nothing is going to recover so ensuring such `Unsubstitute` call is just making the code complex without gaining any benefit.
    - The same rule applies to all test apps under `GacUISrc` solution.
    - Just like `Unsubstitute`, `Stop(Windows|Mini)HttpAutomationService` does not need protection, as the HTTP service will automatically stop after the app has crashed.
  - Everything in `AutomationServiceHost` is not useful either; delete it.
  - The reason is that, only the test app knows what actual service to create. For example, for a trivial GacUI application, there are:
    - `WindowsAutomationService` for Windows
    - `WGacAutomationService` for Linux, and the source code is not in this repo.
    - `CocoaAutomationService` for macOS, and the source code is not in this repo.
    - And there is a `RemoteProtocolAutomationService` for GacUI remote protocol core application.
    - So every test app is going to create its own automation service as a value type, perform substitution, and then start either WindowsHttp or MiniHttp to expose the automation service via HTTP.
    - Since every test app is going to decide which automation service to substitute and which HTTP service to start, any "guiding code" in `Source/RemotingHelpers` is not useful. Just call specific functions in test apps directly.
  - `../Tools/Copilot/Guidelines/Running-GacUI.md` needs to be updated to say, different test apps under each mode/platform need to start different `*AutomationService` with different Start/Stop functions, but all of them work in the same way.
- Fix `Test/Linux` test apps' `vmake` accordingly, but there is no way to test them on Windows; the testing part will be done separately.
- `CppTest_Rvm`:
  - Remove `Main.Linux.cpp` and `Test/Linux/CppTest_Rvm`, as this cannot be done in the `GacUI` repo.
  - In `GuiMain`, when `/MiniHttp` is used, it should call `StartMiniHttpAutomationService`:
    - `StartMiniHttpAutomationService` is not Linux/macOS specific, it is designed to pair with `/MiniHttp` in Windows/Linux/macOS.
    - The `Start(NamedPipe|Http|MiniHttp)Server` functions are too similar; just extract most of the code into a template function, and keep very simple `Start(NamedPipe|Http|MiniHttp)Server` functions that call it, such as `StartServer<named_pipe::NamedPipeServer>`:
      - Use `SetupHostedWindowsDirect2DRenderer` directly in `StartServer` first, this part will be fixed when Linux/macOS starts to run it.
- `RemotingTest_RvmHost`:
  - No need to split `main` into `Transport` and `ParseTransport`, we can inline `ParseTransport` in `main`. And when it is not Windows, just don't try to see if the argument is `/Http` or `/Pipe`, no need to create error messages to tell that they are Windows only -- as this is a test app, such information is expected to be read from the source code.
- `RemotingTest_Core` and `RemotingTest_Rendering_Win32`:
  - Similar issues as in `CppTest_Rvm` and `RemotingTest_RvmHost` apply here.
- Fix `Project.md` if anything is affected.
- One more code review on `Source/RemotingHelpers` and affected test apps:
  - Theoretically, we prefer those `*Scope` classes over try-catch, but if finalization just does not need to be done because the exception will crash the app directly, the `*Scope` pattern itself could be omitted, and the finalization function could be called directly at the very end, pretending the exception is not going to happen.
  - No need to detect if the network connection is alive, because if any connection actually breaks, the underlying API (http.sys/WinHttp/TCP socket) will tell you at the next call. Knowing ahead the status of the network doesn't bring any benefit.
    - Remove all constructs around heartbeats. Heartbeats are unnecessary in the whole test-app organization, as all test apps are supposed to be running on the same computer, where network quality is not an issue.
    - Remove all other such mechanisms as well, not only heartbeats.
    - Some messages seem to be no longer used or useful, clean up the code in configuration and constants.
  - In test apps, all messages are for business purposes only, which means that if a construct is invented only to increase network reliability, it is not needed either.
  - Something might happen during multiple apps tearing down:
    - The library and test app logic already offer enough signals. For example, when a remote protocol core sends `ControllerConnectionStopped`, the renderer knows everything is starting to tear down. In this case, there is no need to care whether other processes are still alive, no need to care about response messages, and no need to care about network protocol issues after this point. The renderer can just ignore all negative signals and make sure it can exit directly.
    - The idea applies to all other test apps.
  - I would like to see a significant amount of code deleted without introducing too much other code.

## DETAILS

### Scope and preserved behavior

- This is a deletion-oriented refactoring of test support. Keep application-specific composition in the test apps and keep `Source/RemotingHelpers` limited to reusable implementations.
- Preserve business behavior that is still observable and required:
  - `ViewModelReadyChannel` and the `Ready` message remain the post-admission signal that the RVM host has registered its service. They are startup ordering, not liveness detection.
  - Losing `RemotingTest_RvmHost` before requester shutdown remains fatal once the loss is observed. Do not recover, reconnect, or reuse the old proxy.
  - A Core-authored `!Error`, renderer replacement, stale-renderer detachment, and `ControllerConnectionStopped` retain the behavior required by `DebugRemoteProtocolSop.md`.
  - Keep explicit async-channel detachment and transport stopping where they prevent callbacks from accessing stack objects after those objects are destroyed. These are real lifetime boundaries, not graceful peer acknowledgements.
- Do not preserve proactive idle-disconnection detection as a requirement. With no business traffic, `/Http` and `/MiniHttp` do not need to discover that a peer disappeared. The next real transport or RPC operation may report the failure; at that point the affected test app terminates instead of retrying or recovering.
- Do not add a replacement heartbeat, polling loop, lease, retry, keep-alive, disconnect acknowledgement, or reverse shutdown handshake under another name.

### Shared `Source_RemotingHelpers` project

- Delete `Source/RemotingHelpers/AutomationService/AutomationServiceHost.h` and `.cpp` before constructing the shared-item inventory.
- Add `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` and explicitly enumerate every remaining file, without wildcards:
  - `AutomationService/MiniHttpAutomationService.h` and `.cpp`.
  - `AutomationService/Windows/WindowsAutomationService.Windows.h` and `.cpp`.
  - `RemotingClient/RemotingClient.h` and `.cpp`.
  - `RemotingClient/RemotingChannelClient.h` and `.cpp`.
  - `RemotingServer/RemotingChannelServer.h`.
- Add the shared project to `GacUISrc.sln`, nest it under the existing `Source Files` solution folder, and add the normal shared-item mappings. A separate `.vcxitems.filters` file is unnecessary.
- Import the complete shared project into exactly these current consumers:
  - `CppTest`.
  - `CppTest_Metaonly`.
  - `CppTest_Reflection`.
  - `CppTest_Rvm`.
  - `GacUI_Host`.
  - `Playground`.
  - `RemotingTest_Core`.
  - `RemotingTest_Rendering_Win32`.
  - `RemotingTest_RvmHost`.
- Remove every direct `Source/RemotingHelpers` `ClCompile`/`ClInclude` entry and its matching `.vcxproj.filters` entry from those projects. Do not add project-specific exclusions merely because a portable helper is unused.
- Because `RemotingTest_RvmHost` will now compile the complete helper set, add the existing `Lib_GacUI` and `Lib_GacUI_App` dependencies needed to resolve it. Apply the same principle to the Unix source-item lists: include the no-reflection GacUI stack needed by all portable helper implementations, and exclude only the Windows-specific translation unit on non-Windows platforms.
- The current `UnitTest` sources do not test `Source/RemotingHelpers` directly, so no unit test is deleted under this rule. Keep `TestRemoteViewModelRuntime.cpp`; it tests the test-app business rule that only the exact RPC and `Ready` channel pair identifies an RVM host. Delete a unit test only if the implementation audit finds another test that directly targets a helper being removed.

### Direct automation-service ownership

- In `WindowsAutomationService.Windows.h` and `.cpp`:
  - Keep `WindowsAutomationService`, `WindowsAutomationServiceHosted`, and `WindowsAutomationServiceRenderer` as concrete value types.
  - Keep `StartWindowsHttpAutomationService` and `StopWindowsHttpAutomationService`.
  - Delete `WindowsAutomationServiceType`, `StartWindowsAutomationService`, `StopWindowsAutomationService`, `GetWindowsAutomationService`, their process-global service pointer, and all automation `*Scope` classes.
  - Delete `WindowsAutomationServiceBase::Stop()` if it still only forwards to `TBase::Stop()`.
  - Remove the scope-only dependency on `MiniHttpAutomationService.h`.
- In `MiniHttpAutomationService.h` and `.cpp`:
  - Keep the explicit `StartMiniHttpAutomationService(socketServer, applicationName)` and `StopMiniHttpAutomationService` functions.
  - Delete `MiniHttpAutomationServiceScope`.
  - Delete the overload that infers an application name from the service's available feature. Choosing Core versus renderer policy belongs to the app.
  - Fail fast when the substituted automation service is unavailable; do not silently return from `StartMiniHttpAutomationService` and leave a later `StopMiniHttpAutomationService` unmatched.
- Each app performs one straight-line sequence on the normal path: construct the concrete service as a stack value, call `Substitute(&service, false)`, start the selected endpoint, run the application, stop the endpoint, call `service.Stop()`, and call `Unsubstitute(&service)`. Do not surround this sequence with cleanup-only try-catch or a new scope wrapper.
- Use these explicit compositions:

| Application/mode | Concrete automation service | HTTP endpoint |
| --- | --- | --- |
| `CppTest` | `WindowsAutomationServiceHosted` | Windows HTTP |
| `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, `Playground` | `WindowsAutomationService` | Windows HTTP |
| `CppTest_Rvm /Pipe` and `/Http` | `WindowsAutomationServiceHosted` | Windows HTTP on port 8888 |
| `CppTest_Rvm /MiniHttp` | `WindowsAutomationServiceHosted` | MiniHTTP on the RVM socket on port 8888 |
| `RemotingTest_Core /Pipe` and `/Http` | `RemoteProtocolAutomationService` | Windows HTTP on port 8888 |
| `RemotingTest_Core /MiniHttp` | `RemoteProtocolAutomationService` | MiniHTTP on the Core protocol socket on port 8888 |
| Win32 renderer `/Pipe` and `/Http` | `WindowsAutomationServiceRenderer` | Windows HTTP on port 8889 |
| Win32 renderer `/MiniHttp` | `WindowsAutomationServiceRenderer` | MiniHTTP on its port-8889 socket |
| Wayland/macOS renderer composition in the shared renderer source | Its platform renderer automation-service value | MiniHTTP on its port-8889 socket |

- Remove `RemotingAutomationService`, `RemotingHostContext`, and the corresponding fields and parameters from `RvmGuiContext`, `CoreGuiContext`, `RendererGuiContext`, and start/run helpers. A MiniHTTP socket pointer in an app-owned context is sufficient where `GuiMain` must select the portable endpoint.

### Remote-view-model liveness and shutdown cleanup

- Reduce `RemotingRpcConfiguration` to business configuration. Remove `heartbeatMessage`, `requesterStoppingMessage`, `heartbeatIntervalMilliseconds`, `startupGraceMilliseconds`, and `leaseTimeoutMilliseconds`.
- In `RemotingClient.cpp`, remove:
  - `GetMonotonicTime` and the generic `RepeatingThread` created only for leases/heartbeats.
  - `hostLeaseActive`, `hostLeaseExpiration`, the requester state thread, lease renewal/expiry, `ClaimHostLoss`, `ProcessState`, and their wake/stop calls.
  - The hosting heartbeat thread, `SendHeartbeat`, `StartHeartbeat`, and the public `RemotingHostingClient::StartHeartbeat` API.
  - The `RequesterStopping` send/receive path. `BeginStopping` only records normal requester shutdown; stopping the real server/transport supplies the terminal signal.
  - `BroadcastingLocalClient::SendToClient`, the hosting client's control reader, and other helpers that become unused after the teardown-only message is deleted.
- Keep only the `Ready` control message. `RemotingHostingClient::SendReady` remains after the local service is registered, and requester-side registration still validates the exact accepted host ID.
- On an unexpected host disconnect before `Stopping`, invoke the configured terminal action directly. Remove the fatal lease-thread handoff, saved/taken fatal state, `GetFatalError`, and post-`Run` checks that become unreachable once the terminal action does not return. Keep the requester phase and one-host admission state because they enforce business startup/shutdown rules.
- `RemotingRequesterSession::Stop` still finalizes RPC on its task queue, stops the actual channel server, queues task-loop exit, joins the task-queue thread, and releases the acquired service. Remove only liveness and acknowledgement machinery, not required local lifetime cleanup.
- In `RemoteViewModelTestShared.h`, remove `ViewModelHeartbeatMessage`, `ViewModelRequesterStoppingMessage`, and all heartbeat/lease/grace constants. Remove the matching configuration assignments and the `StartHeartbeat()` call from `RemotingTest_RvmHost`.
- Treat a hosting-client `OnDisconnected`, read error, or fatal local error as a terminal process signal. Do not add an in-band message merely to distinguish normal requester teardown; normal requester shutdown should still leave no surviving host process.
- Remove empty destructors and one-line wrappers made redundant by the refactoring.

### Core, renderer, and transport composition

- `CppTest_Rvm`:
  - Delete `RequesterServerScope<TServer>` and use direct `Start`, session start, session stop, and server stop calls.
  - Add one variadic `StartServer<TServer>(...)` that owns parser/session/callback/server construction, requests the view model, binds the GUI context, calls `SetupHostedWindowsDirect2DRenderer`, and then performs normal straight-line shutdown.
  - Reduce `StartNamedPipeServer`, `StartHttpServer`, and `StartMiniHttpServer` to concrete calls to the template. The MiniHTTP wrapper creates and passes the one socket shared by RPC and automation.
  - Delete `Main.Linux.cpp`, remove it from the Windows project/filter, and delete `Test/Linux/CppTest_Rvm`.
- `RemotingTest_RvmHost`:
  - Keep `RunHost(networkClient)`, but inline transport parsing in `main`.
  - Put `/Pipe` and `/Http` comparisons inside the MSVC branch, keep `/MiniHttp` common, and return failure for all other input without a `Transport` enum, `ParseTransport`, a switch, or non-Windows explanatory messages.
- `RemotingTest_Core`:
  - Replace `StartConfiguredServer<TServer>` and its factory lambdas with one variadic `StartServer<TServer>(...)`, while preserving the optional RVM session and renderer-admission business rules.
  - Delete the synchronization/event machinery in `CoreFatalState`. On an RVM-host fatal error, make one best-effort `BroadcastError` call when the server is available, catch only a delivery failure at that non-returning callback boundary, and then unconditionally terminate outside the catch. This preserves the Core-authored business error without allowing a transport exception to prevent process termination.
  - Keep `RemotingChannelServer::ClearCoreChannels()` after GUI finalization and before the stack-local JSON/protocol channel objects are destroyed. `SetCoreChannels` stores pointers to those objects, so clearing them is a required lifetime boundary rather than peer-shutdown coordination.
  - After the remote controller has sent `ControllerConnectionStopped`, do not explicitly inspect and disconnect the renderer before stopping the channel server. The server stop is the terminal transport operation.
- `RemotingTest_Rendering_Win32` and `RemotingChannelClient`:
  - Keep one common client-composition function and reduce transport wrappers to concrete client construction.
  - Once `GuiRemoteRendererSingle::IsDisconnectedFromCore()` is true because `ControllerConnectionStopped` was processed, ignore later Core/local errors and disconnect callbacks.
  - Remove `disconnectedProcessed` and duplicate `BeginStopping()` calls if the renderer's existing terminal state and the one normal shutdown path make them redundant.
  - Keep first-error/fatal-retention UI state, main-thread queueing, explicit async-channel detachment, and the final transport `Stop`; they own observable business behavior or stack-callback lifetime.
- In `RemotingChannelServer::OnClientConnected`, retain renderer takeover behavior: detach the stale renderer, try to send it `ControllerConnectionStopped`, and use transport disconnection as the fallback if notification fails. Renderer replacement happens while Core remains live and is required by the SOP; it is not a heartbeat or shutdown acknowledgement.
- Audit every remaining `try`/`catch`, connection-state flag, callback, and helper in `Source/RemotingHelpers` and the affected apps. Each survivor must own required business behavior, an asynchronous non-returning exception boundary, or a concrete object-lifetime guarantee.

### Platform metadata and documentation

- Update only the hand-authored `Test/Linux/RemotingTest_Core/vmake` and `Test/Linux/RemotingTest_RvmHost/vmake` files:
  - Add `Source_RemotingHelpers.vcxitems` to `CPP_VCXPROJS`.
  - Add the no-reflection GacUI source-item dependencies needed to compile the complete portable helper set in `RemotingTest_RvmHost`.
  - Exclude the Windows helper translation unit on non-Windows platforms.
  - Do not edit generated `vmake.txt` or `makefile` files.
- Update `Project.md`, the root `README.md`, and `Test/GacUISrc/README.md` to remove the Linux `CppTest_Rvm` build-only target and to document the Windows-only local RVM app plus the portable Core/RVM-host `/MiniHttp` path.
- Update `Project.md` and the README automation description so `CppTest_Rvm /MiniHttp` is not incorrectly described as using Windows HTTP.
- Update the source-of-truth `../Tools/Copilot/Guidelines/Running-GacUI.md` first, then propagate it to `.github/Guidelines/Running-GacUI.md`. Explain that each mode/platform selects its concrete automation service and Windows HTTP or MiniHTTP start/stop functions, while the `Controls`, `Dom`, and `IO` contract is common.
- Update `.github/KnowledgeBase/manual/gacui/coding-agent/automation-service.md` to remove the deleted scope/enum examples and document direct value construction, substitution, endpoint start/stop, service stop, and unsubstitution.
- Do not modify `Import`, `Release`, generated GacUI resources, metadata binaries, or protocol schemas for this task. `Source/RemotingHelpers` remains an input to the existing remoting-helper CodePack configuration, but preparing foreign-dependency outputs is outside this repository task.

## VERIFICATION

### Static, project, and documentation checks

- Run `git diff --check`.
- Confirm `Source_RemotingHelpers.vcxitems` explicitly lists every remaining helper file exactly once, is present in `GacUISrc.sln`, and has shared-item mappings to all nine consumers listed in `DETAILS`.
- Confirm none of those consumers or their `.vcxproj.filters` files directly lists a file under `Source/RemotingHelpers`. Parse every modified `.vcxproj`, `.vcxitems`, and `.filters` file as XML.
- Confirm `AutomationServiceHost.{h,cpp}`, `CppTest_Rvm/Main.Linux.cpp`, and `Test/Linux/CppTest_Rvm` are gone with no stale project, filter, solution, `vmake`, or current documentation reference. Confirm `UnitTest/TestRemoteViewModelRuntime.cpp` remains and still verifies the exact business-channel pair.
- Search implementation and current documentation and require no remaining `WindowsAutomationServiceType`, automation-service `*Scope`, `AutomationServiceHost`, `RemotingAutomationService`, `RemotingHostContext`, `StartWindowsAutomationService`, `StopWindowsAutomationService`, `GetWindowsAutomationService`, heartbeat/lease/grace symbols, or `RequesterStopping` protocol. Historical task logs, archived learning records, `Import`, and `Release` are excluded from this search.
- Confirm every app constructs the intended concrete automation service, substitutes it directly, starts the selected endpoint, and performs endpoint stop, service stop, and unsubstitution in straight-line normal shutdown order.
- Audit all remaining catches and connection-state checks against `DebugRemoteProtocolSop.md`. Require no cleanup-only catch, heartbeat, poll, lease, retry, or teardown acknowledgement, while retaining renderer replacement, Core-authored fatal delivery, and real async lifetime boundaries. Confirm Core clears its stored channel pointers before the stack channels are destroyed, and that its RVM-host fatal callback unconditionally terminates even when `BroadcastError` fails.
- Confirm the Tools source guideline, propagated repository guideline, knowledge-base page, `Project.md`, and both README files agree on service selection, ports, `/MiniHttp`, and the removal of Linux `CppTest_Rvm`.

### Windows builds and unit tests

- From `Test/GacUISrc`, use the repository's absolute `.github/Scripts/copilotBuild.ps1` path to build all four configurations:
  - `Debug|x64`.
  - `Debug|Win32`.
  - `Release|x64`.
  - `Release|Win32`.
- After every build, require `Build.log` to end with `0 Warning(s)` and `0 Error(s)`.
- Run the complete `Debug|x64` `UnitTest` project through `copilotExecute.ps1`. Require every test file and case to pass and no memory-leak dump after the summary.
- No reflection, XML resource, protocol schema, or generated GacUI source changes are requested; do not run unrelated code-generation projects unless an implementation change expands into one of their documented trigger paths.

### Windows application verification

- Run GacUI applications through the documented repository execution/debugging workflow, with a debugger attached where required by `Running-GacUI.md`.
- Start `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and `Playground` separately. For each app:
  - require `GET http://localhost:8888/Automation/<PROJECT-NAME>/Controls` to return a nonempty valid control tree;
  - post exact `!Exit` to `/IO` and require exact `Queued`;
  - require normal process exit, then repeat immediately to prove the listener was released by straight-line teardown.
- For each of `/Pipe`, `/Http`, and `/MiniHttp`, start `CppTest_Rvm` first and then `RemotingTest_RvmHost` with the same transport:
  - require `CppTest_Rvm/Controls` to expose the running UI;
  - enter a marker and require exactly `Hello, <marker>!`, proving a live `Translate` RPC;
  - remain idle longer than the removed five-second lease, then perform another translation successfully without heartbeat traffic;
  - send exact `!Exit` and require both processes to terminate with no surviving listener or process.
- Repeat the requester/host matrix with the host terminated before the UI closes. Do not require heartbeat-based idle detection; perform the next real `Translate` operation and require the requester to terminate with an error instead of hanging, retrying, or recovering.
- For each transport, run `RemotingTest_Core /RPT` with `RemotingTest_Rendering_Win32` and perform the relevant `DebugRemoteProtocolSop.md` flow:
  - require Core `Controls` on port 8888 and renderer `Dom` on port 8889;
  - drive representative IO once through Core and once through the renderer and require both views to converge;
  - replace the renderer and require the old renderer to settle without a fatal prompt while Core remains usable;
  - close Core normally and require later renderer transport errors to be ignored after `ControllerConnectionStopped`.
- For each transport, run `RemotingTest_Core /RVMT`, `RemotingTest_RvmHost`, and the native renderer in the documented order. Verify live translation and normal shutdown. Then terminate the host, perform the next business RPC if necessary to expose the failure, and require one Core-authored fatal error to reach the renderer before Core terminates.
- Exercise both choices of the existing renderer fatal-error policy for a Core-authored `!Error`: the default close path and the retained frozen-DOM/`ExitOnly` path. Later local disconnect signals must not replace or duplicate that business error.
- In `/MiniHttp` runs, confirm Core protocol and automation share the port-8888 socket, while renderer automation owns its separate port-8889 socket. Repeat the runs to detect stale ports, callbacks, or processes.

### Separate Linux/macOS verification

- Windows results do not verify Unix platforms. Perform this section later on the claimed target OS and report Linux and macOS independently.
- On Linux, use only `.github/Ubuntu/build.sh` from each folder containing `vmake`:
  - full-build `Test/Linux/RemotingTest_Core`;
  - full-build `Test/Linux/RemotingTest_RvmHost`;
  - full-build and run `Test/Linux/UnitTest` with `/C`.
- Inspect generated `vmake.txt` files without editing them. Require every portable remoting-helper source exactly once, no `*.Windows.cpp`, and no reference to deleted `AutomationServiceHost` or `CppTest_Rvm/Main.Linux.cpp`. Run a second incremental build and require no unintended rebuild.
- Run `RemotingTest_Core /FCT /MiniHttp` asynchronously, exercise Core `Controls` and representative IO, then send exact `!Exit` and require normal termination.
- Run `RemotingTest_Core /RVMT /MiniHttp` first and then `RemotingTest_RvmHost /MiniHttp`. Require a live `Translate` result and normal queued exit. Repeat after terminating the host, perform the next translation, and require fail-fast termination without heartbeat-based polling.
- Do not build or run `CppTest_Rvm` on Linux or macOS; its Unix entry point and `vmake` target are intentionally removed.
- Repeat the applicable Core, RVM-host, and UnitTest builds on an actual macOS host before claiming macOS compatibility. If they are not run, explicitly report macOS as unverified.

## REVIEW COMMENTS

No unresolved review comments.

# UPDATES

- Added `Source_RemotingHelpers.vcxitems` with the exact nine remaining helper files, registered it in `GacUISrc.sln`, and imported it into the nine required consumers. Removed all direct helper-file entries from those projects and filters. Deleted `AutomationServiceHost.{h,cpp}`, `CppTest_Rvm/Main.Linux.cpp`, and `Test/Linux/CppTest_Rvm/vmake`.
- Replaced automation scopes, enum-based selection, and global service ownership with app-local concrete service values. Every affected app now performs the requested straight-line substitute, endpoint start, run, endpoint stop, service stop, and unsubstitute sequence. `/MiniHttp` uses the app-owned socket explicitly.
- Removed heartbeat, lease, grace-period, requester-stopping, saved-fatal, and periodic state-thread machinery. `Ready` remains the only RVM control message. Unexpected observed host disconnect calls the terminal action directly; normal UI shutdown marks the requester `Stopping` before endpoint/controller teardown, and final RPC/task/server cleanup remains explicit.
- Consolidated the duplicated CppTest RVM, Core, renderer, and RVM-host transport setup. Preserved renderer takeover, stale renderer detachment, best-effort Core-authored fatal delivery, Core channel-pointer clearing, renderer async detachment, and transport stop boundaries.
- Updated the two hand-authored portable `vmake` files, `Project.md`, both README files, the automation knowledge-base page, and the source-of-truth plus propagated `Running-GacUI.md` guideline. Linux `CppTest_Rvm` is removed; the portable Core/RVM-host `/MiniHttp` path is documented.
- Removed a temporary per-RPC timeout/worker proxy added during runtime diagnosis. It was extra liveness machinery outside the deletion-oriented scope. The final diff adds no thread-creation path and removes the requester state and hosting heartbeat threads.
- Static verification passes: the helper inventory is exact and unique; all nine project imports and ten solution mappings (nine consumers plus the shared project) are present; modified MSBuild XML parses; deleted/stale APIs and files have no exact current references; the RVM channel-pair unit test remains; automation compositions and surviving catch/connection checks match the requested business/lifetime boundaries; the two guideline copies are byte-identical; and `git diff --check` has no error.
- Runtime checks completed before GUI execution was suspended: the four simple apps that reached automation returned nonempty `Controls`, accepted exact `!Exit`, exited, and reused their listener twice. Playground still fails before automation startup in its existing resource-compilation path. CppTest RVM completed live and post-idle translations on `/Pipe`, `/Http`, and `/MiniHttp`, with shared port 8888 confirmed for MiniHTTP. `/Pipe` observed host loss directly; `/Http` and `/MiniHttp` can retain a stateless server-side queue when the host vanishes, which is intentionally not replaced by polling or a lease.
- The `/Pipe /RPT` functional flow reached converged Core/renderer UI state and successfully replaced the renderer. Severe CPU saturation occurred during a later CDB-attached Core diagnostic. The triggering tool call continued Core's existing tight remote-controller pump while CDB emitted an unbounded stream of first-chance C++ exception diagnostics through the debugger/terminal stack. Static inspection found no expanding thread-creation path in the final refactor (which removes two recurring threads), and the automation endpoint start/stop implementation is behaviorally identical to the previous scope-based form. No resource-exhaustion event was logged, so the debugger-output feedback mechanism remains the strongest explanation rather than a proven application thread leak. All Core, renderer, host, requester, and debugger processes were stopped; no further GUI test app is launched in this investigation.
- All four final-tree Windows builds (`Debug|x64`, `Debug|Win32`, `Release|x64`, and `Release|Win32`) pass with `0 Warning(s)` and `0 Error(s)`. Release builds used the unchanged repository script constrained to two logical processors at below-normal priority. The complete `Debug|x64` UnitTest run, constrained to one logical processor, passes 89/89 test files and 1714/1714 test cases with no memory-leak output. Linux and macOS builds/runs remain unverified on this Windows host.

# TEST [CONFIRMED]

Confirm the current architectural problem and preserve its required business behavior with the following checks.

- Static baseline inventory:
  - `Source/RemotingHelpers` currently contains `AutomationServiceHost.{h,cpp}`, the automation service scope classes and enum, heartbeat/lease/requester-stopping state, and the redundant wrappers named in the problem description.
  - The nine target applications directly enumerate different subsets of helper sources in their `*.vcxproj` and `*.vcxproj.filters` files; `GacUISrc.sln` has no `Source_RemotingHelpers` shared project.
  - `CppTest_Rvm/Main.Linux.cpp` and `Test/Linux/CppTest_Rvm` exist, while the two portable remoting `vmake` files do not consume a shared helper project.
  - Current documentation still describes scope/enum ownership and the Linux `CppTest_Rvm` build-only target.
- Keep `UnitTest/TestRemoteViewModelRuntime.cpp`. It verifies that only the exact RPC and `Ready` channel pair identifies an RVM host. Build Debug x64 and run the complete Debug x64 unit-test project; success requires every file and case to pass with no memory-leak dump.
- Preserve the runtime contract while deleting liveness machinery:
  - For `/Pipe`, `/Http`, and `/MiniHttp`, `CppTest_Rvm` plus `RemotingTest_RvmHost` must acquire the view model, translate a marker to exact `Hello, <marker>!`, remain usable after more than five idle seconds without heartbeat traffic, accept exact `!Exit`, and leave no process or listener behind.
  - After terminating the host, the next real RPC operation must terminate the requester instead of hanging, retrying, or recovering. Idle `/Http` and `/MiniHttp` sessions are not required to detect loss proactively.
  - For `/RPT` and `/RVMT`, preserve renderer replacement, stale-renderer detachment, the first Core-authored fatal error, the retained fatal DOM/`ExitOnly` choice, normal `ControllerConnectionStopped`, and the required explicit callback-detachment/transport-stop lifetime boundaries from `DebugRemoteProtocolSop.md`.
- After implementation, require the exact static/project/documentation checks from the problem description, XML-parse every modified project file, run `git diff --check`, build Debug/Release for x64/Win32 with zero warnings and errors, and run the complete Debug x64 unit suite without leaks.
- Exercise each migrated automation endpoint twice. Require nonempty valid `Controls` or `Dom`, exact `Queued` for `!Exit`, normal process exit, and immediate listener reuse. In `/MiniHttp`, Core/RPC and Core automation share port 8888 while renderer automation owns port 8889.
- Linux and macOS cannot be verified in this Windows environment. Only hand-authored `vmake` files are changed here; report both platforms unverified and reserve their build/run matrix for the separate target-OS verification requested by the task.

The problem is confirmed against commit `96de4c597`:

- The helper inventory contains exactly the eleven current files, including the two obsolete `AutomationServiceHost` files. `GacUISrc.sln` has no `Source_RemotingHelpers` project, and the nine target consumers directly enumerate inconsistent helper subsets in both project and filter files.
- The current implementation contains `WindowsAutomationServiceType`, all three automation scope layers, global Windows service ownership, the inferred MiniHTTP application-name overload, `RemotingHostContext`, heartbeat/lease/grace state, `RequesterStopping`, `RequesterServerScope`, `StartConfiguredServer`, synchronized `CoreFatalState`, and duplicate renderer stopping state.
- `CppTest_Rvm/Main.Linux.cpp` and `Test/Linux/CppTest_Rvm/vmake` exist. The portable Core and host `vmake` files do not import a shared helper inventory, and the documented Linux build-only target and scope/enum examples remain.
- Debug x64 `GacUISrc.sln` built successfully with `0 Warning(s)` and `0 Error(s)`.
- The complete Debug x64 `UnitTest` run passed 89/89 test files and 1,714/1,714 test cases. There was no memory-leak sidecar or appended leak dump. `TestRemoteViewModelRuntime.cpp` passed and remains the correct business-channel admission regression.

# PROPOSALS

- No.1 Delete reliability/lifecycle scaffolding and centralize helper compilation.

  Replace the duplicated per-application helper source lists with one explicit `Source_RemotingHelpers.vcxitems` inventory, imported by the nine current consumers and by the two hand-authored portable `vmake` descriptions. Delete the obsolete host abstraction and Unix-local `CppTest_Rvm` target. Each test application will own its concrete automation service as a stack value and perform the normal-path sequence directly: substitute, start the selected endpoint, run, stop the endpoint, stop the service, and unsubstitute.

  Reduce remote-view-model control traffic to the business-significant `Ready` signal. Remove heartbeat, lease, requester-stopping, saved-fatal, and polling machinery; retain exact host admission, task-queue finalization, channel detachment, renderer replacement, Core-authored fatal delivery, and stored-channel clearing where they enforce observable behavior or object lifetime. Consolidate the duplicated transport wrappers in `CppTest_Rvm`, Core, renderer, and RVM host without adding replacement reliability layers.

  Update the project metadata and current documentation to describe the concrete platform/mode compositions and the portable `/MiniHttp` path. Validate exact shared-project mappings and stale-symbol removal, XML-parse modified project files, build all four Windows configurations, run the full Debug x64 unit suite, and exercise the available Windows endpoint/process matrices. Linux and macOS runtime results remain explicitly unverified in this Windows environment.

  ### CODE CHANGE

  Implementation will delete the obsolete abstractions and liveness protocol first, then simplify each application around direct ownership, centralize the remaining helper files in the shared project, and finally align Linux metadata and documentation. No protocol schema, generated resource, `Import`, or `Release` output will be changed.
