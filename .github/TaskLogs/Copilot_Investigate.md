# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the following refactor on the design in `Test/RemotingHelpers` and test apps.

1) Move new files
- `Test/RemotingHelpers/ViewModelHostClient/ViewModelHostClient.*` to `Test/Rvmt/ViewModel/ViewModelHostClient.*`.
- `RemoteViewModelTestRuntime.h` to `Test/Rvmt/ViewModel/ViewModelHostServer.h`, and there will be a new `ViewModelHostServer.cpp`.
- `RemoteViewModelTestShared.*` to `Test/Rvmt/ViewModel/ViewModelShared.*`.
All these files should be added to the `Source_RemotingHelpers` folder, and the Solution Explorer folders should be properly updated; other projects will no longer reference these files directly.

2) Move source file content
- In `ViewModelHostClient.h` there are some `inline constexpr` declarations; move them to `ViewModelShared.h`.
- From `ViewModelHostClient.*` move `RemotingRequesterSession` to `ViewModelHostServer.*`.
- Delete `RemotingJsonDispatcherClient` and `RemotingDispatcherFactory`:
  - These two constructs were originally created to defer the `InitializeRpc` function to the implementation.
  - Now all files are moved into `Test/Rvmt/ViewModel`, so such twisted decoupling will no longer be needed.
  - `InitializeRpc` is in `ViewModelShared.cpp`; rewrite it as an ordinary function that receives enough arguments to perform the initialization.
  - Call the actual `InitializeRpc` directly; it needs to be declared in `ViewModelShared.h`, of course.
  - Therefore `CreateDispatcherFactory` will no longer be needed. Where the original `RemotingDispatcherFactory` is needed, `RpcJsonDispatcherClientForTaskQueue` could be created directly instead.

3) Inline `RemotingRequesterSession::Impl`
- `RemotingRequesterSession::Impl` is no longer needed.
- All members could just be moved into `RemotingRequesterSession` directly.
- Short methods of `RemotingRequesterSession::Impl` could just be inlined.

# Details

## Assessment and scope

The idea is correct with the implementation qualifications below.

- Keep `RemotingRequesterSession` in this task. It is currently owned and used only by `RemoteViewModelChannelServer<TServerBase>`. That is an intentional intermediate boundary, not a reason to merge the session into the server during this task.
- Move both server-side local clients, `BroadcastingLocalClient` and `RequesterLocalClient`, together with all session-only state and helpers to `ViewModelHostServer.cpp`. After this task, no server-side implementation should remain in `ViewModelHostClient.*`.
- Only flatten `RemotingRequesterSession::Impl`. Replacing the session with inheritance/overrides or merging it into `RemoteViewModelChannelServer<TServerBase>` is the future final refactoring step and is explicitly out of scope here.
- Except for the explicitly deleted dispatcher/factory layer and its constructor dependencies, preserve the remaining public names, namespaces, `RemoteViewModelChannelServer<TServerBase>::GetSession()`, and existing app-driven lifecycle methods.
- “Inline” in item 3 means removing the `Impl` forwarding layer. It does not require putting synchronization-heavy method bodies in the header. Private helper types may remain forward-declared, with an out-of-line `RemotingRequesterSession` destructor when required for incomplete types.

## File ownership after the move

- `ViewModelShared.*` owns:
  - `ViewModelChannelName`, `ViewModelReadyChannelName`, `ViewModelServiceName`, `ViewModelReadyMessage`, `RemoteViewModelHostDisconnectedError`, and `InvalidRemoteViewModelClientId`.
  - The generated RemoteViewModelTest RPC initialization and the smallest concrete dispatcher type needed to access it.
  - Shared construction/recognition of the exact `Ready` control package if helper functions are retained.
- `ViewModelHostClient.*` owns only the network-side `ViewModelHostClient`, including connection, dispatcher setup, `SendReady()`, and terminal transport-error behavior.
- `ViewModelHostServer.h` owns the `RemotingRequesterSession` declaration and the `RemoteViewModelChannelServer<TServerBase>` template.
- `ViewModelHostServer.cpp` owns the `RemotingRequesterSession` implementation and its private server-side helpers, including `RequesterPhase`, `TaskQueueThread`, `BroadcastingLocalClient`, `RequesterLocalClient`, `FinalizeRpcOnTaskQueue`, broker registration, requester task-queue ownership, and shutdown handling.
- Remove the trivial `RemoteViewModelHostClient` subclass from `Test/GacUISrc/RemotingTest_RvmHost/Main.cpp`. It exists only to inject `CreateDispatcherFactory()`; construct `ViewModelHostClient` directly after the factory is removed.

## Dispatcher qualification

The last bullet in item 2 describes direct ownership correctly, but `RpcJsonDispatcherClientForTaskQueue` cannot literally be initialized by an unrelated free function: `RpcJsonDispatcherClient::SetRpcObjects` is protected.

- Define a minimal concrete `RemoteViewModelJsonDispatcherClient : RpcJsonDispatcherClientForTaskQueue` in `ViewModelShared.*`.
- Keep `InitializeRpc(RemoteViewModelJsonDispatcherClient*, vint)` as an ordinary, non-virtual function declared in `ViewModelShared.h` and defined in `ViewModelShared.cpp`. Give that function the narrow access needed to call `SetRpcObjects`, for example by declaring it as a friend of the concrete dispatcher.
- Instantiate `RemoteViewModelJsonDispatcherClient` directly in both `RequesterLocalClient` and `ViewModelHostClient`, then call `InitializeRpc` directly.
- Remove `RemotingJsonDispatcherClient`, `RemotingDispatcherFactory`, `CreateDispatcherFactory()`, their constructor parameters, and all virtual/factory forwarding. Do not modify `Import/VlppWorkflowLibrary.h` to expose `SetRpcObjects`.

## Shared project inventory

`Source_RemotingHelpers.vcxitems` is imported by nine applications, but only these three compile the generated RemoteViewModelTest RPC module:

- `CppTest_Rvm`
- `RemotingTest_Core`
- `RemotingTest_RvmHost`

The other six importers must not acquire a generated-RPC dependency. MSBuild conditions are also unsuitable here because the Linux/macOS source extractor does not reliably evaluate them.

- Remove the old `ViewModelHostClient.*` entries from `Source_RemotingHelpers.vcxitems` and its filters.
- Add `Test/GacUISrc/Source_RemotingHelpers/Source_Rvmt_ViewModel.vcxitems` and `Source_Rvmt_ViewModel.vcxitems.filters` as a separate RVM-only shared inventory containing exactly:
  - `Test/Rvmt/ViewModel/ViewModelHostClient.*`
  - `Test/Rvmt/ViewModel/ViewModelHostServer.*`
  - `Test/Rvmt/ViewModel/ViewModelShared.*`
- Add that shared project to `Test/GacUISrc/GacUISrc.sln` and its `SharedMSBuildProjectFiles` mappings. Keep its files in the `Source_RemotingHelpers` project directory and nest `Source_Rvmt_ViewModel` beside `Source_RemotingHelpers` under the existing `GacUI_Remoting_Tests` solution folder.
- Import the RVM-only inventory from exactly `CppTest_Rvm.vcxproj`, `RemotingTest_Core.vcxproj`, and `RemotingTest_RvmHost.vcxproj`.
- Remove the old direct source/header entries and matching filter entries from those three projects. “No longer reference directly” means no direct `ClCompile` or `ClInclude` inventory entry; application source files still include the public headers they use.
- Update includes in `CppTest_Rvm/GuiMain.cpp`, `RemotingTest_Core/GuiMain.cpp`, and `RemotingTest_RvmHost/Main.cpp` to the new paths. Use the exact file and directory casing.
- Ensure every new `.cpp` is compiled exactly once by each of the three RVM applications and by no other application.

## Behavioral invariants

This is an ownership refactor. Preserve the following behavior exactly so that the later session-removal refactor starts from a known-good baseline:

- The broker and requester remain two distinct local clients with distinct client IDs, channels, readers, and dispatchers.
- Admit exactly one RVM host advertising exactly `ViewModelChannel` and `ViewModelReadyChannel`. Reject empty, mixed, duplicate, unknown, and other multi-channel combinations.
- `ViewModelReadyChannel` carrying exact `Ready` remains the post-route registration barrier. Do not register the host with the RPC broker in the remote-client admission callback.
- Renderer admission for `RemotingTest_Core /RVMT` remains closed until the host is registered, the service is acquired, the window is constructed, and `BeginRunning()` succeeds.
- An unexpected accepted-host disconnection before `Stopping` invokes the terminal action exactly once. A normal shutdown must not be reclassified as host failure.
- Preserve shutdown ordering: `BeginStopping()`, finalize requester RPC on its task queue, stop the real server, queue task-queue exit, join the task-queue thread, and release the service.
- Preserve fail-fast test-app behavior. Add no heartbeat, polling, lease, retry, reconnection, recovery, shutdown acknowledgement, requester-stopping message, or reverse shutdown handshake.
- Preserve existing renderer replacement, stale-renderer detachment, normal `ControllerConnectionStopped`, and Core-authored fatal-error behavior.

## Related files and scope limits

- Update `Project.md`, `.github/Learning/Learning_Coding.md`, and `.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md` where the old paths or the old “constants beside `ViewModelHostClient`” ownership are documented. Do not rewrite unrelated guidance.
- `Project.md`, `README.md`, and `Test/GacUISrc/README.md` currently tell verification to wait for a Core readiness line that the application does not emit. Replace that instruction with the exact observable readiness signal: Core's `/Automation/RemotingTest_Core/Controls` response contains the `Remote View Model Test` window.
- The renderer source exposes automation as `RemotingTest_Rendering_Native`, while `Project.md` and `DebugRemoteProtocolWithNativeRenderer.md` document `RemotingTest_Rendering_Win32`. Make those documents use the actual `http://localhost:8889/Automation/RemotingTest_Rendering_Native/...` route.
- Do not change the generated `RemoteViewModelTestRpc.*` files, Workflow/XML resources, reflection registration, protocol schemas, production `Source`, `Import`, or `Release`.
- Moving `ViewModelHostClient.*` out of `Test/RemotingHelpers` makes the currently generated `Release/RemotingHelpers.*` and `Release/IncludeOnly/RemotingHelpers.*` artifacts stale because `Release/CodegenConfig.xml` scans the old tree. `Project.md` forbids changing `Release` in this task, so record an authorized future CodePack regeneration/removal action explicitly; do not claim that the existing release artifacts are current.
- No metadata, parser, protocol, or `GacUI_Compiler` code generation is triggered by this task. The Release CodePack refresh is deliberately deferred as described above.

# Linux/macOS catch-up

There is no literal `Test/Linux/affected-test-apps` directory. The two affected hand-authored configurations are:

- `Test/Linux/RemotingTest_Core/vmake`
- `Test/Linux/RemotingTest_RvmHost/vmake`

Make these source-structure fixes as part of the refactoring even though they cannot be executed from the current Windows environment:

- Keep `Source_RemotingHelpers.vcxitems` in `CPP_VCXPROJS` and add the new `Source_Rvmt_ViewModel.vcxitems` to `CPP_VCXPROJS` in both files.
- Do not add `ViewModelHostClient.cpp`, `ViewModelHostServer.cpp`, or `ViewModelShared.cpp` to `CPP_ADDS`; the new shared inventory is the canonical source list.
- Remove the obsolete `-I ../../GacUISrc/RemotingTest_RvmHost` include path. Add `-I ../../Rvmt/ViewModel` only if application sources intentionally use bare header names; otherwise prefer explicit project-relative includes.
- Preserve exclusion of `WindowsAutomationService.Windows.cpp` and the existing x64 selection/removal pattern for generated `RemoteViewModelTestRpc.cpp`, with no duplicate compilation.
- Do not recreate `Test/Linux/CppTest_Rvm`; `CppTest_Rvm` is intentionally Windows-only.
- Never edit the generated `vmake.txt` or `makefile` files. Their current stale source paths are evidence that a full target-platform regeneration is still required, not files to repair manually.

Linux and macOS verification is deferred and must be reported as unverified by the Windows implementation. On each actual target OS:

1. From each affected directory, run `../../../.github/Ubuntu/build.sh -f` and then `../../../.github/Ubuntu/build.sh`.
2. Inspect regenerated `vmake.txt` and require each of `ViewModelHostClient.cpp`, `ViewModelHostServer.cpp`, and `ViewModelShared.cpp` exactly once, with no old RVM path and no Windows-only source.
3. Run `RemotingTest_Core /RVMT /MiniHttp` and `RemotingTest_RvmHost /MiniHttp` through service acquisition, an exact `Hello, <marker>!` RPC result using Core `Controls`, normal shutdown, and host-loss fail-fast behavior.
4. Smoke `/RPT /MiniHttp` and `/FCT /MiniHttp`. Use the supported platform renderer where available; on macOS, use the sibling iGac renderer for renderer replacement and fatal-delivery coverage.
5. Record Linux and macOS results independently. Windows builds are not evidence for either platform.

# Verification

## Static and project verification

- Run `git diff --check`.
- Confirm the old physical files are gone:
  - `Test/RemotingHelpers/ViewModelHostClient/ViewModelHostClient.*`
  - `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModelTestRuntime.h`
  - `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModelTestShared.*`
- Confirm exactly one `RemotingRequesterSession` declaration/implementation remains, under `ViewModelHostServer.*`, and no `RemotingRequesterSession::Impl` remains.
- Confirm `BroadcastingLocalClient`, `RequesterLocalClient`, requester task-queue/broker ownership, Ready handling, and requester shutdown logic occur only in `ViewModelHostServer.cpp`.
- Confirm `ViewModelHostClient.*` contains no session, server, local-client, broker, renderer-admission, or requester-shutdown implementation and does not include `ViewModelHostServer.h`.
- Confirm the six fixed RVM constants have one definition location in `ViewModelShared.h`.
- Confirm one ordinary `InitializeRpc` declaration/definition and direct calls from the host and requester sides.
- Confirm there is no `RemotingJsonDispatcherClient`, `RemotingDispatcherFactory`, `CreateDispatcherFactory`, or trivial `RemoteViewModelHostClient` subclass.
- Confirm the host predicate still accepts exactly the two-channel `{ViewModelChannel, ViewModelReadyChannel}` set, reserves only one host, and rejects every other channel set.
- Confirm only exact `Ready` received through `ViewModelReadyChannel` after route establishment can claim broker registration, and that registration can happen only once for the accepted host.
- Parse every changed `.vcxproj`, `.vcxitems`, and `.filters` file as XML. Wildcards are not allowed in project inventories.
- Confirm the RVM-only shared inventory is imported by exactly the three generated-RPC consumers, with all six moved files listed once, no old direct entries, and no duplicate generated-RPC symbols.
- Confirm the remaining six `Source_RemotingHelpers` importers have no dependency on `RemoteViewModelTestRpc.h` or the new RVM-only inventory.
- Search active source, project metadata, hand-authored `vmake` files, and current documentation for stale old paths. Exclude generated Unix `vmake.txt`/`makefile` and the forbidden `Import`/`Release` trees from this source-refactor check.
- Confirm the final change set contains no generated files, reflection/XML/protocol changes, production-library changes, or unrelated edits.

## Windows builds and unit-test exception

Build `Test/GacUISrc/GacUISrc.sln` only through the absolute `.github/Scripts/copilotBuild.ps1` path in all four configurations:

- Debug x64
- Debug Win32
- Release x64
- Release Win32

After each build completes, require `.github/Scripts/Build.log` to report `0 Warning(s)` and `0 Error(s)`. All four configurations are required because this task adds a shared project and changes x86/x64 generated-RPC source ownership.

By explicit task-level instruction, do not run `UnitTest` and do not add unit tests. This is an exception to the general `Project.md` execution rule: the changed C++ files belong only to test helpers/test applications, and complete application verification below replaces UnitTest execution for this task.

Run the exhaustive application matrix in Debug x64. Also run the two RVM topologies with `/MiniHttp` in Debug Win32 to execute the x86-generated RPC wrappers. Release runtime runs are not required.

For both RVM topologies, execute the complete `Remote View Model Test (/RVMT)` section in `DebugRemoteProtocolSop.md`. The RVM subsections below emphasize the task-specific acceptance criteria. Use the actual native-renderer automation route `/Automation/RemotingTest_Rendering_Native`.

## Windows application matrix

Use matching transport arguments in every process of a scenario:

| Topology | `/Pipe` | `/Http` | `/MiniHttp` |
| --- | --- | --- | --- |
| `CppTest_Rvm` + `RemotingTest_RvmHost` | Required | Required | Required |
| `RemotingTest_Core /RVMT` + host + `RemotingTest_Rendering_Win32` | Required | Required | Required |
| `RemotingTest_Core /RPT` + `RemotingTest_Rendering_Win32` | Required | Required | Required |
| `RemotingTest_Core /FCT` + `RemotingTest_Rendering_Win32` | Required | Required | Required |

For every run, follow `.github/Guidelines/Running-GacUI.md` and `.github/Guidelines/Running-ComputerUse.md`: retain process IDs, bound every wait, inspect native crash dialogs after timeouts, clean up only processes from the current run, confirm all listeners/processes stop, and confirm an immediate restart succeeds.

### `CppTest_Rvm`

For each transport:

1. Start `CppTest_Rvm` first and prove it remains blocked before a host is available.
2. Start `RemotingTest_RvmHost` with the same transport.
3. Require `CppTest_Rvm/Controls` on port 8888, the exact title `Remote View Model Test`, and live controls.
4. Enter a unique marker and require exactly `Hello, <marker>!`. UI startup alone is not proof that RPC initialization works.
5. Attempt a second host and require rejection while the first host and requester remain usable.
6. Close normally and require no stale requester, host, or listener. Do not require the host's normal-disconnection exit code to be zero because `ViewModelHostClient` deliberately terminates on disconnection.
7. In a fresh run, terminate the accepted host while the UI is running; trigger another real RPC if needed. Require requester termination with an error and no reconnect, retry, or hang.

### `RemotingTest_Core /RVMT`

For each transport:

1. Start Core first. Prove that a renderer cannot acquire the UI before the host/service is ready.
2. Start the host, wait for actual service acquisition and UI readiness, then start the renderer.
3. Require Core `Controls` on port 8888 and renderer `Dom` on port 8889. Before renderer connection, use Core `Controls` once to prove Core remains usable without a renderer.
4. Enter different unique markers through Core IO and renderer IO. Require both views to converge on exactly `Hello, <marker>!`.
5. Close the renderer unexpectedly. Require Core and host to remain alive; connect a replacement renderer and require the current marker to survive.
6. While the replacement is active, connect another renderer. Require the newcomer to take over and the stale renderer to detach without a fatal prompt or retry loop.
7. Close the application normally through the active renderer and require the normal `ControllerConnectionStopped` path with no fatal error.
8. In a fresh run, terminate the accepted host while Core and renderer are running. Require Core to terminate nonzero and the renderer to receive exactly one Core-authored `RemotingTest_RvmHost disconnected.` error.
9. Across the host-loss runs, cover both renderer choices for that fatal error: ordinary close and retained frozen fatal DOM. Later local disconnects must not replace or duplicate the business error.

### Non-RVM Core modes

- For every transport, run the complete `/RPT` scenario in `DebugRemoteProtocolSop.md`, including state mutation, renderer replacement, active takeover, stale-renderer detachment, state continuity, and intentional shutdown.
- For every transport, run the complete `/FCT` scenario in `DebugRemoteProtocolSop.md` with a fresh Core, including list mutation and text persistence.
- Exercise representative IO from both Core and renderer and require their visible states to converge.
- These runs prove that the non-RVM modes still require no RVM session or host at runtime. `RemotingTest_Core` already links the generated RVM module in every mode, so these runs do not prove that the binary lacks that link-time dependency.

### Remaining shared-helper consumers

Smoke-start `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and `Playground` in Debug x64. For each, require a nonempty `Controls` response, queue the documented exit operation, require normal termination, and require an immediate restart. `RemotingTest_Rendering_Win32` is already covered by the Core matrix.

GacJS coverage is not required unless implementation expands beyond RVM file ownership/session flattening into the generic server, renderer-client behavior, or underlying remote-protocol transport.


# UPDATES

# TEST [CONFIRMED]

This is an ownership and build-inventory refactor rather than a behavioral defect requiring a new unit test. The task explicitly forbids adding or running `UnitTest`; verification uses static ownership checks, four complete solution builds, and the application matrix from the problem description.

The current tree confirms the structural problem:

- `Test/RemotingHelpers/ViewModelHostClient/ViewModelHostClient.*` combines the network host client with `BroadcastingLocalClient`, `RequesterLocalClient`, broker/task-queue state, `RemotingRequesterSession`, and `RemotingRequesterSession::Impl`.
- `RemotingJsonDispatcherClient`, `RemotingDispatcherFactory`, and `CreateDispatcherFactory()` exist only to defer generated RemoteViewModelTest RPC initialization across the current directory boundary.
- `RemoteViewModelTestRuntime.h` and `RemoteViewModelTestShared.*` remain under `Test/GacUISrc/RemotingTest_RvmHost`, while `CppTest_Rvm` and `RemotingTest_Core` compile/reference them directly from that application directory.
- The general nine-consumer `Source_RemotingHelpers.vcxitems` includes `ViewModelHostClient.*`, even though the RVM implementation depends on the generated RemoteViewModelTest RPC module that only three consumers compile.
- The two hand-authored Unix `vmake` files still depend on the old application directory include path, and current documentation names a nonexistent Core readiness console line and the wrong native-renderer automation prefix.

Success requires every static ownership/inventory criterion in the problem description, XML parsing of all changed project files, `git diff --check`, clean Debug/Release x Win32/x64 solution builds with zero warnings and errors, and the complete prescribed Windows application matrix. The refactor must preserve the exact Ready barrier, singleton host admission, requester/broker separation, renderer admission phase, fatal host-loss behavior, and shutdown ordering. Linux and macOS remain explicitly unverified until their target-platform build and runtime steps are executed.

# PROPOSALS

- No.1 Split the RVM-specific helper inventory and flatten requester ownership

## No.1 Split the RVM-specific helper inventory and flatten requester ownership

Move the six RVM-specific view-model files into `Test/Rvmt/ViewModel` and compile them through a dedicated `Source_Rvmt_ViewModel.vcxitems` imported only by `CppTest_Rvm`, `RemotingTest_Core`, and `RemotingTest_RvmHost`. Keep the generated-RPC-independent helper inventory in `Source_RemotingHelpers.vcxitems` for its existing nine consumers.

Make `ViewModelShared.*` the single owner of fixed RVM constants and generated RPC dispatcher initialization. Replace the abstract dispatcher/factory pair with the concrete `RemoteViewModelJsonDispatcherClient`, whose narrowly friended ordinary `InitializeRpc` function can call protected `SetRpcObjects`.

Make `ViewModelHostClient.*` network-host-only. Move `RemotingRequesterSession`, both server-side local clients, task-queue/broker helpers, Ready processing, admission state, and shutdown logic to `ViewModelHostServer.*`. Flatten only `RemotingRequesterSession::Impl`: store the session state directly in `RemotingRequesterSession`, keep synchronization-heavy method bodies out of line, and preserve the public lifecycle and `RemoteViewModelChannelServer<TServerBase>::GetSession()` boundary.

Update all Windows project/solution inventory, the two hand-authored Unix `vmake` files, application includes, and the narrowly affected documentation. Preserve every behavioral invariant and defer Release CodePack regeneration because `Release` is forbidden in this task.

### CODE CHANGE
