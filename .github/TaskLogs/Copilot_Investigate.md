# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

1) Release `Workflow` to `GacUI`, as `RpcJsonDispatcherClient::SetRpcObjects` was made public.

2) Delete `RemoteViewModelJsonDispatcherClient`.
- `InitializeRpc` will become a static function receiving a `rpc_controller::channeling::RpcJsonDispatcherClient` and `clientId`.
- Files named `RemoteViewModelTestInitialize.(h|cpp)` will be created in `Test/GacUISrc/Generated_RemoteViewModelTest` and added to `Generated_RemoteViewModelTest`.
  - Note that `RemoteViewModelTestRpc.h` could be either the x64 or x86 version. It will be resolved correctly because test apps already set the import folder per platform.
- `CreateViewModelReadyMessage` and `IsViewModelReadyMessage` can be moved to `ViewModelShared.h` as inline functions.
- Delete `ViewModelShared.cpp`; `RemoteViewModelJsonDispatcherClient` is no longer needed.

3) Remove the actual dependency of `Test/RemotingHelpers/Rvmt` on `Generated_RemoteViewModelTest`.
- The goal in this step is to let each test app call `InitializeRpc` directly.
- `RemotingTest_RvmHost`:
  - `channelClient->Connect();` is called in `Main.cpp`.
  - `Connect()` currently calls `InitializeRpc`, but it should no longer do so.
  - `InitializeRpc` will be called immediately after `Connect()`.
- `CppTest_Rvm` and `RemotingTest_Core`:
  - Currently `RpcServerHelpers::RequestService` calls `RpcServiceAccessLocalClient::Connect`, which calls `InitializeRpc`. Although it is not currently the case in the test apps, `RpcServerHelpers::RequestService` should be callable multiple times, as there is no limitation that `RemotingTest_RvmHost` can expose only one service (although in fact it does). As a demo app, it should show how to architect this correctly. Assume the design should allow `RequestService` to be called multiple times for multiple services, even for the same type name. The underlying architecture already ensures that each type name has only one instance, so `RpcServerHelpers` does not need to worry about this.
  - Both `GuiMain.cpp` files call `RequestService`.
  - Add `RemoteViewModelChannelServer::Connect`, which calls `RpcServerHelpers::Connect`, which calls `requesterClient->Connect`; the latter should no longer call `InitializeRpc` internally.
    - Pay attention to `RpcServerHelpers::Connect`: although `InitializeRpc` from `requesterClient->Connect` is currently called in the middle, much of the code after it also appears to belong in `Connect`. Carefully determine which work should happen only once and put it in `RpcServerHelpers::Connect`.
  - Both `GuiMain.cpp` files should now call, in order:
    - `Connect`
    - `InitializeRpc`
    - `RequestService`

4) All files in `Test/RemotingHelpers/Rvmt` are now clean:
- They no longer depend on `Generated_RemoteViewModelTest` syntactically or semantically, which means they are independent of any actual Workflow RPC-generated code.
- `Release/CodegenConfig.xml` should now include `Test/RemotingHelpers`, generating `Test.RemotingHelpers(.Windows)?.(h|cpp)`:
  - Fix `../Tools/Tools/Build.ps1 UpdateRelease`: after these files are copied to the `Release` folder, delete them immediately. Test files are not included in the release.
    - To verify it, run `Build.ps1 UpdateRelease` and confirm that they are deleted. Revert all changes made to `Release`; do not actually update the `Release` repo.
  - Fix `../wGac/import.sh` and `../iGac/import.sh`. Unfortunately, you are not able to test them, but the change should be very simple:
    - Delete the whole `Import-Test` folder if it exists, and recreate it.
    - After copying all code, move `Test.RemotingHelpers(.Windows)?.(h|cpp)` from `Import` to `Import-Test`.

## DETAILS

### Workflow release and generated initializer

- `Workflow/Source` already exposes `RpcJsonDispatcherClient::SetRpcObjects` publicly, but `Workflow/Release/VlppWorkflowLibrary.{h,cpp}` and `GacUI/Import/VlppWorkflowLibrary.{h,cpp}` are stale. Regenerate the Workflow release through its normal CodePack pipeline, then import it through the normal GacUI pipeline; do not hand-edit released or imported amalgamated files. The resulting declaration must be public and the obsolete `GetRpcJsonLifecycle` release/import surface must be gone.
- Add the handwritten common pair `RemoteViewModelTestInitialize.h/.cpp` at the root of `Test/GacUISrc/Generated_RemoteViewModelTest`. This is a narrow task-authorized exception inside an otherwise generated folder; do not hand-edit the generated x86/x64 RPC files.
- Define `RemoteViewModelTestInitialize::InitializeRpc` as a public static member receiving a non-owning `rpc_controller::channeling::RpcJsonDispatcherClient*` and a `vint clientId`. Its implementation owns all references to `vl_workflow_global::RemoteViewModelTestRpc` and constructs/configures the generated lifecycle before calling the now-public `SetRpcObjects`.
- Let the common initializer source include the platform-selected `RemoteViewModelTestRpc.h`. Keep platform selection in the consuming projects' existing x86/x64 include paths rather than adding platform knowledge to `Test/RemotingHelpers`.

### Generic helper boundary

- Replace every `RemoteViewModelJsonDispatcherClient` field and construction in `Test/RemotingHelpers/Rvmt` with the existing generic task-queue dispatcher (`RpcJsonDispatcherClientForTaskQueue`, via the `RpcDispatcherClient` alias where appropriate). `ViewModelShared.h` keeps only generic aliases, fixed RVM channel/error constants, and the inline ready-message helpers.
- Delete `ViewModelShared.cpp` and remove it from `Source_RemotingHelpers.vcxitems` and `Source_RemotingHelpers.vcxitems.filters`. Also remove the generated x86/x64 `Generated_RemoteViewModelTest` include directories from that shared-items project.
- After the refactor, nothing below `Test/RemotingHelpers` may include `RemoteViewModelTestRpc.h`, name `vl_workflow_global::RemoteViewModelTestRpc`, or otherwise depend on generated RVM types. Generated composition belongs only to the affected test applications and the new initializer pair.

### Host-side connection order

- `ViewModelHostClient::Connect` must perform only the generic transport/channel connection (`WaitForServer`) and ready-control-channel lookup. It must not initialize generated RPC objects.
- In `RemotingTest_RvmHost/Main.cpp`, keep the stable order: `Connect`; `RemoteViewModelTestInitialize::InitializeRpc(dispatcher, channelClient->GetClientId())`; resolve and register the local service; send the Ready message; initialize the dispatcher; run the task queue. Preserve the Ready barrier and do not expose the host to requester admission before the service is registered.

### Requester connection and repeatable service access

- Add an explicit one-time `RemoteViewModelChannelServer::Connect`/`RpcServerHelpers::Connect` phase. `Connect` must receive the complete list of service type names that this requester can request during the connection. `RpcJsonDispatcherClient::PrepareConnection` captures that list before connecting, so an empty or incomplete list followed by immediate `RequestService` can race asynchronous broker registration. Repeated requests may use the same type or any type declared to `Connect`; supporting arbitrary new type names after connection is outside this task and would require a new Workflow waiting API.
- `RpcServerHelpers::Connect` owns all generic work that happens once: connect the requester local client with the required-service list, set the server local-client ID, publish the generic requester dispatcher, replay any latched accepted-host loss outside helper locks, register the requester with the broker, and start the task-queue thread. It must expose the non-owning generic dispatcher and assigned requester `clientId` to the application so the application can call `RemoteViewModelTestInitialize::InitializeRpc` directly.
- Keep generated initialization outside both `RpcServiceAccessLocalClient::Connect` and `RpcServerHelpers::Connect`. In each `CppTest_Rvm` and `RemotingTest_Core` `GuiMain.cpp`, call `Connect(requiredServiceNames)`, then the generated initializer with the returned/exposed dispatcher and client ID, then `RequestService`.
- On the first `RequestService`, initialize the generic dispatcher exactly once, acquire the requested service, and enter `RequesterPhase::Running` only after successful acquisition. Later calls perform only the lifecycle lookup and return their own result. Remove the one-shot `!rpcInitialized` rejection and the single cached `service` member.
- Track connection/task-queue startup separately from dispatcher initialization. `Stop` must still terminate and join a task queue started by `Connect` if generated initialization or the first request fails.
- Preserve singleton-host admission, the startup host-loss latch, injection outside `lockState`/broker locks, stopping suppression, requester/broker separation, and renderer admission only after the first successful service acquisition.

### Project and platform inventories

- Add the common initializer pair to `Generated_RemoteViewModelTest.vcxitems` and its `.filters` file under a common/shared filter with no platform exclusion.
- `RemotingTest_RvmHost` does not import that shared-items project, so add the pair explicitly to its `.vcxproj` and `.filters`. Keep its architecture-specific generated RPC source selection unchanged.
- Add the common initializer source explicitly to `Test/Linux/RemotingTest_Core/vmake`. Confirm Linux RvmHost obtains it exactly once from its updated Visual Studio project inventory.
- Remove the no-longer-needed `Generated_RemoteViewModelTest` shared-items import from `RemotingTest_Rendering_Win32.vcxproj` and the corresponding solution shared-items mapping from `GacUISrc.sln`.

### Test-only CodePack and release boundaries

- Extend `Release/CodegenConfig.xml` with the folder, category, and output codepair needed to generate `Test.RemotingHelpers.h/.cpp` in both `GacUI/Release` and `GacUI/Release/IncludeOnly`. Merely adding the folder is insufficient. The current tree has no `.Windows` helper source, so do not require an empty `Test.RemotingHelpers.Windows` pair; support that exact optional pair in cleanup/import logic if matching source exists in the future.
- Keep these dedicated test packs in the GacUI repository because `wGac` and `iGac` consume them. They must not be merged into `GacUI.*`/`GacUI.Windows.*`, and they must not enter the aggregate sibling `Release` repository.
- `../Tools/Tools/BuildRelease.ps1` currently copies only `Gac*.h/.cpp` into `../Release/Import`, so preserve that production allowlist. Add exact defensive cleanup for stale `Test.RemotingHelpers.{h,cpp}` and `Test.RemotingHelpers.Windows.{h,cpp}` in the aggregate `Release/Import`; never delete the source pairs from `GacUI/Release` or `GacUI/Release/IncludeOnly`.
- In both lowercase platform scripts, make any existing `Import` and `Import-Test` trees writable, delete and recreate both, copy the normal GacUI release snapshot, then move the required neutral helper pair and any optional Windows pair from `Import` to `Import-Test`. Fail if the required neutral pair is missing, handle the optional pair without an unmatched-glob failure, and make both final trees read-only.
- Update `wGac/README.md`, `wGac/README_CN.md`, and `iGac/readme.md` to describe `Import-Test` and its test-only helper pairs, as required by those repositories' maintenance rules.

### Documentation and knowledge

- Keep the existing Learning heading **`Test/RemotingHelpers` stays test-only and outside Release CodePack** unchanged, including its index entry, but revise the body in `.github/Learning/Learning_Coding.md`. Explain that "Release CodePack" in this heading means the production/public artifact set assembled into the aggregate `Release` repository: the same GacUI CodePack run may emit separate test-only pairs retained only for platform test imports.
- Update the matching statements in `Project.md`: `Test/RemotingHelpers` remains forbidden to production `Source` and excluded from ordinary public GacUI/aggregate Release artifacts, generated-specific initialization moves to `RemoteViewModelTestInitialize.*`, and the dedicated test pairs live in GacUI `Release`/`Release/IncludeOnly` for `Import-Test` consumers.
- Update `.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md` so `ViewModelShared.*` no longer owns generated RPC initialization. Update the Workflow RPC JSON manual/knowledge that still recommends subclassing merely to reach protected `SetRpcObjects`. Maintain canonical and synchronized knowledge/manual copies together according to each repository's knowledge-base rules.

## VERIFICATION

### Static, generated, and build checks

- Run `git diff --check` in every changed repository. Validate XML, `.vcxitems`, `.filters`, solution mappings, and `vmake` inventories; every common initializer source must compile exactly once per target.
- Search all of `Test/RemotingHelpers` and confirm there is no generated RVM include, symbol, type, generated include directory, or `ViewModelShared.cpp` inventory entry. Inspect the generated test codepair(s) and confirm they contain no `RemoteViewModelTestRpc`, `vl_workflow_global`, or deleted dispatcher-subclass reference, while all remaining helper headers and implementations are present.
- Run the prescribed full Workflow release task (`../Tools/Tools/Build.ps1 Workflow`) and GacUI import/build/release task (`../Tools/Tools/Build.ps1 GacUI`). Inspect output as well as exit status. Confirm Workflow Release and GacUI Import match, `SetRpcObjects` is public, and the obsolete accessor is absent.
- Build `Test/GacUISrc/GacUISrc.sln` through `.github/Scripts/copilotBuild.ps1` in `Debug|x64` and `Debug|Win32`; require `0 Warning(s)` and `0 Error(s)`. Run `UnitTest` in `Debug|x64` through `.github/Scripts/copilotExecute.ps1`; require all selected tests to pass and no memory-leak dump.
- Build Linux `RemotingTest_Core` and `RemotingTest_RvmHost` with `.github/Ubuntu/build.sh -f`, and verify their generated source inventories contain the common initializer exactly once.

### RPC behavior

- Add or perform a focused check that calls `RequestService` twice after one `Connect` and one generated initialization, retains both returned wrappers, and successfully invokes both. Cover repeated requests for the same type; cover multiple declared types if another available test service makes that practical. Confirm no second connection, task-queue start, dispatcher initialization, or generated initialization occurs.
- On Windows, exercise `CppTest_Rvm` + `RemotingTest_RvmHost` and `RemotingTest_Core /RVMT` + `RemotingTest_RvmHost` + `RemotingTest_Rendering_Win32` over `/Pipe`, `/Http`, and `/MiniHttp`. Start the requester before the host, perform a real `Translate` and check the exact response, then close normally with no surviving process or listener.
- Preserve the existing host-loss behavior: cover accepted-host loss during startup and during/just before the next real RPC, require bounded failure rather than a hang, and confirm normal stop or renderer replacement does not inject a false host-loss failure. Keep renderer admission blocked until requester initialization and first service acquisition complete.
- Run the portable `/MiniHttp` flow on Linux and macOS only when those environments are available; otherwise record those platforms explicitly as unverified.

### Packaging and documentation

- After GacUI CodePack, require `GacUI/Release/Test.RemotingHelpers.{h,cpp}` and the matching `Release/IncludeOnly` pair. Require a `.Windows` pair only when matching Windows helper inputs exist. Confirm ordinary `GacUI*` pairs do not absorb test-helper code.
- Snapshot the aggregate `../Release` worktree, run `../Tools/Tools/Build.ps1 UpdateRelease`, and verify the exact helper pairs are absent from `../Release/Import` while they remain in GacUI Release. Inspect output because the wrapper may print a failure without rethrowing. Restore only changes produced by this verification and leave the aggregate Release repository exactly as it was before the command.
- Run `bash -n` on `../wGac/import.sh` and `../iGac/import.sh`. If their native import environments are unavailable, do not claim an end-to-end platform import; statically verify exact paths, writable cleanup, required/optional pair handling, and final read-only permissions. When runnable, require ordinary files only in `Import` and helper pairs only in `Import-Test`.
- Re-read the updated Learning, `Project.md`, RPC design knowledge/manuals, and platform READMEs. Confirm they consistently describe the separate test-only CodePack pairs and the production/aggregate Release exclusion while retaining the requested Learning title.

## REVIEW COMMENTS

# TEST [CONFIRMED]

The reproduction is confirmed in the unmodified implementation. `Test/RemotingHelpers/Rvmt/ViewModelShared.cpp` directly includes `RemoteViewModelTestRpc.h` and names `vl_workflow_global::RemoteViewModelTestRpc`; `ViewModelHostClient` and `RpcServiceAccessLocalClient` construct `RemoteViewModelJsonDispatcherClient`; the shared-items inventory injects generated x86/x64 include paths; and `RpcServerHelpers::RequestService` rejects every call after the first with `CHECK_ERROR(!rpcInitialized, ...)`. Workflow source already exposes `SetRpcObjects` publicly, while Workflow Release and GacUI Import still keep it protected and retain `GetRpcJsonLifecycle`. The existing `Debug|x64` solution baseline builds successfully with zero warnings and zero errors, confirming the reproduction is a design/inventory defect rather than a pre-existing compile failure.

- Static dependency reproduction:
  - Search `Test/RemotingHelpers` for `RemoteViewModelTestRpc.h`, `vl_workflow_global::RemoteViewModelTestRpc`, `RemoteViewModelJsonDispatcherClient`, generated include directories, and the `ViewModelShared.cpp` inventory entry. The current tree fails the requested generic-helper boundary when any of these generated-specific dependencies are present.
  - Inspect `RpcServerHelpers::RequestService` and confirm the current `!rpcInitialized` invariant, per-call requester connection, dispatcher initialization, task-queue startup, and cached single `service` make a second service request impossible.
  - Inspect Workflow `Source`, Workflow `Release`, and GacUI `Import`: the source declaration is public, while the released/imported declarations are stale and the obsolete `GetRpcJsonLifecycle` surface remains.
- Focused behavior regression:
  - After one explicit `Connect(requiredServiceNames)` and one `RemoteViewModelTestInitialize::InitializeRpc`, request `rvmt::IViewModel` twice, retain both wrappers, and call `Translate` through each wrapper. Both calls must return the exact expected greetings without a second transport connection, task-queue start, dispatcher initialization, or generated initialization.
- Full acceptance:
  - Require the static, generated, Windows build/unit-test, Linux inventory/build, Windows `/Pipe`/`/Http`/`/MiniHttp` RPC, host-loss, packaging, shell-syntax, and documentation checks listed in `# PROBLEM DESCRIPTION`.
  - Windows builds must end with `0 Warning(s)` and `0 Error(s)`; the selected unit tests must all pass without a memory-leak dump. The aggregate `Release` repository must remain byte-for-byte/worktree clean after `UpdateRelease` verification.

# PROPOSALS
